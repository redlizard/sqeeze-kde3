/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

/*
  Copyright (C) 2006 John Tapsell <johnflux@gmail.com>
  Copyright (C) 2006 Eike Hein <hein@kde.org>
*/

#include "konvisettingsdialog.h"
#include "konviconfigdialog.h"
#include "config/preferences.h"
#include "chatwindowappearance_preferences.h"
#include "connectionbehavior_preferences.h"
#include "highlight_preferences.h"
#include "warnings_preferences.h"
#include "log_preferences.h"
#include "quickbuttons_preferences.h"
#include "autoreplace_preferences.h"
#include "chatwindowbehaviour_preferences.h"
#include "fontappearance_preferences.h"
#include "nicklistbehavior_preferences.h"
#include "tabs_preferences.h"
#include "colorsappearance_preferences.h"
#include "generalbehavior_preferences.h"
#include "dcc_preferences.h"
#include "osd_preferences.h"
#include "theme_preferences.h"
#include "alias_preferences.h"
#include "ignore_preferences.h"
#include "watchednicknames_preferences.h"
#include "tabnotifications_preferences.h"

#include <qsplitter.h>
#include <qcombobox.h>

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klistview.h>


KonviSettingsDialog::KonviSettingsDialog( QWidget *parent) :
	           KonviConfigDialog( parent, "settings", Preferences::self(), KDialogBase::TreeList)
{
  m_modified = false;
  setShowIconsInTreeList(true);

  QStringList iconPath;

  iconPath << i18n("Interface");
  setFolderIcon( iconPath, SmallIcon("looknfeel") );

  iconPath.clear();
  iconPath << i18n("Behavior");
  setFolderIcon( iconPath, SmallIcon("configure") );

  iconPath.clear();
  iconPath<< i18n("Behavior");
  setFolderIcon( iconPath, SmallIcon("configure") );

  iconPath.clear();
  iconPath<< i18n("Notifications");
  setFolderIcon( iconPath, SmallIcon("playsound") );

  QStringList pagePath;

  //Interface/Chat Window
  m_confChatWindowAppearanceWdg = new ChatWindowAppearance_Config( 0, "ChatWindowAppearance" );
  m_confChatWindowAppearanceWdg->kcfg_TimestampFormat->insertItem("hh:mm");
  m_confChatWindowAppearanceWdg->kcfg_TimestampFormat->insertItem("hh:mm:ss");
  m_confChatWindowAppearanceWdg->kcfg_TimestampFormat->insertItem("h:m ap");
  pagePath.clear();
  pagePath << i18n("Interface") << i18n("Chat Window");
  addPage ( m_confChatWindowAppearanceWdg, pagePath, "view_text", i18n("Chat Window") );

  //Interface/Themes
  m_confThemeWdg = new Theme_Config( this, "Theme" );
  pagePath.clear();
  pagePath << i18n("Interface") << i18n("Nicklist Themes");
  addPage ( m_confThemeWdg, pagePath, "iconthemes", i18n("Nicklist Themes") );
  m_indexToPageMapping.insert(lastAddedIndex(), m_confThemeWdg);
  connect(m_confThemeWdg, SIGNAL(modified()), this, SLOT(modifiedSlot()));

  //Interface/Colors
  m_confColorsAppearanceWdg = new ColorsAppearance_Config( this, "ColorsAppearance" );
  pagePath.clear();
  pagePath << i18n("Interface") << i18n("Colors");
  addPage ( m_confColorsAppearanceWdg, pagePath, "colorize", i18n("Colors") );

  //Interface/Fonts
  m_confFontAppearanceWdg = new FontAppearance_Config( this, "FontAppearance" );
  pagePath.clear();
  pagePath << i18n("Interface") << i18n("Fonts");
  addPage ( m_confFontAppearanceWdg, pagePath, "fonts", i18n("Fonts") );

  //Interface/Quick Buttons
  m_confQuickButtonsWdg = new QuickButtons_Config( this, "QuickButtons" );
  pagePath.clear();
  pagePath << i18n("Interface") << i18n("Quick Buttons");
  addPage ( m_confQuickButtonsWdg, pagePath, "keyboard", i18n("Quick Buttons") );
  m_indexToPageMapping.insert(lastAddedIndex(), m_confQuickButtonsWdg);
  connect(m_confQuickButtonsWdg, SIGNAL(modified()), this, SLOT(modifiedSlot()));

  //Interface/Tabs
  m_confTabBarWdg = new Tabs_Config( this, "TabBar" );
  pagePath.clear();
  pagePath << i18n("Interface") << i18n("Tabs");
  addPage ( m_confTabBarWdg, pagePath, "tab_new", i18n("Tabs") );

  //Behavior/General
  m_confGeneralBehaviorWdg = new GeneralBehavior_Config( this, "GeneralBehavior" );
  pagePath.clear();
  pagePath << i18n("Behavior") << i18n("General");
  addPage ( m_confGeneralBehaviorWdg, pagePath, "exec", i18n("General") );

  //Behavior/Connection
  m_confConnectionBehaviorWdg = new ConnectionBehavior_Config( this, "ConnectionBehavior" );
  pagePath.clear();
  pagePath << i18n("Behavior") << i18n("Connection");
  addPage ( m_confConnectionBehaviorWdg, pagePath, "connect_creating", i18n("Connection") );

  //Behaviour/Chat Window
  m_confChatwindowBehaviourWdg = new ChatwindowBehaviour_Config( this, "ChatwindowBehaviour" );
  pagePath.clear();
  pagePath << i18n("Behavior") << i18n("Chat Window");
  addPage ( m_confChatwindowBehaviourWdg, pagePath, "view_text", i18n("Chat Window") );

  //Behaviour/Nickname List
  m_confNicklistBehaviorWdg = new NicklistBehavior_Config( this, "NicklistBehavior" );
  pagePath.clear();
  pagePath << i18n("Behavior") << i18n("Nickname List");
  addPage ( m_confNicklistBehaviorWdg, pagePath, "player_playlist", i18n("Nickname List") );
  connect(m_confNicklistBehaviorWdg, SIGNAL(modified()), this, SLOT(modifiedSlot()));
  m_indexToPageMapping.insert(lastAddedIndex(), m_confNicklistBehaviorWdg);

  //Behaviour/Command Aliases
  m_confAliasWdg = new Alias_Config( this, "Alias" );
  pagePath.clear();
  pagePath << i18n("Behavior") << i18n("Command Aliases");
  addPage ( m_confAliasWdg, pagePath, "editcopy", i18n("Command Aliases") );
  m_indexToPageMapping.insert(lastAddedIndex(), m_confAliasWdg);
  connect(m_confAliasWdg, SIGNAL(modified()), this, SLOT(modifiedSlot()));

  //Behaviour/Auto Replace
  m_confAutoreplaceWdg = new Autoreplace_Config( this, "Autoreplace" );
  pagePath.clear();
  pagePath << i18n("Behavior") << i18n("Auto Replace");
  addPage ( m_confAutoreplaceWdg, pagePath, "kview", i18n("Auto Replace") );
  m_indexToPageMapping.insert(lastAddedIndex(), m_confAutoreplaceWdg);
  connect(m_confAutoreplaceWdg, SIGNAL(modified()), this, SLOT(modifiedSlot()));

  //Behaviour/Ignore
  m_confIgnoreWdg = new Ignore_Config(this, "Ignore");
  pagePath.clear();
  pagePath << i18n("Behavior") << i18n("Ignore");
  addPage ( m_confIgnoreWdg, pagePath, "stop", i18n("Ignore") );
  connect(m_confIgnoreWdg, SIGNAL(modified()), this, SLOT(modifiedSlot()));
  m_indexToPageMapping.insert(lastAddedIndex(), m_confIgnoreWdg);

  //Behaviour/Logging
  m_confLogWdg = new Log_Config( this, "Log" );
  pagePath.clear();
  pagePath << i18n("Behavior") << i18n("Logging");
  addPage ( m_confLogWdg, pagePath, "log", i18n("Logging") );

  m_confDCCWdg = new DCC_Config( this, "DCC" );
  pagePath.clear();
  pagePath << i18n("Behavior") << i18n("DCC");
  addPage ( m_confDCCWdg, pagePath, "2rightarrow", i18n("DCC") );

  //Notifications/Tab Bar
  m_confTabNotificationsWdg = new TabNotifications_Config( this, "TabBar" );
  pagePath.clear();
  pagePath << i18n("Notifications") << i18n("Tabs");
  addPage ( m_confTabNotificationsWdg, pagePath, "tab_new", i18n("Tabs") );

  //Notification/Highlighting
  m_confHighlightWdg = new Highlight_Config( this, "Highlight" );
  pagePath.clear();
  pagePath << i18n("Notifications") << i18n("Highlight");
  addPage ( m_confHighlightWdg, pagePath, "paintbrush", i18n("Highlight") );
  connect(m_confHighlightWdg, SIGNAL(modified()), this, SLOT(modifiedSlot()));
  m_indexToPageMapping.insert(lastAddedIndex(), m_confHighlightWdg);

  //Notification/Watched Nicknames
  m_confWatchedNicknamesWdg = new WatchedNicknames_Config( this, "WatchedNicknames" );
  pagePath.clear();
  pagePath << i18n("Notifications") << i18n("Watched Nicknames");
  addPage ( m_confWatchedNicknamesWdg, pagePath, "kfind", i18n("Watched Nicknames") );
  // remember index so we can open this page later from outside
  m_watchedNicknamesIndex=lastAddedIndex();
  connect(m_confWatchedNicknamesWdg, SIGNAL(modified()), this, SLOT(modifiedSlot()));
  m_indexToPageMapping.insert(lastAddedIndex(), m_confWatchedNicknamesWdg);

  //Notification/On Screen Display
  m_confOSDWdg = new OSD_Config( this, "OSD" );
  pagePath.clear();
  pagePath << i18n("Notifications") << i18n("On Screen Display");
  addPage ( m_confOSDWdg, pagePath, "tv", i18n("On Screen Display") );
  //no modified connection needed - it's all kcfg widgets
  m_indexToPageMapping.insert(lastAddedIndex(), m_confOSDWdg);

  //Notification/Warning Dialogs
  m_confWarningsWdg = new Warnings_Config( this, "Warnings" );
  pagePath.clear();
  pagePath << i18n("Notifications") << i18n("Warning Dialogs");
  addPage ( m_confWarningsWdg, pagePath, "messagebox_warning", i18n("Warning Dialogs") );
  m_indexToPageMapping.insert(lastAddedIndex(), m_confWarningsWdg);
  connect(m_confWarningsWdg, SIGNAL(modified()), this, SLOT(modifiedSlot()));

  unfoldTreeList();
}

void KonviSettingsDialog::showEvent(QShowEvent* e)
{
  KonviConfigDialog::showEvent(e);

  QSplitter* splitter = ((QSplitter*)child(0, "QSplitter", true));
  KListView* listView = ((KListView*)child(0, "KListView", true));

  if (splitter && listView)
  {
    int visible = listView->visibleWidth();
    int content = listView->contentsWidth();

    if (visible < content)
    {
      int shiftSplitterBy = content - visible;
      resize(width()+shiftSplitterBy, height());
      QValueList<int> oldSizes = splitter->sizes();
      QValueList<int> newSizes;
      newSizes << oldSizes[0] + shiftSplitterBy << oldSizes[1] - shiftSplitterBy;
      splitter->setSizes(newSizes);
    }
  }
}

void KonviSettingsDialog::modifiedSlot()
{
  // this is for the non KConfigXT parts to tell us, if the user actually changed
  // something or went back to the old settings
// kdDebug() << "KonviSettingsDialog::modifiedSlot()" << endl;
  m_modified = false;
  QIntDictIterator<KonviSettingsPage> it( m_indexToPageMapping );
  for ( ; it.current(); ++it )
  {
    if ( (*it).hasChanged() )
    {
      m_modified = true;
//      kdDebug() << "KonviSettingsDialog::modifiedSlot(): modified!" << endl;
      break;
    }
  }
  updateButtons();
}

KonviSettingsDialog::~KonviSettingsDialog()
{
}

void KonviSettingsDialog::updateSettings()
{
  QIntDictIterator<KonviSettingsPage> it( m_indexToPageMapping );
  for ( ; it.current(); ++it )
  {
    // this is for the non KConfigXT parts to update the UI (like quick buttons)
    (*it).saveSettings();
  }
  m_modified = false;
  emit settingsChanged();
}

void KonviSettingsDialog::updateWidgets()
{
  QIntDictIterator<KonviSettingsPage> it( m_indexToPageMapping );
  for ( ; it.current(); ++it )
  {
    (*it).loadSettings();
  }
  m_modified = false;
}

void KonviSettingsDialog::updateWidgetsDefault()
{
  QIntDictIterator<KonviSettingsPage> it( m_indexToPageMapping );
  for ( ; it.current(); ++it )
  {
    (*it).restorePageToDefaults();
  }
  m_modified = true;
}

void KonviSettingsDialog::openWatchedNicknamesPage()
{
  // page index has been calculated in the constructor
  showPage(m_watchedNicknamesIndex);
}

// accessor method - will be used by KonviConfigDialog::updateButtons()
bool KonviSettingsDialog::hasChanged()
{
  return m_modified;
}

// accessor method - will be used by KonviConfigDialog::updateButtons()
bool KonviSettingsDialog::isDefault()
{
  return true;
}

#include "konvisettingsdialog.moc"

