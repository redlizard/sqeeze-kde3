    /*

    Copyright (C) 2001 Stefan Westerfeld
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* only compile this if we have libpthread available */
#ifdef HAVE_LIBPTHREAD

#include <gsl/gslconfig.h>

#include <sys/types.h>
#include <stddef.h>
#include <stdarg.h>
#include <pthread.h>
#include <semaphore.h>
#include <debug.h>
#include <string.h>

#include "thread.h"

/*
 * define this if you want to protect mutexes against being locked twice by
 * the same thread
 */
#undef PTHREAD_DEBUG

namespace Arts {

extern void *gslGlobalMutexTable;

namespace PosixThreads {

class ThreadCondition_impl;

class Mutex_impl : public Arts::Mutex_impl {
protected:
	friend class ThreadCondition_impl;
	pthread_mutex_t mutex;

#ifdef PTHREAD_DEBUG
	pthread_t owner;
#endif

public:
	Mutex_impl()
	{
		pthread_mutex_init(&mutex, 0);
#ifdef PTHREAD_DEBUG
		owner = 0;
#endif
	}
	void lock()
	{
#ifdef PTHREAD_DEBUG
		pthread_t self = pthread_self();
		arts_assert(owner != self);
#endif

		pthread_mutex_lock(&mutex);

#ifdef PTHREAD_DEBUG
		arts_assert(!owner);
		owner = self;
#endif
	}
	bool tryLock()
	{
#ifdef PTHREAD_DEBUG
		pthread_t self = pthread_self();
		arts_assert(owner != self);
#endif

		int result = pthread_mutex_trylock(&mutex);

#ifdef PTHREAD_DEBUG
		if(result == 0)
		{
			arts_assert(!owner);
			owner = self;
		}
#endif
		return(result == 0);
	}
	void unlock()
	{
#ifdef PTHREAD_DEBUG
		arts_assert(owner == pthread_self());
		owner = 0;
#endif

		pthread_mutex_unlock(&mutex);
	}
};

class RecMutex_impl : public Arts::Mutex_impl {
protected:
	friend class ThreadCondition_impl;
	pthread_mutex_t mutex;
	pthread_t owner;
	int count;

public:
	RecMutex_impl()
	{
		pthread_mutex_init(&mutex, 0);
		owner = 0;
		count = 0;
	}
	void lock()
	{
		pthread_t self = pthread_self();
		if(owner != self)
		{
			pthread_mutex_lock(&mutex);
#ifdef PTHREAD_DEBUG
			arts_assert(count == 0);
			arts_assert(!owner);
#endif
			owner = self;
		}
		count++;
	}
	bool tryLock()
	{
		pthread_t self = pthread_self();
		if(owner != self)
		{
			int result = pthread_mutex_trylock(&mutex);
			if(result != 0)
				return false;

#ifdef PTHREAD_DEBUG
			arts_assert(count == 0);
			arts_assert(!owner);
#endif
			owner = self;
		}
		count++;
		return true;
	}
	void unlock()
	{
#ifdef PTHREAD_DEBUG
		arts_assert(owner == pthread_self());
		arts_assert(count > 0);
#endif

		count--;
		if(count == 0)
		{
			owner = 0;
			pthread_mutex_unlock(&mutex);
		}
	}
};


class Thread_impl : public Arts::Thread_impl {
protected:
	friend class PosixThreads;
	pthread_t pthread;
	Thread *thread;

public:
	Thread_impl(Thread *thread) : thread(thread) {
	}
	void setPriority(int priority) {
		struct sched_param sp;
		sp.sched_priority = priority;
		if (pthread_setschedparam(pthread, SCHED_FIFO, &sp))
			arts_debug("Thread::setPriority: sched_setscheduler failed");
	}
	static pthread_key_t privateDataKey;
	static void *threadStartInternal(void *impl)
	{
		pthread_setspecific(privateDataKey, impl);

    	((Thread_impl *)impl)->thread->run();
    	return 0;
	}
	void start() {
		pthread_create(&pthread,0,threadStartInternal,this);
	}
	void waitDone() {
    	void *foo;
    	pthread_join(pthread,&foo);
	}
};

pthread_key_t Thread_impl::privateDataKey;

class ThreadCondition_impl : public Arts::ThreadCondition_impl {
protected:
	pthread_cond_t cond;

public:
	ThreadCondition_impl() {
		pthread_cond_init(&cond, 0);
	}
	~ThreadCondition_impl() {
		pthread_cond_destroy(&cond);
	}
	void wakeOne() {
		pthread_cond_signal(&cond);
	}
	void wakeAll() {
		pthread_cond_broadcast(&cond);
	}
	void wait(Arts::Mutex_impl *mutex) {
#ifdef PTHREAD_DEBUG
		pthread_t self = pthread_self();
		arts_assert(((Mutex_impl *)mutex)->owner == self);
		((Mutex_impl *)mutex)->owner = 0;
#endif

		pthread_cond_wait(&cond, &((Mutex_impl*)mutex)->mutex);

#ifdef PTHREAD_DEBUG
		arts_assert(((Mutex_impl *)mutex)->owner == 0);
		((Mutex_impl *)mutex)->owner = self;
#endif
	}
};

class Semaphore_impl : public Arts::Semaphore_impl
{
private:
	sem_t semaphore;

public:
	Semaphore_impl(int shared, int count) {
		sem_init(&semaphore, shared, count);
	}

	~Semaphore_impl() {
		sem_destroy(&semaphore);
	}

	void wait() {
		sem_wait(&semaphore);
	}

	int tryWait() {
		return sem_trywait(&semaphore);
	}

	void post() {
		sem_post(&semaphore);
	}

	int getValue() {
		int retval;
		sem_getvalue(&semaphore, &retval);
		return retval;
	}
};

class PosixThreads : public SystemThreads {
private:
	pthread_t mainThread;
public:
	PosixThreads() {
		mainThread = pthread_self();
	}
	bool isMainThread() {
		return pthread_equal(pthread_self(), mainThread);
	}
	Arts::Mutex_impl *createMutex_impl() {
		return new Mutex_impl();
	}
	Arts::Mutex_impl *createRecMutex_impl() {
		return new RecMutex_impl();
	}
	Arts::Thread_impl *createThread_impl(Arts::Thread *thread) {
		return new Thread_impl(thread);
	}
	Arts::ThreadCondition_impl *createThreadCondition_impl() {
		return new ThreadCondition_impl();
	}
	Thread *getCurrentThread() {
		void *data = pthread_getspecific(Thread_impl::privateDataKey);

		if(data)
			return ((Thread_impl *)data)->thread;
		else
			return 0; /* main thread */
	}
	Arts::Semaphore_impl *createSemaphore_impl(int shared, int count) {
		return new Semaphore_impl(shared, count);
	}
};

// set posix threads on startup
static class SetSystemThreads {
private:
	PosixThreads posixThreads;

public:
	SetSystemThreads() {
		if(pthread_key_create(&Thread_impl::privateDataKey, 0))
			arts_debug("SystemThreads init: pthread_key_create failed");

		SystemThreads::init(&posixThreads);

	}
	~SetSystemThreads() {
		SystemThreads::init(0);

		if(pthread_key_delete(Thread_impl::privateDataKey))
			arts_debug("SystemThreads init: pthread_key_delete failed");
	}
} initOnStartup;

#include <gsl/gslcommon.h>
/* -fast- locking for gsl on platforms with unix98 support */
#if (GSL_HAVE_MUTEXATTR_SETTYPE > 0)
static void pth_mutex_init (GslMutex *mutex)
{
	/* need NULL attribute here, which is the fast mutex on glibc
	 * and cannot be chosen through the pthread_mutexattr_settype()
	 */
	pthread_mutex_init ((pthread_mutex_t*) mutex, NULL);
}
static void pth_rec_mutex_init (GslRecMutex *mutex)
{
	pthread_mutexattr_t attr;

	pthread_mutexattr_init (&attr);
	pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init ((pthread_mutex_t*) mutex, &attr);
	pthread_mutexattr_destroy (&attr);
}
static void pth_rec_cond_init (GslCond *cond)
{
	pthread_cond_init ((pthread_cond_t*) cond, NULL);
}
static void pth_rec_cond_wait_timed (GslCond *cond, GslMutex *mutex,
									 gulong abs_secs, gulong abs_usecs)
{
	struct timespec abstime;

	abstime.tv_sec = abs_secs;
	abstime.tv_nsec = abs_usecs * 1000;
	pthread_cond_timedwait ((pthread_cond_t*) cond, (pthread_mutex_t*) mutex, &abstime);
}
static GslMutexTable pth_mutex_table = {
	pth_mutex_init,
	(void (*) (GslMutex*)) pthread_mutex_lock,
	(int  (*) (GslMutex*)) pthread_mutex_trylock,
	(void (*) (GslMutex*)) pthread_mutex_unlock,
	(void (*) (GslMutex*)) pthread_mutex_destroy,
	pth_rec_mutex_init,
	(void (*) (GslRecMutex*)) pthread_mutex_lock,
	(int  (*) (GslRecMutex*)) pthread_mutex_trylock,
	(void (*) (GslRecMutex*)) pthread_mutex_unlock,
	(void (*) (GslRecMutex*)) pthread_mutex_destroy,
	pth_rec_cond_init,
	(void (*)            (GslCond*)) pthread_cond_signal,
	(void (*)            (GslCond*)) pthread_cond_broadcast,
	(void (*) (GslCond*, GslMutex*)) pthread_cond_wait,
	pth_rec_cond_wait_timed,
	(void (*)            (GslCond*)) pthread_cond_destroy,
};

static class SetGslMutexTable {
public:
	SetGslMutexTable()
	{
		gslGlobalMutexTable = &pth_mutex_table;
	}
} initGslMutexTable;
#endif

}

}

#endif
