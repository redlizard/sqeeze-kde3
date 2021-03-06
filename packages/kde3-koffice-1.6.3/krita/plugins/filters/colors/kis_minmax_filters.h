/*
 * This file is part of Krita
 *
 * Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_MINMAX_FILTERS_H
#define KIS_MINMAX_FILTERS_H

// TODO: remove that
#define LCMS_HEADER <lcms.h>
// TODO: remove it !

#include "kis_filter.h"

class KisFilterMax : public KisFilter
{
    public:
        KisFilterMax();
    public:
        virtual void process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration*, const QRect&);
        static inline KisID id() { return KisID("maximize", i18n("Maximize Channel")); };
        virtual bool supportsPainting() { return true; }
        virtual bool supportsPreview() { return true; }
        virtual bool supportsIncrementalPainting() { return false; }
        virtual ColorSpaceIndependence colorSpaceIndependence() { return FULLY_INDEPENDENT; };
};

class KisFilterMin : public KisFilter
{
    public:
        KisFilterMin();
    public:
        virtual void process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration*, const QRect&);
        static inline KisID id() { return KisID("minimize", i18n("Minimize Channel")); };
        virtual bool supportsPainting() { return true; }
        virtual bool supportsPreview() { return true; }
        virtual bool supportsIncrementalPainting() { return false; }
        virtual ColorSpaceIndependence colorSpaceIndependence() { return FULLY_INDEPENDENT; };
};

#endif
