/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   version 2 as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KWEBDESKTOP_H
#define KWEBDESKTOP_H

#include <qobject.h>
#include <qcstring.h>
#include <kparts/browserextension.h>
#include <khtml_part.h>

namespace KIO { class Job; }

class KWebDesktop : public QObject
{
    Q_OBJECT
public:
    KWebDesktop( QObject* parent, const QCString & imageFile, int width, int height )
        : QObject( parent ),
          m_part( 0 ),
          m_imageFile( imageFile ),
          m_width( width ),
          m_height( height ) {}
    ~KWebDesktop();

    KParts::ReadOnlyPart* createPart( const QString& mimeType );

private slots:
    void slotCompleted();

private:
    KParts::ReadOnlyPart* m_part;
    QCString m_imageFile;
    int m_width;
    int m_height;
};


class KWebDesktopRun : public QObject
{
    Q_OBJECT
public:
    KWebDesktopRun( KWebDesktop* webDesktop, const KURL & url );
    ~KWebDesktopRun() {}

protected slots:
    void slotMimetype( KIO::Job *job, const QString &_type );
    void slotFinished( KIO::Job * job );

private:
    KWebDesktop* m_webDesktop;
    KURL m_url;
};

#endif
