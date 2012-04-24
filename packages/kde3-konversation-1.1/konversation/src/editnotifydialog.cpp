/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

/*
  begin:     Wed Sep 1 2004
  copyright: (C) 2004 by Gary Cramblitt
  email:     garycramblitt@comcast.net
*/

#include "editnotifydialog.h"
#include "konversationapplication.h"
#include "servergroupsettings.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qwhatsthis.h>

#include <klineedit.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <klocale.h>


EditNotifyDialog::EditNotifyDialog(QWidget* parent,
const QString& network,
const QString& nickname):

KDialogBase(parent,"editnotify",true,i18n("Edit Watched Nickname"),
KDialogBase::Ok | KDialogBase::Cancel,
KDialogBase::Ok,true)

{
    QWidget* page=new QWidget(this);
    setMainWidget(page);

    QHBoxLayout* layout = new QHBoxLayout(page);
    layout->setSpacing(spacingHint());

    QLabel* networkNameLabel=new QLabel(i18n("&Network name:"),page);
    QString networkNameWT = i18n(
        "Pick the server network you will connect to here.");
    QWhatsThis::add(networkNameLabel, networkNameWT);
    m_networkNameCombo=new KComboBox(page,"notify_network_combo");
    QWhatsThis::add(m_networkNameCombo, networkNameWT);
    networkNameLabel->setBuddy(m_networkNameCombo);

    QLabel* nicknameLabel=new QLabel(i18n("N&ickname:"),page);
    QString nicknameWT = i18n(
        "<qt>The nickname to watch for when connected to a server in the network.</qt>");
    QWhatsThis::add(nicknameLabel, nicknameWT);
    m_nicknameInput = new KLineEdit(nickname, page);
    QWhatsThis::add(m_nicknameInput, nicknameWT);
    nicknameLabel->setBuddy(m_nicknameInput);

    // Build a list of unique server network names.
    // TODO: The "ServerGroupList type is a misnomer (it is actually networks), which
    // should be fixed at some point.
    Konversation::ServerGroupList serverNetworks = Preferences::serverGroupList();
    QStringList networkNames;

    for(Konversation::ServerGroupList::iterator it = serverNetworks.begin(); it != serverNetworks.end(); ++it)
    {
        QString name = (*it)->name();

        if (!networkNames.contains(name))
        {
            networkNames.append(name);
        }
    }

    networkNames.sort();
    // Add network names to network combobox and select the one corresponding to argument.
    for (QStringList::ConstIterator it = networkNames.begin(); it != networkNames.end(); ++it)
    {
        m_networkNameCombo->insertItem(*it);
        if(*it == network) m_networkNameCombo->setCurrentItem(m_networkNameCombo->count()-1);
    }

    layout->addWidget(networkNameLabel);
    layout->addWidget(m_networkNameCombo);
    layout->addWidget(nicknameLabel);
    layout->addWidget(m_nicknameInput);

    setButtonOK(KGuiItem(i18n("&OK"),"button_ok",i18n("Change notify information")));
    setButtonCancel(KGuiItem(i18n("&Cancel"),"button_cancel",i18n("Discards all changes made")));
}

EditNotifyDialog::~EditNotifyDialog()
{
}

void EditNotifyDialog::slotOk()
{
    emit notifyChanged(m_networkNameCombo->currentText(),
        m_nicknameInput->text());
    delayedDestruct();
}

#include "editnotifydialog.moc"
