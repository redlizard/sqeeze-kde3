/***************************************************************************
                          kospage.cpp  -  description
                             -------------------
    begin                : Tue May 22 2001
    copyright            : (C) 2001 by Ralf Nolden
    email                : nolden@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlabel.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qtextview.h>
#include <qmap.h>

#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <klocale.h>
#include <dcopclient.h>
#include <kipc.h>
#include <krun.h>
#include <kkeynative.h>
#include <kdebug.h>
#include <kiconloader.h>

#include "libkonq/konq_defaults.h"

#include "kospage.h"

KOSPage::KOSPage(QWidget *parent, const char *name ) : KOSPageDlg(parent,name) {
	px_osSidebar->setPixmap(UserIcon("step2.png"));
	// initialize the textview with the default description - KDE of course
	slotKDEDescription();
	// Set the configfiles
	cglobal = new KConfig("kdeglobals");
	claunch = new KConfig("klaunchrc", false, false);
	cwin = new KConfig("kwinrc");
	cdesktop = new KConfig("kdesktoprc");
	ckcminput = new KConfig("kcminputrc");
	ckcmdisplay = new KConfig("kcmdisplayrc");
	ckonqueror = new KConfig("konquerorrc");
	cklipper = new KConfig("klipperrc", false, false);
	ckaccess = new KConfig("kaccessrc");
	// Save the current user defaults
	getUserDefaults();
	// set default-selections for this page
	setDefaults();
}

KOSPage::~KOSPage(){
	delete cglobal;
	delete claunch;
	delete cwin;
	delete cdesktop;
	delete ckcmdisplay;
	delete ckcminput;
	delete ckonqueror;
	delete cklipper;
	delete ckaccess;
}


void KOSPage::save(bool currSettings){
	kdDebug() << "KOSPage::save()" << endl;
	// save like we want. Just set the Radiobutton to either how it is set in the dialog (currSettings=true, default)
	// or, if false, take the settings we got in getUserDefaults()
	saveCheckState(currSettings);
	// sync all configs
	cglobal->sync();
	claunch->sync();
	cwin->sync();
	cdesktop->sync();
	ckcmdisplay->sync();
	ckcminput->sync();
	ckonqueror->sync();
	cklipper->sync();
	ckaccess->sync();
	///////////////////////////////////////////
	// kcmdisplay changes
	KIPC::sendMessageAll(KIPC::SettingsChanged);
	QApplication::syncX();
	// enable/disable the mac menu, call dcop
	// Tell kdesktop about the new config file
	kapp->dcopClient()->send("kdesktop", "KDesktopIface", "configure()", QByteArray());
	///////////////////////////////////////////
	/// restart kwin  for window effects
	kapp->dcopClient()->send("kwin*", "", "reconfigure()", "");
	///////////////////////////////////////////
	
	// Make the kaccess daemon read the changed config file
	kapp->startServiceByDesktopName("kaccess");
}


	/** called by save() -- currSettings= true -> take the radiobutton, otherwise take user values set */
void KOSPage::saveCheckState(bool currSettings){
	if(currSettings){
		// Set the path for the keysscheme resource files
		KGlobal::dirs()->addResourceType("keys", KStandardDirs::kde_default("data")+"kcmkeys");
		// write the settings to the configfiles, depending on wich radiobutton is checked
		if(rb_kde->isChecked()){
			writeKDE();
			emit selectedOS("KDE");	// send a signal to be caught by the KStylePage to set the according style by default depending on the OS selection
		}
		else if(rb_unix->isChecked()){
			writeUNIX();
			emit selectedOS("CDE");	// send a signal to be caught by the KStylePage to set the according style by default depending on the OS selection
		}
		else if(rb_windows->isChecked()){
			writeWindows();
			emit selectedOS("win");	// send a signal to be caught by the KStylePage to set the according style by default depending on the OS selection
		}
		else if(rb_mac->isChecked()){
			writeMacOS();
			emit selectedOS("mac");	// send a signal to be caught by the KStylePage to set the according style by default depending on the OS selection
		}

		ckaccess->setGroup("Keyboard");
		ckaccess->writeEntry("Gestures", cb_gestures->isChecked(), true, true);

	}
	else {  // User has pressed "cancel & dismiss", so his old settings are written back
		writeUserDefaults();
	}
}


	/** write the settings for KDE-Behavior (called by saveCheckStatus) */
void KOSPage::writeKDE(){
	kdDebug() << "KOSPage::writeKDE()" << endl;

	ckcmdisplay->setGroup("KDE");
	ckcmdisplay->writeEntry("macStyle", false, true, true);

	cglobal->setGroup("KDE");
	cglobal->writeEntry("SingleClick", true, true, true);

	claunch->setGroup("FeedbackStyle");
	claunch->writeEntry("BusyCursor", true);

	cwin->setGroup("Windows");
	cwin->writeEntry("TitlebarDoubleClickCommand", "Shade");
	cwin->writeEntry("FocusPolicy", "ClickToFocus");
	cwin->writeEntry("AltTabStyle", "KDE");
	cwin->setGroup( "MouseBindings");
	cwin->writeEntry("CommandActiveTitlebar2", "Lower");
	cwin->writeEntry("CommandActiveTitlebar3", "Operations menu");

	cdesktop->setGroup( "Menubar" );
	cdesktop->writeEntry("ShowMenubar", false);
	cdesktop->setGroup( "Mouse Buttons" );
	cdesktop->writeEntry("Middle", "WindowListMenu");
	cdesktop->setGroup( "FMSettings" );
	cdesktop->writeEntry("UnderlineLinks", false);

	ckonqueror->setGroup( "FMSettings" );
	ckonqueror->writeEntry("UnderlineLinks", false);

	ckcminput->setGroup("KDE");
	ckcminput->writeEntry("ChangeCursor", true, true, true );

	cklipper->setGroup("General");
	cklipper->writeEntry("Synchronize", false);

	writeKeyEntrys(locate("keys", "kde3.kksrc"));
}


	/** write the settings for fvwm-like-behavior (called by saveCheckStatus) */
void KOSPage::writeUNIX(){
	kdDebug() << "KOSPage::writeUNIX()" << endl;

	ckcmdisplay->setGroup("KDE");
	ckcmdisplay->writeEntry("macStyle", false, true, true);

	cglobal->setGroup("KDE");
	cglobal->writeEntry("SingleClick", true, true, true);

	claunch->setGroup("FeedbackStyle");
	claunch->writeEntry("BusyCursor", false);

	cwin->setGroup("Windows");
	cwin->writeEntry("TitlebarDoubleClickCommand", "Shade");
	cwin->writeEntry("FocusPolicy", "FocusStrictlyUnderMouse");
	cwin->writeEntry("AltTabStyle", "CDE");
	cwin->setGroup( "MouseBindings");
	cwin->writeEntry("CommandActiveTitlebar2", "Operations menu");
	cwin->writeEntry("CommandActiveTitlebar3", "Lower");

	cdesktop->setGroup( "Menubar" );
	cdesktop->writeEntry("ShowMenubar", false);
	cdesktop->setGroup( "Mouse Buttons" );
	cdesktop->writeEntry("Middle", "AppMenu");
	cdesktop->setGroup( "FMSettings" );
	cdesktop->writeEntry("UnderlineLinks", false);

	ckonqueror->setGroup( "FMSettings" );
	ckonqueror->writeEntry("UnderlineLinks", false);

	ckcminput->setGroup("KDE");
	ckcminput->writeEntry("ChangeCursor", false, true, true );

	cklipper->setGroup("General");
	cklipper->writeEntry("Synchronize", true);

	writeKeyEntrys(locate("keys", "unix3.kksrc"));
}


	/** write the settings for windows-like-behavior (called by saveCheckStatus) */
void KOSPage::writeWindows(){
	kdDebug() << "KOSPage::writeWindows()" << endl;

	ckcmdisplay->setGroup("KDE");
	ckcmdisplay->writeEntry("macStyle", false, true, true);

	cglobal->setGroup("KDE");
	cglobal->writeEntry("SingleClick", false, true, true);

	claunch->setGroup("FeedbackStyle");
	claunch->writeEntry("BusyCursor", true);

	cwin->setGroup("Windows");
	cwin->writeEntry("TitlebarDoubleClickCommand", "Maximize");
	cwin->writeEntry("FocusPolicy", "ClickToFocus");
	cwin->writeEntry("AltTabStyle", "KDE");
	cwin->setGroup( "MouseBindings");
	cwin->writeEntry("CommandActiveTitlebar2", "Lower");
	cwin->writeEntry("CommandActiveTitlebar3", "Operations menu");

	cdesktop->setGroup( "Menubar" );
	cdesktop->writeEntry("ShowMenubar", false);
	cdesktop->setGroup( "Mouse Buttons" );
	cdesktop->writeEntry("Middle", "WindowListMenu");
	cdesktop->setGroup( "FMSettings" );
	cdesktop->writeEntry("UnderlineLinks", false);

	ckonqueror->setGroup( "FMSettings" );
	ckonqueror->writeEntry("UnderlineLinks", false);

	ckcminput->setGroup("KDE");
	ckcminput->writeEntry("ChangeCursor", false, true, true );

	cklipper->setGroup("General");
	cklipper->writeEntry("Synchronize", false);

	// set the schemefile depending on whether or not the keyboard has got Metakeys.
	if( KKeyNative::keyboardHasWinKey() ) {
		writeKeyEntrys(locate("keys", "win4.kksrc"));
	}
	else {
		writeKeyEntrys(locate("keys", "win3.kksrc"));
	}
}


	/** write the settings for MacOS-like-behavior (called by saveCheckStatus) */
void KOSPage::writeMacOS(){
	kdDebug() << "KOSPage::writeMacOS()" << endl;

	ckcmdisplay->setGroup("KDE");
	ckcmdisplay->writeEntry("macStyle", true, true, true);

	cglobal->setGroup("KDE");
	cglobal->writeEntry("SingleClick", true, true, true);

	claunch->setGroup("FeedbackStyle");
	claunch->writeEntry("BusyCursor", false);

	cwin->setGroup("Windows");
	cwin->writeEntry("TitlebarDoubleClickCommand", "Shade");
	cwin->writeEntry("FocusPolicy", "ClickToFocus");
	cwin->writeEntry("AltTabStyle", "KDE");
	cwin->setGroup( "MouseBindings");
	cwin->writeEntry("CommandActiveTitlebar2", "Lower");
	cwin->writeEntry("CommandActiveTitlebar3", "Operations menu");

	cdesktop->setGroup( "Menubar" );
	cdesktop->writeEntry("ShowMenubar", true);
	cdesktop->setGroup( "Mouse Buttons" );
	cdesktop->writeEntry("Middle", "WindowListMenu");
	cdesktop->setGroup( "FMSettings" );
	cdesktop->writeEntry("UnderlineLinks", false);

	ckonqueror->setGroup( "FMSettings" );
	ckonqueror->writeEntry("UnderlineLinks", false);

	ckcminput->setGroup("KDE");
	ckcminput->writeEntry("ChangeCursor", true, true, true );

	cklipper->setGroup("General");
	cklipper->writeEntry("Synchronize", false);

	writeKeyEntrys(locate("keys", "mac4.kksrc"));
}


	/** write Keyscheme to kdeglobals (called by saveCheckState) */
void KOSPage::writeKeyEntrys(QString keyfile){
	kdDebug() << "KOSPage::writeKeyEntrys()" << endl;

	// load the given .kksrc - file
	KSimpleConfig* scheme = new KSimpleConfig(keyfile, true);
	// load the default .kksrc - file
	KSimpleConfig* defScheme = new KSimpleConfig(locate("keys", "kde3.kksrc"), true);

	// we need the entries from the default - file, so we can compare with them
	QMap<QString, QString> defMap = defScheme->entryMap("Global Shortcuts");
	// first delete the group in kdeglobals, then write the non-default entries from the global .kksrc - file
	cglobal->deleteGroup("Global Shortcuts", true, true);
	// get the Global - Shortcuts and write them to kdeglobals
	cglobal->setGroup("Global Shortcuts");
	QMap<QString, QString> givenMap = scheme->entryMap("Global Shortcuts");
	for ( QMap<QString, QString>::Iterator it = givenMap.begin(); it != givenMap.end(); ++it ) {
		if ( (defMap[it.key()] == it.data()) && (it.data() != "none") ) {
			cglobal->writeEntry(it.key(), "default("+it.data()+")", true, true);
		} else {
			cglobal->writeEntry(it.key(), it.data(), true, true);
		}
	}

	// we need the entries from the default - file, so we can compare with them
	defMap = defScheme->entryMap("Shortcuts");
	// first delete the group in kdeglobals, then write the non-default entries from the global .kksrc - file
	cglobal->deleteGroup("Shortcuts", true, true);
	cglobal->setGroup("Shortcuts");
	givenMap = scheme->entryMap("Shortcuts");
	for ( QMap<QString, QString>::Iterator it = givenMap.begin(); it != givenMap.end(); ++it ) {
		// only write the entry, if it defers from kde3.kksrc
		if ( defMap[it.key()] != it.data() ) {
			cglobal->writeEntry(it.key(), it.data(), true, true);
		}
	}

	delete scheme;
	delete defScheme;
}

void KOSPage::slotKDEDescription(){
	kdDebug() << "slotKDEDescription()" << endl;
	textview_ospage->setText("");
	textview_ospage->setText(i18n(
	"<b>Window activation:</b> <i>Focus on click</i><br>"
	"<b>Titlebar double-click:</b> <i>Shade window</i><br>"
	"<b>Mouse selection:</b> <i>Single click</i><br>"
	"<b>Application startup notification:</b> <i>busy cursor</i><br>"
	"<b>Keyboard scheme:</b> <i>KDE default</i><br>"
	));
}

void KOSPage::slotUnixDescription(){
	kdDebug() << "slotUnixDescription()" << endl;
	textview_ospage->setText("" );
	textview_ospage->setText(i18n(
	"<b>Window activation:</b> <i>Focus follows mouse</i><br>"
	"<b>Titlebar double-click:</b> <i>Shade window</i><br>"
	"<b>Mouse selection:</b> <i>Single click</i><br>"
	"<b>Application startup notification:</b> <i>none</i><br>"
	"<b>Keyboard scheme:</b> <i>UNIX</i><br>"
	));
}

void KOSPage::slotWindowsDescription(){
	kdDebug() << "slotWindowsDescription()" << endl;
	textview_ospage->setText("");
	textview_ospage->setText(i18n(
	"<b>Window activation:</b> <i>Focus on click</i><br>"
	"<b>Titlebar double-click:</b> <i>Maximize window</i><br>"
	"<b>Mouse selection:</b> <i>Double click</i><br>"
	"<b>Application startup notification:</b> <i>busy cursor</i><br>"
	"<b>Keyboard scheme:</b> <i>Windows</i><br>"
	));
}

void KOSPage::slotMacDescription(){
	kdDebug() << "slotMacDescription()" << endl;
	textview_ospage->setText("");
	textview_ospage->setText(i18n(
	"<b>Window activation:</b> <i>Focus on click</i><br>"
	"<b>Titlebar double-click:</b> <i>Shade window</i><br>"
	"<b>Mouse selection:</b> <i>Single click</i><br>"
	"<b>Application startup notification:</b> <i>none</i><br>"
	"<b>Keyboard scheme:</b> <i>Mac</i><br>"
	));
}


/** retrieves the user's local values. In case he doesn't have these set, use the default values of KDE */
void KOSPage::getUserDefaults(){
	ckcmdisplay->setGroup("KDE");
	b_MacMenuBar = ckcmdisplay->readBoolEntry("macStyle", false);

	cglobal->setGroup("KDE");
	b_SingleClick = cglobal->readBoolEntry("SingleClick", true);

	claunch->setGroup("FeedbackStyle");
	b_BusyCursor = claunch->readBoolEntry("BusyCursor", true);

	cwin->setGroup("Windows");
	s_TitlebarDCC = cwin->readEntry("TitlebarDoubleClickCommand", "Shade");
	s_FocusPolicy = cwin->readEntry("FocusPolicy", "ClickToFocus");
	s_AltTabStyle = cwin->readEntry("AltTabStyle", "KDE");
	cwin->setGroup( "MouseBindings");
	s_TitlebarMMB = cwin->readEntry("CommandActiveTitlebar2", "Lower");
	s_TitlebarRMB = cwin->readEntry("CommandActiveTitlebar3", "Operations menu");

	cdesktop->setGroup( "Menubar" );
	b_ShowMenuBar = cdesktop->readBoolEntry("ShowMenubar", false);
	cdesktop->setGroup( "Mouse Buttons" );
	s_MMB = cdesktop->readEntry("Middle", "WindowListMenu");
	cdesktop->setGroup( "FMSettings" );
	b_DesktopUnderline = cdesktop->readBoolEntry("UnderlineLinks", DEFAULT_UNDERLINELINKS);

	ckonqueror->setGroup( "FMSettings" );
	b_KonqUnderline = ckonqueror->readBoolEntry("UnderlineLinks", DEFAULT_UNDERLINELINKS);

	ckcminput->setGroup("KDE");
	b_ChangeCursor = ckcminput->readBoolEntry("ChangeCursor", true);

	cklipper->setGroup("General");
	b_syncClipboards = cklipper->readBoolEntry("Synchronize", false);

	map_GlobalUserKeys = cglobal->entryMap("Global Shortcuts");
	map_AppUserKeys = cglobal->entryMap("Shortcuts");

	ckaccess->setGroup("Keyboard");
	b_Gestures = ckaccess->readBoolEntry("Gestures", true);
}


	/** writes the user-defaults back */
void KOSPage::writeUserDefaults(){
	kdDebug() << "KOSPage::writeUserDefaults()" << endl;

	ckcmdisplay->setGroup("KDE");
	ckcmdisplay->writeEntry("macStyle", b_MacMenuBar, true, true);

	cglobal->setGroup("KDE");
	cglobal->writeEntry("SingleClick", b_SingleClick, true, true);

	claunch->setGroup("FeedbackStyle");
	claunch->writeEntry("BusyCursor", b_BusyCursor);

	cwin->setGroup("Windows");
	cwin->writeEntry("TitlebarDoubleClickCommand", s_TitlebarDCC);
	cwin->writeEntry("FocusPolicy", s_FocusPolicy);
	cwin->writeEntry("AltTabStyle", s_AltTabStyle);
	cwin->setGroup( "MouseBindings");
	cwin->writeEntry("CommandActiveTitlebar2", s_TitlebarMMB);
	cwin->writeEntry("CommandActiveTitlebar3", s_TitlebarRMB);

	cdesktop->setGroup( "Menubar" );
	cdesktop->writeEntry("ShowMenubar", b_ShowMenuBar);
	cdesktop->setGroup( "Mouse Buttons" );
	cdesktop->writeEntry("Middle", s_MMB);
	cdesktop->setGroup( "FMSettings" );
	cdesktop->writeEntry("UnderlineLinks", b_DesktopUnderline);

	ckonqueror->setGroup( "FMSettings" );
	ckonqueror->writeEntry("UnderlineLinks", b_KonqUnderline);

	ckcminput->setGroup("KDE");
	ckcminput->writeEntry("ChangeCursor", b_ChangeCursor, true, true );

	cklipper->setGroup("General");
	cklipper->writeEntry("Synchronize", b_syncClipboards);

	ckaccess->setGroup("Keyboard");
	ckaccess->writeEntry("Gestures", b_Gestures, true, true);

	writeUserKeys();
}

	/** called by writeUserDefaults() */
void KOSPage::writeUserKeys(){
	kdDebug() << "KOSPage::writeUserKeys()" << endl;

	cglobal->setGroup("Global Shortcuts");
	QMap<QString, QString>::Iterator it;	
	for ( it = map_GlobalUserKeys.begin(); it != map_GlobalUserKeys.end(); ++it ) {
		cglobal->writeEntry(it.key(), it.data(), true, true);
	}

	cglobal->deleteGroup("Shortcuts", true, true);
	cglobal->setGroup("Shortcuts");
	for ( it = map_AppUserKeys.begin(); it != map_AppUserKeys.end(); ++it ) {
		cglobal->writeEntry(it.key(), it.data(), true, true);
	}
}


	/** resets the radio button selected to kde */
void KOSPage::setDefaults(){
    rb_kde->setChecked(true);
    cb_gestures->setChecked(false);
}

#include "kospage.moc"

