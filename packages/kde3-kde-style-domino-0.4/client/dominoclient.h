/*
	Copyright (C) 2006 Michael Lentner <michaell@gmx.net>
	
	based on KDE2 Default KWin client:
	Copyright (C) 1999, 2001 Daniel Duley <mosfet@kde.org>
	Matthias Ettrich <ettrich@kde.org>
	Karol Szwed <gallium@kde.org>
	
	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public
	License version 2 as published by the Free Software Foundation.
	
	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
	Library General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

#ifndef _DOMINOCLIENT_H
#define _DOMINOCLIENT_H

#include <kcommondecoration.h>
#include <kdecorationfactory.h>
#include <qlayout.h>

namespace Domino {

class DominoClient;
class TitleBar;

class DominoHandler: public QObject, public KDecorationFactory
{
	public:
		DominoHandler();
		~DominoHandler();
				KDecoration* createDecoration( KDecorationBridge* b );
		bool reset( unsigned long changed );
		virtual bool supports( Ability ability );
		
		QColor buttonContourColor;
		QColor buttonMouseOverContourColor;
		QColor buttonPressedContourColor;
		
		bool customButtonColor;
		QColor buttonColor;
		bool customButtonIconColor;
		QColor buttonIconColor;
		bool customBorderColor;
		QColor borderColor;
		bool darkLines;
		
	private:
		unsigned long readConfig( bool update );
		void createPixmaps();
		void freePixmaps();

};


class DominoButton : public KCommonDecorationButton
{
	public:
		DominoButton(ButtonType type, DominoClient *parent, const char *name);
		~DominoButton();

		void reset(unsigned long changed);

	protected:
		void enterEvent(QEvent *);
		void leaveEvent(QEvent *);
		void drawButton(QPainter *p);
		void drawButtonLabel(QPainter*) {;}

		bool isMouseOver;
		DominoClient* client;
};


class DominoClient : public KCommonDecoration
{
	Q_OBJECT
	public:
		DominoClient( KDecorationBridge* b, KDecorationFactory* f );
		~DominoClient();

		virtual QString visibleName() const;
		virtual QString defaultButtonsLeft() const;
		virtual QString defaultButtonsRight() const;
		virtual bool decorationBehaviour(DecorationBehaviour behaviour) const;
		virtual int layoutMetric(LayoutMetric lm, bool respectWindowState = true, const KCommonDecorationButton * = 0) const;
		virtual KCommonDecorationButton *createButton(ButtonType type);
		void borders( int& left, int& right, int& top, int& bottom ) const;
		
		void init();
		void reset( unsigned long changed );
		void resize( const QSize& s );
		void updateMask();
		
		void updateWindowShape();
		void updateCaption();
		void shadeChange();
		void activeChange();
		void iconChange();
		
		TitleBar* titleBar;
		QWidget* borderTopLeft;
		QWidget* borderTopRight;
		QWidget* borderBottomLeft;
		QWidget* borderBottomRight;
		QWidget* borderLeft;
		QWidget* borderRight;
		QWidget* borderBottom;
		QGridLayout* mainlayout;
		QButton* menuButton;
		
		void setPixmaps();
		bool titleBarMouseOver;
		bool globalMouseTracking;
		
	public slots:
		void slotShade();
		void menuButtonDestroyed();

	protected:
		bool eventFilter(QObject*, QEvent*);
		void paintEvent(QPaintEvent*);
		void resizeEvent(QResizeEvent*);
		void showEvent(QShowEvent*);

	private:
		
		void createLayout();
};

class TitleBar : public QWidget
{
	public:
		TitleBar(QWidget* parent=0, const char* name=0) : QWidget(parent, name) {}
		void enterEvent(QEvent * e);
		bool eventFilter(QObject*, QEvent*);
		DominoClient* client;
};



void renderGradient(QPainter *painter, QRect &rect,
	    QColor &c1, QColor &c2, const char* name = 0);
QColor alphaBlendColors(const QColor &backgroundColor, const QColor &foregroundColor, const int alpha);
QImage tintImage(const QImage &img, const QColor &tintColor);
}

#endif
