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

#include <stdio.h>
#include <string>
#include <unistd.h>
#include "dispatcher.h"
#include "thread.h"

using namespace Arts;
using namespace std;

class Counter : public Arts::Thread 
{
	string name;
public:
	Counter(const string& name) :name(name) {}

	void run() {
		for(int i = 0;i < 10;i++)
		{
			printf("[%s] %d\n",name.c_str(),i+1);
			sleep(1);
		}
		printf("[%s] terminating.\n", static_cast<Counter *>
			(SystemThreads::the()->getCurrentThread())->name.c_str());
	}
};

int main()
{
	Dispatcher dispatcher;
	Counter c1("counter1"), c2("counter2");

	if(SystemThreads::supported())
		printf("We have a system threads (counters should count parallel).\n");
	else
		printf("No system threads (counters will not count parallel).\n");

	c1.start();
	c2.start();
	c1.waitDone();
	c2.waitDone();
	return 0;
}
