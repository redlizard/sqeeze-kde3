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

#include <gsl/gsldatahandle.h>
#include <gsl/gslloader.h>
#include <gsl/gslwaveosc.h>
#include "artsflow.h"
#include "stdsynthmodule.h"
#include "debug.h"
#include <algorithm>
#include <cstring>
#include <gslpp/datahandle.h>

using namespace std;

namespace Arts {

static GSL::DataHandle getDHandle(DataHandle handle);

class DataHandle_impl : virtual public DataHandle_skel
{
protected:
	GSL::DataHandle dhandle_;
	long errno_;
	friend GSL::DataHandle getDHandle(DataHandle handle);

public:
	DataHandle_impl(GSL::DataHandle dhandle = GSL::DataHandle::null())
		: dhandle_(dhandle)
	{
		errno_ = dhandle_.isNull() ? 0 : dhandle_.open();
	}

	~DataHandle_impl()
	{
		if(dhandle_.isOpen())
			dhandle_.close();
	}

	long valueCount()
	{
		return dhandle_.valueCount();
	}

	long bitDepth()
	{
		return dhandle_.bitDepth();
	}

	long channelCount()
	{
		return dhandle_.channelCount();
	}

	long errorNo()
	{
		return errno_;
	}

	GSL::DataHandle createCropped(long headCutValueCount,
								  long tailCutValueCount)
	{
		return dhandle_.createCropped(headCutValueCount, tailCutValueCount);
	}

	GSL::DataHandle createCut(long cutOffset,
							  long cutValueCount)
	{
		return dhandle_.createCut(cutOffset, cutValueCount);
	}

	GSL::DataHandle createReversed()
	{
		return dhandle_.createReversed();
	}
};

class ReversedDataHandle_impl : virtual public DataHandle_impl,
								virtual public ReversedDataHandle_skel
{
	void init(DataHandle sourceHandle)
	{
		DataHandle_impl *impl = dynamic_cast<DataHandle_impl *>(sourceHandle._base());
		dhandle_ = impl->createReversed();
	}
};

class CroppedDataHandle_impl : virtual public DataHandle_impl,
							   virtual public CroppedDataHandle_skel
{
	void init(DataHandle sourceHandle,
			  long headCutValueCount,
			  long tailCutValueCount)
	{
		DataHandle_impl *impl = dynamic_cast<DataHandle_impl *>(sourceHandle._base());
		dhandle_ = impl->createCropped(headCutValueCount, tailCutValueCount);
	}
};

class CutDataHandle_impl : virtual public DataHandle_impl,
						   virtual public CutDataHandle_skel
{
	void init(DataHandle sourceHandle,
			  long cutOffset,
			  long cutValueCount)
	{
		DataHandle_impl *impl = dynamic_cast<DataHandle_impl *>(sourceHandle._base());
		dhandle_ = impl->createCut(cutOffset, cutValueCount);
	}
};

static GSL::DataHandle getDHandle(DataHandle handle)
{
	DataHandle_impl *impl = dynamic_cast<DataHandle_impl *>(handle._base());
	if(impl)
		return impl->dhandle_;
	return GSL::DataHandle::null();
}

GslWaveChunk* const_wchunk_from_freq(gpointer wchunk_data, gfloat)
{
	return (GslWaveChunk*)wchunk_data;
}

// FIXME: Think about whether this is needed as a parameter like mixerFrequency
#define OSCILLATOR_FREQUENCY 440.0f

class DataHandlePlay_impl : virtual public DataHandlePlay_skel,
							virtual public StdSynthModule
{
protected:
	DataHandle dhandle_;

	GSL::DataHandle gslDHandle_;
	int errno_;
	GslWaveChunk *wchunk_;
	GslErrorType wchunkError_;
	GslWaveOscData *wosc_;

	float mixerFrequency_;
	unsigned short channelIndex_;
	float speed_;
	unsigned long pos_;
	bool finished_;
	bool paused_;

public:
	DataHandlePlay_impl()
		: dhandle_(DataHandle::null()),
		  gslDHandle_(GSL::DataHandle::null()),
		  wchunk_(NULL),
		  wosc_(NULL),
		  mixerFrequency_(0), // FIXME: doc says soundserver's freq
		  channelIndex_(0),
		  speed_(1.0f),
		  pos_(0),
		  finished_(false),
		  paused_(false)
	{}

	~DataHandlePlay_impl()
	{
		handle(DataHandle::null());
	}

	DataHandle handle()
	{
		return dhandle_;
	}

	void handle(DataHandle handle)
	{
		deleteWaveChunk();

		if(!gslDHandle_.isNull() && !errno_)
			gslDHandle_.close();

		dhandle_ = handle;

		if(handle.isNull())
			gslDHandle_ = GSL::DataHandle::null();
		else
		{
			gslDHandle_ = getDHandle(dhandle_);

			if(gslDHandle_.isNull())
			{
				arts_debug("ERROR: could not get internal GSL::DataHandle!");
				finished(true); // FIXME: can this happen?
			}
			else
			{
				errno_ = gslDHandle_.open();
				if(errno_)
					arts_debug("DataHandlePlay got error from GSL::DataHandle.open(): '%s'",
							   strerror(errno_));
			}
		}
	}

	void streamInit()
	{
		if(!gslDHandle_.isNull() && !wosc_)
		{
			if(!wchunk_)
				createWaveChunk();
			configureWaveOsc();
		}
	}

	float mixerFrequency() { return wosc_ ? wosc_->mix_freq : 0; }
	void mixerFrequency(float f)
	{
		if(wchunk_)
			arts_warning("DataHandlePlay: cannot change mixerFrequency after start of sound processing!");

		if(mixerFrequency() != f)
		{
			mixerFrequency_ = f;
			mixerFrequency_changed(f);
		}
	}

	long channelIndex() { return channelIndex_; }
	void channelIndex(long ci)
	{
		if(channelIndex_ != ci)
		{
			channelIndex_ = ci;
			if(wosc_)
			{
				GslWaveOscConfig config = wosc_->config;
				config.channel = ci;
				gsl_wave_osc_config(wosc_, &config);
			}
			channelIndex_changed(ci);
		}
	}

	float speed() { return speed_; }
	void speed(float s)
	{
		if(speed_ != s)
		{
			speed_ = s;
			if(wosc_)
			{
				GslWaveOscConfig config = wosc_->config;
				config.cfreq = OSCILLATOR_FREQUENCY * speed();
				gsl_wave_osc_config(wosc_, &config);
			}
			speed_changed(s);
		}
	}

	long pos() { return wosc_ ? wosc_->block.offset : 0; }
	void pos(long p)
	{
		if(pos() != p)
		{
			GslWaveOscConfig config = wosc_->config;
			config.start_offset = p;
			gsl_wave_osc_config(wosc_, &config);
			pos_changed(p);
		}
	}

	bool finished() { return finished_; }
	void finished(bool f)
	{
		if(finished_ != f)
		{
			finished_ = f;
			finished_changed(f);
		}
	}

	bool paused() { return paused_; }
	void paused(bool p)
	{
		paused_ = p;
	}

	void deleteWaveChunk()
	{
		deleteWaveOsc();

		if(wchunk_)
		{
			arts_debug("DataHandlePlay_impl: close()ing gsl_wave_chunk");
			gsl_wave_chunk_close(wchunk_);
			gsl_wave_chunk_unref(wchunk_);
			wchunk_ = NULL;
		}
	}

	void createWaveChunk()
	{
		deleteWaveChunk();

		if(!gslDHandle_.isNull() && gslDHandle_.isOpen())
		{
			GslDataCache *dcache = gslDHandle_.createGslDataCache();
			if(!dcache)
			{
				arts_debug("FATAL: creating data cache failed!");
				finished(true); // FIXME: can this happen?
			}
			else
			{
				wchunk_ =
					gsl_wave_chunk_new(dcache,
									   OSCILLATOR_FREQUENCY,
									   mixerFrequency_,
									   GSL_WAVE_LOOP_NONE,
									   0, 0, 0);
				arts_debug("DataHandlePlay_impl: open()ing gsl_wave_chunk");
				wchunkError_ = gsl_wave_chunk_open(wchunk_);
				gsl_data_cache_unref(dcache);
			}
		}
	}

	void deleteWaveOsc()
	{
		if(wosc_)
		{
			gsl_wave_osc_shutdown(wosc_);
			delete wosc_;
			wosc_ = NULL;
		}
	}

	void configureWaveOsc()
	{
		if(wchunk_)
		{
			GslWaveOscConfig config;
			memset(&config, 0, sizeof (GslWaveOscConfig));
			config.start_offset = 0;
			config.play_dir = 1;
			config.wchunk_data = (gpointer)wchunk_;
			config.wchunk_from_freq = &const_wchunk_from_freq;
			config.channel = channelIndex();
			config.cfreq = OSCILLATOR_FREQUENCY * speed();

			if(!wosc_)
			{
				wosc_ = new GslWaveOscData();
				gsl_wave_osc_init(wosc_);
			}

			gsl_wave_osc_config(wosc_, &config);
		}
	}

	void calculateBlock(unsigned long samples)
	{
		if(!paused_ && wchunk_)
		{
			if (!gsl_wave_osc_process(wosc_, samples, 0, 0, 0, outvalue))
				arts_debug("gsl_wave_osc_process failed.");
			finished(wosc_->done);
		}
		else
			for(unsigned long i = 0; i < samples; i++)
				outvalue[i] = 0.f;
		/*
		unsigned long haveSamples = 0;

		while(haveSamples != samples)
		{
			long n = min(gslDHandle_.valueCount() - pos_, samples - haveSamples);
			long got = gslDHandle_.isNull() ?
					   0 : gslDHandle_.read(pos_, n, &outvalue[haveSamples]);
			if(got <= 0)
			{
				for(unsigned long i = haveSamples; i < samples; i++)
					outvalue[i] = 0.f;
				break;
			}
			pos_ += got;
			haveSamples += got;
		}
		finished(pos_ >= gslDHandle_.valueCount());
		*/
	}

	DataHandlePlay clone()
	{
		arts_debug("DataHandlePlay_impl: clone()ing myself");
		DataHandlePlay_impl *result = new DataHandlePlay_impl();

		result->dhandle_ = dhandle_;
		result->gslDHandle_ = gslDHandle_;
		result->errno_ = errno_;
		
		createWaveChunk();
		if(wchunk_)
		{
			result->wchunk_ = wchunk_;
			gsl_wave_chunk_ref(wchunk_);
			result->wchunkError_ = gsl_wave_chunk_open(wchunk_);
		}
		else
			result->wchunkError_ = wchunkError_;

		result->mixerFrequency_ = mixerFrequency_;
		result->channelIndex_ = channelIndex_;
		result->speed_ = speed_;
		result->pos_ = pos_;
		result->finished_ = finished_;
		result->paused_ = paused_;

		return DataHandlePlay::_from_base(result);
	}
};

class WaveDataHandle_impl : virtual public DataHandle_impl,
							virtual public WaveDataHandle_skel
{
	GSL::WaveDataHandle wdhandle_;

public:
	WaveDataHandle_impl() : wdhandle_(GSL::WaveDataHandle::null()) {}

	float mixerFrequency()
	{
		return wdhandle_.mixerFrequency();
	}

	float oscillatorFrequency()
	{
		return wdhandle_.oscillatorFrequency();
	}

	bool load(const string& filename)
	{
		return load(filename, 0, 0);
	}

	bool load(const string& filename,
			  long waveIndex,
			  long chunkIndex)
	{
		wdhandle_ = GSL::WaveDataHandle(filename, waveIndex, chunkIndex);
		if(dhandle_.isOpen())
			dhandle_.close();
		dhandle_ = wdhandle_;
		errno_ = dhandle_.isNull() ? 0 : dhandle_.open();
		return isLoaded();
	}

	bool isLoaded()
	{
		if(wdhandle_.isNull())
			return false;

		return !wdhandle_.error();
	}

	DataHandlePlay createPlayer()
	{
		DataHandlePlay_impl *result= new DataHandlePlay_impl();

		result->mixerFrequency(mixerFrequency());
		result->handle(WaveDataHandle::_from_base(this->_copy()));

		return DataHandlePlay::_from_base(result);
	}
};

REGISTER_IMPLEMENTATION(DataHandlePlay_impl);
REGISTER_IMPLEMENTATION(DataHandle_impl);
REGISTER_IMPLEMENTATION(CroppedDataHandle_impl);
REGISTER_IMPLEMENTATION(CutDataHandle_impl);
REGISTER_IMPLEMENTATION(ReversedDataHandle_impl);
REGISTER_IMPLEMENTATION(WaveDataHandle_impl);

}
