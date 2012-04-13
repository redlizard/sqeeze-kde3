/* kasprefsdlg.cpp
**
** Copyright (C) 2001-2004 Richard Moore <rich@kde.org>
** Contributor: Mosfet
**     All rights reserved.
**
** KasBar is dual-licensed: you can choose the GPL or the BSD license.
** Short forms of both licenses are included below.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
** OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
** SUCH DAMAGE.
*/

/*
** Bug reports and questions can be sent to kde-devel@kde.org
*/

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgrid.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qvbox.h>
#include <qwhatsthis.h>

#include <kcolorbutton.h>
#include <kconfig.h>
#include <kdialogbase.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <knuminput.h>

#include "kastasker.h"

#include "kasprefsdlg.h"
#include "kasprefsdlg.moc"

#define Icon(x) KGlobal::iconLoader()->loadIcon( x, KIcon::NoGroup, KIcon::SizeMedium )
#define LargeIcon(x) KGlobal::iconLoader()->loadIcon( x, KIcon::NoGroup, KIcon::SizeLarge )


KasPrefsDialog::KasPrefsDialog( KasTasker *kas, QWidget *parent )
   : KDialogBase( KDialogBase::IconList, i18n("Kasbar Preferences"),
		  KDialogBase::Ok | KDialogBase::Cancel,
		  KDialogBase::Ok,
		  parent, "kasbarPrefsDialog", /*true*/false ),
     kasbar( kas ),
     res( kas->resources() )
{
   addLookPage();
   addBackgroundPage();
   addThumbsPage();
   addBehavePage();
//   addIndicatorsPage();
   addColorsPage();
   addAdvancedPage();

   resize( 470, 500 );
}

KasPrefsDialog::~KasPrefsDialog()
{

}

void KasPrefsDialog::itemSizeChanged( int sz )
{
    customSize->setEnabled( sz == KasBar::Custom );
}

void KasPrefsDialog::addLookPage()
{
   QVBox *lookPage = addVBoxPage( i18n("Appearance"), QString::null, Icon( "appearance" ) );

   //
   // Item size
   //

   QGrid *itemSizeBox = new QGrid( 2, lookPage );
   itemSizeBox->setSpacing( spacingHint() );

   QWhatsThis::add( itemSizeBox,
		    i18n( "Specifies the size of the task items." ) );

   QLabel *itemSizeLabel = new QLabel( i18n("Si&ze:"), itemSizeBox );

   itemSizeCombo = new QComboBox( itemSizeBox );
   itemSizeCombo->insertItem( i18n( "Enormous" ) );
   itemSizeCombo->insertItem( i18n( "Huge" ) );
   itemSizeCombo->insertItem( i18n( "Large" ) );
   itemSizeCombo->insertItem( i18n( "Medium" ) );
   itemSizeCombo->insertItem( i18n( "Small" ) );
   itemSizeCombo->insertItem( i18n( "Custom" ) );

   itemSizeLabel->setBuddy( itemSizeCombo );

   connect( itemSizeCombo, SIGNAL( activated( int ) ),
	    kasbar, SLOT( setItemSize( int ) ) );
   connect( itemSizeCombo, SIGNAL( activated( int ) ), SLOT( itemSizeChanged( int ) ) );

   new QWidget( itemSizeBox );

   customSize = new QSpinBox( 5, 1000, 1, itemSizeBox );

   customSize->setValue( kasbar->itemExtent() );

   connect( customSize, SIGNAL( valueChanged( int ) ),
	    kasbar, SLOT( setItemExtent( int ) ) );
   connect( customSize, SIGNAL( valueChanged( int ) ),
	    kasbar, SLOT( customSizeChanged( int ) ) );

   int sz = kasbar->itemSize();
   itemSizeCombo->setCurrentItem( sz );
   customSize->setEnabled( sz == KasBar::Custom );

   //
   // Boxes per line
   //

   QHBox *maxBoxesBox = new QHBox( lookPage );
   QWhatsThis::add( maxBoxesBox,
		    i18n( "Specifies the maximum number of items that should be placed in a line "
			  "before starting a new row or column. If the value is 0 then all the "
			  "available space will be used." ) );
   QLabel *maxBoxesLabel = new QLabel( i18n("Bo&xes per line: "), maxBoxesBox );

   KConfig *conf = kasbar->config();
   if ( conf )
       conf->setGroup( "Layout" );
   maxBoxesSpin = new KIntSpinBox( 0, 50, 1,
				   conf ? conf->readNumEntry( "MaxBoxes", 0 ) : 11,
				   10,
				   maxBoxesBox, "maxboxes" );
   connect( maxBoxesSpin, SIGNAL( valueChanged( int ) ), kasbar, SLOT( setMaxBoxes( int ) ) );
   maxBoxesLabel->setBuddy( maxBoxesSpin );

   //
   // Mode
   //

   detachedCheck = new QCheckBox( i18n("&Detach from screen edge"), lookPage );
   QWhatsThis::add( detachedCheck, i18n( "Detaches the bar from the screen edge and makes it draggable." ) );

   detachedCheck->setEnabled( !kasbar->isStandAlone() );
   detachedCheck->setChecked( kasbar->isDetached() );
   connect( detachedCheck, SIGNAL( toggled(bool) ), kasbar, SLOT( setDetached(bool) ) );

   (void) new QWidget( lookPage, "spacer" );
   (void) new QWidget( lookPage, "spacer" );
   (void) new QWidget( lookPage, "spacer" );
}

void KasPrefsDialog::addBackgroundPage()
{
   QVBox *bgPage = addVBoxPage( i18n("Background"), QString::null, Icon( "background" ) );

   transCheck = new QCheckBox( i18n("Trans&parent"), bgPage );
   QWhatsThis::add( transCheck, i18n( "Enables pseudo-transparent mode." ) );
   transCheck->setChecked( kasbar->isTransparent() );
   connect( transCheck, SIGNAL( toggled(bool) ), kasbar, SLOT( setTransparent(bool) ) );

   tintCheck = new QCheckBox( i18n("Enable t&int"), bgPage );
   QWhatsThis::add( tintCheck,
		    i18n( "Enables tinting the background that shows through in transparent mode." ) );
   tintCheck->setChecked( kasbar->hasTint() );
   connect( tintCheck, SIGNAL( toggled(bool) ), kasbar, SLOT( setTint(bool) ) );

   QHBox *tintColBox = new QHBox( bgPage );
   QWhatsThis::add( tintColBox,
		    i18n( "Specifies the color used for the background tint." ) );
   connect( tintCheck, SIGNAL( toggled(bool) ), tintColBox, SLOT( setEnabled(bool) ) );
   tintColBox->setEnabled( kasbar->hasTint() );

   QLabel *tintLabel = new QLabel( i18n("Tint &color:"), tintColBox );

   tintButton = new KColorButton( kasbar->tintColor(), tintColBox );
   connect( tintButton, SIGNAL( changed( const QColor & ) ),
	    kasbar, SLOT( setTintColor( const QColor & ) ) );
   tintLabel->setBuddy( tintButton );

   QHBox *tintAmtBox = new QHBox( bgPage );
   QWhatsThis::add( tintAmtBox,
		    i18n( "Specifies the strength of the background tint." ) );
   connect( tintCheck, SIGNAL( toggled(bool) ), tintAmtBox, SLOT( setEnabled(bool) ) );
   tintAmtBox->setEnabled( kasbar->hasTint() );

   QLabel *tintStrengthLabel = new QLabel( i18n("Tint &strength: "), tintAmtBox );

   int percent = (int) (kasbar->tintAmount() * 100.0);
   tintAmount = new QSlider( 0, 100, 1, percent, Horizontal, tintAmtBox );
   tintAmount->setTracking( true );
   connect( tintAmount, SIGNAL( valueChanged( int ) ),
	    kasbar, SLOT( setTintAmount( int ) ) );
   tintStrengthLabel->setBuddy( tintAmount );

   (void) new QWidget( bgPage, "spacer" );
   (void) new QWidget( bgPage, "spacer" );
   (void) new QWidget( bgPage, "spacer" );
}

void KasPrefsDialog::addThumbsPage()
{
   QVBox *thumbsPage = addVBoxPage( i18n("Thumbnails"), QString::null, Icon( "icons" ) );

   thumbsCheck = new QCheckBox( i18n("Enable thu&mbnails"), thumbsPage );
   QWhatsThis::add( thumbsCheck,
		    i18n( "Enables the display of a thumbnailed image of the window when "
			  "you move your mouse pointer over an item. The thumbnails are "
			  "approximate, and may not reflect the current window contents.\n\n"
			  "Using this option on a slow machine may cause performance problems." ) );
   thumbsCheck->setChecked( kasbar->thumbnailsEnabled() );
   connect( thumbsCheck, SIGNAL( toggled(bool) ), kasbar, SLOT( setThumbnailsEnabled(bool) ) );

   embedThumbsCheck = new QCheckBox( i18n("&Embed thumbnails"), thumbsPage );
   embedThumbsCheck->setChecked( kasbar->embedThumbnails() );
   connect( embedThumbsCheck, SIGNAL( toggled(bool) ), kasbar, SLOT( setEmbedThumbnails(bool) ) );

   QHBox *thumbSizeBox = new QHBox( thumbsPage );
   QWhatsThis::add( thumbSizeBox,
		    i18n( "Controls the size of the window thumbnails. Using large sizes may "
			  "cause performance problems." ) );
   QLabel *thumbSizeLabel = new QLabel( i18n("Thumbnail &size: "), thumbSizeBox );
   int percent = (int) (kasbar->thumbnailSize() * 100.0);
   thumbSizeSlider = new QSlider( 0, 100, 1, percent, Horizontal, thumbSizeBox );
   connect( thumbSizeSlider, SIGNAL( valueChanged( int ) ),
	    kasbar, SLOT( setThumbnailSize( int ) ) );
   thumbSizeLabel->setBuddy( thumbSizeSlider );

   QHBox *thumbUpdateBox = new QHBox( thumbsPage );
   thumbUpdateBox->setSpacing( spacingHint() );
   QWhatsThis::add( thumbUpdateBox,
		    i18n( "Controls the frequency with which the thumbnail of the active window "
			  "is updated. If the value is 0 then no updates will be performed.\n\n"
			  "Using small values may cause performance problems on slow machines." ) );
   QLabel *thumbUpdateLabel = new QLabel( i18n("&Update thumbnail every: "), thumbUpdateBox );
   thumbUpdateSpin = new QSpinBox( 0, 1000, 1, thumbUpdateBox );
   thumbUpdateSpin->setValue( kasbar->thumbnailUpdateDelay() );
   connect( thumbUpdateSpin, SIGNAL( valueChanged( int ) ),
   	    kasbar, SLOT( setThumbnailUpdateDelay( int ) ) );
   (void) new QLabel( i18n("seconds"), thumbUpdateBox );
   thumbUpdateLabel->setBuddy( thumbUpdateSpin );

   (void) new QWidget( thumbsPage, "spacer" );
   (void) new QWidget( thumbsPage, "spacer" );
   (void) new QWidget( thumbsPage, "spacer" );
}

void KasPrefsDialog::addBehavePage()
{
   QVBox *behavePage = addVBoxPage( i18n("Behavior"), QString::null, Icon( "window_list" ) );

   groupWindowsCheck = new QCheckBox( i18n("&Group windows"), behavePage );
   QWhatsThis::add( groupWindowsCheck,
		    i18n( "Enables the grouping together of related windows." ) );
   groupWindowsCheck->setChecked( kasbar->groupWindows() );
   connect( groupWindowsCheck, SIGNAL( toggled(bool) ),
	    kasbar, SLOT( setGroupWindows(bool) ) );

   showAllWindowsCheck = new QCheckBox( i18n("Show all &windows"), behavePage );
   QWhatsThis::add( showAllWindowsCheck,
		    i18n( "Enables the display of all windows, not just those on the current desktop." ) );
   showAllWindowsCheck->setChecked( kasbar->showAllWindows() );
   connect( showAllWindowsCheck, SIGNAL( toggled(bool) ), kasbar, SLOT( setShowAllWindows(bool) ) );

   groupInactiveCheck = new QCheckBox( i18n("&Group windows on inactive desktops"), behavePage );
   QWhatsThis::add( groupInactiveCheck,
		    i18n( "Enables the grouping together of windows that are not on the current desktop." ) );
   groupInactiveCheck->setChecked( kasbar->groupInactiveDesktops() );
   connect( groupInactiveCheck, SIGNAL( toggled(bool) ),
	    kasbar, SLOT( setGroupInactiveDesktops(bool) ) );

   onlyShowMinimizedCheck = new QCheckBox( i18n("Only show &minimized windows"), behavePage );
   QWhatsThis::add( onlyShowMinimizedCheck,
		    i18n( "When this option is checked only minimized windows are shown in the bar. " \
			  "This gives Kasbar similar behavior to the icon handling in older environments " \
			  "like CDE or OpenLook." ) );
   onlyShowMinimizedCheck->setChecked( kasbar->onlyShowMinimized() );
   connect( onlyShowMinimizedCheck, SIGNAL( toggled(bool) ), kasbar, SLOT( setOnlyShowMinimized(bool) ) );

   (void) new QWidget( behavePage, "spacer" );
   (void) new QWidget( behavePage, "spacer" );
}

void KasPrefsDialog::addColorsPage()
{
   QVBox *colorsPage = addVBoxPage( i18n("Colors"), QString::null, Icon( "colors" ) );

   // Item label colors
   QGrid *group = new QGrid( 2, colorsPage );

   QLabel *labelPenLabel = new QLabel( i18n("Label foreground:"), group );

   labelPenButton = new KColorButton( res->labelPenColor(), group );
   connect( labelPenButton, SIGNAL( changed( const QColor & ) ),
	    res, SLOT( setLabelPenColor( const QColor & ) ) );
   labelPenLabel->setBuddy( labelPenButton );

   QLabel *labelBackgroundLabel = new QLabel( i18n("Label background:"), group );
   labelBackgroundButton = new KColorButton( res->labelBgColor(), group );
   connect( labelBackgroundButton, SIGNAL( changed( const QColor & ) ),
	    res, SLOT( setLabelBgColor( const QColor & ) ) );
   labelBackgroundLabel->setBuddy( labelBackgroundButton );

   // Inactive colors
   group = new QGrid( 2, colorsPage );

   QLabel *inactivePenLabel = new QLabel( i18n("Inactive foreground:"), group );
   inactivePenButton = new KColorButton( res->inactivePenColor(), group );
   connect( inactivePenButton, SIGNAL( changed( const QColor & ) ),
	    res, SLOT( setInactivePenColor( const QColor & ) ) );
   inactivePenLabel->setBuddy( inactivePenButton );

   QLabel *inactiveBgLabel = new QLabel( i18n("Inactive background:"), group );
   inactiveBgButton = new KColorButton( res->inactiveBgColor(), group );
   connect( inactiveBgButton, SIGNAL( changed( const QColor & ) ),
	    res, SLOT( setInactiveBgColor( const QColor & ) ) );
   inactiveBgLabel->setBuddy( inactiveBgButton );

   // Active colors
   group = new QGrid( 2, colorsPage );

   QLabel *activePenLabel = new QLabel( i18n("Active foreground:"), group );
   activePenButton = new KColorButton( res->activePenColor(), group );
   connect( activePenButton, SIGNAL( changed( const QColor & ) ),
	    res, SLOT( setActivePenColor( const QColor & ) ) );
   activePenLabel->setBuddy( activePenButton );

   QLabel *activeBgLabel = new QLabel( i18n("Active background:"), group );
   activeBgButton = new KColorButton( res->activeBgColor(), group );
   connect( activeBgButton, SIGNAL( changed( const QColor & ) ),
	    res, SLOT( setActiveBgColor( const QColor & ) ) );
   activeBgLabel->setBuddy( activeBgButton );

   group = new QGrid( 2, colorsPage );

   QLabel *progressLabel = new QLabel( i18n("&Progress color:"), group );
   progressButton = new KColorButton( res->progressColor(), group );
   connect( progressButton, SIGNAL( changed( const QColor & ) ),
	    res, SLOT( setProgressColor( const QColor & ) ) );
   progressLabel->setBuddy( progressButton );

   QLabel *attentionLabel = new QLabel( i18n("&Attention color:"), group );
   attentionButton = new KColorButton( res->attentionColor(), group );
   connect( attentionButton, SIGNAL( changed( const QColor & ) ),
	    res, SLOT( setAttentionColor( const QColor & ) ) );
   attentionLabel->setBuddy( attentionButton );

   (void) new QWidget( colorsPage, "spacer" );
}

void KasPrefsDialog::addIndicatorsPage()
{
   QVBox *indicatorsPage = addVBoxPage( i18n("Indicators"), QString::null, Icon( "bell" ) );

   (void) new QWidget( indicatorsPage, "spacer" );
   (void) new QWidget( indicatorsPage, "spacer" );
}

void KasPrefsDialog::addAdvancedPage()
{
   QVBox *advancedPage = addVBoxPage( i18n("Advanced"), QString::null, Icon( "misc" ) );

   // Startup notifier
   notifierCheck = new QCheckBox( i18n("Enable &startup notifier"), advancedPage );
   QWhatsThis::add( notifierCheck,
		    i18n( "Enables the display of tasks that are starting but have not yet "
			  "created a window." ) );
   notifierCheck->setChecked( kasbar->notifierEnabled() );
   connect( notifierCheck, SIGNAL( toggled(bool) ), kasbar, SLOT( setNotifierEnabled(bool) ) );

   // Status advanced
   modifiedCheck = new QCheckBox( i18n("Enable &modified indicator"), advancedPage );
   QWhatsThis::add( modifiedCheck,
		    i18n( "Enables the display of a floppy disk state icon for windows containing "
			  "a modified document." ) );
   modifiedCheck->setChecked( kasbar->showModified() );
   connect( modifiedCheck, SIGNAL( toggled(bool) ), kasbar, SLOT( setShowModified(bool) ) );

   progressCheck = new QCheckBox( i18n("Enable &progress indicator"), advancedPage );
   QWhatsThis::add( progressCheck,
		    i18n( "Enables the display of a progress bar in the label of windows show "
			  "are progress indicators." ) );
   progressCheck->setChecked( kasbar->showProgress() );
   connect( progressCheck, SIGNAL( toggled(bool) ), kasbar, SLOT( setShowProgress(bool) ) );

   attentionCheck = new QCheckBox( i18n("Enable &attention indicator"), advancedPage );
   QWhatsThis::add( attentionCheck,
		    i18n( "Enables the display of an icon that indicates a window that needs attention." ) );
   attentionCheck->setChecked( kasbar->showAttention() );
   connect( attentionCheck, SIGNAL( toggled(bool) ), kasbar, SLOT( setShowAttention(bool) ) );

   inactiveFramesCheck = new QCheckBox( i18n("Enable frames for inactive items"), advancedPage );
   QWhatsThis::add( inactiveFramesCheck,
		    i18n( "Enables frames around inactive items, if you want the bar to disappear into " \
			  "the background you should probably uncheck this option." ) );
   inactiveFramesCheck->setChecked( kasbar->paintInactiveFrames() );
   connect( inactiveFramesCheck, SIGNAL( toggled(bool) ), kasbar, SLOT( setPaintInactiveFrames(bool) ) );

   (void) new QWidget( advancedPage, "spacer" );
   (void) new QWidget( advancedPage, "spacer" );
}

void KasPrefsDialog::customSizeChanged ( int value )
{
   customSize->setSuffix( i18n(" pixel", " pixels", value) );
}

void KasPrefsDialog::accept()
{
   KConfig *conf = kasbar->config();
   if ( conf ) {
       kasbar->writeConfig( conf );

       conf->setGroup("Layout");
       // TODO: This needs to be made independent of the gui and moved to kastasker
       conf->writeEntry( "MaxBoxes", maxBoxesSpin->value() );

       conf->sync();
   }

   QDialog::accept();
}

void KasPrefsDialog::reject()
{
   kasbar->readConfig();
   QDialog::reject();
}
