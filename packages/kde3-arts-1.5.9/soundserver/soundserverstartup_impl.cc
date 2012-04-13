    /*

    Copyright (C) 2003 Stefan Westerfeld
                       stefan@space.twc.de

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    */

#include "config.h"
#include "debug.h"
#include "soundserverstartup_impl.h"
#include <sys/time.h>
#include <time.h>
#include <cstdlib>

using namespace Arts;

SoundServerStartup_impl::SoundServerStartup_impl()
{
	locked = false;
	/*
	 * create a sane random seed, pseudo random is not enough for
	 * our purposes
	 */
	timeval tv;
	gettimeofday(&tv, NULL);

	int pid = getpid();
	srand(tv.tv_sec);
	srand(rand() + tv.tv_usec);
	srand(rand() + pid);
}

SoundServerStartup_impl::~SoundServerStartup_impl()
{
	if (locked)
		unlock();
}

void SoundServerStartup_impl::cleanReference()
{
	SoundServerStartup test = Reference("global:Arts_SoundServerStartup");
	if(test.isNull())
		Dispatcher::the()->globalComm().erase("Arts_SoundServerStartup");
}

void SoundServerStartup_impl::lock()
{
	arts_return_if_fail (!locked);

	while (!ObjectManager::the()->addGlobalReference(self(), "Arts_SoundServerStartup"))
	{
		arts_debug ("[artsd: %5d] parallel startup detected: sleeping",getpid());

		/*
		 * if several instances of artsd are started at the same time,
		 * we don't want all of them to retry getting a reference at
		 * exactly the same moment ; thus we sleep for a random amount
		 * of time, rather than for exactly one second, if possible
		 */
#ifdef HAVE_USLEEP
		usleep((rand() % 700000) + 200000);
#else
		sleep(1);
#endif
		cleanReference();
	}
	locked = true;
	arts_debug ("[artsd: %5d] SoundServerStartup --> got lock",getpid());
}

void SoundServerStartup_impl::unlock()
{
	arts_return_if_fail (locked);

	arts_debug ("[artsd: %5d] SoundServerStartup <-- released lock",getpid());
	Dispatcher::the()->globalComm().erase("Arts_SoundServerStartup");
	locked = false;
}

namespace Arts {
#ifndef __SUNPRO_CC
	/* See bottom of simplesoundserver_impl.cc for the reason this is here.  */
REGISTER_IMPLEMENTATION(SoundServerStartup_impl);
#endif
}
