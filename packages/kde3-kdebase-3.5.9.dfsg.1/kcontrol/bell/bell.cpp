/*
  Copyright (c) 1997 Christian Czezatke (e9025461@student.tuwien.ac.at)
                1998 Bernd Wuebben <wuebben@kde.org>
                2000 Matthias Elter <elter@kde.org>
                2001 Carsten PFeiffer <pfeiffer@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <knotifyclient.h>
#include <knuminput.h>

#include "bell.h"
#include "bell.moc"

#include <X11/Xlib.h>

extern "C"
{
  KDE_EXPORT KCModule *create_bell(QWidget *parent, const char *)
  {
    return new KBellConfig(parent, "kcmbell");
  }

  KDE_EXPORT void init_bell()
  {
    XKeyboardState kbd;
    XKeyboardControl kbdc;

    XGetKeyboardControl(kapp->getDisplay(), &kbd);

    KConfig config("kcmbellrc", true, false);
    config.setGroup("General");

    kbdc.bell_percent = config.readNumEntry("Volume", kbd.bell_percent);
    kbdc.bell_pitch = config.readNumEntry("Pitch", kbd.bell_pitch);
    kbdc.bell_duration = config.readNumEntry("Duration", kbd.bell_duration);
    XChangeKeyboardControl(kapp->getDisplay(),
                           KBBellPercent | KBBellPitch | KBBellDuration,
                           &kbdc);
  }
}

KBellConfig::KBellConfig(QWidget *parent, const char *name):
    KCModule(parent, name)
{
  QBoxLayout *layout = new QVBoxLayout(this, 0, KDialog::spacingHint());

  int row = 0;
  QGroupBox *box = new QGroupBox( i18n("Bell Settings"), this );
  box->setColumnLayout( 0, Qt::Horizontal );
  layout->addWidget(box);
  layout->addStretch();
  QGridLayout *grid = new QGridLayout(box->layout(), KDialog::spacingHint());
  grid->setColStretch(0, 0);
  grid->setColStretch(1, 1);
  grid->addColSpacing(0, 30);

  m_useBell = new QCheckBox( i18n("&Use system bell instead of system notification" ), box );
  QWhatsThis::add(m_useBell, i18n("You can use the standard system bell (PC speaker) or a "
				  "more sophisticated system notification, see the "
				  "\"System Notifications\" control module for the "
				  "\"Something Special Happened in the Program\" event."));
  connect(m_useBell, SIGNAL( toggled( bool )), SLOT( useBell( bool )));
  row++;
  grid->addMultiCellWidget(m_useBell, row, row, 0, 1);

  setQuickHelp( i18n("<h1>System Bell</h1> Here you can customize the sound of the standard system bell,"
    " i.e. the \"beep\" you always hear when there is something wrong. Note that you can further"
    " customize this sound using the \"Accessibility\" control module; for example, you can choose"
    " a sound file to be played instead of the standard bell."));

  m_volume = new KIntNumInput(50, box);
  m_volume->setLabel(i18n("&Volume:"));
  m_volume->setRange(0, 100, 5);
  m_volume->setSuffix("%");
  m_volume->setSteps(5,25);
  grid->addWidget(m_volume, ++row, 1);
  QWhatsThis::add( m_volume, i18n("Here you can customize the volume of the system bell. For further"
    " customization of the bell, see the \"Accessibility\" control module.") );

  m_pitch = new KIntNumInput(m_volume, 800, box);
  m_pitch->setLabel(i18n("&Pitch:"));
  m_pitch->setRange(20, 2000, 20);
  m_pitch->setSuffix(i18n(" Hz"));
  m_pitch->setSteps(40,200);
  grid->addWidget(m_pitch, ++row, 1);
  QWhatsThis::add( m_pitch, i18n("Here you can customize the pitch of the system bell. For further"
    " customization of the bell, see the \"Accessibility\" control module.") );

  m_duration = new KIntNumInput(m_pitch, 100, box);
  m_duration->setLabel(i18n("&Duration:"));
  m_duration->setRange(1, 1000, 50);
  m_duration->setSuffix(i18n(" msec"));
  m_duration->setSteps(20,100);
  grid->addWidget(m_duration, ++row, 1);
  QWhatsThis::add( m_duration, i18n("Here you can customize the duration of the system bell. For further"
    " customization of the bell, see the \"Accessibility\" control module.") );

  QBoxLayout *boxLayout = new QHBoxLayout();
  m_testButton = new QPushButton(i18n("&Test"), box, "test");
  boxLayout->addWidget(m_testButton, 0, AlignRight);
  grid->addLayout( boxLayout, ++row, 1 );
  connect( m_testButton, SIGNAL(clicked()), SLOT(ringBell()));
  QWhatsThis::add( m_testButton, i18n("Click \"Test\" to hear how the system bell will sound using your changed settings.") );

  // watch for changes
  connect(m_volume, SIGNAL(valueChanged(int)), SLOT(changed()));
  connect(m_pitch, SIGNAL(valueChanged(int)), SLOT(changed()));
  connect(m_duration, SIGNAL(valueChanged(int)), SLOT(changed()));
  
  KAboutData *about =
    new KAboutData(I18N_NOOP("kcmbell"), I18N_NOOP("KDE Bell Control Module"),
                  0, 0, KAboutData::License_GPL,
                  I18N_NOOP("(c) 1997 - 2001 Christian Czezatke, Matthias Elter"));

  about->addAuthor("Christian Czezatke", I18N_NOOP("Original author"), "e9025461@student.tuwien.ac.at");
  about->addAuthor("Bernd Wuebben", 0, "wuebben@kde.org");
  about->addAuthor("Matthias Elter", I18N_NOOP("Current maintainer"), "elter@kde.org");
  about->addAuthor("Carsten Pfeiffer", 0, "pfeiffer@kde.org");
  setAboutData(about);

  load();
}

void KBellConfig::load()
{
   load( false );
}

void KBellConfig::load( bool useDefaults )
{
  XKeyboardState kbd;
  XGetKeyboardControl(kapp->getDisplay(), &kbd);

  m_volume->setValue(kbd.bell_percent);
  m_pitch->setValue(kbd.bell_pitch);
  m_duration->setValue(kbd.bell_duration);

  KConfig cfg("kdeglobals", false, false);
  cfg.setReadDefaults(  useDefaults );
  cfg.setGroup("General");
  m_useBell->setChecked(cfg.readBoolEntry("UseSystemBell", false));
  useBell(m_useBell->isChecked());
  emit changed( useDefaults );
}

void KBellConfig::save()
{
  XKeyboardControl kbd;

  int bellVolume = m_volume->value();
  int bellPitch = m_pitch->value();
  int bellDuration = m_duration->value();

  kbd.bell_percent = bellVolume;
  kbd.bell_pitch = bellPitch;
  kbd.bell_duration = bellDuration;
  XChangeKeyboardControl(kapp->getDisplay(),
                         KBBellPercent | KBBellPitch | KBBellDuration,
                         &kbd);

  KConfig config("kcmbellrc", false, false);
  config.setGroup("General");
  config.writeEntry("Volume",bellVolume);
  config.writeEntry("Pitch",bellPitch);
  config.writeEntry("Duration",bellDuration);

  config.sync();

  KConfig cfg("kdeglobals", false, false);
  cfg.setGroup("General");
  cfg.writeEntry("UseSystemBell", m_useBell->isChecked());
  cfg.sync();
  
  if (!m_useBell->isChecked())
  {
    KConfig config("kaccessrc", false);

    config.setGroup("Bell");
    config.writeEntry("SystemBell", false);
    config.writeEntry("ArtsBell", false);
    config.writeEntry("VisibleBell", false);
  }
}

void KBellConfig::ringBell()
{
  if (!m_useBell->isChecked()) {
    KNotifyClient::beep();
    return;
  }

  // store the old state
  XKeyboardState old_state;
  XGetKeyboardControl(kapp->getDisplay(), &old_state);

  // switch to the test state
  XKeyboardControl kbd;
  kbd.bell_percent = m_volume->value();
  kbd.bell_pitch = m_pitch->value();
  if (m_volume->value() > 0)
    kbd.bell_duration = m_duration->value();
  else
    kbd.bell_duration = 0;
  XChangeKeyboardControl(kapp->getDisplay(),
                         KBBellPercent | KBBellPitch | KBBellDuration,
                         &kbd);
  // ring bell
  XBell(kapp->getDisplay(),0);

  // restore old state
  kbd.bell_percent = old_state.bell_percent;
  kbd.bell_pitch = old_state.bell_pitch;
  kbd.bell_duration = old_state.bell_duration;
  XChangeKeyboardControl(kapp->getDisplay(),
                         KBBellPercent | KBBellPitch | KBBellDuration,
                         &kbd);
}

void KBellConfig::defaults()
{
   load( true );
}

void KBellConfig::useBell( bool on )
{
  m_volume->setEnabled( on );
  m_pitch->setEnabled( on );
  m_duration->setEnabled( on );
  m_testButton->setEnabled( on );
  changed();
}
