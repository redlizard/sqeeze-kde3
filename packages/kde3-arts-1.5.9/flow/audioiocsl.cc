    /*

    Copyright (C) 2000-2002 Stefan Westerfeld
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

/**
 * only compile 'csl' AudioIO class if libcsl is present
 */
#ifdef HAVE_LIBCSL
#include <csl/csl.h>

/* g_newa */
#include <gsl/gsldefs.h>

#include <vector>

#include "audioio.h"
#include "audiosubsys.h"
#include "debug.h"
#include "dispatcher.h"
#include "iomanager.h"

namespace Arts {

class AudioIOCSL : public AudioIO,
				   public IONotify
{
protected:
	CslPcmStream *inputStream, *outputStream;
	CslDriver *cslDriver;
	int requestedFragmentSize;
	int requestedFragmentCount;
	const char *cslDriverName;

	std::vector<CslPollFD> cslFds, cslOldFds;
	int csl2iomanager(int cslTypes);
	void updateFds();

	void notifyIO(int fd, int types);
	static void handleRead(void *user_data, CslPcmStream *stream);
	static void handleWrite(void *user_data, CslPcmStream *stream);

public:
	AudioIOCSL(const char *driverName = 0);

	void setParam(AudioParam param, int& value);
	int getParam(AudioParam param);

	bool open();
	void close();
	int read(void *buffer, int size);
	int write(void *buffer, int size);
};

REGISTER_AUDIO_IO(AudioIOCSL,"csl","Common Sound Layer");

class AudioIOCSLFactory : public AudioIOFactory {
protected:
	const char *driverName;
	std::string _name;
	std::string _fullName;

public:
	AudioIOCSLFactory(const char *driverName)
		: driverName(driverName)
	{
		_name = "csl-";
		_name += driverName;

		_fullName = "Common Sound Layer (";
		_fullName += driverName;
		_fullName += ")";
	}
	virtual ~AudioIOCSLFactory()
	{
	}
	AudioIO *createAudioIO() { return new AudioIOCSL(driverName); }
	virtual const char *name() { return _name.c_str(); }
	virtual const char *fullName() { return _fullName.c_str(); }
};

static class AudioIOCSLInit {
protected:
	std::list<AudioIOCSLFactory *> factories;

public:
    AudioIOCSLInit()
    {
		unsigned int i,n;
		const char **drivers = csl_list_drivers(&n);

		for(i = 0; i < n; i++)
			factories.push_back(new AudioIOCSLFactory(drivers[i]));
    }
	~AudioIOCSLInit()
	{
		std::list<AudioIOCSLFactory *>::iterator i;
		for(i = factories.begin(); i != factories.end(); i++)
			delete (*i);

		factories.clear();
	}
} aci;

};

using namespace std;
using namespace Arts;

AudioIOCSL::AudioIOCSL(const char *driverName)
	: cslDriverName(driverName)
{
	/*
	 * default parameters
	 */
	param(samplingRate) = 44100;
	paramStr(deviceName) = "todo";
	requestedFragmentSize = param(fragmentSize) = 1024;
	requestedFragmentCount = param(fragmentCount) = 7;
	param(channels) = 2;
	param(direction) = 2;

#ifdef WORDS_BIGENDIAN
	param(format) = 17;
#else
	param(format) = 16;
#endif
}

bool AudioIOCSL::open()
{
	string& errorMsg = paramStr(lastError);
	string& _deviceName = paramStr(deviceName);
	int& _channels = param(channels);
	int& _fragmentSize = param(fragmentSize);
	int& _fragmentCount = param(fragmentCount);
	int& _samplingRate = param(samplingRate);
	int& _format = param(format);
	int fmt = 0;
	char *env = 0;

	if(cslDriverName && strcmp(cslDriverName, "arts") == 0)
		env = arts_strdup_printf("ARTS_SERVER=%s",_deviceName.c_str());

	if(env)
	{
		putenv(env);
		arts_debug("AudioIOCsl: set %s\n",env);
	}

	CslErrorType error;
	error = csl_driver_init(cslDriverName, &cslDriver);    /* choose backend */

	if(env)
	{
		putenv("ARTS_SERVER");
		free(env);
	}

    if (error)
	{
    	errorMsg = "unable to initialize CSL driver: ";
		errorMsg += csl_strerror(error);
		return false;
	}

	if(_format == 8)
		fmt = CSL_PCM_FORMAT_U8;
	else if(_format == 16)
		fmt = CSL_PCM_FORMAT_S16_LE;
	else if(_format == 17)
		fmt = CSL_PCM_FORMAT_S16_BE;

	inputStream = outputStream = 0;
	if(param(direction) & directionRead)
	{
   	    /* open PCM output stream */
        error = csl_pcm_open_output(cslDriver,
                                    "artsd output",
                                    _samplingRate,
									_channels,
									fmt, &inputStream);
		if (error)
		{
			errorMsg = "failed to open CSL input stream: ";
			errorMsg += csl_strerror(error);
			return false;
		}
		csl_pcm_set_callback(inputStream, handleRead, 0, 0);
	}
	if(param(direction) & directionWrite)
	{
	    /* open PCM output stream */
        error = csl_pcm_open_output(cslDriver,
                                    "artsd output",
                                    _samplingRate,
									_channels,
									fmt, &outputStream);
		if (error)
		{
			close();

			errorMsg = "failed to open CSL output stream: ";
			errorMsg += csl_strerror(error);
			return false;
		}
		csl_pcm_set_callback(outputStream, handleWrite, 0, 0);
	}
#if 0
	if (_format && (ossBits(gotFormat) != ossBits(requestedFormat)))
	{  
		char details[80];
		sprintf(details," (_format = %d, asked driver to give %d, got %d)",
			_format, requestedFormat, gotFormat);

		_error = "Can't set playback format";
		_error += details;

		close();
		return false;
	}

	if(gotFormat == AFMT_U8)
		_format = 8;
	else if(gotFormat == AFMT_S16_LE)
		_format = 16;
	else if(gotFormat == AFMT_S16_BE)
		_format = 17;
	else
	{
		char details[80];
		sprintf(details," (_format = %d, asked driver to give %d, got %d)",
			_format, requestedFormat, gotFormat);

		_error = "unknown format given by driver";
		_error += details;

		close();
		return false;
	}


	int stereo=-1;     /* 0=mono, 1=stereo */

	if(_channels == 1)
	{
		stereo = 0;
	}
	if(_channels == 2)
	{
		stereo = 1;
	}

	if(stereo == -1)
	{
		_error = "internal error; set channels to 1 (mono) or 2 (stereo)";

		close();
		return false;
	}

	int requeststereo = stereo;

	if (ioctl(audio_fd, SNDCTL_DSP_STEREO, &stereo)==-1)
	{
		_error = "SNDCTL_DSP_STEREO failed - ";
		_error += strerror(errno);

		close();
		return false;
	}

	if (requeststereo != stereo)
	{
		_error = "audio device doesn't support number of requested channels";

		close();
		return false;
	}

	int speed = _samplingRate;

	if (ioctl(audio_fd, SNDCTL_DSP_SPEED, &speed)==-1)  
	{
		_error = "SNDCTL_DSP_SPEED failed - ";
		_error += strerror(errno);

		close();
		return false;
	}  

    /*
	 * Some soundcards seem to be able to only supply "nearly" the requested
	 * sampling rate, especially PAS 16 cards seem to quite radical supplying
	 * something different than the requested sampling rate ;)
	 *
	 * So we have a quite large tolerance here (when requesting 44100 Hz, it
	 * will accept anything between 38690 Hz and 49510 Hz). Most parts of the
	 * aRts code will do resampling where appropriate, so it shouldn't affect
	 * sound quality.
	 */
    int tolerance = _samplingRate/10+1000;

	if (abs(speed-_samplingRate) > tolerance)
	{  
		_error = "can't set requested samplingrate";

		char details[80];
		sprintf(details," (requested rate %d, got rate %d)",
			_samplingRate, speed);
		_error += details;

		close();
		return false;
	} 
	_samplingRate = speed;

	/*
	 * set the fragment settings to what the user requested
	 */
	
	_fragmentSize = requestedFragmentSize;
	_fragmentCount = requestedFragmentCount;

	/*
	 * lower 16 bits are the fragment size (as 2^S)
	 * higher 16 bits are the number of fragments
	 */
	int frag_arg = 0;

	int size = _fragmentSize;
	while(size > 1) { size /= 2; frag_arg++; }
	frag_arg += (_fragmentCount << 16);
	if(ioctl(audio_fd, SNDCTL_DSP_SETFRAGMENT, &frag_arg) == -1)
	{
		char buffer[1024];
		_error = "can't set requested fragments settings";
		sprintf(buffer,"size%d:count%d\n",_fragmentSize,_fragmentCount);
		close();
		return false;
	}

	/*
	 * now see what we really got as cards aren't required to supply what
	 * we asked for
	 */
	audio_buf_info info;
	if(ioctl(audio_fd,SNDCTL_DSP_GETOSPACE, &info) == -1)
	{
		_error = "can't retrieve fragment settings";
		close();
		return false;
	}

	// update fragment settings with what we got
	_fragmentSize = info.fragsize;
	_fragmentCount = info.fragstotal;

	artsdebug("buffering: %d fragments with %d bytes "
		"(audio latency is %1.1f ms)", _fragmentCount, _fragmentSize,
		(float)(_fragmentSize*_fragmentCount) /
		(float)(2.0 * _samplingRate * _channels)*1000.0);

	/*
	 * Workaround for broken kernel drivers: usually filling up the audio
	 * buffer is _only_ required if _fullDuplex is true. However, there
	 * are kernel drivers around (especially everything related to ES1370/1371)
	 * which will not trigger select()ing the file descriptor unless we have
	 * written something first.
	 */
	char *zbuffer = (char *)calloc(sizeof(char), _fragmentSize);
	if(_format == 8)
		for(int zpos = 0; zpos < _fragmentSize; zpos++)
			zbuffer[zpos] |= 0x80;
	
	for(int fill = 0; fill < _fragmentCount; fill++)
	{
		int len = ::write(audio_fd,zbuffer,_fragmentSize);
		if(len != _fragmentSize)
		{
			arts_debug("AudioIOCSL: failed prefilling audio buffer (might cause synchronization problems in conjunction with full duplex)");
			fill = _fragmentCount+1;
		}
	}
	free(zbuffer);

	/*
	 * Triggering - the original aRts code did this for full duplex:
	 *
	 *  - stop audio i/o using SETTRIGGER(~(PCM_ENABLE_INPUT|PCM_ENABLE_OUTPUT))
	 *  - fill buffer (see zbuffer code two lines above)
	 *  - start audio i/o using SETTRIGGER(PCM_ENABLE_INPUT|PCM_ENABLE_OUTPUT)
	 *
	 * this should guarantee synchronous start of input/output. Today, it
	 * seems there are too many broken drivers around for this.
	 */

	if(device_caps & DSP_CAP_TRIGGER)
	{
		int enable_bits = 0;

		if(param(direction) & 1) enable_bits |= PCM_ENABLE_INPUT;
		if(param(direction) & 2) enable_bits |= PCM_ENABLE_OUTPUT;

		if(ioctl(audio_fd,SNDCTL_DSP_SETTRIGGER, &enable_bits) == -1)
		{
			_error = "can't start sound i/o";

			close();
			return false;
		}
	}
#endif
	updateFds();
	return true;
}

void AudioIOCSL::close()
{
	if(inputStream)
	{
		csl_pcm_close(inputStream);
		inputStream = 0;
	}
	if(outputStream)
	{
		csl_pcm_close(outputStream);
		outputStream = 0;
	}
	updateFds();
}

void AudioIOCSL::setParam(AudioParam p, int& value)
{
	switch(p)
	{
		case fragmentSize:
				param(p) = requestedFragmentSize = value;
			break;
		case fragmentCount:
				param(p) = requestedFragmentCount = value;
			break;
		default:
				param(p) = value;
			break;
	}
}

int AudioIOCSL::getParam(AudioParam p)
{
    CslErrorType error;
	CslPcmStatus status;

	switch(p)
	{
		case canRead:
				error = csl_pcm_get_status (inputStream, &status);
				if (error) /* FIXME */
					arts_fatal("unable to obtain csl stream status: %s",
							   csl_strerror (error));

				updateFds();
				return status.n_bytes_available;
			break;

		case canWrite:
				error = csl_pcm_get_status(outputStream, &status);
				if (error) /* FIXME */
					arts_fatal("unable to obtain csl stream status: %s",
							   csl_strerror (error));

				updateFds();
				return status.n_bytes_available;
			break;

		case autoDetect:
				/* CSL is pretty experimental currently */
				return 1;	
			break;

		default:
				return param(p);
			break;
	}
}

int AudioIOCSL::read(void *buffer, int size)
{
	arts_assert(inputStream != 0);

	int result = csl_pcm_read(inputStream, size, buffer);
	updateFds();

	return result;
}

void AudioIOCSL::handleRead(void *, CslPcmStream *)
{
	AudioSubSystem::the()->handleIO(AudioSubSystem::ioRead);
}

int AudioIOCSL::write(void *buffer, int size)
{
	arts_assert(outputStream != 0);

	int result = csl_pcm_write(outputStream, size, buffer);
	updateFds();

	return result;
}

void AudioIOCSL::handleWrite(void *, CslPcmStream *)
{
	AudioSubSystem::the()->handleIO(AudioSubSystem::ioWrite);
}

/* mainloop integration: make CSL callbacks work inside the aRts mainloop */

int AudioIOCSL::csl2iomanager(int cslTypes)
{
	/* FIXME: doublecheck this list */
	int types = 0;

	if(cslTypes & CSL_POLLIN)
		types |= IOType::read;
	if(cslTypes & CSL_POLLOUT)
		types |= IOType::write;
	if(cslTypes & CSL_POLLERR)
		types |= IOType::except;

	return types;
}

void AudioIOCSL::updateFds()
{
	unsigned int n_fds = csl_poll_count_fds(cslDriver);
	CslPollFD *newFds = g_newa(CslPollFD, n_fds);

	unsigned int have_fds = csl_poll_get_fds(cslDriver, n_fds, newFds);
	arts_assert(have_fds == n_fds);

	cslFds.clear();

	unsigned int i;
	for(i = 0; i < have_fds; i++)
		cslFds.push_back(newFds[i]);

	/* FIXME: if csl provided a flag for this, we could save some work here */
	bool fdsChanged;
	if(cslFds.size() == cslOldFds.size())
	{
		fdsChanged = false;
		for(i = 0; i < have_fds; i++)
		{
			if(cslFds[i].events != cslOldFds[i].events)
				fdsChanged = true;
			if(cslFds[i].fd != cslOldFds[i].fd)
				fdsChanged = true;
		}
	}
	else
	{
		fdsChanged = true;
	}
	if(!fdsChanged)
		return;

	vector<CslPollFD>::iterator ci;

	/* remove old watches */
	/*
	 * UGLY! due to broken API, we can only remove all watches here, and not
	 * do anything selectively - its not a problem for the code here, but it
	 * might be a problem elsewhere. Unfortunately, it can't be fixed without
	 * breaking BC.
	 */
	Dispatcher::the()->ioManager()->remove(this, IOType::all);
	arts_debug("AudioIOCSL::updateFds(): removing watches");

	/* add new watches */
	for(ci = cslFds.begin(); ci < cslFds.end(); ci++)
	{
		int types = csl2iomanager(ci->events);
		if(types)
		{
			Dispatcher::the()->ioManager()->watchFD(ci->fd, types, this);
			arts_debug("AudioIOCSL::updateFds(): adding watch on %d", ci->fd);
		}
	}

	cslOldFds = cslFds;
}

void AudioIOCSL::notifyIO(int fd, int type)
{
	vector<CslPollFD>::iterator fi;

	for(fi = cslFds.begin(); fi != cslFds.end(); fi++)
	{
		if(fi->fd == fd)
		{
			int ftype = csl2iomanager(fi->events);
			fi->revents = 0;

			if(type & ftype & IOType::read)
				fi->revents |= CSL_POLLIN;
			if(type & ftype & IOType::write)
				fi->revents |= CSL_POLLOUT;
			if(type & ftype & IOType::except)
				fi->revents |= CSL_POLLERR;

			if(fi->revents)
				csl_poll_handle_fds(cslDriver, 1, &(*fi));
		}
	}
	updateFds();
}

#endif
