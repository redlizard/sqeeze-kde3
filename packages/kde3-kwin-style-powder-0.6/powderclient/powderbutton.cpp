//
// C++ Implementation: powderbutton
//
// Description:
//
//
// Author: Remi Villatel <maxilys@tele2.fr>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <kdecoration.h>
#include <qtooltip.h>
#include <qpainter.h>
#include <qpixmap.h>

#include "enums.h"
#include "powderbutton.h"
#include "powderhandler.h"
#include "pixmaps.h"

#include "embeddata.h"

PowderButton::PowderButton(PowderClient *parent, const char *name, const QString& tip, ButtonType type) : QButton(parent->widget(), name), m_client(parent), m_lastMouse(0), m_type(type), hover(false), isOnAllDesktops(false), isMaximized(false)
{
	QToolTip::add(this, tip);
	setCursor(arrowCursor);
	setBackgroundMode(NoBackground);
	// int titleSize = PowderHandler::titleSize();
	// int borderSize = PowderHandler::borderSize();
	int buttonSize = PowderHandler::buttonSize();
	setFixedSize(buttonSize, buttonSize);
}


PowderButton::~PowderButton()
{
}

void PowderButton::setTipText(const QString &tip)
{
	QToolTip::remove(this);
	QToolTip::add(this, tip);
}

QSize PowderButton::sizeHint() const
{
	// int titleSize = PowderHandler::titleSize();
	// int borderSize = PowderHandler::borderSize();
	int buttonSize = PowderHandler::buttonSize();
	return QSize(buttonSize, buttonSize);
}

void PowderButton::enterEvent(QEvent *e)
{
	hover = true;
	repaint(false);
	QButton::enterEvent(e);
}

void PowderButton::leaveEvent(QEvent *e)
{
	hover = false;
	repaint(false);
	QButton::enterEvent(e);
}

void PowderButton::mousePressEvent(QMouseEvent *e)
{
	m_lastMouse = e->button();
	QMouseEvent me(e->type(), e->pos(), e->globalPos(), LeftButton, e->state());
	QButton::mousePressEvent(&me);
}

void PowderButton::mouseReleaseEvent(QMouseEvent *e)
{
	m_lastMouse = e->button();
	QMouseEvent me(e->type(), e->pos(), e->globalPos(), LeftButton, e->state());
	QButton::mouseReleaseEvent(&me);
}

void PowderButton::setOn(bool on)
{
        QButton::setOn(on);
}

void PowderButton::drawButton(QPainter *painter)
{
	if (!PowderHandler::initialized())
		return;

	bool active = m_client->isActive();
	// int titleSize = PowderHandler::titleSize();
	// int borderSize = PowderHandler::borderSize();
	int buttonSize = PowderHandler::buttonSize();

	int ofx = (buttonSize-18)/2;
	int ofy = ofx;

	QColor cottl = KDecoration::options()->color(KDecoration::ColorTitleBar, active);

	QPixmap pufferPixmap;
	pufferPixmap.resize(buttonSize, buttonSize);

	QPainter pufferPainter(&pufferPixmap);
	pufferPixmap.fill(cottl);

	QPixmap menu_icon;

	// Paint the little icon on the buttons.
	// Lotsa trouble with this! Stupid scope!

	if (hover && !isDown())
	{
		// Hover
		if (m_type == ButtonMenu)
		{
			if (PowderHandler::styledMenu()==true)
			{
				pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_menu_hover : *Pixmaps::inactive_menu_hover);
			}
			else
			{
				menu_icon = m_client->icon().pixmap(QIconSet::Small, QIconSet::Normal);
				pufferPainter.drawPixmap(ofx, ofy, menu_icon);
			}
		}
		else if (m_type == ButtonHelp)
			pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_help_hover : *Pixmaps::inactive_help_hover);
		else if (m_type == ButtonMax)
		{
			if (isMaximized)
				pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_restore_hover : *Pixmaps::inactive_restore_hover);
			else
				pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_max_hover : *Pixmaps::inactive_max_hover);
		}
		else if (m_type == ButtonMin)
			pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_min_hover : *Pixmaps::inactive_min_hover);
		else if (m_type == ButtonClose)
			pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_close_hover : *Pixmaps::inactive_close_hover);
		else if (m_type == ButtonOnAllDesktops)
		{
			if (isOnAllDesktops)
				pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_unsticky_hover : *Pixmaps::inactive_unsticky_hover);
			else
				pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_sticky_hover : *Pixmaps::inactive_sticky_hover);
		}
		else if (m_type == ButtonAbove)
		{
			if (m_client->keepAbove())
				pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_unabove_hover : *Pixmaps::inactive_unabove_hover);
			else
				pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_above_hover : *Pixmaps::inactive_above_hover);
		}
		else if (m_type == ButtonBelow)
		{
			if (m_client->keepBelow())
				pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_unbelow_hover : *Pixmaps::inactive_unbelow_hover);
			else
				pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_below_hover : *Pixmaps::inactive_below_hover);
		}
	}
	else if (isDown())
	{
		// Sunken
		if (m_type == ButtonMenu)
		{
			if (PowderHandler::styledMenu()==true)
			{
				pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_menu_sunken : *Pixmaps::inactive_menu_sunken);
			}
			else
			{
				menu_icon = m_client->icon().pixmap(QIconSet::Small, QIconSet::Normal);
				pufferPainter.drawPixmap(ofx, ofy, menu_icon);
			}
		}
		else if (m_type == ButtonHelp)
			pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_help_sunken : *Pixmaps::inactive_help_sunken);
		else if (m_type == ButtonMax)
		{
			if (isMaximized)
				pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_restore_sunken : *Pixmaps::inactive_restore_sunken);
			else
				pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_max_sunken : *Pixmaps::inactive_max_sunken);
		}
		else if (m_type == ButtonMin)
			pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_min_sunken : *Pixmaps::inactive_min_sunken);
		else if (m_type == ButtonClose)
			pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_close_sunken : *Pixmaps::inactive_close_sunken);
		else if (m_type == ButtonOnAllDesktops)
		{
			if (isOnAllDesktops)
				pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_unsticky_sunken : *Pixmaps::inactive_unsticky_sunken);
			else
				pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_sticky_sunken : *Pixmaps::inactive_sticky_sunken);
		}
		else if (m_type == ButtonAbove)
		{
			if (m_client->keepAbove())
				pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_unabove_sunken : *Pixmaps::inactive_unabove_sunken);
			else
				pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_above_sunken : *Pixmaps::inactive_above_sunken);
		}
		else if (m_type == ButtonBelow)
		{
			if (m_client->keepBelow())
				pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_unbelow_sunken : *Pixmaps::inactive_unbelow_sunken);
			else
				pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_below_sunken : *Pixmaps::inactive_below_sunken);
		}
	}
	else
	{
		// Normal
		if (m_type == ButtonMenu)
		{
			if (PowderHandler::styledMenu()==true)
			{
				pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_menu_normal : *Pixmaps::inactive_menu_normal);
			}
			else
			{
				menu_icon = m_client->icon().pixmap(QIconSet::Small, QIconSet::Normal);
				pufferPainter.drawPixmap(ofx, ofy, menu_icon);
			}
		}
		else if (m_type == ButtonHelp)
			pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_help_normal : *Pixmaps::inactive_help_normal);
		else if (m_type == ButtonMax)
		{
			if (isMaximized)
				pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_restore_normal : *Pixmaps::inactive_restore_normal);
			else
				pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_max_normal : *Pixmaps::inactive_max_normal);
		}
		else if (m_type == ButtonMin)
			pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_min_normal : *Pixmaps::inactive_min_normal);
		else if (m_type == ButtonClose)
			pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_close_normal : *Pixmaps::inactive_close_normal);
		else if (m_type == ButtonOnAllDesktops)
		{
			if (isOnAllDesktops)
				pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_unsticky_normal : *Pixmaps::inactive_unsticky_normal);
			else
				pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_sticky_normal : *Pixmaps::inactive_sticky_normal);
		}
		else if (m_type == ButtonAbove)
		{
			if (m_client->keepAbove())
				pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_unabove_normal : *Pixmaps::inactive_unabove_normal);
			else
				pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_above_normal : *Pixmaps::inactive_above_normal);
		}
		else if (m_type == ButtonBelow)
		{
			if (m_client->keepBelow())
				pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_unbelow_normal : *Pixmaps::inactive_unbelow_normal);
			else
				pufferPainter.drawImage(ofx, ofy, active ? *Pixmaps::active_below_normal : *Pixmaps::inactive_below_normal);
		}
	}
	
	pufferPainter.end();
	painter->drawPixmap(0, 0, pufferPixmap);
}
#include "powderbutton.moc"
