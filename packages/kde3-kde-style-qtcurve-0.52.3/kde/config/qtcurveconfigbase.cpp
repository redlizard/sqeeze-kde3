#include <kdialog.h>
#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file './qtcurveconfigbase.ui'
**
** Created: Thu Jun 7 20:54:12 2007
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.7   edited Aug 31 2005 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "qtcurveconfigbase.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qtabwidget.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <kcolorbutton.h>
#include <knuminput.h>
#include <qgroupbox.h>
#include <kpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/*
 *  Constructs a QtCurveConfigBase as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
QtCurveConfigBase::QtCurveConfigBase( QWidget* parent, const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "QtCurveConfigBase" );
    QtCurveConfigBaseLayout = new QGridLayout( this, 1, 1, 0, 6, "QtCurveConfigBaseLayout"); 

    titleLabel = new QLabel( this, "titleLabel" );
    titleLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)5, 0, 0, titleLabel->sizePolicy().hasHeightForWidth() ) );
    titleLabel->setAlignment( int( QLabel::AlignCenter ) );

    QtCurveConfigBaseLayout->addMultiCellWidget( titleLabel, 0, 0, 0, 1 );
    spacer2 = new QSpacerItem( 20, 16, QSizePolicy::Minimum, QSizePolicy::Fixed );
    QtCurveConfigBaseLayout->addItem( spacer2, 1, 0 );

    optionsTab = new QTabWidget( this, "optionsTab" );
    optionsTab->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)3, 0, 0, optionsTab->sizePolicy().hasHeightForWidth() ) );

    tab = new QWidget( optionsTab, "tab" );
    tabLayout = new QGridLayout( tab, 1, 1, 11, 6, "tabLayout"); 

    textLabel1_6 = new QLabel( tab, "textLabel1_6" );

    tabLayout->addWidget( textLabel1_6, 1, 0 );

    textLabel1_2_3 = new QLabel( tab, "textLabel1_2_3" );

    tabLayout->addWidget( textLabel1_2_3, 3, 0 );

    textLabel1_2_3_2 = new QLabel( tab, "textLabel1_2_3_2" );

    tabLayout->addWidget( textLabel1_2_3_2, 2, 0 );

    textLabel1_7 = new QLabel( tab, "textLabel1_7" );

    tabLayout->addWidget( textLabel1_7, 0, 0 );

    textLabel1 = new QLabel( tab, "textLabel1" );

    tabLayout->addWidget( textLabel1, 4, 0 );

    textLabel1_8_2 = new QLabel( tab, "textLabel1_8_2" );

    tabLayout->addWidget( textLabel1_8_2, 7, 0 );

    lvAppearance = new QComboBox( FALSE, tab, "lvAppearance" );

    tabLayout->addWidget( lvAppearance, 7, 1 );

    embolden = new QCheckBox( tab, "embolden" );

    tabLayout->addWidget( embolden, 5, 1 );

    tabAppearance = new QComboBox( FALSE, tab, "tabAppearance" );

    tabLayout->addWidget( tabAppearance, 8, 1 );

    progressAppearance = new QComboBox( FALSE, tab, "progressAppearance" );

    tabLayout->addWidget( progressAppearance, 4, 1 );

    scrollbarType = new QComboBox( FALSE, tab, "scrollbarType" );
    scrollbarType->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, scrollbarType->sizePolicy().hasHeightForWidth() ) );

    tabLayout->addWidget( scrollbarType, 3, 1 );

    sliderThumbs = new QComboBox( FALSE, tab, "sliderThumbs" );
    sliderThumbs->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, sliderThumbs->sizePolicy().hasHeightForWidth() ) );

    tabLayout->addMultiCellWidget( sliderThumbs, 3, 3, 2, 6 );

    stripedProgress = new QCheckBox( tab, "stripedProgress" );

    tabLayout->addWidget( stripedProgress, 4, 4 );

    gradientPbGroove = new QCheckBox( tab, "gradientPbGroove" );

    tabLayout->addMultiCellWidget( gradientPbGroove, 4, 4, 2, 3 );

    splitters = new QComboBox( FALSE, tab, "splitters" );

    tabLayout->addMultiCellWidget( splitters, 1, 1, 1, 6 );

    round = new QComboBox( FALSE, tab, "round" );

    tabLayout->addMultiCellWidget( round, 0, 0, 2, 6 );

    defBtnIndicator = new QComboBox( FALSE, tab, "defBtnIndicator" );
    defBtnIndicator->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, defBtnIndicator->sizePolicy().hasHeightForWidth() ) );

    tabLayout->addMultiCellWidget( defBtnIndicator, 5, 5, 2, 6 );

    sliderAppearance = new QComboBox( FALSE, tab, "sliderAppearance" );

    tabLayout->addWidget( sliderAppearance, 2, 1 );

    animatedProgress = new QCheckBox( tab, "animatedProgress" );

    tabLayout->addMultiCellWidget( animatedProgress, 4, 4, 5, 6 );

    lvLines = new QCheckBox( tab, "lvLines" );

    tabLayout->addMultiCellWidget( lvLines, 7, 7, 2, 6 );

    stdFocus = new QCheckBox( tab, "stdFocus" );

    tabLayout->addMultiCellWidget( stdFocus, 10, 10, 0, 1 );

    drawStatusBarFrames = new QCheckBox( tab, "drawStatusBarFrames" );

    tabLayout->addMultiCellWidget( drawStatusBarFrames, 11, 11, 0, 1 );

    fixParentlessDialogs = new QCheckBox( tab, "fixParentlessDialogs" );

    tabLayout->addMultiCellWidget( fixParentlessDialogs, 12, 12, 0, 1 );

    vArrows = new QCheckBox( tab, "vArrows" );

    tabLayout->addMultiCellWidget( vArrows, 13, 13, 0, 1 );
    spacer6 = new QSpacerItem( 20, 27, QSizePolicy::Minimum, QSizePolicy::Expanding );
    tabLayout->addItem( spacer6, 14, 0 );

    shadeSliders = new QComboBox( FALSE, tab, "shadeSliders" );
    shadeSliders->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, shadeSliders->sizePolicy().hasHeightForWidth() ) );

    tabLayout->addMultiCellWidget( shadeSliders, 2, 2, 2, 3 );

    textLabel1_8_2_2 = new QLabel( tab, "textLabel1_8_2_2" );

    tabLayout->addWidget( textLabel1_8_2_2, 8, 0 );

    textLabel1_8_2_2_2 = new QLabel( tab, "textLabel1_8_2_2_2" );

    tabLayout->addWidget( textLabel1_8_2_2_2, 9, 0 );

    shadeCheckRadio = new QComboBox( FALSE, tab, "shadeCheckRadio" );
    shadeCheckRadio->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, shadeCheckRadio->sizePolicy().hasHeightForWidth() ) );

    tabLayout->addWidget( shadeCheckRadio, 9, 1 );

    customCheckRadioColor = new KColorButton( tab, "customCheckRadioColor" );

    tabLayout->addMultiCellWidget( customCheckRadioColor, 9, 9, 2, 3 );

    customSlidersColor = new KColorButton( tab, "customSlidersColor" );

    tabLayout->addWidget( customSlidersColor, 2, 4 );

    fillSlider = new QCheckBox( tab, "fillSlider" );

    tabLayout->addWidget( fillSlider, 2, 6 );

    textLabel1_3 = new QLabel( tab, "textLabel1_3" );

    tabLayout->addWidget( textLabel1_3, 5, 0 );

    textLabel1_3_2 = new QLabel( tab, "textLabel1_3_2" );

    tabLayout->addWidget( textLabel1_3_2, 6, 0 );

    appearance = new QComboBox( FALSE, tab, "appearance" );

    tabLayout->addWidget( appearance, 0, 1 );

    highlightTab = new QCheckBox( tab, "highlightTab" );

    tabLayout->addMultiCellWidget( highlightTab, 8, 8, 2, 6 );

    shadowButtons = new QCheckBox( tab, "shadowButtons" );

    tabLayout->addMultiCellWidget( shadowButtons, 10, 10, 2, 5 );

    stdSidebarButtons = new QCheckBox( tab, "stdSidebarButtons" );

    tabLayout->addMultiCellWidget( stdSidebarButtons, 11, 11, 2, 6 );

    coloredMouseOver = new QComboBox( FALSE, tab, "coloredMouseOver" );

    tabLayout->addWidget( coloredMouseOver, 6, 1 );

    textLabel1_8 = new QLabel( tab, "textLabel1_8" );

    tabLayout->addWidget( textLabel1_8, 6, 2 );

    highlightFactor = new KIntNumInput( tab, "highlightFactor" );

    tabLayout->addMultiCellWidget( highlightFactor, 6, 6, 3, 6 );

    darkerBorders = new QCheckBox( tab, "darkerBorders" );

    tabLayout->addMultiCellWidget( darkerBorders, 12, 12, 2, 4 );
    optionsTab->insertTab( tab, QString::fromLatin1("") );

    tab_2 = new QWidget( optionsTab, "tab_2" );
    tabLayout_2 = new QGridLayout( tab_2, 1, 1, 11, 6, "tabLayout_2"); 

    textLabel1_4 = new QLabel( tab_2, "textLabel1_4" );

    tabLayout_2->addWidget( textLabel1_4, 0, 0 );

    handles = new QComboBox( FALSE, tab_2, "handles" );
    handles->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, handles->sizePolicy().hasHeightForWidth() ) );

    tabLayout_2->addWidget( handles, 0, 4 );

    textLabel2 = new QLabel( tab_2, "textLabel2" );

    tabLayout_2->addWidget( textLabel2, 0, 3 );
    spacer4 = new QSpacerItem( 20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum );
    tabLayout_2->addItem( spacer4, 0, 2 );

    toolbarBorders = new QComboBox( FALSE, tab_2, "toolbarBorders" );
    toolbarBorders->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, toolbarBorders->sizePolicy().hasHeightForWidth() ) );

    tabLayout_2->addWidget( toolbarBorders, 0, 1 );
    spacer5 = new QSpacerItem( 20, 54, QSizePolicy::Minimum, QSizePolicy::Expanding );
    tabLayout_2->addItem( spacer5, 3, 1 );

    groupBox2 = new QGroupBox( tab_2, "groupBox2" );
    groupBox2->setFlat( TRUE );
    groupBox2->setColumnLayout(0, Qt::Vertical );
    groupBox2->layout()->setSpacing( 6 );
    groupBox2->layout()->setMargin( 11 );
    groupBox2Layout = new QGridLayout( groupBox2->layout() );
    groupBox2Layout->setAlignment( Qt::AlignTop );

    textLabel1_4_2_2 = new QLabel( groupBox2, "textLabel1_4_2_2" );

    groupBox2Layout->addWidget( textLabel1_4_2_2, 0, 0 );
    spacer4_2 = new QSpacerItem( 20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum );
    groupBox2Layout->addItem( spacer4_2, 0, 2 );

    textLabel2_2 = new QLabel( groupBox2, "textLabel2_2" );

    groupBox2Layout->addWidget( textLabel2_2, 0, 3 );

    toolbarSeparators = new QComboBox( FALSE, groupBox2, "toolbarSeparators" );
    toolbarSeparators->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, toolbarSeparators->sizePolicy().hasHeightForWidth() ) );

    groupBox2Layout->addWidget( toolbarSeparators, 0, 4 );

    toolbarAppearance = new QComboBox( FALSE, groupBox2, "toolbarAppearance" );
    toolbarAppearance->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, toolbarAppearance->sizePolicy().hasHeightForWidth() ) );

    groupBox2Layout->addWidget( toolbarAppearance, 0, 1 );

    tabLayout_2->addMultiCellWidget( groupBox2, 2, 2, 0, 4 );

    groupBox2_2 = new QGroupBox( tab_2, "groupBox2_2" );
    groupBox2_2->setFlat( TRUE );
    groupBox2_2->setColumnLayout(0, Qt::Vertical );
    groupBox2_2->layout()->setSpacing( 6 );
    groupBox2_2->layout()->setMargin( 11 );
    groupBox2_2Layout = new QGridLayout( groupBox2_2->layout() );
    groupBox2_2Layout->setAlignment( Qt::AlignTop );

    shadeMenubars = new QComboBox( FALSE, groupBox2_2, "shadeMenubars" );
    shadeMenubars->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, shadeMenubars->sizePolicy().hasHeightForWidth() ) );

    groupBox2_2Layout->addMultiCellWidget( shadeMenubars, 1, 1, 1, 2 );

    customMenubarsColor = new KColorButton( groupBox2_2, "customMenubarsColor" );

    groupBox2_2Layout->addWidget( customMenubarsColor, 1, 3 );

    lighterPopupMenuBgnd = new QCheckBox( groupBox2_2, "lighterPopupMenuBgnd" );

    groupBox2_2Layout->addMultiCellWidget( lighterPopupMenuBgnd, 6, 6, 0, 4 );

    menubarMouseOver = new QCheckBox( groupBox2_2, "menubarMouseOver" );

    groupBox2_2Layout->addMultiCellWidget( menubarMouseOver, 3, 3, 0, 4 );

    roundMbTopOnly = new QCheckBox( groupBox2_2, "roundMbTopOnly" );

    groupBox2_2Layout->addMultiCellWidget( roundMbTopOnly, 4, 4, 0, 4 );

    textLabel1_4_2_3 = new QLabel( groupBox2_2, "textLabel1_4_2_3" );

    groupBox2_2Layout->addWidget( textLabel1_4_2_3, 1, 0 );

    textLabel1_4_2 = new QLabel( groupBox2_2, "textLabel1_4_2" );

    groupBox2_2Layout->addWidget( textLabel1_4_2, 0, 0 );

    menubarAppearance = new QComboBox( FALSE, groupBox2_2, "menubarAppearance" );

    groupBox2_2Layout->addMultiCellWidget( menubarAppearance, 0, 0, 1, 4 );

    textLabel1_4_2_4 = new QLabel( groupBox2_2, "textLabel1_4_2_4" );

    groupBox2_2Layout->addWidget( textLabel1_4_2_4, 5, 0 );

    thinnerMenuItems = new QCheckBox( groupBox2_2, "thinnerMenuItems" );

    groupBox2_2Layout->addMultiCellWidget( thinnerMenuItems, 7, 7, 0, 2 );

    shadeMenubarOnlyWhenActive = new QCheckBox( groupBox2_2, "shadeMenubarOnlyWhenActive" );

    groupBox2_2Layout->addWidget( shadeMenubarOnlyWhenActive, 1, 4 );

    menuitemAppearance = new QComboBox( FALSE, groupBox2_2, "menuitemAppearance" );

    groupBox2_2Layout->addMultiCellWidget( menuitemAppearance, 5, 5, 1, 3 );

    borderMenuitems = new QCheckBox( groupBox2_2, "borderMenuitems" );

    groupBox2_2Layout->addWidget( borderMenuitems, 5, 4 );

    customMenuTextColor = new QCheckBox( groupBox2_2, "customMenuTextColor" );
    customMenuTextColor->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, customMenuTextColor->sizePolicy().hasHeightForWidth() ) );

    groupBox2_2Layout->addMultiCellWidget( customMenuTextColor, 2, 2, 0, 1 );

    customMenuNormTextColor = new KColorButton( groupBox2_2, "customMenuNormTextColor" );

    groupBox2_2Layout->addWidget( customMenuNormTextColor, 2, 2 );

    customMenuSelTextColor = new KColorButton( groupBox2_2, "customMenuSelTextColor" );

    groupBox2_2Layout->addWidget( customMenuSelTextColor, 2, 3 );

    tabLayout_2->addMultiCellWidget( groupBox2_2, 1, 1, 0, 4 );
    optionsTab->insertTab( tab_2, QString::fromLatin1("") );

    QtCurveConfigBaseLayout->addMultiCellWidget( optionsTab, 2, 2, 0, 1 );
    spacer6_2 = new QSpacerItem( 321, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    QtCurveConfigBaseLayout->addItem( spacer6_2, 3, 0 );

    optionBtn = new KPushButton( this, "optionBtn" );

    QtCurveConfigBaseLayout->addWidget( optionBtn, 3, 1 );
    languageChange();
    resize( QSize(638, 568).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // tab order
    setTabOrder( optionsTab, appearance );
    setTabOrder( appearance, round );
    setTabOrder( round, splitters );
    setTabOrder( splitters, sliderAppearance );
    setTabOrder( sliderAppearance, shadeSliders );
    setTabOrder( shadeSliders, customSlidersColor );
    setTabOrder( customSlidersColor, fillSlider );
    setTabOrder( fillSlider, scrollbarType );
    setTabOrder( scrollbarType, sliderThumbs );
    setTabOrder( sliderThumbs, progressAppearance );
    setTabOrder( progressAppearance, gradientPbGroove );
    setTabOrder( gradientPbGroove, stripedProgress );
    setTabOrder( stripedProgress, animatedProgress );
    setTabOrder( animatedProgress, embolden );
    setTabOrder( embolden, defBtnIndicator );
    setTabOrder( defBtnIndicator, coloredMouseOver );
    setTabOrder( coloredMouseOver, highlightFactor );
    setTabOrder( highlightFactor, lvAppearance );
    setTabOrder( lvAppearance, lvLines );
    setTabOrder( lvLines, tabAppearance );
    setTabOrder( tabAppearance, highlightTab );
    setTabOrder( highlightTab, shadeCheckRadio );
    setTabOrder( shadeCheckRadio, customCheckRadioColor );
    setTabOrder( customCheckRadioColor, stdFocus );
    setTabOrder( stdFocus, shadowButtons );
    setTabOrder( shadowButtons, drawStatusBarFrames );
    setTabOrder( drawStatusBarFrames, stdSidebarButtons );
    setTabOrder( stdSidebarButtons, fixParentlessDialogs );
    setTabOrder( fixParentlessDialogs, darkerBorders );
    setTabOrder( darkerBorders, vArrows );
    setTabOrder( vArrows, toolbarBorders );
    setTabOrder( toolbarBorders, handles );
    setTabOrder( handles, menubarAppearance );
    setTabOrder( menubarAppearance, shadeMenubars );
    setTabOrder( shadeMenubars, customMenubarsColor );
    setTabOrder( customMenubarsColor, shadeMenubarOnlyWhenActive );
    setTabOrder( shadeMenubarOnlyWhenActive, customMenuTextColor );
    setTabOrder( customMenuTextColor, customMenuNormTextColor );
    setTabOrder( customMenuNormTextColor, customMenuSelTextColor );
    setTabOrder( customMenuSelTextColor, menubarMouseOver );
    setTabOrder( menubarMouseOver, roundMbTopOnly );
    setTabOrder( roundMbTopOnly, menuitemAppearance );
    setTabOrder( menuitemAppearance, borderMenuitems );
    setTabOrder( borderMenuitems, lighterPopupMenuBgnd );
    setTabOrder( lighterPopupMenuBgnd, thinnerMenuItems );
    setTabOrder( thinnerMenuItems, toolbarAppearance );
    setTabOrder( toolbarAppearance, toolbarSeparators );
    setTabOrder( toolbarSeparators, optionBtn );
}

/*
 *  Destroys the object and frees any allocated resources
 */
QtCurveConfigBase::~QtCurveConfigBase()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void QtCurveConfigBase::languageChange()
{
    setCaption( tr2i18n( "Form1" ) );
    titleLabel->setText( tr2i18n( "title" ) );
    textLabel1_6->setText( tr2i18n( "Splitters:" ) );
    textLabel1_2_3->setText( tr2i18n( "Scrollbars:" ) );
    textLabel1_2_3_2->setText( tr2i18n( "Sliders:" ) );
    textLabel1_7->setText( tr2i18n( "Appearance:" ) );
    textLabel1->setText( tr2i18n( "Progress bars:" ) );
    textLabel1_8_2->setText( tr2i18n( "Listviews:" ) );
    QWhatsThis::add( textLabel1_8_2, tr2i18n( "This controls the % that widgets will be highlighted by when the mouse hovers over them" ) );
    embolden->setText( tr2i18n( "Bold text, and" ) );
    QWhatsThis::add( progressAppearance, tr2i18n( "The setting here affects the general appearance - and will be applied to buttons, scrollbars, sliders, combo boxes, and spin buttons." ) );
    stripedProgress->setText( tr2i18n( "Striped" ) );
    gradientPbGroove->setText( tr2i18n( "Gradient groove" ) );
    animatedProgress->setText( tr2i18n( "Animated" ) );
    lvLines->setText( tr2i18n( "Lines" ) );
    stdFocus->setText( tr2i18n( "Dotted focus rectangle" ) );
    QWhatsThis::add( stdFocus, tr2i18n( "Some applications, such as Qt Designer, and Krita, make use of 'dock windows' These have a handle strip along the top - enabling this option will draw the name of the window in the handle strip." ) );
    drawStatusBarFrames->setText( tr2i18n( "Draw statusbar frames" ) );
    fixParentlessDialogs->setText( tr2i18n( "'Fix' parentless dialogs" ) );
    QWhatsThis::add( fixParentlessDialogs, tr2i18n( "<h2><font color=\"#ff0000\">Warning: Experimental!</font></h1><p><p>Some applications - such as Kate, Kaffeine, and GIMP - produce dialogs that have no 'parent'. This causes the dialog to recieve an entry in the taskbar, and allows the dialog to be minimised independantly of the main application window.</p>\n"
"\n"
"<p>If you enable this option, QtCurve will try to 'fix' this by assigning the dialogs a parent.</p>\n"
"\n"
"<p><b>Note:</b> This may brake some applications, as it wiill alter the behaviour of dialogs in a way the application has not intended. Therefore, please use with care.</p>" ) );
    vArrows->setText( tr2i18n( "'V' style arrows" ) );
    textLabel1_8_2_2->setText( tr2i18n( "Tabs:" ) );
    QWhatsThis::add( textLabel1_8_2_2, tr2i18n( "This controls the % that widgets will be highlighted by when the mouse hovers over them" ) );
    textLabel1_8_2_2_2->setText( tr2i18n( "Checks/radios:" ) );
    QWhatsThis::add( textLabel1_8_2_2_2, tr2i18n( "This controls the % that widgets will be highlighted by when the mouse hovers over them" ) );
    customCheckRadioColor->setText( QString::null );
    customSlidersColor->setText( QString::null );
    fillSlider->setText( tr2i18n( "Fill used" ) );
    textLabel1_3->setText( tr2i18n( "Default button:" ) );
    textLabel1_3_2->setText( tr2i18n( "Mouse-over:" ) );
    QWhatsThis::add( appearance, tr2i18n( "The setting here affects the general appearance - and will be applied to buttons, scrollbars, sliders, combo boxes, and spin buttons." ) );
    highlightTab->setText( tr2i18n( "Highlight selected" ) );
    QWhatsThis::add( highlightTab, tr2i18n( "This will cause a coloured stripe to be drawn over the current tab." ) );
    shadowButtons->setText( tr2i18n( "Shadow buttons" ) );
    stdSidebarButtons->setText( tr2i18n( "Standard buttons for sidebars" ) );
    QWhatsThis::add( coloredMouseOver, tr2i18n( "The setting here affects the general appearance - and will be applied to buttons, scrollbars, sliders, combo boxes, and spin buttons." ) );
    textLabel1_8->setText( tr2i18n( "Highlight by:" ) );
    QWhatsThis::add( textLabel1_8, tr2i18n( "This controls the % that widgets will be highlighted by when the mouse hovers over them" ) );
    highlightFactor->setSuffix( tr2i18n( "%" ) );
    darkerBorders->setText( tr2i18n( "Darker borders" ) );
    optionsTab->changeTab( tab, tr2i18n( "General" ) );
    textLabel1_4->setText( tr2i18n( "Border:" ) );
    textLabel2->setText( tr2i18n( "Handles:" ) );
    groupBox2->setTitle( tr2i18n( "Toolbars" ) );
    textLabel1_4_2_2->setText( tr2i18n( "Appearance:" ) );
    textLabel2_2->setText( tr2i18n( "Separators:" ) );
    groupBox2_2->setTitle( tr2i18n( "Menus" ) );
    customMenubarsColor->setText( QString::null );
    lighterPopupMenuBgnd->setText( tr2i18n( "Lighter popup-menu background" ) );
    menubarMouseOver->setText( tr2i18n( "Enable mouse-over for menubar items" ) );
    roundMbTopOnly->setText( tr2i18n( "Round selected menubar items on top only" ) );
    textLabel1_4_2_3->setText( tr2i18n( "Coloration:" ) );
    textLabel1_4_2->setText( tr2i18n( "Menubar appearance:" ) );
    textLabel1_4_2_4->setText( tr2i18n( "Menuitem appearance:" ) );
    thinnerMenuItems->setText( tr2i18n( "Thinner menu items" ) );
    shadeMenubarOnlyWhenActive->setText( tr2i18n( "Active window only" ) );
    borderMenuitems->setText( tr2i18n( "Border" ) );
    customMenuTextColor->setText( tr2i18n( "Custom text colors (normal/selected):" ) );
    customMenuNormTextColor->setText( QString::null );
    customMenuSelTextColor->setText( QString::null );
    optionsTab->changeTab( tab_2, tr2i18n( "Menus && Toolbars" ) );
    optionBtn->setText( tr2i18n( "Options" ) );
}

#include "qtcurveconfigbase.moc"
