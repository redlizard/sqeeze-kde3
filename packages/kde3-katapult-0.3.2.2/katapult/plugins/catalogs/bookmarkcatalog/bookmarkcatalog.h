/***************************************************************************
 *   Copyright (C) 2005 by Joe Ferris                                      *
 *   jferris@optimistictech.com                                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef PROGRAMCATALOG_H
#define PROGRAMCATALOG_H

#include <kgenericfactory.h>

#include <qptrlist.h>

#include "cachedcatalog.h"

class QWidget;
class QString;
class KBookmarkManager;
class KBookmarkGroup;
class BookmarkCatalogSettings;

/**
@author Joe Ferris
*/
class BookmarkCatalog : public CachedCatalog
{
	Q_OBJECT
public:
	BookmarkCatalog(QObject *, const char *, const QStringList&);
	virtual ~BookmarkCatalog();
	
	virtual void initialize();
	virtual void readSettings(KConfigBase *);
	virtual void writeSettings(KConfigBase *);
	virtual unsigned int minQueryLen() const;
	virtual QWidget * configure();
	
public slots:
	void minQueryLenChanged(int);
	void toggleMozEnabled(bool);
	void toggleMozAuto(bool);
	void changeMozFile(const QString &);
	QString searchMozDir(QString);
	
private:
	void cacheBookmarkList(KBookmarkGroup);
	void cacheMozillaBookmarks();
	QString detectMozillaFile();
	
	KBookmarkManager *manager;
	int _minQueryLen;
	bool _mozEnabled;
	bool _mozAuto;
	QString _mozFile;
	BookmarkCatalogSettings *settings;
};

#endif
