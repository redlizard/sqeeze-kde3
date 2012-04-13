    /*

    Copyright (C) 2001-2003 Stefan Westerfeld
                            stefan@space.twc.de
 
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
   
    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*
 * Only compile this AudioIO class if we have MAS
 */
#ifdef HAVE_LIBMAS

extern "C" {
#include <mas/mas.h>
#include <mas/mas_getset.h>
#include <mas/mas_source.h>
}

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <algorithm>

#include "debug.h"
#include "audioio.h"
#include "audiosubsys.h"
#include "iomanager.h"
#include "dispatcher.h"

namespace Arts {

    class AudioIOMAS : public AudioIO, public TimeNotify {
    protected:
		mas_channel_t audio_channel;
		mas_port_t    mix_sink;
		mas_port_t    srate_source, srate_sink;
		mas_port_t    audio_source, audio_sink;
		mas_port_t    endian_sink, endian_source;
		mas_port_t    sbuf_source, sbuf_sink;
		mas_port_t    squant_sink, squant_source;
		mas_port_t    open_source; /* (!) */
		mas_device_t  endian;
		mas_device_t  srate;
		mas_device_t  squant;
		mas_device_t  sbuf;
		mas_data     *data;
    	mas_package	  package;
        int32         mas_error;

		std::list<mas_channel_t>	allocated_channels;
		std::list<mas_port_t>		allocated_ports;
		std::list<mas_device_t>		allocated_devices;

		double lastUpdate, bytesPerSec;
		int  readBufferAvailable;
		int  writeBufferAvailable;

		double currentTime();
		void updateBufferSizes();

#ifdef WORDS_BIGENDIAN
		static const int defaultFormat = 17;
#else
		static const int defaultFormat = 16;
#endif
		bool close_with_error(const std::string& text);
    public:
		AudioIOMAS();

		// Timer callback
		void notifyTime();

		void setParam(AudioParam param, int& value);
		int getParam(AudioParam param);

		bool open();
		void close();
		int read(void *buffer, int size);
		int write(void *buffer, int size);
    };

    REGISTER_AUDIO_IO(AudioIOMAS,"mas","MAS Audio Input/Output");

};

using namespace std;
using namespace Arts;

AudioIOMAS::AudioIOMAS()
{
    /*
     * default parameters
     */
    param(samplingRate) = 44100;
    paramStr(deviceName) = ""; // TODO
    param(fragmentSize) = 4096;
    param(fragmentCount) = 7;
    param(channels) = 2;
    param(direction) = 2;
    param(format) = defaultFormat;
}

namespace {
	int masInitCount = 0;
}

// Opens the audio device
bool AudioIOMAS::open()
{
    string& _error = paramStr(lastError);
    string& _deviceName = paramStr(deviceName);
    int& _channels = param(channels);
    int& _fragmentSize = param(fragmentSize);
    int& _fragmentCount = param(fragmentCount);
    int& _samplingRate = param(samplingRate);
    int& _format = param(format);

	/* FIXME: do we need to free what we allocate with mas_init() in close() */
	if (!masInitCount)
	{
		mas_error = mas_init();

		if (mas_error < 0)
			return close_with_error("error connecting to MAS server");
	}
	masInitCount++;

	if (param(direction) != 2)
	{
		_error = "unsupported direction (currently no full duplex support)";
		return false;
	}

	/*
	 * data path
	 *
	 * audio_sink
	 * audio_channel: data_channel ("artsd")
	 * audio_source
	 *   |
	 *   V
	 * endian_sink
	 * endian: instantiate_device ("endian")
	 * open_source = endian_source
	 *   |
	 *   V
	 * [squant_sink]
	 * [squant]
	 * [squant_source]
	 *   |
	 *   V
	 * [srate_sink]
	 * [srate]
	 * [srate_source]
	 *   |
	 *   V
	 * sbuf_sink
	 * sbuf
	 * sbuf_source
	 *   |
	 *   V
	 * mix_sink: port ("default_mix_sink")
	 */

	// audio_channel, source & sink
	mas_error = mas_make_data_channel("artsd", &audio_channel, &audio_source, &audio_sink);
	if (mas_error < 0)
		return close_with_error("error initializing MAS data channel");

	allocated_channels.push_back(audio_channel);
	allocated_ports.push_back(audio_source);
	allocated_ports.push_back(audio_sink);

	// endian, source & sink
	mas_error = mas_asm_instantiate_device( "endian", 0, 0, &endian );
	if ( mas_error < 0 )
		return close_with_error("error initantiating MAS endian device");

	allocated_devices.push_back(endian);

	mas_error = mas_asm_get_port_by_name( endian, "sink", &endian_sink );
	if ( mas_error < 0 )
		return close_with_error("error getting MAS endian device sink port");

	allocated_ports.push_back(endian_sink);

	mas_error = mas_asm_get_port_by_name( endian, "source", &endian_source );
	if ( mas_error < 0 )
		return close_with_error("error getting MAS endian device source port");

	allocated_ports.push_back(endian_source);

	char ratestring[16], resolutionstring[16];
	sprintf (ratestring, "%u", _samplingRate);
	sprintf (resolutionstring, "%u", _format);

	mas_data_characteristic* dc;

    dc = (mas_data_characteristic *)MAS_NEW( dc );
	masc_setup_dc( dc, 6 );
	masc_append_dc_key_value( dc, "format", (_format==8) ? "ulinear":"linear" );

	masc_append_dc_key_value( dc, "resolution", resolutionstring );
	masc_append_dc_key_value( dc, "sampling rate", ratestring );
	masc_append_dc_key_value( dc, "channels", "2" );
	masc_append_dc_key_value( dc, "endian", "little" );

    mas_error = mas_asm_connect_source_sink( audio_source, endian_sink, dc );
	if ( mas_error < 0 )
		return close_with_error("error connecting MAS net audio source to endian sink");

    /* The next device is 'if needed' only. After the following if()
       statement, open_source will contain the current unconnected
       source in the path (will be either endian_source or
       squant_source in this case)
    */
    open_source = endian_source;
    
    if ( _format != 16 )
    {
		arts_debug("MAS output: Sample resolution is not 16 bit/sample, instantiating squant device.");

		// squant, source & sink
        mas_error = mas_asm_instantiate_device( "squant", 0, 0, &squant );
		if ( mas_error < 0 ) 
			return close_with_error("error creating MAS squant device");

		allocated_devices.push_back(squant);

		mas_error = mas_asm_get_port_by_name( squant, "sink", &squant_sink );
		if ( mas_error < 0 ) 
			return close_with_error("error getting MAS squant device sink port");

		allocated_ports.push_back(squant_sink);

		mas_error = mas_asm_get_port_by_name( squant, "source", &squant_source );
		if ( mas_error < 0 ) 
			return close_with_error("error getting MAS squant device source port");
        
		allocated_ports.push_back(squant_source);

        arts_debug( "MAS output: Connecting endian -> squant.");

        masc_strike_dc( dc );
        masc_setup_dc( dc, 6 );
        masc_append_dc_key_value( dc,"format",(_format==8) ? "ulinear":"linear" );
        masc_append_dc_key_value( dc, "resolution", resolutionstring );
        masc_append_dc_key_value( dc, "sampling rate", ratestring );
        masc_append_dc_key_value( dc, "channels", "2" );
        masc_append_dc_key_value( dc, "endian", "host" );

        mas_error = mas_asm_connect_source_sink( endian_source, squant_sink, dc );
        if ( mas_error < 0 ) 
			return close_with_error("error connecting MAS endian output to squant device");

        /* sneaky: the squant device is optional -> pretend it isn't there */
        open_source = squant_source;
    }

    
    /* Another 'if necessary' device, as above */
    if ( _samplingRate != 44100 )
    {
		arts_debug ("MAS output: Sample rate is not 44100, instantiating srate device.");

		// srate, source & sink
		mas_error = mas_asm_instantiate_device( "srate", 0, 0, &srate );
		if ( mas_error < 0 ) 
			return close_with_error("error initantiating MAS srate device");
	
		allocated_devices.push_back(srate);

		mas_error = mas_asm_get_port_by_name( srate, "sink", &srate_sink );
		if ( mas_error < 0 ) 
			return close_with_error("error getting MAS srate sink port");

		allocated_ports.push_back(srate_sink);

		mas_error = mas_asm_get_port_by_name( srate, "source", &srate_source );
		if ( mas_error < 0 ) 
			return close_with_error("error getting MAS srate source port");

		allocated_ports.push_back(srate_source);

		arts_debug( "MAS output: Connecting to srate.");
        
		masc_strike_dc( dc );
		masc_setup_dc( dc, 6 );
		masc_append_dc_key_value( dc, "format", "linear" );
		masc_append_dc_key_value( dc, "resolution", "16" );
		masc_append_dc_key_value( dc, "sampling rate", ratestring );
		masc_append_dc_key_value( dc, "channels", "2" );
		masc_append_dc_key_value( dc, "endian", "host" );
		
		mas_error = mas_asm_connect_source_sink( open_source, srate_sink, dc );
		if ( mas_error < 0 ) 
			return close_with_error("error connecting to MAS srate device");

        open_source = srate_source;        
    }

	// sbuf, source & sink
    mas_error = mas_asm_instantiate_device( "sbuf", 0, 0, &sbuf );
	if ( mas_error < 0 ) 
		return close_with_error("error initantiating MAS sbuf device");

	allocated_devices.push_back(sbuf);

	mas_error = mas_asm_get_port_by_name( sbuf, "sink", &sbuf_sink );
	if ( mas_error < 0 )
		return close_with_error("error getting MAS sbuf device sink port");

	allocated_ports.push_back(sbuf_sink);

	mas_error = mas_asm_get_port_by_name( sbuf, "source", &sbuf_source );
	if ( mas_error < 0 )
		return close_with_error("error getting MAS sbuf device source port");

	allocated_ports.push_back(sbuf_source);

    masc_strike_dc( dc );
    masc_setup_dc( dc, 6 );

    masc_append_dc_key_value( dc, "format", "linear" );
    masc_append_dc_key_value( dc, "resolution", "16" );
    masc_append_dc_key_value( dc, "sampling rate", "44100" );
    masc_append_dc_key_value( dc, "channels", "2" );
    masc_append_dc_key_value( dc, "endian", "host" );

	arts_debug("MAS output: Connecting to sbuf.");

    mas_error = mas_asm_connect_source_sink( open_source, sbuf_sink, dc );
    if ( mas_error < 0 ) 
		return close_with_error("error connecting to MAS mixer device");

	/* configure sbuf */

	float BUFTIME_MS = _fragmentSize * _fragmentCount;
	BUFTIME_MS *= 1000.0;
	BUFTIME_MS /= (float)_channels;
	if (_format > 8)
		BUFTIME_MS /= 2.0;
	BUFTIME_MS /= (float)_samplingRate;

	arts_debug("MAS output: BUFTIME_MS = %f", BUFTIME_MS);

	masc_setup_package( &package, NULL, 0, 0 );
	masc_pushk_uint32( &package, "buftime_ms", (uint32) BUFTIME_MS );
	masc_finalize_package( &package );
	mas_set( sbuf, "buftime_ms", &package );
	masc_strike_package( &package );

	masc_setup_package( &package, NULL, 0, 0 );
	masc_pushk_int32( &package, "mc_clkid", 9 );
	masc_finalize_package( &package );
	mas_set( sbuf, "mc_clkid", &package );
	masc_strike_package( &package );

	mas_source_play( sbuf );

	// mix_sink
   	mas_error = mas_asm_get_port_by_name( 0, "default_mix_sink", &mix_sink );
	if (mas_error < 0)
		return close_with_error("error finding MAS default sink");

	allocated_ports.push_back(mix_sink);

	arts_debug("MAS output: Connecting sbuf to mix_sink.");

    mas_error = mas_asm_connect_source_sink( sbuf_source, mix_sink, dc );
    if ( mas_error < 0 ) 
		return close_with_error("error connecting to MAS mixer device");

	data = (mas_data *)MAS_NEW( data );
	masc_setup_data( data, _fragmentSize ); /* we can reuse this */
	data->length = _fragmentSize;
	data->allocated_length = data->length;
	data->header.type = 10;

    arts_debug("MAS output: playing.");
 
    // Install the timer
    Dispatcher::the()->ioManager()->addTimer(10, this);

	bytesPerSec = _channels * _samplingRate;
	if (_format > 8)
		bytesPerSec *= 2;

	lastUpdate = 0;

    return true;
} 

double AudioIOMAS::currentTime()
{
	timeval tv;
	gettimeofday(&tv,0);

	return (double)tv.tv_sec + (double)tv.tv_usec/1000000.0;
}

bool AudioIOMAS::close_with_error(const string& text)
{
	string& error = paramStr(lastError);
    error = text;
	error += masc_strmerror (mas_error);
	return false;
}

void AudioIOMAS::close()
{
	list<mas_port_t>::iterator pi;
	for (pi = allocated_ports.begin(); pi != allocated_ports.end(); pi++)
		mas_free_port (*pi);
	allocated_ports.clear();

	list<mas_channel_t>::iterator ci;
	for (ci = allocated_channels.begin(); ci != allocated_channels.end(); ci++)
		mas_free_channel (*ci);
	allocated_channels.clear();

	list<mas_device_t>::iterator di;
	for (di = allocated_devices.begin(); di != allocated_devices.end(); di++)
	{
		mas_device_t device = *di;
		mas_error = mas_asm_terminate_device_instance(device, 0);
		if (mas_error < 0)
			arts_warning ("MAS output: error while closing device: %s", masc_strmerror(mas_error));

		mas_free_device(device);
	}
	allocated_devices.clear();

    Dispatcher::the()->ioManager()->removeTimer(this);
}

void AudioIOMAS::updateBufferSizes()
{
	double time = currentTime();
	double waterMark = param(fragmentSize);
	waterMark *= 1.3;

	if ((time - lastUpdate) * bytesPerSec < waterMark)
		return;
	
	lastUpdate = time;

	uint32	inbuf_ms;
	int32	mas_error;

	mas_error = mas_get( sbuf, "inbuf_ms", 0 , &package );
	if ( mas_error < 0 )
		arts_fatal ("MAS output: error getting size of buffer: %s", masc_strmerror(mas_error));

	masc_pull_uint32( &package, &inbuf_ms );
	masc_strike_package( &package );

	//arts_debug("  inbuf_ms = %u", inbuf_ms);

	float bytes = inbuf_ms;
	bytes /= 1000.0;
	bytes *= param(samplingRate);
	bytes *= param(channels);
	if(param(format) > 8)
		bytes *= 2;

	int bytesFree =  param(fragmentSize) * param(fragmentCount) - (int)bytes;

	if (bytesFree < param(fragmentSize))
		bytesFree = 0;

	writeBufferAvailable = bytesFree;

	arts_debug ("MAS output buffer: %6d / %6d bytes used => %6d bytes free",
		(int)bytes, param(fragmentSize) * param(fragmentCount), writeBufferAvailable);
}

// This is called on each timer tick
void AudioIOMAS::notifyTime()
{
	updateBufferSizes();

    int& _direction = param(direction);
    int& _fragmentSize = param(fragmentSize);

    for (;;) {
		int todo = 0;

		if ((_direction & directionRead) && (getParam(canRead) >= _fragmentSize))
		    todo |= AudioSubSystem::ioRead;

		if ((_direction & directionWrite) && (getParam(canWrite) >= _fragmentSize))
		    todo |= AudioSubSystem::ioWrite;

		if (!todo)
		    return;

		AudioSubSystem::the()->handleIO(todo);
    }
}

void AudioIOMAS::setParam(AudioParam p, int& value)
{
    switch(p) {
#if 0
    case fragmentSize:
		param(p) = requestedFragmentSize = value;
		break;
    case fragmentCount:
		param(p) = requestedFragmentCount = value;
		break;
#endif
    default:
		param(p) = value;
		break;
    }
}

int AudioIOMAS::getParam(AudioParam p)
{
    int bytes;
    int count;

    switch(p)
    {
#if 0
    case canRead:
		if (ioctl(audio_fd, AUDIO_GETINFO, &auinfo) < 0)
		    return (0);
		bytes = (auinfo.record.samples * bytesPerSample) - bytesRead;
		if (bytes < 0) {
		    printf("Error: bytes %d < 0, samples=%u, bytesRead=%u\n",
				   bytes, auinfo.record.samples, bytesRead);
		    bytes = 0;
		}
		return bytes;

    case canWrite:
		if (ioctl(audio_fd, AUDIO_GETINFO, &auinfo) < 0) 
		    return (0);
		count = SUN_MAX_BUFFER_SIZE - 
		    (bytesWritten - (auinfo.play.samples * bytesPerSample));
		return count;
#endif
    case canWrite:
			return writeBufferAvailable;

	case autoDetect:
		/*
		 * Fairly small priority, for we haven't tested this a lot
		 */
		return 3;

    default:
		return param(p);
    }
}

int AudioIOMAS::read(void *buffer, int size)
{
#if 0
    size = ::read(audio_fd, buffer, size);
    if (size < 0)
		return 0;

    bytesRead += size;
    return size;
#endif
	return 0;
}

int AudioIOMAS::write(void *buffer, int size)
{
	static int ts = 0;
	static int seq = 0;
	data->header.sequence = seq++;
	data->header.media_timestamp = ts;
	ts += size / 4;

	assert(size == data->length);
	memcpy(data->segment, buffer, size);

	int32 mas_error = mas_send( audio_channel , data );
	if (mas_error < 0)
		arts_fatal ("MAS output: problem during mas_send: %s", masc_strmerror(mas_error));

	writeBufferAvailable -= size;
    return size;
}

#endif /* HAVE_LIBMAS */
