/* 
 * colorspaceconversion.h -- Part of Krita
 *
 * Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef COLORSPACECONVERSION_H
#define COLORSPACECONVERSION_H

#include <kparts/plugin.h>

class KisView;

/**
 * Dialog for converting between color models.
 */
class ColorSpaceConversion : public KParts::Plugin
{
    Q_OBJECT
public:
    ColorSpaceConversion(QObject *parent, const char *name, const QStringList &);
    virtual ~ColorSpaceConversion();
    
private slots:

    void slotImgColorSpaceConversion();
    void slotLayerColorSpaceConversion();

private:

    KisView * m_view;
    KisPainter * m_painter;

};

#endif // COLORSPACECONVERSION_H
