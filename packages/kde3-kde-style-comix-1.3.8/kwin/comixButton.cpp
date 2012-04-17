/***************************************************************************
 *   Copyright (C) 2004 by Jens Luetkens                                   *
 *   j.luetkens@limitland.de                                               *
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

#include <qpainter.h>
#include <qtooltip.h>
#include <qbutton.h>
#include <klocale.h>
#include <kpixmap.h>
#include <kpixmapeffect.h>

#include "comixClient.h"
#include "comixButton.h"

using namespace COMIX;

static unsigned char help_bits[] =			{ 0x18, 0x18, 0x00, 0x1e, 0x10, 0x10, 0x10, 0x10 };
static unsigned char sticky_bits[] =		{ 0x3c, 0x7e, 0xe7, 0xc3, 0xc3, 0xe7, 0x7e, 0x3c };
static unsigned char unsticky_bits[] =		{ 0x3c, 0x7e, 0xff, 0xff, 0xff, 0xff, 0x7e, 0x3c };
static unsigned char iconify_bits[] =		{ 0x00, 0x00, 0x3c, 0x3c, 0x3c, 0x3c, 0x00, 0x00 };
static unsigned char maximize_bits[] =		{ 0xff, 0xff, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xff };
static unsigned char unmaximize_bits[] =	{ 0x00, 0x00, 0xff, 0xff, 0xc3, 0xc3, 0xff, 0xff };
static unsigned char close_bits[] =			{ 0xc3, 0xe7, 0x7e, 0x3c, 0x3c, 0x7e, 0xe7, 0xc3 };
static unsigned char shade_bits[] =			{ 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00 };
static unsigned char unshade_bits[] =		{ 0x00, 0xff, 0xff, 0x81, 0x81, 0x81, 0xff, 0x00 };
static unsigned char above_bits[] =			{ 0x00, 0x18, 0x3c, 0x66, 0xc3, 0xff, 0x00, 0x00 };
static unsigned char unabove_bits[] =		{ 0x00, 0x18, 0x3c, 0x7e, 0xff, 0xff, 0x00, 0x00 };
static unsigned char below_bits[] =			{ 0x00, 0x00, 0xff, 0xc3, 0x66, 0x3c, 0x18, 0x00 };
static unsigned char unbelow_bits[] =		{ 0x00, 0x00, 0xff, 0xff, 0x7e, 0x3c, 0x18, 0x00 };
/*static unsigned char resize_bits[] =		{ 0x18, 0x18, 0x18, 0xff, 0xff, 0x18, 0x18, 0x18 };*/
static unsigned char empty_bits[] =			{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const int buttonSize = 				8;
static const int halfbuttonSize = 			4;

ComixButton::ComixButton(ComixClient * parent, const char *name, const QString& tip, ButtonType type, bool toggle)
		: QButton(parent->widget(), name),
		client_		(parent),
		mouseOver_  (false),
		mouseDown_  (false),
		sunken_		(false),
		lastMouse_	(0)
{
	
	setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
	setBackgroundMode(NoBackground);

    setToggleButton(toggle);

	switch (type) {

		case ButtonHelp:
			bitmap_ = QBitmap(buttonSize, buttonSize, help_bits, true);
			break;

		case ButtonOnAllDesktops:
			if ( isOn() ) {
				bitmap_ = QBitmap(buttonSize, buttonSize, unsticky_bits, true);
			}
			else {
				bitmap_ = QBitmap(buttonSize, buttonSize, sticky_bits, true);
			}
			break;

		case ButtonIconify:
			bitmap_ = QBitmap(buttonSize, buttonSize, iconify_bits, true);
			break;

		case ButtonMaximize:
			if ( isOn() ) {
				bitmap_ = QBitmap(buttonSize, buttonSize, unmaximize_bits, true);
			}
			else {
				bitmap_ = QBitmap(buttonSize, buttonSize, maximize_bits, true);
			}
			break;

		case ButtonClose:
			bitmap_ = QBitmap(buttonSize, buttonSize, close_bits, true);
			break;
		
		case ButtonShade:
			if ( isOn() ) {
				bitmap_ = QBitmap(buttonSize, buttonSize, unshade_bits, true);
			}
			else {
				bitmap_ = QBitmap(buttonSize, buttonSize, shade_bits, true);
			}
			break;

		case ButtonAbove:
			if ( isOn() ) {
				bitmap_ = QBitmap(buttonSize, buttonSize, unabove_bits, true);
			}
			else {
				bitmap_ = QBitmap(buttonSize, buttonSize, above_bits, true);
			}
			break;

		case ButtonBelow:
			if ( isOn() ) {
				bitmap_ = QBitmap(buttonSize, buttonSize, unbelow_bits, true);
			}
			else {
				bitmap_ = QBitmap(buttonSize, buttonSize, below_bits, true);
			}
			break;

		/*
		case ButtonResize:
			bitmap_ = QBitmap(buttonSize, buttonSize, resize_bits, true);
			resizeButton = true;
			break;
		*/

		default:
			bitmap_ = QBitmap(buttonSize, buttonSize, empty_bits, true);
			break;
	}
	
	bitmap_.setMask(bitmap_);

    QToolTip::add(this, tip);

	repaint();
}

ComixButton::~ComixButton()
{
	// Empty.
}

//
// setOnAllDesktops(bool b)
//
void ComixButton::setOnAllDesktops(bool b)
{
	if ( b ) {
		bitmap_ = QBitmap(buttonSize, buttonSize, unsticky_bits, true);
	}
	else {
		bitmap_ = QBitmap(buttonSize, buttonSize, sticky_bits, true);
	}
	bitmap_.setMask(bitmap_);

	repaint();
}

//
// setMaximized(bool b)
//
void ComixButton::setMaximized(bool b)
{
	if ( b ) {
		bitmap_ = QBitmap(buttonSize, buttonSize, unmaximize_bits, true);
	}
	else {
		bitmap_ = QBitmap(buttonSize, buttonSize, maximize_bits, true);
	}
	bitmap_.setMask(bitmap_);

	repaint();
}

//
// setShaded(bool b)
//
void ComixButton::setShaded(bool b)
{
	if ( b ) {
		bitmap_ = QBitmap(buttonSize, buttonSize, unshade_bits, true);
	}
	else {
		bitmap_ = QBitmap(buttonSize, buttonSize, shade_bits, true);
	}
	bitmap_.setMask(bitmap_);

	repaint();
}

//
// setAbove(bool b)
//
void ComixButton::setAbove(bool b)
{
	if ( b ) {
		bitmap_ = QBitmap(buttonSize, buttonSize, unabove_bits, true);
	}
	else {
		bitmap_ = QBitmap(buttonSize, buttonSize, above_bits, true);
	}
	bitmap_.setMask(bitmap_);

	repaint();
}

//
// setBelow(bool b)
//
void ComixButton::setBelow(bool b)
{
	if ( b ) {
		bitmap_ = QBitmap(buttonSize, buttonSize, unbelow_bits, true);
	}
	else {
		bitmap_ = QBitmap(buttonSize, buttonSize, below_bits, true);
	}
	bitmap_.setMask(bitmap_);

	repaint();
}

//
// setOn(bool on)
//
void ComixButton::setOn(bool on)
{
    QButton::setOn(on);
}

//
// enterEvent(QEvent * e)
//
void ComixButton::enterEvent(QEvent * e)
{
	mouseOver_ = true;
	if( mouseDown_ ) sunken_ = true;
	repaint();
	QButton::enterEvent(e);
}

//
// leaveEvent(QEvent * e)
//
void ComixButton::leaveEvent(QEvent * e)
{
	mouseOver_ = false;
	//sunken_ = false;
	repaint();
	QButton::leaveEvent(e);
}

//
// mousePressEvent(QMouseEvent* e)
//
void ComixButton::mousePressEvent(QMouseEvent* e)
{

    lastMouse_ = e->button();
    // pass on event after changing button to LeftButton
    QMouseEvent me(e->type(), e->pos(), e->globalPos(),
                   LeftButton, e->state());
	
	/*
	if( resizeButton ) {
		// must be resize button
		client_->setResizeMouse( e->globalPos() );
		setCursor( sizeAllCursor );
		resizing_ = true;
	}
	*/
	
	mouseDown_ = true;
	sunken_ = true;
 
    QButton::mousePressEvent(&me);
}

//
// mouseReleaseEvent(QMouseEvent* e)
//
void ComixButton::mouseReleaseEvent(QMouseEvent* e)
{

    lastMouse_ = e->button();
    // pass on event after changing button to LeftButton
    QMouseEvent me(e->type(), e->pos(), e->globalPos(),
                   LeftButton, e->state());

	/*
	if( resizeButton ) {
		// must be resize button
		setCursor( arrowCursor );
		resizing_ = false;
	}
	*/
	
	mouseDown_ = false;
	sunken_ = false;
 
    QButton::mouseReleaseEvent(&me);
}

//
// mouseMoveEvent(QMouseEvent* e)
//
void ComixButton::mouseMoveEvent(QMouseEvent*)
{
}


//
// paintEvent(QPaintEvent *)
//
void ComixButton::paintEvent(QPaintEvent *)
{

	QPainter p(this);
	QRect buttonRect(rect());
	
	bool down = sunken_ || isOn() || isDown();
	
	client_->drawButton(buttonRect, &p, down);

	// Draw bitmap icon.
	QPoint center(buttonRect.center());

	p.setBrush(NoBrush);
	p.setPen(client_->options()->color(KDecoration::ColorFont, mouseOver_));
	p.drawPixmap(center.x() - halfbuttonSize + 1, center.y() - halfbuttonSize + 1, bitmap_);

}

