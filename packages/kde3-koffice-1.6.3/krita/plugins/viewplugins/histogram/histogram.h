/* 
 * histogram.h -- Part of Krita
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

#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <kparts/plugin.h>

class KisView;
class KAction;
class KisImage;

class Histogram : public KParts::Plugin
{
        Q_OBJECT
    public:
        Histogram(QObject *parent, const char *name, const QStringList &);
        virtual ~Histogram();
    
    private slots:
        void slotActivated();
        void slotLayersChanged();

    private:
        KisImage* m_img;
        KisView * m_view;
        KisPainter * m_painter;
        KAction* m_action;

};

#endif // HISTOGRAM_H
