    /*

    Copyright (C) 2003 Stefan Westerfeld
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

/*
 * This is a threaded example using the aRts C API.
 * 
 * Compile programs using the aRts C API with
 * 
 *    cc -o artscmt artscmt.c `artsc-config --cflags` `artsc-config --libs`
 *
 * If you are using a makefile, it could look like this:
 *
 *    CFLAGS=`artsc-config --cflags`
 *    LDFLAGS=`artsc-config --libs`
 *
 *    artscmt: artscmt.c
 */
#include <artsc.h>
#include <pthread.h>
#include <assert.h>
#include <math.h>
#include <unistd.h>
#include <stdio.h>

pthread_mutex_t arts_mutex; /* pthread mutex */

struct Writer
{
  pthread_t thread;
  arts_stream_t stream;
  float freq;
};

#define BUFFER_SIZE 1024

static void* writer(void *arg)
{
  struct Writer *self = arg;
  int pos = 0;

  while(pos < 44100*10)
  {
    char buffer[BUFFER_SIZE], *to = buffer;
    int i, written;

    for(i=0;i<BUFFER_SIZE/4;i++)
    {
      /* generate two sin waves */
      float fpos = ((float)pos)/44100.0;
      long sample  = (long)(sin(fpos*6.28*self->freq)*15000.0);
      pos++;

      /* put the samples in the packet */
      *to++ = sample & 0xff;
      *to++ = (sample >> 8) & 0xff;                                              
      *to++ = sample & 0xff;
      *to++ = (sample >> 8) & 0xff;
    }

    written = 0;
    do
    {
      int space;
      
      /*
       * Since there is more than one thread, it is important not to keep the lock
       * for a long time. We definitely don't want arts_write to block, while we
       * keep the lock, to ensure that other threads can do something as well. So
       * we check the available buffer space before writing to avoid blocking.
       */
      pthread_mutex_lock(&arts_mutex);
      space = arts_stream_get(self->stream, ARTS_P_BUFFER_SPACE);
      if (space >= BUFFER_SIZE)
      { 
	written = arts_write(self->stream, buffer, BUFFER_SIZE);
	assert(written == BUFFER_SIZE); /* should handle errors here */
      }
      pthread_mutex_unlock(&arts_mutex);
     
      /*
       * If the buffer is full, wait some time to get free space again. The amout of
       * time to wait needs to correspond to the buffer size we use for refilling.
       */
      if (!written)
	usleep(10000); /* 10ms */
    } while(!written);
  }
  return 0;
}

int main()
{
  struct Writer writer1, writer2;
  int error;

  error = arts_init();
  if(error < 0)
  {
    fprintf(stderr, "error initializing aRts driver: %s\n", arts_error_text(error));
    return 1;
  }

  pthread_mutex_init(&arts_mutex, 0);

  writer1.stream = arts_play_stream(44100, 16, 2, "artscmt1");
  writer1.freq = 440;
  pthread_create(&writer1.thread, NULL, writer, &writer1);

  writer2.stream = arts_play_stream(44100, 16, 2, "artscmt2");
  writer2.freq = 880;
  pthread_create(&writer2.thread, NULL, writer, &writer2);

  pthread_join(writer1.thread, NULL);
  pthread_join(writer2.thread, NULL);

  arts_close_stream(writer1.stream);
  arts_close_stream(writer2.stream);

  pthread_mutex_destroy(&arts_mutex);

  arts_free();

  return 0;
}
