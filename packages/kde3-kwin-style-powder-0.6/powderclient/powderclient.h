//
// C++ Interface: powderclient
//
// Description:
//
//
// Author: Remi Villatel <maxilys@tele2>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef POWDERCLIENT_H
#define POWDERCLIENT_H

#include <kdecoration.h>
#include <kdecorationfactory.h>
#include <qlayout.h>

#include "powderbutton.h"
#include "enums.h"

class PowderButton;
class PowderClient : public KDecoration
{
Q_OBJECT
public:
	PowderClient(KDecorationBridge* bridge, KDecorationFactory* factory);
	~PowderClient();

	virtual void init();
	virtual bool eventFilter(QObject* o, QEvent* e);

	virtual QSize minimumSize() const;
	virtual void borders(int& left, int& right, int& top, int& bottom) const;

	QColor my_bright(QColor rrggbb, int amount);
	QColor my_dark(QColor rrggbb, int amount);
	QColor my_median(QColor first, QColor second);

protected:
	virtual void desktopChange();
	virtual void activeChange();
	virtual void captionChange();
	virtual void maximizeChange();
	virtual void iconChange();
	virtual void shadeChange() {};

	virtual void paintEvent(QPaintEvent *e);
	virtual void showEvent(QShowEvent*);

	virtual void resize(const QSize&);
	virtual void reset(unsigned long changed);

private:
	void uniframe(QPainter& current,
			int left, int top, int right, int bottom,
			int radius,
			QColor inner, QColor outer);

	void addButtons(QBoxLayout *layout, const QString &buttons);
	void resizeEvent(QResizeEvent *e);
	void mouseDoubleClickEvent(QMouseEvent *e);
	Position mousePosition(const QPoint &point) const;
	bool isKeptAbove, isKeptBelow;

	PowderButton *m_button[ButtonTypeCount];
	QGridLayout *mainLayout;
	QVBoxLayout *topLayout;
	QHBoxLayout *titleLayout, *leftButtonLayout, *rightButtonLayout, *midLayout;
	QSpacerItem *m_titleBar,
		*m_leftSpacer, *m_rightSpacer,
		*m_topSpacer, *m_midSpacer, *m_bottomSpacer,
		*m_leftButtonSpacer, *m_rightButtonSpacer,
		*m_leftTitleSpacer, *m_rightTitleSpacer;

private slots:
	void maxButtonPressed();
	void aboveButtonPressed();
	void keepAboveChange(bool);
	void belowButtonPressed();
	void keepBelowChange(bool);
	void menuButtonPressed();
};

#endif
