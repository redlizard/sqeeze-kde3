#include <kdialog.h>
#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file './configdialog.ui'
**
** Created: Wed Jan 16 12:23:05 2008
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.8   edited Jan 11 14:47 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "configdialog.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
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
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, sizePolicy().hasHeightForWidth() ) );
    ConfigDialogLayout = new QVBoxLayout( this, 0, 6, "ConfigDialogLayout"); 

    tabWidget3 = new QTabWidget( this, "tabWidget3" );

    tab = new QWidget( tabWidget3, "tab" );
    tabLayout = new QGridLayout( tab, 1, 1, 11, 6, "tabLayout"); 

    titlesize = new QSpinBox( tab, "titlesize" );
    titlesize->setMaxValue( 64 );
    titlesize->setMinValue( 16 );

    tabLayout->addWidget( titlesize, 1, 2 );

    framesize = new QSpinBox( tab, "framesize" );
    framesize->setMaxValue( 64 );
    framesize->setMinValue( 1 );
    framesize->setValue( 4 );

    tabLayout->addWidget( framesize, 2, 2 );

    titleshadow = new QCheckBox( tab, "titleshadow" );
    titleshadow->setChecked( TRUE );

    tabLayout->addWidget( titleshadow, 3, 2 );

    roundCorners = new QCheckBox( tab, "roundCorners" );
    roundCorners->setChecked( TRUE );

    tabLayout->addWidget( roundCorners, 4, 2 );

    titleBarStyle = new QComboBox( FALSE, tab, "titleBarStyle" );

    tabLayout->addWidget( titleBarStyle, 5, 2 );

    textLabel1_4 = new QLabel( tab, "textLabel1_4" );
    textLabel1_4->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tabLayout->addWidget( textLabel1_4, 5, 1 );

    textLabel1_2 = new QLabel( tab, "textLabel1_2" );
    textLabel1_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tabLayout->addWidget( textLabel1_2, 2, 1 );

    textLabel1 = new QLabel( tab, "textLabel1" );
    textLabel1->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tabLayout->addWidget( textLabel1, 1, 1 );

    textLabel1_3 = new QLabel( tab, "textLabel1_3" );
    textLabel1_3->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tabLayout->addMultiCellWidget( textLabel1_3, 0, 0, 0, 1 );
    spacer5 = new QSpacerItem( 71, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    tabLayout->addItem( spacer5, 2, 0 );
    spacer4 = new QSpacerItem( 81, 21, QSizePolicy::Expanding, QSizePolicy::Minimum );
    tabLayout->addItem( spacer4, 2, 3 );

    titlealign = new QButtonGroup( tab, "titlealign" );
    titlealign->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)0, 0, 0, titlealign->sizePolicy().hasHeightForWidth() ) );
    titlealign->setLineWidth( 0 );
    titlealign->setColumnLayout(0, Qt::Vertical );
    titlealign->layout()->setSpacing( 6 );
    titlealign->layout()->setMargin( 0 );
    titlealignLayout = new QHBoxLayout( titlealign->layout() );
    titlealignLayout->setAlignment( Qt::AlignTop );

    AlignLeft = new QRadioButton( titlealign, "AlignLeft" );
    titlealignLayout->addWidget( AlignLeft );

    AlignHCenter = new QRadioButton( titlealign, "AlignHCenter" );
    AlignHCenter->setChecked( TRUE );
    titlealignLayout->addWidget( AlignHCenter );

    AlignRight = new QRadioButton( titlealign, "AlignRight" );
    titlealignLayout->addWidget( AlignRight );

    tabLayout->addWidget( titlealign, 0, 2 );
    tabWidget3->insertTab( tab, QString::fromLatin1("") );

    tab_2 = new QWidget( tabWidget3, "tab_2" );
    tabLayout_2 = new QGridLayout( tab_2, 1, 1, 11, 6, "tabLayout_2"); 

    textLabel1_5 = new QLabel( tab_2, "textLabel1_5" );
    textLabel1_5->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tabLayout_2->addWidget( textLabel1_5, 1, 0 );

    buttonStyle = new QComboBox( FALSE, tab_2, "buttonStyle" );

    tabLayout_2->addWidget( buttonStyle, 1, 1 );

    textLabel4 = new QLabel( tab_2, "textLabel4" );
    textLabel4->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)5, 0, 0, textLabel4->sizePolicy().hasHeightForWidth() ) );
    textLabel4->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tabLayout_2->addWidget( textLabel4, 0, 0 );

    buttonsize = new QSpinBox( tab_2, "buttonsize" );
    buttonsize->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, buttonsize->sizePolicy().hasHeightForWidth() ) );
    buttonsize->setButtonSymbols( QSpinBox::UpDownArrows );
    buttonsize->setMaxValue( 64 );
    buttonsize->setMinValue( 1 );
    buttonsize->setValue( 16 );

    tabLayout_2->addWidget( buttonsize, 0, 1 );

    animatebuttons = new QCheckBox( tab_2, "animatebuttons" );
    animatebuttons->setEnabled( TRUE );
    animatebuttons->setChecked( TRUE );

    tabLayout_2->addWidget( animatebuttons, 2, 1 );

    layout11 = new QHBoxLayout( 0, 0, 6, "layout11"); 
    spacer3 = new QSpacerItem( 30, 20, QSizePolicy::Maximum, QSizePolicy::Minimum );
    layout11->addItem( spacer3 );

    textLabel2 = new QLabel( tab_2, "textLabel2" );
    layout11->addWidget( textLabel2 );

    btnComboBox = new QComboBox( FALSE, tab_2, "btnComboBox" );
    btnComboBox->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, btnComboBox->sizePolicy().hasHeightForWidth() ) );
    layout11->addWidget( btnComboBox );

    tabLayout_2->addLayout( layout11, 3, 1 );

    squareButton = new QCheckBox( tab_2, "squareButton" );
    squareButton->setChecked( TRUE );

    tabLayout_2->addMultiCellWidget( squareButton, 4, 4, 1, 2 );

    lightBorder = new QCheckBox( tab_2, "lightBorder" );

    tabLayout_2->addMultiCellWidget( lightBorder, 5, 5, 1, 2 );

    nomodalbuttons = new QCheckBox( tab_2, "nomodalbuttons" );
    nomodalbuttons->setChecked( TRUE );

    tabLayout_2->addWidget( nomodalbuttons, 6, 1 );

    menuClose = new QCheckBox( tab_2, "menuClose" );
    menuClose->setEnabled( TRUE );

    tabLayout_2->addMultiCellWidget( menuClose, 7, 7, 1, 2 );
    spacer3_3 = new QSpacerItem( 121, 21, QSizePolicy::Expanding, QSizePolicy::Minimum );
    tabLayout_2->addItem( spacer3_3, 1, 2 );
    tabWidget3->insertTab( tab_2, QString::fromLatin1("") );
    ConfigDialogLayout->addWidget( tabWidget3 );
    languageChange();
    resize( QSize(388, 265).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( animatebuttons, SIGNAL( toggled(bool) ), textLabel2, SLOT( setEnabled(bool) ) );
    connect( animatebuttons, SIGNAL( toggled(bool) ), btnComboBox, SLOT( setEnabled(bool) ) );

    // tab order
    setTabOrder( AlignLeft, AlignHCenter );
    setTabOrder( AlignHCenter, AlignRight );
    setTabOrder( AlignRight, titlesize );
    setTabOrder( titlesize, framesize );
    setTabOrder( framesize, titleshadow );
    setTabOrder( titleshadow, roundCorners );
    setTabOrder( roundCorners, buttonsize );
    setTabOrder( buttonsize, animatebuttons );
    setTabOrder( animatebuttons, btnComboBox );
    setTabOrder( btnComboBox, menuClose );
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
    titlesize->setSuffix( tr2i18n( " pixels" ) );
    framesize->setSuffix( tr2i18n( " pixels" ) );
    framesize->setSpecialValueText( tr2i18n( "1 pixel" ) );
    titleshadow->setText( tr2i18n( "&Use shadowed text" ) );
    titleshadow->setAccel( QKeySequence( tr2i18n( "Alt+U" ) ) );
    QWhatsThis::add( titleshadow, tr2i18n( "Check this option if you want the titlebar text to have a 3D look with a shadow behind it." ) );
    roundCorners->setText( tr2i18n( "R&ound top corners" ) );
    roundCorners->setAccel( QKeySequence( tr2i18n( "Alt+O" ) ) );
    titleBarStyle->clear();
    titleBarStyle->insertItem( tr2i18n( "Gradients" ) );
    titleBarStyle->insertItem( tr2i18n( "Glass" ) );
    textLabel1_4->setText( tr2i18n( "Titlebar style:" ) );
    textLabel1_2->setText( tr2i18n( "Frame width:" ) );
    textLabel1->setText( tr2i18n( "Title height:" ) );
    textLabel1_3->setText( tr2i18n( "Text alignment:" ) );
    titlealign->setTitle( QString::null );
    AlignLeft->setText( tr2i18n( "L&eft" ) );
    AlignLeft->setAccel( QKeySequence( tr2i18n( "Alt+E" ) ) );
    QToolTip::add( AlignLeft, QString::null );
    AlignHCenter->setText( tr2i18n( "Ce&nter" ) );
    AlignHCenter->setAccel( QKeySequence( tr2i18n( "Alt+N" ) ) );
    QToolTip::add( AlignHCenter, QString::null );
    AlignRight->setText( tr2i18n( "Ri&ght" ) );
    AlignRight->setAccel( QKeySequence( tr2i18n( "Alt+G" ) ) );
    QToolTip::add( AlignRight, QString::null );
    tabWidget3->changeTab( tab, tr2i18n( "Titleb&ar" ) );
    textLabel1_5->setText( tr2i18n( "Button Style:" ) );
    buttonStyle->clear();
    buttonStyle->insertItem( tr2i18n( "Gradients" ) );
    buttonStyle->insertItem( tr2i18n( "Glass" ) );
    buttonStyle->insertItem( tr2i18n( "Reverse Gradients" ) );
    buttonStyle->insertItem( tr2i18n( "Flat" ) );
    buttonStyle->setCurrentItem( 1 );
    textLabel4->setText( tr2i18n( "Button size:" ) );
    buttonsize->setSuffix( tr2i18n( " pixels" ) );
    buttonsize->setSpecialValueText( tr2i18n( "1 pixel" ) );
    animatebuttons->setText( tr2i18n( "&Animate buttons" ) );
    animatebuttons->setAccel( QKeySequence( tr2i18n( "Alt+A" ) ) );
    QWhatsThis::add( animatebuttons, tr2i18n( "Check this option if you want to use button animations when hovering with the mouse." ) );
    textLabel2->setText( tr2i18n( "Animation style:" ) );
    btnComboBox->clear();
    btnComboBox->insertItem( tr2i18n( "Colorize" ) );
    btnComboBox->insertItem( tr2i18n( "Intensify" ) );
    btnComboBox->insertItem( tr2i18n( "Fade" ) );
    squareButton->setText( tr2i18n( "S&quare buttons" ) );
    squareButton->setAccel( QKeySequence( tr2i18n( "Alt+Q" ) ) );
    lightBorder->setText( tr2i18n( "Li&ghter button borders" ) );
    lightBorder->setAccel( QKeySequence( tr2i18n( "Alt+G" ) ) );
    nomodalbuttons->setText( tr2i18n( "&No buttons on modal windows" ) );
    nomodalbuttons->setAccel( QKeySequence( tr2i18n( "Alt+N" ) ) );
    menuClose->setText( tr2i18n( "Close window when &menu double clicked" ) );
    menuClose->setAccel( QKeySequence( tr2i18n( "Alt+M" ) ) );
    tabWidget3->changeTab( tab_2, tr2i18n( "&Buttons appearance" ) );
}

#include "configdialog.moc"
