/***************************************************************************
*   Copyright (C) 2003 by Sandro Giessl                                   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/

#include <kpixmap.h>
#include <kpixmapeffect.h>

#include <qcolor.h>
#include <qpainter.h>

#include "misc.h"

QColor hsvRelative(QColor& baseColor, int relativeH, int relativeS, int relativeV)
{
    int h, s, v;
    baseColor.hsv(&h, &s, &v);

    h += relativeH;
    s += relativeS;
    v += relativeV;

    if(h < 0) { h = 0; }
    else if(h > 359) { h = 359; }
    if(s < 0) { s = 0; }
    else if(s > 255) { s = 255; }
    if(v < 0) { v = 0; }
    else if(v > 255) { v = 255; }

    QColor c;
    c.setHsv( h, s, v );
    return c;
}
