/*
    icqreadaway.h  -  ICQ Protocol Plugin

    Copyright (c) 2003 by Stefan Gehn <metz AT gehn.net>
    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef ICQREADAWAY_H
#define ICQREADAWAY_H

#include <kdebug.h>
#include <kdialogbase.h>

class ICQAccount;
class ICQContact;
class KTextBrowser;
class QVBox;

class ICQReadAway : public KDialogBase
{
	Q_OBJECT

	public:
		ICQReadAway(ICQContact *, QWidget *parent = 0, const char* name = "ICQReadAway");

	private slots:
		void slotFetchAwayMessage();
		void slotAwayMessageChanged();
		void slotCloseClicked();
		void slotUrlClicked(const QString &url);
		void slotMailClicked(const QString&, const QString &address);

	signals:
		void closing();

	private:
		ICQAccount *mAccount;
		ICQContact *mContact;
		QVBox *mMainWidget;
		KTextBrowser *awayMessageBrowser;
};
#endif
// vim: set noet ts=4 sts=4 sw=4:
