/* $Id: klearlookconfig.cpp,v 1.9 2005/12/14 07:20:58 jck Exp $
*/

/*
 
  Klearlook (C) Joerg C. Koenig, 2005 jck@gmx.org
 
----
 
  Based upon QtCurve (C) Craig Drummond, 2003 Craig.Drummond@lycos.co.uk
      Bernhard Rosenkränzer <bero@r?dh?t.com>
      Preston Brown <pbrown@r?dh?t.com>
      Than Ngo <than@r?dh?t.com>
 
  Released under the GNU General Public License (GPL) v2.
 
  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files(the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:
 
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
 
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.
*/


#include <qsettings.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <kglobal.h>
#include <klocale.h>
#include "klearlookconfig.h"
#include "../version.h"
#include "../common.c"

extern "C" {
    QWidget* allocate_kstyle_config( QWidget * parent ) {
        KGlobal::locale() ->insertCatalogue( "kstyle_klearlook_config" );
        return new KlearlookConfig( parent );
    }
}

static const char *toStr( EDefBtnIndicator ind ) {
    switch ( ind ) {
        case IND_BORDER:
            return "border";
        case IND_NONE:
            return "none";
        case IND_FONT_COLOUR:
            return "fontcolour";
        default:
            return "corner";
    }
}

static const char *toStr( EGroove ind, bool allowNone = true ) {
    switch ( ind ) {
        case GROOVE_RAISED:
            return "raised";
        case GROOVE_NONE:
            if ( allowNone )
                return "none";
        default:
            return "sunken";
    }
}

static const char *toStr( ETBarBorder ind ) {
    switch ( ind ) {
        case TB_DARK:
            return "dark";
        case TB_LIGHT:
            return "light";
        case TB_NONE:
            return "none";
        default:
            return "light";
    }
}

static const char *toStr( ELvExpander exp ) {
    switch ( exp ) {
        case LV_EXP_PM:
            return "plusminus";
        default:
            return "arrow";
    }
}

static const char *toStr( ELvLines exp ) {
    switch ( exp ) {
        case LV_LINES_NONE:
            return "none";
        case LV_LINES_DOTTED:
            return "dotted";
        default:
            return "solid";
    }
}

static const char *toStr( EProfile exp ) {
    switch ( exp ) {
        case PROFILE_FLAT:
            return "flat";
        case PROFILE_RAISED:
            return "raised";
        default:
            return "sunken";
    }
}

static const char *toStr( EAppearance exp ) {
    switch ( exp ) {
        case APPEARANCE_FLAT:
            return "flat";
        case APPEARANCE_GRADIENT:
            return "gradient";
        default:
            return "lightgradient";
    }
}

KlearlookConfig::KlearlookConfig( QWidget *parent )
    : KlearlookConfigBase( parent ) {
    QSettings s;

    versionLabel->setText( VERSION );

    border = s.readBoolEntry( "/klearlookstyle/Settings/border", true );
    borderWidget->setChecked( border );
    connect( borderWidget, SIGNAL( toggled( bool ) ), SLOT( borderToggled() ) );

    icons = s.readBoolEntry( "/klearlookstyle/Settings/icons", true );
    menuIcons->setChecked( icons );
    connect( menuIcons, SIGNAL( toggled( bool ) ), SLOT( updateChanged() ) );

    darkMenubar = s.readBoolEntry( "/klearlookstyle/Settings/darkMenubar", true );
    darkMenubarWidget->setChecked( darkMenubar );
    connect( darkMenubarWidget, SIGNAL( toggled( bool ) ), SLOT( updateChanged() ) );
    
    popupmenuHighlightLevel = s.readNumEntry( "/klearlookstyle/Settings/popupmenuHighlightLevel", 0);    
    popupmenuHighlightLevelWidget->setValue( popupmenuHighlightLevel );
    connect( popupmenuHighlightLevelWidget, SIGNAL( valueChanged(int) ), SLOT( updateChanged() ) );

    round = s.readBoolEntry( "/klearlookstyle/Settings/round", true );
    roundWidget->setChecked( round );
    connect( roundWidget, SIGNAL( toggled( bool ) ), SLOT( roundToggled() ) );

    QString tmp = s.readEntry( "/klearlookstyle/Settings/toolbarBorders", QString::null );
    toolbarBorders = tmp.isEmpty() ? TB_LIGHT : qtc_to_tbar_border( tmp.latin1() );
    toolbarBordersWidget->setCurrentItem( toolbarBorders );
    connect( toolbarBordersWidget, SIGNAL( activated( int ) ), SLOT( updateChanged() ) );

    bool etched = s.readBoolEntry( "/klearlookstyle/Settings/etched", true );

    tmp = s.readEntry( "/klearlookstyle/Settings/sliderThumbs", QString::null );
    sliderThumbs = tmp.isEmpty()
                   ? etched ? GROOVE_SUNKEN : GROOVE_RAISED
               : qtc_to_groove( tmp.latin1() );
    sliderThumbsCombo->setCurrentItem( sliderThumbs );
    connect( sliderThumbsCombo, SIGNAL( activated( int ) ), SLOT( updateChanged() ) );

    tmp = s.readEntry( "/klearlookstyle/Settings/lvExpander", QString::null );
    lvExpander = tmp.isEmpty()
                 ? LV_EXP_ARR
                 : qtc_to_lv_expander( tmp.latin1() );
    lvExpanderCombo->setCurrentItem( lvExpander );
    connect( lvExpanderCombo, SIGNAL( activated( int ) ), SLOT( updateChanged() ) );

    tmp = s.readEntry( "/klearlookstyle/Settings/lvLines", QString::null );
    lvLines = tmp.isEmpty()
              ? LV_LINES_SOLID
              : qtc_to_lv_lines( tmp.latin1() );
    lvLinesCombo->setCurrentItem( lvLines );
    connect( lvLinesCombo, SIGNAL( activated( int ) ), SLOT( lvChanged() ) );

    lvDark = s.readBoolEntry( "/klearlookstyle/Settings/lvDark", false );
    lvDarkWidget->setChecked( lvDark );
    connect( lvDarkWidget, SIGNAL( toggled( bool ) ), SLOT( updateChanged() ) );

    handles = qtc_to_groove( s.readEntry( "/klearlookstyle/Settings/handles", DEF_HANDLE_STR ).latin1() );
    if ( GROOVE_NONE == handles )
        handles = GROOVE_SUNKEN;
    handlesCombo->setCurrentItem( handles );
    connect( handlesCombo, SIGNAL( activated( int ) ), SLOT( updateChanged() ) );


    appearance = qtc_to_appearance( s.readEntry( "/klearlookstyle/Settings/appearance", DEF_APPEARANCE_STR ).latin1() );
    appearanceCombo->setCurrentItem( appearance );
    connect( appearanceCombo, SIGNAL( activated( int ) ), SLOT( updateChanged() ) );

    pm = qtc_to_profile( s.readEntry( "/klearlookstyle/Settings/pm", DEF_PROFILE_STR ).latin1() );
    pmCombo->setCurrentItem( pm );
    connect( pmCombo, SIGNAL( activated( int ) ), SLOT( updateChanged() ) );

    vArrow = s.readBoolEntry( "/klearlookstyle/Settings/vArrow", false );
    vArrowWidget->setChecked( vArrow );
    connect( vArrowWidget, SIGNAL( toggled( bool ) ), SLOT( updateChanged() ) );
    embolden = s.readBoolEntry( "/klearlookstyle/Settings/embolden", true );
    emboldenWidget->setChecked( embolden );
    connect( emboldenWidget, SIGNAL( toggled( bool ) ), SLOT( emboldenToggled() ) );
    crLabelHighlight = s.readBoolEntry( "/klearlookstyle/Settings/crLabelHighlight", true );
    crLabelHighlightWidget->setChecked( crLabelHighlight );
    connect( crLabelHighlightWidget, SIGNAL( toggled( bool ) ), SLOT( updateChanged() ) );

    defBtnIndicator = qtc_to_ind( s.readEntry( "/klearlookstyle/Settings/defBtnIndicator", DEF_IND_STR ).latin1() );
    defaultIndicatorCombo->setCurrentItem( defBtnIndicator );
    connect( defaultIndicatorCombo, SIGNAL( activated( int ) ), SLOT( dbiChanged() ) );

    borderSplitter = s.readBoolEntry( "/klearlookstyle/Settings/borderSplitter", true );
    borderSplitterWidget->setChecked( borderSplitter );
    connect( borderSplitterWidget, SIGNAL( toggled( bool ) ), SLOT( updateChanged() ) );

}

KlearlookConfig::~KlearlookConfig() {}


void KlearlookConfig::save() {
    QSettings s;
    s.writeEntry( "/klearlookstyle/Settings/border", borderWidget->isChecked() );
    s.writeEntry( "/klearlookstyle/Settings/icons", menuIcons->isChecked() );
    s.writeEntry( "/klearlookstyle/Settings/darkMenubar", darkMenubarWidget->isChecked() );
    s.writeEntry( "/klearlookstyle/Settings/darkMenubar", darkMenubarWidget->isChecked() );
    s.writeEntry( "/klearlookstyle/Settings/popupmenuHighlightLevel", popupmenuHighlightLevelWidget->value() );
    s.writeEntry( "/klearlookstyle/Settings/toolbarBorders", toStr( ( ETBarBorder ) toolbarBordersWidget->currentItem() ) );
    s.writeEntry( "/klearlookstyle/Settings/appearance", toStr( ( EAppearance ) appearanceCombo->currentItem() ) );
    s.writeEntry( "/klearlookstyle/Settings/pm", toStr( ( EProfile ) pmCombo->currentItem() ) );
    s.writeEntry( "/klearlookstyle/Settings/vArrow", vArrowWidget->isChecked() );
    s.writeEntry( "/klearlookstyle/Settings/borderSplitter", borderSplitterWidget->isChecked() );
    s.writeEntry( "/klearlookstyle/Settings/embolden", emboldenWidget->isChecked() );
    s.writeEntry( "/klearlookstyle/Settings/crLabelHighlight", crLabelHighlightWidget->isChecked() );
    s.writeEntry( "/klearlookstyle/Settings/defBtnIndicator", toStr( ( EDefBtnIndicator ) defaultIndicatorCombo->currentItem() ) );
    s.writeEntry( "/klearlookstyle/Settings/sliderThumbs", toStr( ( EGroove ) sliderThumbsCombo->currentItem() ) );
    s.writeEntry( "/klearlookstyle/Settings/handles", toStr( ( EGroove ) handlesCombo->currentItem(), false ) );
    s.writeEntry( "/klearlookstyle/Settings/lvExpander", toStr( ( ELvExpander ) lvExpanderCombo->currentItem() ) );
    s.writeEntry( "/klearlookstyle/Settings/lvLines", toStr( ( ELvLines ) lvLinesCombo->currentItem() ) );
    s.writeEntry( "/klearlookstyle/Settings/lvDark", lvDarkWidget->isChecked() );
}

void KlearlookConfig::defaults() {
    appearanceCombo->setCurrentItem( APPEARANCE_GRADIENT );
    borderWidget->setChecked( true );
    menuIcons->setChecked( true );
    popupmenuHighlightLevelWidget->setValue(6);
    darkMenubarWidget->setChecked( true );
    roundWidget->setChecked( true );
    toolbarBordersWidget->setCurrentItem( TB_LIGHT );
    pmCombo->setCurrentItem( PROFILE_RAISED );
    vArrowWidget->setChecked( false );
    borderSplitterWidget->setChecked( true );
    emboldenWidget->setChecked( false );
    crLabelHighlightWidget->setChecked( true );
    defaultIndicatorCombo->setCurrentItem( IND_NONE );
    sliderThumbsCombo->setCurrentItem( GROOVE_RAISED );
    handlesCombo->setCurrentItem( GROOVE_RAISED );
    lvExpanderCombo->setCurrentItem( LV_EXP_ARR );
    lvLinesCombo->setCurrentItem( LV_LINES_SOLID );
    lvDarkWidget->setChecked( false );
}

void KlearlookConfig::borderToggled() {
    if ( roundWidget->isChecked() && !borderWidget->isChecked() )
        roundWidget->setChecked( false );
    updateChanged();
}

void KlearlookConfig::roundToggled() {
    if ( roundWidget->isChecked() && !borderWidget->isChecked() )
        borderWidget->setChecked( true );
    updateChanged();
}

void KlearlookConfig::emboldenToggled() {
    //if(!emboldenWidget->isChecked() && IND_NONE==defaultIndicatorCombo->currentItem())
    //    defaultIndicatorCombo->setCurrentItem(IND_CORNER);
    updateChanged();
}

void KlearlookConfig::dbiChanged() {
    //if(IND_NONE==defaultIndicatorCombo->currentItem() && !emboldenWidget->isChecked())
    //    emboldenWidget->setChecked(true);
    updateChanged();
}

void KlearlookConfig::lvChanged() {
    //lvDarkWidget->setEnabled(LV_LINES_NONE!=lvLinesCombo->currentItem());
    updateChanged();
}

void KlearlookConfig::updateChanged() {
    if ( borderWidget->isChecked() != border ||
            menuIcons->isChecked() != icons ||
            roundWidget->isChecked() != round ||
            popupmenuHighlightLevelWidget->value() != popupmenuHighlightLevel ||
            darkMenubarWidget->isChecked() != darkMenubar ||
            toolbarBordersWidget->currentItem() != toolbarBorders ||
            appearanceCombo->currentItem() != ( int ) appearance ||
            pmCombo->currentItem() != ( int ) pm ||
            vArrowWidget->isChecked() != vArrow ||
            borderSplitterWidget->isChecked() != borderSplitter ||
            emboldenWidget->isChecked() != embolden ||
            crLabelHighlightWidget->isChecked() != crLabelHighlight ||
            defaultIndicatorCombo->currentItem() != ( int ) defBtnIndicator ||
            sliderThumbsCombo->currentItem() != ( int ) sliderThumbs ||
            handlesCombo->currentItem() != ( int ) handles ||
            lvExpanderCombo->currentItem() != ( int ) lvExpander ||
            lvLinesCombo->currentItem() != ( int ) lvLines ||
            lvDarkWidget->isChecked() != lvDark )
        emit changed( true );
    else
        emit changed( false );
}


#include "klearlookconfig.moc"
