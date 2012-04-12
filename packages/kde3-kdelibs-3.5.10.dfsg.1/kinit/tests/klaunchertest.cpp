/*
    This file is part of KDE 

    Copyright (C) 1998 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License 
    version 2 as published by the Free Software Foundation.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this library; see the file COPYING. If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <qstring.h>
#include <kapplication.h>
#include <dcopclient.h>
#include <stdio.h>
#include <qvaluelist.h>
#include <kservice.h>


int main(int argc, char *argv[])
{
   KApplication::kdeinitExec("konsole");

   KApplication k(argc, argv, "klaunchertest");
 
   kapp->dcopClient()->registerAs( kapp->name()) ;

#if 0
   QString error;
   QCString dcopService;
   int pid;
   int result = KApplication::startServiceByDesktopName(
		QString::fromLatin1("konsole"), QString::null, &error, &dcopService, &pid );

   printf("Result = %d, error = \"%s\", dcopService = \"%s\", pid = %d\n",
      result, error.ascii(), dcopService.data(), pid);

   result = KApplication::startServiceByDesktopName(
		QString::fromLatin1("konqueror"), QString::null,  &error, &dcopService, &pid );

   printf("Result = %d, error = \"%s\", dcopService = \"%s\", pid = %d\n",
      result, error.ascii(), dcopService.data(), pid);
#endif
}

