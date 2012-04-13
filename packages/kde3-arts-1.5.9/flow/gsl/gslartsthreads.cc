    /*

    Copyright (C) 2001-2002 Stefan Westerfeld
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

#include "thread.h"
#include "debug.h"
#include "gslglib.h"
#include <pthread.h>

#if defined(__DECCXX)
#define EXTC extern "C"
#else
#define EXTC
#endif

EXTC gpointer
gsl_arts_mutex_new ()
{
  return new Arts::Mutex;
}

EXTC void
gsl_arts_mutex_free (gpointer mutex)
{
  Arts::Mutex *m = static_cast<Arts::Mutex *>(mutex);
  delete m;
}

EXTC void
gsl_arts_mutex_lock (gpointer mutex)
{
  Arts::Mutex *m = static_cast<Arts::Mutex *>(mutex);
  m->lock();
}

EXTC gboolean
gsl_arts_mutex_trylock (gpointer mutex)
{
  Arts::Mutex *m = static_cast<Arts::Mutex *>(mutex);
  return m->tryLock();
}

EXTC void
gsl_arts_mutex_unlock (gpointer mutex)
{
  Arts::Mutex *m = static_cast<Arts::Mutex *>(mutex);
  m->unlock();
}

EXTC gpointer
gsl_arts_cond_new ()
{
  return new Arts::ThreadCondition;
}

EXTC void
gsl_arts_cond_free (gpointer cond)
{
  Arts::ThreadCondition *c = static_cast<Arts::ThreadCondition *>(cond);
  delete c;
}

EXTC void
gsl_arts_cond_signal (gpointer cond)
{
  Arts::ThreadCondition *c = static_cast<Arts::ThreadCondition *>(cond);
  c->wakeOne();
}

EXTC void
gsl_arts_cond_broadcast (gpointer cond)
{
  Arts::ThreadCondition *c = static_cast<Arts::ThreadCondition *>(cond);
  c->wakeAll();
}

EXTC void
gsl_arts_cond_wait (gpointer cond, gpointer mutex)
{
  Arts::ThreadCondition *c = static_cast<Arts::ThreadCondition *>(cond);
  Arts::Mutex *m = static_cast<Arts::Mutex *>(mutex);

  c->wait(*m);
}

EXTC void
gsl_arts_cond_timed_wait (gpointer /*cond*/, gpointer /*mutex*/, GTimeVal * /*time*/)
{
  arts_assert(false);
}


class GslArtsThread : public Arts::Thread {
protected:
  gpointer (*func)(gpointer data);
  gpointer data;
  gpointer result;

public:
  GThread gthread;

  GslArtsThread(gpointer (*func)(gpointer data), gpointer data)
    : func(func), data(data)
  {
    gthread.data = 0;
  }
  void run()
  {
    result = func(data);
  }
};


/* KCC (KAI C++) is buggy.  If we write out the type of the first argument
   to gsl_arts_thread_create(), ala
     gsl_arts_thread_create (gpointer (*func) (gpointer data2), ...)
   it becomes C++ linkage, i.e. it's name gets mangled, _despite_ declared
   extern "C" in the header.  Other sources only calling this function,
   i.e. those only seeing the prototype correctly call the unmangled
   extern "C" variant, but for the above reason it isn't defined anywhere.
   The solution is to go through a typedef for that argument, _which must
   also be declared extern "C"_.  I'm not sure, but I think it's an error
   of KCC, that it puts the invisible type of the first arg into C++ mode,
   although the whole function should be C only.  If one declares
   two equal function types, one extern "C", one not, they are even assignment
   compatible.  But somehow KCC puts that type into C++ mode, which for
   other strange reasons force the whole function to go into C++ linkage.
   It's enough, when this typedef is local to this file.  (matz)  */
   
/* Due to gcc's unhappyness with 'extern "C" typedef ...' we enclose
   it in a real extern "C" {} block.  */
extern "C" {
typedef gpointer (*t_func)(gpointer data2);
}

// This is C++:
GThread* gsl_arts_thread_create_full_CPP(gpointer (*func)(gpointer data),
                                  gpointer               data,
                                  gulong                 /*stack_size*/,
                                  gboolean               /*joinable*/,
                                  gboolean               /*bound*/,
                                  GThreadPriority        /*priority*/,
                                  GError               ** /*error*/) 
{
  GslArtsThread *thread = new GslArtsThread(func, data);
  return &thread->gthread;
}

// This is C:
extern "C" GThread* 
gsl_arts_thread_create_full(gpointer (*func)(gpointer data),
                                  gpointer               data,
                                  gulong                 stack_size,
                                  gboolean               joinable,
                                  gboolean               bound,
                                  GThreadPriority        priority,
                                  GError               **error)
{
  return gsl_arts_thread_create_full_CPP(func, 
                                         data, 
					 stack_size, 
					 joinable, 
					 bound,
					 priority,
					 error);
}

EXTC gpointer
gsl_arts_thread_self ()
{
  GslArtsThread *current = static_cast<GslArtsThread *>(Arts::SystemThreads::the()->getCurrentThread());
 
  if(current)
    return &current->gthread;
  else
    {
      static GThread mainThread = { 0, };
      return &mainThread;
    }
}

EXTC void 
gsl_arts_thread_init (gpointer /*arg*/)
{
}

/* vim:set ts=8 sts=2 sw=2: */
