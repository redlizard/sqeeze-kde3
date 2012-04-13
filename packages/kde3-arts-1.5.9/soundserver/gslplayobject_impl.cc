    /*

    Copyright (C) 2002 Hans Meine
                       hans_meine@gmx.net

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

#include "flowsystem.h"
#include "soundserver.h"
#include "artsflow.h"
#include "stdsynthmodule.h"
#include "debug.h"
#include <math.h>

using namespace std;
using namespace Arts;

class GSLPlayObject_impl : public GSLPlayObject_skel, public StdSynthModule {
protected:
	DataHandlePlay leftPlay_, rightPlay_;
	poState state_;
	WaveDataHandle dhandle_;
	std::string filename_;

	int sampleCount()
	{
		if(!dhandle_.isLoaded()) return 0;
		return dhandle_.valueCount() / dhandle_.channelCount();
	}

public:
	/*
	 * construction, destruction
	 */
	GSLPlayObject_impl()
		: leftPlay_(DataHandlePlay::null()),
		  rightPlay_(DataHandlePlay::null()),
		  state_(posIdle)
	{
	}

	float speed() { return leftPlay_.speed(); }
	void speed(float newSpeed)
	{
		if(newSpeed != speed())
		{
			leftPlay_.speed(newSpeed);
			if(!rightPlay_.isNull())
				rightPlay_.speed(newSpeed);
			speed_changed(newSpeed);
		}
	}

	void streamStart()
	{
		arts_return_if_fail(dhandle_.isLoaded());

		if(dhandle_.channelCount()>=2)
		{
			rightPlay_ = leftPlay_.clone();
			rightPlay_.channelIndex(1);
		}

		leftPlay_.start();
		if(!rightPlay_.isNull())
			rightPlay_.start();

		arts_debug("GSLPlayObject: streamStart(%d channels) playing %d, %d",
				   dhandle_.channelCount(),
				   leftPlay_.channelIndex(),
				   rightPlay_.isNull() ? -1 : rightPlay_.channelIndex());

		_node()->virtualize("left", leftPlay_._node(), "outvalue");
		if(dhandle_.channelCount()>=2)
			_node()->virtualize("right", rightPlay_._node(), "outvalue");
		else
			_node()->virtualize("right", leftPlay_._node(), "outvalue");
	}

	void streamEnd()
	{
		arts_return_if_fail(dhandle_.isLoaded());

		_node()->devirtualize("left", leftPlay_._node(), "outvalue");
		if(dhandle_.channelCount()>=2)
			_node()->devirtualize("right", rightPlay_._node(), "outvalue");
		else
			_node()->devirtualize("right", leftPlay_._node(), "outvalue");

		leftPlay_.stop();
		if(!rightPlay_.isNull())
			rightPlay_.stop();
	}

	/*
	 * KMedia2 private interface
	 */
	bool loadMedia(const std::string &filename)
	{
		arts_return_val_if_fail(!dhandle_.isLoaded(), false);

		arts_debug("GSLPlayObject: loadMedia('%s')", filename.c_str());
                filename_ = filename;
		dhandle_.load(filename);
		if(dhandle_.isLoaded())
		{
			leftPlay_ = dhandle_.createPlayer();
			if(dhandle_.channelCount()>2)
				arts_debug("GSLPlayObject: can't play more than 2 channels, "
						   "playing first two!");

			leftPlay_._node()->connect("finished_changed", _node(), "done");
		}
		return dhandle_.isLoaded();
	}

	/*
	 * KMedia2 interface
	 */
	std::string description()
	{
		return "no description available (see "__FILE__")";
	}

	poTime currentTime()
	{
		if(!dhandle_.isLoaded())
			return poTime(0, 0, 0, "samples");

		float timesec = leftPlay_.pos()
						/ dhandle_.channelCount()
						/ dhandle_.mixerFrequency();
		float timems = (timesec - floor(timesec)) * 1000.0;

		return poTime((long)timesec, (long)timems,
					  (long)leftPlay_.pos(), "samples");
	}

    poTime overallTime()
	{
		float timesec = (float)sampleCount() / dhandle_.mixerFrequency();
		float timems = (timesec - floor(timesec)) * 1000.0;

		return poTime((long)timesec, (long)timems,
					  (long)sampleCount(), "samples");
	}

	poCapabilities capabilities()
	{
  		return static_cast<poCapabilities>(capPause | capSeek);
	}

	std::string mediaName()
	{
		return filename_;
	}

	poState state()
	{
		return state_;
	}

	bool done() { return leftPlay_.finished(); }
	void done(bool d)
	{
		if(d && (state_ == posPlaying))
			halt();
	}

	void play()
	{
		leftPlay_.paused(false);
		if(!rightPlay_.isNull())
			rightPlay_.paused(false);
		state_ = posPlaying;
	}

	void halt()
	{
		leftPlay_.paused(true);
		if(!rightPlay_.isNull())
			rightPlay_.paused(true);
		state_ = posIdle;
		seek(poTime(-1, -1, 0, "samples"));
	}

	void pause()
	{
		leftPlay_.paused(true);
		if(!rightPlay_.isNull())
			rightPlay_.paused(true);
		state_ = posPaused;
	}

	void seek(const class poTime &newTime)
	{
		long newsamples = -1;
		if(newTime.seconds != -1 && newTime.ms != -1)
		{
			float flnewtime = (float)newTime.seconds+((float)newTime.ms/1000.0);
			newsamples = (long)(flnewtime * dhandle_.mixerFrequency());
		}
		else if(newTime.custom >= 0 && newTime.customUnit == "samples")
		{
			newsamples = (long)newTime.custom;
		}

		if(newsamples > sampleCount())
			newsamples = sampleCount();

		if(newsamples >= 0)
		{
			leftPlay_.pos(newsamples * dhandle_.channelCount());
			if(!rightPlay_.isNull())
				rightPlay_.pos(newsamples * dhandle_.channelCount());
		}
	}
};

REGISTER_IMPLEMENTATION(GSLPlayObject_impl);
