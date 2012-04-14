/*  aKode: FLAC-Decoder (using libFLAC 1.1.3 API)

    Copyright (C) 2004-2005 Allan Sandfeld Jensen <kde@carewolf.com>

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

#include "akodelib.h"

#ifdef HAVE_LIBFLAC113

#include <string.h>
#include <iostream>

#include <FLAC/format.h>
#include <FLAC/stream_decoder.h>

#include "audioframe.h"
#include "decoder.h"
#include "file.h"
#include "flac_decoder.h"


namespace aKode {

class FLACDecoder;

static bool checkFLAC(File *src) {
    char header[6];
    bool res = false;
    src->seek(0);
    if(src->read(header, 4) == 4) {
        // skip id3v2 headers
        if (memcmp(header, "ID3", 3) == 0) {
            if (src->read(header, 6) != 6) goto end;
            int size = 0;
            if (header[1] & 0x10) size += 10;
            size += header[5];
            size += header[4] << 7;
            size += header[3] << 14;
            size += header[2] << 21;
            src->seek(10+size);
            if (src->read(header, 4) != 4) goto end;
        }
        if (memcmp(header, "fLaC",4) == 0 ) res = true;
    }
end:
    return res;
}

static bool checkOggFLAC(File *src) {
    char header[34];
    bool res = false;
    src->seek(0);
    if (src->read(header, 34) == 34)
        if (memcmp(header, "OggS",4) == 0 )
            // old FLAC 1.1.0 format
            if (memcmp(header+28, "fLaC",4) == 0) res = true;
            else
            // new FLAC 1.1.1 format
            if (memcmp(header+29, "FLAC",4) == 0) res = true;
    return res;
}


bool FLACDecoderPlugin::canDecode(File* src) {
    src->openRO();
    bool res = checkFLAC(src);
    if (!res) res = checkOggFLAC(src);
    src->close();
    return res;
}

extern "C" { FLACDecoderPlugin flac_decoder; }
extern "C" { FLACDecoderPlugin oggflac_decoder; }

struct FLACDecoder::private_data {
    private_data() : decoder(0), valid(false), out(0), source(0), eof(false), error(false) {};

    FLAC__StreamDecoder *decoder;
    const FLAC__StreamMetadata_StreamInfo* si;
    const FLAC__StreamMetadata_VorbisComment* vc;

    bool valid;
    AudioFrame *out;
    File *source;
    AudioConfiguration config;

    uint32_t max_block_size;
    uint64_t position, length;

    bool eof, error;
};

static FLAC__StreamDecoderReadStatus flac_read_callback(
        const FLAC__StreamDecoder *,
        FLAC__byte buffer[],
        unsigned *bytes,
        void *client_data)
{
    FLACDecoder::private_data *data = (FLACDecoder::private_data*)client_data;

    long res = data->source->read((char*)buffer, *bytes);
    if (res<=0) {
        if (data->source->eof()) {
            return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
        }
        else
            return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
    }
    else {
        *bytes = res;
        return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
    }
}

static FLAC__StreamDecoderSeekStatus flac_seek_callback(
        const FLAC__StreamDecoder *,
        FLAC__uint64 absolute_byte_offset,
        void *client_data)
{
    FLACDecoder::private_data *data = (FLACDecoder::private_data*)client_data;

    if(data->source->seek(absolute_byte_offset))
        return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
    else
        return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
}

static FLAC__StreamDecoderTellStatus flac_tell_callback(
        const FLAC__StreamDecoder *,
        FLAC__uint64 *absolute_byte_offset,
        void *client_data)
{
    FLACDecoder::private_data *data = (FLACDecoder::private_data*)client_data;

    long res = data->source->position();
    if (res<0)
        return FLAC__STREAM_DECODER_TELL_STATUS_ERROR;
    else {
        *absolute_byte_offset = res;
        return FLAC__STREAM_DECODER_TELL_STATUS_OK;
    }
}

static FLAC__StreamDecoderLengthStatus flac_length_callback(
        const FLAC__StreamDecoder *,
        FLAC__uint64 *stream_length,
        void *client_data)
{
    FLACDecoder::private_data *data = (FLACDecoder::private_data*)client_data;

    long res = data->source->length();
    if (res<0)
        return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;
    else {
        *stream_length = res;
        return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
    }
}

static FLAC__bool eof_callback(
        const FLAC__StreamDecoder *,
        void *client_data)
{
    FLACDecoder::private_data *data = (FLACDecoder::private_data*)client_data;

    return data->source->eof();
}

static FLAC__StreamDecoderWriteStatus write_callback(
        const FLAC__StreamDecoder *,
        const FLAC__Frame *frame,
        const FLAC__int32 * const buffer[],
        void* client_data)
{
    FLACDecoder::private_data *data = (FLACDecoder::private_data*)client_data;

    if (!data->out)  // Handle spurious callbacks (happens during seeks)
        data->out = new AudioFrame;

    const long frameSize = frame->header.blocksize;
    const char bits = frame->header.bits_per_sample;
    const char channels = frame->header.channels;

    AudioFrame* const outFrame = data->out;

    outFrame->reserveSpace(channels, frameSize, bits);
    outFrame->sample_rate = frame->header.sample_rate;

    if (channels == 1 || channels == 2)
	outFrame->channel_config = aKode::MonoStereo;
    else if (channels > 2 && channels < 8)
	outFrame->channel_config = aKode::Surround;
    else
	outFrame->channel_config = aKode::MultiChannel;

    for(int i = 0; i<channels; i++) {
	if (outFrame->data[i] == 0) break;
        if (bits<=8) {
            int8_t** data = (int8_t**)outFrame->data;
            for(long j=0; j<frameSize; j++)
                data[i][j] = buffer[i][j];
        } else
        if (bits<=16) {
            int16_t** data = (int16_t**)outFrame->data;
            for(long j=0; j<frameSize; j++)
                data[i][j] = buffer[i][j];
        } else {
            int32_t** data = (int32_t**)outFrame->data;
            for(long j=0; j<frameSize; j++)
                data[i][j] = buffer[i][j];
        }
    }
    data->position+=frameSize;
    data->valid = true;
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

static void metadata_callback(
        const FLAC__StreamDecoder *,
        const FLAC__StreamMetadata *metadata,
        void* client_data)
{
    FLACDecoder::private_data *data = (FLACDecoder::private_data*)client_data;

    if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
        data->length         = metadata->data.stream_info.total_samples;
        data->config.sample_rate    = metadata->data.stream_info.sample_rate;
        data->config.sample_width   = metadata->data.stream_info.bits_per_sample;
        data->config.channels       = metadata->data.stream_info.channels;
        data->max_block_size = metadata->data.stream_info.max_blocksize;

        if (data->config.channels <= 2)
	   data->config.channel_config = aKode::MonoStereo;
        else if (data->config.channels <= 7)
	   data->config.channel_config = aKode::Surround;
        else
	   data->config.channel_config = aKode::MultiChannel;

        data->si = &metadata->data.stream_info;

	data->position = 0;

    } else
    if (metadata->type == FLAC__METADATA_TYPE_VORBIS_COMMENT) {
        data->vc = &metadata->data.vorbis_comment;
    }
}

static void error_callback(
        const FLAC__StreamDecoder *,
	FLAC__StreamDecoderErrorStatus status,
        void* /* client_data */)
{
    std::cerr << "FLAC error: " << FLAC__StreamDecoderErrorStatusString[status] << "\n";
    switch (status) {
        case FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC:
            break;
        case FLAC__STREAM_DECODER_ERROR_STATUS_BAD_HEADER:
            break;
        case FLAC__STREAM_DECODER_ERROR_STATUS_FRAME_CRC_MISMATCH:
            break;
        case FLAC__STREAM_DECODER_ERROR_STATUS_UNPARSEABLE_STREAM:
            break;
    }
    //data->valid = false;
}

const AudioConfiguration* FLACDecoder::audioConfiguration() {
    return &m_data->config;
}

FLACDecoder::FLACDecoder(File* src) {
    m_data = new private_data;
    m_data->out = 0;
    m_data->decoder = FLAC__stream_decoder_new();
    m_data->source = src;
    m_data->source->openRO();
    m_data->source->fadvise();
    // ### check return value
    FLAC__stream_decoder_init_stream(
        m_data->decoder,
        flac_read_callback,
        flac_seek_callback,
        flac_tell_callback,
        flac_length_callback,
        eof_callback,
        write_callback,
        metadata_callback,
        error_callback,
        m_data
    );
    FLAC__stream_decoder_process_until_end_of_metadata(m_data->decoder);
}

FLACDecoder::~FLACDecoder() {
    FLAC__stream_decoder_finish(m_data->decoder);
    FLAC__stream_decoder_delete(m_data->decoder);
    m_data->source->close();
    delete m_data;
}

bool FLACDecoder::readFrame(AudioFrame* frame) {
    if (m_data->error || m_data->eof) return false;

    if (m_data->out) { // Handle spurious callbacks
        frame->freeSpace();
        *frame = *m_data->out; // copy
        m_data->out->data = 0; // nullify, don't free!
        delete m_data->out;
        m_data->out = 0;
        return true;
    }
    m_data->valid = false;
    m_data->out = frame;
    bool ret = FLAC__stream_decoder_process_single(m_data->decoder);
    m_data->out = 0;
    if (ret && m_data->valid) {
        frame->pos = position();
        return true;
    } else {
        FLAC__StreamDecoderState state = FLAC__stream_decoder_get_state(m_data->decoder);
        if (state == FLAC__STREAM_DECODER_END_OF_STREAM)
            m_data->eof = true;
        else
        if (state > FLAC__STREAM_DECODER_END_OF_STREAM)
            m_data->error = true;
        return false;
    }
}

long FLACDecoder::length() {
    float pos = ((float)m_data->length)/m_data->config.sample_rate;
    return (long)(pos*1000.0);
}

long FLACDecoder::position() {
    float pos = ((float)m_data->position)/m_data->config.sample_rate;
    return (long)(pos*1000.0);
}

bool FLACDecoder::eof() {
    return m_data->eof;
}

bool FLACDecoder::error() {
    return m_data->error;
}

bool FLACDecoder::seekable() {
    return m_data->source->seekable();
}

bool FLACDecoder::seek(long pos) {
    if (m_data->error) return false;
    float samplePos = (float)pos * (float)m_data->config.sample_rate / 1000.0;
    m_data->position = (uint64_t)samplePos;
    return FLAC__stream_decoder_seek_absolute(m_data->decoder, m_data->position);
}

} // namespace

#endif // HAVE_LIBFLAC113
