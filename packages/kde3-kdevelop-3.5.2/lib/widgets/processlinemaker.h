/* This file is part of the KDE project
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>

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

#ifndef _PROCESSLINEMAKER_H_
#define _PROCESSLINEMAKER_H_

#include <qobject.h>

/**
@file processlinemaker.h
Utility objects for process output views.
*/

class KProcess;

/**
Convenience class to catch output of KProcess.
*/
class ProcessLineMaker : public QObject
{
Q_OBJECT

public:
	ProcessLineMaker();
	ProcessLineMaker( const KProcess* );

	void clearBuffers();
	void flush();

public slots:
	void slotReceivedStdout(const char*);
	void slotReceivedStderr(const char*);

protected slots:
	void slotReceivedStdout(KProcess *, char *buffer, int buflen);
	void slotReceivedStderr(KProcess *, char *buffer, int buflen);

signals:
	void receivedStdoutLine( const QCString& line );
        void receivedStderrLine( const QCString& line );
        void receivedPartialStdoutLine( const QCString& line );
        void receivedPartialStderrLine( const QCString& line );

private:
	QCString stdoutbuf;
	QCString stderrbuf;
};

#endif

//kate: replace-tabs off; indent-spaces off;
