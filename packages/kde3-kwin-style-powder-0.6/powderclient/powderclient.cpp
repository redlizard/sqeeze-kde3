//
// C++ Implementation: powderclient
//
// Description:
//
//
// Author: Remi Villatel <maxilys@tele2.fr>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <klocale.h>
#include <kpixmap.h>
#include <kpixmapeffect.h>

#include <qapplication.h>
#include <qcursor.h>
#include <qfontmetrics.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qtooltip.h>

#include "powderclient.h"
#include "powderhandler.h"
#include "powderbutton.h"
#include "pixmaps.h"
#include "enums.h"


PowderClient::PowderClient(KDecorationBridge *bridge, KDecorationFactory *factory)
 : KDecoration(bridge, factory), m_titleBar(0)
{
}

PowderClient::~PowderClient()
{
	// Delete buttons from memory.
	for (int n=0; n<ButtonTypeCount; n++)
	{
		if (m_button[n]) delete m_button[n];
	}
}

void PowderClient::init()
{
	createMainWidget(WResizeNoErase | WRepaintNoErase);
	widget()->installEventFilter(this);
	widget()->setBackgroundMode(NoBackground);	// to avoid flicker

	int titleSize = PowderHandler::titleSize();
	int buttonSize = PowderHandler::buttonSize();
	int borderSize = PowderHandler::borderSize();

	// Set up layout
	mainLayout = new QGridLayout(widget(), 4, 1);	// 1 rows of 1 col.
	mainLayout->setResizeMode(QLayout::FreeResize);

	topLayout = new QVBoxLayout();

	m_topSpacer = new QSpacerItem(1, 5, QSizePolicy::Expanding, QSizePolicy::Fixed);
	titleLayout = new QHBoxLayout();
	m_midSpacer = new QSpacerItem(1, 7, QSizePolicy::Expanding, QSizePolicy::Fixed);
	midLayout = new QHBoxLayout();
	m_bottomSpacer = new QSpacerItem(1, borderSize, QSizePolicy::Expanding, QSizePolicy::Fixed);

	topLayout->addLayout(titleLayout);
	topLayout->addItem(m_midSpacer);
	//topLayout->setRowSpacing(0, 5);		// Fixed to 5 pixels.

	m_titleBar = new QSpacerItem(1, titleSize, QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_leftButtonSpacer = new QSpacerItem(5, buttonSize, QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_leftTitleSpacer = new QSpacerItem(3, buttonSize, QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_rightTitleSpacer = new QSpacerItem(5, buttonSize, QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_rightButtonSpacer = new QSpacerItem(5, buttonSize, QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_leftSpacer = new QSpacerItem(borderSize, 1, QSizePolicy::Fixed, QSizePolicy::Expanding);
	m_rightSpacer = new QSpacerItem(borderSize, 1, QSizePolicy::Fixed, QSizePolicy::Expanding);

	for (int n=0; n<ButtonTypeCount; n++) m_button[n] = 0; // reset buttons

	leftButtonLayout = new QHBoxLayout();
	leftButtonLayout->setSpacing(0);
	rightButtonLayout = new QHBoxLayout();
	rightButtonLayout->setSpacing(0);
	// Add the left buttons
	leftButtonLayout->addItem(m_leftButtonSpacer); // Left button margin
	addButtons(leftButtonLayout, options()->customButtonPositions() ? options()->titleButtonsLeft() : QString(default_left));
	leftButtonLayout->addItem(m_leftTitleSpacer); // Left title margin
	// Add the right buttons
	rightButtonLayout->addItem(m_rightTitleSpacer); // Right title margin
	addButtons(rightButtonLayout, options()->customButtonPositions() ? options()->titleButtonsRight() : QString(default_right));
	rightButtonLayout->addItem(m_rightButtonSpacer); // Right button margin

	titleLayout->addLayout(leftButtonLayout); // Contains the left buttons
	titleLayout->addItem(m_titleBar); // The text
	titleLayout->addLayout(rightButtonLayout); // Contains the right buttons

	midLayout->addItem(m_leftSpacer);
	if (isPreview())
	{
		midLayout->addWidget(new QLabel(i18n("<center><b>Powder Preview</b></center>"), widget() ), 1 );
	}
	else
	{
		midLayout->addItem(new QSpacerItem(0, 0));
	}
	midLayout->addItem(m_rightSpacer);

	// Layout order // MXLS
	mainLayout->addItem(m_topSpacer, 0, 0);
	mainLayout->setRowSpacing(1, 5);		// Fixed to 5 pixels.
	mainLayout->addLayout(topLayout, 1, 0);
	mainLayout->addLayout(midLayout, 2, 0);
	mainLayout->addItem(m_bottomSpacer, 3, 0);
	mainLayout->setRowStretch(2, 1);		// Only 3rd row will stretch.

	// MXLS: More connections
	connect( this, SIGNAL( keepAboveChanged( bool )), SLOT( keepAboveChange( bool )));
	connect( this, SIGNAL( keepBelowChanged( bool )), SLOT( keepBelowChange( bool )));
}

bool PowderClient::eventFilter(QObject *o, QEvent *e)
{
	if (o != widget())
		return false;

	switch (e->type())
	{
		case QEvent::Resize:
			resizeEvent(static_cast<QResizeEvent*>(e));
			return true;
		case QEvent::Paint:
			paintEvent(static_cast<QPaintEvent*>(e));
			return true;
		case QEvent::MouseButtonDblClick:
			mouseDoubleClickEvent(static_cast<QMouseEvent*>(e));
			return true;
		case QEvent::MouseButtonPress:
			processMousePressEvent(static_cast<QMouseEvent*>(e));
			return true;
		case QEvent::Show:
			showEvent(static_cast<QShowEvent*>(e));
			return true;
		default:
			return false;
	}
}

void PowderClient::addButtons(QBoxLayout *layout, const QString &s)
{
	if (s.length() > 0)
	{
		for (unsigned n=0; n<s.length(); n++)
		{
			switch (s[n])
			{
				case 'M': // Menu
					if (!m_button[ButtonMenu])
					{
						m_button[ButtonMenu] = new PowderButton(this, "menu", i18n("Menu"), ButtonMenu);
						connect(m_button[ButtonMenu], SIGNAL(pressed()), this, SLOT(menuButtonPressed()));
						layout->addWidget(m_button[ButtonMenu], 0, Qt::AlignHCenter | Qt::AlignTop);
					}
					break;
				case 'H': // Help
					if (!m_button[ButtonHelp] && providesContextHelp())
					{
						m_button[ButtonHelp] = new PowderButton(this, "help", i18n("Help"), ButtonHelp);
						connect(m_button[ButtonHelp], SIGNAL(clicked()), this, SLOT(showContextHelp()));
						layout->addWidget(m_button[ButtonHelp], 0, Qt::AlignHCenter | Qt::AlignTop);
					}
					break;
				case 'I': // Minimize
					if ((!m_button[ButtonMin]) && isMinimizable())
					{
						m_button[ButtonMin] = new PowderButton(this, "minimize", i18n("Minimize"), ButtonMin);
						connect(m_button[ButtonMin], SIGNAL(clicked()), this, SLOT(minimize()));
						layout->addWidget(m_button[ButtonMin], 0, Qt::AlignHCenter | Qt::AlignTop);
					}
					break;
				case 'A': // Maximize
					if ((!m_button[ButtonMax]) && isMaximizable())
					{
						m_button[ButtonMax] = new PowderButton(this, "maximize", (maximizeMode()!=MaximizeRestore)?i18n("Minimize"):i18n("Maximize"), ButtonMax);
						connect(m_button[ButtonMax], SIGNAL(clicked()), this, SLOT(maxButtonPressed()));
						layout->addWidget(m_button[ButtonMax], 0, Qt::AlignHCenter | Qt::AlignTop);
					}
					break;
				case 'X': // Close
                  			if ((!m_button[ButtonClose]) && isCloseable())
					{
						m_button[ButtonClose] = new PowderButton(this, "close", i18n("Close"), ButtonClose);
						connect(m_button[ButtonClose], SIGNAL(clicked()), this, SLOT(closeWindow()));
						layout->addWidget(m_button[ButtonClose], 0, Qt::AlignHCenter | Qt::AlignTop);
					}
					break;
				case 'S': // OnAllDesktops
					if (!m_button[ButtonOnAllDesktops])
					{
						m_button[ButtonOnAllDesktops] = new PowderButton(this, "onAllDesktops", isOnAllDesktops() ? i18n("Not On All Desktops") : i18n("On All Desktops"), ButtonOnAllDesktops);
						m_button[ButtonOnAllDesktops]->setOnAllDesktops(isOnAllDesktops());
						connect(m_button[ButtonOnAllDesktops], SIGNAL(clicked()), this, SLOT(toggleOnAllDesktops()));
						layout->addWidget(m_button[ButtonOnAllDesktops], 0, Qt::AlignHCenter | Qt::AlignTop);
					}
					break;
				case 'F': // Above all others
					if (!m_button[ButtonAbove])
					{
						m_button[ButtonAbove] = new PowderButton(this, "above", i18n("Keep Above Others"), ButtonAbove);
						m_button[ButtonAbove]->setKeepAbove(isKeptAbove);
						connect(m_button[ButtonAbove], SIGNAL(clicked()), this, SLOT(aboveButtonPressed()));
						layout->addWidget(m_button[ButtonAbove], 0, Qt::AlignHCenter | Qt::AlignTop);
					}
				break;
				case 'B': // Below all others
					if (!m_button[ButtonBelow])
					{
						m_button[ButtonBelow] = new PowderButton(this, "below", i18n("Keep Below Others"), ButtonBelow);
						m_button[ButtonBelow]->setKeepBelow(isKeptBelow);
						connect(m_button[ButtonBelow], SIGNAL(clicked()), this, SLOT(belowButtonPressed()));
						layout->addWidget(m_button[ButtonBelow], 0, Qt::AlignHCenter | Qt::AlignTop);
					}
				break;
				case '_': // Spacer
					layout->addSpacing(PowderHandler::buttonSpacing());
			}
		}
	}
}

void PowderClient::paintEvent(QPaintEvent*)
{
	if (!PowderHandler::initialized()) return;

	bool active = isActive();

	QPainter painter(widget());

	QRect topRect(m_topSpacer->geometry());
	QRect titleRect(m_titleBar->geometry());
	QRect midRect(m_midSpacer->geometry());
	QRect leftRect(m_leftSpacer->geometry());
	QRect rightRect(m_rightSpacer->geometry());
	QRect bottomRect(m_bottomSpacer->geometry());

	QFontMetrics fm(options()->font(active, false));

	int rr = rightRect.right();
	int bb = bottomRect.bottom();
	
	QRegion mask;
	
	if(maximizeMode()!=MaximizeFull || PowderHandler::maxCorner()==true)
	{
		mask = QRegion(0, 0, rr+1, bb+1);

		switch(PowderHandler::cornerRadius())
		{
			case 6:
			{
				// Remove top left corner
				mask -= QRegion(0, 0, 6, 1);
				mask -= QRegion(0, 1, 4, 1);
				mask -= QRegion(0, 2, 3, 1);
				mask -= QRegion(0, 3, 2, 1);
				mask -= QRegion(0, 4, 1, 2);
				// Remove top right corner
				mask -= QRegion(rr-5, 0, 6, 1);
				mask -= QRegion(rr-3, 1, 4, 1);
				mask -= QRegion(rr-2, 2, 3, 1);
				mask -= QRegion(rr-1, 3, 2, 1);
				mask -= QRegion(rr, 4, 1, 2);
				// Remove bottom left corner
				mask -= QRegion(0, bb, 6, 1);
				mask -= QRegion(0, bb-1, 4, 1);
				mask -= QRegion(0, bb-2, 3, 1);
				mask -= QRegion(0, bb-3, 2, 1);
				mask -= QRegion(0, bb-5, 1, 2);
				// Remove bottom right corner
				mask -= QRegion(rr-5, bb, 6, 1);
				mask -= QRegion(rr-3, bb-1, 4, 1);
				mask -= QRegion(rr-2, bb-2, 3, 1);
				mask -= QRegion(rr-1, bb-3, 2, 1);
				mask -= QRegion(rr, bb-5, 1, 2);
			}
			case 5:
			{
				// Remove top left corner
				mask -= QRegion(0, 0, 5, 1);
				mask -= QRegion(0, 1, 3, 1);
				mask -= QRegion(0, 2, 2, 1);
				mask -= QRegion(0, 3, 1, 2);
				// Remove top right corner
				mask -= QRegion(rr-4, 0, 5, 1);
				mask -= QRegion(rr-2, 1, 3, 1);
				mask -= QRegion(rr-1, 2, 2, 1);
				mask -= QRegion(rr, 3, 1, 2);
				// Remove bottom left corner
				mask -= QRegion(0, bb, 5, 1);
				mask -= QRegion(0, bb-1, 3, 1);
				mask -= QRegion(0, bb-2, 2, 1);
				mask -= QRegion(0, bb-4, 1, 2);
				// Remove bottom right corner
				mask -= QRegion(rr-4, bb, 5, 1);
				mask -= QRegion(rr-2, bb-1, 3, 1);
				mask -= QRegion(rr-1, bb-2, 2, 1);
				mask -= QRegion(rr, bb-4, 1, 2);
			}
			case 4:
			{
				// Remove top left corner
				mask -= QRegion(0, 0, 4, 1);
				mask -= QRegion(0, 1, 2, 1);
				mask -= QRegion(0, 2, 1, 2);
				// Remove top right corner
				mask -= QRegion(rr-4, 0, 4, 1);
				mask -= QRegion(rr-1, 1, 2, 1);
				mask -= QRegion(rr, 2, 1, 2);
				// Remove bottom left corner
				mask -= QRegion(0, bb, 4, 1);
				mask -= QRegion(0, bb-1, 2, 1);
				mask -= QRegion(0, bb-3, 1, 2);
				// Remove bottom right corner
				mask -= QRegion(rr-4, bb, 4, 1);
				mask -= QRegion(rr-1, bb-1, 2, 1);
				mask -= QRegion(rr, bb-3, 1, 2);
			}
			case 3:
			{
				// Remove top left corner
				mask -= QRegion(0, 0, 3, 1);
				mask -= QRegion(0, 1, 1, 2);
				// Remove top right corner
				mask -= QRegion(rr-2, 0, 3, 1);
				mask -= QRegion(rr, 1, 1, 2);
				// Remove bottom left corner
				mask -= QRegion(0, bb, 3, 1);
				mask -= QRegion(0, bb-2, 1, 2);
				// Remove bottom right corner
				mask -= QRegion(rr-2, bb, 3, 1);
				mask -= QRegion(rr, bb-2, 1, 2);
			}
			case 2:
			{
				// Remove top left corner
				mask -= QRegion(0, 0, 2, 1);
				mask -= QRegion(0, 1, 1, 1);
				// Remove top right corner
				mask -= QRegion(rr-1, 0, 2, 1);
				mask -= QRegion(rr, 1, 1, 1);
				// Remove bottom left corner
				mask -= QRegion(0, bb, 2, 1);
				mask -= QRegion(0, bb-1, 1, 1);
				// Remove bottom right corner
				mask -= QRegion(rr-1, bb, 2, 1);
				mask -= QRegion(rr, bb-1, 1, 1);
			}
			default:
			{
				// Remove top left corner
				mask -= QRegion(0, 0, 1, 1);
				// Remove top right corner
				mask -= QRegion(rr, 0, 1, 1);
				// Remove bottom left corner
				mask -= QRegion(0, bb, 1, 1);
				// Remove bottom right corner
				mask -= QRegion(rr, bb, 1, 1);
			}
		}
	}
	else
	{
		mask = QRegion(0, 0, rightRect.right()+1, bottomRect.bottom()+1);
	}
	setMask(mask);

	QColor cottl = options()->color(ColorTitleBar, active);
	QColor dummy = qRgb(0, 0, 0);

	QColorGroup widgetGroup;
	widgetGroup = widget()->colorGroup();
	QColor coground = widgetGroup.background();

	QRect frame;

	int yy = titleRect.height()+5+5;
 
	// The whole titlebar (almost)
	frame.setCoords(0, 0, rr, yy);
	painter.fillRect(frame, cottl);
	// The two sides (almost)
	frame.setCoords(0, yy, 7, bb);
	painter.fillRect(frame, cottl);
	frame.setCoords(rr-7, yy, rr, bb);
	painter.fillRect(frame, cottl);
	// The bottom (almost)
	frame.setCoords(0, bb-7, rr, bb);
	painter.fillRect(frame, cottl);

	if (titleRect.width() > 0)	// Do we even have to paint the title?
	{
		QPixmap titlePixmap;
		titlePixmap.resize(titleRect.width(), titleRect.height());
		titlePixmap.fill(cottl);

		int flags =  (PowderHandler::centerTitle() ? AlignHCenter : AlignAuto) | AlignVCenter;

		QPainter pufferPainter(&titlePixmap);
		pufferPainter.setFont(options()->font(active, false));

		// Shift the text 4 px to the right.
		//pufferPainter.translate(4, 0);

		// Draw title text
		pufferPainter.setPen(options()->color(ColorFont, active));
		pufferPainter.drawText(titlePixmap.rect(), flags, caption());

		pufferPainter.end();

		painter.drawPixmap(titleRect.left(), titleRect.top(), titlePixmap);

	}

	// Now we're going to draw the frame...
	int radius = PowderHandler::cornerRadius();

	// External borders
	if (maximizeMode()!=MaximizeFull || PowderHandler::maxCorner()==true)
	{
		uniframe(painter, 0, 0, rr, bb, radius, cottl, dummy);
	}
	else	// Square corners when maximized (on request).
	{
		uniframe(painter, 0, 0, rr, bb, 1, cottl, dummy);
	}
	// Frame effect
	if (!isShade())
	{
		// The inner frame effect
		frame.setCoords(7, yy, rr-7, yy+1);
		painter.fillRect(frame, coground);
		frame.setCoords(7, yy, 9, bb-8);
		painter.fillRect(frame, coground);
		frame.setCoords(rr-9, yy, rr-7, bb-8);
		painter.fillRect(frame, coground);
		frame.setCoords(7, bb-8, rr-7, bb-7);
		painter.fillRect(frame, coground);
		uniframe(painter, 5, yy-2, rr-5, bb-5, radius-1, coground, cottl);
	}
	else	// No room in a shaded window!
	{
		frame.setCoords(5, yy-2, rr-5, bb-3);
		painter.fillRect(frame, cottl);
	}

}

QColor PowderClient::my_bright(QColor rrggbb, int amount)
{
	int zr, zg, zb;
	rrggbb.getRgb(&zr, &zg, &zb);
	int diff = (amount*92)/10;	// Maximal diff: 0x5C
	zr += diff;
	if (zr>255) zr=255;
	zg += diff;
	if (zg>255) zg=255;
	zb += diff;
	if (zb>255) zb=255;
	return QColor(zr, zg, zb);
}

QColor PowderClient::my_dark(QColor rrggbb, int amount)
{
	int zr, zg, zb;
	rrggbb.getRgb(&zr, &zg, &zb);
	int diff = (amount*92)/10;;	// Maximal diff: 0x5C
	zr -= diff;
	if (zr<0) zr=0;
	zg -= diff;
	if (zg<0) zg=0;
	zb -= diff;
	if (zb<0) zb=0;
	return QColor(zr, zg, zb);
}

QColor PowderClient::my_median(QColor first, QColor second)
{
	int r1, g1, b1, r2, g2, b2;
	first.getRgb(&r1, &g1, &b1);
	second.getRgb(&r2, &g2, &b2);
	return QColor((r1+r2)/2, (g1+g2)/2, (b1+b2)/2);
}

void PowderClient::uniframe(QPainter& current,
			int left, int top, int right, int bottom,
			int radius,
			QColor inner, QColor outer)
{
	if (radius<1) radius=1; // Useful!!!
	if (radius>6) radius=6; // This one, just to be sure.

	int contrast = PowderHandler::gradientContrast();
	// Some colors to use.
	QColor bright = my_bright(inner, contrast);
	QColor dark = my_dark(inner, contrast);
	QColor median = my_median(bright, dark);
	//
	QColor smoothen_bright = my_median(bright, outer);
	QColor smoothen_median = my_median(median, outer);
	QColor smoothen_dark = my_median(dark, outer);
	//
	QColor aliased_bright = my_median(bright, inner);
	QColor aliased_median = my_median(median, inner);
	QColor aliased_dark = my_median(dark, inner);

	int lightning = PowderHandler::lightPos();
	//
	QColor topleft, aliased_topleft, smoothen_topleft;
	QColor topright, aliased_topright, smoothen_topright;
	QColor bottomleft, aliased_bottomleft, smoothen_bottomleft;
	QColor bottomright, aliased_bottomright, smoothen_bottomright;
	//
	if (lightning==0)	// From top left.
	{
		topright=median;
		aliased_topright=aliased_median;
		smoothen_topright=smoothen_median;
		bottomleft=median;
		aliased_bottomleft=aliased_median;
		smoothen_bottomleft=smoothen_median;
	}
	else			// From top only.
	{
		topright=bright;
		aliased_topright=aliased_bright;
		smoothen_topright=smoothen_bright;
		bottomleft=dark;
		aliased_bottomleft=aliased_dark;
		smoothen_bottomleft=smoothen_dark;
	}
	//			// Always.
	topleft=bright;
	aliased_topleft=aliased_bright;
	smoothen_topleft=smoothen_bright;
	bottomright=dark;
	aliased_bottomright=aliased_dark;
	smoothen_bottomright=smoothen_dark;

	KPixmap tempPixmap;
	//
	if (lightning==0)	// From top left
	{
		// Top
		tempPixmap.resize(right-left-radius-radius+1, 2);
		KPixmapEffect::gradient(tempPixmap, topleft, topright, KPixmapEffect::HorizontalGradient);
		current.drawPixmap(left+radius, top, tempPixmap);
		// Bottom
		KPixmapEffect::gradient(tempPixmap, bottomleft, bottomright, KPixmapEffect::HorizontalGradient);
		current.drawPixmap(left+radius, bottom-1, tempPixmap);
	}
	else
	{
		// Top
		current.setPen(topleft);
		current.drawLine(left+radius, top, right-radius, top);
		current.drawLine(left+radius, top+1, right-radius, top+1);
		// Bottom
		current.setPen(bottomleft);
		current.drawLine(left+radius, bottom-1, right-radius, bottom-1);
		current.drawLine(left+radius, bottom, right-radius, bottom);
	}
	// Left
	tempPixmap.resize(2, bottom-top-radius-radius+1);
	KPixmapEffect::gradient(tempPixmap, topleft, bottomleft, KPixmapEffect::VerticalGradient);
	current.drawPixmap(left, top+radius, tempPixmap);
	// Right
	if (lightning==0)	// From top left
		KPixmapEffect::gradient(tempPixmap, topright, bottomright, KPixmapEffect::VerticalGradient);
	// Otherwise keep the same pixmap.
	current.drawPixmap(right-1, top+radius, tempPixmap);

	if (radius==6)
	{
		current.setPen(topleft);
		current.drawLine(left+4, top+1, left+5, top+1);
		current.drawLine(left+3, top+2, left+5, top+2);
		current.drawLine(left+2, top+3, left+3, top+3);
		current.drawLine(left+1, top+4, left+2, top+4);
		current.drawLine(left+1, top+5, left+2, top+5);
		current.setPen(topright);
		current.drawLine(right-4, top+1, right-5, top+1);
		current.drawLine(right-3, top+2, right-5, top+2);
		current.drawLine(right-2, top+3, right-3, top+3);
		current.drawLine(right-1, top+4, right-2, top+4);
		current.drawLine(right-1, top+5, right-2, top+5);
		current.setPen(bottomleft);
		current.drawLine(left+4, bottom-1, left+5, bottom-1);
		current.drawLine(left+3, bottom-2, left+5, bottom-2);
		current.drawLine(left+2, bottom-3, left+3, bottom-3);
		current.drawLine(left+1, bottom-4, left+2, bottom-4);
		current.drawLine(left+1, bottom-5, left+2, bottom-5);
		current.setPen(bottomright);
		current.drawLine(right-4, bottom-1, right-5, bottom-1);
		current.drawLine(right-3, bottom-2, right-5, bottom-2);
		current.drawLine(right-2, bottom-3, right-3, bottom-3);
		current.drawLine(right-1, bottom-4, right-2, bottom-4);
		current.drawLine(right-1, bottom-5, right-2, bottom-5);
		//
		current.setPen(aliased_topleft);
		current.drawPoint(left+2, top+6);
		current.drawPoint(left+3, top+4);
		current.drawPoint(left+4, top+3);
		current.drawPoint(left+6, top+2);
		current.setPen(aliased_topright);
		current.drawPoint(right-2, top+6);
		current.drawPoint(right-3, top+4);
		current.drawPoint(right-4, top+3);
		current.drawPoint(right-6, top+2);
		current.setPen(aliased_bottomleft);
		current.drawPoint(left+2, bottom-6);
		current.drawPoint(left+3, bottom-4);
		current.drawPoint(left+4, bottom-3);
		current.drawPoint(left+6, bottom-2);
		current.setPen(aliased_bottomright);
		current.drawPoint(right-2, bottom-6);
		current.drawPoint(right-3, bottom-4);
		current.drawPoint(right-4, bottom-3);
		current.drawPoint(right-6, bottom-2);
		//
		// No smoothen. We can only be out of the window.
	}
	else if (radius==5)
	{
		current.setPen(topleft);
		current.drawLine(left+3, top+1, left+4, top+1);
		current.drawLine(left+2, top+2, left+4, top+2);
		current.drawLine(left+1, top+3, left+2, top+3);
		current.drawLine(left+1, top+4, left+2, top+4);
		current.setPen(topright);
		current.drawLine(right-3, top+1, right-4, top+1);
		current.drawLine(right-2, top+2, right-4, top+2);
		current.drawLine(right-1, top+3, right-2, top+3);
		current.drawLine(right-1, top+4, right-2, top+4);
		current.setPen(bottomleft);
		current.drawLine(left+3, bottom-1, left+4, bottom-1);
		current.drawLine(left+2, bottom-2, left+4, bottom-2);
		current.drawLine(left+1, bottom-3, left+2, bottom-3);
		current.drawLine(left+1, bottom-4, left+2, bottom-4);
		current.setPen(bottomright);
		current.drawLine(right-3, bottom-1, right-4, bottom-1);
		current.drawLine(right-2, bottom-2, right-4, bottom-2);
		current.drawLine(right-1, bottom-3, right-2, bottom-3);
		current.drawLine(right-1, bottom-4, right-2, bottom-4);
		//
		current.setPen(aliased_topleft);
		current.drawPoint(left+2, top+5);
		current.drawPoint(left+3, top+3);
		current.drawPoint(left+5, top+2);
		current.setPen(aliased_topright);
		current.drawPoint(right-2, top+5);
		current.drawPoint(right-3, top+3);
		current.drawPoint(right-5, top+2);
		current.setPen(aliased_bottomleft);
		current.drawPoint(left+2, bottom-5);
		current.drawPoint(left+3, bottom-3);
		current.drawPoint(left+5, bottom-2);
		current.setPen(aliased_bottomright);
		current.drawPoint(right-2, bottom-5);
		current.drawPoint(right-3, bottom-3);
		current.drawPoint(right-5, bottom-2);
		//
		current.setPen(smoothen_topleft);
		current.drawPoint(left+4, top);
		current.drawPoint(left+2, top+1);
		current.drawPoint(left+1, top+2);
		current.drawPoint(left, top+4);
		current.setPen(smoothen_topright);
		current.drawPoint(right-4, top);
		current.drawPoint(right-2, top+1);
		current.drawPoint(right-1, top+2);
		current.drawPoint(right, top+4);
		current.setPen(smoothen_bottomleft);
		current.drawPoint(left+4, bottom);
		current.drawPoint(left+2, bottom-1);
		current.drawPoint(left+1, bottom-2);
		current.drawPoint(left, bottom-4);
		current.setPen(smoothen_bottomright);
		current.drawPoint(right-4, bottom);
		current.drawPoint(right-2, bottom-1);
		current.drawPoint(right-1, bottom-2);
		current.drawPoint(right, bottom-4);
	}
	else if (radius==4)
	{
		current.setPen(topleft);
		current.drawLine(left+2, top+1, left+3, top+1);
		current.drawLine(left+1, top+2, left+3, top+2);
		current.drawLine(left+1, top+3, left+2, top+3);
		current.setPen(topright);
		current.drawLine(right-2, top+1, right-3, top+1);
		current.drawLine(right-1, top+2, right-3, top+2);
		current.drawLine(right-1, top+3, right-2, top+3);
		current.setPen(bottomleft);
		current.drawLine(left+2, bottom-1, left+3, bottom-1);
		current.drawLine(left+1, bottom-2, left+3, bottom-2);
		current.drawLine(left+1, bottom-3, left+2, bottom-3);
		current.setPen(bottomright);
		current.drawLine(right-2, bottom-1, right-3, bottom-1);
		current.drawLine(right-1, bottom-2, right-3, bottom-2);
		current.drawLine(right-1, bottom-3, right-2, bottom-3);
		//
		current.setPen(aliased_topleft);
		current.drawPoint(left+2, top+4);
		current.drawPoint(left+4, top+2);
		current.setPen(aliased_topright);
		current.drawPoint(right-2, top+4);
		current.drawPoint(right-4, top+2);
		current.setPen(aliased_bottomleft);
		current.drawPoint(left+2, bottom-4);
		current.drawPoint(left+4, bottom-2);
		current.setPen(aliased_bottomright);
		current.drawPoint(right-2, bottom-4);
		current.drawPoint(right-4, bottom-2);
		//
		current.setPen(smoothen_topleft);
		current.drawPoint(left+3, top);
		current.drawPoint(left, top+3);
		current.setPen(smoothen_topright);
		current.drawPoint(right-3, top);
		current.drawPoint(right, top+3);
		current.setPen(smoothen_bottomleft);
		current.drawPoint(left+3, bottom);
		current.drawPoint(left, bottom-3);
		current.setPen(smoothen_bottomright);
		current.drawPoint(right-3, bottom);
		current.drawPoint(right, bottom-3);
	}
	else if (radius==3)
	{
		current.setPen(topleft);
		current.drawLine(left+1, top+1, left+2, top+1);
		current.drawLine(left+1, top+2, left+2, top+2);
		current.setPen(topright);
		current.drawLine(right-1, top+1, right-2, top+1);
		current.drawLine(right-1, top+2, right-2, top+2);
		current.setPen(bottomleft);
		current.drawLine(left+1, bottom-1, left+2, bottom-1);
		current.drawLine(left+1, bottom-2, left+2, bottom-2);
		current.setPen(bottomright);
		current.drawLine(right-1, bottom-1, right-2, bottom-1);
		current.drawLine(right-1, bottom-2, right-2, bottom-2);
		//
		current.setPen(aliased_topleft);
		current.drawPoint(left+2, top+3);
		current.drawPoint(left+3, top+2);
		current.setPen(aliased_topright);
		current.drawPoint(right-2, top+3);
		current.drawPoint(right-3, top+2);
		current.setPen(aliased_bottomleft);
		current.drawPoint(left+2, bottom-3);
		current.drawPoint(left+3, bottom-2);
		current.setPen(aliased_bottomright);
		current.drawPoint(right-2, bottom-3);
		current.drawPoint(right-3, bottom-2);
		//
		current.setPen(smoothen_topleft);
		current.drawPoint(left+2, top);
		current.drawPoint(left, top+2);
		current.setPen(smoothen_topright);
		current.drawPoint(right-2, top);
		current.drawPoint(right, top+2);
		current.setPen(smoothen_bottomleft);
		current.drawPoint(left+2, bottom);
		current.drawPoint(left, bottom-2);
		current.setPen(smoothen_bottomright);
		current.drawPoint(right-2, bottom);
		current.drawPoint(right, bottom-2);
	}
	else if (radius==2)
	{
		current.setPen(topleft);
		current.drawPoint(left+1, top+1);
		current.setPen(topright);
		current.drawPoint(right-1, top+1);
		current.setPen(bottomleft);
		current.drawPoint(left+1, bottom-1);
		current.setPen(bottomright);
		current.drawPoint(right-1, bottom-1);
		//
		current.setPen(aliased_topleft);
		current.drawPoint(left+2, top+2);
		current.setPen(aliased_topright);
		current.drawPoint(right-2, top+2);
		current.setPen(aliased_bottomleft);
		current.drawPoint(left+2, bottom-2);
		current.setPen(aliased_bottomright);
		current.drawPoint(right-2, bottom-2);
		//
		current.setPen(smoothen_topleft);
		current.drawPoint(left+1, top);
		current.drawPoint(left, top+1);
		current.setPen(smoothen_topright);
		current.drawPoint(right-1, top);
		current.drawPoint(right, top+1);
		current.setPen(smoothen_bottomleft);
		current.drawPoint(left+1, bottom);
		current.drawPoint(left, bottom-1);
		current.setPen(smoothen_bottomright);
		current.drawPoint(right-1, bottom);
		current.drawPoint(right, bottom-1);
	}
	else
	{
		current.setPen(smoothen_topleft);
		current.drawPoint(left, top);
		current.setPen(smoothen_topright);
		current.drawPoint(right, top);
		current.setPen(smoothen_bottomleft);
		current.drawPoint(left, bottom);
		current.setPen(smoothen_bottomright);
		current.drawPoint(right, bottom);
	}
}

void PowderClient::showEvent(QShowEvent*)
{
	widget()->update();
}

void PowderClient::resizeEvent(QResizeEvent*)
{
	if ((widget()->isVisibleToTLW()) && (!widget()->testWFlags(WStaticContents)))
	{
		QRegion region = widget()->rect();
		region.subtract(m_titleBar->geometry());
		widget()->erase(region);
	}
}

void PowderClient::captionChange()
{
	widget()->repaint(m_titleBar->geometry(), false);
}

void PowderClient::mouseDoubleClickEvent(QMouseEvent *e)
{
	if (m_titleBar->geometry().contains(e->pos()))
	{
		titlebarDblClickOperation();
	}
}

PowderClient::Position PowderClient::mousePosition(const QPoint &point) const
{
	const int corner = 21;
	const int bordersize = PowderHandler::borderSize();
	PowderClient::Position pos = PositionCenter;

	QRect topRect(m_topSpacer->geometry());

	if (isShade() || !isResizable())
						pos=PositionCenter; 
	else if (point.y() <= topRect.height())
	{
		// inside top frame
		if (point.x() <= corner)	pos = PositionTopLeft;
		else if (point.x() >= (width()-corner))
						pos = PositionTopRight;
		else				pos = PositionTop;
	}
	else if (point.y() >= (height()-bordersize))
	{
		// inside handle
		if (point.x() <= corner)	pos = PositionBottomLeft;
		else if (point.x() >= (width()-corner))
						pos = PositionBottomRight;
		else				pos = PositionBottom;
	}
	else if (point.x() <= bordersize)
	{
		// on left frame
		if (point.y() <= corner)	pos = PositionTopLeft;
		else if (point.y() >= (height()-corner))
						pos = PositionBottomLeft;
		else				pos = PositionLeft;
	}
	else if (point.x() >= width()-bordersize)
	{
		// on right frame
		if (point.y() <= corner)	pos = PositionTopRight;
		else if (point.y() >= (height()-corner))
						pos = PositionBottomRight;
		else				pos = PositionRight;
	}
	else
	{
		// inside the frame
		pos = PositionCenter;
	}
	return pos;
}

void PowderClient::iconChange()
{
	if (m_button[ButtonMenu])
	{
		m_button[ButtonMenu]->repaint(false);
	}
}

void PowderClient::activeChange()
{
	// Repaint the buttons when state changes
	for (int n=0; n<ButtonTypeCount; n++)
		if (m_button[n]) m_button[n]->repaint(false);
	widget()->repaint(false);
}

void PowderClient::maximizeChange()
{
	const bool m = (maximizeMode() != MaximizeRestore);
	if (m_button[ButtonMax])
	{
		m_button[ButtonMax]->setMaximized(m);
		m_button[ButtonMax]->setTipText(m ? i18n("Restore") : i18n("Maximize"));
	}
}

void PowderClient::desktopChange()
{
	if (m_button[ButtonOnAllDesktops])
	{
		m_button[ButtonOnAllDesktops]->setOnAllDesktops(isOnAllDesktops());
		m_button[ButtonOnAllDesktops]->setTipText(isOnAllDesktops() ? i18n("Not On All Desktops") : i18n("On All Desktops"));
	}
}

void PowderClient::keepAboveChange(bool a)
{
	if (m_button[ButtonAbove])
	{
		m_button[ButtonAbove]->setOn(a);
		m_button[ButtonAbove]->repaint(false);
	}
}

void PowderClient::keepBelowChange(bool b)
{
	if (m_button[ButtonBelow])
	{
		m_button[ButtonBelow]->setOn(b);
		m_button[ButtonBelow]->repaint(false);
	}
}

void PowderClient::maxButtonPressed()
{
	if (m_button[ButtonMax])
	{
		switch (m_button[ButtonMax]->lastMousePress())
		{
			case MidButton:
				maximize(maximizeMode() ^ MaximizeVertical);
				break;
			case RightButton:
				maximize(maximizeMode() ^ MaximizeHorizontal);
				break;
			default:
				maximize(maximizeMode() == MaximizeFull ? MaximizeRestore : MaximizeFull);
		}
	}
}

void PowderClient::aboveButtonPressed()
{
	if (m_button[ButtonAbove])
		setKeepAbove(!keepAbove());
}

void PowderClient::belowButtonPressed()
{
	if (m_button[ButtonBelow])
		setKeepBelow(!keepBelow());
}

void PowderClient::menuButtonPressed()
{
	if (m_button[ButtonMenu])
	{
		static QTime* t = 0;
		static PowderClient* lastClient = 0;
		if (t == 0) 
			t = new QTime;
		bool dbl = (lastClient == this && t->elapsed() <= QApplication::doubleClickInterval());
		lastClient = this;
		t->start();
		
		if (dbl)
		{	// Double Click on icon, close window
			closeWindow();
			return;
		}
		else
		{
			QPoint pt(m_button[ButtonMenu]->rect().bottomLeft().x(), m_button[ButtonMenu]->rect().bottomLeft().y());
			showWindowMenu(m_button[ButtonMenu]->mapToGlobal(pt));
			m_button[ButtonMenu]->setDown(false);
		}
	}
}

QSize PowderClient::minimumSize() const
{
	QString s1 = options()->customButtonPositions() ? options()->titleButtonsLeft() : QString(default_left) ;
	QString s2 = options()->customButtonPositions() ? options()->titleButtonsRight() : QString(default_right) ;
	int i = (s1.length()+s2.length()+1) * PowderHandler::buttonSize();
	return QSize(i, i/3);
}

void PowderClient::borders(int& left, int& right, int& top, int& bottom) const
{
	m_leftSpacer->changeSize(PowderHandler::borderSize(), 1, QSizePolicy::Fixed, QSizePolicy::Expanding );
	m_rightSpacer->changeSize(PowderHandler::borderSize(), 1, QSizePolicy::Fixed, QSizePolicy::Expanding );
	m_topSpacer->changeSize(1, 5, QSizePolicy::Expanding, QSizePolicy::Fixed );
	//m_midSpacer->changeSize(1, 7, QSizePolicy::Expanding, QSizePolicy::Fixed );
	//m_bottomSpacer->changeSize(1, PowderHandler::borderSize(), QSizePolicy::Expanding, QSizePolicy::Fixed );
	m_leftButtonSpacer->changeSize( 5, PowderHandler::titleSize(), QSizePolicy::Fixed, QSizePolicy::Expanding );
	m_leftTitleSpacer->changeSize( 3, PowderHandler::titleSize(), QSizePolicy::Fixed, QSizePolicy::Expanding );
	m_rightTitleSpacer->changeSize( 3, PowderHandler::titleSize(), QSizePolicy::Fixed, QSizePolicy::Expanding );
	m_rightButtonSpacer->changeSize( 5, PowderHandler::titleSize(), QSizePolicy::Fixed, QSizePolicy::Expanding );
	m_titleBar->changeSize(1, PowderHandler::titleSize(), QSizePolicy::Expanding, QSizePolicy::Fixed);

	if (!isShade())
	{
		m_midSpacer->changeSize(1, 7, QSizePolicy::Expanding, QSizePolicy::Fixed );
		m_bottomSpacer->changeSize(1, PowderHandler::borderSize(), QSizePolicy::Expanding, QSizePolicy::Fixed );
		left = right = bottom = PowderHandler::borderSize();
		top = PowderHandler::titleSize() + 5 + 7;
	}
	else
	{
		m_midSpacer->changeSize(1, 2, QSizePolicy::Expanding, QSizePolicy::Fixed );
		m_bottomSpacer->changeSize(1, 3, QSizePolicy::Expanding, QSizePolicy::Fixed );
		left = right = PowderHandler::borderSize();
		bottom = 3;
		top = PowderHandler::titleSize() + 5 + 2;
	}
	// activate updated layout
	widget()->layout()->activate();
}

void PowderClient::reset(unsigned long) // unsigned long changed
{
	// TODO: Implementation
}

void PowderClient::resize(const QSize& s)
{
	widget()->resize(s);
}


//////////////// Plugin Stuff

extern "C"
{
	KDecorationFactory *create_factory()
	{
		return new PowderHandler();
	}
}

#include "powderclient.moc"
