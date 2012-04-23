///////////////////////////////////////////////////////////////////////
// -------------------
// DeKorator window decoration for KDE
// -------------------
// Copyright (C) 2005  moty rahamim <moty.rahamim@gmail.com>
//
// Based on Example-0.8, some ideas and code have been taken from plastik and KCMicons
//
// Example window decoration for KDE
// Copyright (c) 2003, 2004 David Johnson <david@usermode.org>
//
// Plastik KWin window decoration
// Copyright (C) 2003-2005 Sandro Giessl <sandro@giessl.com>
//
// KCMicons for KDE
// Copyright (c) 2000 Antonio Larrosa <larrosa@kde.org>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; see the file COPYING.  If not, write to
// the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
// Boston, MA 02110-1301, USA.
///////////////////////////////////////////////////////////////////////

#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
#include <qwhatsthis.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <kcolorbutton.h>
#include <klistview.h>

#include "deKoratorconfig.h"
#include "configdialog.h"
#include <themes.h>

#include <kurlrequester.h>

//////////////////////////////////////////////////////////////////////////////
// DeKoratorConfig()
// -------------
// Constructor

DeKoratorConfig::DeKoratorConfig( KConfig* config, QWidget* parent )
        : QObject( parent ), config_( 0 ), dialog_( 0 )
{
    // create the configuration object
    config_ = new KConfig( "kwindeKoratorrc" );
    KGlobal::locale() ->insertCatalogue( "kwin_deKorator_config" );



    // create and show the configuration dialog
    dialog_ = new ConfigDialog( parent );

    themes_ = new IconThemesConfig( dialog_, dialog_->themesKlstView /*, dialog_->removeThemBtn*/ );

    dialog_->show();

    // load the configuration
    load( config_ );

    // setup the connections

    // misc
    connect( dialog_->titlealign, SIGNAL( clicked( int ) ),
             this, SLOT( selectionChanged( int ) ) );
    connect( dialog_->useMenuImageChkBox, SIGNAL( clicked() ), SIGNAL( changed() ) );
	connect( dialog_->ignoreAppIcnCol, SIGNAL( clicked() ), SIGNAL( changed() ) );
    connect( dialog_->dblClkCloseChkBox, SIGNAL( clicked() ), SIGNAL( changed() ) );
    connect( dialog_->showBtmBorderChkBox, SIGNAL( clicked() ), SIGNAL( changed() ) );
    connect( dialog_->useShdtextChkBox, SIGNAL( clicked() ), SIGNAL( changed() ) );
    connect( dialog_->activeShdtextXSpinBox, SIGNAL( valueChanged( int ) ), SIGNAL( changed() ) );
    connect( dialog_->activeShdtextYSpinBox, SIGNAL( valueChanged( int ) ), SIGNAL( changed() ) );
    connect( dialog_->activeShdColBtn, SIGNAL( clicked() ), SIGNAL( changed() ) );
    connect( dialog_->inActiveShdtextXSpinBox, SIGNAL( valueChanged( int ) ), SIGNAL( changed() ) );
    connect( dialog_->inActiveShdtextYSpinBox, SIGNAL( valueChanged( int ) ), SIGNAL( changed() ) );
    connect( dialog_->inActiveShdColBtn, SIGNAL( clicked() ), SIGNAL( changed() ) );
    connect( dialog_->inActiveShdColBtn, SIGNAL( clicked( ) ), SIGNAL( changed() ) );
    connect( dialog_->btnShiftXSpinBox, SIGNAL( valueChanged( int ) ), SIGNAL( changed() ) );
    connect( dialog_->btnShiftYSpinBox, SIGNAL( valueChanged( int ) ), SIGNAL( changed() ) );


    // colors
    connect( dialog_->colorizeActFramesChkBox, SIGNAL( clicked() ), SIGNAL( changed() ) );
    connect( dialog_->colorizeActButtonsChkBox, SIGNAL( clicked() ), SIGNAL( changed() ) );
    connect( dialog_->colorizeInActFramesChkBox, SIGNAL( clicked() ), SIGNAL( changed() ) );
    connect( dialog_->colorizeInActButtonsChkBox, SIGNAL( clicked() ), SIGNAL( changed() ) );
    connect( dialog_->useAnimChkBox, SIGNAL( clicked() ), SIGNAL( changed() ) );
    connect( dialog_->animateEfeectsCombo, SIGNAL( activated( const QString& ) ), SIGNAL( changed() ) );

    connect( dialog_->stepsSpinBox, SIGNAL( valueChanged( int ) ), SIGNAL( changed() ) );
    connect( dialog_->intervalSpinBox, SIGNAL( valueChanged( int ) ), SIGNAL( changed() ) );
    connect( dialog_->KeepAnimateChkBox, SIGNAL( clicked() ), SIGNAL( changed() ) );

    connect( dialog_->hoverTypeCombo, SIGNAL( activated( const QString& ) ), SIGNAL( changed() ) );
    connect( dialog_->effectsAmount_SpinBox, SIGNAL( valueChanged( int ) ), SIGNAL( changed() ) );
    connect( dialog_->activeHighlightClrBtn, SIGNAL( clicked( ) ), SIGNAL( changed() ) );
    connect( dialog_->inActiveHighlightClrBtn, SIGNAL( clicked( ) ), SIGNAL( changed() ) );
    connect( dialog_->decoColorizeComboBox, SIGNAL( activated( const QString& ) ), SIGNAL( changed() ) );
    connect( dialog_->buttonsColorizeComboBox, SIGNAL( activated( const QString& ) ), SIGNAL( changed() ) );
    //  connect( dialog_->buttonsHoverColorizeComboBox, SIGNAL( activated( const QString& ) ), SIGNAL( changed() ) );

    // buttons
    connect( dialog_->useCusBtnClrChkBox, SIGNAL( clicked() ), SIGNAL( changed() ) );
    connect( dialog_->cusColActBtnChkBox, SIGNAL( clicked() ), SIGNAL( changed() ) );
    connect( dialog_->cusColInActBtnChkBox, SIGNAL( clicked() ), SIGNAL( changed() ) );

    connect( dialog_->closeClrBtn, SIGNAL( clicked( ) ), SIGNAL( changed() ) );
    connect( dialog_->minClrBtn, SIGNAL( clicked( ) ), SIGNAL( changed() ) );
    connect( dialog_->maxClrBtn, SIGNAL( clicked( ) ), SIGNAL( changed() ) );
    connect( dialog_->restoreClrBtn, SIGNAL( clicked( ) ), SIGNAL( changed() ) );
    connect( dialog_->helpClrBtn, SIGNAL( clicked( ) ), SIGNAL( changed() ) );
    connect( dialog_->menuClrBtn, SIGNAL( clicked( ) ), SIGNAL( changed() ) );
    connect( dialog_->stickyClrBtn, SIGNAL( clicked( ) ), SIGNAL( changed() ) );
    connect( dialog_->stickyDownClrBtn, SIGNAL( clicked( ) ), SIGNAL( changed() ) );
    connect( dialog_->aboveClrBtn, SIGNAL( clicked( ) ), SIGNAL( changed() ) );
    connect( dialog_->aboveDownClrBtn, SIGNAL( clicked( ) ), SIGNAL( changed() ) );
    connect( dialog_->belowClrBtn, SIGNAL( clicked( ) ), SIGNAL( changed() ) );
    connect( dialog_->belowDownClrBtn, SIGNAL( clicked( ) ), SIGNAL( changed() ) );
    connect( dialog_->shadeClrBtn, SIGNAL( clicked( ) ), SIGNAL( changed() ) );
    connect( dialog_->shadeDownClrBtn, SIGNAL( clicked( ) ), SIGNAL( changed() ) );

    // path's
    connect( dialog_->framesPathKurl, SIGNAL( textChanged( const QString& ) ), SIGNAL( changed() ) );
    dialog_->framesPathKurl->setMode( 2 );
    connect( dialog_->buttonsPathKurl, SIGNAL( textChanged( const QString& ) ), SIGNAL( changed() ) );
    dialog_->buttonsPathKurl->setMode( 2 );
    connect( dialog_->useMasks_Chkbox, SIGNAL( clicked() ), SIGNAL( changed() ) );
    connect( dialog_->masksPathKurl, SIGNAL( textChanged( const QString& ) ), SIGNAL( changed() ) );
    dialog_->masksPathKurl->setMode( 2 );

    // themes
    connect( dialog_->themesKlstView, SIGNAL( selectionChanged( QListViewItem * ) ),
             SLOT( themeSelected( QListViewItem * ) ) );
    connect( dialog_->installthemeBtn, SIGNAL( clicked( ) ), SLOT( installNewTheme() ) );
    connect( dialog_->removeThemBtn, SIGNAL( clicked( ) ), SLOT( removeSelectedTheme() ) );
    connect( dialog_->applyThemeBtn, SIGNAL( clicked( ) ), SLOT( setTheme() ) );

}

//////////////////////////////////////////////////////////////////////////////
// ~DeKoratorConfig()
// --------------
// Destructor

DeKoratorConfig::~DeKoratorConfig()
{
    if ( dialog_ ) delete dialog_;
    if ( config_ ) delete config_;
    if ( themes_ ) delete themes_;
}

//////////////////////////////////////////////////////////////////////////////
// selectionChanged()
// ------------------
// Selection has changed

void DeKoratorConfig::selectionChanged( int )
{
    emit changed();
}

//////////////////////////////////////////////////////////////////////////////
// load()
// ------
// Load configuration data

void DeKoratorConfig::load( KConfig* )
{
    // misc
    config_->setGroup( "MISC" );

    QString value = config_->readEntry( "TitleAlignment", "AlignHCenter" );
    QRadioButton *button = ( QRadioButton* ) dialog_->titlealign->child( value );
    if ( button ) button->setChecked( TRUE );
    dialog_->useMenuImageChkBox->setChecked( config_->readBoolEntry( "UseMenuImage", FALSE ) );
	dialog_->ignoreAppIcnCol->setChecked( config_->readBoolEntry( "IgnoreAppIconCol", FALSE ) );
    dialog_->dblClkCloseChkBox->setChecked( config_->readBoolEntry( "DblClkClose", FALSE ) );
    dialog_->showBtmBorderChkBox->setChecked( config_->readBoolEntry( "ShowBtmBorder", FALSE ) );
    QColor color = QColor( 150, 150, 150 );
    dialog_->useShdtextChkBox->setChecked( config_->readBoolEntry( "UseShdtext", FALSE ) );
    dialog_->activeShdtextXSpinBox->setValue( config_->readNumEntry( "ActiveShdtextX", 0 ) );
    dialog_->activeShdtextYSpinBox->setValue( config_->readNumEntry( "ActiveShdtextY", 0 ) );
    dialog_->activeShdColBtn->setColor( config_->readColorEntry( "ActiveShadowColor", &color ) );
    dialog_->inActiveShdtextXSpinBox->setValue( config_->readNumEntry( "InActiveShdtextX", 0 ) );
    dialog_->inActiveShdtextYSpinBox->setValue( config_->readNumEntry( "InActiveShdtextY", 0 ) );
    dialog_->inActiveShdColBtn->setColor( config_->readColorEntry( "InActiveShadowColor", &color ) );
    dialog_->btnShiftXSpinBox->setValue( config_->readNumEntry( "BtnShiftX", 0 ) );
    dialog_->btnShiftYSpinBox->setValue( config_->readNumEntry( "BtnShiftY", 0 ) );


    // colors
    config_->setGroup( "COLORS" );

    dialog_->colorizeActFramesChkBox->setChecked( config_->readBoolEntry( "ColorizeActFrames", FALSE ) );
    dialog_->colorizeActButtonsChkBox->setChecked( config_->readBoolEntry( "ColorizeActButtons", FALSE ) );
    dialog_->colorizeInActFramesChkBox->setChecked( config_->readBoolEntry( "ColorizeInActFrames", FALSE ) );
    dialog_->colorizeInActButtonsChkBox->setChecked( config_->readBoolEntry( "ColorizeInActButtons", FALSE ) );
    dialog_->useAnimChkBox->setChecked( config_->readBoolEntry( "UseAnimation", FALSE ) );
    dialog_->animateEfeectsCombo->setCurrentText( config_->readEntry( "AnimationType", "Intensity" ) );

    dialog_->stepsSpinBox->setValue( config_->readNumEntry( "AnimSteps", 5 ) );
    dialog_->intervalSpinBox->setValue( config_->readNumEntry( "AnimInterval", 30 ) );
    dialog_->KeepAnimateChkBox->setChecked( config_->readBoolEntry( "KeepAnimating", FALSE ) );

    dialog_->activeHighlightClrBtn->setColor( config_->readColorEntry( "ActiveHighlightColor", &color ) );
    dialog_->inActiveHighlightClrBtn->setColor( config_->readColorEntry( "InActiveHighlightColor", &color ) );
    dialog_->hoverTypeCombo->setCurrentText( config_->readEntry( "ButtonsHoverType", "To Gray" ) );
    dialog_->effectsAmount_SpinBox->setValue( config_->readNumEntry( "EffectAmount", 5 ) );
    dialog_->decoColorizeComboBox->setCurrentText( config_->readEntry( "DecoColorize", "Liquid Method" ) );
    dialog_->buttonsColorizeComboBox->setCurrentText( config_->readEntry( "ButtonsColorize", "Liquid Method" ) );
    //  dialog_->buttonsHoverColorizeComboBox->setCurrentText( config_->readEntry( "ButtonsHoverColorize", "Liquid Method" ) );

    // buttons
    config_->setGroup( "BUTTONS" );

    dialog_->useCusBtnClrChkBox->setChecked( config_->readBoolEntry( "UseCustomButtonsColors", FALSE ) );
    dialog_->cusColActBtnChkBox->setChecked( config_->readBoolEntry( "customColorsActiveButtons", FALSE ) );
    dialog_->cusColInActBtnChkBox->setChecked( config_->readBoolEntry( "customColorsInActiveButtons", FALSE ) );
    dialog_->closeClrBtn->setColor( config_->readColorEntry( "CloseButtonColor", &color ) );
    dialog_->minClrBtn->setColor( config_->readColorEntry( "MinButtonColor", &color ) );
    dialog_->maxClrBtn->setColor( config_->readColorEntry( "MaxButtonColor", &color ) );
    dialog_->restoreClrBtn->setColor( config_->readColorEntry( "RestoreButtonColor", &color ) );
    dialog_->helpClrBtn->setColor( config_->readColorEntry( "HelpButtonColor", &color ) );
    dialog_->menuClrBtn->setColor( config_->readColorEntry( "MenuButtonColor", &color ) );
    dialog_->stickyClrBtn->setColor( config_->readColorEntry( "StickyButtonColor", &color ) );
    dialog_->stickyDownClrBtn->setColor( config_->readColorEntry( "StickyDownButtonColor", &color ) );
    dialog_->aboveClrBtn->setColor( config_->readColorEntry( "AboveButtonColor", &color ) );
    dialog_->aboveDownClrBtn->setColor( config_->readColorEntry( "AboveDownButtonColor", &color ) );
    dialog_->belowClrBtn->setColor( config_->readColorEntry( "BelowButtonColor", &color ) );
    dialog_->belowDownClrBtn->setColor( config_->readColorEntry( "BelowDownButtonColor", &color ) );
    dialog_->shadeClrBtn->setColor( config_->readColorEntry( "ShadeButtonColor", &color ) );
    dialog_->shadeDownClrBtn->setColor( config_->readColorEntry( "ShadeDownButtonColor", &color ) );

    // path's
    config_->setGroup( "PATHS" );

    dialog_->framesPathKurl->setURL( config_->readEntry( "FramesPath", "" ) );
    dialog_->buttonsPathKurl->setURL( config_->readEntry( "ButtonsPath", "" ) );
    dialog_->useMasks_Chkbox->setChecked( config_->readBoolEntry( "UseMasks", FALSE ) );
    dialog_->masksPathKurl->setURL( config_->readEntry( "MasksPath", "" ) );
}

//////////////////////////////////////////////////////////////////////////////
// save()
// ------
// Save configuration data

void DeKoratorConfig::save( KConfig* )
{
    // misc
    config_->setGroup( "MISC" );

    QRadioButton *button = ( QRadioButton* ) dialog_->titlealign->selected();
    if ( button ) config_->writeEntry( "TitleAlignment", QString( button->name() ) );
    config_->writeEntry( "UseMenuImage", dialog_->useMenuImageChkBox->isChecked() );
	config_->writeEntry( "IgnoreAppIconCol", dialog_->ignoreAppIcnCol->isChecked() );
    config_->writeEntry( "DblClkClose", dialog_->dblClkCloseChkBox->isChecked() );
    config_->writeEntry( "ShowBtmBorder", dialog_->showBtmBorderChkBox->isChecked() );
    config_->writeEntry( "UseShdtext", dialog_->useShdtextChkBox->isChecked() );
    config_->writeEntry( "ActiveShdtextX", dialog_->activeShdtextXSpinBox->value() );
    config_->writeEntry( "ActiveShdtextY", dialog_->activeShdtextYSpinBox->value() );
    config_->writeEntry( "ActiveShadowColor", dialog_->activeShdColBtn->color() );
    config_->writeEntry( "InActiveShdtextX", dialog_->inActiveShdtextXSpinBox->value() ); config_->writeEntry( "InActiveShdtextY", dialog_->inActiveShdtextYSpinBox->value() );
    config_->writeEntry( "InActiveShadowColor", dialog_->inActiveShdColBtn->color() );
    config_->writeEntry( "BtnShiftX", dialog_->btnShiftXSpinBox->value() );
    config_->writeEntry( "BtnShiftY", dialog_->btnShiftYSpinBox->value() );


    // colors
    config_->setGroup( "COLORS" );

    config_->writeEntry( "ColorizeActFrames", dialog_->colorizeActFramesChkBox->isChecked() );
    config_->writeEntry( "ColorizeActButtons", dialog_->colorizeActButtonsChkBox->isChecked() );
    config_->writeEntry( "ColorizeInActFrames", dialog_->colorizeInActFramesChkBox->isChecked() );
    config_->writeEntry( "ColorizeInActButtons", dialog_->colorizeInActButtonsChkBox->isChecked() );
    config_->writeEntry( "UseAnimation", dialog_->useAnimChkBox->isChecked() );
    config_->writeEntry( "AnimationType", dialog_->animateEfeectsCombo->currentText() );

    config_->writeEntry( "AnimSteps", dialog_->stepsSpinBox->value() );
    config_->writeEntry( "AnimInterval", dialog_->intervalSpinBox->value() );
    config_->writeEntry( "KeepAnimating", dialog_->KeepAnimateChkBox->isChecked() );

    config_->writeEntry( "ButtonsHoverType", dialog_->hoverTypeCombo->currentText() );
    config_->writeEntry( "EffectAmount", dialog_->effectsAmount_SpinBox->value() );
    config_->writeEntry( "ActiveHighlightColor", dialog_->activeHighlightClrBtn->color() );
    config_->writeEntry( "InActiveHighlightColor", dialog_->inActiveHighlightClrBtn->color() );
    config_->writeEntry( "DecoColorize", dialog_->decoColorizeComboBox->currentText() );
    config_->writeEntry( "ButtonsColorize", dialog_->buttonsColorizeComboBox->currentText() );
    //  config_->writeEntry( "ButtonsHoverColorize", dialog_->buttonsHoverColorizeComboBox->currentText() );

    // buttons
    config_->setGroup( "BUTTONS" );

    config_->writeEntry( "UseCustomButtonsColors", dialog_->useCusBtnClrChkBox->isChecked() );
    config_->writeEntry( "customColorsActiveButtons", dialog_->cusColActBtnChkBox->isChecked() );
    config_->writeEntry( "customColorsInActiveButtons", dialog_->cusColInActBtnChkBox->isChecked() );
    config_->writeEntry( "CloseButtonColor", dialog_->closeClrBtn->color() );
    config_->writeEntry( "MinButtonColor", dialog_->minClrBtn->color() );
    config_->writeEntry( "MaxButtonColor", dialog_->maxClrBtn->color() );
    config_->writeEntry( "RestoreButtonColor", dialog_->restoreClrBtn->color() );
    config_->writeEntry( "HelpButtonColor", dialog_->helpClrBtn->color() );
    config_->writeEntry( "MenuButtonColor", dialog_->menuClrBtn->color() );
    config_->writeEntry( "StickyButtonColor", dialog_->stickyClrBtn->color() );
    config_->writeEntry( "StickyDownButtonColor", dialog_->stickyDownClrBtn->color() );
    config_->writeEntry( "AboveButtonColor", dialog_->aboveClrBtn->color() );
    config_->writeEntry( "AboveDownButtonColor", dialog_->aboveDownClrBtn->color() );
    config_->writeEntry( "BelowButtonColor", dialog_->belowClrBtn->color() );
    config_->writeEntry( "BelowDownButtonColor", dialog_->belowDownClrBtn->color() );
    config_->writeEntry( "ShadeButtonColor", dialog_->shadeClrBtn->color() );
    config_->writeEntry( "ShadeDownButtonColor", dialog_->shadeDownClrBtn->color() );

    // path's
    config_->setGroup( "PATHS" );

    config_->writeEntry( "FramesPath", dialog_->framesPathKurl->url() );
    config_->writeEntry( "ButtonsPath", dialog_->buttonsPathKurl->url() );
    config_->writeEntry( "UseMasks", dialog_->useMasks_Chkbox->isChecked() );
    config_->writeEntry( "MasksPath", dialog_->masksPathKurl->url() );

    config_->sync();
}

//////////////////////////////////////////////////////////////////////////////
// defaults()
// ----------
// Set configuration defaults

void DeKoratorConfig::defaults()
{

    // misc
    QRadioButton * button =
        ( QRadioButton* ) dialog_->titlealign->child( "AlignHCenter" );
    if ( button ) button->setChecked( TRUE );
    dialog_->useMenuImageChkBox->setChecked( FALSE );
	dialog_->ignoreAppIcnCol->setChecked( FALSE );
    dialog_->dblClkCloseChkBox->setChecked( FALSE );
    dialog_->showBtmBorderChkBox->setChecked( FALSE );
    dialog_->useShdtextChkBox->setChecked( FALSE );
    dialog_->activeShdtextXSpinBox->setValue( 0 );
    dialog_->activeShdtextYSpinBox->setValue( 0 );
    dialog_->activeShdColBtn->setColor( QColor( 150, 150, 150 ) );
    dialog_->inActiveShdtextXSpinBox->setValue( 0 );
    dialog_->inActiveShdtextYSpinBox->setValue( 0 );
    dialog_->inActiveShdColBtn->setColor( QColor( 150, 150, 150 ) );
    dialog_->btnShiftXSpinBox->setValue( 0 );
    dialog_->btnShiftYSpinBox->setValue( 0 );

    // colors
    dialog_->colorizeActFramesChkBox->setChecked( FALSE );
    dialog_->colorizeActButtonsChkBox->setChecked( FALSE );
    dialog_->colorizeInActFramesChkBox->setChecked( FALSE );
    dialog_->colorizeInActButtonsChkBox->setChecked( FALSE );
    dialog_->useAnimChkBox->setChecked( FALSE );
    dialog_->animateEfeectsCombo->setCurrentText( "Intensity" );

    dialog_->stepsSpinBox->setValue( 5 );
    dialog_->intervalSpinBox->setValue( 30 );
    dialog_->KeepAnimateChkBox->setChecked( FALSE );

    dialog_->hoverTypeCombo->setCurrentText( "To Gray" );
    dialog_->effectsAmount_SpinBox->setValue( 5 );
    dialog_->activeHighlightClrBtn->setColor( QColor( 150, 150, 150 ) );
    dialog_->inActiveHighlightClrBtn->setColor( QColor( 150, 150, 150 ) );
    dialog_->decoColorizeComboBox->setCurrentText( "Liquid Method" );
    dialog_->buttonsColorizeComboBox->setCurrentText( "Liquid Method" );
    //  dialog_->buttonsHoverColorizeComboBox->setCurrentText( "Liquid Method" );

    // buttons
    dialog_->useCusBtnClrChkBox->setChecked( FALSE );
    dialog_->cusColActBtnChkBox->setChecked( FALSE );
    dialog_->cusColInActBtnChkBox->setChecked( FALSE );
    dialog_->closeClrBtn->setColor( QColor( 150, 150, 150 ) );
    dialog_->minClrBtn->setColor( QColor( 150, 150, 150 ) );
    dialog_->maxClrBtn->setColor( QColor( 150, 150, 150 ) );
    dialog_->restoreClrBtn->setColor( QColor( 150, 150, 150 ) );
    dialog_->helpClrBtn->setColor( QColor( 150, 150, 150 ) );
    dialog_->menuClrBtn->setColor( QColor( 150, 150, 150 ) );
    dialog_->stickyClrBtn->setColor( QColor( 150, 150, 150 ) );
    dialog_->stickyDownClrBtn->setColor( QColor( 150, 150, 150 ) );
    dialog_->aboveClrBtn->setColor( QColor( 150, 150, 150 ) );
    dialog_->aboveDownClrBtn->setColor( QColor( 150, 150, 150 ) );
    dialog_->belowClrBtn->setColor( QColor( 150, 150, 150 ) );
    dialog_->belowDownClrBtn->setColor( QColor( 150, 150, 150 ) );
    dialog_->shadeClrBtn->setColor( QColor( 150, 150, 150 ) );
    dialog_->shadeDownClrBtn->setColor( QColor( 150, 150, 150 ) );

    // path's
    dialog_->framesPathKurl->setURL( "" );
    dialog_->buttonsPathKurl->setURL( "" );
    dialog_->useMasks_Chkbox->setChecked( config_->readBoolEntry( "UseMasks", FALSE ) );
    dialog_->masksPathKurl->setURL( "" );
}

//////////////////////////////////////////////////////////////////////////////
// themeSelected()
// ----------
//

void DeKoratorConfig::themeSelected( QListViewItem *item )
{
	themes_->themeSelected( item, dialog_->previewLabael, dialog_->removeThemBtn );
}

//////////////////////////////////////////////////////////////////////////////
// installNewTheme()
// ----------
//

void DeKoratorConfig::installNewTheme()
{
    themes_->installNewTheme();
}

//////////////////////////////////////////////////////////////////////////////
// removeSelectedTheme()
// ----------
//

void DeKoratorConfig::removeSelectedTheme()
{
    themes_->removeSelectedTheme();
}

//////////////////////////////////////////////////////////////////////////////
// setTheme()
// ----------
//

void DeKoratorConfig::setTheme()
{
    themes_->setTheme( dialog_->framesPathKurl, dialog_->buttonsPathKurl, dialog_->masksPathKurl );
}

//////////////////////////////////////////////////////////////////////////////
// Plugin Stuff                                                             //
//////////////////////////////////////////////////////////////////////////////

extern "C"
{
    QObject * allocate_config( KConfig * config, QWidget * parent )
    {
        return ( new DeKoratorConfig( config, parent ) );
    }
}

#include "deKoratorconfig.moc"
