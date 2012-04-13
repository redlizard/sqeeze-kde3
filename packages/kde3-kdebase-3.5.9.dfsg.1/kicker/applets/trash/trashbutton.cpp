/* This file is part of the KDE project
   Copyright (c) 2004 Kevin Ottens <ervin ipsquad net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "trashbutton.h"

#include <qpopupmenu.h>
#include <qtooltip.h>

#include <klocale.h>
#include <krun.h>
#include <kpopupmenu.h>

#include <kio/netaccess.h>

#include <konq_operations.h>
#include <konq_popupmenu.h>

TrashButton::TrashButton(QWidget *parent)
	: PanelPopupButton(parent), mActions(this, this),
	  mFileItem(KFileItem::Unknown, KFileItem::Unknown, "trash:/")
{
	KIO::UDSEntry entry;
	KIO::NetAccess::stat("trash:/", entry, 0L);
	mFileItem.assign(KFileItem(entry, "trash:/"));

	KAction *a = KStdAction::paste(this, SLOT(slotPaste()),
	                               &mActions, "paste");
	a->setShortcut(0);

	move(0, 0);
	resize(20, 20);

	setTitle(i18n("Trash"));
	setIcon( "trashcan_empty" );

	setAcceptDrops(true);

	// Activate this code only if we find a way to have both an
	// action and a popup menu for the same kicker button
	//connect(this, SIGNAL(clicked()), this, SLOT(slotClicked()));

	setPopup(new QPopupMenu());
}

TrashButton::~TrashButton()
{
}

void TrashButton::setItemCount(int count)
{
    if (count==0)
    {
        setIcon( "trashcan_empty" );
        QToolTip::add(this, i18n("Empty"));
    }
    else
    {
        setIcon( "trashcan_full" );
        QToolTip::add(this, i18n("One item", "%n items", count));
    }
}

void TrashButton::initPopup()
{
	QPopupMenu *old_popup = popup();

	KFileItemList items;
	items.append(&mFileItem);

	KonqPopupMenu::KonqPopupFlags kpf =
		  KonqPopupMenu::ShowProperties
		| KonqPopupMenu::ShowNewWindow;

	KParts::BrowserExtension::PopupFlags bef =
		  KParts::BrowserExtension::DefaultPopupItems;

	KonqPopupMenu *new_popup = new KonqPopupMenu(0L, items,
	                                   KURL("trash:/"), mActions, 0L,
	                                   this, kpf, bef);
	KPopupTitle *title = new KPopupTitle(new_popup);
	title->setTitle(i18n("Trash"));

	new_popup->insertItem(title, -1, 0);

	setPopup(new_popup);

	if (old_popup!=0L) delete old_popup;
}

// Activate this code only if we find a way to have both an
// action and a popup menu for the same kicker button
/*
void TrashButton::slotClicked()
{
	mFileItem.run();
}
*/

void TrashButton::slotPaste()
{
	KonqOperations::doPaste(this, mFileItem.url());
}

void TrashButton::dragEnterEvent(QDragEnterEvent* e)
{
	e->accept(true);
}

void TrashButton::dropEvent(QDropEvent *e)
{
	KonqOperations::doDrop(0L, mFileItem.url(), e, this);
}

QString TrashButton::tileName()
{
	return mFileItem.name();
}

void TrashButton::setPanelPosition(KPanelApplet::Position position)
{
	switch(position)
	{
	case KPanelApplet::pBottom:
		setPopupDirection(KPanelApplet::Up);
		break;
	case KPanelApplet::pTop:
		setPopupDirection(KPanelApplet::Down);
		break;
	case KPanelApplet::pRight:
		setPopupDirection(KPanelApplet::Left);
		break;
	case KPanelApplet::pLeft:
		setPopupDirection(KPanelApplet::Right);
		break;
	}
}

#include "trashbutton.moc"
