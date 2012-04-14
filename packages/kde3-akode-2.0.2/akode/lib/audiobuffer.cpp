/*  aKode AudioBuffer

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

#include "audiobuffer.h"
#include "audioframe.h"

namespace aKode {

AudioBuffer::AudioBuffer(unsigned int len) : length(len), readPos(0), writePos(0),
    flushed(false), released(false), paused(false), m_eof(false)
{
    buffer = new AudioFrame[len];
}

AudioBuffer::~AudioBuffer() {
    delete[] buffer;
}

bool AudioBuffer::put(AudioFrame* buf, bool blocking) {
    mutex.lock();
    if (released) goto fail;
    flushed = false;
    if ((writePos+1) % length == readPos) {
        if (blocking) {
            not_full.wait(&mutex);
            if (flushed || released) goto fail;
        }
        else
            goto fail;
    }

    swapFrames(&buffer[writePos], buf);
    writePos = (writePos+1) % length;

    not_empty.signal();
    mutex.unlock();
    return true;
fail:
    mutex.unlock();
    return false;
}

bool AudioBuffer::get(AudioFrame* buf, bool blocking) {
    mutex.lock();
    if (released) goto fail;
    if (readPos == writePos || paused) {
        if (blocking && !m_eof) {
            not_empty.wait(&mutex);
            if (released) goto fail;
            if (empty()) goto fail;
        }
        else
            goto fail;
    }

    swapFrames(buf, &buffer[readPos]);
    readPos = (readPos+1) % length;

    not_full.signal();
    mutex.unlock();
    return true;
fail:
    mutex.unlock();
    return false;
}

long AudioBuffer::position() {
    long out = -1;
    mutex.lock();
    if (!empty() && !released)
        out = buffer[readPos].pos;
    mutex.unlock();
    return out;
}

bool AudioBuffer::empty() {
    return (readPos == writePos);
}

bool AudioBuffer::full() {
    return (readPos == (writePos+1) % length);
}

void AudioBuffer::setEOF() {
    mutex.lock();
    m_eof = true;
    not_empty.signal();
    mutex.unlock();
}

bool AudioBuffer::eof() {
    return m_eof && empty();
}

void AudioBuffer::reset() {
    // We assume all processes have been released at this point
    readPos = writePos = 0;
    flushed = released = paused = m_eof = false;
}

void AudioBuffer::flush() {
    mutex.lock();
    // Don't free the frames, most likely this is just a seek
    // and the same buffer-sizes will be needed afterwards.
    readPos = writePos = 0;
    flushed = true;
    not_full.signal();
    mutex.unlock();
}

void AudioBuffer::release() {
    mutex.lock();
    released = true;
    not_full.signal();
    not_empty.signal();
    mutex.unlock();
}

void AudioBuffer::pause() {
    paused = true;
}

void AudioBuffer::resume() {
    mutex.lock();
    paused = false;
    if (!empty())
        not_empty.signal();
    mutex.unlock();
}

} // namespace
