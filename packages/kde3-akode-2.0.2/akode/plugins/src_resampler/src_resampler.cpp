/*  aKode Resampler (Secret Rabbit Code)

    Copyright (C) 2004,2006 Allan Sandfeld Jensen <kde@carewolf.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston,
    MA  02110-1301, USA.
*/

#include "akodelib.h"

#include <samplerate.h>

#include "arithmetic.h"
#include "audioframe.h"
#include "src_resampler.h"

namespace aKode {

SRCResampler* SRCResamplerPlugin::openResampler() {
    return new SRCResampler();
}

extern "C" { SRCResamplerPlugin src_resampler; };

SRCResampler::SRCResampler() : speed(1.0), sample_rate(44100) {};

template<typename S>
static void _convert1_FP(AudioFrame *in, float* outdata)
{
    long length = in->length;
    int channels = in->channels;

    S** indata = (S**)in->data;
    for(int j=0; j<length; j++) {
        for(int i=0; i<channels; i++) {
            outdata[i+j*channels] = (float)(indata[i][j]);
        }
    }
}

template<typename S>
static void _convert1_Int(AudioFrame *in, float* outdata)
{
    float scale = 1.0/(float)Arithm_Int<S>::max(in->sample_width);

    long length = in->length;
    int channels = in->channels;

    S** indata = (S**)in->data;
    for(int j=0; j<length; j++) {
        for(int i=0; i<channels; i++) {
            outdata[i+j*channels] = scale*indata[i][j];
        }
    }
}

// demux channels
template<typename S>
static void _convert2_FP(float *indata, AudioFrame* out)
{
    long length = out->length;
    int channels = out->channels;

    S** outdata = (S**)out->data;
    for(int j=0; j<length; j++) {
        for(int i=0; i<channels; i++) {
            outdata[i][j] = (S)(indata[i+j*channels]);
        }
    }
}

template<typename S>
static void _convert2_Int(float *indata, AudioFrame* out)
{
    float scale = Arithm_Int<S>::max(out->sample_width);

    long length = out->length;
    int channels = out->channels;

    S** outdata = (S**)out->data;
    for(int j=0; j<length; j++) {
        for(int i=0; i<channels; i++) {
            outdata[i][j] = (S)(scale*indata[i+j*channels]);
        }
    }
}

// A high quality resampling by using libsamplerate
bool SRCResampler::doFrame(AudioFrame* in, AudioFrame* out)
{
    // Ouch, I dont like this much mallocing
    float *tmp1 = new float[in->channels*in->length];

    // convert to float frames
    if (in->sample_width == -64) {
        _convert1_FP<double>(in, tmp1);
    } else
    if (in->sample_width == -32) {
        _convert1_FP<float>(in, tmp1);
    } else
    if (in->sample_width <= 8) {
        _convert1_Int<int8_t>(in, tmp1);
    } else
    if (in->sample_width <= 16) {
        _convert1_Int<int16_t>(in, tmp1);
    } else
        _convert1_Int<int32_t>(in, tmp1);

    float ratio = (sample_rate/(float)in->sample_rate) / speed;
    long outlength = (long)((in->length + ratio) * ratio); // max size
    float *tmp2 = new float[in->channels*outlength];

    // We should figure out if interleaving first is faster
    // than making a call per channel
//    for (int i=0; i<in->channels; i++) {
        SRC_DATA src_data;

        src_data.data_in = tmp1;
        src_data.data_out = tmp2;
        src_data.input_frames = in->length;
        src_data.output_frames = outlength;
        src_data.src_ratio = ratio;

        src_simple(&src_data, SRC_SINC_MEDIUM_QUALITY, in->channels);
//    }

    AudioConfiguration config = *in;
    config.sample_width = -32;
    config.sample_rate = sample_rate;
    out->reserveSpace(&config, src_data.output_frames_gen);
    out->pos = in->pos;

    // convert from float frames
#if 0
    if (out->sample_width > 0) {
        if (out->sample_width <= 8) {
            _convert2_Int<int8_t>(tmp2, out);
        } else
        if (out->sample_width <= 16) {
            _convert2_Int<int16_t>(tmp2, out);
        } else
            _convert2_Int<int32_t>(tmp2, out);
    } else
        if (out->sample_width == -64)
            _convert2_FP<double>(tmp2, out);
        else
#endif
            _convert2_FP<float>(tmp2, out);


    delete[] tmp1;
    delete[] tmp2;

    return true;
}

void SRCResampler::setSpeed(float value) {
    speed = value;
}

void SRCResampler::setSampleRate(unsigned int rate) {
    sample_rate = rate;
}

} // namespace
