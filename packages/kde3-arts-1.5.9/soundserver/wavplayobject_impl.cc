    /*

    Copyright (C) 2000 Stefan Westerfeld
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

#include "config.h"

#ifdef HAVE_LIBAUDIOFILE

#include "soundserver.h"
#include "cachedwav.h"
#include "stdsynthmodule.h"
#include "convert.h"
#include "debug.h"
#include <math.h>

using namespace std;
using namespace Arts;

class WavPlayObject_impl :public WavPlayObject_skel, public StdSynthModule {
protected:
	CachedWav *wav;
	double flpos;
	poState _state;

	int sampleCount()
	{
		if(!wav) return 0;
		return wav->bufferSize / wav->channelCount / (wav->sampleWidth/8);
	}
public:
	/*
	 * construction, destruction
	 */
	WavPlayObject_impl() :wav(0), flpos(0.0), _state(posIdle)
	{
		arts_debug("WavPlayObject_impl");
	}

	virtual ~WavPlayObject_impl()
	{
		arts_debug("~WavPlayObject_impl");
		if(wav) wav->decRef();
	}

	/*
	 * KMedia2 private interface
	 */

	bool loadMedia(const string &filename) {
		arts_debug("Wav: loadMedia %s", filename.c_str());
		wav = CachedWav::load(Cache::the(), filename);
		return (wav != 0);
	}

	/*
	 * KMedia2 interface
	 */
	string description() {
		return "no description (see "__FILE__")";
	}

	poTime currentTime() {
		if(!wav) return poTime(0,0,0,"samples");

		float timesec = flpos / (float)wav->samplingRate;
		float timems = (timesec - floor(timesec)) * 1000.0;

		return poTime( int( timesec ), int( timems ), (int)flpos, "samples" );
	}

	poTime overallTime() {
		float timesec = (float)sampleCount() / (float)wav->samplingRate;
		float timems = (timesec - floor(timesec)) * 1000.0;

		return poTime( int( timesec ), int( timems ), sampleCount(), "samples" );
	}

	poCapabilities capabilities() {
  		return static_cast<poCapabilities>(capPause+capSeek);
	}

	string mediaName() {
		return wav ? wav->mediaName() : "";
	}

	poState state() {
		return _state;
	}

	void play() {
		_state = posPlaying;
	}

	void halt() {
		_state = posIdle;
		flpos = 0.0;
	}

	void seek(const class poTime &newTime) {
		if(!wav) return;

		float fnewsamples = -1;
		if(newTime.seconds != -1 && newTime.ms != -1)
		{
			float flnewtime = (float)newTime.seconds+((float)newTime.ms/1000.0);
			fnewsamples = flnewtime * (float)wav->samplingRate;
		}
		else if(newTime.custom >= 0 && newTime.customUnit == "samples")
		{
			fnewsamples = newTime.custom;
		}

		if(fnewsamples > (float)sampleCount())
			fnewsamples = (float)sampleCount();

		if(fnewsamples >= 0) flpos = fnewsamples;
	}

	void pause() {
		arts_debug("pause");
		_state = posPaused;
	}

	void calculateBlock(unsigned long samples) {
		unsigned long haveSamples = 0;

		if(wav && _state == posPlaying)
		{
			double speed = wav->samplingRate / samplingRateFloat;

			haveSamples = uni_convert_stereo_2float(samples, wav->buffer,
		   		wav->bufferSize,wav->channelCount,wav->sampleWidth,
		   		left,right,speed,flpos);

			flpos += (double)haveSamples * speed;
		}

		if(haveSamples != samples)
		{
			unsigned long i;

			for(i=haveSamples;i<samples;i++)
				left[i] = right[i] = 0.0;

			if(_state == posPlaying) {
				_state = posIdle;
				flpos = 0.0;
			}
		}
	}
};

REGISTER_IMPLEMENTATION(WavPlayObject_impl);
#endif
