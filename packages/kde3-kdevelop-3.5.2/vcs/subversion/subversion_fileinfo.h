/**

  	 Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>

	 This program is free software; you can redistribute it and/or
	 modify it under the terms of the GNU General Public
	 License as published by the Free Software Foundation; either
	 version 2 of the License, or (at your option) any later version.

	 This program is distributed in the hope that it will be useful,
	 but WITHOUT ANY WARRANTY; without even the implied warranty of
	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	 General Public License for more details.

	 You should have received a copy of the GNU General Public License
	 along with this program; see the file COPYING.  If not, write to
	 the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
	 Boston, MA 02111-1307, USA.
	 */

#ifndef SVNFILEINFOPROVIDER_H
#define SVNFILEINFOPROVIDER_H

#include <qmap.h>

#include <kdevversioncontrol.h>
#include <kio/job.h>
#include <dcopclient.h>
#include <dcopobject.h>
#include <subversion_part.h>

/**
Provider for SVN file information

@author Mickael Marchand
*/
class SVNFileInfoProvider : public KDevVCSFileInfoProvider, public DCOPObject/*, virtual public DCOPClient*/
{
    Q_OBJECT
	K_DCOP

public:
    SVNFileInfoProvider( subversionPart *parent, const char *name = 0);
    virtual ~SVNFileInfoProvider();

// -- Sync interface
    virtual const VCSFileInfoMap *status( const QString &dirPath );
// -- These two are used for subversionPart and subversionCore. Of couruse, others can use it.
	const VCSFileInfoMap* statusExt( const QString &dirPath, bool checkRepos, bool fullRecurse, bool getAll, bool noIgnore );
	void slotStatusExt( const QString&, const QString& , int, int, int, int, long int ) ;
	
// -- Async interface for requesting data
    virtual bool requestStatus( const QString &dirPath, void *callerData, bool recursive = true, bool checkRepos = true );

	QString projectDirectory() const;

k_dcop:
	void slotStatus( const QString& , int, int, int, int, long int ) ;

public slots:
	void slotResult( KIO::Job * );

private:
    mutable void *m_savedCallerData;
	mutable QString m_previousDirPath;
	mutable QString m_recursivePreviousDirPath;
	mutable VCSFileInfoMap *m_cachedDirEntries;
	mutable VCSFileInfoMap *m_recursiveDirEntries;
	KIO::SimpleJob *job;
	subversionPart *m_part;
};

#endif
