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

#ifndef KWIN_COMIX_BUTTON_H
#define KWIN_COMIX_BUTTON_H

#include <qbutton.h>
#include <qbitmap.h>

#include "comixClient.h"

namespace COMIX {

class ComixClient;

class ComixButton : public QButton
{
	Q_OBJECT

public:

	enum Position
	{
	    Left, Mid, Right
	};

	ComixButton(ComixClient * parent, const char *name, const QString &tip=NULL, ButtonType type = ButtonHelp, bool toggle = false);
	virtual ~ComixButton();

    int lastMousePress() const { return lastMouse_; }

	void setOnAllDesktops(bool);
	void setMaximized(bool);
	void setShaded(bool);
	void setAbove(bool);
	void setBelow(bool);

    virtual void setOn(bool on);

private:

	void setBitmap(const QBitmap &);

	void enterEvent(QEvent *);
	void leaveEvent(QEvent *);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);

	void paintEvent(QPaintEvent *);
	
	QBitmap			bitmap_;
    ComixClient *	client_;

	bool			mouseOver_;
	bool			mouseDown_;
	bool			sunken_;
    int 			lastMouse_;
	/*
	bool			resizing_;
	bool			resizeButton;
	*/
	
/*
signals:

	void toggleSticky();
	void maximize(int mode);
*/

};

}

#endif
