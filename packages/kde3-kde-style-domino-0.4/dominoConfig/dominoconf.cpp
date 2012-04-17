/*
Copyright (C) 2006 Michael Lentner <michaell@gmx.net>

based on the Plastik configuration dialog:
Copyright (C) 2003 Sandro Giessl <ceebx@users.sourceforge.net>

based on the Keramik configuration dialog:
Copyright (c) 2003 Maksim Orlovich <maksim.orlovich@kdemail.net>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
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

#include <qapplication.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qsettings.h>
#include <kglobal.h>
#include <klocale.h>
#include <qpushbutton.h>
#include <qwidget.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <kcolorbutton.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qscrollbar.h>
#include <qheader.h>
#include <qlistview.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qstyle.h>
#include <qpainter.h>
#include <qframe.h>
#include <qlistbox.h>
#include <qheader.h>
#include <kinputdialog.h>
#include <kstandarddirs.h>
#include <qdir.h>
#include <qbuttongroup.h>
#include <kmessagebox.h>
#include <qtabbar.h>
#include <ktabwidget.h>
#include <qslider.h>
#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <qvbuttongroup.h>

#include <X11/Xft/Xft.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "dominoconf.h"
// #include <../domino.h>
#include <../domino/misc.cpp>

#include "configData.h"



extern "C"
{
	KDE_EXPORT QWidget* allocate_kstyle_config(QWidget* parent)
	{
		KGlobal::locale()->insertCatalogue("kstyle_domino_config");
		return new DominoStyleConfig(parent);
	}
}


DominoStyleConfig::DominoStyleConfig(QWidget* parent): QWidget(parent)
{
	
	previewStyle = new PreviewStyle();
	previewStyle->configMode = "1";
	styleConfig = this;
	initialized = false;
	
	int dummy;
	hasCompositeExtension = XQueryExtension(qt_xdisplay(), "Composite", &dummy, &dummy, &dummy);
	
	//Should have no margins here, the dialog provides them
	QVBoxLayout* layout = new QVBoxLayout(this, 0, 1);
	KGlobal::locale()->insertCatalogue("kstyle_domino_config");

	DominoKTabWidget *tabWidget = new DominoKTabWidget(this);
	layout->addWidget(tabWidget);
	
////////////////////////////////////////////////////////////////////////////
// tab 1
////////////////////////////////////////////////////////////////////////////
	
	QScrollView* sv = new QScrollView(tabWidget);
	sv->setVScrollBarMode(QScrollView::AlwaysOn);
	QWidget* scrollWidget = new QWidget(sv->viewport());
	sv->addChild(scrollWidget);
	sv->setResizePolicy( QScrollView::AutoOneFit );
	QVBoxLayout* page1layout = new QVBoxLayout(scrollWidget);
	page1layout->setMargin(5);
	tabWidget->addTab(sv, i18n("General"));
	
	animateProgressBar = new QCheckBox(i18n("Animate progress bars"), scrollWidget);
	animateProgressBar->setFocus();
	centerTabs = new QCheckBox(i18n("Center tabs"), scrollWidget);
	drawTriangularExpander = new QCheckBox(i18n("Triangular tree expander"), scrollWidget);
	smoothScrolling = new QCheckBox(i18n("Smoother scrolling (works best with line steps bigger than 3)"), scrollWidget);
	
	
	QHBox *hbox5 = new QHBox(scrollWidget);
	hbox5->layout()->addItem(new QSpacerItem(23, 0, QSizePolicy::Fixed, QSizePolicy::Minimum) );
	buttonHeightAdjustmentLabel = new QLabel(hbox5);
	buttonHeightAdjustment = new DSpinBox(-5, 5, 1, hbox5);
	buttonHeightAdjustment->setAlignRight();
	buttonHeightAdjustment->setSuffix(" px");
	
	customCheckMarkColor = new QCheckBox(i18n("Custom checkmark color"), scrollWidget);
	QHBox *hbox3 = new QHBox(scrollWidget);
	hbox3->layout()->addItem(new QSpacerItem(20, 0, QSizePolicy::Fixed, QSizePolicy::Minimum) );
	checkMarkColor = new KColorButton(hbox3);
	
	customToolTipColor = new QCheckBox(i18n("Custom tooltip color"), scrollWidget);
	QHBox *hbox4 = new QHBox(scrollWidget);
	hbox4->layout()->addItem(new QSpacerItem(20, 0, QSizePolicy::Fixed, QSizePolicy::Minimum) );
	toolTipColor = new KColorButton(hbox4);
	
	highlightToolBtnIcons = new QCheckBox(i18n("Highlight tool button icons on mouse over"), scrollWidget);
	
	toolBtnAsBtn = new QCheckBox(i18n("Draw tool buttons as normal buttons"), scrollWidget);
	
	// rubber options
	
	QHBox *hbox27 = new QHBox(scrollWidget);
	hbox27->layout()->addItem(new QSpacerItem(23, 0, QSizePolicy::Fixed, QSizePolicy::Minimum) );
	labelRubberOptions = new QLabel(i18n("Rubber band options:"), hbox27 );
	
	QHBox *hbox28 = new QHBox(scrollWidget);
	hbox28->layout()->addItem(new QSpacerItem(20, 0, QSizePolicy::Fixed, QSizePolicy::Minimum) );
	
	QVButtonGroup* btg = new QVButtonGroup(hbox28);
	btg->setColumnLayout(0, Qt::Vertical );
	btg->layout()->setSpacing( 2 );
	btg->layout()->setMargin( 11 );
	btg->setExclusive(true);
	
	lineRubber = new QRadioButton(i18n("Solid line"), btg);
	distRubber = new QRadioButton(i18n("Distribution's default"), btg);
	argbRubber = new QRadioButton(i18n("Semi transparent (requires a running composite manager)"), btg);
	argbRubber->setEnabled(hasCompositeExtension);
	
	QHBox *hbox29 = new QHBox(btg);
	hbox29->setSpacing(2);
	hbox29->layout()->addItem(new QSpacerItem(23, 0, QSizePolicy::Fixed, QSizePolicy::Minimum) );
	labelArgbRubberColor = new QLabel(i18n("Color:"), hbox29);
	argbRubberColor = new KColorButton(hbox29);
	argbRubberColor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	labelArgbRubberOpacity = new QLabel(i18n("Opacity:"), hbox29);
	argbRubberOpacity = new DSpinBox(hbox29);
	argbRubberOpacity->setAlignRight();
	argbRubberOpacity->setMaxValue(100);
	
	btg->layout()->add(lineRubber);
	btg->layout()->add(distRubber);
	btg->layout()->add(argbRubber);
	
	btg->layout()->add(hbox29);
	
	if(!hasCompositeExtension) {
		argbRubberOpacity->setEnabled(false);
		argbRubberColor->setEnabled(false);
		labelArgbRubberColor->setEnabled(false);
		labelArgbRubberOpacity->setEnabled(false);
	}
	
	connect(btg, SIGNAL(released(int)), SLOT(rubberSelChanged(int)));
	connect(argbRubber, SIGNAL(toggled(bool)), SLOT(updateArgbRubberOptions(bool)));
	
	// popupMenu options
	
	QHBox *hbox25 = new QHBox(scrollWidget);
	hbox25->layout()->addItem(new QSpacerItem(23, 0, QSizePolicy::Fixed, QSizePolicy::Minimum) );
	labelPopupMenuOptions = new QLabel(i18n("Popupmenu options:"), hbox25 );
	
	
	QHBox *hbox26 = new QHBox(scrollWidget);
	hbox26->layout()->addItem(new QSpacerItem(20, 0, QSizePolicy::Fixed, QSizePolicy::Minimum) );
	
	QGroupBox* menuOptionsGb = new QGroupBox(hbox26);
	menuOptionsGb->setColumnLayout(0, Qt::Vertical );
	menuOptionsGb->layout()->setSpacing( 2 );
	menuOptionsGb->layout()->setMargin( 11 );
	
	
	customPopupMenuColor = new QCheckBox(i18n("Custom popupmenu color"), menuOptionsGb);
	QHBox *hbox6 = new QHBox(menuOptionsGb);
	hbox6->layout()->addItem(new QSpacerItem(20, 0, QSizePolicy::Fixed, QSizePolicy::Minimum) );
	popupMenuColor = new KColorButton(hbox6);
	
	
	customSelMenuItemColor = new QCheckBox(i18n("Custom selected menu item color"), menuOptionsGb);
	QHBox *hbox24 = new QHBox(menuOptionsGb);
	hbox24->layout()->addItem(new QSpacerItem(20, 0, QSizePolicy::Fixed, QSizePolicy::Minimum) );
	selMenuItemColor = new KColorButton(hbox24);
	
	drawPopupMenuGradient = new QCheckBox(i18n("Draw a gradient"), menuOptionsGb);
	indentPopupMenuItems = new QCheckBox(i18n("Indent menu items"), menuOptionsGb);
	
	menuOptionsGb->layout()->add(customPopupMenuColor);
	menuOptionsGb->layout()->add(hbox6);
	menuOptionsGb->layout()->add(customSelMenuItemColor);
	menuOptionsGb->layout()->add(hbox24);
	menuOptionsGb->layout()->add(drawPopupMenuGradient);
	menuOptionsGb->layout()->add(indentPopupMenuItems);

	
	// groupbox
	tintGroupBoxBackground = new QCheckBox(i18n("Tint groupbox background"), scrollWidget);
	QHBox *hbox12 = new QHBox(scrollWidget);
	hbox12->layout()->addItem(new QSpacerItem(20, 0, QSizePolicy::Fixed, QSizePolicy::Minimum) );
	
	grFrame = new QGroupBox(hbox12);
	grFrame->setColumnLayout(0, Qt::Vertical );
	grFrame->layout()->setSpacing( 6 );
	grFrame->layout()->setMargin( 11 );
	
// 	grFrame->setFrameShape( QFrame::StyledPanel );
// 	grFrame->setFrameShadow( QFrame::Raised );
	QVBoxLayout* grFrameLayout = new QVBoxLayout(grFrame->layout(), QBoxLayout::BottomToTop);
	
	QFrame* prTintGroupBoxFrame = new QFrame(grFrame);
	prTintGroupBoxFrame->setPaletteBackgroundColor(qApp->palette().active().background());
	prTintGroupBoxFrame->setFrameShape( QFrame::StyledPanel );
	prTintGroupBoxFrame->setFrameShadow( QFrame::Raised );
	QVBoxLayout* prTintGroupBoxFrameLayout = new QVBoxLayout(prTintGroupBoxFrame, QBoxLayout::BottomToTop);
	prTintGroupBoxFrameLayout->setMargin(20);
	gb1 = new QGroupBox(1, Qt::Vertical, prTintGroupBoxFrame);
	gb1->setStyle(previewStyle);
	gb1->setMinimumWidth(30);
	gb1->setMinimumHeight(70);
	gb2 = new QGroupBox(1, Qt::Vertical, gb1);
	gb2->setStyle(previewStyle);
	gb3 = new QGroupBox(gb2);
	gb3->setStyle(previewStyle);
// 	grFrameLayout->addWidget(gb1);
	prTintGroupBoxFrameLayout->addWidget(gb1);
	grFrameLayout->addWidget(prTintGroupBoxFrame);
	
	grValueLabel = new QLabel("0", grFrame);
	grValueLabel->setMinimumWidth(50);
	QLabel* grLabel1 = new QLabel(i18n("Brightness:"), grFrame);
	QLabel* grLabel2 = new QLabel(i18n("brighter"), grFrame);
	QLabel* grLabel3 = new QLabel(i18n("darker"), grFrame);
	grSlider = new QSlider(-50, 50, 1, 0, Qt::Horizontal, grFrame);
	
	QHBoxLayout* grHBoxLayout1 = new QHBoxLayout(0, 0, 6);
	grHBoxLayout1->addWidget(grLabel1);
	grHBoxLayout1->addWidget(grValueLabel);
	grHBoxLayout1->addItem( new QSpacerItem( 61, 21, QSizePolicy::Preferred, QSizePolicy::Minimum ) );
	grHBoxLayout1->addWidget(grLabel2);
	grHBoxLayout1->addWidget(grSlider);
	grHBoxLayout1->addWidget(grLabel3);
	grFrameLayout->addLayout(grHBoxLayout1);
	
	QVBoxLayout* grVBoxLayout2 = new QVBoxLayout(0, 0, 6);
	customGroupBoxBackgroundColor = new QCheckBox(i18n("Custom background color"), grFrame);
	QHBox *hbox13 = new QHBox(grFrame);
	hbox13->layout()->addItem(new QSpacerItem(20, 0, QSizePolicy::Fixed, QSizePolicy::Minimum) );
	groupBoxBackgroundColor = new KColorButton(hbox13);
	
	grVBoxLayout2->addWidget(customGroupBoxBackgroundColor);
	grVBoxLayout2->addWidget(hbox13);
	grFrameLayout->addLayout(grVBoxLayout2);
	
	connect(grSlider, SIGNAL(valueChanged(int)), this, SLOT(grValueLabelNum(int)));
	connect(grSlider, SIGNAL(valueChanged(int)), this, SLOT(updateGroupBoxSettings()));
	connect(groupBoxBackgroundColor, SIGNAL(changed(const QColor&)), this, SLOT(updateGroupBoxSettings()));
	connect(tintGroupBoxBackground, SIGNAL(toggled(bool)), this, SLOT(updateGroupBoxSettings()));
	connect(tintGroupBoxBackground, SIGNAL(toggled(bool)), this, SLOT(resetGroupBoxBackgroundColor()));
	connect(tintGroupBoxBackground, SIGNAL(toggled(bool)), grFrame, SLOT(setEnabled(bool)));
	connect(customGroupBoxBackgroundColor, SIGNAL(toggled(bool)), this, SLOT(updateGroupBoxSettings()));
	connect(customGroupBoxBackgroundColor, SIGNAL(toggled(bool)), groupBoxBackgroundColor, SLOT(setEnabled(bool)));
	
	
	// focusIndicator
	indicateFocus = new QCheckBox(scrollWidget);
	QHBox *hbox14 = new QHBox(scrollWidget);
	hbox14->layout()->addItem(new QSpacerItem(20, 0, QSizePolicy::Fixed, QSizePolicy::Minimum) );
	
	
	indicateFocusFrame = new QGroupBox(hbox14);
// 	indicateFocusFrame->setFrameShape( QFrame::StyledPanel );
// 	indicateFocusFrame->setFrameShadow( QFrame::Raised );
	indicateFocusFrame->setColumnLayout(0, Qt::Vertical );
	indicateFocusFrame->layout()->setSpacing( 6 );
	indicateFocusFrame->layout()->setMargin( 11 );
	QGridLayout* indicateFocusLayout = new QGridLayout( indicateFocusFrame->layout(), 2, 2 );
	indicateFocusLayout->setAlignment( Qt::AlignTop );
	
	QHBoxLayout* hbox11 = new QHBoxLayout(0, 0, 6);
	indicatorModeCombo = new QComboBox(indicateFocusFrame);
	hbox11->addWidget( indicatorModeCombo );
	hbox11->addItem( new QSpacerItem( 61, 21, QSizePolicy::Expanding, QSizePolicy::Minimum ) );
	
	QHBoxLayout* hbox7 = new QHBoxLayout(0, 0, 6);
	labelIndicatorColor = new QLabel(indicateFocusFrame);
	labelIndicatorColor->setMinimumWidth(labelIndicatorColor->fontMetrics().width(i18n("Button color:")));
	hbox7->addWidget( labelIndicatorColor );
// 	hbox7->addItem( new QSpacerItem( 21, 20, QSizePolicy::Fixed, QSizePolicy::Minimum ) );
	indicatorColor = new KColorButton( indicateFocusFrame );
	indicatorColor->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed );
	hbox7->addWidget( indicatorColor );
	
	labelIndicatorColorOpacity = new QLabel(i18n("Opacity:"), indicateFocusFrame);
	hbox7->addWidget( labelIndicatorColorOpacity );
	indicatorOpacity = new QSpinBox(0, 100, 1, indicateFocusFrame);
	hbox7->addWidget( indicatorOpacity );
	
	
	QHBoxLayout* hbox8 = new QHBoxLayout(0, 0, 5);
	labelIndicatorBtnColor = new QLabel(indicateFocusFrame );
	hbox8->addWidget( labelIndicatorBtnColor );
// 	hbox8->addItem( new QSpacerItem( 21, 20, QSizePolicy::Fixed, QSizePolicy::Minimum ) );
	indicatorBtnColor = new KColorButton( indicateFocusFrame );
	indicatorBtnColor->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed );
	indicatorBtnColor->setFocusPolicy(QWidget::NoFocus);
	hbox8->addWidget( indicatorBtnColor );
	
	labelIndicatorBtnColorOpacity = new QLabel(i18n("Opacity:"), indicateFocusFrame);
	hbox8->addWidget( labelIndicatorBtnColorOpacity );
	indicatorBtnOpacity = new QSpinBox(0, 100, 1, indicateFocusFrame);
	hbox8->addWidget( indicatorBtnOpacity );
	
	
	QFrame* prFrame = new QFrame( indicateFocusFrame);
	prFrame->setPaletteBackgroundColor(qApp->palette().active().background());
	prFrame->setFrameShape( QFrame::StyledPanel );
	prFrame->setFrameShadow( QFrame::Raised );
	QVBoxLayout* prFrameLayout = new QVBoxLayout( prFrame, 3, 6);
	
	QHBoxLayout* hbox9 = new QHBoxLayout(0, 0, 6);
	hbox9->addItem(new QSpacerItem( 61, 21, QSizePolicy::Expanding, QSizePolicy::Minimum ));
	indicatorPrevWidget = new IndicatorPrevWidget( prFrame, "IndicatorPrevWidget"/*, Qt::WNoAutoErase*/ );
	int width = indicatorPrevWidget->fontMetrics().width(i18n("focused"));
	indicatorPrevWidget->setMinimumWidth(width+8);
	hbox9->addWidget( indicatorPrevWidget );
	hbox9->addItem(new QSpacerItem( 61, 21, QSizePolicy::Expanding, QSizePolicy::Minimum ));
	
	QHBoxLayout* hbox10 = new QHBoxLayout(0, 0, 6);
	hbox10->addItem(new QSpacerItem( 61, 21, QSizePolicy::Expanding, QSizePolicy::Minimum ));
	indicatorPrevButton = new IndicatorPrevButton( prFrame );
	width = indicatorPrevButton->fontMetrics().width(i18n("focused"));
	indicatorPrevButton->setMinimumWidth(width+16);
	hbox10->addWidget( indicatorPrevButton );
	hbox10->addItem(new QSpacerItem( 61, 21, QSizePolicy::Expanding, QSizePolicy::Minimum ));
	prFrameLayout->addLayout(hbox9);
	prFrameLayout->addLayout(hbox10);
	
	indicateFocusLayout->addMultiCellLayout( hbox11, 0, 0, 0, 2 );
	indicateFocusLayout->addLayout(hbox7, 1, 0);
	indicateFocusLayout->addLayout(hbox8, 2, 0);
	indicateFocusLayout->addMultiCellWidget( prFrame, 1, 2, 1, 2 );
	
	connect(indicatorModeCombo, SIGNAL( activated(int)), this, SLOT(indicatorModeChanged()));
	connect(indicatorModeCombo,  SIGNAL(activated(int)), this, SLOT(updateFocusIndicatorSettings()));
	connect(indicateFocus, SIGNAL(toggled(bool)), indicatorPrevButton, SLOT(setEnabled(bool)));
	connect(indicateFocus, SIGNAL(toggled(bool)), indicateFocusFrame, SLOT(setEnabled(bool)));
	
	
	
	// text effect
	drawTextEffect = new QCheckBox(scrollWidget);
	QHBox *hbox15 = new QHBox(scrollWidget);
	hbox15->layout()->addItem(new QSpacerItem(20, 0, QSizePolicy::Fixed, QSizePolicy::Minimum) );
	
	textEffectFrame = new QGroupBox(hbox15);
// 	textEffectFrame->setFrameShape( QFrame::StyledPanel );
// 	textEffectFrame->setFrameShadow( QFrame::Raised );
	textEffectFrame->setColumnLayout(0, Qt::Vertical );
	textEffectFrame->layout()->setSpacing( 6 );
	textEffectFrame->layout()->setMargin( 11 );
	QGridLayout* textEffectLayout = new QGridLayout( textEffectFrame->layout(), 2, 2 );
	textEffectLayout->setAlignment( Qt::AlignTop );
	
	QHBoxLayout* hbox16 = new QHBoxLayout(0, 0, 6);
	labelTextEffectColor = new QLabel(textEffectFrame);
	labelTextEffectColor->setMinimumWidth(labelTextEffectColor->fontMetrics().width(i18n("Button color:")));
	hbox16->addWidget( labelTextEffectColor );
	textEffectColor = new KColorButton( textEffectFrame );
	textEffectColor->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed );
	hbox16->addWidget( textEffectColor );
	
	labelTextEffectColorOpacity = new QLabel(i18n("Opacity:"), textEffectFrame);
	hbox16->addWidget( labelTextEffectColorOpacity );
	textEffectOpacity = new QSpinBox(0, 100, 1, textEffectFrame);
	hbox16->addWidget( textEffectOpacity );
	
	labelTextEffectPos = new QLabel(i18n("Position:"), textEffectFrame);
	hbox16->addWidget( labelTextEffectPos );
	
	textEffectPos = new QComboBox(textEffectFrame);
	textEffectPos->insertItem(i18n("top left"), 0);
	textEffectPos->insertItem(i18n("top"), 1);
	textEffectPos->insertItem(i18n("top right"), 2);
	textEffectPos->insertItem(i18n("right"), 3);
	textEffectPos->insertItem(i18n("bottom right"), 4);
	textEffectPos->insertItem(i18n("bottom"), 5);
	textEffectPos->insertItem(i18n("bottom left"), 6);
	textEffectPos->insertItem(i18n("left"), 7);
	hbox16->addWidget( textEffectPos );
	
	
	QHBoxLayout* hbox17 = new QHBoxLayout(0, 0, 5);
	labelTextEffectButtonColor = new QLabel(textEffectFrame);
	hbox17->addWidget( labelTextEffectButtonColor );
	textEffectButtonColor = new KColorButton( textEffectFrame );
	textEffectButtonColor->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed );
	textEffectButtonColor->setFocusPolicy(QWidget::NoFocus);
	hbox17->addWidget( textEffectButtonColor );
	
	labelTextEffectButtonColorOpacity = new QLabel(i18n("Opacity:"), textEffectFrame);
	hbox17->addWidget( labelTextEffectButtonColorOpacity );
	textEffectButtonOpacity = new QSpinBox(0, 100, 1, textEffectFrame);
	hbox17->addWidget( textEffectButtonOpacity );
	
	labelTextEffectButtonPos = new QLabel(i18n("Position:"), textEffectFrame);
	hbox17->addWidget( labelTextEffectButtonPos );
	
	textEffectButtonPos = new QComboBox(textEffectFrame);
	textEffectButtonPos->insertItem(i18n("top left"), 0);
	textEffectButtonPos->insertItem(i18n("top"), 1);
	textEffectButtonPos->insertItem(i18n("top right"), 2);
	textEffectButtonPos->insertItem(i18n("right"), 3);
	textEffectButtonPos->insertItem(i18n("bottom right"), 4);
	textEffectButtonPos->insertItem(i18n("bottom"), 5);
	textEffectButtonPos->insertItem(i18n("bottom left"), 6);
	textEffectButtonPos->insertItem(i18n("left"), 7);
	hbox17->addWidget( textEffectButtonPos );
	
	
	QFrame* prTextEffectFrame = new QFrame( textEffectFrame);
	prTextEffectFrame->setPaletteBackgroundColor(qApp->palette().active().background());
	prTextEffectFrame->setFrameShape( QFrame::StyledPanel );
	prTextEffectFrame->setFrameShadow( QFrame::Raised );
	QVBoxLayout* prTextEffectFrameLayout = new QVBoxLayout( prTextEffectFrame, 3, 6);
	
	QHBoxLayout* hbox18 = new QHBoxLayout(0, 0, 6);
	hbox18->addItem(new QSpacerItem( 61, 21, QSizePolicy::Expanding, QSizePolicy::Minimum ));
	textEffectPrevWidget = new TextEffectPrevWidget( prTextEffectFrame, "TextEffectPrevWidget",  Qt::WNoAutoErase);
	textEffectPrevWidget->setStyle(previewStyle);
	width = textEffectPrevWidget->fontMetrics().width(i18n("123 text"));
	textEffectPrevWidget->setMinimumWidth(width+8);
	hbox18->addWidget( textEffectPrevWidget );
	hbox18->addItem(new QSpacerItem( 61, 21, QSizePolicy::Expanding, QSizePolicy::Minimum ));
	
	QHBoxLayout* hbox19 = new QHBoxLayout(0, 0, 6);
	hbox19->addItem(new QSpacerItem( 61, 21, QSizePolicy::Expanding, QSizePolicy::Minimum ));
	textEffectPrevButton = new TextEffectPrevButton( prTextEffectFrame );
	textEffectPrevButton->setStyle(previewStyle);
	textEffectPrevButton->setText("123 text");
	width = textEffectPrevButton->fontMetrics().width(i18n("123 text"));
	//textEffectPrevButton->setMinimumWidth(width+16);
	
	hbox19->addWidget( textEffectPrevButton );
	hbox19->addItem(new QSpacerItem( 61, 21, QSizePolicy::Expanding, QSizePolicy::Minimum ));
	prTextEffectFrameLayout->addLayout(hbox18);
	prTextEffectFrameLayout->addLayout(hbox19);

	textEffectModeCombo = new QComboBox(textEffectFrame);
	QHBoxLayout* hbox20 = new QHBoxLayout(0, 0, 6);
	hbox20->addWidget( textEffectModeCombo );
	hbox20->addItem( new QSpacerItem( 61, 21, QSizePolicy::Expanding, QSizePolicy::Minimum ) );
	
	textEffectLayout->addMultiCellLayout( hbox20, 0, 0, 0, 2 );
	textEffectLayout->addLayout(hbox16, 1, 0);
	textEffectLayout->addLayout(hbox17, 2, 0);
	textEffectLayout->addMultiCellWidget( prTextEffectFrame, 1, 2, 1, 2 );
	
	connect(drawTextEffect, SIGNAL(toggled(bool)), textEffectFrame, SLOT(setEnabled(bool)));
	connect(drawTextEffect, SIGNAL(toggled(bool)), this, SLOT(updateTextEffectSettings()));
	connect(textEffectModeCombo, SIGNAL( activated(int)), this, SLOT(textEffectModeChanged()));
	connect(textEffectModeCombo, SIGNAL( activated(int)), this, SLOT(updateTextEffectSettings()));
	
	
	drawButtonSunkenShadow = new QCheckBox(i18n("Draw a shadow on pressed sunken buttons"), scrollWidget);
	connect(drawButtonSunkenShadow, SIGNAL(toggled(bool)), this, SLOT(updateButtonContourSettings()));
	
	QHBox *hbox21 = new QHBox(scrollWidget);
	hbox21->layout()->addItem(new QSpacerItem(23, 0, QSizePolicy::Fixed, QSizePolicy::Minimum) );
	buttonTypLabel = new QLabel(i18n("Button Type:"), hbox21);
	buttonContourType = new QComboBox(hbox21);
	buttonContourType->insertItem(i18n("Sunken"), 0);
	buttonContourType->insertItem(i18n("Raised"), 1);
	
	
	
	// button contour colors
	QHBox *hbox22 = new QHBox(scrollWidget);
	hbox22->layout()->addItem(new QSpacerItem(23, 0, QSizePolicy::Fixed, QSizePolicy::Minimum) );
	labelButtonContourColors = new QLabel(i18n("Button contour colors:"), hbox22 );
	
	
	QHBox *hbox23 = new QHBox(scrollWidget);
	hbox23->layout()->addItem(new QSpacerItem(20, 0, QSizePolicy::Fixed, QSizePolicy::Minimum) );
	
	QGroupBox* buttonContourColorFrame = new QGroupBox(hbox23);
// 	buttonContourColorFrame->setFrameShape( QFrame::StyledPanel );
// 	buttonContourColorFrame->setFrameShadow( QFrame::Raised );
	buttonContourColorFrame->setColumnLayout(0, Qt::Vertical );
	buttonContourColorFrame->layout()->setSpacing( 6 );
	buttonContourColorFrame->layout()->setMargin( 11 );
	
	QGridLayout* buttonContourColorFrameLayout = new QGridLayout( buttonContourColorFrame->layout(), 4, 3 );
	buttonContourColorFrameLayout->setAlignment( Qt::AlignTop );
	
	QLabel* labelSC = new QLabel(i18n("Standard color:"), buttonContourColorFrame);
	buttonContourColor = new KColorButton(buttonContourColorFrame);
	QLabel* labelMC = new QLabel(i18n("Mouseover color:"), buttonContourColorFrame);
	buttonMouseOverContourColor = new KColorButton(buttonContourColorFrame);
	QLabel* labelPC = new QLabel(i18n("Pressed color:"), buttonContourColorFrame);
	buttonPressedContourColor = new KColorButton(buttonContourColorFrame);
	QLabel* labelDBC = new QLabel(i18n("Defaultbutton color:"), buttonContourColorFrame);
	buttonDefaultButtonContourColor = new KColorButton(buttonContourColorFrame);
	
	buttonContourColorFrameLayout->addWidget(labelSC, 0, 0);
	buttonContourColorFrameLayout->addItem(new QSpacerItem(20, 0, QSizePolicy::Preferred, QSizePolicy::Minimum) );
	buttonContourColorFrameLayout->addWidget(buttonContourColor, 0, 1);
	buttonContourColorFrameLayout->addItem(new QSpacerItem(20, 0, QSizePolicy::Expanding, QSizePolicy::Minimum) );
	
	buttonContourColorFrameLayout->addWidget(labelMC, 1, 0);
	buttonContourColorFrameLayout->addItem(new QSpacerItem(20, 0, QSizePolicy::Preferred, QSizePolicy::Minimum) );
	buttonContourColorFrameLayout->addWidget(buttonMouseOverContourColor, 1, 1);
	
	buttonContourColorFrameLayout->addWidget(labelPC, 2, 0);
	buttonContourColorFrameLayout->addItem(new QSpacerItem(20, 0, QSizePolicy::Preferred, QSizePolicy::Minimum) );
	buttonContourColorFrameLayout->addWidget(buttonPressedContourColor, 2, 1);
	
	buttonContourColorFrameLayout->addWidget(labelDBC, 3, 0);
	buttonContourColorFrameLayout->addItem(new QSpacerItem(20, 0, QSizePolicy::Preferred, QSizePolicy::Minimum) );
	buttonContourColorFrameLayout->addWidget(buttonDefaultButtonContourColor, 3, 1);
	
	
	
	QGroupBox* prButtonContourColorFrame = new QGroupBox(buttonContourColorFrame);
	prButtonContourColorFrame->setPaletteBackgroundColor(qApp->palette().active().background());
	prButtonContourColorFrame->setFrameShape( QFrame::StyledPanel );
	prButtonContourColorFrame->setFrameShadow( QFrame::Raised );
	
	
	QHBoxLayout* prButtonContourColorFrameLayout = new QHBoxLayout( prButtonContourColorFrame, 3, 6);
	
	prButtonContourColorFrameLayout->addItem(new QSpacerItem( 61, 21, QSizePolicy::Expanding, QSizePolicy::Minimum ));
	buttonContourPrevButton = new QPushButton(prButtonContourColorFrame);
	((DominoQWidget*)buttonContourPrevButton)->setWFlags(Qt::WNoAutoErase);
	buttonContourPrevButton->setStyle(previewStyle);
	prButtonContourColorFrameLayout->addWidget( buttonContourPrevButton );
	prButtonContourColorFrameLayout->addItem(new QSpacerItem( 61, 21, QSizePolicy::Expanding, QSizePolicy::Minimum ));
	
	buttonContourColorFrameLayout->addMultiCellWidget(prButtonContourColorFrame, 0, 3, 2, 2);
	
	
	connect(buttonContourType, SIGNAL(activated(int)), this, SLOT(updateButtonContourSettings()));
	
	connect(buttonContourColor, SIGNAL(changed(const QColor&)), this, SLOT(updateButtonContourSettings()));
	connect(buttonMouseOverContourColor, SIGNAL(changed(const QColor&)), this, SLOT(updateButtonContourSettings()));
	connect(buttonPressedContourColor, SIGNAL(changed(const QColor&)), this, SLOT(updateButtonContourSettings()));
	connect(buttonDefaultButtonContourColor, SIGNAL(changed(const QColor&)), this, SLOT(updateButtonContourSettings()));
	
	
	
	
	page1layout->add(animateProgressBar);
	page1layout->add(centerTabs);
	page1layout->add(drawTriangularExpander);
	page1layout->add(smoothScrolling);
	page1layout->add(highlightToolBtnIcons);
	page1layout->add(toolBtnAsBtn);
	page1layout->add(hbox5);
	page1layout->add(customCheckMarkColor);
	page1layout->add(hbox3);
	page1layout->add(customToolTipColor);
	page1layout->add(hbox4);
	page1layout->add(hbox27);
	page1layout->addSpacing(3);
	page1layout->add(hbox28);
	page1layout->addSpacing(8);
	page1layout->add(hbox25);
	page1layout->addSpacing(3);
	page1layout->add(hbox26);
	page1layout->addSpacing(8);
	page1layout->add(tintGroupBoxBackground);
	page1layout->add(hbox12);
	page1layout->addSpacing(8);
	page1layout->add(indicateFocus);
	page1layout->add(hbox14);
	page1layout->addSpacing(8);
	page1layout->add(drawTextEffect);
	page1layout->add(hbox15);
	page1layout->addSpacing(8);
	page1layout->add(drawButtonSunkenShadow);
	page1layout->addSpacing(8);
	page1layout->add(hbox21);
	page1layout->addSpacing(8);
	page1layout->add(hbox22);
	page1layout->addSpacing(3);
	page1layout->add(hbox23);
	page1layout->addStretch(1);
	
	
	QWidget *page2 = new QWidget();
	tabWidget->addTab(page2, "Custom Gradients");
	
	QVBoxLayout* page2layout = new QVBoxLayout(page2);

////////////////////////////////////////////////////////////////////////////
// tab 2 (custom Gradients)
////////////////////////////////////////////////////////////////////////////

	tabWidget2 = new DominoKTabWidget( page2, "tabWidget2" );
	connect(tabWidget2, SIGNAL(testCanDecode(const QDragMoveEvent *, bool &)), SLOT(testCanDecode(const QDragMoveEvent *, bool &)));
	page2layout->add(tabWidget2);

////////////////////////////////////////////////////////////////////////////
// tab 2.1 (buttons)
////////////////////////////////////////////////////////////////////////////

	tab1 = new QWidget( tabWidget2, "tab1" );
	tab1Layout = new QVBoxLayout( tab1, 11, 6, "tab1Layout"); 
	
	groupBox22 = new QGroupBox( tab1, "groupBox22" );
	groupBox22->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, groupBox22->sizePolicy().hasHeightForWidth() ) );
	groupBox22->setColumnLayout(0, Qt::Vertical );
	groupBox22->layout()->setSpacing( 0 );
	groupBox22->layout()->setMargin( 11 );
	groupBox22Layout = new QGridLayout( groupBox22->layout() );
	groupBox22Layout->setAlignment( Qt::AlignTop );
	spacer6 = new QSpacerItem( 83, 25, QSizePolicy::Expanding, QSizePolicy::Minimum );
	groupBox22Layout->addItem( spacer6, 3, 3 );

	textLabel1_2 = new QLabel( groupBox22, "textLabel1_2" );
	textLabel1_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)5, 0, 0, textLabel1_2->sizePolicy().hasHeightForWidth() ) );
	textLabel1_2->setTextFormat( QLabel::PlainText );

	groupBox22Layout->addWidget( textLabel1_2, 3, 4 );

	kColorButton27_2 = new KColorButton( groupBox22, "kColorButton27_2" );
	kColorButton27_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, kColorButton27_2->sizePolicy().hasHeightForWidth() ) );
	kColorButton27_2->setMinimumSize( QSize( 75, 0 ) );

	groupBox22Layout->addWidget( kColorButton27_2, 3, 5 );
	spacer3 = new QSpacerItem( 12, 25, QSizePolicy::Fixed, QSizePolicy::Minimum );
	groupBox22Layout->addItem( spacer3, 3, 6 );

	groupBox10_2_2_2 = new QGroupBox( groupBox22, "groupBox10_2_2_2" );
	groupBox10_2_2_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)5, 0, 0, groupBox10_2_2_2->sizePolicy().hasHeightForWidth() ) );
	groupBox10_2_2_2->setMinimumSize( QSize( 170, 0 ) );
	groupBox10_2_2_2->setColumnLayout(0, Qt::Vertical );
	groupBox10_2_2_2->layout()->setSpacing( 6 );
	groupBox10_2_2_2->layout()->setMargin( 11 );
	groupBox10_2_2_2Layout = new QVBoxLayout( groupBox10_2_2_2->layout() );
	groupBox10_2_2_2Layout->setAlignment( Qt::AlignTop );


	pushButton16 = new QPushButton(groupBox10_2_2_2, "dominoPreviewWidget" );
	pushButton16->setStyle(previewStyle);
	groupBox10_2_2_2Layout->addWidget( pushButton16 );

	comboBox17 = new QComboBox( FALSE, groupBox10_2_2_2, "dominoPreviewWidget" );
	comboBox17->setStyle(previewStyle);
	groupBox10_2_2_2Layout->addWidget( comboBox17 );

	spinBox44 = new DSpinBox( groupBox10_2_2_2, "dominoPreviewWidget" );
	spinBox44->spinWidget()->setStyle(previewStyle);
	spinBox44->setAlignRight();
	groupBox10_2_2_2Layout->addWidget( spinBox44 );

	groupBox22Layout->addMultiCellWidget( groupBox10_2_2_2, 0, 1, 0, 0 );
	spacer43 = new QSpacerItem( 5, 20, QSizePolicy::Fixed, QSizePolicy::Minimum );
	groupBox22Layout->addMultiCell( spacer43, 0, 1, 1, 1 );

	groupBox8_2_2_2 = new QGroupBox( groupBox22, "groupBox8_2_2_2" );
	groupBox8_2_2_2->setCheckable( TRUE );
	connect(((QCheckBox*)groupBox8_2_2_2->child("qt_groupbox_checkbox", 0, false)), SIGNAL(clicked()), SLOT(btn1GradientChanged()));
	groupBox8_2_2_2->setColumnLayout(0, Qt::Vertical );
	groupBox8_2_2_2->layout()->setSpacing( 7 );
	groupBox8_2_2_2->layout()->setMargin( 4 );
	groupBox8_2_2_2Layout = new QGridLayout( groupBox8_2_2_2->layout() );
	groupBox8_2_2_2Layout->setAlignment( Qt::AlignTop );

	textLabel4_3_2_2_2 = new QLabel( groupBox8_2_2_2, "textLabel4_3_2_2_2" );
	textLabel4_3_2_2_2->setTextFormat( QLabel::PlainText );

	groupBox8_2_2_2Layout->addWidget( textLabel4_3_2_2_2, 1, 0 );

	spinBox2_2_2_2_2 = new DSpinBox( groupBox8_2_2_2, "spinBox2_2_2_2_2" );
	spinBox2_2_2_2_2->setAlignRight();
	spinBox2_2_2_2_2->setMaxValue( 100 );

	groupBox8_2_2_2Layout->addWidget( spinBox2_2_2_2_2, 1, 1 );

	textLabel2_2_2_2_2 = new QLabel( groupBox8_2_2_2, "textLabel2_2_2_2_2" );
	textLabel2_2_2_2_2->setTextFormat( QLabel::PlainText );

	groupBox8_2_2_2Layout->addWidget( textLabel2_2_2_2_2, 1, 2 );

	kColorButton1_2_2_2_2 = new KColorButton( groupBox8_2_2_2, "kColorButton1_2_2_2_2" );

	groupBox8_2_2_2Layout->addWidget( kColorButton1_2_2_2_2, 1, 3 );

	textLabel3_3_2_2_2 = new QLabel( groupBox8_2_2_2, "textLabel3_3_2_2_2" );
	textLabel3_3_2_2_2->setTextFormat( QLabel::PlainText );

	groupBox8_2_2_2Layout->addWidget( textLabel3_3_2_2_2, 0, 0 );

	spinBox1_2_2_2_2 = new DSpinBox( groupBox8_2_2_2, "spinBox1_2_2_2_2" );
	spinBox1_2_2_2_2->setAlignRight();
	spinBox1_2_2_2_2->setMaxValue( 100 );

	groupBox8_2_2_2Layout->addWidget( spinBox1_2_2_2_2, 0, 1 );

	textLabel1_4_2_2_2 = new QLabel( groupBox8_2_2_2, "textLabel1_4_2_2_2" );
	textLabel1_4_2_2_2->setTextFormat( QLabel::PlainText );

	groupBox8_2_2_2Layout->addWidget( textLabel1_4_2_2_2, 0, 2 );

	kColorButton2_4_2_2_2 = new KColorButton( groupBox8_2_2_2, "kColorButton2_4_2_2_2" );

	groupBox8_2_2_2Layout->addWidget( kColorButton2_4_2_2_2, 0, 3 );

	groupBox22Layout->addMultiCellWidget( groupBox8_2_2_2, 0, 0, 2, 6 );

	groupBox9_2_2_2 = new QGroupBox( groupBox22, "groupBox9_2_2_2" );
	groupBox9_2_2_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, groupBox9_2_2_2->sizePolicy().hasHeightForWidth() ) );
	groupBox9_2_2_2->setCheckable( TRUE );
	connect(((QCheckBox*)groupBox9_2_2_2->child("qt_groupbox_checkbox", 0, false)), SIGNAL(clicked()), SLOT(btn2GradientChanged()));
	groupBox9_2_2_2->setChecked( FALSE );
	groupBox9_2_2_2->setColumnLayout(0, Qt::Vertical );
	groupBox9_2_2_2->layout()->setSpacing( 7 );
	groupBox9_2_2_2->layout()->setMargin( 4 );
	groupBox9_2_2_2Layout = new QGridLayout( groupBox9_2_2_2->layout() );
	groupBox9_2_2_2Layout->setAlignment( Qt::AlignTop );

	textLabel4_4_2_2_2 = new QLabel( groupBox9_2_2_2, "textLabel4_4_2_2_2" );
	textLabel4_4_2_2_2->setTextFormat( QLabel::PlainText );

	groupBox9_2_2_2Layout->addWidget( textLabel4_4_2_2_2, 1, 0 );

	spinBox2_3_2_2_2 = new DSpinBox( groupBox9_2_2_2, "spinBox2_3_2_2_2" );
	spinBox2_3_2_2_2->setAlignRight();
	spinBox2_3_2_2_2->setMaxValue( 100 );

	groupBox9_2_2_2Layout->addWidget( spinBox2_3_2_2_2, 1, 1 );

	textLabel3_4_2_2_2 = new QLabel( groupBox9_2_2_2, "textLabel3_4_2_2_2" );
	textLabel3_4_2_2_2->setTextFormat( QLabel::PlainText );

	groupBox9_2_2_2Layout->addWidget( textLabel3_4_2_2_2, 0, 0 );

	spinBox1_3_2_2_2 = new DSpinBox( groupBox9_2_2_2, "spinBox1_3_2_2_2" );
	spinBox1_3_2_2_2->setAlignRight();
	spinBox1_3_2_2_2->setMaxValue( 100 );

	groupBox9_2_2_2Layout->addWidget( spinBox1_3_2_2_2, 0, 1 );

	kColorButton1_3_2_2_2 = new KColorButton( groupBox9_2_2_2, "kColorButton1_3_2_2_2" );
	kColorButton1_3_2_2_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, kColorButton1_3_2_2_2->sizePolicy().hasHeightForWidth() ) );

	groupBox9_2_2_2Layout->addWidget( kColorButton1_3_2_2_2, 1, 3 );

	kColorButton2_5_2_2_2 = new KColorButton( groupBox9_2_2_2, "kColorButton2_5_2_2_2" );

	groupBox9_2_2_2Layout->addWidget( kColorButton2_5_2_2_2, 0, 3 );

	textLabel1_5_2_2_2 = new QLabel( groupBox9_2_2_2, "textLabel1_5_2_2_2" );
	textLabel1_5_2_2_2->setTextFormat( QLabel::PlainText );

	groupBox9_2_2_2Layout->addWidget( textLabel1_5_2_2_2, 0, 2 );

	textLabel2_3_2_2_2 = new QLabel( groupBox9_2_2_2, "textLabel2_3_2_2_2" );
	textLabel2_3_2_2_2->setTextFormat( QLabel::PlainText );

	groupBox9_2_2_2Layout->addWidget( textLabel2_3_2_2_2, 1, 2 );

	groupBox22Layout->addMultiCellWidget( groupBox9_2_2_2, 1, 1, 2, 6 );
	spacer46 = new QSpacerItem( 20, 7, QSizePolicy::Minimum, QSizePolicy::Fixed );
	groupBox22Layout->addItem( spacer46, 2, 4 );
	spacer2 = new QSpacerItem( 82, 25, QSizePolicy::Expanding, QSizePolicy::Minimum );
	groupBox22Layout->addItem( spacer2, 3, 2 );
	tab1Layout->addWidget( groupBox22 );

	layout9 = new QHBoxLayout( 0, 0, 6, "layout9"); 
	spacer7 = new QSpacerItem( 270, 21, QSizePolicy::Expanding, QSizePolicy::Minimum );
	layout9->addItem( spacer7 );

	textLabel7_4 = new QLabel( tab1, "textLabel7_4" );
	layout9->addWidget( textLabel7_4 );

	comboBox3_4 = new QComboBox( FALSE, tab1, "comboBox3_4" );
	layout9->addWidget( comboBox3_4 );

	pushButton4_4 = new QPushButton( tab1, "pushButton4_4" );
	connect(pushButton4_4, SIGNAL(clicked()), SLOT(copyColorsToButton()));
	layout9->addWidget( pushButton4_4 );
	tab1Layout->addLayout( layout9 );
	tabWidget2->insertTab( tab1, QString::fromLatin1("") );
	
/////////////////////////////////////////////////
// tab 2.2 (tabs)
/////////////////////////////////////////////////
	   tab2 = new QWidget( tabWidget2, "tab2" );
	   tab2Layout = new QVBoxLayout( tab2, 11, 6, "tab2Layout"); 

	   groupBox22_2 = new QGroupBox( tab2, "groupBox22_2" );
	   groupBox22_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, groupBox22_2->sizePolicy().hasHeightForWidth() ) );
	   groupBox22_2->setColumnLayout(0, Qt::Vertical );
	   groupBox22_2->layout()->setSpacing( 0 );
	   groupBox22_2->layout()->setMargin( 11 );
	   groupBox22_2Layout = new QGridLayout( groupBox22_2->layout() );
	   groupBox22_2Layout->setAlignment( Qt::AlignTop );
	   spacer46_2 = new QSpacerItem( 20, 7, QSizePolicy::Minimum, QSizePolicy::Fixed );
	   groupBox22_2Layout->addItem( spacer46_2, 2, 5 );
	   spacer43_2 = new QSpacerItem( 5, 20, QSizePolicy::Fixed, QSizePolicy::Minimum );
	   groupBox22_2Layout->addMultiCell( spacer43_2, 0, 1, 1, 1 );

	   groupBox8_2_2_2_2 = new QGroupBox( groupBox22_2, "groupBox8_2_2_2_2" );
	   groupBox8_2_2_2_2->setCheckable( TRUE );
	   connect(((QCheckBox*)groupBox8_2_2_2_2->child("qt_groupbox_checkbox", 0, false)), SIGNAL(clicked()), SLOT(tab1GradientChanged()));
	   groupBox8_2_2_2_2->setColumnLayout(0, Qt::Vertical );
	   groupBox8_2_2_2_2->layout()->setSpacing( 7 );
	   groupBox8_2_2_2_2->layout()->setMargin( 4 );
	   groupBox8_2_2_2_2Layout = new QGridLayout( groupBox8_2_2_2_2->layout() );
	   groupBox8_2_2_2_2Layout->setAlignment( Qt::AlignTop );

	   textLabel4_3_2_2_2_2 = new QLabel( groupBox8_2_2_2_2, "textLabel4_3_2_2_2_2" );
	   textLabel4_3_2_2_2_2->setTextFormat( QLabel::PlainText );

	   groupBox8_2_2_2_2Layout->addWidget( textLabel4_3_2_2_2_2, 1, 0 );

	   spinBox2_2_2_2_2_2 = new DSpinBox( groupBox8_2_2_2_2, "spinBox2_2_2_2_2_2" );
	   spinBox2_2_2_2_2_2->setAlignRight();
	   spinBox2_2_2_2_2_2->setMaxValue( 100 );

	   groupBox8_2_2_2_2Layout->addWidget( spinBox2_2_2_2_2_2, 1, 1 );

	   textLabel2_2_2_2_2_2 = new QLabel( groupBox8_2_2_2_2, "textLabel2_2_2_2_2_2" );
	   textLabel2_2_2_2_2_2->setTextFormat( QLabel::PlainText );

	   groupBox8_2_2_2_2Layout->addWidget( textLabel2_2_2_2_2_2, 1, 2 );

	   kColorButton1_2_2_2_2_2 = new KColorButton( groupBox8_2_2_2_2, "kColorButton1_2_2_2_2_2" );

	   groupBox8_2_2_2_2Layout->addWidget( kColorButton1_2_2_2_2_2, 1, 3 );

	   textLabel3_3_2_2_2_2 = new QLabel( groupBox8_2_2_2_2, "textLabel3_3_2_2_2_2" );
	   textLabel3_3_2_2_2_2->setTextFormat( QLabel::PlainText );

	   groupBox8_2_2_2_2Layout->addWidget( textLabel3_3_2_2_2_2, 0, 0 );

	   spinBox1_2_2_2_2_2 = new DSpinBox( groupBox8_2_2_2_2, "spinBox1_2_2_2_2_2" );
	   spinBox1_2_2_2_2_2->setAlignRight();
	   spinBox1_2_2_2_2_2->setMaxValue( 100 );

	   groupBox8_2_2_2_2Layout->addWidget( spinBox1_2_2_2_2_2, 0, 1 );

	   textLabel1_4_2_2_2_2 = new QLabel( groupBox8_2_2_2_2, "textLabel1_4_2_2_2_2" );
	   textLabel1_4_2_2_2_2->setTextFormat( QLabel::PlainText );

	   groupBox8_2_2_2_2Layout->addWidget( textLabel1_4_2_2_2_2, 0, 2 );

	   kColorButton2_4_2_2_2_2 = new KColorButton( groupBox8_2_2_2_2, "kColorButton2_4_2_2_2_2" );

	   groupBox8_2_2_2_2Layout->addWidget( kColorButton2_4_2_2_2_2, 0, 3 );

	   groupBox22_2Layout->addMultiCellWidget( groupBox8_2_2_2_2, 0, 0, 2, 6 );

	   groupBox9_2_2_2_2 = new QGroupBox( groupBox22_2, "groupBox9_2_2_2_2" );
	   groupBox9_2_2_2_2->setCheckable( TRUE );
	   connect(((QCheckBox*)groupBox9_2_2_2_2->child("qt_groupbox_checkbox", 0, false)), SIGNAL(clicked()), SLOT(tab2GradientChanged()));
	   groupBox9_2_2_2_2->setChecked( FALSE );
	   groupBox9_2_2_2_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, groupBox9_2_2_2_2->sizePolicy().hasHeightForWidth() ) );
	   groupBox9_2_2_2_2->setColumnLayout(0, Qt::Vertical );
	   groupBox9_2_2_2_2->layout()->setSpacing( 7 );
	   groupBox9_2_2_2_2->layout()->setMargin( 4 );
	   groupBox9_2_2_2_2Layout = new QGridLayout( groupBox9_2_2_2_2->layout() );
	   groupBox9_2_2_2_2Layout->setAlignment( Qt::AlignTop );

	   textLabel4_4_2_2_2_2 = new QLabel( groupBox9_2_2_2_2, "textLabel4_4_2_2_2_2" );
	   textLabel4_4_2_2_2_2->setTextFormat( QLabel::PlainText );

	   groupBox9_2_2_2_2Layout->addWidget( textLabel4_4_2_2_2_2, 1, 0 );

	   spinBox2_3_2_2_2_2 = new DSpinBox( groupBox9_2_2_2_2, "spinBox2_3_2_2_2_2" );
	   spinBox2_3_2_2_2_2->setButtonSymbols( QSpinBox::UpDownArrows );
	   spinBox2_3_2_2_2_2->setAlignRight();
	   spinBox2_3_2_2_2_2->setMaxValue( 100 );

	   groupBox9_2_2_2_2Layout->addWidget( spinBox2_3_2_2_2_2, 1, 1 );

	   textLabel3_4_2_2_2_2 = new QLabel( groupBox9_2_2_2_2, "textLabel3_4_2_2_2_2" );
	   textLabel3_4_2_2_2_2->setTextFormat( QLabel::PlainText );

	   groupBox9_2_2_2_2Layout->addWidget( textLabel3_4_2_2_2_2, 0, 0 );

	   spinBox1_3_2_2_2_2 = new DSpinBox( groupBox9_2_2_2_2, "spinBox1_3_2_2_2_2" );
	   spinBox1_3_2_2_2_2->setAlignRight();
	   spinBox1_3_2_2_2_2->setMaxValue( 100 );

	   groupBox9_2_2_2_2Layout->addWidget( spinBox1_3_2_2_2_2, 0, 1 );

	   kColorButton1_3_2_2_2_2 = new KColorButton( groupBox9_2_2_2_2, "kColorButton1_3_2_2_2_2" );
	   kColorButton1_3_2_2_2_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, kColorButton1_3_2_2_2_2->sizePolicy().hasHeightForWidth() ) );

	   groupBox9_2_2_2_2Layout->addWidget( kColorButton1_3_2_2_2_2, 1, 3 );

	   kColorButton2_5_2_2_2_2 = new KColorButton( groupBox9_2_2_2_2, "kColorButton2_5_2_2_2_2" );

	   groupBox9_2_2_2_2Layout->addWidget( kColorButton2_5_2_2_2_2, 0, 3 );

	   textLabel1_5_2_2_2_2 = new QLabel( groupBox9_2_2_2_2, "textLabel1_5_2_2_2_2" );
	   textLabel1_5_2_2_2_2->setTextFormat( QLabel::PlainText );

	   groupBox9_2_2_2_2Layout->addWidget( textLabel1_5_2_2_2_2, 0, 2 );

	   textLabel2_3_2_2_2_2 = new QLabel( groupBox9_2_2_2_2, "textLabel2_3_2_2_2_2" );
	   textLabel2_3_2_2_2_2->setTextFormat( QLabel::PlainText );

	   groupBox9_2_2_2_2Layout->addWidget( textLabel2_3_2_2_2_2, 1, 2 );

	   groupBox22_2Layout->addMultiCellWidget( groupBox9_2_2_2_2, 1, 1, 2, 6 );
	   spacer6_2 = new QSpacerItem( 83, 25, QSizePolicy::Expanding, QSizePolicy::Minimum );
	   groupBox22_2Layout->addItem( spacer6_2, 3, 3 );

	   textLabel1_2_2 = new QLabel( groupBox22_2, "textLabel1_2_2" );
	   textLabel1_2_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)5, 0, 0, textLabel1_2_2->sizePolicy().hasHeightForWidth() ) );
	   textLabel1_2_2->setTextFormat( QLabel::PlainText );

	   groupBox22_2Layout->addWidget( textLabel1_2_2, 3, 4 );
	   spacer2_2 = new QSpacerItem( 82, 25, QSizePolicy::Expanding, QSizePolicy::Minimum );
	   groupBox22_2Layout->addItem( spacer2_2, 3, 2 );
	   spacer3_2 = new QSpacerItem( 12, 25, QSizePolicy::Fixed, QSizePolicy::Minimum );
	   groupBox22_2Layout->addItem( spacer3_2, 3, 6 );

	   kColorButton27_2_2 = new KColorButton( groupBox22_2, "kColorButton27_2_2" );
	   kColorButton27_2_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, kColorButton27_2_2->sizePolicy().hasHeightForWidth() ) );
	   kColorButton27_2_2->setMinimumSize( QSize( 75, 0 ) );

	   groupBox22_2Layout->addWidget( kColorButton27_2_2, 3, 5 );

	   groupBox10_2_2_2_2 = new QGroupBox( groupBox22_2, "groupBox10_2_2_2_2" );
	   groupBox10_2_2_2_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)5, 0, 0, groupBox10_2_2_2_2->sizePolicy().hasHeightForWidth() ) );
	   groupBox10_2_2_2_2->setMinimumSize( QSize( 170, 0 ) );
	   groupBox10_2_2_2_2->setColumnLayout(0, Qt::Vertical );
	   groupBox10_2_2_2_2->layout()->setSpacing( 6 );
	   groupBox10_2_2_2_2->layout()->setMargin( 11 );
	   groupBox10_2_2_2_2Layout = new QVBoxLayout( groupBox10_2_2_2_2->layout() );
	   groupBox10_2_2_2_2Layout->setAlignment( Qt::AlignTop );
	   
	   prevTopTabWidget = new DominoKTabPrevWidget(groupBox10_2_2_2_2);
	   prevTopTabWidget->setStyle(previewStyle);
	   tabBarTop = new PreviewTabBar(0);
	   tabBarTop->setStyle(previewStyle);
	   ((DominoQTabWidget*)prevTopTabWidget)->setTabBar(tabBarTop);
	   
	   prevTopPage1 = new TabWidgetIndicator(prevTopTabWidget);
	   prevTopPage2 = new TabWidgetIndicator(prevTopTabWidget);
	   prevTopPage3 = new TabWidgetIndicator(prevTopTabWidget);
	   
	   prevTopTabWidget->addTab(prevTopPage1, "  1  ");
	   prevTopTabWidget->addTab(prevTopPage2, "  2  ");
	   prevTopTabWidget->addTab(prevTopPage3, "  3  ");
	   prevTopTabWidget->page(0)->parentWidget()->setPaletteBackgroundColor(qApp->palette().active().background());
	   prevTopTabWidget->setCurrentPage(1);
	   
	   prevBottomTabWidget = new DominoKTabPrevWidget(groupBox10_2_2_2_2);
	   prevBottomTabWidget->setStyle(previewStyle);
	   tabBarBottom = new PreviewTabBar(0);
	   tabBarBottom->setStyle(previewStyle);
	   ((DominoQTabWidget*)prevBottomTabWidget)->setTabBar(tabBarBottom);
	   prevBottomTabWidget->setTabPosition(QTabWidget::Bottom);
	   
	   prevBottomPage1 = new TabWidgetIndicator(prevBottomTabWidget);
	   prevBottomPage2 = new TabWidgetIndicator(prevBottomTabWidget);
	   prevBottomPage3 = new TabWidgetIndicator(prevBottomTabWidget);
	   
	   prevBottomTabWidget->addTab(prevBottomPage1, "  1  ");
	   prevBottomTabWidget->addTab(prevBottomPage2, "  2  ");
	   prevBottomTabWidget->addTab(prevBottomPage3, "  3  ");
	   prevBottomTabWidget->page(0)->parentWidget()->setPaletteBackgroundColor(qApp->palette().active().background());
	   prevBottomTabWidget->setCurrentPage(1);
	   
	   groupBox10_2_2_2_2Layout->addWidget(prevTopTabWidget);
	   groupBox10_2_2_2_2Layout->addWidget(prevBottomTabWidget);
	   
	   
	   groupBox22_2Layout->addMultiCellWidget( groupBox10_2_2_2_2, 0, 1, 0, 0 );
	   tab2Layout->addWidget( groupBox22_2 );

	   layout9_2 = new QHBoxLayout( 0, 0, 6, "layout9_2"); 
	   
	   tabComboLayout = new QHBoxLayout( 0, 0, 6, "tabComboLayout");
	   tabPosCombo = new QComboBox( FALSE, tab2, "tabPosCombo" );
	   tabComboLayout->addWidget( tabPosCombo );
	   
	   tabStateCombo = new QComboBox( FALSE, tab2, "tabStateCombo" );
	   tabComboLayout->addWidget( tabStateCombo );
	   
	   QSpacerItem* spacer_combo2 = new QSpacerItem( 20, 21, QSizePolicy::Expanding, QSizePolicy::Minimum );
	   tabComboLayout->addItem( spacer_combo2 );
	   
	   layout9_2->addLayout( tabComboLayout );
	   
	   
	   spacer7_2 = new QSpacerItem( 270, 21, QSizePolicy::Expanding, QSizePolicy::Minimum );
	   layout9_2->addItem( spacer7_2 );

	   textLabel7_4_2 = new QLabel( tab2, "textLabel7_4_2" );
	   layout9_2->addWidget( textLabel7_4_2 );
	   
	   comboBox3_4_2 = new QComboBox( FALSE, tab2, "comboBox3_4_2" );
	   layout9_2->addWidget( comboBox3_4_2 );

	   pushButton4_4_2 = new QPushButton( tab2, "pushButton4_4_2" );
	   connect(pushButton4_4_2, SIGNAL(clicked()), SLOT(copyColorsToTab()));
	   layout9_2->addWidget( pushButton4_4_2 );
	   
	   tab2Layout->addLayout( layout9_2 );
	   tabWidget2->insertTab( tab2, QString::fromLatin1("") );
	   
	   
	
	
/////////////////////////////////////////////////
// tab 2.3 (scrollbars)
/////////////////////////////////////////////////
	tab3 = new QWidget( tabWidget2, "tab3" );
	tab3Layout = new QVBoxLayout( tab3, 11, 6, "tab3Layout"); 

	groupBox22_2_2 = new QGroupBox( tab3, "groupBox22_2_2" );
	groupBox22_2_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, groupBox22_2_2->sizePolicy().hasHeightForWidth() ) );
	groupBox22_2_2->setColumnLayout(0, Qt::Vertical );
	groupBox22_2_2->layout()->setSpacing( 0 );
	groupBox22_2_2->layout()->setMargin( 11 );
	groupBox22_2_2Layout = new QGridLayout( groupBox22_2_2->layout() );
	groupBox22_2_2Layout->setAlignment( Qt::AlignTop );

	groupBox8_2_2_2_2_4 = new QGroupBox( groupBox22_2_2, "groupBox8_2_2_2_2_4" );
	groupBox8_2_2_2_2_4->setCheckable( TRUE );
	groupBox8_2_2_2_2_4->setColumnLayout(0, Qt::Vertical );
	groupBox8_2_2_2_2_4->layout()->setSpacing( 7 );
	groupBox8_2_2_2_2_4->layout()->setMargin( 4 );
	groupBox8_2_2_2_2_4Layout = new QGridLayout( groupBox8_2_2_2_2_4->layout() );
	groupBox8_2_2_2_2_4Layout->setAlignment( Qt::AlignTop );
	connect(((QCheckBox*)groupBox8_2_2_2_2_4->child("qt_groupbox_checkbox", 0, false)), SIGNAL(clicked()), SLOT(scrollBar1GradientChanged()));

	textLabel4_3_2_2_2_2_4 = new QLabel( groupBox8_2_2_2_2_4, "textLabel4_3_2_2_2_2_4" );
	textLabel4_3_2_2_2_2_4->setTextFormat( QLabel::PlainText );

	groupBox8_2_2_2_2_4Layout->addWidget( textLabel4_3_2_2_2_2_4, 1, 0 );

	spinBox2_2_2_2_2_2_4 = new DSpinBox( groupBox8_2_2_2_2_4, "spinBox2_2_2_2_2_2_4" );
	spinBox2_2_2_2_2_2_4->setAlignRight();
	spinBox2_2_2_2_2_2_4->setMaxValue( 100 );

	groupBox8_2_2_2_2_4Layout->addWidget( spinBox2_2_2_2_2_2_4, 1, 1 );

	textLabel2_2_2_2_2_2_4 = new QLabel( groupBox8_2_2_2_2_4, "textLabel2_2_2_2_2_2_4" );
	textLabel2_2_2_2_2_2_4->setTextFormat( QLabel::PlainText );

	groupBox8_2_2_2_2_4Layout->addWidget( textLabel2_2_2_2_2_2_4, 1, 2 );

	kColorButton1_2_2_2_2_2_4 = new KColorButton( groupBox8_2_2_2_2_4, "kColorButton1_2_2_2_2_2_4" );

	groupBox8_2_2_2_2_4Layout->addWidget( kColorButton1_2_2_2_2_2_4, 1, 3 );

	textLabel3_3_2_2_2_2_4 = new QLabel( groupBox8_2_2_2_2_4, "textLabel3_3_2_2_2_2_4" );
	textLabel3_3_2_2_2_2_4->setTextFormat( QLabel::PlainText );

	groupBox8_2_2_2_2_4Layout->addWidget( textLabel3_3_2_2_2_2_4, 0, 0 );

	spinBox1_2_2_2_2_2_4 = new DSpinBox( groupBox8_2_2_2_2_4, "spinBox1_2_2_2_2_2_4" );
	spinBox1_2_2_2_2_2_4->setAlignRight();
	spinBox1_2_2_2_2_2_4->setMaxValue( 100 );

	groupBox8_2_2_2_2_4Layout->addWidget( spinBox1_2_2_2_2_2_4, 0, 1 );

	textLabel1_4_2_2_2_2_4 = new QLabel( groupBox8_2_2_2_2_4, "textLabel1_4_2_2_2_2_4" );
	textLabel1_4_2_2_2_2_4->setTextFormat( QLabel::PlainText );

	groupBox8_2_2_2_2_4Layout->addWidget( textLabel1_4_2_2_2_2_4, 0, 2 );

	kColorButton2_4_2_2_2_2_4 = new KColorButton( groupBox8_2_2_2_2_4, "kColorButton2_4_2_2_2_2_4" );

	groupBox8_2_2_2_2_4Layout->addWidget( kColorButton2_4_2_2_2_2_4, 0, 3 );

	groupBox22_2_2Layout->addMultiCellWidget( groupBox8_2_2_2_2_4, 0, 0, 2, 6 );
	spacer46_2_2 = new QSpacerItem( 20, 7, QSizePolicy::Minimum, QSizePolicy::Fixed );
	groupBox22_2_2Layout->addItem( spacer46_2_2, 2, 5 );

	groupBox9_2_2_2_2_4 = new QGroupBox( groupBox22_2_2, "groupBox9_2_2_2_2_4" );
	groupBox9_2_2_2_2_4->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, groupBox9_2_2_2_2_4->sizePolicy().hasHeightForWidth() ) );
	groupBox9_2_2_2_2_4->setCheckable( TRUE );
	groupBox9_2_2_2_2_4->setChecked( FALSE );
	groupBox9_2_2_2_2_4->setColumnLayout(0, Qt::Vertical );
	groupBox9_2_2_2_2_4->layout()->setSpacing( 7 );
	groupBox9_2_2_2_2_4->layout()->setMargin( 4 );
	groupBox9_2_2_2_2_4Layout = new QGridLayout( groupBox9_2_2_2_2_4->layout() );
	groupBox9_2_2_2_2_4Layout->setAlignment( Qt::AlignTop );

	connect(((QCheckBox*)groupBox9_2_2_2_2_4->child("qt_groupbox_checkbox", 0, false)), SIGNAL(clicked()), SLOT(scrollBar2GradientChanged()));
	
	textLabel4_4_2_2_2_2_4 = new QLabel( groupBox9_2_2_2_2_4, "textLabel4_4_2_2_2_2_4" );
	textLabel4_4_2_2_2_2_4->setTextFormat( QLabel::PlainText );

	groupBox9_2_2_2_2_4Layout->addWidget( textLabel4_4_2_2_2_2_4, 1, 0 );

	spinBox2_3_2_2_2_2_4 = new DSpinBox( groupBox9_2_2_2_2_4, "spinBox2_3_2_2_2_2_4" );
	spinBox2_3_2_2_2_2_4->setAlignRight();
	spinBox2_3_2_2_2_2_4->setMaxValue( 100 );

	groupBox9_2_2_2_2_4Layout->addWidget( spinBox2_3_2_2_2_2_4, 1, 1 );

	textLabel3_4_2_2_2_2_4 = new QLabel( groupBox9_2_2_2_2_4, "textLabel3_4_2_2_2_2_4" );
	textLabel3_4_2_2_2_2_4->setTextFormat( QLabel::PlainText );

	groupBox9_2_2_2_2_4Layout->addWidget( textLabel3_4_2_2_2_2_4, 0, 0 );

	spinBox1_3_2_2_2_2_4 = new DSpinBox( groupBox9_2_2_2_2_4, "spinBox1_3_2_2_2_2_4" );
	spinBox1_3_2_2_2_2_4->setAlignRight();
	spinBox1_3_2_2_2_2_4->setMaxValue( 100 );

	groupBox9_2_2_2_2_4Layout->addWidget( spinBox1_3_2_2_2_2_4, 0, 1 );

	kColorButton1_3_2_2_2_2_4 = new KColorButton( groupBox9_2_2_2_2_4, "kColorButton1_3_2_2_2_2_4" );
	kColorButton1_3_2_2_2_2_4->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, kColorButton1_3_2_2_2_2_4->sizePolicy().hasHeightForWidth() ) );

	groupBox9_2_2_2_2_4Layout->addWidget( kColorButton1_3_2_2_2_2_4, 1, 3 );

	kColorButton2_5_2_2_2_2_4 = new KColorButton( groupBox9_2_2_2_2_4, "kColorButton2_5_2_2_2_2_4" );

	groupBox9_2_2_2_2_4Layout->addWidget( kColorButton2_5_2_2_2_2_4, 0, 3 );

	textLabel1_5_2_2_2_2_4 = new QLabel( groupBox9_2_2_2_2_4, "textLabel1_5_2_2_2_2_4" );
	textLabel1_5_2_2_2_2_4->setTextFormat( QLabel::PlainText );

	groupBox9_2_2_2_2_4Layout->addWidget( textLabel1_5_2_2_2_2_4, 0, 2 );

	textLabel2_3_2_2_2_2_4 = new QLabel( groupBox9_2_2_2_2_4, "textLabel2_3_2_2_2_2_4" );
	textLabel2_3_2_2_2_2_4->setTextFormat( QLabel::PlainText );

	groupBox9_2_2_2_2_4Layout->addWidget( textLabel2_3_2_2_2_2_4, 1, 2 );

	groupBox22_2_2Layout->addMultiCellWidget( groupBox9_2_2_2_2_4, 1, 1, 2, 6 );

	groupBox10_2_2_2_2_4 = new QGroupBox( groupBox22_2_2, "groupBox10_2_2_2_2_4" );
	groupBox10_2_2_2_2_4->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)5, 0, 0, groupBox10_2_2_2_2_4->sizePolicy().hasHeightForWidth() ) );
	groupBox10_2_2_2_2_4->setMinimumSize( QSize( 170, 0 ) );
	groupBox10_2_2_2_2_4->setColumnLayout(0, Qt::Vertical );
	groupBox10_2_2_2_2_4->layout()->setSpacing( 6 );
	groupBox10_2_2_2_2_4->layout()->setMargin( 11 );
	groupBox10_2_2_2_2_4Layout = new QGridLayout( groupBox10_2_2_2_2_4->layout() );
	groupBox10_2_2_2_2_4Layout->setAlignment( Qt::AlignTop );
	spacer35 = new QSpacerItem( 112, 21, QSizePolicy::Expanding, QSizePolicy::Minimum );
	groupBox10_2_2_2_2_4Layout->addItem( spacer35, 0, 0 );

	scrollBar5 = new QScrollBar( groupBox10_2_2_2_2_4, "dominoPreviewWidget" );
	scrollBar5->setStyle(previewStyle);
	
	scrollBar5->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
	scrollBar5->setOrientation( QScrollBar::Vertical );
	scrollBar5->setMaxValue(50);
	groupBox10_2_2_2_2_4Layout->addWidget( scrollBar5, 0, 1 );

	scrollBar6 = new QScrollBar( groupBox10_2_2_2_2_4, "dominoPreviewWidget" );
	scrollBar6->setStyle(previewStyle);
	scrollBar6->setOrientation( QScrollBar::Horizontal );
	scrollBar6->setMaxValue(50);
	groupBox10_2_2_2_2_4Layout->addWidget( scrollBar6, 1, 0 );
	

	//////////////////////////////////
	groupBox22_2_2Layout->addMultiCellWidget( groupBox10_2_2_2_2_4, 0, 1, 0, 0 );
	spacer43_2_2 = new QSpacerItem( 5, 20, QSizePolicy::Fixed, QSizePolicy::Minimum );
	groupBox22_2_2Layout->addItem( spacer43_2_2, 1, 1 );
	spacer3_2_2 = new QSpacerItem( 12, 25, QSizePolicy::Fixed, QSizePolicy::Minimum );
	groupBox22_2_2Layout->addItem( spacer3_2_2, 3, 6 );
	spacer6_2_2 = new QSpacerItem( 83, 25, QSizePolicy::Expanding, QSizePolicy::Minimum );
	groupBox22_2_2Layout->addItem( spacer6_2_2, 3, 3 );

	kColorButton27_2_2_2 = new KColorButton( groupBox22_2_2, "kColorButton27_2_2_2" );
	kColorButton27_2_2_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, kColorButton27_2_2_2->sizePolicy().hasHeightForWidth() ) );
	kColorButton27_2_2_2->setMinimumSize( QSize( 75, 0 ) );

	groupBox22_2_2Layout->addWidget( kColorButton27_2_2_2, 3, 5 );
	spacer2_2_2 = new QSpacerItem( 82, 25, QSizePolicy::Expanding, QSizePolicy::Minimum );
	groupBox22_2_2Layout->addItem( spacer2_2_2, 3, 2 );

	textLabel1_2_2_2 = new QLabel( groupBox22_2_2, "textLabel1_2_2_2" );
	textLabel1_2_2_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)5, 0, 0, textLabel1_2_2_2->sizePolicy().hasHeightForWidth() ) );
	textLabel1_2_2_2->setTextFormat( QLabel::PlainText );

	groupBox22_2_2Layout->addWidget( textLabel1_2_2_2, 3, 4 );
	tab3Layout->addWidget( groupBox22_2_2 );

	layout6 = new QHBoxLayout( 0, 0, 6, "layout6");
	comboBox7 = new QComboBox( FALSE, tab3, "comboBox7" );
	layout6->addWidget( comboBox7 );
	spacer7_2_3 = new QSpacerItem( 246, 21, QSizePolicy::Expanding, QSizePolicy::Minimum );
	layout6->addItem( spacer7_2_3 );

	textLabel7_4_2_3 = new QLabel( tab3, "textLabel7_4_2_3" );
	layout6->addWidget( textLabel7_4_2_3 );

	comboBox3_4_2_3 = new QComboBox( FALSE, tab3, "comboBox3_4_2_3" );
	layout6->addWidget( comboBox3_4_2_3 );

	pushButton4_4_2_3 = new QPushButton( tab3, "pushButton4_4_2_3" );
	connect(pushButton4_4_2_3, SIGNAL(clicked()), SLOT(copyColorsToScrollBar()));
	
	layout6->addWidget( pushButton4_4_2_3 );
	tab3Layout->addLayout( layout6 );
	tabWidget2->insertTab( tab3, QString::fromLatin1("") );

/////////////////////////////////////////////////
// tab 2.4 (headers)
/////////////////////////////////////////////////
	tab4 = new QWidget( tabWidget2, "tab4" );
	tab4Layout = new QVBoxLayout( tab4, 11, 6, "tab4Layout"); 

	groupBox22_2_2_3 = new QGroupBox( tab4, "groupBox22_2_2_3" );
	groupBox22_2_2_3->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, groupBox22_2_2_3->sizePolicy().hasHeightForWidth() ) );
	groupBox22_2_2_3->setColumnLayout(0, Qt::Vertical );
	groupBox22_2_2_3->layout()->setSpacing( 0 );
	groupBox22_2_2_3->layout()->setMargin( 11 );
	groupBox22_2_2_3Layout = new QGridLayout( groupBox22_2_2_3->layout() );
	groupBox22_2_2_3Layout->setAlignment( Qt::AlignTop );
	spacer3_2_2_2 = new QSpacerItem( 12, 25, QSizePolicy::Fixed, QSizePolicy::Minimum );
	groupBox22_2_2_3Layout->addItem( spacer3_2_2_2, 3, 6 );
	spacer6_2_2_2 = new QSpacerItem( 83, 25, QSizePolicy::Expanding, QSizePolicy::Minimum );
	groupBox22_2_2_3Layout->addItem( spacer6_2_2_2, 3, 3 );

	kColorButton27_2_2_2_2 = new KColorButton( groupBox22_2_2_3, "kColorButton27_2_2_2_2" );
	kColorButton27_2_2_2_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, kColorButton27_2_2_2_2->sizePolicy().hasHeightForWidth() ) );
	kColorButton27_2_2_2_2->setMinimumSize( QSize( 75, 0 ) );

	groupBox22_2_2_3Layout->addWidget( kColorButton27_2_2_2_2, 3, 5 );
	spacer2_2_2_2 = new QSpacerItem( 82, 25, QSizePolicy::Expanding, QSizePolicy::Minimum );
	groupBox22_2_2_3Layout->addItem( spacer2_2_2_2, 3, 2 );
	spacer46_2_2_2 = new QSpacerItem( 20, 7, QSizePolicy::Minimum, QSizePolicy::Fixed );
	groupBox22_2_2_3Layout->addItem( spacer46_2_2_2, 2, 5 );

	textLabel1_2_2_2_2 = new QLabel( groupBox22_2_2_3, "textLabel1_2_2_2_2" );
	textLabel1_2_2_2_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)5, 0, 0, textLabel1_2_2_2_2->sizePolicy().hasHeightForWidth() ) );
	textLabel1_2_2_2_2->setTextFormat( QLabel::PlainText );

	groupBox22_2_2_3Layout->addWidget( textLabel1_2_2_2_2, 3, 4 );
	spacer43_2_2_2 = new QSpacerItem( 5, 20, QSizePolicy::Fixed, QSizePolicy::Minimum );
	groupBox22_2_2_3Layout->addItem( spacer43_2_2_2, 1, 1 );

	groupBox10_2_2_2_2_4_3 = new QGroupBox( groupBox22_2_2_3, "groupBox10_2_2_2_2_4_3" );
	groupBox10_2_2_2_2_4_3->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)5, 0, 0, groupBox10_2_2_2_2_4_3->sizePolicy().hasHeightForWidth() ) );
	groupBox10_2_2_2_2_4_3->setMinimumSize( QSize( 170, 0 ) );
	groupBox10_2_2_2_2_4_3->setColumnLayout(0, Qt::Vertical );
	groupBox10_2_2_2_2_4_3->layout()->setSpacing( 6 );
	groupBox10_2_2_2_2_4_3->layout()->setMargin( 11 );
	groupBox10_2_2_2_2_4_3Layout = new QVBoxLayout( groupBox10_2_2_2_2_4_3->layout() );
	groupBox10_2_2_2_2_4_3Layout->setAlignment( Qt::AlignTop );

	frame3 = new QFrame( groupBox10_2_2_2_2_4_3, "frame3" );
	frame3->setFrameShape( QFrame::StyledPanel );
	frame3->setFrameShadow( QFrame::Raised ); // ?^
	frame3->setLineWidth( 1 );
	frame3Layout = new QVBoxLayout( frame3, 2, 6, "frame3Layout");
	
	layout1 = new QVBoxLayout( 0, 0, 0, "layout1");
	
	header1 = new QHeader(frame3, "dominoPreviewWidget");
	header1->setStyle(previewStyle);
	header1->setMovingEnabled(false);
	header1->setStretchEnabled(true);
	header1->addLabel("Header");
	
	
	
	layout1->addWidget( header1);
	listBox1 = new QListBox( frame3, "listBox1" );
	listBox1->setLineWidth(0);
	
	layout1->addWidget( listBox1 );
	frame3Layout->addLayout( layout1 );
	groupBox10_2_2_2_2_4_3Layout->addWidget( frame3 );

	//////////////////////////////////////
	groupBox22_2_2_3Layout->addMultiCellWidget( groupBox10_2_2_2_2_4_3, 0, 1, 0, 0 );

	groupBox8_2_2_2_2_4_3 = new QGroupBox( groupBox22_2_2_3, "groupBox8_2_2_2_2_4_3" );
	groupBox8_2_2_2_2_4_3->setCheckable( TRUE );
	connect(((QCheckBox*)groupBox8_2_2_2_2_4_3->child("qt_groupbox_checkbox", 0, false)), SIGNAL(clicked()), SLOT(header1GradientChanged()));

	groupBox8_2_2_2_2_4_3->setColumnLayout(0, Qt::Vertical );
	groupBox8_2_2_2_2_4_3->layout()->setSpacing( 7 );
	groupBox8_2_2_2_2_4_3->layout()->setMargin( 4 );
	groupBox8_2_2_2_2_4_3Layout = new QGridLayout( groupBox8_2_2_2_2_4_3->layout() );
	groupBox8_2_2_2_2_4_3Layout->setAlignment( Qt::AlignTop );

	textLabel4_3_2_2_2_2_4_3 = new QLabel( groupBox8_2_2_2_2_4_3, "textLabel4_3_2_2_2_2_4_3" );
	textLabel4_3_2_2_2_2_4_3->setTextFormat( QLabel::PlainText );

	groupBox8_2_2_2_2_4_3Layout->addWidget( textLabel4_3_2_2_2_2_4_3, 1, 0 );

	spinBox2_2_2_2_2_2_4_3 = new DSpinBox( groupBox8_2_2_2_2_4_3, "spinBox2_2_2_2_2_2_4_3" );
	spinBox2_2_2_2_2_2_4_3->setAlignRight();
	spinBox2_2_2_2_2_2_4_3->setMaxValue( 100 );

	groupBox8_2_2_2_2_4_3Layout->addWidget( spinBox2_2_2_2_2_2_4_3, 1, 1 );

	textLabel2_2_2_2_2_2_4_3 = new QLabel( groupBox8_2_2_2_2_4_3, "textLabel2_2_2_2_2_2_4_3" );
	textLabel2_2_2_2_2_2_4_3->setTextFormat( QLabel::PlainText );

	groupBox8_2_2_2_2_4_3Layout->addWidget( textLabel2_2_2_2_2_2_4_3, 1, 2 );

	kColorButton1_2_2_2_2_2_4_3 = new KColorButton( groupBox8_2_2_2_2_4_3, "kColorButton1_2_2_2_2_2_4_3" );

	groupBox8_2_2_2_2_4_3Layout->addWidget( kColorButton1_2_2_2_2_2_4_3, 1, 3 );

	textLabel3_3_2_2_2_2_4_3 = new QLabel( groupBox8_2_2_2_2_4_3, "textLabel3_3_2_2_2_2_4_3" );
	textLabel3_3_2_2_2_2_4_3->setTextFormat( QLabel::PlainText );

	groupBox8_2_2_2_2_4_3Layout->addWidget( textLabel3_3_2_2_2_2_4_3, 0, 0 );

	spinBox1_2_2_2_2_2_4_3 = new DSpinBox( groupBox8_2_2_2_2_4_3, "spinBox1_2_2_2_2_2_4_3" );
	spinBox1_2_2_2_2_2_4_3->setAlignRight();
	spinBox1_2_2_2_2_2_4_3->setMaxValue( 100 );

	groupBox8_2_2_2_2_4_3Layout->addWidget( spinBox1_2_2_2_2_2_4_3, 0, 1 );

	textLabel1_4_2_2_2_2_4_3 = new QLabel( groupBox8_2_2_2_2_4_3, "textLabel1_4_2_2_2_2_4_3" );
	textLabel1_4_2_2_2_2_4_3->setTextFormat( QLabel::PlainText );

	groupBox8_2_2_2_2_4_3Layout->addWidget( textLabel1_4_2_2_2_2_4_3, 0, 2 );

	kColorButton2_4_2_2_2_2_4_3 = new KColorButton( groupBox8_2_2_2_2_4_3, "kColorButton2_4_2_2_2_2_4_3" );

	groupBox8_2_2_2_2_4_3Layout->addWidget( kColorButton2_4_2_2_2_2_4_3, 0, 3 );

	groupBox22_2_2_3Layout->addMultiCellWidget( groupBox8_2_2_2_2_4_3, 0, 0, 2, 6 );

	groupBox9_2_2_2_2_4_3 = new QGroupBox( groupBox22_2_2_3, "groupBox9_2_2_2_2_4_3" );
	groupBox9_2_2_2_2_4_3->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, groupBox9_2_2_2_2_4_3->sizePolicy().hasHeightForWidth() ) );
	groupBox9_2_2_2_2_4_3->setCheckable( TRUE );
	connect(((QCheckBox*)groupBox9_2_2_2_2_4_3->child("qt_groupbox_checkbox", 0, false)), SIGNAL(clicked()), SLOT(header2GradientChanged()));
	groupBox9_2_2_2_2_4_3->setChecked( FALSE );
	groupBox9_2_2_2_2_4_3->setColumnLayout(0, Qt::Vertical );
	groupBox9_2_2_2_2_4_3->layout()->setSpacing( 7 );
	groupBox9_2_2_2_2_4_3->layout()->setMargin( 4 );
	groupBox9_2_2_2_2_4_3Layout = new QGridLayout( groupBox9_2_2_2_2_4_3->layout() );
	groupBox9_2_2_2_2_4_3Layout->setAlignment( Qt::AlignTop );

	textLabel4_4_2_2_2_2_4_3 = new QLabel( groupBox9_2_2_2_2_4_3, "textLabel4_4_2_2_2_2_4_3" );
	textLabel4_4_2_2_2_2_4_3->setTextFormat( QLabel::PlainText );

	groupBox9_2_2_2_2_4_3Layout->addWidget( textLabel4_4_2_2_2_2_4_3, 1, 0 );

	spinBox2_3_2_2_2_2_4_3 = new DSpinBox( groupBox9_2_2_2_2_4_3, "spinBox2_3_2_2_2_2_4_3" );
	spinBox2_3_2_2_2_2_4_3->setAlignRight();
	spinBox2_3_2_2_2_2_4_3->setMaxValue( 100 );

	groupBox9_2_2_2_2_4_3Layout->addWidget( spinBox2_3_2_2_2_2_4_3, 1, 1 );

	textLabel3_4_2_2_2_2_4_3 = new QLabel( groupBox9_2_2_2_2_4_3, "textLabel3_4_2_2_2_2_4_3" );
	textLabel3_4_2_2_2_2_4_3->setTextFormat( QLabel::PlainText );

	groupBox9_2_2_2_2_4_3Layout->addWidget( textLabel3_4_2_2_2_2_4_3, 0, 0 );

	spinBox1_3_2_2_2_2_4_3 = new DSpinBox( groupBox9_2_2_2_2_4_3, "spinBox1_3_2_2_2_2_4_3" );
	spinBox1_3_2_2_2_2_4_3->setAlignRight();
	spinBox1_3_2_2_2_2_4_3->setMaxValue( 100 );

	groupBox9_2_2_2_2_4_3Layout->addWidget( spinBox1_3_2_2_2_2_4_3, 0, 1 );

	kColorButton1_3_2_2_2_2_4_3 = new KColorButton( groupBox9_2_2_2_2_4_3, "kColorButton1_3_2_2_2_2_4_3" );
	kColorButton1_3_2_2_2_2_4_3->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, kColorButton1_3_2_2_2_2_4_3->sizePolicy().hasHeightForWidth() ) );

	groupBox9_2_2_2_2_4_3Layout->addWidget( kColorButton1_3_2_2_2_2_4_3, 1, 3 );

	kColorButton2_5_2_2_2_2_4_3 = new KColorButton( groupBox9_2_2_2_2_4_3, "kColorButton2_5_2_2_2_2_4_3" );

	groupBox9_2_2_2_2_4_3Layout->addWidget( kColorButton2_5_2_2_2_2_4_3, 0, 3 );

	textLabel1_5_2_2_2_2_4_3 = new QLabel( groupBox9_2_2_2_2_4_3, "textLabel1_5_2_2_2_2_4_3" );
	textLabel1_5_2_2_2_2_4_3->setTextFormat( QLabel::PlainText );

	groupBox9_2_2_2_2_4_3Layout->addWidget( textLabel1_5_2_2_2_2_4_3, 0, 2 );

	textLabel2_3_2_2_2_2_4_3 = new QLabel( groupBox9_2_2_2_2_4_3, "textLabel2_3_2_2_2_2_4_3" );
	textLabel2_3_2_2_2_2_4_3->setTextFormat( QLabel::PlainText );

	groupBox9_2_2_2_2_4_3Layout->addWidget( textLabel2_3_2_2_2_2_4_3, 1, 2 );

	groupBox22_2_2_3Layout->addMultiCellWidget( groupBox9_2_2_2_2_4_3, 1, 1, 2, 6 );
	tab4Layout->addWidget( groupBox22_2_2_3 );

	layout21 = new QHBoxLayout( 0, 0, 6, "layout21"); 
	spacer7_2_3_3 = new QSpacerItem( 354, 21, QSizePolicy::Expanding, QSizePolicy::Minimum );
	layout21->addItem( spacer7_2_3_3 );

	textLabel7_4_2_3_3 = new QLabel( tab4, "textLabel7_4_2_3_3" );
	layout21->addWidget( textLabel7_4_2_3_3 );

	comboBox3_4_2_3_3 = new QComboBox( FALSE, tab4, "comboBox3_4_2_3_3" );
	layout21->addWidget( comboBox3_4_2_3_3 );

	pushButton4_4_2_3_3 = new QPushButton( tab4, "pushButton4_4_2_3_3" );
	connect(pushButton4_4_2_3_3, SIGNAL(clicked()), SLOT(copyColorsToHeader()));
	layout21->addWidget( pushButton4_4_2_3_3 );
	tab4Layout->addLayout( layout21 );
	tabWidget2->insertTab( tab4, QString::fromLatin1("") );

/////////////////////////////////////////////////
// tab 2.5 (radio/checkboxes)
/////////////////////////////////////////////////

	tab5 = new QWidget( tabWidget2, "tab5" );
	tab5Layout = new QVBoxLayout( tab5, 11, 6, "tab5Layout"); 

	groupBox22_2_2_3_2 = new QGroupBox( tab5, "groupBox22_2_2_3_2" );
	groupBox22_2_2_3_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, groupBox22_2_2_3_2->sizePolicy().hasHeightForWidth() ) );
	groupBox22_2_2_3_2->setColumnLayout(0, Qt::Vertical );
	groupBox22_2_2_3_2->layout()->setSpacing( 0 );
	groupBox22_2_2_3_2->layout()->setMargin( 11 );
	groupBox22_2_2_3_2Layout = new QGridLayout( groupBox22_2_2_3_2->layout() );
	groupBox22_2_2_3_2Layout->setAlignment( Qt::AlignTop );

	groupBox8_2_2_2_2_4_3_2 = new QGroupBox( groupBox22_2_2_3_2, "groupBox8_2_2_2_2_4_3_2" );
	groupBox8_2_2_2_2_4_3_2->setCheckable( TRUE );
	connect(((QCheckBox*)groupBox8_2_2_2_2_4_3_2->child("qt_groupbox_checkbox", 0, false)), SIGNAL(clicked()), SLOT(checkItem1GradientChanged()));
	groupBox8_2_2_2_2_4_3_2->setChecked( FALSE );
	groupBox8_2_2_2_2_4_3_2->setColumnLayout(0, Qt::Vertical );
	groupBox8_2_2_2_2_4_3_2->layout()->setSpacing( 7 );
	groupBox8_2_2_2_2_4_3_2->layout()->setMargin( 4 );
	groupBox8_2_2_2_2_4_3_2Layout = new QGridLayout( groupBox8_2_2_2_2_4_3_2->layout() );
	groupBox8_2_2_2_2_4_3_2Layout->setAlignment( Qt::AlignTop );

	textLabel4_3_2_2_2_2_4_3_2 = new QLabel( groupBox8_2_2_2_2_4_3_2, "textLabel4_3_2_2_2_2_4_3_2" );
	textLabel4_3_2_2_2_2_4_3_2->setTextFormat( QLabel::PlainText );

	groupBox8_2_2_2_2_4_3_2Layout->addWidget( textLabel4_3_2_2_2_2_4_3_2, 1, 0 );

	spinBox2_2_2_2_2_2_4_3_2 = new DSpinBox( groupBox8_2_2_2_2_4_3_2, "spinBox2_2_2_2_2_2_4_3_2" );
	spinBox2_2_2_2_2_2_4_3_2->setAlignRight();
	spinBox2_2_2_2_2_2_4_3_2->setMaxValue( 100 );

	groupBox8_2_2_2_2_4_3_2Layout->addWidget( spinBox2_2_2_2_2_2_4_3_2, 1, 1 );

	textLabel2_2_2_2_2_2_4_3_2 = new QLabel( groupBox8_2_2_2_2_4_3_2, "textLabel2_2_2_2_2_2_4_3_2" );
	textLabel2_2_2_2_2_2_4_3_2->setTextFormat( QLabel::PlainText );

	groupBox8_2_2_2_2_4_3_2Layout->addWidget( textLabel2_2_2_2_2_2_4_3_2, 1, 2 );

	kColorButton1_2_2_2_2_2_4_3_2 = new KColorButton( groupBox8_2_2_2_2_4_3_2, "kColorButton1_2_2_2_2_2_4_3_2" );

	groupBox8_2_2_2_2_4_3_2Layout->addWidget( kColorButton1_2_2_2_2_2_4_3_2, 1, 3 );

	textLabel3_3_2_2_2_2_4_3_2 = new QLabel( groupBox8_2_2_2_2_4_3_2, "textLabel3_3_2_2_2_2_4_3_2" );
	textLabel3_3_2_2_2_2_4_3_2->setTextFormat( QLabel::PlainText );

	groupBox8_2_2_2_2_4_3_2Layout->addWidget( textLabel3_3_2_2_2_2_4_3_2, 0, 0 );

	spinBox1_2_2_2_2_2_4_3_2 = new DSpinBox( groupBox8_2_2_2_2_4_3_2, "spinBox1_2_2_2_2_2_4_3_2" );
	spinBox1_2_2_2_2_2_4_3_2->setAlignRight();
	spinBox1_2_2_2_2_2_4_3_2->setMaxValue( 100 );

	groupBox8_2_2_2_2_4_3_2Layout->addWidget( spinBox1_2_2_2_2_2_4_3_2, 0, 1 );

	textLabel1_4_2_2_2_2_4_3_2 = new QLabel( groupBox8_2_2_2_2_4_3_2, "textLabel1_4_2_2_2_2_4_3_2" );
	textLabel1_4_2_2_2_2_4_3_2->setTextFormat( QLabel::PlainText );

	groupBox8_2_2_2_2_4_3_2Layout->addWidget( textLabel1_4_2_2_2_2_4_3_2, 0, 2 );

	kColorButton2_4_2_2_2_2_4_3_2 = new KColorButton( groupBox8_2_2_2_2_4_3_2, "kColorButton2_4_2_2_2_2_4_3_2" );

	groupBox8_2_2_2_2_4_3_2Layout->addWidget( kColorButton2_4_2_2_2_2_4_3_2, 0, 3 );

	groupBox22_2_2_3_2Layout->addMultiCellWidget( groupBox8_2_2_2_2_4_3_2, 0, 0, 2, 6 );
	spacer3_2_2_3 = new QSpacerItem( 12, 25, QSizePolicy::Fixed, QSizePolicy::Minimum );
	groupBox22_2_2_3_2Layout->addItem( spacer3_2_2_3, 3, 6 );
	spacer6_2_2_3 = new QSpacerItem( 83, 25, QSizePolicy::Expanding, QSizePolicy::Minimum );
	groupBox22_2_2_3_2Layout->addItem( spacer6_2_2_3, 3, 3 );

	kColorButton27_2_2_2_3 = new KColorButton( groupBox22_2_2_3_2, "kColorButton27_2_2_2_3" );
	kColorButton27_2_2_2_3->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, kColorButton27_2_2_2_3->sizePolicy().hasHeightForWidth() ) );
	kColorButton27_2_2_2_3->setMinimumSize( QSize( 75, 0 ) );

	groupBox22_2_2_3_2Layout->addWidget( kColorButton27_2_2_2_3, 3, 5 );
	spacer2_2_2_3 = new QSpacerItem( 82, 25, QSizePolicy::Expanding, QSizePolicy::Minimum );
	groupBox22_2_2_3_2Layout->addItem( spacer2_2_2_3, 3, 2 );
	spacer46_2_2_3 = new QSpacerItem( 20, 7, QSizePolicy::Minimum, QSizePolicy::Fixed );
	groupBox22_2_2_3_2Layout->addItem( spacer46_2_2_3, 2, 5 );

	textLabel1_2_2_2_3 = new QLabel( groupBox22_2_2_3_2, "textLabel1_2_2_2_3" );
	textLabel1_2_2_2_3->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)5, 0, 0, textLabel1_2_2_2_3->sizePolicy().hasHeightForWidth() ) );
	textLabel1_2_2_2_3->setTextFormat( QLabel::PlainText );

	groupBox22_2_2_3_2Layout->addWidget( textLabel1_2_2_2_3, 3, 4 );
	spacer43_2_2_2_2 = new QSpacerItem( 5, 20, QSizePolicy::Fixed, QSizePolicy::Minimum );
	groupBox22_2_2_3_2Layout->addItem( spacer43_2_2_2_2, 1, 1 );

	groupBox10_2_2_2_2_4_3_2 = new QGroupBox( groupBox22_2_2_3_2, "groupBox10_2_2_2_2_4_3_2" );
	groupBox10_2_2_2_2_4_3_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)5, 0, 0, groupBox10_2_2_2_2_4_3_2->sizePolicy().hasHeightForWidth() ) );
	groupBox10_2_2_2_2_4_3_2->setMinimumSize( QSize( 170, 0 ) );
	groupBox10_2_2_2_2_4_3_2->setColumnLayout(0, Qt::Vertical );
	groupBox10_2_2_2_2_4_3_2->layout()->setSpacing( 6 );
	groupBox10_2_2_2_2_4_3_2->layout()->setMargin( 11 );
	groupBox10_2_2_2_2_4_3_2Layout = new QVBoxLayout( groupBox10_2_2_2_2_4_3_2->layout() );
	groupBox10_2_2_2_2_4_3_2Layout->setAlignment( Qt::AlignTop );

	radioButton2 = new PreviewRadioButton( groupBox10_2_2_2_2_4_3_2, "dominoPreviewWidget" );
	radioButton2->setStyle(previewStyle);
	groupBox10_2_2_2_2_4_3_2Layout->addWidget( radioButton2 );

	checkBox2 = new PreviewCheckBox( groupBox10_2_2_2_2_4_3_2, "dominoPreviewWidget" );
	checkBox2->setStyle(previewStyle);
	groupBox10_2_2_2_2_4_3_2Layout->addWidget( checkBox2 );

	groupBox22_2_2_3_2Layout->addMultiCellWidget( groupBox10_2_2_2_2_4_3_2, 0, 1, 0, 0 );

	groupBox9_2_2_2_2_4_3_2 = new QGroupBox( groupBox22_2_2_3_2, "groupBox9_2_2_2_2_4_3_2" );
	groupBox9_2_2_2_2_4_3_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, groupBox9_2_2_2_2_4_3_2->sizePolicy().hasHeightForWidth() ) );
	groupBox9_2_2_2_2_4_3_2->setCheckable( TRUE );
	connect(((QCheckBox*)groupBox9_2_2_2_2_4_3_2->child("qt_groupbox_checkbox", 0, false)), SIGNAL(clicked()), SLOT(checkItem2GradientChanged()));

	groupBox9_2_2_2_2_4_3_2->setChecked( FALSE );
	groupBox9_2_2_2_2_4_3_2->setColumnLayout(0, Qt::Vertical );
	groupBox9_2_2_2_2_4_3_2->layout()->setSpacing( 7 );
	groupBox9_2_2_2_2_4_3_2->layout()->setMargin( 4 );
	groupBox9_2_2_2_2_4_3_2Layout = new QGridLayout( groupBox9_2_2_2_2_4_3_2->layout() );
	groupBox9_2_2_2_2_4_3_2Layout->setAlignment( Qt::AlignTop );

	textLabel4_4_2_2_2_2_4_3_2 = new QLabel( groupBox9_2_2_2_2_4_3_2, "textLabel4_4_2_2_2_2_4_3_2" );
	textLabel4_4_2_2_2_2_4_3_2->setTextFormat( QLabel::PlainText );

	groupBox9_2_2_2_2_4_3_2Layout->addWidget( textLabel4_4_2_2_2_2_4_3_2, 1, 0 );

	spinBox2_3_2_2_2_2_4_3_2 = new DSpinBox( groupBox9_2_2_2_2_4_3_2, "spinBox2_3_2_2_2_2_4_3_2" );
	spinBox2_3_2_2_2_2_4_3_2->setAlignRight();
	spinBox2_3_2_2_2_2_4_3_2->setMaxValue( 100 );

	groupBox9_2_2_2_2_4_3_2Layout->addWidget( spinBox2_3_2_2_2_2_4_3_2, 1, 1 );

	textLabel3_4_2_2_2_2_4_3_2 = new QLabel( groupBox9_2_2_2_2_4_3_2, "textLabel3_4_2_2_2_2_4_3_2" );
	textLabel3_4_2_2_2_2_4_3_2->setTextFormat( QLabel::PlainText );

	groupBox9_2_2_2_2_4_3_2Layout->addWidget( textLabel3_4_2_2_2_2_4_3_2, 0, 0 );

	spinBox1_3_2_2_2_2_4_3_2 = new DSpinBox( groupBox9_2_2_2_2_4_3_2, "spinBox1_3_2_2_2_2_4_3_2" );
	spinBox1_3_2_2_2_2_4_3_2->setAlignRight();
	spinBox1_3_2_2_2_2_4_3_2->setMaxValue( 100 );

	groupBox9_2_2_2_2_4_3_2Layout->addWidget( spinBox1_3_2_2_2_2_4_3_2, 0, 1 );

	kColorButton1_3_2_2_2_2_4_3_2 = new KColorButton( groupBox9_2_2_2_2_4_3_2, "kColorButton1_3_2_2_2_2_4_3_2" );
	kColorButton1_3_2_2_2_2_4_3_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, kColorButton1_3_2_2_2_2_4_3_2->sizePolicy().hasHeightForWidth() ) );

	groupBox9_2_2_2_2_4_3_2Layout->addWidget( kColorButton1_3_2_2_2_2_4_3_2, 1, 3 );

	kColorButton2_5_2_2_2_2_4_3_2 = new KColorButton( groupBox9_2_2_2_2_4_3_2, "kColorButton2_5_2_2_2_2_4_3_2" );

	groupBox9_2_2_2_2_4_3_2Layout->addWidget( kColorButton2_5_2_2_2_2_4_3_2, 0, 3 );

	textLabel1_5_2_2_2_2_4_3_2 = new QLabel( groupBox9_2_2_2_2_4_3_2, "textLabel1_5_2_2_2_2_4_3_2" );
	textLabel1_5_2_2_2_2_4_3_2->setTextFormat( QLabel::PlainText );

	groupBox9_2_2_2_2_4_3_2Layout->addWidget( textLabel1_5_2_2_2_2_4_3_2, 0, 2 );

	textLabel2_3_2_2_2_2_4_3_2 = new QLabel( groupBox9_2_2_2_2_4_3_2, "textLabel2_3_2_2_2_2_4_3_2" );
	textLabel2_3_2_2_2_2_4_3_2->setTextFormat( QLabel::PlainText );

	groupBox9_2_2_2_2_4_3_2Layout->addWidget( textLabel2_3_2_2_2_2_4_3_2, 1, 2 );

	groupBox22_2_2_3_2Layout->addMultiCellWidget( groupBox9_2_2_2_2_4_3_2, 1, 1, 2, 6 );
	tab5Layout->addWidget( groupBox22_2_2_3_2 );

	layout22 = new QHBoxLayout( 0, 0, 6, "layout22"); 
	spacer7_2_3_3_2 = new QSpacerItem( 354, 21, QSizePolicy::Expanding, QSizePolicy::Minimum );
	layout22->addItem( spacer7_2_3_3_2 );

	textLabel7_4_2_3_3_2 = new QLabel( tab5, "textLabel7_4_2_3_3_2" );
	layout22->addWidget( textLabel7_4_2_3_3_2 );

	comboBox3_4_2_3_3_2 = new QComboBox( FALSE, tab5, "comboBox3_4_2_3_3_2" );
	layout22->addWidget( comboBox3_4_2_3_3_2 );

	pushButton4_4_2_3_3_2 = new QPushButton( tab5, "pushButton4_4_2_3_3_2" );
	connect(pushButton4_4_2_3_3_2, SIGNAL(clicked()), SLOT(copyColorsToCheckItem()));
	
	layout22->addWidget( pushButton4_4_2_3_3_2 );
	tab5Layout->addLayout( layout22 );
	tabWidget2->insertTab( tab5, QString::fromLatin1("") );
	
	
	
   /// resize( QSize(673, 465).expandedTo(minimumSizeHint()) );
  ///  clearWState( WState_Polished );

////////////////////////////////////////////////////////////////////////////
	// tab 3   load/save scheme
////////////////////////////////////////////////////////////////////////////


	QWidget *page3 = new QWidget(tabWidget);

	tab_lsLayout = new QVBoxLayout( page3, 11, 6, "tab_lsLayout");

	groupBox_ls = new QGroupBox( page3, "groupBox21" );
	groupBox_ls->setColumnLayout(0, Qt::Vertical );
	groupBox_ls->layout()->setSpacing( 6 );
	groupBox_ls->layout()->setMargin( 11 );
	groupBox_lsLayout = new QHBoxLayout( groupBox_ls->layout() );
	groupBox_lsLayout->setAlignment( Qt::AlignTop );

	listView_ls = new QListView( groupBox_ls, "listView_ls" );
	listView_ls->addColumn( tr( "Name" ) );
	listView_ls->header()->setMovingEnabled(false);
	listView_ls->setResizeMode(QListView::LastColumn);
	listView_ls->setShowSortIndicator(true);
	connect(listView_ls, SIGNAL(selectionChanged(QListViewItem*)), SLOT(slotDelPerm(QListViewItem*)));
	connect(listView_ls, SIGNAL(doubleClicked(QListViewItem*, const QPoint&, int)), SLOT(slotLoad()));

	schemeList = new SchemeList();
	getConfigSchemes();
	
	groupBox_lsLayout->addWidget( listView_ls );

	layout8 = new QVBoxLayout( 0, 0, 6, "layout8");
	loadButton = new QPushButton( groupBox_ls, "loadButton" );
	connect(loadButton, SIGNAL(clicked()), SLOT(slotLoad()));
	layout8->addWidget( loadButton );

	saveButton = new QPushButton( groupBox_ls, "saveButton" );
	connect(saveButton, SIGNAL(clicked()), SLOT(slotSave()));
	layout8->addWidget( saveButton );

	deleteButton = new QPushButton( groupBox_ls, "deleteButton" );
	connect(deleteButton, SIGNAL(clicked()), SLOT(slotDelete()));
	layout8->addWidget( deleteButton );
	
	importButton = new QPushButton( groupBox_ls, "importButton" );
	connect(importButton, SIGNAL(clicked()), SLOT(slotImport()));
	layout8->addWidget( importButton );
	
	spacer32 = new QSpacerItem( 31, 141, QSizePolicy::Minimum, QSizePolicy::Expanding );
	layout8->addItem( spacer32 );
	groupBox_lsLayout->addLayout( layout8 );
	

	tab_lsLayout->addWidget( groupBox_ls );
	currentConfigLabel = new QLabel(page3);
	tab_lsLayout->addWidget(currentConfigLabel);
	tabWidget->addTab(page3, "Load/Save");
	
// 	resize( QSize(673, 465).expandedTo(minimumSizeHint()) );
	languageChange();
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	loadConfig("domino"); // dominorc, the current one
	initialized = true;
	configLoaded = false;
	updateChanged(); // enable or disable widgets
	
	// scrollbar buttons or groove
	connect(comboBox7, SIGNAL( activated(const QString &)), SLOT(userLoadScrollBarConfig()));
	// btn
	connect(kColorButton2_4_2_2_2, SIGNAL(changed(const QColor&)),this,  SLOT(btnPv1(const QColor&)));
	connect(kColorButton1_2_2_2_2, SIGNAL(changed(const QColor&)),this,  SLOT(btnPv2(const QColor&)));
	connect(kColorButton2_5_2_2_2, SIGNAL(changed(const QColor&)),this,  SLOT(btnPv3(const QColor&)));
	connect(kColorButton1_3_2_2_2, SIGNAL(changed(const QColor&)),this,  SLOT(btnPv4(const QColor&)));
	connect(kColorButton27_2, SIGNAL(changed(const QColor&)),this,	SLOT(btnPv5(const QColor&)));
	connect(spinBox1_2_2_2_2, SIGNAL(valueChanged(int)), this, SLOT(btnPv6(int)));
	connect(spinBox2_2_2_2_2, SIGNAL(valueChanged(int)), this, SLOT(btnPv7(int)));
	connect(spinBox1_3_2_2_2, SIGNAL(valueChanged(int)), this, SLOT(btnPv8(int)));
	connect(spinBox2_3_2_2_2, SIGNAL(valueChanged(int)), this, SLOT(btnPv9(int)));
	// header
	connect(kColorButton2_4_2_2_2_2_4_3, SIGNAL(changed(const QColor&)),this,  SLOT(hPv1(const QColor&)));
	connect(kColorButton1_2_2_2_2_2_4_3, SIGNAL(changed(const QColor&)),this,  SLOT(hPv2(const QColor&)));
	connect(kColorButton2_5_2_2_2_2_4_3, SIGNAL(changed(const QColor&)),this,  SLOT(hPv3(const QColor&)));
	connect(kColorButton1_3_2_2_2_2_4_3, SIGNAL(changed(const QColor&)),this,  SLOT(hPv4(const QColor&)));
	connect(kColorButton27_2_2_2_2, SIGNAL(changed(const QColor&)),this,  SLOT(hPv5(const QColor&)));
	connect(spinBox1_2_2_2_2_2_4_3, SIGNAL(valueChanged(int)), this, SLOT(hPv6(int)));
	connect(spinBox2_2_2_2_2_2_4_3, SIGNAL(valueChanged(int)), this, SLOT(hPv7(int)));
	connect(spinBox1_3_2_2_2_2_4_3, SIGNAL(valueChanged(int)), this, SLOT(hPv8(int)));
	connect(spinBox2_3_2_2_2_2_4_3, SIGNAL(valueChanged(int)), this, SLOT(hPv9(int)));
	// checkBox
	connect(kColorButton2_4_2_2_2_2_4_3_2, SIGNAL(changed(const QColor&)),this,  SLOT(chbPv1(const QColor&)));
	connect(kColorButton1_2_2_2_2_2_4_3_2, SIGNAL(changed(const QColor&)),this,  SLOT(chbPv2(const QColor&)));
	connect(kColorButton2_5_2_2_2_2_4_3_2, SIGNAL(changed(const QColor&)),this,  SLOT(chbPv3(const QColor&)));
	connect(kColorButton1_3_2_2_2_2_4_3_2, SIGNAL(changed(const QColor&)),this,  SLOT(chbPv4(const QColor&)));
	connect(kColorButton27_2_2_2_3, SIGNAL(changed(const QColor&)),this,  SLOT(chbPv5(const QColor&)));
	connect(spinBox1_2_2_2_2_2_4_3_2, SIGNAL(valueChanged(int)), this, SLOT(chbPv6(int)));
	connect(spinBox2_2_2_2_2_2_4_3_2, SIGNAL(valueChanged(int)), this, SLOT(chbPv7(int)));
	connect(spinBox1_3_2_2_2_2_4_3_2, SIGNAL(valueChanged(int)), this, SLOT(chbPv8(int)));
	connect(spinBox2_3_2_2_2_2_4_3_2, SIGNAL(valueChanged(int)), this, SLOT(chbPv9(int)));

	// scrollbar
	connect(kColorButton2_4_2_2_2_2_4, SIGNAL(changed(const QColor&)),this,  SLOT(sbPv1(const QColor&)));
	connect(kColorButton1_2_2_2_2_2_4, SIGNAL(changed(const QColor&)),this,  SLOT(sbPv2(const QColor&)));
	connect(kColorButton2_5_2_2_2_2_4, SIGNAL(changed(const QColor&)),this,  SLOT(sbPv3(const QColor&)));
	connect(kColorButton1_3_2_2_2_2_4, SIGNAL(changed(const QColor&)),this,  SLOT(sbPv4(const QColor&)));
	connect(kColorButton27_2_2_2, SIGNAL(changed(const QColor&)),this,	SLOT(sbPv5(const QColor&)));
	connect(spinBox1_2_2_2_2_2_4, SIGNAL(valueChanged(int)), this, SLOT(sbPv6(int)));
	connect(spinBox2_2_2_2_2_2_4, SIGNAL(valueChanged(int)), this, SLOT(sbPv7(int)));
	connect(spinBox1_3_2_2_2_2_4, SIGNAL(valueChanged(int)), this, SLOT(sbPv8(int)));
	connect(spinBox2_3_2_2_2_2_4, SIGNAL(valueChanged(int)), this, SLOT(sbPv9(int)));
	// tab
	connect(kColorButton2_4_2_2_2_2, SIGNAL(changed(const QColor&)),this,  SLOT(tabPv1(const QColor&)));
	connect(kColorButton1_2_2_2_2_2, SIGNAL(changed(const QColor&)),this,  SLOT(tabPv2(const QColor&)));
	connect(kColorButton2_5_2_2_2_2, SIGNAL(changed(const QColor&)),this,  SLOT(tabPv3(const QColor&)));
	connect(kColorButton1_3_2_2_2_2, SIGNAL(changed(const QColor&)),this,  SLOT(tabPv4(const QColor&)));
	connect(kColorButton27_2_2, SIGNAL(changed(const QColor&)),this,	SLOT(tabPv5(const QColor&)));
	connect(spinBox1_2_2_2_2_2, SIGNAL(valueChanged(int)), this, SLOT(tabPv6(int)));
	connect(spinBox2_2_2_2_2_2, SIGNAL(valueChanged(int)), this, SLOT(tabPv7(int)));
	connect(spinBox1_3_2_2_2_2, SIGNAL(valueChanged(int)), this, SLOT(tabPv8(int)));
	connect(spinBox2_3_2_2_2_2, SIGNAL(valueChanged(int)), this, SLOT(tabPv9(int)));
	
	connect(tabPosCombo, SIGNAL( activated(int)), SLOT(userLoadTabConfig()));
	connect(tabStateCombo, SIGNAL( activated(int)), SLOT(userLoadTabConfig()));

	
	connect(customCheckMarkColor, SIGNAL(toggled(bool)), SLOT(updateChanged()));
	connect(customCheckMarkColor, SIGNAL(toggled(bool)), SLOT(updateCheckItemIndicators()));
	connect(checkMarkColor, SIGNAL(changed(const QColor&)), SLOT(updateCheckItemIndicators()));
	connect(customToolTipColor, SIGNAL(toggled(bool)), SLOT(updateChanged()));
	
	connect(customPopupMenuColor, SIGNAL(toggled(bool)), SLOT( updateChanged()));
	connect(customSelMenuItemColor, SIGNAL(toggled(bool)), SLOT( updateChanged()));
	connect(tintGroupBoxBackground, SIGNAL(toggled(bool)), SLOT( updateChanged()));
	
	connect(indicatorColor, SIGNAL(changed(const QColor&)), this, SLOT(updateFocusIndicatorSettings()));
	connect(indicatorBtnColor, SIGNAL(changed(const QColor&)), this, SLOT(updateFocusIndicatorSettings()));
	connect(indicatorOpacity, SIGNAL(valueChanged(int)), this, SLOT(updateFocusIndicatorSettings()));
	connect(indicatorBtnOpacity, SIGNAL(valueChanged(int)), this, SLOT(updateFocusIndicatorSettings()));
	
	connect(textEffectColor, SIGNAL(changed(const QColor&)), this, SLOT(updateTextEffectSettings()));
	connect(textEffectPos, SIGNAL(activated(int)), this, SLOT(updateTextEffectSettings()));
	connect(textEffectOpacity, SIGNAL(valueChanged(int)), this, SLOT(updateTextEffectSettings()));
	
	connect(textEffectButtonColor, SIGNAL(changed(const QColor&)), this, SLOT(updateTextEffectSettings()));
	connect(textEffectButtonPos, SIGNAL(activated(int)), this, SLOT(updateTextEffectSettings()));
	connect(textEffectButtonOpacity, SIGNAL(valueChanged(int)), this, SLOT(updateTextEffectSettings()));
	
	//languageChange();
	clearWState( WState_Polished );
	

}


// btn
void DominoStyleConfig::btnPv1(const QColor&col) {
	previewStyle->btnSurface.g1Color1 = col.name();
	pushButton16->repaint(false);
	comboBox17->repaint(false);
	spinBox44->spinWidget()->repaint(false);
}
void DominoStyleConfig::btnPv2(const QColor&col) {
	previewStyle->btnSurface.g1Color2 = col.name();
	pushButton16->repaint(false);
	comboBox17->repaint(false);
	spinBox44->spinWidget()->repaint(false);
}
void DominoStyleConfig::btnPv3(const QColor&col) {
	previewStyle->btnSurface.g2Color1 = col.name();
	pushButton16->repaint(false);
	comboBox17->repaint(false);
	spinBox44->spinWidget()->repaint(false);
}
void DominoStyleConfig::btnPv4(const QColor&col) {
	previewStyle->btnSurface.g2Color2 = col.name();
	pushButton16->repaint(false);
	comboBox17->repaint(false);
	spinBox44->spinWidget()->repaint(false);
}
void DominoStyleConfig::btnPv5(const QColor&col) {
	previewStyle->btnSurface.background = col.name();
	pushButton16->repaint(false);
	comboBox17->repaint(false);
	spinBox44->spinWidget()->repaint(false);
}
void DominoStyleConfig::btnPv6(int i) {
	previewStyle->btnSurface.g1Top = i;
	pushButton16->repaint(false);
	comboBox17->repaint(false);
	spinBox44->spinWidget()->repaint(false);
}
void DominoStyleConfig::btnPv7(int i) {
	previewStyle->btnSurface.g1Bottom = i;
	pushButton16->repaint(false);
	comboBox17->repaint(false);
	spinBox44->spinWidget()->repaint(false);
}
void DominoStyleConfig::btnPv8(int i) {
	previewStyle->btnSurface.g2Top = i;
	pushButton16->repaint(false);
	comboBox17->repaint(false);
	spinBox44->spinWidget()->repaint(false);
}
void DominoStyleConfig::btnPv9(int i) {
	previewStyle->btnSurface.g2Bottom= i;
	pushButton16->repaint(false);
	comboBox17->repaint(false);
	spinBox44->spinWidget()->repaint(false);
}

// header
void DominoStyleConfig::hPv1(const QColor&col) {
	header1->setBackgroundMode(Qt::PaletteBackground);
	previewStyle->headerSurface.g1Color1 = col.name();
}
void DominoStyleConfig::hPv2(const QColor&col) {
	header1->setBackgroundMode(Qt::PaletteBackground);
	previewStyle->headerSurface.g1Color2 = col.name();
}
void DominoStyleConfig::hPv3(const QColor&col) {
	header1->setBackgroundMode(Qt::PaletteBackground);
	previewStyle->headerSurface.g2Color1 = col.name();
}
void DominoStyleConfig::hPv4(const QColor&col) {
	header1->setBackgroundMode(Qt::PaletteBackground);
	previewStyle->headerSurface.g2Color2 = col.name();
}
void DominoStyleConfig::hPv5(const QColor&col) {
	header1->setBackgroundMode(Qt::PaletteBackground);
	previewStyle->headerSurface.background = col.name();
}
void DominoStyleConfig::hPv6(int i) {
	header1->setBackgroundMode(Qt::PaletteBackground);
	previewStyle->headerSurface.g1Top = i;
}
void DominoStyleConfig::hPv7(int i) {
	header1->setBackgroundMode(Qt::PaletteBackground);
	previewStyle->headerSurface.g1Bottom = i;
}
void DominoStyleConfig::hPv8(int i) {
	header1->setBackgroundMode(Qt::PaletteBackground);
	previewStyle->headerSurface.g2Top = i;
}
void DominoStyleConfig::hPv9(int i) {
	header1->setBackgroundMode(Qt::PaletteBackground);
	previewStyle->headerSurface.g2Bottom= i;
}

// checkBox
void DominoStyleConfig::chbPv1(const QColor&col) {
	previewStyle->checkItemSurface.g1Color1 = col.name();
	checkBox2->repaint(false);
	radioButton2->repaint(false);
}
void DominoStyleConfig::chbPv2(const QColor&col) {
	previewStyle->checkItemSurface.g1Color2 = col.name();
	checkBox2->repaint(false);
	radioButton2->repaint(false);
}
void DominoStyleConfig::chbPv3(const QColor&col) {
	previewStyle->checkItemSurface.g2Color1 = col.name();
	checkBox2->repaint(false);
	radioButton2->repaint(false);
}
void DominoStyleConfig::chbPv4(const QColor&col) {
	previewStyle->checkItemSurface.g2Color2 = col.name();
	checkBox2->repaint(false);
	radioButton2->repaint(false);
}
void DominoStyleConfig::chbPv5(const QColor&col) {
	previewStyle->checkItemSurface.background = col.name();
	checkBox2->repaint(false);
	radioButton2->repaint(false);
}
void DominoStyleConfig::chbPv6(int i) {
	previewStyle->checkItemSurface.g1Top = i;
	checkBox2->repaint(false);
	radioButton2->repaint(false);
}
void DominoStyleConfig::chbPv7(int i) {
	previewStyle->checkItemSurface.g1Bottom = i;
	checkBox2->repaint(false);
	radioButton2->repaint(false);
}
void DominoStyleConfig::chbPv8(int i) {
	previewStyle->checkItemSurface.g2Top = i;
	checkBox2->repaint(false);
	radioButton2->repaint(false);
}
void DominoStyleConfig::chbPv9(int i) {
	previewStyle->checkItemSurface.g2Bottom= i;
	checkBox2->repaint(false);
	radioButton2->repaint(false);
}

// scrollBar
void DominoStyleConfig::sbPv1(const QColor&col) {
	currentScrollBarSurface->g1Color1 = col.name();
	previewStyle->removeCachedSbPix(scrollBar5->palette().active().button());
	scrollBar5->repaint(false);
	scrollBar6->repaint(false);
}
void DominoStyleConfig::sbPv2(const QColor&col) {
	currentScrollBarSurface->g1Color2 = col.name();
	previewStyle->removeCachedSbPix(scrollBar5->palette().active().button());
	scrollBar5->repaint(false);
	scrollBar6->repaint(false);
}
void DominoStyleConfig::sbPv3(const QColor&col) {
	currentScrollBarSurface->g2Color1 = col.name();
	previewStyle->removeCachedSbPix(scrollBar5->palette().active().button());
	scrollBar5->repaint(false);
	scrollBar6->repaint(false);
}
void DominoStyleConfig::sbPv4(const QColor&col) {
	currentScrollBarSurface->g2Color2 = col.name();
	previewStyle->removeCachedSbPix(scrollBar5->palette().active().button());
	scrollBar5->repaint(false);
	scrollBar6->repaint(false);
}
void DominoStyleConfig::sbPv5(const QColor&col) {
	currentScrollBarSurface->background = col.name();
	previewStyle->removeCachedSbPix(scrollBar5->palette().active().button());
	scrollBar5->repaint(false);
	scrollBar6->repaint(false);
}
void DominoStyleConfig::sbPv6(int i) {
	currentScrollBarSurface->g1Top = i;
	previewStyle->removeCachedSbPix(scrollBar5->palette().active().button());
	scrollBar5->repaint(false);
	scrollBar6->repaint(false);
}
void DominoStyleConfig::sbPv7(int i) {
	currentScrollBarSurface->g1Bottom = i;
	previewStyle->removeCachedSbPix(scrollBar5->palette().active().button());
	scrollBar5->repaint(false);
	scrollBar6->repaint(false);
}
void DominoStyleConfig::sbPv8(int i) {
	currentScrollBarSurface->g2Top = i;
	previewStyle->removeCachedSbPix(scrollBar5->palette().active().button());
	scrollBar5->repaint(false);
	scrollBar6->repaint(false);
}
void DominoStyleConfig::sbPv9(int i) {
	currentScrollBarSurface->g2Bottom = i;
	previewStyle->removeCachedSbPix(scrollBar5->palette().active().button());
	scrollBar5->repaint(false);
	scrollBar6->repaint(false);
}

// tabs
void DominoStyleConfig::tabPv1(const QColor&col) {
	currentTabSurface->g1Color1 = col.name();
	tabBarTop->repaint(false);
	tabBarBottom->repaint(false);
}
void DominoStyleConfig::tabPv2(const QColor&col) {
	currentTabSurface->g1Color2 = col.name();
	tabBarTop->repaint(false);
	tabBarBottom->repaint(false);
}
void DominoStyleConfig::tabPv3(const QColor&col) {
	currentTabSurface->g2Color1 = col.name();
	tabBarTop->repaint(false);
	tabBarBottom->repaint(false);
}
void DominoStyleConfig::tabPv4(const QColor&col) {
	currentTabSurface->g2Color2 = col.name();
	tabBarTop->repaint(false);
	tabBarBottom->repaint(false);
}
void DominoStyleConfig::tabPv5(const QColor&col) {
	currentTabSurface->background = col.name();
	tabBarTop->repaint(false);
	tabBarBottom->repaint(false);
}
void DominoStyleConfig::tabPv6(int i) {
	currentTabSurface->g1Top = i;
	tabBarTop->repaint(false);
	tabBarBottom->repaint(false);
}
void DominoStyleConfig::tabPv7(int i) {
	currentTabSurface->g1Bottom = i;
	tabBarTop->repaint(false);
	tabBarBottom->repaint(false);
}
void DominoStyleConfig::tabPv8(int i) {
	currentTabSurface->g2Top = i;
	tabBarTop->repaint(false);
	tabBarBottom->repaint(false);
}
void DominoStyleConfig::tabPv9(int i) {
	currentTabSurface->g2Bottom = i;
	tabBarTop->repaint(false);
	tabBarBottom->repaint(false);
}

void DominoStyleConfig::loadConfig(const char* name) {

	QColor background = qApp->palette().active().background();
	QColor button = qApp->palette().active().button();
	QColor highlight = qApp->palette().active().highlight();
	QString cName = name;
	QSettings settings;
	
	settings.beginGroup("/" + cName + "/Settings");
	currentConfig = settings.readEntry("/name", "unnamed");
	origAnimProgressBar = settings.readBoolEntry("/animateProgressBar", true);
	animateProgressBar->setChecked(origAnimProgressBar);
	origCenterTabs = settings.readBoolEntry("/centerTabs", false);
	centerTabs->setChecked(origCenterTabs);
	origDrawTriangularExpander = settings.readBoolEntry("/drawTriangularExpander", true);
	drawTriangularExpander->setChecked(origDrawTriangularExpander);
	origHighlightToolBtnIcons = settings.readBoolEntry("/highlightToolBtnIcon", false);
	highlightToolBtnIcons->setChecked(origHighlightToolBtnIcons);
	origToolBtnAsBtn = settings.readBoolEntry("/drawToolButtonAsButton", true);
	toolBtnAsBtn->setChecked(origToolBtnAsBtn);

	origCustomCheckMarkColor = settings.readBoolEntry("/customCheckMarkColor", false);
	customCheckMarkColor->setChecked(origCustomCheckMarkColor);
	origCheckMarkColor = settings.readEntry("/checkMarkColor", "black");
	checkMarkColor->setColor(origCheckMarkColor);
	
	origTintGroupBoxBackground = settings.readBoolEntry("/tintGroupBoxBackground", true);
	tintGroupBoxBackground->setChecked(origTintGroupBoxBackground);
	origGroupBoxBrightness = settings.readNumEntry("/groupBoxBrightness", 10);
	grSlider->setValue(origGroupBoxBrightness);
	
	origCustomGroupBoxBackgroundColor = settings.readBoolEntry("/customGroupBoxBackgroundColor", false);
	customGroupBoxBackgroundColor->setChecked(origCustomGroupBoxBackgroundColor);
	origGroupBoxBackgroundColor = settings.readEntry("/groupBoxBackgroundColor", background.dark(110).name());
	groupBoxBackgroundColor->setColor(origGroupBoxBackgroundColor);

	origCustomToolTipColor = settings.readBoolEntry("/customToolTipColor", false);
	customToolTipColor->setChecked(origCustomToolTipColor);
	origToolTipColor = settings.readEntry("/toolTipColor", QColor(218,218,179).name());
	toolTipColor->setColor(origToolTipColor);
	
	origSmoothScrolling = settings.readBoolEntry("/smoothScrolling", true);
	smoothScrolling->setChecked(origSmoothScrolling);
	origButtonHeightAdjustment = settings.readNumEntry("/buttonHeightAdjustment", 0);
	buttonHeightAdjustment->setValue(origButtonHeightAdjustment);
	
	origRubberBandType = settings.readNumEntry("/rubberBandType", 3);
	rubberBandType = origRubberBandType;
	argbRubber->setChecked(rubberBandType == DominoStyle::ArgbRubber);
	lineRubber->setChecked(rubberBandType == DominoStyle::LineRubber);
	distRubber->setChecked(rubberBandType == DominoStyle::DistRubber);
	
	if(argbRubber->isChecked() && !hasCompositeExtension)
		distRubber->setChecked(true);
	
	origArgbRubberColor = settings.readEntry("/rubberBandColor", highlight.name());
	argbRubberColor->setColor(origArgbRubberColor);
	origArgbRubberOpacity = settings.readNumEntry("/rubberBandOpacity", 30);
	argbRubberOpacity->setValue(origArgbRubberOpacity);
	updateArgbRubberOptions(rubberBandType == DominoStyle::ArgbRubber);
	
	origCustomPopupMenuColor  = settings.readBoolEntry("/customPopupMenuColor", false);
	customPopupMenuColor->setChecked(origCustomPopupMenuColor);
	origPopupMenuColor = settings.readEntry("/popupMenuColor", background.name());
	popupMenuColor->setColor(origPopupMenuColor);
	
	origCustomSelMenuItemColor  = settings.readBoolEntry("/customSelMenuItemColor", false);
	customSelMenuItemColor->setChecked(origCustomSelMenuItemColor);
	origSelMenuItemColor = settings.readEntry("/selMenuItemColor", highlight.name());
	selMenuItemColor->setColor(origSelMenuItemColor);
	origDrawPopupMenuGradient = settings.readBoolEntry("/drawPopupMenuGradient", true);
	drawPopupMenuGradient->setChecked(origDrawPopupMenuGradient);
	origIndentPopupMenuItems = settings.readBoolEntry("/indentPopupMenuItems", true);
	indentPopupMenuItems->setChecked(origIndentPopupMenuItems);
	
	origIndicateFocus = settings.readBoolEntry("/indicateFocus", true);
	indicateFocus->setChecked(origIndicateFocus);
	origIndicatorBtnColor = settings.readEntry("/indicatorButtonColor", highlight.name());
	indicatorBtnColor->setColor(origIndicatorBtnColor);
	origIndicatorColor = settings.readEntry("/indicatorColor", highlight.name());
	indicatorColor->setColor(origIndicatorColor);
	origIndicatorOpacity = settings.readNumEntry("/indicatorColorOpacity", 60);
	indicatorOpacity->setValue(origIndicatorOpacity);
	origIndicatorBtnOpacity = settings.readNumEntry("/indicatorButtonColorOpacity", 60);
	indicatorBtnOpacity->setValue(origIndicatorBtnOpacity);
	
	origDrawUnderline = settings.readBoolEntry("/drawFocusUnderline", true);
	indicatorModeCombo->setCurrentItem(origDrawUnderline);
	indicatorModeChanged();
	updateFocusIndicatorSettings();
	
	origDrawTextEffect = settings.readBoolEntry("/drawTextEffect", true);
	drawTextEffect->setChecked(origDrawTextEffect);
	origTextEffectMode = settings.readNumEntry("/textEffectMode", 0);
	textEffectModeCombo->setCurrentItem(origTextEffectMode);
	origTextEffectColor = settings.readEntry("/textEffectColor", Qt::white.name());
	textEffectColor->setColor(origTextEffectColor);
	origTextEffectButtonColor = settings.readEntry("/textEffectButtonColor", Qt::white.name());
	textEffectButtonColor->setColor(origTextEffectButtonColor);
	
	origTextEffectOpacity = settings.readNumEntry("/textEffectOpacity", 60);
	textEffectOpacity->setValue(origTextEffectOpacity);
	origTextEffectButtonOpacity = settings.readNumEntry("/textEffectButtonOpacity", 60);
	textEffectButtonOpacity->setValue(origTextEffectButtonOpacity);
	
	origTextEffectPos = settings.readNumEntry("/textEffectPos", 5);
	textEffectPos->setCurrentItem(origTextEffectPos);
	origTextEffectButtonPos = settings.readNumEntry("/textEffectButtonPos", 5);
	textEffectButtonPos->setCurrentItem(origTextEffectButtonPos);
	textEffectModeChanged();
	updateTextEffectSettings();

	origDrawButtonSunkenShadow = settings.readBoolEntry("/drawButtonSunkenShadow", true);
	drawButtonSunkenShadow->setChecked(origDrawButtonSunkenShadow);
	
	origButtonContourType = settings.readNumEntry("/buttonContourType", 1);
	buttonContourType->setCurrentItem(origButtonContourType);
	origButtonContourColor = settings.readEntry("/buttonContourColor", background.dark(250).name());
	buttonContourColor->setColor(origButtonContourColor);
	origButtonDefaultButtonContourColor = settings.readEntry("/buttonDefaultContourColor", highlight.name());
	buttonDefaultButtonContourColor->setColor(origButtonDefaultButtonContourColor);
	origButtonMouseOverContourColor = settings.readEntry("/buttonMouseOverContourColor", background.dark(250).name());
	buttonMouseOverContourColor->setColor(origButtonMouseOverContourColor);
	origButtonPressedContourColor = settings.readEntry("/buttonPressedContourColor", background.dark(250).name());
	buttonPressedContourColor->setColor(origButtonPressedContourColor);
	
	// hidden
	konqTabBarContrast = settings.readNumEntry("/konqTabBarContrast", 15);
	
	// buttons / comboBoxes / spinBoxes
	
	previewStyle->btnSurface.numGradients = settings.readNumEntry("/btnSurface_numGradients", 2);
	switch (previewStyle->btnSurface.numGradients) {
		case 2:
			groupBox8_2_2_2->setChecked(true);
			groupBox9_2_2_2->setChecked(true);
			break;
		case 1:
			groupBox8_2_2_2->setChecked(true);
			groupBox9_2_2_2->setChecked(false);
			break;
		default:
			groupBox8_2_2_2->setChecked(false);
			groupBox9_2_2_2->setChecked(false);
			break;
	}
	kColorButton2_4_2_2_2->setColor(settings.readEntry("/btnSurface_g1Color1", button.name()));
	kColorButton1_2_2_2_2->setColor(settings.readEntry("/btnSurface_g1Color2", button.dark(120).name()));
	spinBox1_2_2_2_2->setValue(settings.readNumEntry("/btnSurface_g1Top", 0));
	spinBox2_2_2_2_2->setValue(settings.readNumEntry("/btnSurface_g1Bottom", 50));
	kColorButton2_5_2_2_2->setColor(settings.readEntry("/btnSurface_g2Color1", button.dark(120).name()));
	kColorButton1_3_2_2_2->setColor(settings.readEntry("/btnSurface_g2Color2", button.dark(110).name()));
	spinBox1_3_2_2_2->setValue(settings.readNumEntry("/btnSurface_g2Top", 50));
	spinBox2_3_2_2_2->setValue(settings.readNumEntry("/btnSurface_g2Bottom", 100));
	kColorButton27_2->setColor(settings.readEntry("/btnSurface_background", button.name()));

	
	previewStyle->btnSurface.g1Color1 = kColorButton2_4_2_2_2->color();
	previewStyle->btnSurface.g1Color2 = kColorButton1_2_2_2_2->color();
	previewStyle->btnSurface.g2Color1 = kColorButton2_5_2_2_2->color();
	previewStyle->btnSurface.g2Color2 = kColorButton1_3_2_2_2->color();
	previewStyle->btnSurface.g1Top = spinBox1_2_2_2_2->value();
	previewStyle->btnSurface.g1Bottom = spinBox2_2_2_2_2->value();
	previewStyle->btnSurface.g2Top = spinBox1_3_2_2_2->value();
	previewStyle->btnSurface.g2Bottom = spinBox2_3_2_2_2->value();
	previewStyle->btnSurface.background = kColorButton27_2->color();
	
	origBtnSurface = previewStyle->btnSurface;
	
	// tabs
	
	previewStyle->activeTabTopSurface.numGradients = settings.readNumEntry("/activeTabTopSurface_numGradients", 2);
	switch (previewStyle->activeTabTopSurface.numGradients) {
		case 2:
			groupBox8_2_2_2_2->setChecked(true);
			groupBox9_2_2_2_2->setChecked(true);
			break;
		case 1:
			groupBox8_2_2_2_2->setChecked(true);
			groupBox9_2_2_2_2->setChecked(false);
			break;
		default:
			groupBox8_2_2_2_2->setChecked(false);
			groupBox9_2_2_2_2->setChecked(false);
			break;
	}
	kColorButton2_4_2_2_2_2->setColor(settings.readEntry("/activeTabTopSurface_g1Color1", background.light(110).name()));
	kColorButton1_2_2_2_2_2->setColor(settings.readEntry("/activeTabTopSurface_g1Color2", background.name()));
	kColorButton2_5_2_2_2_2->setColor(settings.readEntry("/activeTabTopSurface_g2Color1", background.name()));
	kColorButton1_3_2_2_2_2->setColor(settings.readEntry("/activeTabTopSurface_g2Color2", background.name()));
	kColorButton27_2_2->setColor(settings.readEntry("/activeTabTopSurface_background", background.name()));
	spinBox1_2_2_2_2_2->setValue(settings.readNumEntry("/activeTabTopSurface_g1Top", 0));
	spinBox2_2_2_2_2_2->setValue(settings.readNumEntry("/activeTabTopSurface_g1Bottom", 50));
	spinBox1_3_2_2_2_2->setValue(settings.readNumEntry("/activeTabTopSurface_g2Top", 50));
	spinBox2_3_2_2_2_2->setValue(settings.readNumEntry("/activeTabTopSurface_g2Bottom", 100));
	
	previewStyle->activeTabTopSurface.g1Color1 = kColorButton2_4_2_2_2_2->color().name();
	previewStyle->activeTabTopSurface.g1Color2 = kColorButton1_2_2_2_2_2->color().name();
	previewStyle->activeTabTopSurface.g2Color1 = kColorButton2_5_2_2_2_2->color().name();
	previewStyle->activeTabTopSurface.g2Color2 = kColorButton1_3_2_2_2_2->color().name();
	previewStyle->activeTabTopSurface.background = kColorButton27_2_2->color().name();
	previewStyle->activeTabTopSurface.g1Top = spinBox1_2_2_2_2_2->value();
	previewStyle->activeTabTopSurface.g1Bottom = spinBox2_2_2_2_2_2->value();
	previewStyle->activeTabTopSurface.g2Top = spinBox1_3_2_2_2_2->value();
	previewStyle->activeTabTopSurface.g2Bottom = spinBox2_3_2_2_2_2->value();
	
	currentTabSurface = &previewStyle->activeTabTopSurface;
	
	origActiveTabTopSurface = previewStyle->activeTabTopSurface;
	
	previewStyle->tabTopSurface.numGradients = settings.readNumEntry("/tabTopSurface_numGradients", 2);
	previewStyle->tabTopSurface.g1Color1 = settings.readEntry("/tabTopSurface_g1Color1", background.light(110).name());
	previewStyle->tabTopSurface.g1Color2 = settings.readEntry("/tabTopSurface_g1Color2", background.dark(110).name());
	previewStyle->tabTopSurface.g2Color1 = settings.readEntry("/tabTopSurface_g2Color1", background.dark(110).name());
	previewStyle->tabTopSurface.g2Color2 = settings.readEntry("/tabTopSurface_g2Color2", background.dark(110).name());
	previewStyle->tabTopSurface.background = settings.readEntry("/tabTopSurface_background", background.dark(110).name());
	previewStyle->tabTopSurface.g1Top = settings.readNumEntry("/tabTopSurface_g1Top", 0);
	previewStyle->tabTopSurface.g1Bottom = settings.readNumEntry("/tabTopSurface_g1Bottom", 50);
	previewStyle->tabTopSurface.g2Top = settings.readNumEntry("/tabTopSurface_g2Top", 50);
	previewStyle->tabTopSurface.g2Bottom = settings.readNumEntry("/tabTopSurface_g2Bottom", 100);
	
	origTabTopSurface = previewStyle->tabTopSurface;
	
	previewStyle->activeTabBottomSurface.numGradients = settings.readNumEntry("/activeTabBottomSurface_numGradients", 2);
	previewStyle->activeTabBottomSurface.g1Color1 = settings.readEntry("/activeTabBottomSurface_g1Color1", background.name());
	previewStyle->activeTabBottomSurface.g1Color2 = settings.readEntry("/activeTabBottomSurface_g1Color2", background.name());
	previewStyle->activeTabBottomSurface.g2Color1 = settings.readEntry("/activeTabBottomSurface_g2Color1", background.name());
	previewStyle->activeTabBottomSurface.g2Color2 = settings.readEntry("/activeTabBottomSurface_g2Color2", background.dark(120).name());
	previewStyle->activeTabBottomSurface.background = settings.readEntry("/activeTabBottomSurface_background", background.name());
	previewStyle->activeTabBottomSurface.g1Top = settings.readNumEntry("/activeTabBottomSurface_g1Top", 0);
	previewStyle->activeTabBottomSurface.g1Bottom = settings.readNumEntry("/activeTabBottomSurface_g1Bottom", 50);
	previewStyle->activeTabBottomSurface.g2Top = settings.readNumEntry("/activeTabBottomSurface_g2Top", 50);
	previewStyle->activeTabBottomSurface.g2Bottom = settings.readNumEntry("/activeTabBottomSurface_g2Bottom", 100);
	
	origActiveTabBottomSurface = previewStyle->activeTabBottomSurface;
	
	previewStyle->tabBottomSurface.numGradients = settings.readNumEntry("/tabBottomSurface_numGradients", 2);
	previewStyle->tabBottomSurface.g1Color1 = settings.readEntry("/tabBottomSurface_g1Color1", background.dark(110).name());
	previewStyle->tabBottomSurface.g1Color2 = settings.readEntry("/tabBottomSurface_g1Color2", background.dark(110).name());
	previewStyle->tabBottomSurface.g2Color1 = settings.readEntry("/tabBottomSurface_g2Color1", background.dark(110).name());
	previewStyle->tabBottomSurface.g2Color2 = settings.readEntry("/tabBottomSurface_g2Color2", background.dark(120).name());
	previewStyle->tabBottomSurface.background = settings.readEntry("/tabBottomSurface_background", background.dark(110).name());
	previewStyle->tabBottomSurface.g1Top = settings.readNumEntry("/tabBottomSurface_g1Top", 0);
	previewStyle->tabBottomSurface.g1Bottom = settings.readNumEntry("/tabBottomSurface_g1Bottom", 50);
	previewStyle->tabBottomSurface.g2Top = settings.readNumEntry("/tabBottomSurface_g2Top", 50);
	previewStyle->tabBottomSurface.g2Bottom = settings.readNumEntry("/tabBottomSurface_g2Bottom", 100);
	
	origTabBottomSurface = previewStyle->tabBottomSurface;
	
	
	// headers

	previewStyle->headerSurface.numGradients = settings.readNumEntry("/headerSurface_numGradients", 2);
	switch (previewStyle->headerSurface.numGradients) {
		case 2:
			groupBox8_2_2_2_2_4_3->setChecked(true);
			groupBox9_2_2_2_2_4_3->setChecked(true);
			break;
		case 1:
			groupBox8_2_2_2_2_4_3->setChecked(true);
			groupBox9_2_2_2_2_4_3->setChecked(false);
			break;
		default:
			groupBox8_2_2_2_2_4_3->setChecked(false);
			groupBox9_2_2_2_2_4_3->setChecked(false);
			break;
	}
	kColorButton2_4_2_2_2_2_4_3->setColor(settings.readEntry("/headerSurface_g1Color1", background.name()));
	kColorButton1_2_2_2_2_2_4_3->setColor(settings.readEntry("/headerSurface_g1Color2", background.dark(120).name()));
	spinBox1_2_2_2_2_2_4_3->setValue(settings.readNumEntry("/headerSurface_g1Top", 0));
	spinBox2_2_2_2_2_2_4_3->setValue(settings.readNumEntry("/headerSurface_g1Bottom", 50));
	kColorButton2_5_2_2_2_2_4_3->setColor(settings.readEntry("/headerSurface_g2Color1", background.dark(120).name()));
	kColorButton1_3_2_2_2_2_4_3->setColor(settings.readEntry("/headerSurface_g2Color2", background.dark(110).name()));
	spinBox1_3_2_2_2_2_4_3->setValue(settings.readNumEntry("/headerSurface_g2Top", 50));
	spinBox2_3_2_2_2_2_4_3->setValue(settings.readNumEntry("/headerSurface_g2Bottom", 100));
	kColorButton27_2_2_2_2->setColor(settings.readEntry("/headerSurface_background", background.name()));

	previewStyle->headerSurface.g1Color1 = kColorButton2_4_2_2_2_2_4_3->color();
	previewStyle->headerSurface.g1Color2 = kColorButton1_2_2_2_2_2_4_3->color();
	previewStyle->headerSurface.g2Color1 = kColorButton2_5_2_2_2_2_4_3->color();
	previewStyle->headerSurface.g2Color2 = kColorButton1_3_2_2_2_2_4_3->color();
	previewStyle->headerSurface.g1Top = spinBox1_2_2_2_2_2_4_3->value();
	previewStyle->headerSurface.g1Bottom = spinBox2_2_2_2_2_2_4_3->value();
	previewStyle->headerSurface.g2Top = spinBox1_3_2_2_2_2_4_3->value();
	previewStyle->headerSurface.g2Bottom = spinBox2_3_2_2_2_2_4_3->value();
	previewStyle->headerSurface.background = kColorButton27_2_2_2_2->color();

	origHeaderSurface = previewStyle->headerSurface;
	
	
	// radio / checkboxes
	
	previewStyle->checkItemSurface.numGradients = settings.readNumEntry("/checkItemSurface_numGradients", 2);
	switch (previewStyle->checkItemSurface.numGradients) {
		case 2:
			groupBox8_2_2_2_2_4_3_2->setChecked(true);
			groupBox9_2_2_2_2_4_3_2->setChecked(true);
			break;
		case 1:
			groupBox8_2_2_2_2_4_3_2->setChecked(true);
			groupBox9_2_2_2_2_4_3_2->setChecked(false);
			break;
		default:
			groupBox8_2_2_2_2_4_3_2->setChecked(false);
			groupBox9_2_2_2_2_4_3_2->setChecked(false);
			break;
	}
	kColorButton2_4_2_2_2_2_4_3_2->setColor(settings.readEntry("/checkItemSurface_g1Color1", button.name()));
	kColorButton1_2_2_2_2_2_4_3_2->setColor(settings.readEntry("/checkItemSurface_g1Color2", button.dark(120).name()));
	spinBox1_2_2_2_2_2_4_3_2->setValue(settings.readNumEntry("/checkItemSurface_g1Top", 0));
	spinBox2_2_2_2_2_2_4_3_2->setValue(settings.readNumEntry("/checkItemSurface_g1Bottom", 50));
	kColorButton2_5_2_2_2_2_4_3_2->setColor(settings.readEntry("/checkItemSurface_g2Color1", button.dark(120).name()));
	kColorButton1_3_2_2_2_2_4_3_2->setColor(settings.readEntry("/checkItemSurface_g2Color2", button.dark(110).name()));
	spinBox1_3_2_2_2_2_4_3_2->setValue(settings.readNumEntry("/checkItemSurface_g2Top", 50));
	spinBox2_3_2_2_2_2_4_3_2->setValue(settings.readNumEntry("/checkItemSurface_g2Bottom", 100));
	kColorButton27_2_2_2_3->setColor(settings.readEntry("/checkItemSurface_background", button.name()));

	previewStyle->checkItemSurface.g1Color1 = kColorButton2_4_2_2_2_2_4_3_2->color();
	previewStyle->checkItemSurface.g1Color2 = kColorButton1_2_2_2_2_2_4_3_2->color();
	previewStyle->checkItemSurface.g2Color1 = kColorButton2_5_2_2_2_2_4_3_2->color();
	previewStyle->checkItemSurface.g2Color2 = kColorButton1_3_2_2_2_2_4_3_2->color();
	previewStyle->checkItemSurface.g1Top = spinBox1_2_2_2_2_2_4_3_2->value();
	previewStyle->checkItemSurface.g1Bottom = spinBox2_2_2_2_2_2_4_3_2->value();
	previewStyle->checkItemSurface.g2Top = spinBox1_3_2_2_2_2_4_3_2->value();
	previewStyle->checkItemSurface.g2Bottom = spinBox2_3_2_2_2_2_4_3_2->value();
	previewStyle->checkItemSurface.background = kColorButton27_2_2_2_3->color();

	origCheckItemSurface = previewStyle->checkItemSurface;
	
	// schrollBarButtons
	previewStyle->scrollBarSurface.numGradients = settings.readNumEntry("/scrollBarSurface_numGradients", 2);
	previewStyle->scrollBarSurface.g1Color1 = settings.readEntry("/scrollBarSurface_g1Color1", button.name());
	previewStyle->scrollBarSurface.g1Color2 = settings.readEntry("/scrollBarSurface_g1Color2", button.dark(120).name());
	previewStyle->scrollBarSurface.g2Color1 = settings.readEntry("/scrollBarSurface_g2Color1", button.dark(120).name());
	previewStyle->scrollBarSurface.g2Color2 = settings.readEntry("/scrollBarSurface_g2Color2", button.dark(110).name());
	previewStyle->scrollBarSurface.g1Top = settings.readNumEntry("/scrollBarSurface_g1Top", 0);
	previewStyle->scrollBarSurface.g1Bottom = settings.readNumEntry("/scrollBarSurface_g1Bottom", 50);
	previewStyle->scrollBarSurface.g2Top = settings.readNumEntry("/scrollBarSurface_g2Top", 50);
	previewStyle->scrollBarSurface.g2Bottom = settings.readNumEntry("/scrollBarSurface_g2Bottom", 100);
	previewStyle->scrollBarSurface.background = settings.readEntry("/scrollBarSurface_background", button.name());
	
	origScrollBarSurface = previewStyle->scrollBarSurface;
	
	// schrollBarGroove
	previewStyle->scrollBarGrooveSurface.numGradients = settings.readNumEntry("/scrollBarGrooveSurface_numGradients", 0);
	previewStyle->scrollBarGrooveSurface.g1Color1 = settings.readEntry("/scrollBarGrooveSurface_g1Color1", background.name());
	previewStyle->scrollBarGrooveSurface.g1Color2 = settings.readEntry("/scrollBarGrooveSurface_g1Color2", background.dark(120).name());
	previewStyle->scrollBarGrooveSurface.g2Color1 = settings.readEntry("/scrollBarGrooveSurface_g2Color1", background.dark(120).name());
	previewStyle->scrollBarGrooveSurface.g2Color2 = settings.readEntry("/scrollBarGrooveSurface_g2Color2", background.dark(110).name());
	previewStyle->scrollBarGrooveSurface.g1Top = settings.readNumEntry("/scrollBarGrooveSurface_g1Top", 0);
	previewStyle->scrollBarGrooveSurface.g1Bottom = settings.readNumEntry("/scrollBarGrooveSurface_g1Bottom", 50);
	previewStyle->scrollBarGrooveSurface.g2Top = settings.readNumEntry("/scrollBarGrooveSurface_g2Top", 50);
	previewStyle->scrollBarGrooveSurface.g2Bottom = settings.readNumEntry("/scrollBarGrooveSurface_g2Bottom", 100);
	previewStyle->scrollBarGrooveSurface.background = settings.readEntry("/scrollBarGrooveSurface_background", background.dark(150).name());
	
	origScrollBarGrooveSurface = previewStyle->scrollBarGrooveSurface;
	
	comboBox7->setCurrentItem(0);
	
	switch(previewStyle->scrollBarSurface.numGradients) {
		case 2:
			groupBox8_2_2_2_2_4->setChecked(true);
			groupBox9_2_2_2_2_4->setChecked(true);
			break;
		case 1:
			groupBox8_2_2_2_2_4->setChecked(true);
			groupBox9_2_2_2_2_4->setChecked(false);
			break;
		default:
			groupBox8_2_2_2_2_4->setChecked(false);
			groupBox9_2_2_2_2_4->setChecked(false);
			break;
	}
	kColorButton2_4_2_2_2_2_4->setColor(previewStyle->scrollBarSurface.g1Color1);
	kColorButton1_2_2_2_2_2_4->setColor(previewStyle->scrollBarSurface.g1Color2);
	spinBox1_2_2_2_2_2_4->setValue(previewStyle->scrollBarSurface.g1Top);
	spinBox2_2_2_2_2_2_4->setValue(previewStyle->scrollBarSurface.g1Bottom);
	kColorButton2_5_2_2_2_2_4->setColor(previewStyle->scrollBarSurface.g2Color1);
	kColorButton1_3_2_2_2_2_4->setColor(previewStyle->scrollBarSurface.g2Color2);
	spinBox1_3_2_2_2_2_4->setValue(previewStyle->scrollBarSurface.g2Top);
	spinBox2_3_2_2_2_2_4->setValue(previewStyle->scrollBarSurface.g2Bottom);
	kColorButton27_2_2_2->setColor(previewStyle->scrollBarSurface.background);
	
	currentScrollBarSurface = &previewStyle->scrollBarSurface;
	
	settings.endGroup();
	
	currentConfigLabel->setText(i18n("Currently loaded config: ")+currentConfig);
	
}

void DominoStyleConfig::userLoadScrollBarConfig() {

	scrollBar5->setUpdatesEnabled(false);
	scrollBar6->setUpdatesEnabled(false);
	
	// index: groove = 1, buttons/slider = 0
	if(comboBox7->currentItem())
		currentScrollBarSurface = &previewStyle->scrollBarGrooveSurface;
	else
		currentScrollBarSurface = &previewStyle->scrollBarSurface;

	kColorButton2_4_2_2_2_2_4->setColor(currentScrollBarSurface->g1Color1);
	kColorButton1_2_2_2_2_2_4->setColor(currentScrollBarSurface->g1Color2);
	spinBox1_2_2_2_2_2_4->setValue(currentScrollBarSurface->g1Top);
	spinBox2_2_2_2_2_2_4->setValue(currentScrollBarSurface->g1Bottom);
	kColorButton2_5_2_2_2_2_4->setColor(currentScrollBarSurface->g2Color1);
	kColorButton1_3_2_2_2_2_4->setColor(currentScrollBarSurface->g2Color2);
	spinBox1_3_2_2_2_2_4->setValue(currentScrollBarSurface->g2Top);
	spinBox2_3_2_2_2_2_4->setValue(currentScrollBarSurface->g2Bottom);
	kColorButton27_2_2_2->setColor(currentScrollBarSurface->background);
	
	
	switch(currentScrollBarSurface->numGradients) {
		case 2:
			groupBox8_2_2_2_2_4->setChecked(true);
			groupBox9_2_2_2_2_4->setChecked(true);
			break;
		case 1:
			groupBox8_2_2_2_2_4->setChecked(true);
			groupBox9_2_2_2_2_4->setChecked(false);
			break;
		default:
			groupBox8_2_2_2_2_4->setChecked(false);
			groupBox9_2_2_2_2_4->setChecked(false);
			break;
	}
	scrollBar5->setUpdatesEnabled(true);
	scrollBar6->setUpdatesEnabled(true);
}

void DominoStyleConfig::userLoadTabConfig() {
	
	((DominoQTabWidget*)prevTopTabWidget)->tabBar()->setUpdatesEnabled(false);
	((DominoQTabWidget*)prevBottomTabWidget)->tabBar()->setUpdatesEnabled(false);
	
	bool topTabWidgetIsActive = false;
	// index: bottom = 1, top = 0
	if(tabPosCombo->currentItem())
		// index: inactive = 1, active = 0
		if(tabStateCombo->currentItem())
			currentTabSurface = &previewStyle->tabBottomSurface;
		else
			currentTabSurface = &previewStyle->activeTabBottomSurface;
	else {
		topTabWidgetIsActive = true;
		if(tabStateCombo->currentItem())
			currentTabSurface = &previewStyle->tabTopSurface;
		else
			currentTabSurface = &previewStyle->activeTabTopSurface;
	}
	
	// update arrow indicators
	prevBottomPage1->update();
	prevBottomPage2->update();
	prevBottomPage3->update();
	prevTopPage1->update();
	prevTopPage2->update();
	prevTopPage3->update();
	
	
	kColorButton2_4_2_2_2_2->setColor(currentTabSurface->g1Color1);
	kColorButton1_2_2_2_2_2->setColor(currentTabSurface->g1Color2);
	kColorButton2_5_2_2_2_2->setColor(currentTabSurface->g2Color1);
	kColorButton1_3_2_2_2_2->setColor(currentTabSurface->g2Color2);
	kColorButton27_2_2->setColor(currentTabSurface->background);
	spinBox1_2_2_2_2_2->setValue(currentTabSurface->g1Top);
	spinBox2_2_2_2_2_2->setValue(currentTabSurface->g1Bottom);
	spinBox1_3_2_2_2_2->setValue(currentTabSurface->g2Top);
	spinBox2_3_2_2_2_2->setValue(currentTabSurface->g2Bottom);
	
	switch (currentTabSurface->numGradients) {
		case 2:
			groupBox8_2_2_2_2->setChecked(true);
			groupBox9_2_2_2_2->setChecked(true);
			break;
		case 1:
			groupBox8_2_2_2_2->setChecked(true);
			groupBox9_2_2_2_2->setChecked(false);
			break;
		default:
			groupBox8_2_2_2_2->setChecked(false);
			groupBox9_2_2_2_2->setChecked(false);
			break;
	}
	((DominoQTabWidget*)prevTopTabWidget)->tabBar()->setUpdatesEnabled(true);
	((DominoQTabWidget*)prevBottomTabWidget)->tabBar()->setUpdatesEnabled(true);
}

void DominoStyleConfig::saveConfig(const char* fileName, const char* schemeName ) {
	
	QString fName;
	QSettings s;
	int c;

	if(!strcmp(fileName, "domino"))
		fName = fileName;
	else
		fName = QString("domino_").append(fileName);
	
	s.beginGroup("/" + fName + "/Settings");
	
	s.writeEntry("/name", schemeName);
	s.writeEntry("/animateProgressBar", animateProgressBar->isChecked());
	s.writeEntry("/centerTabs", centerTabs->isChecked());
	s.writeEntry("/drawTriangularExpander", drawTriangularExpander->isChecked());
	s.writeEntry("/drawToolButtonAsButton", toolBtnAsBtn->isChecked());
	s.writeEntry("/highlightToolBtnIcon", highlightToolBtnIcons->isChecked());
	s.writeEntry("/customCheckMarkColor", customCheckMarkColor->isChecked());
	s.writeEntry("/checkMarkColor", checkMarkColor->color().name());
	s.writeEntry("/tintGroupBoxBackground", tintGroupBoxBackground->isChecked());
	s.writeEntry("/groupBoxBrightness", grSlider->value());
	s.writeEntry("/customGroupBoxBackgroundColor", customGroupBoxBackgroundColor->isChecked());
	s.writeEntry("/groupBoxBackgroundColor", groupBoxBackgroundColor->color().name());
	s.writeEntry("/customToolTipColor", customToolTipColor->isChecked());
	s.writeEntry("/toolTipColor", toolTipColor->color().name());
	s.writeEntry("/smoothScrolling", smoothScrolling->isChecked());
	s.writeEntry("/buttonHeightAdjustment", buttonHeightAdjustment->value());
	s.writeEntry("/rubberBandType", rubberBandType);
	s.writeEntry("/rubberBandColor", argbRubberColor->color().name());
	s.writeEntry("/rubberBandOpacity", argbRubberOpacity->value());
	
	s.writeEntry("/customPopupMenuColor", customPopupMenuColor->isChecked());
	s.writeEntry("/popupMenuColor", popupMenuColor->color().name());
	s.writeEntry("/customSelMenuItemColor", customSelMenuItemColor->isChecked());
	s.writeEntry("/selMenuItemColor", selMenuItemColor->color().name());
	s.writeEntry("/drawPopupMenuGradient", drawPopupMenuGradient->isChecked());
	s.writeEntry("/indentPopupMenuItems", indentPopupMenuItems->isChecked());
	
	s.writeEntry("/indicateFocus", indicateFocus->isChecked());
	s.writeEntry("/drawFocusUnderline", indicatorModeCombo->currentItem() == 1);
	s.writeEntry("/indicatorButtonColor", indicatorBtnColor->color().name());
	s.writeEntry("/indicatorColor", indicatorColor->color().name());
	s.writeEntry("/indicatorColorOpacity", indicatorOpacity->value());
	s.writeEntry("/indicatorButtonColorOpacity", indicatorBtnOpacity->value());
	
	s.writeEntry("/drawTextEffect", drawTextEffect->isChecked());
	s.writeEntry("/textEffectMode", textEffectModeCombo->currentItem());
	s.writeEntry("/textEffectColor", textEffectColor->color().name());
	s.writeEntry("/textEffectButtonColor", textEffectButtonColor->color().name());
	s.writeEntry("/textEffectOpacity", textEffectOpacity->value());
	s.writeEntry("/textEffectButtonOpacity", textEffectButtonOpacity->value());
	s.writeEntry("/textEffectPos", textEffectPos->currentItem());
	s.writeEntry("/textEffectButtonPos", textEffectButtonPos->currentItem());
	
	s.writeEntry("/drawButtonSunkenShadow", drawButtonSunkenShadow->isChecked());
	
	s.writeEntry("/buttonContourType", buttonContourType->currentItem());
	s.writeEntry("/buttonContourColor", buttonContourColor->color().name());
	s.writeEntry("/buttonDefaultContourColor", buttonDefaultButtonContourColor->color().name());
	s.writeEntry("/buttonMouseOverContourColor", buttonMouseOverContourColor->color().name());
	s.writeEntry("/buttonPressedContourColor", buttonPressedContourColor->color().name());
	
	// hidden
	s.writeEntry("/konqTabBarContrast", konqTabBarContrast);
	
	//////


	// buttons
	c = groupBox9_2_2_2->isChecked() ? 2 : groupBox8_2_2_2->isChecked() ? 1 : 0;
	
	s.writeEntry("/btnSurface_numGradients", c);
	s.writeEntry("/btnSurface_g1Color1", kColorButton2_4_2_2_2->color().name());
	s.writeEntry("/btnSurface_g1Color2", kColorButton1_2_2_2_2->color().name());
	s.writeEntry("/btnSurface_g2Color1", kColorButton2_5_2_2_2->color().name());
	s.writeEntry("/btnSurface_g2Color2", kColorButton1_3_2_2_2->color().name());
	s.writeEntry("/btnSurface_g1Top", spinBox1_2_2_2_2->value());
	s.writeEntry("/btnSurface_g1Bottom", spinBox2_2_2_2_2->value());
	s.writeEntry("/btnSurface_g2Top", spinBox1_3_2_2_2->value());
	s.writeEntry("/btnSurface_g2Bottom", spinBox2_3_2_2_2->value());
	s.writeEntry("/btnSurface_background", kColorButton27_2->color().name());
	
	// headers
	c = groupBox9_2_2_2_2_4_3->isChecked() ? 2 : groupBox8_2_2_2_2_4_3->isChecked() ? 1 : 0;
	
	s.writeEntry("/headerSurface_numGradients", c);
	s.writeEntry("/headerSurface_g1Color1", kColorButton2_4_2_2_2_2_4_3->color().name());
	s.writeEntry("/headerSurface_g1Color2", kColorButton1_2_2_2_2_2_4_3->color().name());
	s.writeEntry("/headerSurface_g2Color1", kColorButton2_5_2_2_2_2_4_3->color().name());
	s.writeEntry("/headerSurface_g2Color2", kColorButton1_3_2_2_2_2_4_3->color().name());
	s.writeEntry("/headerSurface_g1Top", spinBox1_2_2_2_2_2_4_3->value());
	s.writeEntry("/headerSurface_g1Bottom", spinBox2_2_2_2_2_2_4_3->value());
	s.writeEntry("/headerSurface_g2Top", spinBox1_3_2_2_2_2_4_3->value());
	s.writeEntry("/headerSurface_g2Bottom", spinBox2_3_2_2_2_2_4_3->value());
	s.writeEntry("/headerSurface_background", kColorButton27_2_2_2_2->color().name());
	
	
	// radio/checkboxes
	c = groupBox9_2_2_2_2_4_3_2->isChecked() ? 2 : groupBox8_2_2_2_2_4_3_2->isChecked() ? 1 : 0;
	
	s.writeEntry("/checkItemSurface_numGradients", c);
	s.writeEntry("/checkItemSurface_g1Color1", kColorButton2_4_2_2_2_2_4_3_2->color().name());
	s.writeEntry("/checkItemSurface_g1Color2", kColorButton1_2_2_2_2_2_4_3_2->color().name());
	s.writeEntry("/checkItemSurface_g2Color1", kColorButton2_5_2_2_2_2_4_3_2->color().name());
	s.writeEntry("/checkItemSurface_g2Color2", kColorButton1_3_2_2_2_2_4_3_2->color().name());
	s.writeEntry("/checkItemSurface_g1Top", spinBox1_2_2_2_2_2_4_3_2->value());
	s.writeEntry("/checkItemSurface_g1Bottom", spinBox2_2_2_2_2_2_4_3_2->value());
	s.writeEntry("/checkItemSurface_g2Top", spinBox1_3_2_2_2_2_4_3_2->value());
	s.writeEntry("/checkItemSurface_g2Bottom", spinBox2_3_2_2_2_2_4_3_2->value());
	s.writeEntry("/checkItemSurface_background", kColorButton27_2_2_2_3->color().name());
	
	
	// scrollbars

	// buttons

	s.writeEntry("/scrollBarSurface_numGradients", previewStyle->scrollBarSurface.numGradients);
	s.writeEntry("/scrollBarSurface_g1Color1", previewStyle->scrollBarSurface.g1Color1.name());
	s.writeEntry("/scrollBarSurface_g1Color2", previewStyle->scrollBarSurface.g1Color2.name());
	s.writeEntry("/scrollBarSurface_g2Color1", previewStyle->scrollBarSurface.g2Color1.name());
	s.writeEntry("/scrollBarSurface_g2Color2", previewStyle->scrollBarSurface.g2Color2.name());
	s.writeEntry("/scrollBarSurface_g1Top", previewStyle->scrollBarSurface.g1Top);
	s.writeEntry("/scrollBarSurface_g1Bottom", previewStyle->scrollBarSurface.g1Bottom);
	s.writeEntry("/scrollBarSurface_g2Top", previewStyle->scrollBarSurface.g2Top);
	s.writeEntry("/scrollBarSurface_g2Bottom", previewStyle->scrollBarSurface.g2Bottom);
	s.writeEntry("/scrollBarSurface_background", previewStyle->scrollBarSurface.background.name());
	
	// groove
	s.writeEntry("/scrollBarGrooveSurface_numGradients", previewStyle->scrollBarGrooveSurface.numGradients);
	s.writeEntry("/scrollBarGrooveSurface_g1Color1", previewStyle->scrollBarGrooveSurface.g1Color1.name());
	s.writeEntry("/scrollBarGrooveSurface_g1Color2", previewStyle->scrollBarGrooveSurface.g1Color2.name());
	s.writeEntry("/scrollBarGrooveSurface_g2Color1", previewStyle->scrollBarGrooveSurface.g2Color1.name());
	s.writeEntry("/scrollBarGrooveSurface_g2Color2", previewStyle->scrollBarGrooveSurface.g2Color2.name());
	s.writeEntry("/scrollBarGrooveSurface_g1Top", previewStyle->scrollBarGrooveSurface.g1Top);
	s.writeEntry("/scrollBarGrooveSurface_g1Bottom", previewStyle->scrollBarGrooveSurface.g1Bottom);
	s.writeEntry("/scrollBarGrooveSurface_g2Top", previewStyle->scrollBarGrooveSurface.g2Top);
	s.writeEntry("/scrollBarGrooveSurface_g2Bottom", previewStyle->scrollBarGrooveSurface.g2Bottom);
	s.writeEntry("/scrollBarGrooveSurface_background", previewStyle->scrollBarGrooveSurface.background.name());
	
	// tabs
	s.writeEntry("/activeTabTopSurface_numGradients", previewStyle->activeTabTopSurface.numGradients);
	s.writeEntry("/activeTabTopSurface_g1Color1", previewStyle->activeTabTopSurface.g1Color1.name());
	s.writeEntry("/activeTabTopSurface_g1Color2", previewStyle->activeTabTopSurface.g1Color2.name());
	s.writeEntry("/activeTabTopSurface_g2Color1", previewStyle->activeTabTopSurface.g2Color1.name());
	s.writeEntry("/activeTabTopSurface_g2Color2", previewStyle->activeTabTopSurface.g2Color2.name());
	s.writeEntry("/activeTabTopSurface_g1Top", previewStyle->activeTabTopSurface.g1Top);
	s.writeEntry("/activeTabTopSurface_g1Bottom", previewStyle->activeTabTopSurface.g1Bottom);
	s.writeEntry("/activeTabTopSurface_g2Top", previewStyle->activeTabTopSurface.g2Top);
	s.writeEntry("/activeTabTopSurface_g2Bottom", previewStyle->activeTabTopSurface.g2Bottom);
	s.writeEntry("/activeTabTopSurface_background", previewStyle->activeTabTopSurface.background.name());
	
	s.writeEntry("/tabTopSurface_numGradients", previewStyle->tabTopSurface.numGradients);
	s.writeEntry("/tabTopSurface_g1Color1", previewStyle->tabTopSurface.g1Color1.name());
	s.writeEntry("/tabTopSurface_g1Color2", previewStyle->tabTopSurface.g1Color2.name());
	s.writeEntry("/tabTopSurface_g2Color1", previewStyle->tabTopSurface.g2Color1.name());
	s.writeEntry("/tabTopSurface_g2Color2", previewStyle->tabTopSurface.g2Color2.name());
	s.writeEntry("/tabTopSurface_g1Top", previewStyle->tabTopSurface.g1Top);
	s.writeEntry("/tabTopSurface_g1Bottom", previewStyle->tabTopSurface.g1Bottom);
	s.writeEntry("/tabTopSurface_g2Top", previewStyle->tabTopSurface.g2Top);
	s.writeEntry("/tabTopSurface_g2Bottom", previewStyle->tabTopSurface.g2Bottom);
	s.writeEntry("/tabTopSurface_background", previewStyle->tabTopSurface.background.name());
	
	s.writeEntry("/activeTabBottomSurface_numGradients", previewStyle->activeTabBottomSurface.numGradients);
	s.writeEntry("/activeTabBottomSurface_g1Color1", previewStyle->activeTabBottomSurface.g1Color1.name());
	s.writeEntry("/activeTabBottomSurface_g1Color2", previewStyle->activeTabBottomSurface.g1Color2.name());
	s.writeEntry("/activeTabBottomSurface_g2Color1", previewStyle->activeTabBottomSurface.g2Color1.name());
	s.writeEntry("/activeTabBottomSurface_g2Color2", previewStyle->activeTabBottomSurface.g2Color2.name());
	s.writeEntry("/activeTabBottomSurface_g1Top", previewStyle->activeTabBottomSurface.g1Top);
	s.writeEntry("/activeTabBottomSurface_g1Bottom", previewStyle->activeTabBottomSurface.g1Bottom);
	s.writeEntry("/activeTabBottomSurface_g2Top", previewStyle->activeTabBottomSurface.g2Top);
	s.writeEntry("/activeTabBottomSurface_g2Bottom", previewStyle->activeTabBottomSurface.g2Bottom);
	s.writeEntry("/activeTabBottomSurface_background", previewStyle->activeTabBottomSurface.background.name());
	
	s.writeEntry("/tabBottomSurface_numGradients", previewStyle->tabBottomSurface.numGradients);
	s.writeEntry("/tabBottomSurface_g1Color1", previewStyle->tabBottomSurface.g1Color1.name());
	s.writeEntry("/tabBottomSurface_g1Color2", previewStyle->tabBottomSurface.g1Color2.name());
	s.writeEntry("/tabBottomSurface_g2Color1", previewStyle->tabBottomSurface.g2Color1.name());
	s.writeEntry("/tabBottomSurface_g2Color2", previewStyle->tabBottomSurface.g2Color2.name());
	s.writeEntry("/tabBottomSurface_g1Top", previewStyle->tabBottomSurface.g1Top);
	s.writeEntry("/tabBottomSurface_g1Bottom", previewStyle->tabBottomSurface.g1Bottom);
	s.writeEntry("/tabBottomSurface_g2Top", previewStyle->tabBottomSurface.g2Top);
	s.writeEntry("/tabBottomSurface_g2Bottom", previewStyle->tabBottomSurface.g2Bottom);
	s.writeEntry("/tabBottomSurface_background",previewStyle-> tabBottomSurface.background.name());
	
}

void PreviewCheckBox::paintEvent(QPaintEvent* ) {
	
	QPainter p(this);
	drawButton(&p);
}

// checkBoxes and radioButtons without pixmapCache
void PreviewCheckBox::drawButton(QPainter *p) {

	QStyle::SFlags flags = QStyle::Style_Default|Domino_noCache;
	if ( isEnabled() )
		flags |= QStyle::Style_Enabled;
	if ( hasFocus() )
		flags |= QStyle::Style_HasFocus;
	if ( isDown() )
		flags |= QStyle::Style_Down;
	if ( hasMouse() )
		flags |= QStyle::Style_MouseOver;
	if ( state() == QButton::On )
		flags |= QStyle::Style_On;
	else if ( state() == QButton::Off )
		flags |= QStyle::Style_Off;
// 	else if ( state() == QButton::NoChange )
// 		flags |= QStyle::Style_NoChange;
	
	QRect irect = QStyle::visualRect( style().subRect(QStyle::SR_CheckBoxIndicator, this), this );
	p->setBackgroundMode(Qt::OpaqueMode);
	drawButtonLabel(p);
	
	QPixmap pix(18,19);
	QPainter painter(&pix);
	pix.fill(backgroundColor());
	style().drawControl(QStyle::CE_CheckBox, &painter, this, irect, colorGroup(), flags);
	bitBlt(this, 0, 0, &pix);
}

void PreviewRadioButton::paintEvent(QPaintEvent* ) {

	QPainter p(this);
	drawButton(&p);
}

void PreviewRadioButton::drawButton(QPainter *p) {
	
	QStyle::SFlags flags = QStyle::Style_Default|Domino_noCache;
	if ( isEnabled() )
		flags |= QStyle::Style_Enabled;
	if ( hasFocus() )
		flags |= QStyle::Style_HasFocus;
	if ( isDown() )
		flags |= QStyle::Style_Down;
	if ( hasMouse() )
		flags |= QStyle::Style_MouseOver;
	if ( state() == QButton::On )
		flags |= QStyle::Style_On;
	else if ( state() == QButton::Off )
		flags |= QStyle::Style_Off;
	
	QRect irect = QStyle::visualRect( style().subRect(QStyle::SR_RadioButtonIndicator, this), this );
	irect.addCoords(0,-1,0,0);
	p->setBackgroundMode(Qt::OpaqueMode);
	drawButtonLabel( p );
	
	QPixmap pix(17,17);
	QPainter painter(&pix);
	pix.fill(backgroundColor());
	style().drawControl(QStyle::CE_RadioButton, &painter, this, irect, colorGroup(), flags);
	bitBlt(this, 0, 0, &pix);
}


void PreviewTabBar::paintEvent(QPaintEvent* event) {
	
	QString height = QString::number(rect().height());
	QString color = QString::number(palette().active().background().pixel(), 16);
	
	
	QPixmapCache::remove("firsttabEdges2-bottom-sel"+color+height);
	QPixmapCache::remove("lasttabEdges2-bottom-sel"+color+height);
	QPixmapCache::remove("singletabEdges2-bottom-sel"+color+height);
	QPixmapCache::remove("middletabEdges2-bottom-sel"+color+height);
			
	QPixmapCache::remove("tabEdges2-bottom"+color+height);
			
	QPixmapCache::remove("firsttabEdges2-top-sel"+color+height);
	QPixmapCache::remove("lasttabEdges2-top-sel"+color+height);
	QPixmapCache::remove("singletabEdges2-top-sel"+color+height);
	QPixmapCache::remove("middletabEdges2-top-sel"+color+height);
	QPixmapCache::remove("tabEdges2-top"+color+height);
	
	QTabBar::paintEvent(event);
	
}


void PreviewTabBar::mousePressEvent(QMouseEvent* event) {
	
	QTabWidget* tw = static_cast<QTabWidget*>(parentWidget());
	bool tabsAbove = tw->tabPosition() == QTabWidget::Top;
	QPoint point = ((QMouseEvent*)event)->pos();
	QTab* tab = selectTab(point);
	if(!tab)
		return;
	
	bool activeTab = currentTab() == tab->identifier();
	bool changed = false;
	
	if(tabPosCombo->currentItem() == tabsAbove) {
		tabPosCombo->setCurrentItem(!tabsAbove);
		changed = true;
	}
	if(tabStateCombo->currentItem() == activeTab) {
		tabStateCombo->setCurrentItem(!activeTab);
		changed = true;
	}
	if(changed)
		styleConfig->userLoadTabConfig();
}

void IndicatorPrevButton::paintEvent(QPaintEvent* /*event*/)
{
	if(!isEnabled()) {
		erase();
		return;
	}
	QStyle::SFlags flags = QStyle::Style_Default;
	if (isEnabled())
		flags |= QStyle::Style_Enabled;
	if (isDown())
		flags |= QStyle::Style_Down;
	if (isOn())
		flags |= QStyle::Style_On;
	if (! isFlat() && ! isDown())
		flags |= QStyle::Style_Raised;
	if (isDefault())
		flags |= QStyle::Style_ButtonDefault;
	
	QPixmap buffer(width(), height());
	QPainter p(&buffer);
	buffer.fill(paletteBackgroundColor());
	
	previewStyle->drawControl(QStyle::CE_PushButton, &p, this, rect(), colorGroup(), flags);
	
	previewStyle->drawFocusIndicator(&p, style().subRect(QStyle::SR_PushButtonFocusRect, this),  AlignHCenter| AlignVCenter |ShowPrefix,
				    colorGroup(), true, 0, indicatorPreviewText, -1, indicatorBtnColor->color(), (indicatorModeCombo->currentItem() == 1), true);
	QColor fg = colorGroup().buttonText();
	
	TextEffect textEffectMode_save = previewStyle->textEffectSettings.mode;
	if(!drawTextEffect->isChecked())
		previewStyle->textEffectSettings.mode = TextEffect_None;
	previewStyle->dominoDrawItem(&p, rect(),  AlignHCenter| AlignVCenter |ShowPrefix, colorGroup(), true, 0, indicatorPreviewText, -1, &fg, previewStyle->textEffectSettings.mode > 0 ? 1 : 0);
	previewStyle->textEffectSettings.mode = textEffectMode_save;
	
	p.end();
	p.begin(this);
	p.drawPixmap(0,0,buffer);
}

void IndicatorPrevWidget::paintEvent(QPaintEvent* /*event*/)
{
	if(!isEnabled()) {
		erase();
		return;
	}
	
	QPixmap buffer(width(), height());
	QPainter p(&buffer);
	buffer.fill(paletteBackgroundColor());
	
	previewStyle->drawFocusIndicator(&p, rect(),  AlignHCenter| AlignVCenter |ShowPrefix, colorGroup(), true, 0, indicatorPreviewText,
				    -1, indicatorColor->color(), (indicatorModeCombo->currentItem() == 1));
	QColor fg = colorGroup().foreground();
	previewStyle->drawItem(&p, style().subRect(QStyle::SR_PushButtonFocusRect, this),  AlignHCenter| AlignVCenter |ShowPrefix, colorGroup(),
	true, 0, indicatorPreviewText, -1, &fg);
	
	p.end();
	p.begin(this);
	p.drawPixmap(0,0,buffer);
}

void TextEffectPrevButton::paintEvent(QPaintEvent* /*event*/)
{

	if(!isEnabled()) {
		erase();
		return;
	}
	
	
	QStyle::SFlags flags = QStyle::Style_Default;
	if (isEnabled())
		flags |= QStyle::Style_Enabled;
	if (isDown()) {
		flags |= QStyle::Style_Down;
	}
	if (isOn())
		flags |= QStyle::Style_On;
	if (! isFlat() && ! isDown())
		flags |= QStyle::Style_Raised;
	if (isDefault())
		flags |= QStyle::Style_ButtonDefault;
	
	QPixmap buffer(width(), height());
	QPainter p(&buffer);
	previewStyle->drawControl(QStyle::CE_PushButton, &p, this, rect(), colorGroup(), flags);
	
	drawButtonLabel(&p);
	
	p.end();
	p.begin(this);
	p.drawPixmap(0,0,buffer);
	
}

void TextEffectPrevWidget::paintEvent(QPaintEvent* /*event*/)
{
	if(!drawTextEffect->isChecked() || textEffectModeCombo->currentItem() == 0) {
		erase();
		return;
	}
	
	QPixmap buffer(width(), height());
	buffer.fill(paletteBackgroundColor());
	QPainter p(&buffer);
	previewStyle->drawItem(&p, rect(), AlignHCenter| AlignVCenter, colorGroup(), true, 0, textEffectPreviewText);
	
	p.end();
	p.begin(this);
	p.drawPixmap(0,0, buffer);
	
}

void DominoStyleConfig::languageChange()
{
	
	/////////////////////////////////////////////////
	// tab 1 (general)
	/////////////////////////////////////////////////
	buttonHeightAdjustmentLabel->setText(tr("Button height adjustment:"));
	
	indicateFocus->setText(tr("Indicate focus"));
	indicatorModeCombo->clear();
	indicatorModeCombo->insertItem( tr( "Colored outline" ) );
	indicatorModeCombo->insertItem( tr( "Underline" ) );
	labelIndicatorBtnColor->setText(tr("Button color:"));
	labelIndicatorColor->setText(tr("Color:"));
	indicatorPreviewText = tr("focused");
	int width = indicatorPrevButton->fontMetrics().width(indicatorPreviewText);
	indicatorPrevButton->setMinimumWidth(width+16);
	
	drawTextEffect->setText(tr("Draw text effect"));
	textEffectPreviewText = tr("123 text");
	labelTextEffectButtonColor->setText(tr("Button color:"));
	labelTextEffectColor->setText(tr("Color:"));
	width = textEffectPrevButton->fontMetrics().width(textEffectPreviewText);
	textEffectPrevButton->setMinimumWidth(width+16);
	textEffectModeCombo->clear();
	textEffectModeCombo->insertItem( tr( "Only buttons" ), 0 );
	textEffectModeCombo->insertItem( tr( "Everywhere" ), 1 );
	
	/////////////////////////////////////////////////
	// tab 2.1 (buttons)
	/////////////////////////////////////////////////
	textLabel1_2->setText( tr( "Background Color:" ) );
	kColorButton27_2->setText( QString::null );
	groupBox10_2_2_2->setTitle( tr( "Preview" ) );
   // pushButton16->setText( tr( "PushButton" ) );
	groupBox8_2_2_2->setTitle( tr( "First Gradient" ) );
	textLabel4_3_2_2_2->setText( tr( "ends at:" ) );
	spinBox2_2_2_2_2->setSuffix( tr( " %" ) );
	textLabel2_2_2_2_2->setText( tr( "with color:" ) );
	kColorButton1_2_2_2_2->setText( QString::null );
	textLabel3_3_2_2_2->setText( tr( "starts at:" ) );
	spinBox1_2_2_2_2->setSuffix( tr( " %" ) );
	spinBox1_2_2_2_2->setSpecialValueText( QString::null );
	textLabel1_4_2_2_2->setText( tr( "with color:" ) );
	kColorButton2_4_2_2_2->setText( QString::null );
	groupBox9_2_2_2->setTitle( tr( "Second Gradient" ) );
	textLabel4_4_2_2_2->setText( tr( "ends at:" ) );
	spinBox2_3_2_2_2->setSuffix( tr( " %" ) );
	textLabel3_4_2_2_2->setText( tr( "starts at:" ) );
	spinBox1_3_2_2_2->setSuffix( tr( " %" ) );
	spinBox1_3_2_2_2->setSpecialValueText( QString::null );
	kColorButton1_3_2_2_2->setText( QString::null );
	kColorButton2_5_2_2_2->setText( QString::null );
	textLabel1_5_2_2_2->setText( tr( "with color:" ) );
	textLabel2_3_2_2_2->setText( tr( "with color:" ) );
	textLabel7_4->setText( tr( "Copy from:" ) );
	comboBox3_4->clear();
	comboBox3_4->insertItem( tr( "scrollbars" ) );
	comboBox3_4->insertItem( tr( "headers" ) );
	comboBox3_4->insertItem( tr( "radio/checkboxes" ) );
	pushButton4_4->setText( tr( "OK" ) );
	tabWidget2->changeTab( tab1, tr( "Buttons" ) );
	
	/////////////////////////////////////////////////
	// tab 2.2 (tabs)
	/////////////////////////////////////////////////
	   groupBox8_2_2_2_2->setTitle( tr( "First Gradient" ) );
	   textLabel4_3_2_2_2_2->setText( tr( "ends at:" ) );
	   spinBox2_2_2_2_2_2->setSuffix( tr( " %" ) );
	   textLabel2_2_2_2_2_2->setText( tr( "with color:" ) );
	   kColorButton1_2_2_2_2_2->setText( QString::null );
	   textLabel3_3_2_2_2_2->setText( tr( "starts at:" ) );
	   spinBox1_2_2_2_2_2->setSuffix( tr( " %" ) );
	   spinBox1_2_2_2_2_2->setSpecialValueText( QString::null );
	   textLabel1_4_2_2_2_2->setText( tr( "with color:" ) );
	   kColorButton2_4_2_2_2_2->setText( QString::null );
	   groupBox9_2_2_2_2->setTitle( tr( "Second Gradient" ) );
	   textLabel4_4_2_2_2_2->setText( tr( "ends at:" ) );
	   spinBox2_3_2_2_2_2->setSuffix( tr( " %" ) );
	   textLabel3_4_2_2_2_2->setText( tr( "starts at:" ) );
	   spinBox1_3_2_2_2_2->setSuffix( tr( " %" ) );
	   spinBox1_3_2_2_2_2->setSpecialValueText( QString::null );
	   kColorButton1_3_2_2_2_2->setText( QString::null );
	   kColorButton2_5_2_2_2_2->setText( QString::null );
	   textLabel1_5_2_2_2_2->setText( tr( "with color:" ) );
	   textLabel2_3_2_2_2_2->setText( tr( "with color:" ) );
	   textLabel1_2_2->setText( tr( "Background Color:" ) );
	   kColorButton27_2_2->setText( QString::null );
	   groupBox10_2_2_2_2->setTitle( tr( "Preview" ) );
	   textLabel7_4_2->setText( tr( "Copy from:" ) );
	   comboBox3_4_2->clear();
	   comboBox3_4_2->insertItem( tr( "top active tab" ) );
	   comboBox3_4_2->insertItem( tr( "top inactive tab" ) );
	   comboBox3_4_2->insertItem( tr( "bottom active tab" ) );
	   comboBox3_4_2->insertItem( tr( "bottom inactive tab" ) );
	   comboBox3_4_2->insertItem( tr( "buttons" ) );
	   comboBox3_4_2->insertItem( tr( "scrollbars" ) );
	   comboBox3_4_2->insertItem( tr( "headers" ) );
	   comboBox3_4_2->insertItem( tr( "radio/checkboxes" ) );
	   tabPosCombo->insertItem( tr( "top" ) );
	   tabPosCombo->insertItem( tr( "bottom" ) );
	   tabStateCombo->insertItem( tr( "active" ) );
	   tabStateCombo->insertItem( tr( "inactive" ) );
	   pushButton4_4_2->setText( tr( "OK" ) );
	   tabWidget2->changeTab( tab2, tr( "Tabs" ) );
	
	/////////////////////////////////////////////////
	// tab 2.3 (scrollBars)
	/////////////////////////////////////////////////
	groupBox8_2_2_2_2_4->setTitle( tr( "First Gradient" ) );
	textLabel4_3_2_2_2_2_4->setText( tr( "ends at:" ) );
	spinBox2_2_2_2_2_2_4->setSuffix( tr( " %" ) );
	textLabel2_2_2_2_2_2_4->setText( tr( "with color:" ) );
	kColorButton1_2_2_2_2_2_4->setText( QString::null );
	textLabel3_3_2_2_2_2_4->setText( tr( "starts at:" ) );
	spinBox1_2_2_2_2_2_4->setSuffix( tr( " %" ) );
	spinBox1_2_2_2_2_2_4->setSpecialValueText( QString::null );
	textLabel1_4_2_2_2_2_4->setText( tr( "with color:" ) );
	kColorButton2_4_2_2_2_2_4->setText( QString::null );
	groupBox9_2_2_2_2_4->setTitle( tr( "Second Gradient" ) );
	textLabel4_4_2_2_2_2_4->setText( tr( "ends at:" ) );
	spinBox2_3_2_2_2_2_4->setSuffix( tr( " %" ) );
	textLabel3_4_2_2_2_2_4->setText( tr( "starts at:" ) );
	spinBox1_3_2_2_2_2_4->setSuffix( tr( " %" ) );
	spinBox1_3_2_2_2_2_4->setSpecialValueText( QString::null );
	kColorButton1_3_2_2_2_2_4->setText( QString::null );
	kColorButton2_5_2_2_2_2_4->setText( QString::null );
	textLabel1_5_2_2_2_2_4->setText( tr( "with color:" ) );
	textLabel2_3_2_2_2_2_4->setText( tr( "with color:" ) );
	groupBox10_2_2_2_2_4->setTitle( tr( "Preview" ) );
	kColorButton27_2_2_2->setText( QString::null );
	textLabel1_2_2_2->setText( tr( "Background Color:" ) );
	comboBox7->clear();
	comboBox7->insertItem( tr( "Buttons/Slider" ) );
	comboBox7->insertItem( tr( "Groove" ) );
	textLabel7_4_2_3->setText( tr( "Copy from:" ) );
	comboBox3_4_2_3->clear();
	comboBox3_4_2_3->insertItem( tr( "buttons" ) );
	comboBox3_4_2_3->insertItem( tr( "headers" ) );
	comboBox3_4_2_3->insertItem( tr( "radio/checkboxes" ) );
	pushButton4_4_2_3->setText( tr( "OK" ) );
	tabWidget2->changeTab( tab3, tr( "ScrollBars" ) );
	
	/////////////////////////////////////////////////
	// tab 2.4 (headers)
	/////////////////////////////////////////////////
	kColorButton27_2_2_2_2->setText( QString::null );
	textLabel1_2_2_2_2->setText( tr( "Background Color:" ) );
	groupBox10_2_2_2_2_4_3->setTitle( tr( "Preview" ) );
	groupBox8_2_2_2_2_4_3->setTitle( tr( "First Gradient" ) );
	textLabel4_3_2_2_2_2_4_3->setText( tr( "ends at:" ) );
	spinBox2_2_2_2_2_2_4_3->setSuffix( tr( " %" ) );
	textLabel2_2_2_2_2_2_4_3->setText( tr( "with color:" ) );
	kColorButton1_2_2_2_2_2_4_3->setText( QString::null );
	textLabel3_3_2_2_2_2_4_3->setText( tr( "starts at:" ) );
	spinBox1_2_2_2_2_2_4_3->setSuffix( tr( " %" ) );
	spinBox1_2_2_2_2_2_4_3->setSpecialValueText( QString::null );
	textLabel1_4_2_2_2_2_4_3->setText( tr( "with color:" ) );
	kColorButton2_4_2_2_2_2_4_3->setText( QString::null );
	groupBox9_2_2_2_2_4_3->setTitle( tr( "Second Gradient" ) );
	textLabel4_4_2_2_2_2_4_3->setText( tr( "ends at:" ) );
	spinBox2_3_2_2_2_2_4_3->setSuffix( tr( " %" ) );
	textLabel3_4_2_2_2_2_4_3->setText( tr( "starts at:" ) );
	spinBox1_3_2_2_2_2_4_3->setSuffix( tr( " %" ) );
	spinBox1_3_2_2_2_2_4_3->setSpecialValueText( QString::null );
	kColorButton1_3_2_2_2_2_4_3->setText( QString::null );
	kColorButton2_5_2_2_2_2_4_3->setText( QString::null );
	textLabel1_5_2_2_2_2_4_3->setText( tr( "with color:" ) );
	textLabel2_3_2_2_2_2_4_3->setText( tr( "with color:" ) );
	textLabel7_4_2_3_3->setText( tr( "Copy from:" ) );
	comboBox3_4_2_3_3->clear();
	comboBox3_4_2_3_3->insertItem( tr( "buttons" ) );
	comboBox3_4_2_3_3->insertItem( tr( "scrollbars" ) );
	comboBox3_4_2_3_3->insertItem( tr( "radio/checkboxes" ) );
	pushButton4_4_2_3_3->setText( tr( "OK" ) );
	tabWidget2->changeTab( tab4, tr( "Headers" ) );
	
	/////////////////////////////////////////////////
	// tab 2.5 (radio/checkboxes)
	/////////////////////////////////////////////////
	groupBox8_2_2_2_2_4_3_2->setTitle( tr( "First Gradient" ) );
	textLabel4_3_2_2_2_2_4_3_2->setText( tr( "ends at:" ) );
	spinBox2_2_2_2_2_2_4_3_2->setSuffix( tr( " %" ) );
	textLabel2_2_2_2_2_2_4_3_2->setText( tr( "with color:" ) );
	kColorButton1_2_2_2_2_2_4_3_2->setText( QString::null );
	textLabel3_3_2_2_2_2_4_3_2->setText( tr( "starts at:" ) );
	spinBox1_2_2_2_2_2_4_3_2->setSuffix( tr( " %" ) );
	spinBox1_2_2_2_2_2_4_3_2->setSpecialValueText( QString::null );
	textLabel1_4_2_2_2_2_4_3_2->setText( tr( "with color:" ) );
	kColorButton2_4_2_2_2_2_4_3_2->setText( QString::null );
	kColorButton27_2_2_2_3->setText( QString::null );
	textLabel1_2_2_2_3->setText( tr( "Background Color:" ) );
	groupBox10_2_2_2_2_4_3_2->setTitle( tr( "Preview" ) );
	radioButton2->setText( tr( "RadioButton" ) );
	checkBox2->setText( tr( "CheckBox" ) );
	groupBox9_2_2_2_2_4_3_2->setTitle( tr( "Second Gradient" ) );
	textLabel4_4_2_2_2_2_4_3_2->setText( tr( "ends at:" ) );
	spinBox2_3_2_2_2_2_4_3_2->setSuffix( tr( " %" ) );
	textLabel3_4_2_2_2_2_4_3_2->setText( tr( "starts at:" ) );
	spinBox1_3_2_2_2_2_4_3_2->setSuffix( tr( " %" ) );
	spinBox1_3_2_2_2_2_4_3_2->setSpecialValueText( QString::null );
	kColorButton1_3_2_2_2_2_4_3_2->setText( QString::null );
	kColorButton2_5_2_2_2_2_4_3_2->setText( QString::null );
	textLabel1_5_2_2_2_2_4_3_2->setText( tr( "with color:" ) );
	textLabel2_3_2_2_2_2_4_3_2->setText( tr( "with color:" ) );
	textLabel7_4_2_3_3_2->setText( tr( "Copy from:" ) );
	comboBox3_4_2_3_3_2->clear();
	comboBox3_4_2_3_3_2->insertItem( tr( "buttons" ) );
	comboBox3_4_2_3_3_2->insertItem( tr( "scrollbars" ) );
	comboBox3_4_2_3_3_2->insertItem( tr( "headers" ) );
	pushButton4_4_2_3_3_2->setText( tr( "OK" ) );
	
	tabWidget2->changeTab( tab5, tr( "Radio/Checkboxes" ) );
	
	/////////////////////////////////////////////////
	// tab 3 (load/save)
	/////////////////////////////////////////////////
	groupBox_ls->setTitle( QString::null );
	listView_ls->header()->setLabel( 0, tr( "Name" ) );
	loadButton->setText( tr( "Load Config" ) );
	saveButton->setText( tr( "Save Config" ) );
	deleteButton->setText( tr( "Delete Config" ) );
	importButton->setText( tr( "Import Config" ) );


}


DominoStyleConfig::~DominoStyleConfig()
{
	delete schemeList;
}


void DominoStyleConfig::save()
{
	bool settingsChanged;
	
	if(
		  (centerTabs->isChecked() == origCenterTabs) &&
		  (animateProgressBar->isChecked() == origAnimProgressBar) &&
		  (drawTriangularExpander->isChecked() == origDrawTriangularExpander) &&
		  (highlightToolBtnIcons->isChecked() == origHighlightToolBtnIcons) &&
		  (toolBtnAsBtn->isChecked() == origToolBtnAsBtn) &&
		  (customCheckMarkColor->isChecked() == origCustomCheckMarkColor) &&
		  (checkMarkColor->color() == origCheckMarkColor) &&
		  (tintGroupBoxBackground->isChecked() == origTintGroupBoxBackground) &&
		  (customGroupBoxBackgroundColor->isChecked() == origCustomGroupBoxBackgroundColor) &&
		  (groupBoxBackgroundColor->color() == origGroupBoxBackgroundColor) &&
		  (grSlider->value() == origGroupBoxBrightness) &&
		  (customToolTipColor->isChecked() == origCustomToolTipColor) &&
		  (toolTipColor->color() == origToolTipColor) &&
		  (smoothScrolling->isChecked() == origSmoothScrolling) &&
		  (buttonHeightAdjustment->value() == origButtonHeightAdjustment) &&
		  
		  ((rubberBandType == origRubberBandType) || (!hasCompositeExtension && origRubberBandType == DominoStyle::ArgbRubber && rubberBandType == DominoStyle::DistRubber)) &&
		  (argbRubberColor->color() == origArgbRubberColor) &&
		  (argbRubberOpacity->value() == origArgbRubberOpacity) &&
		  
		  (customPopupMenuColor->isChecked() == origCustomPopupMenuColor) &&
		  (popupMenuColor->color() == origPopupMenuColor) &&
		  (customSelMenuItemColor->isChecked() == origCustomSelMenuItemColor) &&
		  (selMenuItemColor->color() == origSelMenuItemColor) &&
		  (drawPopupMenuGradient->isChecked() == origDrawPopupMenuGradient) &&
		  (indentPopupMenuItems->isChecked() == origIndentPopupMenuItems) &&
		  
		  (indicateFocus->isChecked() == origIndicateFocus) &&
		  (indicatorColor->color() == origIndicatorColor) &&
		  (indicatorBtnColor->color() == origIndicatorBtnColor) &&
		  (indicatorModeCombo->currentItem() == (int)origDrawUnderline) &&
		  (indicatorOpacity->value() == origIndicatorOpacity) &&
		  (indicatorBtnOpacity->value() == origIndicatorBtnOpacity) &&
		  
		  (drawTextEffect->isChecked() == origDrawTextEffect) &&
		  (textEffectModeCombo->currentItem() == origTextEffectMode) &&
		  (textEffectColor->color() == origTextEffectColor) &&
		  (textEffectButtonColor->color() == origTextEffectButtonColor) &&
		  (textEffectOpacity->value() == origTextEffectOpacity) &&
		  (textEffectButtonOpacity->value() == origTextEffectButtonOpacity) &&
		  (textEffectPos->currentItem() == origTextEffectPos) &&
		  (textEffectButtonPos->currentItem() == origTextEffectButtonPos) &&
		  (drawButtonSunkenShadow->isChecked() == origDrawButtonSunkenShadow) &&
		  
		  (buttonContourType->currentItem() == origButtonContourType) &&
		  (buttonContourColor->color() == origButtonContourColor) &&
		  (buttonDefaultButtonContourColor->color() == origButtonDefaultButtonContourColor) &&
		  (buttonMouseOverContourColor->color() == origButtonMouseOverContourColor) &&
		  (buttonPressedContourColor->color() == origButtonPressedContourColor) &&
		  
		  (kColorButton2_4_2_2_2->color() == origBtnSurface.g1Color1) &&
		  (kColorButton1_2_2_2_2->color() == origBtnSurface.g1Color2) &&
		  (kColorButton2_5_2_2_2->color() == origBtnSurface.g2Color1) &&
		  (kColorButton1_3_2_2_2->color() == origBtnSurface.g2Color2) &&
		  (spinBox1_2_2_2_2->value() == origBtnSurface.g1Top) &&
		  (spinBox2_2_2_2_2->value() == origBtnSurface.g1Bottom) &&
		  (spinBox1_3_2_2_2->value() == origBtnSurface.g2Top) &&
		  (spinBox2_3_2_2_2->value() == origBtnSurface.g2Bottom) &&
		  (kColorButton27_2->color() == origBtnSurface.background) &&
		  ((groupBox9_2_2_2->isChecked() ? 2 : groupBox8_2_2_2->isChecked() ? 1 : 0 ) == origBtnSurface.numGradients) &&
		  
		  (kColorButton2_4_2_2_2_2_4_3->color() == origHeaderSurface.g1Color1) &&
		  (kColorButton1_2_2_2_2_2_4_3->color() == origHeaderSurface.g1Color2) &&
		  (kColorButton2_5_2_2_2_2_4_3->color() == origHeaderSurface.g2Color1) &&
		  (kColorButton1_3_2_2_2_2_4_3->color() == origHeaderSurface.g2Color2) &&
		  (spinBox1_2_2_2_2_2_4_3->value() == origHeaderSurface.g1Top) &&
		  (spinBox2_2_2_2_2_2_4_3->value() == origHeaderSurface.g1Bottom) &&
		  (spinBox1_3_2_2_2_2_4_3->value() == origHeaderSurface.g2Top) &&
		  (spinBox2_3_2_2_2_2_4_3->value() == origHeaderSurface.g2Bottom) &&
		  (kColorButton27_2_2_2_2->color() == origHeaderSurface.background) &&
		  ((groupBox9_2_2_2_2_4_3->isChecked() ? 2 : groupBox8_2_2_2_2_4_3->isChecked() ? 1 : 0 ) == origHeaderSurface.numGradients) &&

		  (kColorButton2_4_2_2_2_2_4_3_2->color() == origCheckItemSurface.g1Color1) &&
		  (kColorButton1_2_2_2_2_2_4_3_2->color() == origCheckItemSurface.g1Color2) &&
		  (kColorButton2_5_2_2_2_2_4_3_2->color() == origCheckItemSurface.g2Color1) &&
		  (kColorButton1_3_2_2_2_2_4_3_2->color() == origCheckItemSurface.g2Color2) &&
		  (spinBox1_2_2_2_2_2_4_3_2->value() == origCheckItemSurface.g1Top) &&
		  (spinBox2_2_2_2_2_2_4_3_2->value() == origCheckItemSurface.g1Bottom) &&
		  (spinBox1_3_2_2_2_2_4_3_2->value() == origCheckItemSurface.g2Top) &&
		  (spinBox2_3_2_2_2_2_4_3_2->value() == origCheckItemSurface.g2Bottom) &&
		  (kColorButton27_2_2_2_3->color() == origCheckItemSurface.background) &&
		  ((groupBox9_2_2_2_2_4_3_2->isChecked() ? 2 : groupBox8_2_2_2_2_4_3_2->isChecked() ? 1 : 0 ) == origCheckItemSurface.numGradients) &&
		
		  (previewStyle->scrollBarSurface.g1Color1 == origScrollBarSurface.g1Color1) &&
		  (previewStyle->scrollBarSurface.g1Color2 == origScrollBarSurface.g1Color2) &&
		  (previewStyle->scrollBarSurface.g2Color1 == origScrollBarSurface.g2Color1) &&
		  (previewStyle->scrollBarSurface.g2Color2 == origScrollBarSurface.g2Color2) &&
		  (previewStyle->scrollBarSurface.g1Top == origScrollBarSurface.g1Top) &&
		  (previewStyle->scrollBarSurface.g1Bottom == origScrollBarSurface.g1Bottom) &&
		  (previewStyle->scrollBarSurface.g2Top == origScrollBarSurface.g2Top) &&
		  (previewStyle->scrollBarSurface.g2Bottom == origScrollBarSurface.g2Bottom) &&
		  (previewStyle->scrollBarSurface.background == origScrollBarSurface.background) &&
		  (previewStyle->scrollBarSurface.numGradients == origScrollBarSurface.numGradients) &&

		  (previewStyle->scrollBarGrooveSurface.g1Color1 == origScrollBarGrooveSurface.g1Color1) &&
		  (previewStyle->scrollBarGrooveSurface.g1Color2 == origScrollBarGrooveSurface.g1Color2) &&
		  (previewStyle->scrollBarGrooveSurface.g2Color1 == origScrollBarGrooveSurface.g2Color1) &&
		  (previewStyle->scrollBarGrooveSurface.g2Color2 == origScrollBarGrooveSurface.g2Color2) &&
		  (previewStyle->scrollBarGrooveSurface.g1Top == origScrollBarGrooveSurface.g1Top) &&
		  (previewStyle->scrollBarGrooveSurface.g1Bottom == origScrollBarGrooveSurface.g1Bottom) &&
		  (previewStyle->scrollBarGrooveSurface.g2Top == origScrollBarGrooveSurface.g2Top) &&
		  (previewStyle->scrollBarGrooveSurface.g2Bottom == origScrollBarGrooveSurface.g2Bottom) &&
		  (previewStyle->scrollBarGrooveSurface.background == origScrollBarGrooveSurface.background) &&
		  (previewStyle->scrollBarGrooveSurface.numGradients == origScrollBarGrooveSurface.numGradients) &&
		  
		  (previewStyle->activeTabTopSurface.g1Color1 == origActiveTabTopSurface.g1Color1) &&
		  (previewStyle->activeTabTopSurface.g1Color2 == origActiveTabTopSurface.g1Color2) &&
		  (previewStyle->activeTabTopSurface.g2Color1 == origActiveTabTopSurface.g2Color1) &&
		  (previewStyle->activeTabTopSurface.g2Color2 == origActiveTabTopSurface.g2Color2) &&
		  (previewStyle->activeTabTopSurface.background == origActiveTabTopSurface.background) &&
		  (previewStyle->activeTabTopSurface.g1Top == origActiveTabTopSurface.g1Top) &&
		  (previewStyle->activeTabTopSurface.g1Bottom == origActiveTabTopSurface.g1Bottom) &&
		  (previewStyle->activeTabTopSurface.g2Top == origActiveTabTopSurface.g2Top) &&
		  (previewStyle->activeTabTopSurface.g2Bottom == origActiveTabTopSurface.g2Bottom) &&
		  (previewStyle->activeTabTopSurface.numGradients == origActiveTabTopSurface.numGradients) &&
		  
		  (previewStyle->tabTopSurface.g1Color1 == origTabTopSurface.g1Color1) &&
		  (previewStyle->tabTopSurface.g1Color2 == origTabTopSurface.g1Color2) &&
		  (previewStyle->tabTopSurface.g2Color1 == origTabTopSurface.g2Color1) &&
		  (previewStyle->tabTopSurface.g2Color2 == origTabTopSurface.g2Color2) &&
		  (previewStyle->tabTopSurface.background == origTabTopSurface.background) &&
		  (previewStyle->tabTopSurface.g1Top == origTabTopSurface.g1Top) &&
		  (previewStyle->tabTopSurface.g1Bottom == origTabTopSurface.g1Bottom) &&
		  (previewStyle->tabTopSurface.g2Top == origTabTopSurface.g2Top) &&
		  (previewStyle->tabTopSurface.g2Bottom == origTabTopSurface.g2Bottom) &&
		  (previewStyle->tabTopSurface.numGradients == origTabTopSurface.numGradients) &&
		  
		  (previewStyle->activeTabBottomSurface.g1Color1 == origActiveTabBottomSurface.g1Color1) &&
		  (previewStyle->activeTabBottomSurface.g1Color2 == origActiveTabBottomSurface.g1Color2) &&
		  (previewStyle->activeTabBottomSurface.g2Color1 == origActiveTabBottomSurface.g2Color1) &&
		  (previewStyle->activeTabBottomSurface.g2Color2 == origActiveTabBottomSurface.g2Color2) &&
		  (previewStyle->activeTabBottomSurface.background == origActiveTabBottomSurface.background) &&
		  (previewStyle->activeTabBottomSurface.g1Top == origActiveTabBottomSurface.g1Top) &&
		  (previewStyle->activeTabBottomSurface.g1Bottom == origActiveTabBottomSurface.g1Bottom) &&
		  (previewStyle->activeTabBottomSurface.g2Top == origActiveTabBottomSurface.g2Top) &&
		  (previewStyle->activeTabBottomSurface.g2Bottom == origActiveTabBottomSurface.g2Bottom) &&
		  (previewStyle->activeTabBottomSurface.numGradients == origActiveTabBottomSurface.numGradients) &&
		  
		  (previewStyle->tabBottomSurface.g1Color1 == origTabBottomSurface.g1Color1) &&
		  (previewStyle->tabBottomSurface.g1Color2 == origTabBottomSurface.g1Color2) &&
		  (previewStyle->tabBottomSurface.g2Color1 == origTabBottomSurface.g2Color1) &&
		  (previewStyle->tabBottomSurface.g2Color2 == origTabBottomSurface.g2Color2) &&
		  (previewStyle->tabBottomSurface.background == origTabBottomSurface.background) &&
		  (previewStyle->tabBottomSurface.g1Top == origTabBottomSurface.g1Top) &&
		  (previewStyle->tabBottomSurface.g1Bottom == origTabBottomSurface.g1Bottom) &&
		  (previewStyle->tabBottomSurface.g2Top == origTabBottomSurface.g2Top) &&
		  (previewStyle->tabBottomSurface.g2Bottom == origTabBottomSurface.g2Bottom) &&
		  (previewStyle->tabBottomSurface.numGradients == origTabBottomSurface.numGradients)
	  )
		settingsChanged = false;
	else
		settingsChanged = true;

	
	if(settingsChanged) {
		if(!QString(currentConfig).endsWith(" (" + i18n("modified") + ")")) {
			currentConfig = QString(currentConfig).append(" (" + i18n("modified") + ")");
		}
	  }
	  
	  if(configLoaded)
		  settingsChanged = true;
	  if(settingsChanged)
		  emit changed(true);
	  else
		  emit changed(false);
	  
	saveConfig("domino", currentConfig);
}

void DominoStyleConfig::defaults()
{
	QColor background = qApp->palette().active().background();
	QColor button = qApp->palette().active().button();
	QColor highlight = qApp->palette().active().highlight();
	
	animateProgressBar->setChecked(true);
	centerTabs->setChecked(false);
	drawTriangularExpander->setChecked(true);
	toolBtnAsBtn->setChecked(true);
	highlightToolBtnIcons->setChecked(false);
	customCheckMarkColor->setChecked(false);
	checkMarkColor->setColor(QColor(0,0,0));
	tintGroupBoxBackground->setChecked(true);
	customGroupBoxBackgroundColor->setChecked(false);
	groupBoxBackgroundColor->setColor(background.dark(110));
	grSlider->setValue(10);
	customToolTipColor->setChecked(false);
	toolTipColor->setColor(QColor(218,218,179));
	smoothScrolling->setChecked(true);
	buttonHeightAdjustment->setValue(0);
	
	distRubber->setChecked(true);
	argbRubberColor->setColor(highlight);
	argbRubberOpacity->setValue(30);
	
	customPopupMenuColor->setChecked(false);
	popupMenuColor->setColor(background.name());
	customSelMenuItemColor->setChecked(false);
	selMenuItemColor->setColor(highlight.name());
	drawPopupMenuGradient->setChecked(true);
	indentPopupMenuItems->setChecked(true);
	
	indicateFocus->setChecked(true);
	indicatorColor->setColor(highlight.name());
	indicatorBtnColor->setColor(highlight.name());
	indicatorModeCombo->setCurrentItem(1); // underline
	indicatorModeChanged();
	indicatorOpacity->setValue(60);
	indicatorBtnOpacity->setValue(60);
	
	drawTextEffect->setChecked(true);
	textEffectModeCombo->setCurrentItem(0); // only on buttons
	textEffectColor->setColor(Qt::white.name());
	textEffectButtonColor->setColor(Qt::white.name());
	textEffectOpacity->setValue(60);
	textEffectButtonOpacity->setValue(60);
	textEffectPos->setCurrentItem(5);
	textEffectButtonPos->setCurrentItem(5);
	textEffectModeChanged();
	
	drawButtonSunkenShadow->setChecked(true);
	
	buttonContourType->setCurrentItem(1); // raised
	buttonContourColor->setColor(background.dark(250).name());
	buttonDefaultButtonContourColor->setColor(highlight.name());
	buttonMouseOverContourColor->setColor(background.dark(250).name());
	buttonPressedContourColor->setColor(background.dark(250).name());

// buttons / comboBoxes / spinBoxes
	groupBox9_2_2_2->setChecked(true);
	kColorButton2_5_2_2_2->setColor(button.dark(120).name());
	kColorButton1_3_2_2_2->setColor(button.dark(110).name());
	spinBox1_3_2_2_2->setValue(50);
	spinBox2_3_2_2_2->setValue(100);
	groupBox8_2_2_2->setChecked(true);
	kColorButton2_4_2_2_2->setColor(button.name());
	kColorButton1_2_2_2_2->setColor(button.dark(120).name());
	spinBox1_2_2_2_2->setValue(0);
	spinBox2_2_2_2_2->setValue(50);
	kColorButton27_2->setColor(button.name());

// headers
	groupBox9_2_2_2_2_4_3->setChecked(true);
	kColorButton2_5_2_2_2_2_4_3->setColor(background.dark(120).name());
	kColorButton1_3_2_2_2_2_4_3->setColor(background.dark(110).name());
	spinBox1_3_2_2_2_2_4_3->setValue(50);
	spinBox2_3_2_2_2_2_4_3->setValue(100);
	groupBox8_2_2_2_2_4_3->setChecked(true);
	kColorButton2_4_2_2_2_2_4_3->setColor(background.name());
	kColorButton1_2_2_2_2_2_4_3->setColor(background.dark(120).name());
	spinBox1_2_2_2_2_2_4_3->setValue(0);
	spinBox2_2_2_2_2_2_4_3->setValue(50);
	kColorButton27_2_2_2_2->setColor(background.name());

// radio / checkboxes
	groupBox9_2_2_2_2_4_3_2->setChecked(true);
	kColorButton2_5_2_2_2_2_4_3_2->setColor(button.dark(120).name());
	kColorButton1_3_2_2_2_2_4_3_2->setColor(button.dark(110).name());
	spinBox1_3_2_2_2_2_4_3_2->setValue(50);
	spinBox2_3_2_2_2_2_4_3_2->setValue(100);
	groupBox8_2_2_2_2_4_3_2->setChecked(true);
	kColorButton2_4_2_2_2_2_4_3_2->setColor(button.name());
	kColorButton1_2_2_2_2_2_4_3_2->setColor(button.dark(120).name());
	spinBox1_2_2_2_2_2_4_3_2->setValue(0);
	spinBox2_2_2_2_2_2_4_3_2->setValue(50);
	kColorButton27_2_2_2_3->setColor(button.name());

// scrollBar Groove / Buttons

	previewStyle->scrollBarGrooveSurface.numGradients = 0;
	previewStyle->scrollBarGrooveSurface.g1Color1 = background.name();
	previewStyle->scrollBarGrooveSurface.g1Color2 = background.dark(120).name();
	previewStyle->scrollBarGrooveSurface.g2Color1 = background.dark(120).name();
	previewStyle->scrollBarGrooveSurface.g2Color2 = background.dark(110).name();
	previewStyle->scrollBarGrooveSurface.g1Top = 0;
	previewStyle->scrollBarGrooveSurface.g1Bottom = 50;
	previewStyle->scrollBarGrooveSurface.g2Top = 50;
	previewStyle->scrollBarGrooveSurface.g2Bottom = 100;
	previewStyle->scrollBarGrooveSurface.background = background.dark(150).name();

	previewStyle->scrollBarSurface.numGradients = 2;
	previewStyle->scrollBarSurface.g1Color1 = button.name();
	previewStyle->scrollBarSurface.g1Color2 = button.dark(120).name();
	previewStyle->scrollBarSurface.g2Color1 = button.dark(120).name();
	previewStyle->scrollBarSurface.g2Color2 = button.dark(110).name();
	previewStyle->scrollBarSurface.g1Top = 0;
	previewStyle->scrollBarSurface.g1Bottom = 50;
	previewStyle->scrollBarSurface.g2Top = 50;
	previewStyle->scrollBarSurface.g2Bottom = 100;
	previewStyle->scrollBarSurface.background = button.name();

	comboBox7->setCurrentItem(0);
	groupBox8_2_2_2_2_4->setChecked(true);
	groupBox9_2_2_2_2_4->setChecked(true);
	kColorButton2_5_2_2_2_2_4->setColor(previewStyle->scrollBarSurface.g2Color1);
	kColorButton1_3_2_2_2_2_4->setColor(previewStyle->scrollBarSurface.g2Color2);
	spinBox1_3_2_2_2_2_4->setValue(previewStyle->scrollBarSurface.g2Top);
	spinBox2_3_2_2_2_2_4->setValue(previewStyle->scrollBarSurface.g2Bottom);
	kColorButton2_4_2_2_2_2_4->setColor(previewStyle->scrollBarSurface.g1Color1);
	kColorButton1_2_2_2_2_2_4->setColor(previewStyle->scrollBarSurface.g1Color2);
	spinBox1_2_2_2_2_2_4->setValue(previewStyle->scrollBarSurface.g1Top);
	spinBox2_2_2_2_2_2_4->setValue(previewStyle->scrollBarSurface.g1Bottom);
	kColorButton27_2_2_2->setColor(previewStyle->scrollBarSurface.background);
	
	// tabs
	previewStyle->activeTabTopSurface.numGradients = 2;
	previewStyle->activeTabTopSurface.g1Color1 = background.light(110).name();
	previewStyle->activeTabTopSurface.g1Color2 = background.name();
	previewStyle->activeTabTopSurface.g2Color1 = background.name();
	previewStyle->activeTabTopSurface.g2Color2 = background.name();
	previewStyle->activeTabTopSurface.background = background.name();
	previewStyle->activeTabTopSurface.g1Top = 0;
	previewStyle->activeTabTopSurface.g1Bottom = 50;
	previewStyle->activeTabTopSurface.g2Top = 50;
	previewStyle->activeTabTopSurface.g2Bottom = 100;
	
	previewStyle->tabTopSurface.numGradients = 2;
	previewStyle->tabTopSurface.g1Color1 = background.light(110).name();
	previewStyle->tabTopSurface.g1Color2 = background.dark(110).name();
	previewStyle->tabTopSurface.g2Color1 = background.dark(110).name();
	previewStyle->tabTopSurface.g2Color2 = background.dark(110).name();
	previewStyle->tabTopSurface.background = background.dark(110).name();
	previewStyle->tabTopSurface.g1Top = 0;
	previewStyle->tabTopSurface.g1Bottom = 50;
	previewStyle->tabTopSurface.g2Top = 50;
	previewStyle->tabTopSurface.g2Bottom = 100;
	
	previewStyle->activeTabBottomSurface.numGradients = 2;
	previewStyle->activeTabBottomSurface.g1Color1 = background.name();
	previewStyle->activeTabBottomSurface.g1Color2 = background.name();
	previewStyle->activeTabBottomSurface.g2Color1 = background.name();
	previewStyle->activeTabBottomSurface.g2Color2 = background.dark(120).name();
	previewStyle->activeTabBottomSurface.background = background.name();
	previewStyle->activeTabBottomSurface.g1Top = 0;
	previewStyle->activeTabBottomSurface.g1Bottom = 50;
	previewStyle->activeTabBottomSurface.g2Top = 50;
	previewStyle->activeTabBottomSurface.g2Bottom = 100;
	
	previewStyle->tabBottomSurface.numGradients = 2;
	previewStyle->tabBottomSurface.g1Color1 = background.dark(110).name();
	previewStyle->tabBottomSurface.g1Color2 = background.dark(110).name();
	previewStyle->tabBottomSurface.g2Color1 = background.dark(110).name();
	previewStyle->tabBottomSurface.g2Color2 = background.dark(120).name();
	previewStyle->tabBottomSurface.background = background.dark(110).name();
	previewStyle->tabBottomSurface.g1Top = 0;
	previewStyle->tabBottomSurface.g1Bottom = 50;
	previewStyle->tabBottomSurface.g2Top = 50;
	previewStyle->tabBottomSurface.g2Bottom = 100;
	
	groupBox8_2_2_2_2->setChecked(true);
	groupBox9_2_2_2_2->setChecked(true);
	kColorButton2_4_2_2_2_2->setColor(background.light(110).name());
	kColorButton1_2_2_2_2_2->setColor(background.name());
	kColorButton2_5_2_2_2_2->setColor(background.name());
	kColorButton1_3_2_2_2_2->setColor(background.name());
	kColorButton27_2_2->setColor(background.name());
	spinBox1_2_2_2_2_2->setValue(0);
	spinBox2_2_2_2_2_2->setValue(50);
	spinBox1_3_2_2_2_2->setValue(50);
	spinBox2_3_2_2_2_2->setValue(100);
	
	tabPosCombo->setCurrentItem(0);
	tabStateCombo->setCurrentItem(0);
}

void DominoStyleConfig::updateChanged()
{
	checkMarkColor->setEnabled(customCheckMarkColor->isChecked());
	toolTipColor->setEnabled(customToolTipColor->isChecked());
	popupMenuColor->setEnabled(customPopupMenuColor->isChecked());
	selMenuItemColor->setEnabled(customSelMenuItemColor->isChecked());
	groupBoxBackgroundColor->setEnabled(customGroupBoxBackgroundColor->isChecked());
	grFrame->setEnabled(tintGroupBoxBackground->isChecked());
	indicateFocusFrame->setEnabled(indicateFocus->isChecked());
	textEffectFrame->setEnabled(drawTextEffect->isChecked());
	indicatorModeChanged();
	textEffectModeChanged();
}

void DominoStyleConfig::copyColorsToButton() {
	
	const DSurface* ds;
	switch(comboBox3_4->currentItem()) {
		case 0:
			ds = &previewStyle->scrollBarSurface;
			break;
		case 1:
			ds = &previewStyle->headerSurface;
			break;
		case 2:
			ds = &previewStyle->checkItemSurface;
			break;
		default:
			return;
	}
	
	kColorButton2_4_2_2_2->setColor(ds->g1Color1);
	kColorButton1_2_2_2_2->setColor(ds->g1Color2);
	kColorButton2_5_2_2_2->setColor(ds->g2Color1);
	kColorButton1_3_2_2_2->setColor(ds->g2Color2);
	kColorButton27_2->setColor(ds->background);
	spinBox1_2_2_2_2->setValue(ds->g1Top);
	spinBox2_2_2_2_2->setValue(ds->g1Bottom);
	spinBox1_3_2_2_2->setValue(ds->g2Top);
	spinBox2_3_2_2_2->setValue(ds->g2Bottom);
	switch(ds->numGradients) {
		case 2:
			groupBox8_2_2_2->setChecked(true);
			groupBox9_2_2_2->setChecked(true);
			break;
		case 1:
			groupBox8_2_2_2->setChecked(true);
			groupBox9_2_2_2->setChecked(false);
			break;
		default:
			groupBox8_2_2_2->setChecked(false);
			groupBox9_2_2_2->setChecked(false);
	}
	btnNumGradientsChanged();
}

void DominoStyleConfig::copyColorsToTab() {
	
	const DSurface* ds;
	switch(comboBox3_4_2->currentItem()) {
		case 0:
			ds = &previewStyle->activeTabTopSurface;
			break;
		case 1:
			ds = &previewStyle->tabTopSurface;
			break;
		case 2:
			ds = &previewStyle->activeTabBottomSurface;
			break;
		case 3:
			ds = &previewStyle->tabBottomSurface;
			break;
		case 4:
			ds = &previewStyle->btnSurface;
			break;
		case 5:
			ds = &previewStyle->scrollBarSurface;
			break;
		case 6:
			ds = &previewStyle->headerSurface;
			break;
		case 7:
			ds = &previewStyle->checkItemSurface;
			break;
		default:
			return;
	}
	
	kColorButton2_4_2_2_2_2->setColor(ds->g1Color1);
	kColorButton1_2_2_2_2_2->setColor(ds->g1Color2);
	kColorButton2_5_2_2_2_2->setColor(ds->g2Color1);
	kColorButton1_3_2_2_2_2->setColor(ds->g2Color2);
	kColorButton27_2_2->setColor(ds->background);
	spinBox1_2_2_2_2_2->setValue(ds->g1Top);
	spinBox2_2_2_2_2_2->setValue(ds->g1Bottom);
	spinBox1_3_2_2_2_2->setValue(ds->g2Top);
	spinBox2_3_2_2_2_2->setValue(ds->g2Bottom);
	switch(ds->numGradients) {
		case 2:
			groupBox8_2_2_2_2->setChecked(true);
			groupBox9_2_2_2_2->setChecked(true);
			break;
		case 1:
			groupBox8_2_2_2_2->setChecked(true);
			groupBox9_2_2_2_2->setChecked(false);
			break;
		default:
			groupBox8_2_2_2_2->setChecked(false);
			groupBox9_2_2_2_2->setChecked(false);
	}
	tabNumGradientsChanged();
}

void DominoStyleConfig::copyColorsToScrollBar() {
	
	const DSurface* ds;
	switch(comboBox3_4_2_3->currentItem()) {
		case 0:
			ds = &previewStyle->btnSurface;
			break;
		case 1:
			ds = &previewStyle->headerSurface;
			break;
		case 2:
			ds = &previewStyle->checkItemSurface;
			break;
		default:
			return;
	}
	
	kColorButton2_4_2_2_2_2_4->setColor(ds->g1Color1);
	kColorButton1_2_2_2_2_2_4->setColor(ds->g1Color2);
	kColorButton2_5_2_2_2_2_4->setColor(ds->g2Color1);
	kColorButton1_3_2_2_2_2_4->setColor(ds->g2Color2);
	kColorButton27_2_2_2->setColor(ds->background);
	spinBox1_2_2_2_2_2_4->setValue(ds->g1Top);
	spinBox2_2_2_2_2_2_4->setValue(ds->g1Bottom);
	spinBox1_3_2_2_2_2_4->setValue(ds->g2Top);
	spinBox2_3_2_2_2_2_4->setValue(ds->g2Bottom);
	switch(ds->numGradients) {
		case 2:
			groupBox8_2_2_2_2_4->setChecked(true);
			groupBox9_2_2_2_2_4->setChecked(true);
			break;
		case 1:
			groupBox8_2_2_2_2_4->setChecked(true);
			groupBox9_2_2_2_2_4->setChecked(false);
			break;
		default:
			groupBox8_2_2_2_2_4->setChecked(false);
			groupBox9_2_2_2_2_4->setChecked(false);
	}
	scrollBarNumGradientsChanged();
}

void DominoStyleConfig::copyColorsToHeader() {
	
	const DSurface* ds;
	switch(comboBox3_4_2_3_3->currentItem()) {
		case 0:
			ds = &previewStyle->btnSurface;
			break;
		case 1:
			ds = &previewStyle->scrollBarSurface;
			break;
		case 2:
			ds = &previewStyle->checkItemSurface;
			break;
		default:
			return;
	}
	
	kColorButton2_4_2_2_2_2_4_3->setColor(ds->g1Color1);
	kColorButton1_2_2_2_2_2_4_3->setColor(ds->g1Color2);
	kColorButton2_5_2_2_2_2_4_3->setColor(ds->g2Color1);
	kColorButton1_3_2_2_2_2_4_3->setColor(ds->g2Color2);
	kColorButton27_2_2_2_2->setColor(ds->background);
	spinBox1_2_2_2_2_2_4_3->setValue(ds->g1Top);
	spinBox2_2_2_2_2_2_4_3->setValue(ds->g1Bottom);
	spinBox1_3_2_2_2_2_4_3->setValue(ds->g2Top);
	spinBox2_3_2_2_2_2_4_3->setValue(ds->g2Bottom);
	switch(ds->numGradients) {
		case 2:
			groupBox8_2_2_2_2_4_3->setChecked(true);
			groupBox9_2_2_2_2_4_3->setChecked(true);
			break;
		case 1:
			groupBox8_2_2_2_2_4_3->setChecked(true);
			groupBox9_2_2_2_2_4_3->setChecked(false);
			break;
		default:
			groupBox8_2_2_2_2_4_3->setChecked(false);
			groupBox9_2_2_2_2_4_3->setChecked(false);
	}
	headerNumGradientsChanged();
}

void DominoStyleConfig::copyColorsToCheckItem() {
	
	const DSurface* ds;
	switch(comboBox3_4_2_3_3_2->currentItem()) {
		case 0:
			ds = &previewStyle->btnSurface;
			break;
		case 1:
			ds = &previewStyle->scrollBarSurface;
			break;
		case 2:
			ds = &previewStyle->headerSurface;
			break;
		default:
			return;
	}
	
	kColorButton2_4_2_2_2_2_4_3_2->setColor(ds->g1Color1);
	kColorButton1_2_2_2_2_2_4_3_2->setColor(ds->g1Color2);
	kColorButton2_5_2_2_2_2_4_3_2->setColor(ds->g2Color1);
	kColorButton1_3_2_2_2_2_4_3_2->setColor(ds->g2Color2);
	kColorButton27_2_2_2_3->setColor(ds->background);
	spinBox1_2_2_2_2_2_4_3_2->setValue(ds->g1Top);
	spinBox2_2_2_2_2_2_4_3_2->setValue(ds->g1Bottom);
	spinBox1_3_2_2_2_2_4_3_2->setValue(ds->g2Top);
	spinBox2_3_2_2_2_2_4_3_2->setValue(ds->g2Bottom);
	switch(ds->numGradients) {
		case 2:
			groupBox8_2_2_2_2_4_3_2->setChecked(true);
			groupBox9_2_2_2_2_4_3_2->setChecked(true);
			break;
		case 1:
			groupBox8_2_2_2_2_4_3_2->setChecked(true);
			groupBox9_2_2_2_2_4_3_2->setChecked(false);
			break;
		default:
			groupBox8_2_2_2_2_4_3_2->setChecked(false);
			groupBox9_2_2_2_2_4_3_2->setChecked(false);
	}
	checkItemNumGradientsChanged();
}

void DominoStyleConfig::slotLoad() {
	
	QString sName;
	if(listView_ls->selectedItem()) {
		sName = listView_ls->currentItem()->text(0);
	}
	else
		return;

	SchemeEntry * entry;
	QString fName;
	for (entry = schemeList->first(); entry; entry = schemeList->next()) {
		if(!strcmp(entry->getSchemeName(), sName)) {
			fName = entry->getFileName();
			break;
		}
	}
	
	loadConfig(fName.left(fName.length()-2));
	configLoaded = true;
}

void DominoStyleConfig::slotSave() {
	
	bool ok;
	QString cName;
	if(listView_ls->selectedItem()) {
		cName = listView_ls->currentItem()->text(0);
	}
	
	cName = KInputDialog::getText(i18n("Save Config Scheme"), i18n("Enter a name for the config scheme:"), cName, &ok, this);
	if (!ok)
		return;
	
	if (!listView_ls->findItem(cName, 0)) {
		new QListViewItem(listView_ls, cName);
		schemeList->append( new SchemeEntry("domino_" + cName.simplifyWhiteSpace().lower() + "rc", cName, true ));
	}
	else {
		int result = KMessageBox::warningContinueCancel(this,	i18n("A config scheme with the name '%1' already exists.\n"
				"Do you want to overwrite it?\n").arg(cName),
				i18n("Save Config Scheme"), i18n("Overwrite"));
		if (result == KMessageBox::Cancel)
			return;
	}
	
	cName = cName.simplifyWhiteSpace();
	saveConfig(cName, cName);
	currentConfig = cName;
	currentConfigLabel->setText(i18n("Currently loaded config: ")+currentConfig);

}

void DominoStyleConfig::slotDelete() {
	
	
	QString sName;
	if(listView_ls->selectedItem())
		sName = listView_ls->currentItem()->text(0);
	else
		return;
	
	SchemeEntry * entry;
	QString fName;
	for (entry = schemeList->first(); entry; entry = schemeList->next()) {
		if(!strcmp(entry->getSchemeName(), sName)) {
			fName = entry->getFileName();
			schemeList->remove(entry);
			break;
		}
	}
	
	delete listView_ls->currentItem();
	QDir qDir(QDir::homeDirPath() + ".qt/");
	qDir.remove(fName);
	qDir.remove("." + fName + ".lock");
}


void DominoStyleConfig::getConfigSchemes() {
	
	// only local
	QString qtHome(QDir::homeDirPath() + "/.qt");
	QSettings s;
	s.insertSearchPath( QSettings::Unix, qtHome);
	
	QDir schemeFile(qtHome, "domino_*rc");
	for(uint i = 0; i < schemeFile.count(); i++) {
		bool deletable = QFileInfo(qtHome, schemeFile[i]).isWritable();
		s.beginGroup("/" + schemeFile[i].left(schemeFile[i].length()-2) + "/Settings");
		QString origName = s.readEntry("/name", "unnamed");
		QString sName = origName;
		if(listView_ls->findItem(sName, 0)) {
			for(int i = 2; listView_ls->findItem(sName, 0); i++) {
				QString num = QString().setNum(i);
				sName = QString(origName).append(" (" + num + ")" );
			}
		}
		new QListViewItem(listView_ls, sName);
		schemeList->append( new SchemeEntry(schemeFile[i], sName, deletable ));
		
		s.endGroup();
	}
}

void DominoStyleConfig::slotDelPerm(QListViewItem* li) {
	
	if(!li)
		return;
	
	SchemeEntry * entry;
	QString selected = li->text(0);

	for ( entry = schemeList->first(); entry; entry = schemeList->next() ) {
		if(!strcmp(entry->getSchemeName(), selected)) {
			deleteButton->setEnabled(entry->isDeletable());
			return;
		}
	}
}


void DominoStyleConfig::slotImport() {
	
	KURL file ( KFileDialog::getOpenFileName(QString::null, QString::null, this) );
	if ( file.isEmpty() )
		return;
	
	QString sFile = file.fileName( false );
	
	KSimpleConfig *config = new KSimpleConfig(file.directory(false)+sFile);
	config->setGroup("Settings");
	QString cName = config->readEntry("name", i18n("unnamed"));
	delete config;
	
	QString fName = "domino_" + cName.simplifyWhiteSpace().lower() + "rc";
	
	if (!listView_ls->findItem(cName, 0)) {
		new QListViewItem(listView_ls, cName);
		schemeList->append( new SchemeEntry(fName, cName, true ));
	}
	else {
		int result = KMessageBox::warningContinueCancel(this,	i18n("A config scheme with the name '%1' already exists.\n"
				"Do you want to overwrite it?\n").arg(cName),
				i18n("Save Config Scheme"), i18n("Overwrite"));
		if (result == KMessageBox::Cancel)
			return;
	}
	
	QString saveDir = QDir::homeDirPath() + "/.qt/";
	if (!KIO::NetAccess::file_copy(file, KURL( saveDir+fName ), -1, true ) )
	{
		KMessageBox::error(this, KIO::NetAccess::lastErrorString(),i18n("Import failed."));
		return;
	}
}






void DominoStyleConfig::scrollBar1GradientChanged() {
	scrollBarNumGradientsChanged(1);
}

void DominoStyleConfig::scrollBar2GradientChanged() {
	scrollBarNumGradientsChanged(2);
}

void DominoStyleConfig::scrollBarNumGradientsChanged(int g) {
	
	switch(g) {
		case 2:
			if(groupBox9_2_2_2_2_4->isChecked() && !groupBox8_2_2_2_2_4->isChecked())
				groupBox8_2_2_2_2_4->setChecked(true);
			break;
		case 1:
			if(groupBox9_2_2_2_2_4->isChecked())
				groupBox9_2_2_2_2_4->setChecked(false);
		default:
			break;
	}
	
	int c = groupBox9_2_2_2_2_4->isChecked() ? 2 : groupBox8_2_2_2_2_4->isChecked() ? 1 : 0;
	currentScrollBarSurface->numGradients = c;
	
	previewStyle->removeCachedSbPix(scrollBar5->palette().active().button());
	scrollBar5->repaint(false);
	scrollBar6->repaint(false);
}

void DominoStyleConfig::btn1GradientChanged() {
	btnNumGradientsChanged(1);
}

void DominoStyleConfig::btn2GradientChanged() {
	btnNumGradientsChanged(2);
}

void DominoStyleConfig::btnNumGradientsChanged(int g) {
	
	switch(g) {
		case 2:
			if(groupBox9_2_2_2->isChecked() && !groupBox8_2_2_2->isChecked())
				groupBox8_2_2_2->setChecked(true);
			break;
		case 1:
			if(groupBox9_2_2_2->isChecked())
				groupBox9_2_2_2->setChecked(false);
		default:
			break;
	}
	
	int c = groupBox9_2_2_2->isChecked() ? 2 : groupBox8_2_2_2->isChecked() ? 1 : 0;
	previewStyle->btnSurface.numGradients = c;
	
	spinBox44->spinWidget()->repaint(false);
	comboBox17->repaint(false);
	pushButton16->repaint(false);
}

void DominoStyleConfig::header1GradientChanged() {
	headerNumGradientsChanged(1);
}

void DominoStyleConfig::header2GradientChanged() {
	headerNumGradientsChanged(2);
}

void DominoStyleConfig::headerNumGradientsChanged(int g) {
	
	switch(g) {
		case 2:
			if(groupBox9_2_2_2_2_4_3->isChecked() && !groupBox8_2_2_2_2_4_3->isChecked())
				groupBox8_2_2_2_2_4_3->setChecked(true);
			break;
		case 1:
			if(groupBox9_2_2_2_2_4_3->isChecked())
				groupBox9_2_2_2_2_4_3->setChecked(false);
		default:
			break;
	}
	
	int c = groupBox9_2_2_2_2_4_3->isChecked() ? 2 : groupBox8_2_2_2_2_4_3->isChecked() ? 1 : 0;
	previewStyle->headerSurface.numGradients = c;
	
	header1->setBackgroundMode(Qt::PaletteBackground);
	header1->repaint(false);
}

void DominoStyleConfig::checkItem1GradientChanged() {
	checkItemNumGradientsChanged(1);
}

void DominoStyleConfig::checkItem2GradientChanged() {
	checkItemNumGradientsChanged(2);
}

void DominoStyleConfig::checkItemNumGradientsChanged(int g) {
	
	switch(g) {
		case 2:
			if(groupBox9_2_2_2_2_4_3_2->isChecked() && !groupBox8_2_2_2_2_4_3_2->isChecked())
				groupBox8_2_2_2_2_4_3_2->setChecked(true);
			break;
		case 1:
			if(groupBox9_2_2_2_2_4_3_2->isChecked())
				groupBox9_2_2_2_2_4_3_2->setChecked(false);
		default:
			break;
	}
	
	int c = groupBox9_2_2_2_2_4_3_2->isChecked() ? 2 : groupBox8_2_2_2_2_4_3_2->isChecked() ? 1 : 0;
	previewStyle->checkItemSurface.numGradients = c;
	
	checkBox2->repaint(false);
	radioButton2->repaint(false);
}

void DominoStyleConfig::tab1GradientChanged() {
	tabNumGradientsChanged(1);
}

void DominoStyleConfig::tab2GradientChanged() {
	tabNumGradientsChanged(2);
}

void DominoStyleConfig::tabNumGradientsChanged(int g) {
	
	switch(g) {
		case 2:
			if(groupBox9_2_2_2_2->isChecked() && !groupBox8_2_2_2_2->isChecked())
				groupBox8_2_2_2_2->setChecked(true);
			break;
		case 1:
			if(groupBox9_2_2_2_2->isChecked())
				groupBox9_2_2_2_2->setChecked(false);
		default:
			break;
	}
	int c = groupBox9_2_2_2_2->isChecked() ? 2 : groupBox8_2_2_2_2->isChecked() ? 1 : 0;
	currentTabSurface->numGradients = c;
	
	tabBarTop->repaint(false);
	tabBarBottom->repaint(false);
}


// Accept color drops on tabs.
void DominoStyleConfig::testCanDecode(const QDragMoveEvent * e, bool & accept) {
	accept = e->provides("application/x-color");
}

void DominoStyleConfig::resetGroupBoxBackgroundColor() {
	QColor bg = qApp->palette().active().background();
	gb1->setPaletteBackgroundColor(bg);
	gb2->setPaletteBackgroundColor(bg);
	gb3->setPaletteBackgroundColor(bg);
}

void DominoStyleConfig::textEffectModeChanged() {
	bool enable = (bool)textEffectModeCombo->currentItem();
	textEffectPrevWidget->setEnabled(enable);
	textEffectColor->setEnabled(enable);
	textEffectOpacity->setEnabled(enable);
	textEffectPos->setEnabled(enable);
	
}
void DominoStyleConfig::indicatorModeChanged() {
	// bool 0=underline, 1=outline
	bool enable = indicatorModeCombo->currentItem() ? false : true;
	indicatorBtnColor->setEnabled(enable);
	indicatorBtnOpacity->setEnabled(enable);
	indicatorColor->setEnabled(enable);
	indicatorOpacity->setEnabled(enable);
	
}


void TabWidgetIndicator::paintEvent(QPaintEvent*) {
	
	// index: bottom = 1, top = 0
	bool topTabSelected = tabPosCombo->currentItem() == 0;
	// index: inactive = 1, active = 0
	bool activeTabSelected = tabStateCombo->currentItem() == 0;
	DominoKTabPrevWidget* tw = (DominoKTabPrevWidget*) (topTabSelected ? prevTopTabWidget : prevBottomTabWidget);
	QWidget* w = tw->currentPage();
	QTabBar* tabBar = tw->tabBar();
	QRect pr = w->rect();
	QRect tr1;
	QRect tr2;
	
	QPoint point = w->mapToParent(QPoint(0,0));
	
	QPixmap arrow_tmp = tintImage( topTabSelected ? config_findImage("tabIndicatorArrow") : config_findImage("tabIndicatorArrow").mirror(), qApp->palette().active().foreground());
	QPixmap arrow(arrow_tmp.size());
	arrow.fill(paletteBackgroundColor());
	bitBlt(&arrow, 0, 0, &arrow_tmp);
	
	QPainter p(w);
	if(activeTabSelected) {
		tr1 = tabBar->tabAt(tw->currentPageIndex())->rect();
		tr1.moveBy(-point.x()+tabBar->geometry().x(), 0);
		if(topTabSelected) {
			p.drawPixmap(tr1.x()+tr1.width()/2-arrow.width()/2, tr1.y()+tr1.height()/2-arrow.height()/2, arrow);
		}
		else {
			p.drawPixmap(tr1.x()+tr1.width()/2-arrow.width()/2, tr1.y()+tr1.height()/2-arrow.height()/2, arrow);
		}
	}
	else {
		switch(tabBar->currentTab()) {
			case 0:
				tr1 = tabBar->tabAt(1)->rect();
				tr2 = tabBar->tabAt(2)->rect();
				break;
			case 1:
				tr1 = tabBar->tabAt(0)->rect();
				tr2 = tabBar->tabAt(2)->rect();
				break;
			case 2:
				tr1 = tabBar->tabAt(0)->rect();
				tr2 = tabBar->tabAt(1)->rect();
				break;
			default:
				break;
		}

		tr1.moveBy(-point.x()+tabBar->geometry().x(), 0);
		tr2.moveBy(-point.x()+tabBar->geometry().x(), 0);
		p.drawPixmap(tr1.x()+tr1.width()/2-arrow.width()/2, tr1.y()+tr1.height()/2-arrow.height()/2, arrow);
		p.drawPixmap(tr2.x()+tr2.width()/2-arrow.width()/2, tr2.y()+tr2.height()/2-arrow.height()/2, arrow);
	}
	
}

void DominoStyleConfig::grValueLabelNum(int num) {
	grValueLabel->setNum(num <= 0 ? QABS(num) : -num);
}


void DominoStyleConfig::updateTextEffectSettings() {
	
	QPoint pos;
	switch(textEffectPos->currentItem()) {
		case 0:
			pos = QPoint(-1,-1);
			break;
		case 1:
			pos = QPoint(0,-1);
			break;
		case 2:
			pos = QPoint(1,-1);
			break;
		case 3:
			pos= QPoint(1,0);
			break;
		case 4:
			pos = QPoint(1,1);
			break;
		case 5:
			pos = QPoint(0,1);
			break;
		case 6:
			pos = QPoint(-1,1);
			break;
		case 7:
			pos = QPoint(-1,0);
			break;
		default:
			pos = QPoint(0,1);
	}
	
	previewStyle->textEffectSettings.pos = pos;
	previewStyle->textEffectSettings.color = textEffectColor->color();
	previewStyle->textEffectSettings.opacity = textEffectOpacity->value()*255/100;
	previewStyle->textEffectSettings.mode = (TextEffect)(textEffectModeCombo->currentItem()+1);
	
	
	switch(textEffectButtonPos->currentItem()) {
		case 0:
			pos = QPoint(-1,-1);
			break;
		case 1:
			pos = QPoint(0,-1);
			break;
		case 2:
			pos = QPoint(1,-1);
			break;
		case 3:
			pos= QPoint(1,0);
			break;
		case 4:
			pos = QPoint(1,1);
			break;
		case 5:
			pos = QPoint(0,1);
			break;
		case 6:
			pos = QPoint(-1,1);
			break;
		case 7:
			pos = QPoint(-1,0);
			break;
		default:
			pos = QPoint(0,1);
	}
	
	previewStyle->textEffectSettings.buttonPos = pos;
	previewStyle->textEffectSettings.buttonColor = textEffectButtonColor->color();
	previewStyle->textEffectSettings.buttonOpacity = textEffectButtonOpacity->value()*255/100;
	
	textEffectPrevWidget->update();
	textEffectPrevButton->update();
	
	indicatorPrevButton->update();
	indicatorPrevWidget->update();
	
}

void DominoStyleConfig::updateFocusIndicatorSettings() {
	
	previewStyle->focusIndicatorSettings.color = indicatorColor->color();
	previewStyle->focusIndicatorSettings.opacity = indicatorOpacity->value()*255/100;
	previewStyle->focusIndicatorSettings.buttonColor = indicatorBtnColor->color();
	previewStyle->focusIndicatorSettings.buttonOpacity = indicatorBtnOpacity->value()*255/100;
	// 0=underline, 1=outline
	previewStyle->focusIndicatorSettings.drawUnderline = indicatorModeCombo->currentItem() == 1;
	previewStyle->focusIndicatorSettings.indicateFocus = indicateFocus->isChecked();
	
	indicatorPrevButton->update();
	indicatorPrevWidget->update();
}




void DominoStyleConfig::updateButtonContourSettings() {
	
	
	delete previewStyle->buttonContour;
	previewStyle->buttonContour = new ButtonContour();
	
	previewStyle->buttonContour->setDefaultType((ContourType)buttonContourType->currentItem(), false);
	previewStyle->buttonContour->setColor(Contour_Default, buttonContourColor->color());
	previewStyle->buttonContour->setColor(Contour_Pressed, buttonPressedContourColor->color());
	previewStyle->buttonContour->setColor(Contour_MouseOver, buttonMouseOverContourColor->color());
	previewStyle->buttonContour->setColor(Contour_DefaultButton, buttonDefaultButtonContourColor->color());
	previewStyle->buttonContour->drawButtonSunkenShadow = drawButtonSunkenShadow->isChecked();
	previewStyle->buttonContour->setDefaultType((ContourType)buttonContourType->currentItem(), true);
	
	// update all preview widgets with a button contour
	textEffectPrevButton->update();
	indicatorPrevButton->update();
	buttonContourPrevButton->update();
	previewStyle->removeCachedSbPix(scrollBar5->palette().active().button());
	header1->setBackgroundMode(Qt::PaletteBackground);
	
	
}

void DominoStyleConfig::updateCheckItemIndicators() {
	
	QColor color = customCheckMarkColor->isChecked() ? checkMarkColor->color() : qApp->palette().active().foreground();
	if(color == previewStyle->_checkMarkColor)
		return;
	
	if(previewStyle->checkMark)
		delete previewStyle->checkMark;
	if(previewStyle->radioIndicator)
		delete previewStyle->radioIndicator;

	previewStyle->_checkMarkColor = color;
	previewStyle->checkMark = previewStyle->createCheckMark(color);
	previewStyle->radioIndicator = previewStyle->createRadioIndicator(color);
}

void DominoStyleConfig::updateGroupBoxSettings() {
	
	previewStyle->groupBoxSettings.customColor = customGroupBoxBackgroundColor->isChecked();
	previewStyle->groupBoxSettings.brightness = grSlider->value();
	previewStyle->groupBoxSettings.tintBackground = tintGroupBoxBackground->isChecked();
	previewStyle->groupBoxSettings.color = groupBoxBackgroundColor->color();
	
	gb1->setPaletteBackgroundColor(previewStyle->getGroupBoxColor(gb1));
	gb1->update();
	if(customGroupBoxBackgroundColor->isChecked()) // gb1 has a static color
		gb2->setPaletteBackgroundColor(previewStyle->getGroupBoxColor(gb2));
	gb2->update();
	gb3->update();
}


void DominoStyleConfig::rubberSelChanged(int) {
	rubberBandType = argbRubber->isChecked() ? 1 : lineRubber->isChecked() ? 2 : 3;
}

void DominoStyleConfig::updateArgbRubberOptions(bool enabled) {
	if(!hasCompositeExtension)
		return;
	
	labelArgbRubberColor->setEnabled(enabled);
	argbRubberColor->setEnabled(enabled);
	labelArgbRubberOpacity->setEnabled(enabled);
	argbRubberOpacity->setEnabled(enabled);
}


#include "dominoconf.moc"
