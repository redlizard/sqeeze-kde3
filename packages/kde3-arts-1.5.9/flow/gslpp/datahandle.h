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

#ifndef GSLPP_DATAHANDLE_H
#define GSLPP_DATAHANDLE_H

#include <gsl/gsldatahandle.h>
#include <gsl/gslloader.h>
#include <string>

namespace GSL
{
	class WaveFileInfo;
	class WaveDataHandle;
	class WaveDescription;

	class DataHandle
	{
	protected:
		GslDataHandle *handle_;

		void clearHandle();
		void copyFrom(const DataHandle &other);

		DataHandle(GslDataHandle *handle);

	public:
		static DataHandle null() { return DataHandle(); }

		DataHandle(): handle_(0) {}

		DataHandle(const DataHandle &other);

		DataHandle &operator =(const DataHandle &other);

		bool operator ==(const DataHandle &other) const;

		bool isNull() const;

		virtual ~DataHandle();

		/**
		 * returns error code with errno meaning
		 */
		virtual gint open();

		virtual void close();

		bool isOpen() const;

		/**
		 * read from this datahandle and fill buffer pointed to by
		 * values with sample data from the range
		 * [valueOffset..valueOffset+valueCount-1]
		 *
		 * returns number of sample values actually read, -1 on error
		 * (errno _may_ be useful then)
		 */
		virtual GslLong read(GslLong valueOffset, GslLong valueCount, gfloat *values);

		/**
		 * Create a new data handle by chopping off headCutValueCount
		 * values at the start, and tailCutValueCount values at the
		 * end of this data handle.
		 */
		DataHandle createCropped(GslLong headCutValueCount,
								 GslLong tailCutValueCount);

		/**
		 * Create a new data handle by removing the values
		 * [cutOffset..cutOffset+cutValueCount-1].
		 */
		DataHandle createCut(GslLong cutOffset,
							 GslLong cutValueCount);

		/**
		 * Create a new data handle which contains the same values as
		 * this one but in reversed order.
		 */
		DataHandle createReversed();

		// this will maybe be made private in the future, having a
		// better interface instead
		GslDataCache *createGslDataCache();

		GslLong valueCount() const;

		guint bitDepth() const;

		guint channelCount() const;
	};

	class WaveChunkDescription
	{
		friend class WaveDescription;

		const GslWaveDsc    *parent_;
		guint                parentIndex_;

		WaveChunkDescription(const GslWaveDsc *parent, guint index);

	public:
		float oscillatorFrequency();
		float mixerFrequency();

		GslWaveLoopType loopType();
		GslLong loopStart();	/* sample offset */
		GslLong loopEnd();	/* sample offset */
		guint loopCount();

		WaveDataHandle createDataHandle();
	};

	class WaveDescription
	{
		friend class WaveFileInfo; // for construction of WaveDescriptions

		mutable GslWaveFileInfo    *parentInfo_;  // mutable for ensurePointer()
		std::string                 name_;
		guint                       parentIndex_;
		mutable GslWaveDsc         *desc_;
		mutable GslErrorType        error_;

		void ensurePointer() const; // lazy construction

		void copyFrom(const WaveDescription &other);

		WaveDescription(GslWaveFileInfo *parent, guint index, const std::string &name);

	public:
		~WaveDescription();

		WaveDescription(const WaveDescription &other);

		WaveDescription &operator =(const WaveDescription &other);

		const std::string &name() const;

		GslErrorType error() const;

		guint chunkCount() const;

		WaveChunkDescription chunkDescription(guint index) const;

		guint channelCount() const;
	};

	class WaveFileInfo
	{
		GslWaveFileInfo *info_;
		GslErrorType     error_;
		std::string      filename_;

		void copyFrom(const WaveFileInfo &other);

	public:
		WaveFileInfo(const std::string &filename);

		~WaveFileInfo();

		WaveFileInfo(const WaveFileInfo &other);

		WaveFileInfo &operator =(const WaveFileInfo &other);

		guint waveCount() const;

		std::string waveName(guint index) const;

		WaveDescription waveDescription(guint index);

		GslErrorType error() const;
	};

	class WaveDataHandle: public DataHandle
	{
		friend class WaveChunkDescription;

		GslErrorType     error_;

		float oscillatorFrequency_;
		float mixerFrequency_;

		WaveDataHandle();
		WaveDataHandle(const GslWaveDsc *waveDesc, guint chunkIndex = 0);

	public:
		WaveDataHandle(const std::string& filename,
					   guint waveIndex = 0,
					   guint chunkIndex = 0);

		static WaveDataHandle null() { return WaveDataHandle(); }

		GslErrorType error() const;

		float oscillatorFrequency() const;

		float mixerFrequency() const;
	};
}

#endif // GSLPP_DATAHANDLE_H
