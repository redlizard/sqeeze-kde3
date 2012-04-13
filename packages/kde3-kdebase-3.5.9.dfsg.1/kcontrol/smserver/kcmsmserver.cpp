/*
 *  kcmsmserver.cpp
 *  Copyright (c) 2000,2002 Oswald Buddenhagen <ossi@kde.org>
 *
 *  based on kcmtaskbar.cpp
 *  Copyright (c) 2000 Kurt Granroth <granroth@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 */
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qradiobutton.h>

#include <dcopclient.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kgenericfactory.h>
#include <klineedit.h>

#include "kcmsmserver.h"
#include "smserverconfigimpl.h"

typedef KGenericFactory<SMServerConfig, QWidget > SMSFactory;
K_EXPORT_COMPONENT_FACTORY (kcm_smserver, SMSFactory("kcmsmserver") )

SMServerConfig::SMServerConfig( QWidget *parent, const char* name, const QStringList & )
  : KCModule (SMSFactory::instance(), parent, name)
{
    setQuickHelp( i18n("<h1>Session Manager</h1>"
    " You can configure the session manager here."
    " This includes options such as whether or not the session exit (logout)"
    " should be confirmed, whether the session should be restored again when logging in"
    " and whether the computer should be automatically shut down after session"
    " exit by default."));

    QVBoxLayout *topLayout = new QVBoxLayout(this);
    dialog = new SMServerConfigImpl(this);
    connect(dialog, SIGNAL(changed()), SLOT(changed()));

    dialog->show();
    topLayout->add(dialog);
    load();

}

void SMServerConfig::load()
{
	load( false );
}

void SMServerConfig::load(bool useDefaults )
{
  KConfig *c = new KConfig("ksmserverrc", false, false);
  c->setReadDefaults( useDefaults );
  c->setGroup("General");
  dialog->confirmLogoutCheck->setChecked(c->readBoolEntry("confirmLogout", true));
  bool en = c->readBoolEntry("offerShutdown", true);
  dialog->offerShutdownCheck->setChecked(en);
  dialog->sdGroup->setEnabled(en);

  QString s = c->readEntry( "loginMode" );
  if ( s == "default" )
      dialog->emptySessionRadio->setChecked(true);
  else if ( s == "restoreSavedSession" )
      dialog->savedSessionRadio->setChecked(true);
  else // "restorePreviousLogout"
      dialog->previousSessionRadio->setChecked(true);

  switch (c->readNumEntry("shutdownType", int(KApplication::ShutdownTypeNone))) {
  case int(KApplication::ShutdownTypeHalt):
    dialog->haltRadio->setChecked(true);
    break;
  case int(KApplication::ShutdownTypeReboot):
    dialog->rebootRadio->setChecked(true);
    break;
  default:
    dialog->logoutRadio->setChecked(true);
    break;
  }
  dialog->excludeLineedit->setText( c->readEntry("excludeApps"));

  delete c;

  emit changed(useDefaults);
}

void SMServerConfig::save()
{
  KConfig *c = new KConfig("ksmserverrc", false, false);
  c->setGroup("General");
  c->writeEntry( "confirmLogout", dialog->confirmLogoutCheck->isChecked());
  c->writeEntry( "offerShutdown", dialog->offerShutdownCheck->isChecked());
  QString s = "restorePreviousLogout";
  if ( dialog->emptySessionRadio->isChecked() )
      s = "default";
  else if ( dialog->savedSessionRadio->isChecked() )
      s = "restoreSavedSession";
  c->writeEntry( "loginMode", s );

  c->writeEntry( "shutdownType",
                 dialog->haltRadio->isChecked() ?
                   int(KApplication::ShutdownTypeHalt) :
                   dialog->rebootRadio->isChecked() ?
                     int(KApplication::ShutdownTypeReboot) :
                     int(KApplication::ShutdownTypeNone));
  c->writeEntry("excludeApps", dialog->excludeLineedit->text());
  c->sync();
  delete c;

  // update the k menu if necessary
  QByteArray data;
  kapp->dcopClient()->send( "kicker", "kicker", "configure()", data );
}

void SMServerConfig::defaults()
{
	load( true );
}

#include "kcmsmserver.moc"

