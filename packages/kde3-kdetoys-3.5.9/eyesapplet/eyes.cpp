/*
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 *  based on keyes (C) 1999 by Jerome Tollet <tollet@magic.fr>
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
 */

#include <math.h>

#include <qpainter.h>
#include <qcursor.h>

#include <klocale.h>
#include <kglobal.h>
#include <kapplication.h>

#include "eyes.h"
#include "eyes.moc"

extern "C"
{
    KDE_EXPORT KPanelApplet* init(QWidget *parent, const QString& configFile)
    {
	KGlobal::locale()->insertCatalogue("keyesapplet");
        EyesApplet *applet = new EyesApplet(configFile, KPanelApplet::Normal, 0, parent, "keyesapplet");
        return applet;
    }
}

EyesApplet::EyesApplet(const QString& configFile, Type t, int actions,
                       QWidget *parent, const char *name)
  : KPanelApplet( configFile, t, actions, parent, name )
{
    startTimer(41); // 24 frames/second ;-)
}

int EyesApplet::widthForHeight(int h) const
{
    return static_cast<int>(1.4 * h); // rectangular shape.
}
int EyesApplet::heightForWidth(int w) const
{
    return static_cast<int>(w / 1.4); // rectangular shape.
}

void EyesApplet::resizeEvent( QResizeEvent*e )
{
    QWidget::resizeEvent(e);
}

void EyesApplet::timerEvent(QTimerEvent*)
{
    // draw pupils
    QPainter p(this);
    drawPupils(&p);
}

void EyesApplet::paintEvent(QPaintEvent* e)
{
    // draw background
    QWidget::paintEvent(e);

    // draw eyes, no pupils
    QPainter p(this);
    p.setPen(QPen(black, 2));
    p.setBrush(QBrush(white));

    int w = width()-1;
    int h = height()-1;

    // left eye
    p.drawEllipse(0, 0, w/2, h);

    // right eye
    p.drawEllipse(w/2, 0, w/2, h);

    // draw pupils
    oldleft = QPoint(-1, -1);
    oldright = QPoint(-1, -1);
    drawPupils(&p);
}

void EyesApplet::drawPupils(QPainter* p)
{
    QPoint pos, mouse, vect;
    double cos_alpha,sin_alpha;

    mouse = mapFromGlobal(QCursor::pos());
    int tmp = QMIN(height(),width())/6;

    // left pupil
    vect.setX(mouse.x() - width()/4);
    vect.setY(mouse.y() - height()/2);

    cos_alpha = vect.x() / sqrt(double(vect.x() * vect.x() + vect.y() * vect.y()));
    sin_alpha = vect.y() / sqrt(double(vect.x() * vect.x() + vect.y() * vect.y()));

    if(vect.x() * vect.x() + vect.y() * vect.y() > (width()/4 - tmp) * (width()/4 - tmp)*
       cos_alpha * cos_alpha+ (height()/2-tmp) * (height()/2-tmp) * sin_alpha * sin_alpha) {
        pos.setX(int((width()/4-tmp) * cos_alpha+width()/4));
        pos.setY(int((height()/2-tmp) * sin_alpha+height()/2));
    }
    else
	pos = mouse;

    if(pos != oldleft) {

        int sizeEye=QMIN(height(),width())/6;

        // draw over old pos
	p->setPen(QPen(NoPen));
	p->setBrush(QBrush(white));
	p->drawEllipse(oldleft.x() - sizeEye/2, oldleft.y() - sizeEye/2, sizeEye, sizeEye);

        // draw left pupil
        p->setPen(QPen(NoPen));
	p->setBrush(QBrush(black));
	p->drawEllipse(pos.x() - sizeEye/2, pos.y() - sizeEye/2, sizeEye, sizeEye);

	oldleft = pos;
    }

    // right pupil
    vect.setX(mouse.x() - 3*width()/4);
    vect.setY(mouse.y() - height()/2);

    cos_alpha = vect.x()/sqrt(double(vect.x()*vect.x()+vect.y()*vect.y()));
    sin_alpha = vect.y()/sqrt(double(vect.x()*vect.x()+vect.y()*vect.y()));

    if(vect.x()*vect.x() + vect.y()*vect.y() > (width()/4-tmp)*(width()/4-tmp)
       *cos_alpha*cos_alpha+(height()/2-tmp)*(height()/2-tmp)*sin_alpha*sin_alpha)
    {
        pos.setX(int((width()/4-tmp)*cos_alpha+3*width()/4));
        pos.setY(int((height()/2-tmp)*sin_alpha+height()/2));
    }
    else
	pos = mouse;

    if(pos != oldright) {

        int sizeEye=QMIN(height(),width())/6;

        // draw over old pos
	p->setPen(QPen(NoPen));
	p->setBrush(QBrush(white));
	p->drawEllipse(oldright.x() - sizeEye/2, oldright.y() - sizeEye/2, sizeEye, sizeEye);

        // draw left pupil
        p->setPen(QPen(NoPen));
	p->setBrush(QBrush(black));
        p->drawEllipse(pos.x() - sizeEye/2, pos.y() - sizeEye/2, sizeEye, sizeEye);

	oldright = pos;
    }
}
