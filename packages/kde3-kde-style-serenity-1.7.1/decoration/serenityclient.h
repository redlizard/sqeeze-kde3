//
// C++ Interface: serenityclient
//
// Author: Remi Villatel <maxilys@tele2>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SERENITYCLIENT_H
#define SERENITYCLIENT_H

#include <kdeversion.h>
#ifndef KDE_IS_VERSION
#define KDE_IS_VERSION(a, b, c) 0
#endif

#include <kdecoration.h>
#include <kdecorationfactory.h>
#include <qlayout.h>

#include "serenitybutton.h"
#include "enums.h"

class SerenityButton;
class SerenityClient : public KDecoration
{
Q_OBJECT
public:
	SerenityClient(KDecorationBridge* bridge, KDecorationFactory* factory);
	~SerenityClient();

	virtual void init();
	virtual bool eventFilter(QObject* o, QEvent* e);

	virtual QSize minimumSize() const;
	virtual void borders(int& left, int& right, int& top, int& bottom) const;
	
protected:
	virtual void desktopChange();
	virtual void activeChange();
	virtual void captionChange();
	virtual void maximizeChange();
	virtual void iconChange();
	virtual void shadeChange(); /// {};

	virtual void paintEvent(QPaintEvent *e);
	virtual void showEvent(QShowEvent*);

	virtual void resize(const QSize&);
	virtual void reset(unsigned long changed);

private:
	void uniframe(QPainter& current,
			int left, int top, int right, int bottom,
			int radius,
			QColor ground, bool active);

	QString reduced(QString str, int width, QFontMetrics fm);

	void addButtons(QBoxLayout *layout, const QString &buttons);
	void resizeEvent(QResizeEvent *e);
	void mouseDoubleClickEvent(QMouseEvent *e);
#if KDE_IS_VERSION(3, 5, 0)
	void wheelEvent(QWheelEvent *e);
#endif
	Position mousePosition(const QPoint &point) const;
	bool isKeptAbove, isKeptBelow;

	SerenityButton *m_button[ButtonTypeCount];
	QVBoxLayout *mainLayout;
	QBoxLayout *titleLayout, *leftButtonLayout, *rightButtonLayout, 
		*midLayout;
	QSpacerItem *m_topSpacer, *m_titleBar,
		*m_decoSpacer, *m_bottomSpacer,
		*m_leftButtonSpacer, *m_rightButtonSpacer;
	//
	QPixmap captionBuffer;
	int captionStyle;
	bool captionActive;

private slots:
	void maxButtonPressed();
	void aboveButtonPressed();
	void keepAboveChange(bool);
	void belowButtonPressed();
	void keepBelowChange(bool);
	void menuButtonPressed();
};

#endif
