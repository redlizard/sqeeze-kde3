//
// C++ Implementation: serenitybutton
//
// Author: Remi Villatel <maxilys@tele2.fr>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <kdecoration.h>
#include <qtooltip.h>
#include <qpainter.h>
#include <qpixmap.h>

#include "enums.h"
#include "serenitybutton.h"
#include "serenityhandler.h"
#include "pixmaps.h"

#include "embeddata.h"

SerenityButton::SerenityButton(SerenityClient *parent, const char *name, const QString& tip, ButtonType type) 
	: QButton(parent->widget(), name), 
		m_client(parent), m_lastMouse(0), m_type(type), 
		hover(false), isOnAllDesktops(false), isMaximized(false)
{
	QToolTip::add(this, tip);
	setCursor(arrowCursor);
	setBackgroundMode(NoBackground);

	int buttonSize = SerenityHandler::buttonSize();
	setFixedSize(buttonSize, buttonSize);
}


SerenityButton::~SerenityButton()
{
}

void SerenityButton::setTipText(const QString &tip)
{
	QToolTip::remove(this);
	QToolTip::add(this, tip);
}

QSize SerenityButton::sizeHint() const
{
	int buttonSize = SerenityHandler::buttonSize();
	return QSize(buttonSize, buttonSize);
}

void SerenityButton::enterEvent(QEvent *e)
{
	hover = true;
	repaint(false);
	QButton::enterEvent(e);
}

void SerenityButton::leaveEvent(QEvent *e)
{
	hover = false;
	repaint(false);
	QButton::enterEvent(e);
}

void SerenityButton::mousePressEvent(QMouseEvent *e)
{
	m_lastMouse = e->button();
	QMouseEvent me(e->type(), e->pos(), e->globalPos(), LeftButton, e->state());
	QButton::mousePressEvent(&me);
}

void SerenityButton::mouseReleaseEvent(QMouseEvent *e)
{
	m_lastMouse = e->button();
	QMouseEvent me(e->type(), e->pos(), e->globalPos(), LeftButton, e->state());
	QButton::mouseReleaseEvent(&me);
}

void SerenityButton::setOn(bool on)
{
        QButton::setOn(on);
}

void SerenityButton::drawButton(QPainter *painter)
{
	if (!SerenityHandler::initialized())
		return;

	bool active = m_client->isActive();
	int buttonSize = SerenityHandler::buttonSize();
	int wide = buttonSize*3/2;
	if (wide & 1)	// Odd size
		wide++;
	//
	int ofX = ( (SerenityHandler::menuWide() ? wide : buttonSize) - 16) / 2;
	int ofY = (buttonSize-16)/2;

	QPixmap pix;
	pix.resize(wide, buttonSize);
	QPainter paint(&pix);

	QPixmap menu_icon;

	// Paint the buttons.
	//
	if (hover && !isDown())
	{
		// Hover
		switch (m_type)
		{
			case ButtonMenu:
			{
				paint.drawPixmap(0, 0, active 
							? *Pixmaps::active_menu_hover 
							: *Pixmaps::inactive_menu_hover);
				if (SerenityHandler::styledMenu() == false)
				{
					menu_icon = m_client->icon().pixmap(QIconSet::Small, QIconSet::Normal);
					paint.drawPixmap(ofX, ofY, menu_icon);
				}
				break;
			}
			case ButtonHelp:
			{ 
				paint.drawPixmap(0, 0, active 
							? *Pixmaps::active_help_hover 
							: *Pixmaps::inactive_help_hover);
				break;
			}
			case ButtonMax:
			{
				if (isMaximized)
				{
					paint.drawPixmap(0, 0, active 
								? *Pixmaps::active_max_sunken 
								: *Pixmaps::inactive_max_sunken);
				}
				else 
				{
					paint.drawPixmap(0, 0, active 
								? *Pixmaps::active_max_hover 
								: *Pixmaps::inactive_max_hover);
				}
				break;
			}
			case ButtonMin:
			{
				paint.drawPixmap(0, 0, active 
							? *Pixmaps::active_min_hover 
							: *Pixmaps::inactive_min_hover);
				break;
			}
			case ButtonClose:
			{
				paint.drawPixmap(0, 0, active 
							? *Pixmaps::active_close_hover 
							: *Pixmaps::inactive_close_hover);
				break;
			}
			case ButtonOnAllDesktops:
			{
				if (isOnAllDesktops)
				{
					paint.drawPixmap(0, 0, active 
								? *Pixmaps::active_sticky_sunken 
								: *Pixmaps::inactive_sticky_sunken);
				}
				else 
				{
					paint.drawPixmap(0, 0, active 
								? *Pixmaps::active_sticky_hover 
								: *Pixmaps::inactive_sticky_hover);
				}
				break;
			}
			case ButtonAbove:
			{
				if ( m_client->keepAbove() )
				{
					paint.drawPixmap(0, 0, active 
								? *Pixmaps::active_above_sunken 
								: *Pixmaps::inactive_above_sunken);
				}
				else 
				{
					paint.drawPixmap(0, 0, active 
								? *Pixmaps::active_above_hover 
								: *Pixmaps::inactive_above_hover);
				}
				break;
			}
			case ButtonBelow:
			{
				if ( m_client->keepBelow() )
				{
					paint.drawPixmap(0, 0, active 
								? *Pixmaps::active_below_sunken 
								: *Pixmaps::inactive_below_sunken);
				}
				else 
				{
					paint.drawPixmap(0, 0, active 
								? *Pixmaps::active_below_hover 
								: *Pixmaps::inactive_below_hover);
				}
				break;
			}
			default:
				break;
		}
	}
	else if (isDown())
	{
		// Sunken
		switch (m_type)
		{
			case ButtonMenu:
			{
				paint.drawPixmap(0, 0, active 
							? *Pixmaps::active_menu_sunken 
							: *Pixmaps::inactive_menu_sunken);
				if (SerenityHandler::styledMenu() == false)
				{
					menu_icon = m_client->icon().pixmap(QIconSet::Small, QIconSet::Normal);
					paint.drawPixmap(ofX, ofY, menu_icon);
				}
				break;
			}
			case ButtonHelp:
			{
				paint.drawPixmap(0, 0, active 
							? *Pixmaps::active_help_sunken 
							: *Pixmaps::inactive_help_sunken);
				break;
			}
			case ButtonMax:
			{
				paint.drawPixmap(0, 0, active 
							? *Pixmaps::active_max_sunken 
							: *Pixmaps::inactive_max_sunken);
				break;
			}
			case ButtonMin:
			{
				paint.drawPixmap(0, 0, active 
							? *Pixmaps::active_min_sunken 
							: *Pixmaps::inactive_min_sunken);
				break;
			}
			case ButtonClose:
			{
				paint.drawPixmap(0, 0, active 
							? *Pixmaps::active_close_sunken 
							: *Pixmaps::inactive_close_sunken);
				break;
			}
			case ButtonOnAllDesktops:
			{
				paint.drawPixmap(0, 0, active 
							? *Pixmaps::active_sticky_sunken 
							: *Pixmaps::inactive_sticky_sunken);
				break;
			}
			case ButtonAbove:
			{
				paint.drawPixmap(0, 0, active 
							? *Pixmaps::active_above_sunken 
							: *Pixmaps::inactive_above_sunken);
				break;
			}
			case ButtonBelow:
			{
				paint.drawPixmap(0, 0, active 
							? *Pixmaps::active_below_sunken 
							: *Pixmaps::inactive_below_sunken);
				break;
			}
			default:
				break;
		}
	}
	else
	{
		// Normal
		switch (m_type)
		{
			case ButtonMenu:
			{
				paint.drawPixmap(0, 0, active 
							? *Pixmaps::active_menu_normal 
							: *Pixmaps::inactive_menu_normal);
				if ( (SerenityHandler::styledMenu() == false) 
				&& (SerenityHandler::hiddenMenu() == false) )
				{
					menu_icon = m_client->icon().pixmap(QIconSet::Small, QIconSet::Normal);
					paint.drawPixmap(ofX, ofY, menu_icon);
				}
				break;
			}
			case ButtonHelp:
			{
				paint.drawPixmap(0, 0, active 
							? *Pixmaps::active_help_normal 
							: *Pixmaps::inactive_help_normal);
				break;
			}
			case ButtonMax:
			{
				if (isMaximized)
				{
					paint.drawPixmap(0, 0, active 
								? *Pixmaps::active_max_sunken 
								: *Pixmaps::inactive_max_sunken);
				}
				else 
				{
					paint.drawPixmap(0, 0, active 
								? *Pixmaps::active_max_normal 
								: *Pixmaps::inactive_max_normal);
				}
				break;
			}
			case ButtonMin:
			{
				paint.drawPixmap(0, 0, active 
							? *Pixmaps::active_min_normal 
							: *Pixmaps::inactive_min_normal);
				break;
			}
			case ButtonClose:
			{
				paint.drawPixmap(0, 0, active 
							? *Pixmaps::active_close_normal 
							: *Pixmaps::inactive_close_normal);
				break;
			}
			case ButtonOnAllDesktops:
			{
				if (isOnAllDesktops)
				{
					paint.drawPixmap(0, 0, active 
								? *Pixmaps::active_sticky_sunken 
								: *Pixmaps::inactive_sticky_sunken);
				}
				else 
				{
					paint.drawPixmap(0, 0, active 
								? *Pixmaps::active_sticky_normal 
								: *Pixmaps::inactive_sticky_normal);
				}
				break;
			}
			case ButtonAbove:
			{
				if (m_client->keepAbove())
				{
					paint.drawPixmap(0, 0, active 
								? *Pixmaps::active_above_sunken 
								: *Pixmaps::inactive_above_sunken);
				}
				else 
				{
					paint.drawPixmap(0, 0, active 
								? *Pixmaps::active_above_normal 
								: *Pixmaps::inactive_above_normal);
				}
				break;
			}
			case ButtonBelow:
			{
				if (m_client->keepBelow())
				{
					paint.drawPixmap(0, 0, active 
								? *Pixmaps::active_below_sunken 
								: *Pixmaps::inactive_below_sunken);
				}
				else 
				{
					paint.drawPixmap(0, 0, active 
								? *Pixmaps::active_below_normal 
								: *Pixmaps::inactive_below_normal);
				}
				break;
			}
			default:
				break;
		}
	}
	paint.end();
	painter->drawPixmap(0, 0, pix);
}
#include "serenitybutton.moc"
