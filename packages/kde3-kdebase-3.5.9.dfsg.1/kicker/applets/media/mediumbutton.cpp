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

#include "mediumbutton.h"

#include <qapplication.h>
#include <qclipboard.h>
#include <qpainter.h>
#include <qdrawutil.h>
#include <qpopupmenu.h>
#include <qstyle.h>
#include <qtooltip.h>

#include <kmessagebox.h>
#include <kmimetype.h>
#include <klocale.h>
#include <kdesktopfile.h>
#include <krun.h>
#include <kglobalsettings.h>
#include <kcursor.h>
#include <kapplication.h>
#include <kipc.h>
#include <kiconloader.h>
#include <kurldrag.h>
#include <kpopupmenu.h>

#include <konq_operations.h>
#include <konq_popupmenu.h>
#include <konq_drag.h>

MediumButton::MediumButton(QWidget *parent, const KFileItem &fileItem)
	: PanelPopupButton(parent), mActions(this, this), mFileItem(fileItem)
{
	KAction *a = KStdAction::paste(this, SLOT(slotPaste()),
	                               &mActions, "pasteto");
	a->setShortcut(0);
	a = KStdAction::copy(this, SLOT(slotCopy()), &mActions, "copy");
	a->setShortcut(0);

	resize(20, 20);

	setAcceptDrops(mFileItem.isWritable());
	
	setTitle(mFileItem.text());

	refreshType();

	connect(&mOpenTimer, SIGNAL(timeout()), SLOT(slotDragOpen()));

	// Activate this code only if we find a way to have both an
	// action and a popup menu for the same kicker button
	//connect(this, SIGNAL(clicked()), this, SLOT(slotClicked()));

	setPopup(new QPopupMenu());
}

MediumButton::~MediumButton()
{
	QPopupMenu *menu = popup();
	setPopup(0);
	delete menu;
}

const KFileItem &MediumButton::fileItem() const
{
	return mFileItem;
}

void MediumButton::setFileItem(const KFileItem &fileItem)
{
	mFileItem.assign(fileItem);
	setAcceptDrops(mFileItem.isWritable());
	setTitle(mFileItem.text());
	refreshType();
}

void MediumButton::initPopup()
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
	                                   KURL("media:/"), mActions, 0L,
	                                   this, kpf, bef);
	KPopupTitle *title = new KPopupTitle(new_popup);
	title->setTitle(mFileItem.text());

	new_popup->insertItem(title, -1, 0);

	setPopup(new_popup);

	if (old_popup!=0L) delete old_popup;
}

void MediumButton::refreshType()
{
    KMimeType::Ptr mime = mFileItem.determineMimeType();
    QToolTip::add(this, mime->comment());
    setIcon(mime->icon(QString::null, false));
}

// Activate this code only if we find a way to have both an
// action and a popup menu for the same kicker button
/*
void MediumButton::slotClicked()
{
	mFileItem.run();
}
*/

void MediumButton::slotPaste()
{
	KonqOperations::doPaste(this, mFileItem.url());
}

void MediumButton::slotCopy()
{
	KonqDrag * obj = KonqDrag::newDrag(mFileItem.url(), false);

	QApplication::clipboard()->setData( obj );
}

void MediumButton::dragEnterEvent(QDragEnterEvent* e)
{
	if (mFileItem.isWritable())
	{
		mOpenTimer.start(1000, true);
		e->accept(true);
	}
}

void MediumButton::dragLeaveEvent(QDragLeaveEvent* e)
{
	mOpenTimer.stop();

	PanelPopupButton::dragLeaveEvent( e );
}

void MediumButton::dropEvent(QDropEvent *e)
{
	mOpenTimer.stop();

	KonqOperations::doDrop(&mFileItem, mFileItem.url(), e, this);
}

void MediumButton::slotDragOpen()
{
	mFileItem.run();
}

QString MediumButton::tileName()
{
	return mFileItem.text();
}

void MediumButton::setPanelPosition(KPanelApplet::Position position)
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

#include "mediumbutton.moc"
