#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file './configdialog.ui'
**
** Created: Tue Dec 23 23:38:48 2003
**      by: The User Interface Compiler ()
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "configdialog.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/*
 *  Constructs a ConfigDialog as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
ConfigDialog::ConfigDialog( QWidget* parent, const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "ConfigDialog" );
    ConfigDialogLayout = new QHBoxLayout( this, 11, 6, "ConfigDialogLayout"); 

    layout1 = new QGridLayout( 0, 1, 1, 0, 6, "layout1"); 

    titleAlign = new QButtonGroup( this, "titleAlign" );
    titleAlign->setColumnLayout(0, Qt::Vertical );
    titleAlign->layout()->setSpacing( 6 );
    titleAlign->layout()->setMargin( 11 );
    titleAlignLayout = new QHBoxLayout( titleAlign->layout() );
    titleAlignLayout->setAlignment( Qt::AlignTop );

    AlignLeft = new QRadioButton( titleAlign, "AlignLeft" );
    titleAlignLayout->addWidget( AlignLeft );

    AlignHCenter = new QRadioButton( titleAlign, "AlignHCenter" );
    titleAlignLayout->addWidget( AlignHCenter );

    AlignRight = new QRadioButton( titleAlign, "AlignRight" );
    titleAlignLayout->addWidget( AlignRight );

    layout1->addMultiCellWidget( titleAlign, 0, 0, 0, 4 );

    highContrastButtons = new QCheckBox( this, "highContrastButtons" );

    layout1->addMultiCellWidget( highContrastButtons, 3, 3, 0, 2 );

    designatedTitlebarColor = new QCheckBox( this, "designatedTitlebarColor" );

    layout1->addMultiCellWidget( designatedTitlebarColor, 1, 1, 0, 4 );

    bordersBlendColor = new QCheckBox( this, "bordersBlendColor" );

    layout1->addMultiCellWidget( bordersBlendColor, 2, 2, 0, 3 );

    titleShadow = new QCheckBox( this, "titleShadow" );

    layout1->addMultiCellWidget( titleShadow, 4, 4, 0, 4 );
    ConfigDialogLayout->addLayout( layout1 );
    languageChange();
    resize( QSize(541, 194).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // tab order
    setTabOrder( AlignLeft, AlignHCenter );
    setTabOrder( AlignHCenter, AlignRight );
    setTabOrder( AlignRight, designatedTitlebarColor );
    setTabOrder( designatedTitlebarColor, bordersBlendColor );
    setTabOrder( bordersBlendColor, highContrastButtons );
    setTabOrder( highContrastButtons, titleShadow );
}

/*
 *  Destroys the object and frees any allocated resources
 */
ConfigDialog::~ConfigDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void ConfigDialog::languageChange()
{
    setCaption( tr2i18n( "Config Dialog" ) );
    titleAlign->setTitle( tr2i18n( "Title &Alignment" ) );
    AlignLeft->setText( tr2i18n( "Left" ) );
    AlignHCenter->setText( tr2i18n( "Center" ) );
    AlignRight->setText( tr2i18n( "Right" ) );
    highContrastButtons->setText( tr2i18n( "Use high contrast hovered buttons" ) );
    designatedTitlebarColor->setText( tr2i18n( "Use the &designated title bar color for the title bar" ) );
    designatedTitlebarColor->setAccel( QKeySequence( tr2i18n( "Alt+D" ) ) );
    QWhatsThis::add( designatedTitlebarColor, tr2i18n( "Check this option if the title bar should obtain its color from the title bar color setting instead of the window background color." ) );
    bordersBlendColor->setText( tr2i18n( "Use title blend color for window borders" ) );
    QWhatsThis::add( bordersBlendColor, tr2i18n( "Check this option if the borders should obtain their color from the title blend color setting instead of the window background color." ) );
    titleShadow->setText( tr2i18n( "Use shadowed &text" ) );
    titleShadow->setAccel( QKeySequence( tr2i18n( "Alt+T" ) ) );
    QWhatsThis::add( titleShadow, tr2i18n( "Check this option if you want the titlebar text to have a 3D look with a shadow behind it." ) );
}

#include "configdialog.moc"
