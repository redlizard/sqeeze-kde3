/*  This file is part of the KDE libraries
    Copyright (C) 2002 Simon MacMullen

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef _KWORDCREATOR_H_
#define _KWORDCREATOR_H_ "$Id: kofficecreator.h 508787 2006-02-12 18:28:12Z ingwa $"

#include <kio/thumbcreator.h>
#include <kparts/part.h>
#include "../../lib/kofficecore/KoDocument.h"

class KoDocument;

class KOfficeCreator : public QObject, public ThumbCreator
{
    Q_OBJECT
public:
    KOfficeCreator();
    virtual ~KOfficeCreator();
    virtual bool create(const QString &path, int width, int height, QImage &img);
    virtual Flags flags() const;

protected:
    virtual void timerEvent(QTimerEvent *);

private slots:
    void slotCompleted();

private:
    KoDocument *m_doc;
    bool m_completed;
};

#endif
