//
// C++ Implementation: serenityclient
//
// Author: Remi Villatel <maxilys@tele2.fr>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <kdeversion.h>
#ifndef KDE_IS_VERSION
#define KDE_IS_VERSION(a, b, c) 0
#endif

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

#include "serenityclient.h"
#include "serenityhandler.h"
#include "serenitybutton.h"
#include "pixmaps.h"
#include "enums.h"


SerenityClient::SerenityClient(KDecorationBridge *bridge, KDecorationFactory *factory)
 : KDecoration(bridge, factory), m_titleBar(0), 
				captionBuffer(QPixmap(0, 0)), 
				captionStyle(-1), 
				captionActive(false)
{
}

SerenityClient::~SerenityClient()
{
	// Delete buttons from memory.
	for (int n=0; n<ButtonTypeCount; n++)
	{
		if (m_button[n]) delete m_button[n];
	}
}

void SerenityClient::init()
{
//
	createMainWidget(WResizeNoErase | WRepaintNoErase);
	widget()->setBackgroundMode(NoBackground);	// To avoid flicker
	widget()->installEventFilter(this);

	// Set up layout
	int borderSize = SerenityHandler::borderSize();
	int buttonSize = SerenityHandler::buttonSize();
	int spacing = SerenityHandler::buttonSpacing();

	mainLayout = new QVBoxLayout(widget());
	//
	QBoxLayout::Direction direction;
	if (QApplication::reverseLayout())
		direction = QBoxLayout::RightToLeft;
	else
		direction = QBoxLayout::LeftToRight;
	//
	m_topSpacer = new QSpacerItem(10, borderSize, QSizePolicy::Expanding, QSizePolicy::Fixed);
	//
	titleLayout = new QBoxLayout(0, direction, 0, 0, 0);
	m_titleBar = new QSpacerItem(buttonSize, buttonSize, QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_leftButtonSpacer = new QSpacerItem(borderSize, buttonSize, QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_rightButtonSpacer = new QSpacerItem(borderSize, buttonSize, QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_decoSpacer = new QSpacerItem(10, borderSize, QSizePolicy::Expanding, QSizePolicy::Fixed);
	//
	midLayout = new QBoxLayout(0, direction, 0, 0, 0);
	//
	m_bottomSpacer = new QSpacerItem(10, borderSize, QSizePolicy::Expanding, QSizePolicy::Fixed);

	for (int n=0; n<ButtonTypeCount; n++) m_button[n] = 0; // Reset buttons

	leftButtonLayout = new QBoxLayout(0, direction, 0, 0, 0);
	leftButtonLayout->setSpacing(SerenityHandler::extraSpacing());
	rightButtonLayout = new QBoxLayout(0, direction, 0, 0, 0);
	rightButtonLayout->setSpacing(SerenityHandler::extraSpacing());
	// Add the left buttons
	leftButtonLayout->addItem(m_leftButtonSpacer); // Left button margin
	addButtons(leftButtonLayout, options()->customButtonPositions() 
					? options()->titleButtonsLeft() 
					: QString(default_left));
	// Add the right buttons
	addButtons(rightButtonLayout, options()->customButtonPositions() 
					? options()->titleButtonsRight() 
					: QString(default_right));
	rightButtonLayout->addItem(m_rightButtonSpacer); // Right button margin

	titleLayout->addLayout(leftButtonLayout); // Contains the left buttons
	titleLayout->addSpacing(spacing);
	titleLayout->addItem(m_titleBar); // The text
	titleLayout->addSpacing(spacing);
	titleLayout->addLayout(rightButtonLayout); // Contains the right buttons

	midLayout->addSpacing(borderSize);
	if (isPreview())
	{
		midLayout->addWidget(new QLabel(i18n("<center><b>Serenity Preview</b></center>"), widget() ), 1);
	}
	else
	{
		midLayout->addItem(new QSpacerItem(0, 0));
	}
	midLayout->addSpacing(borderSize);

	// Layout order
	mainLayout->addItem(m_topSpacer);
	mainLayout->addLayout(titleLayout);
	mainLayout->addItem(m_decoSpacer);
	mainLayout->addLayout(midLayout);
	mainLayout->addItem(m_bottomSpacer);

	/// MXLS: More connections
	connect( this, SIGNAL( keepAboveChanged( bool )), SLOT( keepAboveChange( bool )));
	connect( this, SIGNAL( keepBelowChanged( bool )), SLOT( keepBelowChange( bool )));
}

bool SerenityClient::eventFilter(QObject *o, QEvent *e)
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
#if KDE_IS_VERSION(3, 5, 0)
		case QEvent::Wheel:
			wheelEvent( static_cast< QWheelEvent* >( e ));
			return true;
#endif
		default:
			return false;
	}
}

void SerenityClient::addButtons(QBoxLayout *layout, const QString &s)
{
	int wide = SerenityHandler::buttonSize() * 3/2;
	if (wide & 1)	// Odd size
		wide++;
	//
	if (s.length() > 0)
	{
		for (uint n=0; n<s.length(); n++)
		{
			switch (s[n])
			{
				case 'M': // Menu
					if (!m_button[ButtonMenu])
					{
						m_button[ButtonMenu] = new SerenityButton(this, "menu", i18n("Menu"), ButtonMenu);
						if (SerenityHandler::menuWide()) 
							m_button[ButtonMenu]->setFixedWidth(wide);
						connect(m_button[ButtonMenu], SIGNAL(pressed()), this, SLOT(menuButtonPressed()));
						layout->addWidget(m_button[ButtonMenu], 0, Qt::AlignHCenter | Qt::AlignTop);
					}
					break;
				case 'H': // Help
					if (!m_button[ButtonHelp] && providesContextHelp())
					{
						m_button[ButtonHelp] = new SerenityButton(this, "help", i18n("Help"), ButtonHelp);
						if (SerenityHandler::helpWide()) 
							m_button[ButtonHelp]->setFixedWidth(wide);
						connect(m_button[ButtonHelp], SIGNAL(clicked()), this, SLOT(showContextHelp()));
						layout->addWidget(m_button[ButtonHelp], 0, Qt::AlignHCenter | Qt::AlignTop);
					}
					break;
				case 'I': // Minimize
					if ((!m_button[ButtonMin]) && isMinimizable())
					{
						m_button[ButtonMin] = new SerenityButton(this, "minimize", i18n("Minimize"), ButtonMin);
						if (SerenityHandler::minWide()) 
							m_button[ButtonMin]->setFixedWidth(wide);
						connect(m_button[ButtonMin], SIGNAL(clicked()), this, SLOT(minimize()));
						layout->addWidget(m_button[ButtonMin], 0, Qt::AlignHCenter | Qt::AlignTop);
					}
					break;
				case 'A': // Maximize
					if ((!m_button[ButtonMax]) && isMaximizable())
					{
						m_button[ButtonMax] = new SerenityButton(this, "maximize", (maximizeMode()!=MaximizeRestore)?i18n("Minimize"):i18n("Maximize"), ButtonMax);
						if (SerenityHandler::maxWide()) 
							m_button[ButtonMax]->setFixedWidth(wide);
						connect(m_button[ButtonMax], SIGNAL(clicked()), this, SLOT(maxButtonPressed()));
						layout->addWidget(m_button[ButtonMax], 0, Qt::AlignHCenter | Qt::AlignTop);
					}
					break;
				case 'X': // Close
                  			if ((!m_button[ButtonClose]) && isCloseable())
					{
						m_button[ButtonClose] = new SerenityButton(this, "close", i18n("Close"), ButtonClose);
						if (SerenityHandler::closeWide()) 
							m_button[ButtonClose]->setFixedWidth(wide);
						connect(m_button[ButtonClose], SIGNAL(clicked()), this, SLOT(closeWindow()));
						layout->addWidget(m_button[ButtonClose], 0, Qt::AlignHCenter | Qt::AlignTop);
					}
					break;
				case 'S': // OnAllDesktops
					if (!m_button[ButtonOnAllDesktops])
					{
						m_button[ButtonOnAllDesktops] = new SerenityButton(this, "onAllDesktops", isOnAllDesktops() ? i18n("Not On All Desktops") : i18n("On All Desktops"), ButtonOnAllDesktops);
						if (SerenityHandler::stickyWide()) 
							m_button[ButtonOnAllDesktops]->setFixedWidth(wide);
						m_button[ButtonOnAllDesktops]->setOnAllDesktops(isOnAllDesktops());
						connect(m_button[ButtonOnAllDesktops], SIGNAL(clicked()), this, SLOT(toggleOnAllDesktops()));
						layout->addWidget(m_button[ButtonOnAllDesktops], 0, Qt::AlignHCenter | Qt::AlignTop);
					}
					break;
				case 'F': // Above all others
					if (!m_button[ButtonAbove])
					{
						m_button[ButtonAbove] = new SerenityButton(this, "above", i18n("Keep Above Others"), ButtonAbove);
						if (SerenityHandler::aboveWide()) 
							m_button[ButtonAbove]->setFixedWidth(wide);
						m_button[ButtonAbove]->setKeepAbove(isKeptAbove);
						connect(m_button[ButtonAbove], SIGNAL(clicked()), this, SLOT(aboveButtonPressed()));
						layout->addWidget(m_button[ButtonAbove], 0, Qt::AlignHCenter | Qt::AlignTop);
					}
				break;
				case 'B': // Below all others
					if (!m_button[ButtonBelow])
					{
						m_button[ButtonBelow] = new SerenityButton(this, "below", i18n("Keep Below Others"), ButtonBelow);
						if (SerenityHandler::belowWide()) 
							m_button[ButtonBelow]->setFixedWidth(wide);
						m_button[ButtonBelow]->setKeepBelow(isKeptBelow);
						connect(m_button[ButtonBelow], SIGNAL(clicked()), this, SLOT(belowButtonPressed()));
						layout->addWidget(m_button[ButtonBelow], 0, Qt::AlignHCenter | Qt::AlignTop);
					}
				break;
				case '_': // Spacer
					if ( (n != 0) && (n != (s.length()-1)) )
						layout->addSpacing(SerenityHandler::buttonSpacing() 
								- SerenityHandler::extraSpacing());
			}
		}
	}
}

void SerenityClient::paintEvent(QPaintEvent*)
{
	if (!SerenityHandler::initialized()) return;

	bool active = isActive();

	QPainter p(widget());

	int borderSize = SerenityHandler::borderSize();
	int absWidth = width();
	int absRight = absWidth-1;
	QRect bottomRect(m_bottomSpacer->geometry());
	int absBottom = bottomRect.bottom();
	//
	QRegion mask = QRegion(0, 0, absWidth, absBottom+1);
	bool definitelyFramed;
	if (maximizeMode() != MaximizeFull) 
	{
		definitelyFramed = true;
		// Radius 4
		// Remove top left corner
		mask -= QRegion(0, 0, 4, 1);
		mask -= QRegion(0, 1, 2, 1);
		mask -= QRegion(0, 2, 1, 2);
		// Top right corner
		mask -= QRegion(absRight-3, 0, 4, 1);
		mask -= QRegion(absRight-1, 1, 2, 1);
		mask -= QRegion(absRight, 2, 1, 2);
		// Bottom left corner
		mask -= QRegion(0, absBottom, 4, 1);
		mask -= QRegion(0, absBottom-1, 2, 1);
		mask -= QRegion(0, absBottom-3, 1, 2);
		// Bottom right corner
		mask -= QRegion(absRight-3, absBottom, 4, 1);
		mask -= QRegion(absRight-1, absBottom-1, 2, 1);
		mask -= QRegion(absRight, absBottom-3, 1, 2);
	}
	else
	{
		definitelyFramed = false;
	}
	setMask(mask);

	QColor ground = SerenityHandler::std_ground();
	QColor tbGround = KDecoration::options()->color(KDecoration::ColorTitleBar, active);
	int contrast = SerenityHandler::gradientContrast();
	QColor lightGrad = Pixmaps::brighter(ground, contrast*3);
	QColor darkGrad = Pixmaps::darker(ground, contrast*4);
	QColor tbLightGrad = Pixmaps::brighter(tbGround, contrast*3);
	QColor tbDarkGrad = Pixmaps::darker(tbGround, contrast*4);
	//
	int topGradHeight = borderSize;
	QRect titleRect(m_titleBar->geometry());
	int tbHeight = titleRect.height();
	//
	int thickness = QRect(m_topSpacer->geometry()).height();
	//
	int decoY = thickness;
	int decoHeight = thickness;
	int tbWidth = absWidth - thickness*2;
	int rightX = absRight-thickness+1;
	int bottomY = bottomRect.top();

	int titleStyle = SerenityHandler::titleStyle();
	if (titleStyle == 1)
	{
		decoY += tbHeight;
	}
	else if (titleStyle == 2)
	{
		decoY += tbHeight/2;
		decoHeight += tbHeight/2;
	}
	else if (titleStyle == 3)
	{
		topGradHeight += tbHeight/2;
		decoY += tbHeight/2;
		decoHeight += topGradHeight;
	}

	if (definitelyFramed && SerenityHandler::zenBorder()) 
	{
		QRect frame = QRect(borderSize, 0, tbWidth, tbHeight + borderSize*2);
		QRegion clip = QRegion(frame) - QRegion(titleRect);
		//
		p.setClipRegion(clip);
		if (titleStyle > 0)
		{
			if (titleStyle < 3)
			{
				p.fillRect(borderSize, borderSize, tbWidth, tbHeight, tbGround);
				p.setClipping(false);
				Pixmaps::renderGradient(&p, 
						QRect(0, borderSize, borderSize, tbHeight), 
						tbLightGrad, tbGround, 
						Pixmaps::HorizontalGradient);
				Pixmaps::renderGradient(&p, 
						QRect(rightX, borderSize, borderSize, tbHeight), 
						tbGround, tbDarkGrad, 
						Pixmaps::HorizontalGradient);
				p.setClipRegion(clip);
			}
			p.fillRect(borderSize, decoY, tbWidth, decoHeight, ground);
		}
		else // (titleStyle == 0)
		{
			p.fillRect(borderSize, borderSize, tbWidth, tbHeight+borderSize, 
					ground);
		}
		p.setClipping(false);
		if (titleStyle == 2)
		{
			Pixmaps::renderDiagonalGradient(&p, 
						QRect(0, 0, borderSize, topGradHeight), 
						tbLightGrad, tbLightGrad, tbGround);
			Pixmaps::renderGradient(&p, 
						QRect(borderSize, 0, tbWidth, topGradHeight), 
						tbLightGrad, tbGround, 
						Pixmaps::VerticalGradient);
			Pixmaps::renderDiagonalGradient(&p, 
						QRect(rightX, 0, borderSize, topGradHeight), 
						tbLightGrad, tbGround, tbDarkGrad);
		}
		else
		{
			Pixmaps::renderDiagonalGradient(&p, 
						QRect(0, 0, borderSize, topGradHeight), 
						lightGrad, lightGrad, ground);
			Pixmaps::renderGradient(&p, 
						QRect(borderSize, 0, tbWidth, topGradHeight), 
						lightGrad, ground, 
						Pixmaps::VerticalGradient);
			Pixmaps::renderDiagonalGradient(&p, 
						QRect(rightX, 0, borderSize, topGradHeight), 
						lightGrad, ground, darkGrad);
		}
		// Sides
		int height = bottomY - decoY + 1;
		Pixmaps::renderGradient(&p, 
					QRect(0, decoY, borderSize, height), 
					lightGrad, ground, 
					Pixmaps::HorizontalGradient);
		Pixmaps::renderGradient(&p, 
					QRect(rightX, decoY, borderSize, height), 
					ground, darkGrad, 
					Pixmaps::HorizontalGradient);
		// Bottom
		Pixmaps::renderDiagonalGradient(&p, 
					QRect(0, bottomY, borderSize, borderSize), 
					lightGrad, ground, darkGrad);
		Pixmaps::renderGradient(&p, 
					QRect(borderSize, bottomY, tbWidth, borderSize), 
					ground, darkGrad, 
					Pixmaps::VerticalGradient);
		Pixmaps::renderDiagonalGradient(&p, 
					QRect(rightX, bottomY, borderSize, borderSize), 
					ground, darkGrad, darkGrad);
	}
	else	// No Zen border.
	{
		/// MXLS: Pay attention to the fact that 
		/// topSpacer.height(), decoSpacer.height() 
		/// and bottomSpacer.height()
		/// can be null when the window is maximized.

		QRect frame = QRect(0, 0, absWidth, tbHeight + thickness*2);
		QRegion clip = QRegion(frame) - QRegion(titleRect);
		//
		p.setClipRegion(clip);
		if (titleStyle == 1)
		{
			p.fillRect(0, thickness, absWidth, tbHeight, tbGround);
			if (borderSize == 2)
			{
				p.fillRect(0, 0, absWidth, thickness, tbGround);
				if (!isShade())
				{
					p.fillRect(0, decoY, absWidth, thickness, ground);
					// Bottom
					p.setClipping(false);
					p.fillRect(0, bottomY, absWidth, thickness, ground);
				}
				else
				{
					p.fillRect(0, decoY, absWidth, decoHeight, tbGround);
					// Bottom
					p.setClipping(false);
					p.fillRect(0, bottomY, absWidth, borderSize, tbGround);
				}
			}
			else
			{
				p.fillRect(0, 0, absWidth, thickness, ground);
				p.fillRect(0, decoY, absWidth, thickness, ground);
				// Bottom
				p.setClipping(false);
				p.fillRect(0, bottomY, absWidth, thickness, ground);
			}
		}
		else if (titleStyle == 2)
		{
			p.fillRect(0, 0, absWidth, tbHeight+thickness, tbGround);
			p.fillRect(0, decoY, absWidth, decoHeight, ground);
		}
		else if (titleStyle == 3)
		{
			Pixmaps::renderGradient(&p, 
						QRect(0, thickness-borderSize, 
							absWidth, topGradHeight), 
						lightGrad, ground, 
						Pixmaps::VerticalGradient);
			p.fillRect(0, decoY, absWidth, decoHeight, ground);
		}
		else // (titleStyle == 0)
		{
			p.fillRect(frame, ground);
		}
		p.setClipping(false);
		decoY = frame.bottom();
		// The two side borders
		frame.setCoords(0, decoY, thickness, bottomY);
		p.fillRect(frame, ground);
		frame.setCoords(rightX, decoY, absRight, bottomY);
		p.fillRect(frame, ground);
		// The bottom border
		if (titleStyle != 1)
		{
			frame.setCoords(0, bottomY, absRight, absBottom);
			p.fillRect(frame, ground);
		}
	}
	/// Now we're going to draw the (external) frame, eventually.
	if (definitelyFramed)
	{
		// Round corners
		uniframe(p, 0, 0, absRight, absBottom, 4, ground, active);
	}
	/// Draw title (caption).
	if (titleRect.width() > 0)	// Do we even have to paint the title?
	{
		int buttonStyle = SerenityHandler::buttonStyle();
		QColor captionColor = options()->color(ColorFont, active);
		if ( (captionActive != active)
		|| (captionBuffer.size() != titleRect.size()) 
		|| (captionStyle != buttonStyle) )
		{
			captionBuffer.resize(titleRect.width(), titleRect.height());
			QPainter bufferPainter(&captionBuffer);
			//
			if ((SerenityHandler::titleFrame()) && (buttonStyle != 1))
			{
				// Draw a caption framing
				if (buttonStyle == 0) 
				{
					Pixmaps::renderSurface(&bufferPainter, 
						QRect( 0, 0, titleRect.width(), titleRect.height() ), 
						SerenityHandler::titleNormalColor(active),
						Qt::blue, 0, active);
				}
				else if (buttonStyle == 2)
				{
					Pixmaps::renderFlatArea(&bufferPainter, 
						QRect( 0, 0, titleRect.width(), titleRect.height() ), 
						SerenityHandler::titleNormalColor(active), active);
				}
				else if (buttonStyle == 3)
				{
					captionBuffer.fill(SerenityHandler::titleNormalColor(active));
					Pixmaps::renderCarving(&bufferPainter, 
						QRect( 0, 0, titleRect.width(), titleRect.height() ), 
						SerenityHandler::titleNormalColor(active), 
						active);
				}
			}
			else
			{
				Pixmaps::titleFillGround(&bufferPainter,
							captionBuffer.rect(),
							ground, tbGround,
							titleStyle);
			}
			captionActive = active;
			captionStyle = buttonStyle;
		}
		QPixmap carbon = captionBuffer;
		QPainter cp(&carbon);
		// Draw caption text
		QRect caR = carbon.rect();
		caR.addCoords(1, 1, -1, -1);	// Leave some room around eventually.
		cp.setFont(options()->font(active, false));
		cp.setPen(captionColor);
		cp.drawText( caR, 
			(SerenityHandler::centerTitle() ? AlignHCenter : AlignAuto) | AlignVCenter, 
			reduced(caption(), caR.width(), 
				cp.fontMetrics()).stripWhiteSpace() );
		p.drawPixmap(titleRect.left(), titleRect.top(), carbon);
	}
}

void SerenityClient::uniframe(QPainter& current,
			int left, int top, int right, int bottom,
			int radius,
			QColor ground, bool active)
{
	// Useless, radius is always a constant.
	//if (radius<1) radius=1;
	//if (radius>4) radius=4;

	QColor contour;
	if (SerenityHandler::frameColor() == 2)
	{
		contour = options()->color(ColorFrame, active);
	}
	else if (SerenityHandler::frameColor() == 1)
	{
		contour = SerenityHandler::colorMix(ground, 
					SerenityHandler::std_pen(),
					active ? 184 : 216);
	}
	else	// (SerenityHandler::frameColor() == 0)
	{
		contour = SerenityHandler::colorMix(ground, 
					KDecoration::options()->color(KDecoration::ColorFont, active),
					184);
	}
	QColor aliasedTop = SerenityHandler::colorMix(SerenityHandler::titleGroundTop(active), contour);
	QColor aliasedBottom = SerenityHandler::colorMix(ground, contour);
	
	current.setPen(contour);
	current.drawRect(left, top, right-left+1, bottom-top+1);

	if (radius==4)
	{
		current.drawLine(left+2, top+1, left+3, top+1);
		current.drawLine(left+1, top+2, left+1, top+3);
		//
		current.drawLine(right-2, top+1, right-3, top+1);
		current.drawLine(right-1, top+2, right-1, top+3);
		//
		current.drawLine(left+2, bottom-1, left+3, bottom-1);
		current.drawLine(left+1, bottom-2, left+1, bottom-3);
		//
		current.drawLine(right-2, bottom-1, right-3, bottom-1);
		current.drawLine(right-1, bottom-2, right-1, bottom-3);
		//
		current.setPen(aliasedTop);
		current.drawPoint(left+1, top+4);
		current.drawPoint(left+2, top+2);
		current.drawPoint(left+4, top+1);
		//
		current.drawPoint(right-1, top+4);
		current.drawPoint(right-2, top+2);
		current.drawPoint(right-4, top+1);
		//
		current.setPen(aliasedBottom);
		current.drawPoint(left+1, bottom-4);
		current.drawPoint(left+2, bottom-2);
		current.drawPoint(left+4, bottom-1);
		//
		current.drawPoint(right-1, bottom-4);
		current.drawPoint(right-2, bottom-2);
		current.drawPoint(right-4, bottom-1);
	}
	else	// Radius == 1
	{
		current.setPen(aliasedTop);
		current.drawPoint(left, top);
		current.drawPoint(right, top);
		current.setPen(aliasedBottom);
		current.drawPoint(left, bottom);
		current.drawPoint(right, bottom);
	}
}

QString SerenityClient::reduced(QString str, int width, QFontMetrics fm)
{
	int wst = fm.width(str);
	int i;
	if (wst > width)	// To remove the name of the app.
	{
		i = str.findRev(" - ");
		if (i > 0)
			str = str.left(i);
	}
	else return str;
	//
	wst = fm.width(str);
	if (wst > width)	// To remove "Something: "
	{
		i = str.find(": ");
		if (i > 0)
			str = str.mid(i+2);
	}
	else return str;
	//
	wst = fm.width(str);
	if (wst > width)	// To change "file:///" into "/"
	{
		i = str.find(":///");
		if (i > 0)
			str = str.mid(i+3);
	}
	else return str;
	//
	wst = fm.width(str);
	if (wst > width)	// To remove "protocol://"
	{
		i = str.find("://");
		if (i > 0)
			str = str.mid(i+3);
	}
	else return str;
	//
	wst = fm.width(str);
	if (wst > width)	// To remove "anything:"
	{
		i = str.find(":");
		if (i > 0)
			str = str.mid(i+1);
	}
	else return str;
	//
	wst = fm.width(str);
	if (wst > width)	// To shorten "(anything)"
	{
		i = str.findRev("(");
		if (i > 0)
			str = str.left(i) + "(...)";
	}
	else return str;
	//
	wst = fm.width(str);
	if (wst > width)	// To shorten "[anything]"
	{
		i = str.findRev("[");
		if (i > 0)
			str = str.left(i) + "[...]";
	}
	else return str;
	//
	wst = fm.width(str);
	if (wst > width)	// To remove "(...)"
	{
		i = str.findRev("(");
		if (i > 0)
			str = str.left(i);
	}
	else return str;
	//
	wst = fm.width(str);
	if (wst > width)	// To remove "[...]"
	{
		i = str.findRev("[");
		if (i > 0)
			str = str.left(i);
	}
	//			// To shorten a path
	while ( (fm.width(str) > width) && (str.find("/", 2) > 0) )
	{
		i = str.find("/", 2);
		if (i > 0)
			str = "..." + str.mid(i+1);
	}
	//
	return str;
}


void SerenityClient::showEvent(QShowEvent*)
{
	widget()->update();
}

void SerenityClient::resizeEvent(QResizeEvent*)
{
	if ((widget()->isVisibleToTLW()) && (!widget()->testWFlags(WStaticContents)))
	{
		QRegion region = widget()->rect();
		region.subtract(m_titleBar->geometry());
		widget()->erase(region);
	}
}

void SerenityClient::captionChange()
{
	widget()->repaint(m_titleBar->geometry(), false);
}

void SerenityClient::mouseDoubleClickEvent(QMouseEvent *e)
{
	if (m_titleBar->geometry().contains(e->pos()))
	{
		titlebarDblClickOperation();
	}
}

#if KDE_IS_VERSION(3, 5, 0)
void SerenityClient::wheelEvent(QWheelEvent *e)
{
	if (isSetShade() || titleLayout->geometry().contains( e->pos() ) )
	{
		titlebarMouseWheelOperation( e->delta());
	}
}
#endif

SerenityClient::Position SerenityClient::mousePosition(const QPoint &point) const
{
	const int corner = 21;
	const int borderSize = SerenityHandler::borderSize();
	SerenityClient::Position pos = PositionCenter;

	if (isShade() || !isResizable())
						pos = PositionCenter; 
	else if (point.y() <= borderSize)
	{
		// Inside top frame
		if (point.x() <= corner)	pos = PositionTopLeft;
		else if (point.x() >= (width()-corner))
						pos = PositionTopRight;
		else				pos = PositionTop;
	}
	else if (point.y() >= (height()-borderSize))
	{
		// Inside bottom
		if (point.x() <= corner)	pos = PositionBottomLeft;
		else if (point.x() >= (width()-corner))
						pos = PositionBottomRight;
		else				pos = PositionBottom;
	}
	else if (point.x() <= borderSize)
	{
		// On left frame
		if (point.y() <= corner)	pos = PositionTopLeft;
		else if (point.y() >= (height()-corner))
						pos = PositionBottomLeft;
		else				pos = PositionLeft;
	}
	else if (point.x() >= width()-borderSize)
	{
		// On right frame
		if (point.y() <= corner)	pos = PositionTopRight;
		else if (point.y() >= (height()-corner))
						pos = PositionBottomRight;
		else				pos = PositionRight;
	}
	else
	{
		// Inside the frame
		pos = PositionCenter;
	}
	return pos;
}

void SerenityClient::iconChange()
{
	if (m_button[ButtonMenu])
	{
		m_button[ButtonMenu]->repaint(false);
	}
}

void SerenityClient::activeChange()
{
	// Repaint the buttons when state changes
	for (int n=0; n<ButtonTypeCount; n++)
		if (m_button[n]) m_button[n]->repaint(false);
	widget()->repaint(false);
}

void SerenityClient::maximizeChange()
{
	if (m_button[ButtonMax])
	{
		const bool m = (maximizeMode() != MaximizeRestore);
		m_button[ButtonMax]->setMaximized(m);
		m_button[ButtonMax]->setTipText(m ? i18n("Restore") : i18n("Maximize"));
	}
	widget()->update();	/// MXLS: Gotcha!
}

void SerenityClient::desktopChange()
{
	if (m_button[ButtonOnAllDesktops])
	{
		const bool sticky = isOnAllDesktops();
		m_button[ButtonOnAllDesktops]->setOnAllDesktops(sticky);
		m_button[ButtonOnAllDesktops]->setTipText(sticky 
							? i18n("Not On All Desktops") 
							: i18n("On All Desktops"));
	}
}

void SerenityClient::shadeChange()
{
	if (m_button[ButtonShade])
	{
		const bool shaded = isSetShade();
		m_button[ButtonShade]->setOn(shaded);
		m_button[ButtonShade]->setTipText(shaded 
						? i18n("Unshade") 
		    				: i18n("Shade"));
		m_button[ButtonShade]->repaint(false);
	}
	if (maximizeMode() == MaximizeFull)	/// MXLS: Gotcha!
	{
		maximize(MaximizeRestore);
	}
}

void SerenityClient::keepAboveChange(bool a)
{
	if (m_button[ButtonAbove])
	{
		m_button[ButtonAbove]->setOn(a);
		m_button[ButtonAbove]->repaint(false);
	}
}

void SerenityClient::keepBelowChange(bool b)
{
	if (m_button[ButtonBelow])
	{
		m_button[ButtonBelow]->setOn(b);
		m_button[ButtonBelow]->repaint(false);
	}
}

void SerenityClient::maxButtonPressed()
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

void SerenityClient::aboveButtonPressed()
{
	if (m_button[ButtonAbove])
		setKeepAbove(!keepAbove());
}

void SerenityClient::belowButtonPressed()
{
	if (m_button[ButtonBelow])
		setKeepBelow(!keepBelow());
}

void SerenityClient::menuButtonPressed()
{
	if (m_button[ButtonMenu])
	{
		static QTime* t = 0;
		static SerenityClient* lastClient = 0;
		if (t == 0) 
			t = new QTime;
		bool dbl = (lastClient == this && t->elapsed() <= QApplication::doubleClickInterval());
		lastClient = this;
		t->start();
		
		if (dbl)
		{	// Double Click on menu button...
			closeWindow();
			return;
		}
		else
		{
			QPoint pt(m_button[ButtonMenu]->rect().left(), m_button[ButtonMenu]->rect().bottom());
			showWindowMenu(m_button[ButtonMenu]->mapToGlobal(pt));
			m_button[ButtonMenu]->setDown(false);
		}
	}
}

QSize SerenityClient::minimumSize() const
{
	QString s1 = options()->customButtonPositions() ? options()->titleButtonsLeft() : QString(default_left) ;
	QString s2 = options()->customButtonPositions() ? options()->titleButtonsRight() : QString(default_right) ;
	int i = (s1.length()+s2.length()+3) * SerenityHandler::buttonSize();
	return QSize(i, i/3);
}

void SerenityClient::borders(int& left, int& right, int& top, int& bottom) const
{
	int borderSize = SerenityHandler::borderSize();
	int titleBorderSize = SerenityHandler::titleBorderSize();
	int buttonSize = SerenityHandler::buttonSize();
	if ( (maximizeMode() == MaximizeFull) && (SerenityHandler::maxBorder() == false) )
	{
		borderSize = 0;
		titleBorderSize = 0;
	}
	m_topSpacer->changeSize(10, borderSize, QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_titleBar->changeSize(10, buttonSize, QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_leftButtonSpacer->changeSize(titleBorderSize, buttonSize, QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_rightButtonSpacer->changeSize(titleBorderSize, buttonSize, QSizePolicy::Fixed, QSizePolicy::Fixed);
	//
	if (!isShade())
	{
		m_decoSpacer->changeSize(10, borderSize, QSizePolicy::Expanding, QSizePolicy::Fixed );
		top = borderSize + buttonSize + borderSize;
	}
	else
	{
		m_decoSpacer->changeSize(10, 0, QSizePolicy::Expanding, QSizePolicy::Fixed );
		top = borderSize + buttonSize;
	}
	m_bottomSpacer->changeSize(10, borderSize, QSizePolicy::Expanding, QSizePolicy::Fixed );
	//
	left = right = borderSize;
	bottom = borderSize;
	// Activate updated layout
	widget()->layout()->activate();
}

void SerenityClient::reset(unsigned long) // unsigned long changed
{
	// TODO: Implementation
}

void SerenityClient::resize(const QSize& s)
{
	widget()->resize(s);
}


// Plugin Stuff
extern "C"
{
	KDecorationFactory *create_factory()
	{
		return new SerenityHandler();
	}
}

#include "serenityclient.moc"
