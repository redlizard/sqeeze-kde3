/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Allen Winter <winter@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef KHOLIDAYS_HOLIDAYS_H
#define KHOLIDAYS_HOLIDAYS_H

#include <qstring.h>
#include <qstringlist.h>
#include <qdatetime.h>
#include <qvaluelist.h>

#include <kdepimmacros.h>

struct KHoliday {
  QString text;
  QString shortText;
  int Category;
};

class KDE_EXPORT KHolidays {
  public:
    /**
       Return a list of all available locations which have a holiday definition.
       One of these can then be passed to the constructor for a new KHolidays
       object.
    */
    static QStringList locations();
    /**
       Return the file name for the holiday file of the given location.
    */
    static QString fileForLocation( const QString &location );
    /**
       Return the directory for user-specific holiday files (i.e. somewhere below
       $KDEDIR/share/apps/). Don't automatically create that path by default.
    */
    static QString userPath( bool create = false );
    /** 
       Generate the filename (without the path) for a given region.
    */
    static QString generateFileName( const QString &location );

    KHolidays( const QString& location );
    ~KHolidays();

    /// return the location with which this object was constructed
    QString location() const;

    QValueList<KHoliday> getHolidays( const QDate& );
    
    KDE_DEPRECATED QString shortText( const QDate& );
    KDE_DEPRECATED QString getHoliday( const QDate& );

    enum { WORKDAY, HOLIDAY };
    KDE_DEPRECATED int category( const QDate& );

  private:
    bool parseFile( const QDate& );

    QString mLocation;    // location string used to determine holidays file
    QString mHolidayFile; // name of file containing holiday data
    int mYearLast;        // save off the last year we have seen
};

#endif
