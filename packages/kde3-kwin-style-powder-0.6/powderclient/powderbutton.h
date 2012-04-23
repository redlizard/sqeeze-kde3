//
// C++ Interface: powderbutton
//
// Description:
//
//
// Author: Remi Villatel <maxilys@tele2.fr>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef POWDERBUTTON_H
#define POWDERBUTTON_H

#include <kdecoration.h>
#include <qbutton.h>

#include "powderclient.h"
#include "enums.h"

class PowderClient;

class PowderButton : public QButton
{
Q_OBJECT
public:
	PowderButton(PowderClient *parent = 0, const char *name = 0, const QString &tip=NULL, ButtonType type = (ButtonType)0);
	~PowderButton();

	void setTipText(const QString &tip);

	void setOnAllDesktops(bool oad) { isOnAllDesktops = oad; repaint(false); }
	void setMaximized(bool max) { isMaximized = max; repaint(false); }
	void setKeepAbove(bool ka) { isKeptAbove = ka; repaint(false); }
	void setKeepBelow(bool kb) { isKeptBelow = kb; repaint(false); }
	virtual void setOn(bool on);
	QSize sizeHint() const;
	int lastMousePress() const { return m_lastMouse; }
	void reset() { repaint(false); }
	PowderClient* client() { return m_client; }

private:
	void enterEvent(QEvent *e);
	void leaveEvent(QEvent *e);
	void mousePressEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void drawButton(QPainter *painter);

	PowderClient *m_client;
	int m_lastMouse;
	ButtonType m_type;
	bool hover;
	bool isOnAllDesktops, isMaximized;
	bool isKeptAbove, isKeptBelow;
};

#endif
