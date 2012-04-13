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

#include "thread.h"
#include <string.h>

using namespace Arts;

// Thread:
Thread::~Thread()
{
	delete impl;
}

Thread_impl::~Thread_impl()
{
}

// Mutex:
Mutex::~Mutex()
{
	delete impl;
}

Mutex_impl::~Mutex_impl()
{
}

// ThreadCondition:

ThreadCondition::~ThreadCondition()
{
	delete impl;
}

ThreadCondition_impl::~ThreadCondition_impl()
{
}

// Semaphore
Semaphore::~Semaphore()
{
	delete impl;
}

Semaphore_impl::~Semaphore_impl()
{
}

// No threading:

namespace Arts {

class SystemThreadsNoMutex_impl : public Mutex_impl {
public:
	void lock() {};
	bool tryLock() { return true; };
	void unlock() {};
};

static Thread *systemThreadsNoneCurrent = 0;

class SystemThreadsNoThread_impl : public Thread_impl {
private:
	Thread *thread;
public:
	SystemThreadsNoThread_impl(Thread *thread) : thread(thread) {}
	void setPriority(int) {}
	void start() {
		Thread *oldCurrent = systemThreadsNoneCurrent;

		systemThreadsNoneCurrent = thread;
		thread->run();
		systemThreadsNoneCurrent = oldCurrent;
	}
	void waitDone() {}
};

class SystemThreadsNoThreadCondition_impl : public ThreadCondition_impl {
public:
	void wakeOne() {};
	void wakeAll() {};
	void wait(Mutex_impl *) {};
};

class SystemThreadsNoSemaphore_impl : public Semaphore_impl {
public:
	void wait() {}
	int tryWait() { return 0; }
	void post() {}
	int getValue() { return 0; }
};

class SystemThreadsNone : public SystemThreads {
public:
	bool isMainThread() {
		return (systemThreadsNoneCurrent == 0);
	}
	Mutex_impl *createMutex_impl() {
		return new SystemThreadsNoMutex_impl();
	}
	Mutex_impl *createRecMutex_impl() {
		return new SystemThreadsNoMutex_impl();
	}
	Thread_impl *createThread_impl(Thread *thread) {
		return new SystemThreadsNoThread_impl(thread);
	}
	ThreadCondition_impl *createThreadCondition_impl() {
		return new SystemThreadsNoThreadCondition_impl();
	}
	Thread *getCurrentThread() {
		return systemThreadsNoneCurrent;
	}
	Semaphore_impl *createSemaphore_impl(int, int) {
		return new SystemThreadsNoSemaphore_impl();
	}
};

}

static SystemThreadsNone systemThreadsNone;
static SystemThreads *SystemThreads_the = 0;

SystemThreads *SystemThreads::the()
{
	if(!SystemThreads_the) return &systemThreadsNone;
	return SystemThreads_the;
}

bool SystemThreads::init(SystemThreads *the)
{
	if(SystemThreads_the)
		return false;
	else
	{
		SystemThreads_the = the;
		return true;
	}
}

bool SystemThreads::supported()
{
	return (SystemThreads_the != 0);
}

SystemThreads::~SystemThreads()
{
}

namespace Arts { void ARTS_EXPORT *gslGlobalMutexTable = 0; }
