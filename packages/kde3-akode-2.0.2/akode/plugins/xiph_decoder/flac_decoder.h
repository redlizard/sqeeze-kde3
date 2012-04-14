/*  aKode: FLAC-Decoder

    Copyright (C) 2004 Allan Sandfeld Jensen <kde@carewolf.com>

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
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef _AKODE_FLAC_DECODER_H
#define _AKODE_FLAC_DECODER_H

#include "akodelib.h"
#if defined(HAVE_LIBFLAC) || defined(HAVE_LIBFLAC113)

#include "decoder.h"

namespace aKode {

class File;
class AudioFrame;

class FLACDecoder : public Decoder {
public:
    FLACDecoder(File* src);
    virtual ~FLACDecoder();
    virtual bool readFrame(AudioFrame*);
    virtual long length();
    virtual long position();
    virtual bool seek(long);
    virtual bool seekable();
    virtual bool eof();
    virtual bool error();

    virtual const AudioConfiguration* audioConfiguration();

    struct private_data;
private:
    private_data *m_data;
};

#ifdef HAVE_LIBOGGFLAC
class OggFLACDecoder : public Decoder {
public:
    OggFLACDecoder(File* src);
    virtual ~OggFLACDecoder();
    virtual bool readFrame(AudioFrame*);
    virtual long length();
    virtual long position();
    virtual bool seek(long);
    virtual bool seekable();
    virtual bool eof();
    virtual bool error();

    virtual const AudioConfiguration* audioConfiguration();

    struct private_data;
private:
    private_data *m_data;
};
#endif

class FLACDecoderPlugin : public DecoderPlugin {
public:
    virtual bool canDecode(File*);
    virtual Decoder* openDecoder(File* fil) {
        return new FLACDecoder(fil);
    };
};

extern "C" FLACDecoderPlugin flac_decoder;
#ifdef HAVE_LIBFLAC113
extern "C" FLACDecoderPlugin oggflac_decoder;
#endif

#ifdef HAVE_LIBOGGFLAC
class OggFLACDecoderPlugin : public DecoderPlugin {
public:
    virtual bool canDecode(File*);
    virtual Decoder* openDecoder(File* fil) {
        return new OggFLACDecoder(fil);
    };
};

extern "C" OggFLACDecoderPlugin oggflac_decoder;
#endif

} // namespace


#endif // HAVE_LIBFLAC
#endif
