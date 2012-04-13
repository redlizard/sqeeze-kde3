/***************************************************************************
                          keyecandypage.cpp  -  description
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
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qslider.h>
#include <qcolor.h>
#include <qsettings.h>
#include <qfont.h>

#include <ksimpleconfig.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <dcopclient.h>
#include <kipc.h>
#include <kapplication.h>
#include <klistview.h>
#include <krun.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <ktrader.h>
#include <kglobalsettings.h>

#include <stdlib.h>

#include <kdebug.h>

#include "ksysinfo.h"
#include "keyecandypage.h"

KEyeCandyPage::KEyeCandyPage(QWidget *parent, const char *name ) : KEyeCandyPageDlg(parent,name) {

	kwinconf = new KConfig("kwinrc", false, true);
	kwineventconf = new KConfig("kwin.eventsrc", false, false);
	kickerconf =  new KConfig("kickerrc",false, false);
	konquerorconf =  new KConfig("konquerorrc",false, false);
	konqiconconf = new KConfig("konqiconviewrc",false,false);
	kdesktopconf = new KConfig("kdesktoprc", false, false);

	sys = new KSysInfo();

	prevImage=false;
	prevText=false;
	prevOther=false;

	osStyle="";

	px_eyecandySidebar->setPixmap(UserIcon("step3.png"));

	klv_features->addColumn(i18n("Features"));
	klv_features->setFullWidth (true);

	// Level 1
	desktop_wallpaper = new QCheckListItem(klv_features, i18n("Desktop Wallpaper"),
				QCheckListItem::CheckBox);
	desktop_window_effects= new QCheckListItem(klv_features, i18n("Window Moving/Resizing Effects"),
				QCheckListItem::CheckBox);
	desktop_window_moving_contents= new QCheckListItem(klv_features, i18n("Display Contents in Moving/Resizing Windows"),
				QCheckListItem::CheckBox);
	// Level 2
	backgrounds_konqueror= new QCheckListItem(klv_features, i18n("File Manager Background Picture"),
				QCheckListItem::CheckBox);
	backgrounds_panel= new QCheckListItem(klv_features, i18n("Panel Background Picture"),
				QCheckListItem::CheckBox);
	// Level 3
	icon_zooming_panel = new QCheckListItem(klv_features, i18n("Panel Icon Popups"),
				QCheckListItem::CheckBox);
	icon_effect_gamma = new QCheckListItem(klv_features, i18n("Icon Highlighting"),
				QCheckListItem::CheckBox);
	icon_mng_animation = new QCheckListItem(klv_features, i18n("File Manager Icon Animation"),
				QCheckListItem::CheckBox);

	sound_scheme = new QCheckListItem(klv_features, i18n("Sound Theme"),
				QCheckListItem::CheckBox);

	// Level 4
	///////////////////////////////////////////////////////////////////////////////////
	/// DEPENDEND ON RESOLUTION; DEFAULT: DON`T USE IN LEVELS
	icon_effect_size_desktop = new QCheckListItem(klv_features, i18n("Large Desktop Icons"),
				QCheckListItem::CheckBox);
	icon_effect_size_panel = new QCheckListItem(klv_features, i18n("Large Panel Icons"),
				QCheckListItem::CheckBox);
	/// DEPENDEND ON RESOLUTION; DEFAULT: DON`T USE IN LEVELS
	///////////////////////////////////////////////////////////////////////////////////
	antialiasing_fonts = new QCheckListItem(klv_features, i18n("Smoothed Fonts "
				"(Antialiasing)"), QCheckListItem::CheckBox);

	// Level 5
	preview_images = new QCheckListItem(klv_features, i18n("Preview Images"),
				QCheckListItem::CheckBox);
	pushbutton_icons = new QCheckListItem(klv_features, i18n("Icons on Buttons"),
				QCheckListItem::CheckBox);

	// Level 6
	animated_combo = new QCheckListItem(klv_features, i18n("Animated Combo Boxes"),
				QCheckListItem::CheckBox);

	// Level 7
	fading_tooltips = new QCheckListItem(klv_features, i18n("Fading Tooltips"), QCheckListItem::CheckBox);

	// Level 8
	preview_text = new QCheckListItem(klv_features, i18n("Preview Text Files"),
				QCheckListItem::CheckBox);
	// Level 9
	fading_menus= new QCheckListItem(klv_features, i18n("Fading Menus"),
				QCheckListItem::CheckBox);
	preview_other = new QCheckListItem(klv_features, i18n("Preview Other Files"),
				QCheckListItem::CheckBox);

	getUserDefaults();	// get user's current settings
	setDefaults();		// set the initial level on the slider and checkboxes
}

KEyeCandyPage::~KEyeCandyPage(){
	delete kwinconf;
	delete kwineventconf;
	delete kickerconf;
	delete konquerorconf;
	delete konqiconconf;
	delete kdesktopconf;
	delete sys;
}

/** enables/disables the QCheckListItems in the klv_features
according to the level the slider moved. */
void KEyeCandyPage::slotEyeCandySliderMoved(int value){
	// Level 1
	desktop_wallpaper->setOn(false);
	desktop_window_effects->setOn(false);
	desktop_window_moving_contents->setOn(false);
	// Level 2
	backgrounds_konqueror->setOn(false);
	backgrounds_panel->setOn(false);
	// Level 3
	icon_effect_gamma->setOn(false);
	icon_zooming_panel->setOn(false);
	icon_mng_animation->setOn(false);
	// Level 4
	icon_effect_size_desktop->setOn(false);
	icon_effect_size_panel->setOn(false);
	antialiasing_fonts->setOn(false);
	// Level 5
	preview_images->setOn(false);
	// Level 6
	animated_combo->setOn(false);
	// Level 7
	fading_tooltips->setOn(false);
	// Level 8
	preview_text->setOn(false);
	// Level 9
	fading_menus->setOn(false);
	preview_other->setOn(false);
	sound_scheme->setOn(false);
	pushbutton_icons->setOn(false);

	if( value >= 1){
		// Level 1
		desktop_wallpaper->setOn(true);
		desktop_window_effects->setOn(true);
		desktop_window_moving_contents->setOn(true);
	}
	if( value >= 2){
		// Level 2
		backgrounds_konqueror->setOn(true);
		backgrounds_panel->setOn(true);
	}
	if( value >= 3){
		// Level 3
		icon_effect_gamma->setOn(true);
		if(!(sys->isXfromXFreeInc() && sys->getXRelease()==40100000) )
			icon_zooming_panel->setOn(true);
	}
	if( value >= 4){
		// Level 4
		icon_mng_animation->setOn(true);
		if(osStyle=="mac" || osStyle=="CDE"){
			for (int i = 0; i < QApplication::desktop()->numScreens(); i++) {
				if(QApplication::desktop()->screenGeometry(i).width() >= 1024)
					icon_effect_size_desktop->setOn(true); // enable 48x48 icons by default if a screen size is wider than 1024
				if(QApplication::desktop()->screenGeometry(i).width() >= 1280)
					icon_effect_size_panel->setOn(true);
			}
		}
		antialiasing_fonts->setOn(true);
	}
	if( value >= 5){
		// Level 5
		preview_images->setOn(true);
	}
	if( value >= 6){
		// Level 6
		animated_combo->setOn(true);
	}
	if( value >= 7){
		// Level 7
		fading_tooltips->setOn(true);
		fading_menus->setOn(true);
	}
	if( value >= 8){
		// Level 8
		preview_text->setOn(true);
		// icon-size (now for all selections)
		for (int i = 0; i < QApplication::desktop()->numScreens(); i++) {
			if(QApplication::desktop()->screenGeometry(i).width() >= 1024)
				icon_effect_size_desktop->setOn(true);
			if(QApplication::desktop()->screenGeometry(i).width() >= 1280)
				icon_effect_size_panel->setOn(true);
		}
	}
	if( value >= 9){
		// Level 9
		preview_other->setOn(true);
		sound_scheme->setOn(true);
		pushbutton_icons->setOn(true);
	}
}


//---------------------------DESKTOP--------------------------------------------------
/** This should be self-explanatory, enabling/disabling the default desktop wallpaper. Level 0 disables,
Level 1 enables this (and all levels above). */
void KEyeCandyPage::enableDesktopWallpaper(bool enable, bool user){
	kdesktopconf->setGroup("Desktop0");

	if( st_UserWallpaper.WallpaperMode == "NoWallpaper")
	deskbgimage="KDE34.png";

	if(enable && !user){
		// if the user has a different mode than the default of NoMulti, we don't change anyting on that.
		if( st_UserWallpaper.MultiWallpaperMode == "NoMulti" )
			kdesktopconf->writeEntry("MultiWallpaperMode", "NoMulti");
		// if the wallpaper is the new default one, set mode to scaled to leave user settings untouched
		if( deskbgimage == "KDE34.png"){
			kdesktopconf->writeEntry("WallpaperMode", "Scaled");
			//here we change the kdesktop font color to white as it fits better
			// to the KDE34png.png gray background
			kdesktopconf->setGroup("FMSettings");
			kdesktopconf->writeEntry("NormalTextColor", QColor("#FFFFFF") );
			kdesktopconf->setGroup("Desktop0");
		}
		else{
			kdesktopconf->writeEntry("WallpaperMode", st_UserWallpaper.WallpaperMode );
		}
		// write the bg image name, this is the user's image if he already set that on desktop0
		kdesktopconf->writePathEntry("Wallpaper", deskbgimage);
		kdesktopconf->setGroup("Background Common");
		// when the user set his desktop to *not* use common desktop and no wallpaper = he can have set
		// different color schemes for his desktops, we set the common desktop again to set the new
		// default wallpaper on *all* desktops.
		if(!st_UserWallpaper.CommonDesktop && (st_UserWallpaper.WallpaperMode == "NoWallpaper") )
			kdesktopconf->writeEntry("CommonDesktop", true);
		// the user set his desktop *not* to use common desktop, but *has* set a wallpaper = multiple
		// desktops with different wallpapers.
		if(!st_UserWallpaper.CommonDesktop && (!(st_UserWallpaper.WallpaperMode == "NoWallpaper")) )
			kdesktopconf->writeEntry("CommonDesktop", false);
	}
	else{
		kdesktopconf->setGroup("Desktop0");
		kdesktopconf->writeEntry("WallpaperMode", "NoWallpaper");
		kdesktopconf->setGroup("FMSettings");
		kdesktopconf->writeEntry("NormalTextColor", desktopTextColor); //restore the user's color
		kdesktopconf->setGroup("Background Common");
		// only set this to the user's setting. the default is true anyway
		if(st_UserWallpaper.WallpaperMode == "NoWallpaper")
			kdesktopconf->writeEntry("CommonDesktop", st_UserWallpaper.CommonDesktop);
		else
			kdesktopconf->writeEntry("CommonDesktop", true);
	}
	if(user){
		// reset everything
		kdesktopconf->setGroup("Desktop0");
		kdesktopconf->writeEntry("MultiWallpaperMode", st_UserWallpaper.MultiWallpaperMode);
		kdesktopconf->writeEntry("WallpaperMode", st_UserWallpaper.WallpaperMode);
		kdesktopconf->writePathEntry("Wallpaper", st_UserWallpaper.Wallpaper);
		kdesktopconf->setGroup("Background Common");
		kdesktopconf->writeEntry("CommonDesktop", st_UserWallpaper.CommonDesktop);
		kdesktopconf->setGroup("FMSettings");
		kdesktopconf->writeEntry("NormalTextColor", desktopTextColor); //restore the user's color
	}
}

/** this function enables/disables the window effects for Shading, Minimize and Restore. The contents in moving/resized windows is set in enableWindowContens(bool ) */
void KEyeCandyPage::enableDesktopWindowEffects(bool enable,bool restore){
// see /kdebase/kcontrol/kwm module, KAdvancedConfig class. Used are:
// -Animate minimize and restore
// -Animate shade
// -Enable Hover
//-Enable move/resize on maximised windows
	kwinconf->setGroup( "Windows" );
	if(!restore){
		kwinconf->writeEntry("AnimateMinimize", enable );
		kwinconf->writeEntry("AnimateShade", enable );
		kwinconf->writeEntry("MoveResizeMaximizedWindows",enable);
		kwinconf->writeEntry("ShadeHover", enable );
	} else {
		kwinconf->writeEntry("AnimateMinimize", b_AnimateMinimize );
		kwinconf->writeEntry("AnimateShade", b_AnimateShade );
		kwinconf->writeEntry("MoveResizeMaximizedWindows",b_MoveResizeMaximizedWindows);
		kwinconf->writeEntry("ShadeHover", b_ShadeHover);
	}
}

/** enable/disable window moving with contents shown */
void KEyeCandyPage::enableDesktopWindowMovingContents(bool enable, bool restore){
// see /kdebase/kcontrol/kwm module, KAdvancedConfig class. Used are:
// -Display content in moving window
// -Display content in resizing window
// And KGlobalSettings::opaqueResize() for QSplitters

	kwinconf->setGroup( "Windows" );
	KGlobal::config()->setGroup("KDE");
	if (enable){
		kwinconf->writeEntry("ResizeMode","Opaque");
		kwinconf->writeEntry("MoveMode","Opaque");
		KGlobal::config()->writeEntry("OpaqueResize", true, true, true);
	} else {
		kwinconf->writeEntry("ResizeMode","Transparent");
		kwinconf->writeEntry("MoveMode","Transparent");
		KGlobal::config()->writeEntry("OpaqueResize", false, true, true);
	}
	if(restore){
		kwinconf->writeEntry("ResizeMode",s_ResizeMode);
		kwinconf->writeEntry("MoveMode",s_MoveMode);
		KGlobal::config()->writeEntry("OpaqueResize", b_OpaqueResize, true, true);
	}
}

//---------------------------DESKTOP--------------------------------------------------

//---------------------------BACKGROUNDS--------------------------------------------------
/** Here, the background tiles/wallpapers for Konqueror and Kicker are set to the default values. Enabled in Level 2. */
void KEyeCandyPage::enableBackgroundsPanel(bool enable){
	kickerconf->setGroup("General");
	kickerconf->writeEntry("UseBackgroundTheme", enable);
}

/** Here, the background tiles/wallpapers for Konqueror and Kicker are set to the default values. Enabled in Level 2. */
void KEyeCandyPage::enableBackgroundsKonqueror(bool enable){
	konquerorconf->setGroup("Settings");
	if(enable){
		if(konqbgimage.isEmpty())
			konqbgimage="kde4ever.png";
		konquerorconf->writePathEntry("BgImage", konqbgimage);
	} else
		konquerorconf->writePathEntry("BgImage", QString::null);
}
//----------------------------BACKGROUNDS-------------------------------------------------


//----------------------------ICON STUFF-------------------------------------------------

/** Level 0-2 disable this, Level 3 and above enable this.  */
void KEyeCandyPage::enableIconZoomingPanel(bool enable){
	// Kicker Icon zooming feature. See /kdebase/kcontrol/kicker, LookAndFeelTab
	kickerconf->setGroup("buttons");
	kickerconf->writeEntry("EnableIconZoom", enable);
}

/** enable Icon highlighting,  Level 3 */
void KEyeCandyPage::enableIconEffectGamma(bool enable, bool user){
	if(enable){
		KGlobal::config()->setGroup("DesktopIcons");
		KGlobal::config()->writeEntry("ActiveEffect", "togamma", true, true);
		KGlobal::config()->writeEntry("ActiveValue", "0.7", true, true);
		KGlobal::config()->setGroup("PanelIcons");
		KGlobal::config()->writeEntry("ActiveEffect", "togamma", true, true);
		KGlobal::config()->writeEntry("ActiveValue", "0.7", true, true);
	} else {
		if(user){
			KGlobal::config()->setGroup("DesktopIcons");
			KGlobal::config()->writeEntry("ActiveEffect", st_UserGamma.EffectDesktop, true, true);
			KGlobal::config()->writeEntry("ActiveValue", st_UserGamma.ValueDesktop, true, true);
			KGlobal::config()->setGroup("PanelIcons");
			KGlobal::config()->writeEntry("ActiveEffect", st_UserGamma.EffectPanel, true, true);
			KGlobal::config()->writeEntry("ActiveValue", st_UserGamma.ValuePanel, true, true);
		} else {
			KGlobal::config()->setGroup("DesktopIcons");
			KGlobal::config()->writeEntry("ActiveEffect", "none", true, true);
			KGlobal::config()->setGroup("PanelIcons");
			KGlobal::config()->writeEntry("ActiveEffect", "none", true, true);
		}
	}
}

void KEyeCandyPage::enableIconMngAnimation(bool enable) {
	KGlobal::config()->setGroup("DesktopIcons");
	KGlobal::config()->writeEntry("Animated", enable, true, true);
}

/** No descriptions */
void KEyeCandyPage::enableIconEffectSizePanel(bool enable){
	QByteArray data;
	QDataStream stream( data, IO_WriteOnly );

	if(enable)
		stream << 56;
	else
		stream << panelsize;

	kapp->dcopClient()->send( "kicker", "Panel", "setPanelSize(int)",data);
}

/** No descriptions */
void KEyeCandyPage::enableIconEffectSizeDesktop(bool enable){
	KGlobal::config()->setGroup("DesktopIcons");
	if( enable ) {	// use 48x48 icons
		KGlobal::config()->writeEntry("Size", 48, true, true);
	} else {
		KGlobal::config()->writeEntry("Size", desktopiconsize, true, true);
	}
}
//----------------------------ICON STUFF-------------------------------------------------


//----------------------------STYLE EFFECTS-------------------------------------------------

/** Enable fading tooltips in Level 7 */
void KEyeCandyPage::enableFadingToolTips(bool enable){
	KGlobal::config()->setGroup("KDE");
	KGlobal::config()->writeEntry( "EffectFadeTooltip", enable, true, true);
}

/** enables/disables fading menus which are off by default in KDE. Enable this in Level 9 */
void KEyeCandyPage::enableFadingMenus(bool enable){
	KGlobal::config()->setGroup("KDE");
	KGlobal::config()->writeEntry("EffectFadeMenu", enable,true, true);
}

/** enables/disables icons on pushbuttons, which are off by default in KDE. Enable this in Level 5 */
void KEyeCandyPage::enablePushButtonIcons(bool enable){
	KGlobal::config()->setGroup("KDE");
	KGlobal::config()->writeEntry("ShowIconsOnPushButtons", enable,true, true);
}

/** Enable animated combo boxes, see styles kcontrol module. Enable in Level 6 (disabled by default anyway, so doesn't need to be
disabled in levels below 4) */
void KEyeCandyPage::enableAnimatedCombo(bool enable){
	KGlobal::config()->setGroup("KDE");
	KGlobal::config()->writeEntry("EffectAnimateCombo", enable, true, true);
}

/** generally enable/disable style-Effects, depending on if one of the three is enabled. */
void KEyeCandyPage::enableEffects(bool enable){
	KGlobal::config()->setGroup("KDE");
	KGlobal::config()->writeEntry("EffectsEnabled", enable, true, true);
}
//----------------------------STYLE EFFECTS-------------------------------------------------


//----------------------------PREVIEWS-------------------------------------------------
/** enables desktop/konqueror image previews, level 5 */
void KEyeCandyPage::enablePreviewImages(bool enable){
	prevImage=enable;
}

/** enables text preview in konq/kdesktop. Enable in Level 8 */
void KEyeCandyPage::enablePreviewText(bool enable){
	prevText=enable;
}

/** enables all other file previews that are available besides text and image preview. Enable in Level 9. */
void KEyeCandyPage::enablePreviewOther(bool enable){
	prevOther=enable;
}

/** as the preview entries in the rc files (konqiconviewrc and kdesktoprc) are a string list and
this list would be overwritten by the three different possibilities, we just set bool values
and ask them here, set the according string list here.
kdesktoprc's stringlist holds the preview-types wich are supposed to be shown, while konquiconviewrc's
one holds the types, wich are NOT supposed to be shown. We need to take care of that here, too.*/
void KEyeCandyPage::enablePreview(bool currSettings){
	QStringList desktopPreviews;
	QStringList konquerorNoPreviews;
	KTrader::OfferList plugins = KTrader::self()->query("ThumbCreator");
	for (KTrader::OfferList::ConstIterator it = plugins.begin(); it != plugins.end(); ++it) {
		if(prevOther){
			desktopPreviews.append((*it)->desktopEntryName());
			kdDebug() << "Adding Preview:" << (*it)->desktopEntryName() << endl;
		} else {
			konquerorNoPreviews.append((*it)->desktopEntryName());
		}
	}
	if(prevImage) {
		desktopPreviews.append("imagethumbnail");
		konquerorNoPreviews.remove("imagethumbnail");
	}

	if(prevText) {
		desktopPreviews.append("textthumbnail");
		konquerorNoPreviews.remove("textthumbnail");
	}

	if(prevOther){  // remove text/image if not checked
		if(!prevImage) {
			desktopPreviews.remove("imagethumbnail");
			konquerorNoPreviews.append("imagethumbnail");
		}
		if(!prevText) {
			desktopPreviews.remove("textthumbnail");
			konquerorNoPreviews.append("textthumbnail");
		}
	}
#if 0
	if(prevOther)
		desktopPreviews.append("audio/");
#endif
	kdesktopconf->setGroup("Desktop Icons");
	kdesktopconf->writeEntry("Preview", currSettings ? desktopPreviews : kdesktop_prev);

	konqiconconf->setGroup("Settings");
	if (prevOther || prevImage || prevText) {
		konqiconconf->writeEntry("DontPreview", currSettings ? konquerorNoPreviews : konq_dont_prev);
		konqiconconf->writeEntry("PreviewsEnabled", currSettings ? true : b_konq_prev_enable);
	} else {
		if(!currSettings)
			konqiconconf->writeEntry("DontPreview", konq_dont_prev);
		konqiconconf->writeEntry("PreviewsEnabled", currSettings ? false : b_konq_prev_enable);
	}
#if 0
	if(prevOther)
		konqiconconf->writeEntry("EnableSoundPreviews", currSettings ? true : b_konq_prev_sound);
#endif
}
//----------------------------PREVIEWS-------------------------------------------------



//----------------------------OTHER STUFF-------------------------------------------------
/** Enables the default KDE sound scheme in Level 3 */
void KEyeCandyPage::enableSoundScheme(bool enable, bool user) {
	kwineventconf->setGroup("desktop1");
	kwineventconf->writeEntry("presentation", enable ? 1 : (user ? st_UserSnd.desktop1 : 0) );
	kwineventconf->setGroup("desktop2");
	kwineventconf->writeEntry("presentation", enable ? 1 : (user ? st_UserSnd.desktop2 : 0) );
	kwineventconf->setGroup("desktop3");
	kwineventconf->writeEntry("presentation", enable ? 1 : (user ? st_UserSnd.desktop3 : 0) );
	kwineventconf->setGroup("desktop4");
	kwineventconf->writeEntry("presentation", enable ? 1 : (user ? st_UserSnd.desktop4 : 0) );
	kwineventconf->setGroup("desktop5");
	kwineventconf->writeEntry("presentation", enable ? 1 : (user ? st_UserSnd.desktop5 : 0) );
	kwineventconf->setGroup("desktop6");
	kwineventconf->writeEntry("presentation", enable ? 1 : (user ? st_UserSnd.desktop6 : 0) );
	kwineventconf->setGroup("desktop7");
	kwineventconf->writeEntry("presentation", enable ? 1 : (user ? st_UserSnd.desktop7 : 0) );
	kwineventconf->setGroup("desktop8");
	kwineventconf->writeEntry("presentation", enable ? 1 : (user ? st_UserSnd.desktop8 : 0) );

	kwineventconf->setGroup("new");
	kwineventconf->writeEntry("presentation", enable ? 1 : (user ? st_UserSnd.notold : 0) );
	kwineventconf->setGroup("close");
	kwineventconf->writeEntry("presentation", enable ? 1 : (user ? st_UserSnd.close : 0) );

	kwineventconf->setGroup("transnew");
	kwineventconf->writeEntry("presentation", enable ? 1 : (user ? st_UserSnd.transnew : 0) );
	kwineventconf->setGroup("transdelete");
	kwineventconf->writeEntry("presentation", enable ? 1 : (user ? st_UserSnd.transdelete : 0) );

	kwineventconf->setGroup("iconify");
	kwineventconf->writeEntry("presentation", enable ? 1 : (user ? st_UserSnd.iconify : 0) );
	kwineventconf->setGroup("deiconify");
	kwineventconf->writeEntry("presentation", enable ? 1 : (user ? st_UserSnd.deiconify : 0) );
	kwineventconf->setGroup("maximize");
	kwineventconf->writeEntry("presentation", enable ? 1 : (user ? st_UserSnd.maximize : 0) );
	kwineventconf->setGroup("unmaximize");
	kwineventconf->writeEntry("presentation", enable ? 1 : (user ? st_UserSnd.unmaximize : 0) );
	kwineventconf->setGroup("shadeup");
	kwineventconf->writeEntry("presentation", enable ? 1 : (user ? st_UserSnd.shadeup : 0) );
	kwineventconf->setGroup("shadedown");
	kwineventconf->writeEntry("presentation", enable ? 1 : (user ? st_UserSnd.shadedown : 0) );
	kwineventconf->setGroup("sticky");
	kwineventconf->writeEntry("presentation", enable ? 1 : (user ? st_UserSnd.sticky : 0) );
	kwineventconf->setGroup("unsticky");
	kwineventconf->writeEntry("presentation", enable ? 1 : (user ? st_UserSnd.unsticky : 0) );
}

/** Enable Antialiased fonts. Enable in Level 7. */
void KEyeCandyPage::enableAntialiasingFonts(bool enable, bool reset){
	if (!reset){
		if ( ( (sys->isXfromXFreeInc() && (sys->getXRelease() > 40000000)) || sys->isXfromXOrg() )
		     && sys->getRenderSupport()) {
			QSettings().writeEntry("/qt/useXft", enable);
			QSettings().writeEntry("/qt/enableXft", enable);
		}
	} else {
		QSettings().writeEntry("/qt/useXft", b_useXft);
		QSettings().writeEntry("/qt/enableXft", b_enableXft);
	}
}
//----------------------------OTHER STUFF-------------------------------------------------


/** save function to enable/disable the according settings that are made in the
    QCheckListItems of the Eyecandy page. */
void KEyeCandyPage::save(bool currSettings){
	kdDebug() << "KEyeCandyPage::save()" << endl;
	// currSettings==true: save selections. currSettings==false: save user-def.
	saveCheckState(currSettings);
	// save all the KConfig-objects to their respective files
	kwinconf->sync();
	kwineventconf->sync();
	konquerorconf->sync();
	konqiconconf->sync();
	kickerconf->sync();
	kdesktopconf->sync();
	KGlobal::config()->sync();
	// restart kwin  for window effects
	kapp->dcopClient()->send("knotify", "Notify", "reconfigure()", "");
	kapp->dcopClient()->send("kwin*", "", "reconfigure()", "");
	// set the display options (style effects)
	KIPC::sendMessageAll(KIPC::SettingsChanged);
	QApplication::syncX();
	// kicker stuff: Iconzooming etc.
	kapp->dcopClient()->send( "kicker", "Panel", "configure()", "" );
	// Icon stuff
	for (int i=0; i<KIcon::LastGroup; i++) {
		KIPC::sendMessageAll(KIPC::IconChanged, i);
	}
	// font stuff
	KIPC::sendMessageAll(KIPC::FontChanged);
	// unfortunately, the konqiconview does not re-read the configuration to restructure the previews and the background picture
	kapp->dcopClient()->send( "konqueror*", "KonquerorIface", "reparseConfiguration()", "" );
	kapp->dcopClient()->send( "kdesktop", "KDesktopIface", "configure()", "" );
	kapp->dcopClient()->send( "kdesktop", "KBackgroundIface", "configure()", "" );
	kapp->dcopClient()->send( "kdesktop", "KDesktopIface", "lineupIcons()", "" );
}

void KEyeCandyPage::slotEyeCandyShowDetails(bool details){
	if(details){
		klv_features->show();
	} else {
		klv_features->hide();
	}
}

/** sets the slider to the value, we think of as sane for this machine */
void KEyeCandyPage::setDefaults(){
	int speed = sys->getCpuSpeed();
	int level = 4;
	if(speed){
// Enable those higher settings, when we solved the problem with checking, if AA can be done
		if (sys->getRenderSupport()){
			if (speed > 450) level = 7;
			else if (speed > 400) level = 6;
			else if (speed > 350) level = 5;
		} else {
			if (speed > 300) level = 4;
			else if (speed > 250) level = 3;
			else if (speed > 200) level = 2;
			else level = 1;
		}
	}
	// apply the chosen level (this call emits also the moved-signal)
	sld_effects->setValue(level);
}

/** retrieves the user's local values. In case he doesn't have these set, use the default values of KDE, level 4. */
void KEyeCandyPage::getUserDefaults(){
	QByteArray replydata;
	QByteArray data;
	QCString replytype;
	kapp->dcopClient()->call( "kicker", "Panel", "panelSize()",data, replytype, replydata);
	QDataStream stream( replydata, IO_ReadOnly );
	stream >> panelsize;

	// Wallpaper-User-Defaults
	kdesktopconf->setGroup("FMSettings");
	QColor tempcolor=KGlobalSettings::textColor();
	desktopTextColor = kdesktopconf->readColorEntry("NormalTextColor", &tempcolor );
	kdesktopconf->setGroup("Background Common");
	st_UserWallpaper.CommonDesktop = kdesktopconf->readBoolEntry("CommonDesktop", true);
	kdesktopconf->setGroup("Desktop0"); // we only need to set one desktop
	st_UserWallpaper.MultiWallpaperMode = kdesktopconf->readEntry("MultiWallpaperMode", "NoMulti");
	st_UserWallpaper.WallpaperMode = kdesktopconf->readEntry("WallpaperMode", "Scaled");
	st_UserWallpaper.Wallpaper = kdesktopconf->readPathEntry("Wallpaper", "NoWallpaper");
	deskbgimage = kdesktopconf->readPathEntry("Wallpaper", "KDE34.png");
	// Wallpaper-User-Defaults (END)

	KGlobal::config()->setGroup("KDE");

	// Font-User-Defaults
	b_useXft=QSettings().readBoolEntry("/qt/useXft");
	b_enableXft=QSettings().readBoolEntry("/qt/enableXft");

	KGlobal::config()->setGroup("PanelIcons");
	st_UserGamma.EffectPanel=KGlobal::config()->readEntry("ActiveEffect", "none");
	st_UserGamma.ValuePanel=KGlobal::config()->readEntry("ActiveValue", "0.7");

	KGlobal::config()->setGroup("DesktopIcons");
	st_UserGamma.EffectDesktop=KGlobal::config()->readEntry("ActiveEffect", "none");
	st_UserGamma.ValueDesktop=KGlobal::config()->readEntry("ActiveValue", "0.7");
	desktopiconsize=KGlobal::config()->readNumEntry("Size", 32);
	b_iconMngAnim = KGlobal::config()->readBoolEntry( "Animated", true );

	KGlobal::config()->setGroup("KDE");
	b_EffectFadeTooltip=KGlobal::config()->readBoolEntry( "EffectFadeTooltip", false );
	b_PushButtonIcons=KGlobal::config()->readBoolEntry("ShowIconsOnPushButtons", false);
	b_EffectFadeMenu=KGlobal::config()->readBoolEntry("EffectFadeMenu", false);
	b_EffectAnimateCombo=KGlobal::config()->readBoolEntry("EffectAnimateCombo", false);
	b_EffectsEnabled=KGlobal::config()->readBoolEntry("EffectsEnabled", false);
	b_OpaqueResize=KGlobal::config()->readBoolEntry("OpaqueResize", true);

	kickerconf->setGroup("buttons");
	b_EnableIconZoom=kickerconf->readBoolEntry("EnableIconZoom", true);

	konquerorconf->setGroup("Settings");
	konqbgimage=konquerorconf->readPathEntry("BgImage", "");

	kdesktopconf->setGroup("Desktop Icons");
	kdesktop_prev=kdesktopconf->readListEntry("Preview");
	konqiconconf->setGroup("Settings");
	konq_dont_prev=konqiconconf->readListEntry("DontPreview");
	b_konq_prev_sound=konqiconconf->readBoolEntry("EnableSoundPreviews", false);
	b_konq_prev_enable=konqiconconf->readBoolEntry("PreviewsEnabled", true);
	kwinconf->setGroup( "Windows" );
	s_ResizeMode=kwinconf->readEntry("ResizeMode", "Transparent");
	s_MoveMode=kwinconf->readEntry("MoveMode", "Opaque");

	b_AnimateMinimize=kwinconf->readBoolEntry("AnimateMinimize", true );
	b_AnimateShade=kwinconf->readBoolEntry("AnimateShade", true );
	b_MoveResizeMaximizedWindows=kwinconf->readBoolEntry("MoveResizeMaximizedWindows",true);
	b_ShadeHover = kwinconf->readBoolEntry("ShadeHover", false);

	getUserSoundScheme();
}

/** gets the users sound-settings */
void KEyeCandyPage::getUserSoundScheme() {
	kwineventconf->setGroup("desktop1");
	st_UserSnd.desktop1 = kwineventconf->readNumEntry("presentation", 0);
	kwineventconf->setGroup("desktop2");
	st_UserSnd.desktop2 = kwineventconf->readNumEntry("presentation", 0);
	kwineventconf->setGroup("desktop3");
	st_UserSnd.desktop3	= kwineventconf->readNumEntry("presentation", 0);
	kwineventconf->setGroup("desktop4");
	st_UserSnd.desktop4 = kwineventconf->readNumEntry("presentation", 0);
	kwineventconf->setGroup("desktop5");
	st_UserSnd.desktop5	= kwineventconf->readNumEntry("presentation", 0);
	kwineventconf->setGroup("desktop6");
	st_UserSnd.desktop6 = kwineventconf->readNumEntry("presentation", 0);
	kwineventconf->setGroup("desktop7");
	st_UserSnd.desktop7	= kwineventconf->readNumEntry("presentation", 0);
	kwineventconf->setGroup("desktop8");
	st_UserSnd.desktop8 = kwineventconf->readNumEntry("presentation", 0);

	kwineventconf->setGroup("new");
	st_UserSnd.notold = kwineventconf->readNumEntry("presentation", 0);
	kwineventconf->setGroup("close");
	st_UserSnd.close = kwineventconf->readNumEntry("presentation", 0);

	kwineventconf->setGroup("transnew");
	st_UserSnd.transnew = kwineventconf->readNumEntry("presentation", 0);
	kwineventconf->setGroup("transdelete");
	st_UserSnd.transdelete = kwineventconf->readNumEntry("presentation", 0);

	kwineventconf->setGroup("iconify");
	st_UserSnd.iconify = kwineventconf->readNumEntry("presentation", 0 );
	kwineventconf->setGroup("deiconify");
	st_UserSnd.deiconify = kwineventconf->readNumEntry("presentation", 0);
	kwineventconf->setGroup("maximize");
	st_UserSnd.maximize = kwineventconf->readNumEntry("presentation", 0);
	kwineventconf->setGroup("unmaximize");
	st_UserSnd.unmaximize = kwineventconf->readNumEntry("presentation", 0);
	kwineventconf->setGroup("shadeup");
	st_UserSnd.shadeup = kwineventconf->readNumEntry("presentation", 0);
	kwineventconf->setGroup("shadedown");
	st_UserSnd.shadedown = kwineventconf->readNumEntry("presentation", 0);
	kwineventconf->setGroup("sticky");
	st_UserSnd.sticky = kwineventconf->readNumEntry("presentation", 0);
	kwineventconf->setGroup("unsticky");
	st_UserSnd.unsticky = kwineventconf->readNumEntry("presentation", 0);
}

/** calls all enable functions with the state of the checkboxes. This is needed for save() only,
as in case the user quits, we have to set these states again in saveUserDefaults to what they were
prior to running kpersonalizer */
void KEyeCandyPage::saveCheckState(bool currSettings){ // currSettings= true -> take the checkboxes, otherwise take user values set
	if(currSettings){
		enableDesktopWallpaper(desktop_wallpaper->isOn());
		enableDesktopWindowEffects(desktop_window_effects->isOn(), false);
		enableDesktopWindowMovingContents(desktop_window_moving_contents->isOn(), false);

		enableBackgroundsPanel(backgrounds_panel->isOn());
		enableBackgroundsKonqueror(backgrounds_konqueror->isOn());

		enableIconZoomingPanel(icon_zooming_panel->isOn());
		enableIconEffectGamma(icon_effect_gamma->isOn(), false);
		enableIconEffectSizePanel(icon_effect_size_panel->isOn());
		enableIconEffectSizeDesktop(icon_effect_size_desktop->isOn());
		enableIconMngAnimation(icon_mng_animation->isOn());

		enableFadingToolTips(fading_tooltips->isOn());
		enableFadingMenus(fading_menus->isOn());
		enableAnimatedCombo(animated_combo->isOn());
		enableEffects( (fading_tooltips->isOn() || fading_menus->isOn() || animated_combo->isOn()) );

		enablePushButtonIcons(pushbutton_icons->isOn());

		enablePreviewImages(preview_images->isOn());
		enablePreviewText(preview_text->isOn());
		enablePreviewOther(preview_other->isOn());
		enablePreview(true);

		enableSoundScheme(sound_scheme->isOn(), false);

		enableAntialiasingFonts(antialiasing_fonts->isOn(),false);
	} else { //  user's settings
		// restore functions
		enableDesktopWallpaper(false, true);
		enableBackgroundsKonqueror(konqbgimage.isEmpty());  // if empty
		enableIconZoomingPanel(b_EnableIconZoom);
		enableFadingToolTips(b_EffectFadeTooltip);
		enableFadingMenus(b_EffectFadeMenu);
		enableAnimatedCombo(b_EffectAnimateCombo);
		enableEffects(b_EffectsEnabled);
		enablePushButtonIcons(b_PushButtonIcons);
		enablePreview(false);
		enableAntialiasingFonts(b_useXft,true);
		enableDesktopWindowMovingContents(false, true);  // the first parameter is equal to this call
		enableDesktopWindowEffects(false, true);
		enableIconEffectSizePanel(false);
		enableIconEffectSizeDesktop(false);
		enableIconEffectGamma(false, true);
		enableIconMngAnimation(b_iconMngAnim);
		enableSoundScheme(false, true);
		enableBackgroundsPanel(false);
	}
}

/** to be connected to the OS page. changes default values of features according
	to the desktop selection*/
void KEyeCandyPage::slotPresetSlider(const QString& style){
	osStyle=style;
	slotEyeCandySliderMoved( sld_effects-> value());  //apply changes (slider is not moved)
}

#include "keyecandypage.moc"
