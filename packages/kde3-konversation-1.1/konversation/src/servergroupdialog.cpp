/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

/*
  copyright: (C) 2004 by Peter Simonsson
  email:     psn@linux.se
*/

#include "servergroupdialog.h"
#include "identity.h"
#include "konversationapplication.h"
#include "viewcontainer.h"
#include "preferences.h"
#include "serversettings.h"
#include "serverdialog.h"
#include "channeldialog.h"
#include "identitydialog.h"
#include "servergroupdialogui.h"

#include <qframe.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qcheckbox.h>
#include <qwhatsthis.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kcombobox.h>
#include <klistbox.h>
#include <kpushbutton.h>


namespace Konversation
{

    ServerGroupDialog::ServerGroupDialog(const QString& title, QWidget *parent, const char *name)
        : KDialogBase(Swallow, title, Ok|Cancel, Ok, parent, name)
    {
        m_id = -1;
        m_identitiesNeedsUpdate = false;
        m_editedServer = false;

        m_mainWidget = new ServerGroupDialogUI(this);
        setMainWidget(m_mainWidget);

        QWhatsThis::add(m_mainWidget->m_nameEdit, i18n("Enter the name of the Network here. You may create as many entries in the Server List screen with the same Network as you like."));
        m_mainWidget->m_networkLabel->setBuddy(m_mainWidget->m_nameEdit);

        QWhatsThis::add(m_mainWidget->m_identityCBox,i18n("Choose an existing Identity or click the Edit button to add a new Identity or edit an existing one. The Identity will identify you and determine your nickname when you connect to the network."));
        m_mainWidget->m_identityLabel->setBuddy(m_mainWidget->m_identityCBox);
        connect(m_mainWidget->m_editIdentityButton, SIGNAL(clicked()), this, SLOT(editIdentity()));

        IdentityList identities = Preferences::identityList();

        for (IdentityList::ConstIterator it = identities.begin(); it != identities.end(); ++it)
            m_mainWidget->m_identityCBox->insertItem((*it)->getName());

        QWhatsThis::add(m_mainWidget->m_commandEdit, i18n("Optional. This command will be sent to the server after connecting. Example: <b>/msg NickServ IDENTIFY <i>konvirocks</i></b>. This example is for the freenode network, which requires users to register their nickname with a password and login when connecting. <i>konvirocks<i> is the password for the nickname given in Identity. You may enter more than one command by separating them with semicolons."));
        m_mainWidget->m_commandsLabel->setBuddy(m_mainWidget->m_commandEdit);

        QWhatsThis::add(m_mainWidget->m_autoConnectCBox, i18n("Check here if you want Konversation to automatically connect to this network whenever you open Konversation."));

        QWhatsThis::add(m_mainWidget->m_serverLBox, i18n("This is a list of IRC Servers in the network. When connecting to the network, Konversation will attempt to connect to the top server first. If this fails, it will attempt the second server. If this fails, it will attempt the third, and so on. At least one server must be specified. Click a server to highlight it."));
        m_mainWidget->m_removeServerButton->setIconSet(SmallIconSet("editdelete"));
        m_mainWidget->m_removeServerButton->setTextLabel(i18n("Delete"));
        m_mainWidget->m_upServerBtn->setIconSet(SmallIconSet("up"));
        m_mainWidget->m_downServerBtn->setIconSet(SmallIconSet("down"));

        connect(m_mainWidget->m_addServerButton, SIGNAL(clicked()), this, SLOT(addServer()));
        connect(m_mainWidget->m_changeServerButton, SIGNAL(clicked()), this, SLOT(editServer()));
        connect(m_mainWidget->m_removeServerButton, SIGNAL(clicked()), this, SLOT(deleteServer()));
        connect(m_mainWidget->m_serverLBox, SIGNAL(selectionChanged()), this, SLOT(updateServerArrows()));
        connect(m_mainWidget->m_upServerBtn, SIGNAL(clicked()), this, SLOT(moveServerUp()));
        connect(m_mainWidget->m_downServerBtn, SIGNAL(clicked()), this, SLOT(moveServerDown()));

        QWhatsThis::add(m_mainWidget->m_channelLBox, i18n("Optional. This is a list of the channels that will be automatically joined once Konversation has connected to a server. You may leave this blank if you wish to not automatically join any channels."));
        m_mainWidget->m_removeChannelButton->setIconSet(SmallIconSet("editdelete"));
        m_mainWidget->m_removeChannelButton->setTextLabel(i18n("Delete"));
        m_mainWidget->m_upChannelBtn->setIconSet(SmallIconSet("up"));
        m_mainWidget->m_downChannelBtn->setIconSet(SmallIconSet("down"));

        connect(m_mainWidget->m_addChannelButton, SIGNAL(clicked()), this, SLOT(addChannel()));
        connect(m_mainWidget->m_changeChannelButton, SIGNAL(clicked()), this, SLOT(editChannel()));
        connect(m_mainWidget->m_removeChannelButton, SIGNAL(clicked()), this, SLOT(deleteChannel()));
        connect(m_mainWidget->m_channelLBox, SIGNAL(selectionChanged()), this, SLOT(updateChannelArrows()));
        connect(m_mainWidget->m_upChannelBtn, SIGNAL(clicked()), this, SLOT(moveChannelUp()));
        connect(m_mainWidget->m_downChannelBtn, SIGNAL(clicked()), this, SLOT(moveChannelDown()));

        setButtonOK(KGuiItem(i18n("&OK"), "button_ok", i18n("Change network information")));
        setButtonCancel(KGuiItem(i18n("&Cancel"), "button_cancel", i18n("Discards all changes made")));

        m_mainWidget->m_nameEdit->setFocus();
    }

    ServerGroupDialog::~ServerGroupDialog()
    {
    }

    void ServerGroupDialog::setServerGroupSettings(ServerGroupSettingsPtr settings)
    {
        m_id = settings->id();
        m_sortIndex = settings->sortIndex();
        m_expanded = settings->expanded();
        m_enableNotifications = settings->enableNotifications();
        m_mainWidget->m_nameEdit->setText(settings->name());
        m_mainWidget->m_identityCBox->setCurrentText(settings->identity()->getName());
        m_mainWidget->m_commandEdit->setText(settings->connectCommands());
        m_mainWidget->m_autoConnectCBox->setChecked(settings->autoConnectEnabled());
        m_serverList = settings->serverList();
        m_channelHistory = settings->channelHistory();
        ServerList::iterator it;
        m_mainWidget->m_serverLBox->clear();

        for(it = m_serverList.begin(); it != m_serverList.end(); ++it)
        {
            m_mainWidget->m_serverLBox->insertItem((*it).host());
        }

        m_channelList = settings->channelList();
        ChannelList::iterator it2;

        for(it2 = m_channelList.begin(); it2 != m_channelList.end(); ++it2)
        {
            m_mainWidget->m_channelLBox->insertItem((*it2).name());
        }
    }

    ServerGroupSettingsPtr ServerGroupDialog::serverGroupSettings()
    {
        ServerGroupSettingsPtr settings = new ServerGroupSettings(m_id);
        settings->setSortIndex(m_sortIndex);
        settings->setName(m_mainWidget->m_nameEdit->text());
        IdentityList identities = Preferences::identityList();
        settings->setIdentityId(identities[m_mainWidget->m_identityCBox->currentItem()]->id());
        settings->setConnectCommands(m_mainWidget->m_commandEdit->text());
        settings->setAutoConnectEnabled(m_mainWidget->m_autoConnectCBox->isChecked());
        settings->setServerList(m_serverList);
        settings->setChannelList(m_channelList);
        settings->setChannelHistory(m_channelHistory);
        settings->setNotificationsEnabled(m_enableNotifications);
        settings->setExpanded(m_expanded);

        return settings;
    }

    ServerSettings ServerGroupDialog::editedServer()
    {
        if (m_editedServer && m_editedServerIndex < m_serverList.count())
        {
            return m_serverList[m_editedServerIndex];
        }

        return ServerSettings("");
    }

    int ServerGroupDialog::execAndEditServer(ServerSettings server)
    {
        show();
        editServer(server);
        return exec();
    }

    void ServerGroupDialog::addServer()
    {
        ServerDialog dlg(i18n("Add Server"), this);

        if(dlg.exec() == KDialog::Accepted)
        {
            ServerSettings server = dlg.serverSettings();
            m_mainWidget->m_serverLBox->insertItem(server.host());
            m_serverList.append(server);
            updateServerArrows();
        }
    }

    void ServerGroupDialog::editServer()
    {
        uint current = m_mainWidget->m_serverLBox->currentItem();

        if(current < m_serverList.count())
        {
            ServerDialog dlg(i18n("Edit Server"), this);
            dlg.setServerSettings(m_serverList[current]);

            if(dlg.exec() == KDialog::Accepted)
            {
                ServerSettings server = dlg.serverSettings();
                m_mainWidget->m_serverLBox->changeItem(server.host(), current);
                m_serverList[current] = server;
            }
        }
    }

    void ServerGroupDialog::editServer(ServerSettings server)
    {
        // Track the server the Server List dialog told us to edit
        // and find out which server to select in the listbox
        m_editedServer = true;
        m_editedServerIndex = m_serverList.findIndex(server);
        m_mainWidget->m_serverLBox->setCurrentItem(m_editedServerIndex);

        editServer();
    }

    void ServerGroupDialog::deleteServer()
    {
        uint current = m_mainWidget->m_serverLBox->currentItem();

        if (current < m_serverList.count())
        {
            m_serverList.remove(m_serverList.at(current));
            m_mainWidget->m_serverLBox->removeItem(current);

            // Track the server the Server List dialog told us to edit
            if (m_editedServer && m_editedServerIndex==current)
                m_editedServer = false;
        }

        updateServerArrows();
    }

    void ServerGroupDialog::updateServerArrows()
    {
        m_mainWidget->m_upServerBtn->setEnabled( m_mainWidget->m_serverLBox->count()>1 && m_mainWidget->m_serverLBox->currentItem()>0 );

        m_mainWidget->m_downServerBtn->setEnabled( m_mainWidget->m_serverLBox->count()>1 &&
            m_mainWidget->m_serverLBox->currentItem()<m_mainWidget->m_serverLBox->numRows()-1 );
        bool enabled = m_mainWidget->m_serverLBox->currentItem() >= 0;
        m_mainWidget->m_removeServerButton->setEnabled(enabled);
        m_mainWidget->m_changeServerButton->setEnabled(enabled);
    }

    void ServerGroupDialog::moveServerUp()
    {
        uint current = m_mainWidget->m_serverLBox->currentItem();

        if (current > 0)
        {
            ServerSettings server = m_serverList[current];
            m_mainWidget->m_serverLBox->removeItem(current);
            m_mainWidget->m_serverLBox->insertItem(server.host(), current - 1);
            m_mainWidget->m_serverLBox->setCurrentItem(current - 1);
            ServerList::iterator it = m_serverList.remove(m_serverList.at(current));
            --it;
            m_serverList.insert(it, server);

            // Track the server the Server List dialog told us to edit
            if (m_editedServer && m_editedServerIndex==current)
                m_editedServerIndex = current - 1;
        }

        updateServerArrows();
    }

    void ServerGroupDialog::moveServerDown()
    {
        uint current = m_mainWidget->m_serverLBox->currentItem();

        if (current < (m_serverList.count() - 1))
        {
            ServerSettings server = m_serverList[current];
            m_mainWidget->m_serverLBox->removeItem(current);
            m_mainWidget->m_serverLBox->insertItem(server.host(), current + 1);
            m_mainWidget->m_serverLBox->setCurrentItem(current + 1);
            ServerList::iterator it = m_serverList.remove(m_serverList.at(current));
            ++it;
            m_serverList.insert(it, server);

            // Track the server the Server List dialog told us to edit
            if (m_editedServer && m_editedServerIndex==current)
                m_editedServerIndex = current + 1;
        }

        updateServerArrows();
    }

    void ServerGroupDialog::addChannel()
    {
        ChannelDialog dlg(i18n("Add Channel"), this);

        if(dlg.exec() == KDialog::Accepted)
        {
            ChannelSettings channel = dlg.channelSettings();
            m_mainWidget->m_channelLBox->insertItem(channel.name());
            m_channelList.append(channel);
            updateChannelArrows();
        }
    }

    void ServerGroupDialog::editChannel()
    {
        uint current = m_mainWidget->m_channelLBox->currentItem();

        if(current < m_channelList.count())
        {
            ChannelDialog dlg(i18n("Edit Channel"), this);
            dlg.setChannelSettings(m_channelList[current]);

            if(dlg.exec() == KDialog::Accepted)
            {
                ChannelSettings channel = dlg.channelSettings();
                m_mainWidget->m_channelLBox->changeItem(channel.name(), current);
                m_channelList[current] = channel;
            }
        }
    }

    void ServerGroupDialog::deleteChannel()
    {
        uint current = m_mainWidget->m_channelLBox->currentItem();

        if(current < m_channelList.count())
        {
            m_channelList.remove(m_channelList.at(current));
            m_mainWidget->m_channelLBox->removeItem(current);
            updateChannelArrows();
        }
    }

    void ServerGroupDialog::updateChannelArrows()
    {
        m_mainWidget->m_upChannelBtn->setEnabled( m_mainWidget->m_channelLBox->count()>1 && m_mainWidget->m_channelLBox->currentItem()>0 );

        m_mainWidget->m_downChannelBtn->setEnabled( m_mainWidget->m_channelLBox->count()>1 &&
            m_mainWidget->m_channelLBox->currentItem()<m_mainWidget->m_channelLBox->numRows()-1 );
        bool selected = m_mainWidget->m_channelLBox->currentItem() >= 0;
        m_mainWidget->m_removeChannelButton->setEnabled(selected);
        m_mainWidget->m_changeChannelButton->setEnabled(selected);
    }

    void ServerGroupDialog::moveChannelUp()
    {
        uint current = m_mainWidget->m_channelLBox->currentItem();

        if(current > 0)
        {
            ChannelSettings channel = m_channelList[current];
            m_mainWidget->m_channelLBox->removeItem(current);
            m_mainWidget->m_channelLBox->insertItem(channel.name(), current - 1);
            m_mainWidget->m_channelLBox->setCurrentItem(current - 1);
            ChannelList::iterator it = m_channelList.remove(m_channelList.at(current));
            --it;
            m_channelList.insert(it, channel);
        }

        updateChannelArrows();
    }

    void ServerGroupDialog::moveChannelDown()
    {
        uint current = m_mainWidget->m_channelLBox->currentItem();

        if(current < (m_channelList.count() - 1))
        {
            ChannelSettings channel = m_channelList[current];
            m_mainWidget->m_channelLBox->removeItem(current);
            m_mainWidget->m_channelLBox->insertItem(channel.name(), current + 1);
            m_mainWidget->m_channelLBox->setCurrentItem(current + 1);
            ChannelList::iterator it = m_channelList.remove(m_channelList.at(current));
            ++it;
            m_channelList.insert(it, channel);
        }

        updateChannelArrows();
    }

    void ServerGroupDialog::editIdentity()
    {
        IdentityDialog dlg(this);
        dlg.setCurrentIdentity(m_mainWidget->m_identityCBox->currentItem());

        if(dlg.exec() == KDialog::Accepted)
        {
            IdentityList identities = Preferences::identityList();
            m_mainWidget->m_identityCBox->clear();

            for(IdentityList::ConstIterator it = identities.begin(); it != identities.end(); ++it)
            {
                m_mainWidget->m_identityCBox->insertItem((*it)->getName());
            }

            m_mainWidget->m_identityCBox->setCurrentText(dlg.currentIdentity()->getName());
            m_identitiesNeedsUpdate = true; // and what's this for?
            ViewContainer* vc = KonversationApplication::instance()->getMainWindow()->getViewContainer();
            vc->updateViewEncoding(vc->getFrontView());
        }
    }

    void ServerGroupDialog::slotOk()
    {
        if (m_mainWidget->m_nameEdit->text().isEmpty())
        {
            KMessageBox::error(this, i18n("The network name is required."));
        }
        else if (m_serverList.count() == 0)
        {
            KMessageBox::error(this, i18n("You need to add at least one server to the network."));
        }
        else
        {
            accept();
        }
    }

}

#include "servergroupdialog.moc"
