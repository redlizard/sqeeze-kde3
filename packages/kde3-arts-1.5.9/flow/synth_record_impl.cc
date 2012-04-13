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

#include "artsflow.h"
#include "debug.h"
#include "convert.h"
#include "objectmanager.h"
#include "audiosubsys.h"
#include "dispatcher.h"
#include "iomanager.h"
#include "flowsystem.h"
#include "stdsynthmodule.h"
#include <stdio.h>
#include <iostream>
#include <cstring>

#undef DEBUG_WAVEFORM
#ifdef DEBUG_WAVEFORM
#include <fstream>
#endif

using namespace std;
using namespace Arts;

namespace Arts {

class Synth_RECORD_impl :	virtual public Synth_RECORD_skel,
						virtual public ASConsumer,
						virtual public StdSynthModule
{
#ifdef DEBUG_WAVEFORM
	ofstream plotfile;
#endif
protected:
	AudioSubSystem *as;
	bool haveSubSys;

	typedef unsigned char uchar;

	unsigned char *inblock;
	unsigned long maxsamples;
	unsigned long channels;
	int format;
	int bits;

public:
#ifdef DEBUG_WAVEFORM
	Synth_RECORD_impl()
		: plotfile( "/dev/shm/synth_record.plot" )
	{
	}
#endif
	/*
	 * functions from the SynthModule interface (which is inherited by
	 * SynthPlay)
	 */
	void streamInit() {
		as = AudioSubSystem::the();

		channels = as->channels();
		format = as->format();
		bits = as->bits();
		maxsamples = 0;
		inblock = 0;

		haveSubSys = as->attachConsumer(this);
		if(!haveSubSys)
		{
			arts_info("Synth_RECORD: audio subsystem is already used");
			return;
		}
	}

	void streamEnd() {
		arts_debug("Synth_RECORD: detaching");
		if(haveSubSys) as->detachConsumer();

		if(inblock)
		{
			delete[] inblock;
			inblock = 0;
		}
	}

	AutoSuspendState autoSuspend()
	{
		return static_cast<AutoSuspendState>(asSuspend|asProducer);
	}

	void calculateBlock(unsigned long samples)
	{
		// no audio subsystem, no play
		if(!as->running() || !haveSubSys) return;

		if(samples > maxsamples)
		{
			maxsamples = samples;

			if(inblock) delete[] inblock;
			inblock = new uchar[maxsamples * channels * bits / 8];
		}

		assert(channels);

		as->read(inblock,channels * (bits/8) * samples);

		arts_assert(format == 8 || format == 16 || format == 17 || format == 32);
		if(format == 8)
		{
			if(channels == 1)
				convert_mono_8_float(samples,inblock,left);

			if(channels == 2)
				convert_stereo_i8_2float(samples,inblock,left,right);
		}
		else if(format == 16)
		{
			if(channels == 1)
				convert_mono_16le_float(samples,inblock,left);

			if(channels == 2)
				convert_stereo_i16le_2float(samples,inblock,left,right);
		}
		else if(format == 17)
		{
			if(channels == 1)
				convert_mono_16be_float(samples,inblock,left);

			if(channels == 2)
				convert_stereo_i16be_2float(samples,inblock,left,right);
		}
		else if(format == 32)
		{
			if(channels == 2)
			{
				float * end = (float *)(inblock + 8 * samples);
				float * floatbuffer = (float *)inblock;
				while(floatbuffer < end)
				{
#ifdef DEBUG_WAVEFORM
					plotfile << *floatbuffer << "\n";
#endif
					*left++ = *floatbuffer++;
					*right++ = *floatbuffer++;
				}
			}
			else if(channels == 1)
				memcpy(left, inblock, samples);
		}
	}

	/**
	 * havemore from the ASConsumer interface (AudioSubSystem)
	 */
	void haveMore()
	{
		_node()->requireFlow();
	}
};

REGISTER_IMPLEMENTATION(Synth_RECORD_impl);

}

// vim: sw=4 ts=4
