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

#include "datahandle.h"
#include "../../mcop/debug.h"

namespace GSL {

void DataHandle::copyFrom(const DataHandle &other)
{
	handle_ = other.handle_;
	if(handle_)
		gsl_data_handle_ref(handle_);
}

DataHandle::DataHandle(GslDataHandle *handle)
	: handle_(handle)
{
}

DataHandle::DataHandle(const DataHandle &other) { copyFrom(other); }

DataHandle &DataHandle::operator =(const DataHandle &other)
{
	if(!( other == *this )) {
		if(handle_)
			gsl_data_handle_unref(handle_);
		copyFrom(other);
	}
	return *this;
}

bool DataHandle::operator ==(const DataHandle &other) const
{
	return handle_ == other.handle_;
}

bool DataHandle::isNull() const
{
	return handle_ == 0;
}

DataHandle::~DataHandle()
{
	if(handle_)
		gsl_data_handle_unref(handle_);
}

gint DataHandle::open()
{
	arts_return_val_if_fail(handle_ != 0, -1);

	arts_debug("open()ing datahandle (open_count before: %d)..",
			   handle_->open_count);

	return gsl_data_handle_open(handle_);
}

void DataHandle::close()
{
	arts_return_if_fail(handle_ != 0);

	arts_debug("close()ing datahandle (open_count before: %d)..",
			   handle_->open_count);

	gsl_data_handle_close(handle_);
}

bool DataHandle::isOpen() const
{
	if(!handle_)
		return false;

	return handle_->open_count != 0;
}

GslLong DataHandle::read(GslLong valueOffset, GslLong valueCount, gfloat *values)
{
	arts_return_val_if_fail(handle_ != 0, 0);

	return gsl_data_handle_read(handle_, valueOffset, valueCount, values);
}

DataHandle DataHandle::createCropped(GslLong headCutValueCount,
									 GslLong tailCutValueCount)
{
	arts_return_val_if_fail(handle_ != 0, null());

	return DataHandle(gsl_data_handle_new_crop(handle_,
											   headCutValueCount, tailCutValueCount));
}

DataHandle DataHandle::createCut(GslLong cutOffset,
								 GslLong cutValueCount)
{
	arts_return_val_if_fail(handle_ != 0, null());

	return DataHandle(gsl_data_handle_new_cut(handle_, cutOffset,
											  cutValueCount));
}

DataHandle DataHandle::createReversed()
{
	arts_return_val_if_fail(handle_ != 0, null());

	return DataHandle(gsl_data_handle_new_reverse(handle_));
}

GslDataCache *DataHandle::createGslDataCache()
{
	arts_debug("wanna have cache with padding %d for each of %d channels..",
			   gsl_get_config()->wave_chunk_padding,
			   channelCount());
	return gsl_data_cache_from_dhandle(handle_,
									   gsl_get_config()->wave_chunk_padding *
									   channelCount());
}

GslLong DataHandle::valueCount() const
{
	arts_return_val_if_fail(handle_ != 0, 0);
	arts_return_val_if_fail(isOpen(), 0);

	return handle_->setup.n_values;
}

guint DataHandle::channelCount() const
{
	arts_return_val_if_fail(handle_ != 0, 0);
	arts_return_val_if_fail(isOpen(), 0);

	return handle_->setup.n_channels;
}

guint DataHandle::bitDepth() const
{
	arts_return_val_if_fail(handle_ != 0, 0);
	arts_return_val_if_fail(isOpen(), 0);

	return handle_->setup.bit_depth;
}

// ----------------------------------------------------------------------------

WaveChunkDescription::WaveChunkDescription(const GslWaveDsc *parent, guint index)
	: parent_(parent), parentIndex_(index)
{
	if(index>parent->n_chunks)
	{
		arts_debug("wrong index given to WaveChunkDescription constructor, "
				   "using 0 instead..");
		parentIndex_ = 0;
	}
}

float WaveChunkDescription::oscillatorFrequency()
{
	return parent_->chunks[parentIndex_].osc_freq;
}

float WaveChunkDescription::mixerFrequency()
{
	return parent_->chunks[parentIndex_].mix_freq;
}

GslWaveLoopType WaveChunkDescription::loopType()
{
	return parent_->chunks[parentIndex_].loop_type;
}

GslLong WaveChunkDescription::loopStart()
{
	return parent_->chunks[parentIndex_].loop_start;
}

GslLong WaveChunkDescription::loopEnd()
{
	return parent_->chunks[parentIndex_].loop_end;
}

guint WaveChunkDescription::loopCount()
{
	return parent_->chunks[parentIndex_].loop_count;
}

WaveDataHandle WaveChunkDescription::createDataHandle()
{
	return WaveDataHandle(parent_, parentIndex_);
}

// ----------------------------------------------------------------------------

/**
 * We use GslWaveFileInfos' refcounting - probably this lazy
 * construction happens only once, where the object is used. After
 * copying it would have to be constructed again, but that's not
 * likely to happen. (?)
 */
void WaveDescription::ensurePointer() const
{
	if(!desc_)
		desc_ = gsl_wave_dsc_load(parentInfo_, parentIndex_, &error_);
}

void WaveDescription::copyFrom(const WaveDescription &other)
{
	parentInfo_ = other.parentInfo_;
	parentIndex_ = other.parentIndex_;
	gsl_wave_file_info_ref(other.parentInfo_);
}

// internal, private
WaveDescription::WaveDescription(GslWaveFileInfo *parent, guint index,
								 const std::string &name)
	: parentInfo_(parent), name_(name), parentIndex_(index),
	  desc_(0), error_(GSL_ERROR_NONE)
{
	gsl_wave_file_info_ref(parentInfo_);
}

WaveDescription::~WaveDescription()
{
	if(desc_)
		gsl_wave_dsc_free(desc_);
	gsl_wave_file_info_unref(parentInfo_);
}

WaveDescription::WaveDescription(const WaveDescription &other)
	: desc_(0), error_(GSL_ERROR_NONE)
{
	copyFrom(other);
}

WaveDescription &WaveDescription::operator =(const WaveDescription &other)
{
	if(desc_)
		gsl_wave_dsc_free(desc_);
	gsl_wave_file_info_unref(parentInfo_);
	copyFrom(other);
	return *this;
}

const std::string &WaveDescription::name() const
{
	return name_;
}

GslErrorType WaveDescription::error() const
{
	ensurePointer();
	return error_;
}

guint WaveDescription::chunkCount() const
{
	ensurePointer();
	return desc_ ? desc_->n_chunks : 0;
}

WaveChunkDescription WaveDescription::chunkDescription(guint index) const
{
	ensurePointer();
	return WaveChunkDescription(desc_, index);
}

guint WaveDescription::channelCount() const
{
	ensurePointer();
	return desc_ ? desc_->n_channels : 0;
}

// ----------------------------------------------------------------------------

void WaveFileInfo::copyFrom(const WaveFileInfo &other)
{
	info_ = other.info_;
	filename_ = other.filename_;
	if(info_)
		gsl_wave_file_info_ref(info_);

	error_ = other.error_;
}

WaveFileInfo::WaveFileInfo(const std::string &filename)
	: info_(0), error_(GSL_ERROR_NONE), filename_(filename)
{
	info_ = gsl_wave_file_info_load(filename.c_str(), &error_);
}

WaveFileInfo::~WaveFileInfo()
{
	if(info_)
		gsl_wave_file_info_unref(info_);
}

WaveFileInfo::WaveFileInfo(const WaveFileInfo &other)
{
	copyFrom(other);
}

WaveFileInfo &WaveFileInfo::operator =(const WaveFileInfo &other)
{
	if(info_)
		gsl_wave_file_info_unref(info_);
	copyFrom(other);
	return *this;
}

guint WaveFileInfo::waveCount() const
{
	return info_ ? info_->n_waves : 0;
}

std::string WaveFileInfo::waveName(guint index) const
{
	if(index >= waveCount())
		return "";
	return info_->waves[index].name;
}

WaveDescription WaveFileInfo::waveDescription(guint index)
{
	return WaveDescription(info_, index, waveName(index));
}

GslErrorType WaveFileInfo::error() const
{
	return error_;
}

// ----------------------------------------------------------------------------

WaveDataHandle::WaveDataHandle()
	: DataHandle(NULL),
	  oscillatorFrequency_(0),
	  mixerFrequency_(0)
{
}

WaveDataHandle::WaveDataHandle(const GslWaveDsc *waveDesc, guint chunkIndex)
	: DataHandle(NULL),
	  oscillatorFrequency_(0),
	  mixerFrequency_(0)
{
	handle_ = gsl_wave_handle_create(const_cast<GslWaveDsc *>(waveDesc),
									 chunkIndex, &error_);
	if(error() == GSL_ERROR_NONE)
	{
		oscillatorFrequency_ = waveDesc->chunks[chunkIndex].osc_freq;
		mixerFrequency_ = waveDesc->chunks[chunkIndex].mix_freq;
	}
}

WaveDataHandle::WaveDataHandle(const std::string& filename,
							   guint waveIndex,
							   guint chunkIndex)
	: DataHandle(NULL),
	  oscillatorFrequency_(0),
	  mixerFrequency_(0)
{
	GSL::WaveFileInfo info(filename);
	error_ = info.error();

	if(!info.error())
	{
		GSL::WaveDescription desc= info.waveDescription(waveIndex);
		error_ = desc.error();

		if(!desc.error() && (desc.chunkCount() > chunkIndex))
		{
			GSL::WaveChunkDescription chunkDesc= desc.chunkDescription(chunkIndex);

			(*this) = chunkDesc.createDataHandle();
		}
	}
}

GslErrorType WaveDataHandle::error() const
{
	return error_;
}

float WaveDataHandle::oscillatorFrequency() const
{
	return oscillatorFrequency_;
}

float WaveDataHandle::mixerFrequency() const
{
	return mixerFrequency_;
}

}
