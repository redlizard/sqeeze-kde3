    /*

    Copyright (C) 2000-2002 Stefan Westerfeld
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

#include "cpuusage.h"
#include "dispatcher.h"
#include "debug.h"

#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>

#include <iostream>

using namespace std;

namespace Arts {

class Benchmark
{
private:
	struct timeval _start,_stop;
public:
	void start()
	{
		gettimeofday(&_start,NULL);
	}
	float stop()
	{
		gettimeofday(&_stop,NULL);

		float diff = _stop.tv_sec-_start.tv_sec;
		diff += (float)(_stop.tv_usec-_start.tv_usec)/1000000;
		return diff;
	}
};

class CPUUsagePrivate
{
public:
	clock_t oldclock;
	int stalled;
	float usage;
	Benchmark b;
};

/** signal handlers **/

static CPUUsage *cpuUsage = 0;

extern "C" void cpuUsageCheck(int)
{
	if(cpuUsage != 0)
		cpuUsage->check();

	signal(SIGALRM, cpuUsageCheck);
}

/** main stuff **/

CPUUsage::CPUUsage() : d(new CPUUsagePrivate())
{
	d->oldclock = clock();
	d->usage = 0;
	d->stalled = 0;
	d->b.start();
	cpuUsage = this;

	/* setup signal handler & timer */

    struct itimerval oldvalue;
    struct itimerval newvalue = {{ 1, 0 }, {1, 0}};	// 1 second

    setitimer(ITIMER_REAL, &newvalue, &oldvalue);
	signal(SIGALRM, cpuUsageCheck);
}

CPUUsage::~CPUUsage()
{
	delete d;
	cpuUsage = 0;
}

float CPUUsage::usage()
{
	return d->usage;
}

void CPUUsage::check()
{
	float cpu_time = (clock()-d->oldclock)/(float)CLOCKS_PER_SEC;
	float real_time = d->b.stop();

	if(cpu_time > 0 && real_time > 0) // there may be wraparounds
	{
		d->usage = cpu_time / real_time;

		if(d->usage > 0.95)	  // more than 95%  -> not good! (probably freeze)
			d->stalled++;
		else
			d->stalled=0;

		// ok, cancel synthesis due to cpu overload! brutal method
		if(d->stalled > 15)
			arts_fatal("cpu overload, aborting");
	}

	// prepare for next checkpoint
	d->oldclock = clock(); 
	d->b.start();
}

CPUUsage *CPUUsage::the()
{
	return cpuUsage;
}

}
