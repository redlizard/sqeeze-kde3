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

#include <stdio.h>
#include <kmedia2.h>
#include <connect.h>

using namespace std;
using namespace Arts;

int main(int argc, char **argv)
{
	if(argc != 2)
	{
		fprintf(stderr, "usage: catfile <file>\n");
		exit(1);
	}

	Dispatcher dispatcher;
	FileInputStream file;
	StdoutWriter writer;

	if(!file.open(argv[1]))
	{
		printf("can't open file %s\n",argv[1]);
		exit(1);
	}

	connect(file, writer);

	file.start();
	writer.start();
	
	while(!file.eof())
		dispatcher.ioManager()->processOneEvent(false);
}
