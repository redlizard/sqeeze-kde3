/*
    linkaddressbookwizard.h - Kontact's Link IRC Nick to Addressbook contact Wizard

    This code was shamelessly stolen from kopete's add new contact wizard.

    Copyright (c) 2004 by John Tapsell			 <john@geola.co.uk>

    Copyright (c) 2003 by Will Stephenson        <will@stevello.free-online.co.uk>
    Copyright (c) 2002 by Nick Betcher           <nbetcher@kde.org>
    Copyright (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>

Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

*************************************************************************
*                                                                       *
* This program is free software; you can redistribute it and/or modify  *
* it under the terms of the GNU General Public License as published by  *
* the Free Software Foundation; either version 2 of the License, or     *
* (at your option) any later version.                                   *
*                                                                       *
*************************************************************************
*/

#ifndef LINKADDRESSBOOKUI_H
#define LINKADDRESSBOOKUI_H

#include <qptrlist.h>
#include <qvaluelist.h>
#include <qmap.h>

#include <kdialogbase.h>
#include <kdebug.h>
#include <kabc/addressbook.h>

class QCheckListItem;
class LinkAddressbookUI_Base;

class LinkAddressbookUI : public KDialogBase
{
    Q_OBJECT

        public:
        LinkAddressbookUI( QWidget *parent, const char *name, const QString &ircnick, const QString &servername, const QString &servergroup, const QString &suggested_realname);
        ~LinkAddressbookUI();

    private:
        KABC::AddressBook* m_addressBook;
        QString m_ircnick;
        QString m_lower_ircnick;                  //Same as above, but in lower case, for comparisons.
        QString m_servername;
        QString m_servergroup;
        QString m_suggested_realname;
        LinkAddressbookUI_Base* m_mainWidget;

    protected slots:
        virtual void slotOk();
        virtual void slotCancel();
        void slotAddAddresseeClicked();
        void slotAddresseeListClicked( QListViewItem *addressee );
        /**
         * Utility function, populates the addressee list
         */
        void slotLoadAddressees();

};
#endif

// vim: set noet ts=4 sts=4 sw=4:
