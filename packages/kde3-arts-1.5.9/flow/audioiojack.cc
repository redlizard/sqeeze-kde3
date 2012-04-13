/*
    Copyright (C) 2004 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

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

#ifdef HAVE_LIBJACK

#include <jack/jack.h>
#include <jack/ringbuffer.h>

#include "debug.h"
#include "audioio.h"
#include "audiosubsys.h"
#include "iomanager.h"
#include "dispatcher.h"

#ifndef MIN
#define MIN(a,b)        ((a) < (b) ? (a) : (b))
#endif

#undef DEBUG_WAVEFORM
#ifdef DEBUG_WAVEFORM
#include <fstream>
#endif

#include <cstdlib>
#include <cstring>

namespace Arts {

class AudioIOJack : public AudioIO, public TimeNotify {
private:
#ifdef DEBUG_WAVEFORM
	std::ofstream plotfile;
#endif
	char * processBuffer;
	size_t buffersize;

protected:
	jack_client_t *jack;
	jack_port_t *outleft, *outright;
	jack_port_t *inleft, *inright;
	jack_ringbuffer_t *olb, *orb, *ilb, *irb;

public:
	AudioIOJack();

	void notifyTime();

	void setParam( AudioParam p, int & val );
	int getParam(AudioParam param);

	static int jackCallback( jack_nframes_t, void * );

	bool open();
	void close();
	int read(void *buffer, int size);
	int write(void *buffer, int size);
};

REGISTER_AUDIO_IO(AudioIOJack,"jack","Jack Audio Connection Kit");
}

using namespace std;
using namespace Arts;

AudioIOJack::AudioIOJack()
	:
#ifdef DEBUG_WAVEFORM
	plotfile( "/dev/shm/audioiojack.plot" ),
#endif
	jack( 0 )
	, outleft( 0 )
	, outright( 0 )
	, inleft( 0 )
	, inright( 0 )
{
	/*
	 * default parameters
	 */
	param( samplingRate ) = 44100;
	paramStr( deviceName ) = "jack";
	param( fragmentSize ) = 512;
	param( fragmentCount ) = 2;
	param( channels ) = 2;
	param( direction ) = 2;
	param( format ) = 32;
}

bool AudioIOJack::open()
{
	string& _error = paramStr( lastError );
	jack = jack_client_new( "artsd" );
	if( jack == 0 )
	{
		_error = "Couldn't connect to jackd";
		return false;
	}

	int& _sampleRate = param(samplingRate);
	_sampleRate = jack_get_sample_rate( jack );
	int& _fragmentSize = param(fragmentSize);
	int& _fragmentCount = param(fragmentCount);

	/*
	 * don't allow unreasonable large fragmentSize/Count combinations,
	 * because "real" hardware also doesn't
	 */

	if(_fragmentSize > 1024*8) _fragmentSize = 1024*8;

	while(_fragmentSize * _fragmentCount > 1024*128)
		_fragmentCount--;

	jack_set_process_callback( jack, Arts::AudioIOJack::jackCallback, this );

	if( param( direction ) & directionWrite )
	{
		outleft = jack_port_register( jack, "out_1",
				JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0 );
		outright = jack_port_register( jack, "out_2",
				JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0 );
		olb = jack_ringbuffer_create(
				sizeof( jack_default_audio_sample_t ) *
				_fragmentSize * _fragmentCount );
		orb = jack_ringbuffer_create(
				sizeof( jack_default_audio_sample_t ) *
				_fragmentSize * _fragmentCount );
	}
	if( param( direction ) & directionRead )
	{
		inleft = jack_port_register( jack, "in_1",
				JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0 );
		inright = jack_port_register( jack, "in_2",
				JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0 );
		ilb = jack_ringbuffer_create(
				sizeof( jack_default_audio_sample_t ) *
				1024 * 64 );
		irb = jack_ringbuffer_create(
				sizeof( jack_default_audio_sample_t ) *
				1024 * 64 );
	}

	if( jack_activate( jack ) )
	{
		_error = "Activating as jack client failed.";
		return false;
	}

	const char **ports;
	if( param( direction ) & directionRead )
	{
		ports = jack_get_ports( jack, 0, 0, JackPortIsPhysical
				| JackPortIsOutput );
		if( ports == 0 )
		{
			arts_warning( "Cannot find any capture ports to"
					" connect to. You need to manually connect"
					" the capture ports in jack" );
		}
		else
		{
			if( ports[ 0 ] != 0 )
				jack_connect( jack, ports[ 0 ],
						jack_port_name( inleft ) );
			if( ports[ 1 ] != 0 )
				jack_connect( jack, ports[ 1 ],
						jack_port_name( inright ) );
			free( ports );
		}
	}
	if( param( direction ) & directionWrite )
	{
		ports = jack_get_ports( jack, 0, 0, JackPortIsPhysical
				| JackPortIsInput );
		if( ports == 0 )
		{
			arts_warning( "Cannot find any playback ports to"
					" connect to. You need to manually connect"
					" the playback ports in jack" );
		}
		else
		{
			if( ports[ 0 ] != 0 )
				jack_connect( jack, jack_port_name( outleft ),
						ports[ 0 ] );
			if( ports[ 1 ] != 0 )
				jack_connect( jack, jack_port_name( outright ),
						ports[ 1 ] );
			free( ports );
		}
	}

	// Install the timer
	Dispatcher::the()->ioManager()->addTimer(10, this);

	return true;
}

void AudioIOJack::close()
{
	jack_client_close( jack );
    Dispatcher::the()->ioManager()->removeTimer(this);
}

int AudioIOJack::jackCallback( jack_nframes_t nframes, void * args )
{
	AudioIOJack * that = static_cast<AudioIOJack*>( args );

	that->buffersize = nframes * sizeof( jack_default_audio_sample_t );
	if( that->outleft )
	{
		if( jack_ringbuffer_read_space( that->olb ) < that->buffersize )
		{
			that->processBuffer = static_cast<char *>(
					jack_port_get_buffer( that->outleft, nframes ) );
			memset( that->processBuffer, 0, that->buffersize );
			that->processBuffer = static_cast<char *>(
					jack_port_get_buffer( that->outright, nframes ) );
			memset( that->processBuffer, 0, that->buffersize );
		}
		else
		{
			that->processBuffer = static_cast<char *>(
					jack_port_get_buffer( that->outleft, nframes ) );
			jack_ringbuffer_read( that->olb, that->processBuffer, that->buffersize );
			that->processBuffer = static_cast<char *>(
					jack_port_get_buffer( that->outright, nframes ) );
			jack_ringbuffer_read( that->orb, that->processBuffer, that->buffersize );
		}
	}
	if( that->inleft )
	{
		that->processBuffer = static_cast<char *>(
				jack_port_get_buffer( that->inleft, nframes ) );
		jack_ringbuffer_write( that->ilb, that->processBuffer, that->buffersize );
		that->processBuffer = static_cast<char *>(
				jack_port_get_buffer( that->inright, nframes ) );
		jack_ringbuffer_write( that->irb, that->processBuffer, that->buffersize );
	}
	return 0;
}

void AudioIOJack::setParam( AudioParam p, int& val )
{
	// don't change the format - jack only supports 32 bit float
	if( p == format )
		return;
	AudioIO::setParam( p, val );
}

int AudioIOJack::getParam(AudioParam p)
{
	switch(p)
	{
		case canRead:
			return MIN( jack_ringbuffer_read_space( ilb ), jack_ringbuffer_read_space( irb ) ) * param( channels );
		case canWrite:
			return MIN( jack_ringbuffer_write_space( olb ), jack_ringbuffer_write_space( orb ) ) * param( channels );
		default:
			return AudioIO::getParam( p );
	}
}

int AudioIOJack::read(void *buffer, int size)
{
	float * floatbuffer = static_cast<float *>( buffer );
	if( param( channels ) == 2 )
	{
		float * end = ( float * )( static_cast<char *>( buffer ) + size );
		while( floatbuffer < end )
		{
			jack_ringbuffer_read( ilb, ( char* )( floatbuffer++ ), sizeof( float ) );
#ifdef DEBUG_WAVEFORM
			plotfile << *( floatbuffer - 1 ) << "\n";
#endif
			jack_ringbuffer_read( irb, ( char* )( floatbuffer++ ), sizeof( float ) );
		}
	}
	else if( param( channels ) == 1 )
	{
		jack_ringbuffer_read( ilb, ( char* )( floatbuffer ), size );
	}
	return size;
}

int AudioIOJack::write(void *buffer, int size)
{
	float * floatbuffer = static_cast<float *>( buffer );
	if( param( channels ) == 2 )
	{
		float * end = ( float * )( static_cast<char *>( buffer ) + size );
		while( floatbuffer < end )
		{
			jack_ringbuffer_write( olb, ( char* )( floatbuffer++ ), sizeof( float ) );
			jack_ringbuffer_write( orb, ( char* )( floatbuffer++ ), sizeof( float ) );
		}
	}
	else if( param( channels ) == 1 )
	{
		jack_ringbuffer_write( olb, ( char* )( floatbuffer ), size );
	}
	return size;
}

void AudioIOJack::notifyTime()
{
    int& _direction = param(direction);
    int& _fragmentSize = param(fragmentSize);

	for( ;; )
	{
		int todo = 0;
		if( ( _direction & directionRead ) && ( getParam( canRead ) >= _fragmentSize ) )
			todo |= AudioSubSystem::ioRead;

		if( ( _direction & directionWrite ) && ( getParam( canWrite ) >= _fragmentSize ) )
			todo |= AudioSubSystem::ioWrite;

		if( ! todo )
			return;

		AudioSubSystem::the()->handleIO( todo );
	}
}

#endif
// vim: sw=4 ts=4
