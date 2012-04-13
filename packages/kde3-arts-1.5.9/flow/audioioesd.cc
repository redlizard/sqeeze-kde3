    /*

    Copyright (C) 2001 Jochen Hoenicke
                       jochen@gnu.org

    Copyright (C) 2003 Ian Chiew
                       ian@snork.net

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
 * only compile esound AudioIO class if libesd was detected during
 * configure
 */
#ifdef HAVE_LIBESD

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

#include "audioio.h"
#include "audiosubsys.h"
#include "iomanager.h"
#include "dispatcher.h"

#include <esd.h>

#ifdef __BIG_ENDIAN__
#define _format_bits 17
#else
#define _format_bits 16
#endif

namespace Arts {

class AudioIOESD : public AudioIO, public IONotify {
protected:
	int server_fd, read_fd, write_fd;

public:
	AudioIOESD();

	void setParam(AudioParam param, int& value);
	int getParam(AudioParam param);
	void notifyIO(int fd, int type);

	bool open();
	void close();
	void run();
	int read(void *buffer, int size);
	int write(void *buffer, int size);
};

REGISTER_AUDIO_IO(AudioIOESD,"esd","Enlightened Sound Daemon");
}

using namespace std;
using namespace Arts;

AudioIOESD::AudioIOESD()
{
	/*
	 * default parameters
	 */
	param(samplingRate) = 44100;
	paramStr(deviceName) = "null";
	param(fragmentSize) = 1024;
	param(fragmentCount) = 7;
	param(format) = _format_bits;
	param(channels) = 2;
	param(direction) = 2;

	server_fd = -1;
	read_fd = -1;
	write_fd = -1;
}

bool AudioIOESD::open()
{
	int& _channels = param(channels);
	int& _direction = param(direction);
	int& _samplingRate = param(samplingRate);
	int& _format = param(format);
	string& _error = paramStr(lastError);

	if ((server_fd = esd_open_sound(NULL)) == -1)
	{
		_error = "Couldn't connect to server";
		return false;
	}

	esd_server_info_t *server_info = esd_get_server_info(server_fd);

	if (server_info == NULL)
	{
		_error = "Unable to query EsounD server";
		return false;
	}
	
	esd_format_t server_format = server_info->format;

	_samplingRate = server_info->rate;
	_channels     = (server_format & ESD_STEREO) ? 2            : 1;
	_format       = (server_format & ESD_BITS16) ? _format_bits : 8;

	esd_free_server_info(server_info);
	
	if (_direction & directionRead)
	{
		read_fd = esd_record_stream(server_format, _samplingRate,
					    NULL, "aRts");
		if (read_fd == -1)
		{
			_error = "Couldn't create read uflow";
			return false;
		}
	}

	if (_direction & directionWrite)
	{
		write_fd = esd_play_stream(server_format, _samplingRate,
					   NULL, "aRts");
		if (write_fd == -1)
		{
			_error = "Couldn't create write flow";
			return false;
		}
	}

	return true;
}

void AudioIOESD::notifyIO(int, int)
{
}

void AudioIOESD::close()
{
	if (write_fd != -1) 
		esd_close(write_fd);

	if (read_fd != -1)
		esd_close(read_fd);

	if (server_fd != -1)
		esd_close(server_fd);
}

void AudioIOESD::setParam(AudioParam p, int& value)
{
	switch(p)
	{
	case channels:
	case format:
	case direction:
	case samplingRate:
		param(p) = value;
		close();
		open();
		return;

	default:
		param(p) = value;
		return;
	}
}

int AudioIOESD::getParam(AudioParam p)
{
	switch(p)
	{
	case selectReadFD:
		return read_fd;

	case selectWriteFD:
		return write_fd;

	case canRead:
		return ESD_BUF_SIZE;

	case canWrite:
		return ESD_BUF_SIZE;

		// ESD handles are actually socket descriptors, and I know not
		// of any portable way to peek at the socket's send or receive
		// buffers.

	default:
		return param(p);
	}
}

int AudioIOESD::read(void *buffer, int size)
{
	return ::read(read_fd, buffer, size);
}

int AudioIOESD::write(void *buffer, int size)
{
	return ::write(write_fd, buffer, size);
}

#endif
