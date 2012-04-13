/*
   Copyright (C) 2004,2005 Oswald Buddenhagen <ossi@kde.org>
   Copyright (C) 2005 Stephan Kulow <coolo@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the Lesser GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the Lesser GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef DMCTL_H
#define DMCTL_H

#include <kapplication.h>

struct SessEnt {
	QString display, from, user, session;
	int vt;
	bool self:1, tty:1;
};

typedef QValueList<SessEnt> SessList;

class DM {

#ifdef Q_WS_X11

public:
	DM();
	~DM();

	bool canShutdown();
	void shutdown( KApplication::ShutdownType shutdownType,
	               KApplication::ShutdownMode shutdownMode,
	               const QString &bootOption = QString::null );

	void setLock( bool on );

	bool isSwitchable();
	int numReserve();
	void startReserve();
	bool localSessions( SessList &list );
	bool switchVT( int vt );
	void lockSwitchVT( int vt );

	bool bootOptions( QStringList &opts, int &dflt, int &curr );

	static QString sess2Str( const SessEnt &se );
	static void sess2Str2( const SessEnt &se, QString &user, QString &loc );

private:
	int fd;

	bool exec( const char *cmd, QCString &ret );
	bool exec( const char *cmd );

	void GDMAuthenticate();

#else // Q_WS_X11

public:
	DM() {}

	bool canShutdown() { return false; }
	void shutdown( KApplication::ShutdownType shutdownType,
	               KApplication::ShutdownMode shutdownMode,
	               const QString &bootOption = QString::null ) {}

	void setLock( bool ) {}

	bool isSwitchable() { return false; }
	int numReserve() { return -1; }
	void startReserve() {}
	bool localSessions( SessList &list ) { return false; }
	void switchVT( int vt ) {}

	bool bootOptions( QStringList &opts, int &dflt, int &curr );

#endif // Q_WS_X11

}; // class DM

#endif // DMCTL_H
