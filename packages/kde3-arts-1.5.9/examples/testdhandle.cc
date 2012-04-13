	/*

	Copyright (C) 2000 Stefan Westerfeld
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

	Permission is also granted to link this program with the Qt
	library, treating Qt like a library that normally accompanies the
	operating system kernel, whether or not that is in fact the case.

	*/

#include "gslpp/datahandle.h"
#include "soundserver.h"
#include "stdsynthmodule.h"
#include "artsflow.h"
#include "connect.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <math.h>

using namespace std;
using namespace Arts;

int main(int argc, char **argv)
{
	if(argc<2)
	{
		cerr << "USAGE: " << argv[0] << " [filename]\n";
		exit(1);
	}

	Dispatcher dispatcher;
	SoundServer server = Reference("global:Arts_SoundServer");

	cout << "opening '" << argv[1] << "' locally...\n";

	GSL::WaveFileInfo info(argv[1]);
	if(info.error())
	{
		cerr << "GSL error " << info.error() <<
			" (" << gsl_strerror(info.error()) << ") while loading info.\n";
		exit(1);
	}
	cout << "file contains " << info.waveCount() << " waves:\n";
	for(int i=0; i<info.waveCount(); i++)
	{
		GSL::WaveDescription desc= info.waveDescription(i);
		cout << "  wave " << i << ": " << desc.name() <<
			" (" << desc.channelCount() << " channels):\n";
		for(int ci=0; ci<desc.chunkCount(); ci++)
		{
			GSL::WaveChunkDescription cDesc= desc.chunkDescription(ci);
			cout << "    chunk " << ci << ": "
				 << cDesc.oscillatorFrequency()
				 << "hz, recorded with " << cDesc.mixerFrequency()
				 << ", loop type: " << cDesc.loopType() << "\n";
		}
	}

	cout << "opening '" << argv[1] << "' in soundserver for playback...\n";

	if(server.isNull())
	{
		cerr << "Can't connect to sound server" << endl;
		return 1;
	}

	WaveDataHandle handle = DynamicCast(server.createObject("Arts::WaveDataHandle"));
	if(!handle.load(argv[1]))
	{
		cerr << "server can't load wave file '" << argv[1] << "'" << endl;
		return 1;
	}

	printf("channels = %d\n", handle.channelCount());
	//printf("samplingrate = %f\n", handle.samplingRate());
	printf("length[values] = %ld\n", handle.valueCount());

	DataHandlePlay phandle = handle.createPlayer();

	/*DataHandlePlay phandle =
		DynamicCast(server.createObject("Arts::DataHandlePlay"));

	phandle.handle(handle);
	// .createReversed() .createTranslated(0,100000,40000)
	*/

	Synth_BUS_UPLINK bu =
		DynamicCast(server.createObject("Arts::Synth_BUS_UPLINK"));
	connect(phandle,"outvalue",bu,"left");
	connect(phandle,"outvalue",bu,"right");
	bu.busname("out_soundcard");
	phandle.start();
	bu.start();

	while(!phandle.finished())
		sleep(1);
}
