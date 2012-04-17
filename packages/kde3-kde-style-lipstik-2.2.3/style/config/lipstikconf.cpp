/*
Copyright (C) 2004-2007 Patrice Tremblay <tremblaypatrice@yahoo.fr>
			http://www.poplix.info/lipstik

based on plastik:
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

#include <qcheckbox.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qsettings.h>
#include <qcolor.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <kglobal.h>
#include <klocale.h>
#include <kcolorbutton.h>
#include <qtabwidget.h>
#include <qvgroupbox.h>
#include <qspinbox.h>
#include <qgrid.h>

#include "lipstikconf.h"

extern "C"
{
	QWidget* allocate_kstyle_config(QWidget* parent)
	{
		KGlobal::locale()->insertCatalogue("kstyle_lipstik_config");
		return new LipstikStyleConfig(parent);
	}
}

LipstikStyleConfig::LipstikStyleConfig(QWidget* parent): QWidget(parent)
{
	QVBoxLayout* layout = new QVBoxLayout(this, 0, 0);
	QGridLayout* tabLayout = new QGridLayout(this, 2, 1, 10);

	KGlobal::locale()->insertCatalogue("kstyle_lipstik_config");

	///Init tabs
	QTabWidget *tabWidget = new QTabWidget(this);
	layout->addWidget(tabWidget);

	///Init tab1 
	QWidget *page1 = new QWidget(tabWidget);
        layout = new QVBoxLayout(page1);
	tabWidget->insertTab(page1, i18n("Configuration"));
	layout->addLayout(tabLayout);	

	///General group
	QVGroupBox *generalGrp = new QVGroupBox(i18n("General"), page1);

	reverseGradients = new QCheckBox(i18n("Plastik gradients"), generalGrp );
	drawTriangularExpander = new QCheckBox(i18n("Triangular tree expander"), generalGrp);
	drawStatusBarFrame = new QCheckBox(i18n("Draw status bar frame"), generalGrp);
	animateProgressBar = new QCheckBox(i18n("Animate progress bars"), generalGrp);
	flatHeaders = new QCheckBox(i18n("Flatten column headers"), generalGrp);
	tickCheckMark = new QCheckBox(i18n("Use a tick for checkmarks"), generalGrp);

	tabLayout->addWidget(generalGrp,0,0,0);

	///Toolbar group
	QVGroupBox *toolbarGrp = new QVGroupBox(i18n("Toolbars"), page1);
	QLabel* toolBarSpacingLabel;

	drawToolBarSeparator = new QCheckBox(i18n("Draw separators"), toolbarGrp);
	drawToolBarItemSeparator = new QCheckBox(i18n("Draw item separators"), toolbarGrp);
	drawToolBarHandle = new QCheckBox(i18n("Draw handles"), toolbarGrp);
	drawToolBarGradient = new QCheckBox(i18n("Draw gradients"), toolbarGrp);
	invertBarGrad = new QCheckBox(i18n("Invert gradients"), toolbarGrp);

        toolBarSpacingLabel = new QLabel(toolbarGrp);
        toolBarSpacingLabel->setText(i18n("Items spacing :"));
	toolBarSpacing = new QSpinBox ( 0, 6, 1, toolbarGrp );

	tabLayout->addWidget(toolbarGrp,0,1,0);	

	///Menus group
	QVGroupBox *menusGrp = new QVGroupBox(i18n("Menus"), page1);
	QLabel* menuSpacingLabel;

	alterMenuGradients = new QCheckBox(i18n("Button-like menu gradients"), menusGrp);
	sharperMenuGradient = new QCheckBox(i18n("Sharper menu gradients"), menusGrp);

	menuLeftSideBar = new QCheckBox(i18n("Menu Stripe"), menusGrp);
	flatStripe = new QCheckBox(i18n("Flatten Stripe"), menusGrp);
	customMenuStripeColor = new QCheckBox(i18n("Custom Stripe color :"), menusGrp);
	QHBox *menuColorBox = new QHBox(menusGrp);
	menuColorBox->layout()->addItem(new QSpacerItem(20, 0, QSizePolicy::Fixed, QSizePolicy::Minimum) );

	menuStripeColor = new KColorButton(menuColorBox);

        menuSpacingLabel = new QLabel(menusGrp);
        menuSpacingLabel->setText(i18n("Menu spacing :"));
	menuSpacing = new QSpinBox ( 0, 8, 1, menusGrp );

	tabLayout->addWidget(menusGrp,1,0,0);

	///Scrollbar group
	QVGroupBox *scrollGrp = new QVGroupBox(i18n("Scrollbars and Sliders"), page1);
	QLabel* scrollBarStyleLabel;
	QLabel* scrollBarWidthLabel;
	QLabel* sliderStyleLabel;

	paintGroove = new QCheckBox(i18n("Draw Scrollbar grooves"), scrollGrp);

        scrollBarStyleLabel = new QLabel(scrollGrp);
        scrollBarStyleLabel->setText(i18n("Scrollbar style :       "));
	scrollBarStyle = new QComboBox(i18n("Scrollbar style"), scrollGrp);
	scrollBarStyle->setEditable(false);
	scrollBarStyle->insertItem("Windows style");
	scrollBarStyle->insertItem("Platinum style");
	scrollBarStyle->insertItem("KDE style");
	scrollBarStyle->insertItem("Next style");

        scrollBarWidthLabel = new QLabel(scrollGrp);
        scrollBarWidthLabel->setText(i18n("Scrollbar width :       "));
	scrollBarWidth = new QComboBox(i18n("Scrollbar width"), scrollGrp);
	scrollBarWidth->setEditable(false);
	scrollBarWidth->insertItem("Small");
	scrollBarWidth->insertItem("Normal");
	scrollBarWidth->insertItem("Large");

        sliderStyleLabel = new QLabel(scrollGrp);
        sliderStyleLabel->setText(i18n("Slider handle size :       "));
	sliderStyle = new QComboBox(i18n("Slider handle size"), scrollGrp);
	sliderStyle->setEditable(false);
	sliderStyle->insertItem("Small");
	sliderStyle->insertItem("Medium");
	sliderStyle->insertItem("Large");

	tabLayout->addWidget(scrollGrp,1,1,0);	

	///Highlighting group
	QVGroupBox *lightGrp = new QVGroupBox(i18n("Highlighting"), page1);

	drawFocusRect = new QCheckBox(i18n("Colored focus rectangle"), lightGrp);
	comboboxColored = new QCheckBox(i18n("Colored comboboxes handles"), lightGrp);
	coloredMenuBar = new QCheckBox(i18n("Colored menu bar items"), lightGrp);
	drawTabHighlight = new QCheckBox(i18n("Highlight active tab"), lightGrp);
	inputFocusHighlight = new QCheckBox(i18n("Highlight focused input fields"), lightGrp);

	tabLayout->addWidget(lightGrp,0,2,0);	

	///Other colors group
	QVGroupBox *otherGrp = new QVGroupBox(i18n("Custom colors"), page1);

        customFocusHighlightColor = new QCheckBox(i18n("Input fields highlight :"), otherGrp);
	QHBox *hbox1 = new QHBox(otherGrp);
	hbox1->layout()->addItem(new QSpacerItem(20, 0, QSizePolicy::Fixed, QSizePolicy::Minimum) );
	focusHighlightColor = new KColorButton(hbox1);

	customOverHighlightColor = new QCheckBox(i18n("Mouse hover highlight :"), otherGrp);
	QHBox *hbox2 = new QHBox(otherGrp);
	hbox2->layout()->addItem(new QSpacerItem(20, 0, QSizePolicy::Fixed, QSizePolicy::Minimum) );
	overHighlightColor = new KColorButton(hbox2);

	customCheckMarkColor = new QCheckBox(i18n("Checkmark/Radiobuttons :"), otherGrp);
	QHBox *hbox3 = new QHBox(otherGrp);
	hbox3->layout()->addItem(new QSpacerItem(20, 0, QSizePolicy::Fixed, QSizePolicy::Minimum) );
	checkMarkColor = new KColorButton(hbox3);

	customTabHighlightColor = new QCheckBox(i18n("Active tab highlight :"), otherGrp);
	QHBox *hbox4 = new QHBox(otherGrp);
	hbox4->layout()->addItem(new QSpacerItem(20, 0, QSizePolicy::Fixed, QSizePolicy::Minimum) );
	tabHighlightColor = new KColorButton(hbox4);

	tabLayout->addWidget(otherGrp,1,2,0);	

	///Init tab2
	QWidget *page2 = new QWidget(tabWidget);
	QLabel* lipstikLabel;
	QLabel* authorLabel;
	QLabel* authorAddressLabel;
	QLabel* authorWebLabel;
	QLabel* plastikLabel;
	QLabel* alsoLabel;
	QLabel* cuLabel;
	QLabel* dnLabel;
	QLabel* pqLabel;
	QLabel* cxLabel;
	QLabel* kpLabel;
	QLabel* blLabel;
	QLabel* ksLabel;
	QLabel* peLabel;
	QLabel* bastianLabel;
	QLabel* pcbsdLabel;
	QLabel* othersLabel;

        layout = new QVBoxLayout(page2, 10, -1);
	    
	tabWidget->insertTab(page2, i18n("About"));

	lipstikLabel = new QLabel(page2);
        lipstikLabel->setText(i18n("Lipstik 2.2.3"));
	lipstikLabel->setFrameStyle( QFrame::StyledPanel | QFrame::Raised );

	authorLabel = new QLabel(page2);
        authorLabel->setText(i18n("     Copyright 2004-2005-2006-2007 (C) Patrice Tremblay (Poplix)"));

	authorAddressLabel = new QLabel(page2);
        authorAddressLabel->setText(i18n("      poplixos@gmail.com"));

	authorWebLabel = new QLabel(page2);
        authorWebLabel->setText(i18n("      http://poplix.homelinux.com/lipstik"));

	plastikLabel = new QLabel(page2);
        plastikLabel->setText(i18n("Based on plastik [thanks to Sandro Giessl]"));

	alsoLabel = new QLabel(page2);
        alsoLabel->setText(i18n("Also based on :"));

	cuLabel = new QLabel(page2);
        cuLabel->setText(i18n("    QtCurve [thanks to Craig Drummond],"));

	dnLabel = new QLabel(page2);
        dnLabel->setText(i18n("    DotNET [thanks to Chris Lee],"));

	pqLabel = new QLabel(page2);
        pqLabel->setText(i18n("    PlastikQ [thanks to Martin Beck],"));

	cxLabel = new QLabel(page2);
        cxLabel->setText(i18n("    Comix [thanks to Jens Luetkens],"));

	kpLabel = new QLabel(page2);
        kpLabel->setText(i18n("    Krisp [thanks to Keith Meehl],"));

	blLabel = new QLabel(page2);
        blLabel->setText(i18n("    Ballistik [thanks to Florian Merz],"));

	ksLabel = new QLabel(page2);
        ksLabel->setText(i18n("    Kerastik [thanks to Victor Perez Q],"));

	peLabel = new QLabel(page2);
        peLabel->setText(i18n("    Polyester [thanks to Marco Martin]."));

	bastianLabel = new QLabel(page2);
        bastianLabel->setText(i18n("Thanks to Bastian Venthur, the Debian maintainer of kde-style-lipstik."));

	pcbsdLabel = new QLabel(page2);
        pcbsdLabel->setText(i18n("Thanks to Charles A. Landemaine, of PC-BSD, for the feature suggestion."));

	othersLabel = new QLabel(page2);
        othersLabel->setText(i18n("Special thanks to Dominique and all the kde-look.org users."));

	layout->add(lipstikLabel);
	layout->add(authorLabel);
	layout->add(authorAddressLabel);
	layout->add(authorWebLabel);

	layout->addSpacing(10);
	layout->add(plastikLabel);

	layout->addSpacing(10);
	layout->add(alsoLabel);
	layout->add(cuLabel);
	layout->add(dnLabel);
	layout->add(pqLabel);
	layout->add(cxLabel);
	layout->add(kpLabel);
	layout->add(blLabel);
	layout->add(ksLabel);
	layout->add(peLabel);

	layout->addSpacing(10);
	layout->add(bastianLabel);
	layout->add(pcbsdLabel);
	layout->add(othersLabel);

	///Populate!
	QSettings s;

	origReverseGradients = s.readBoolEntry("/lipstikstyle/Settings/reverseGradients", false);
	reverseGradients->setChecked(origReverseGradients);

	origPaintGroove = s.readBoolEntry("/lipstikstyle/Settings/paintGroove", true);
	paintGroove->setChecked(origPaintGroove);

	origSharperMenuGradient = s.readBoolEntry("/lipstikstyle/Settings/sharperMenuGradient", false);
	sharperMenuGradient->setChecked(origSharperMenuGradient);

	origAlterMenuGradients = s.readBoolEntry("/lipstikstyle/Settings/alterMenuGradients", true);
	alterMenuGradients->setChecked(origAlterMenuGradients);

	origMenuLeftSideBar = s.readBoolEntry("/lipstikstyle/Settings/menuLeftSideBar", true);
	menuLeftSideBar->setChecked(origMenuLeftSideBar);

	origFlatStripe = s.readBoolEntry("/lipstikstyle/Settings/flatStripe", false);
	flatStripe->setChecked(origFlatStripe);

	origComboboxColored = s.readBoolEntry("/lipstikstyle/Settings/comboboxColored", false);
	comboboxColored->setChecked(origComboboxColored);

	origColoredMenuBar = s.readBoolEntry("/lipstikstyle/Settings/coloredMenuBar", false);
	coloredMenuBar->setChecked(origColoredMenuBar);

	origDrawTabHighlight = s.readBoolEntry("/lipstikstyle/Settings/drawTabHighlight", true);
	drawTabHighlight->setChecked(origDrawTabHighlight);

	origMenuSpacing = s.readNumEntry("/lipstikstyle/Settings/menuSpacing", 5);
	menuSpacing->setValue(origMenuSpacing);

	origToolBarSpacing = s.readNumEntry("/lipstikstyle/Settings/toolBarSpacing", 0);
	toolBarSpacing->setValue(origToolBarSpacing);

	origScrollBarStyle = s.readEntry("/lipstikstyle/Settings/scrollBarStyle", "WindowsStyleScrollBar");
	scrollBarStyle->setCurrentItem(scrollBarItem(origScrollBarStyle));

	origScrollBarWidth = s.readEntry("/lipstikstyle/Settings/scrollBarWidth", "Small");
	scrollBarWidth->setCurrentItem(scrollBarWidthItem(origScrollBarWidth));

	origSliderStyle = s.readEntry("/lipstikstyle/Settings/sliderStyle", "sliderLipstikStyle");
	sliderStyle->setCurrentItem(sliderItem(origSliderStyle));

	origDrawStatusBarFrame = s.readBoolEntry("/lipstikstyle/Settings/drawStatusBarFrame", false);
	drawStatusBarFrame->setChecked(origDrawStatusBarFrame);

	origDrawToolBarHandle = s.readBoolEntry("/lipstikstyle/Settings/drawToolBarHandle", true);
	drawToolBarHandle->setChecked(origDrawToolBarHandle);

	origAnimProgressBar = s.readBoolEntry("/lipstikstyle/Settings/animateProgressBar", false);
	animateProgressBar->setChecked(origAnimProgressBar);

	origDrawToolBarSeparator = s.readBoolEntry("/lipstikstyle/Settings/drawToolBarSeparator", true);
	drawToolBarSeparator->setChecked(origDrawToolBarSeparator);

	origDrawToolBarGradient = s.readBoolEntry("/lipstikstyle/Settings/drawToolBarGradient", false);
	drawToolBarGradient->setChecked(origDrawToolBarGradient);

	origInvertBarGrad = s.readBoolEntry("/lipstikstyle/Settings/invertBarGrad", true);
	invertBarGrad->setChecked(origInvertBarGrad);

	origDrawToolBarItemSeparator = s.readBoolEntry("/lipstikstyle/Settings/drawToolBarItemSeparator", true);
	drawToolBarItemSeparator->setChecked(origDrawToolBarItemSeparator);

 	origDrawFocusRect = s.readBoolEntry("/lipstikstyle/Settings/drawFocusRect", true);
	drawFocusRect->setChecked(origDrawFocusRect);

 	origFlatHeaders = s.readBoolEntry("/lipstikstyle/Settings/flatHeaders", true);
	flatHeaders->setChecked(origFlatHeaders);

 	origTickCheckMark = s.readBoolEntry("/lipstikstyle/Settings/tickCheckMark", true);
	tickCheckMark->setChecked(origTickCheckMark);

	origDrawTriangularExpander = s.readBoolEntry("/lipstikstyle/Settings/drawTriangularExpander", false);
	drawTriangularExpander->setChecked(origDrawTriangularExpander);

	origInputFocusHighlight = s.readBoolEntry("/lipstikstyle/Settings/inputFocusHighlight", true);
	inputFocusHighlight->setChecked(origInputFocusHighlight);

	origCustomOverHighlightColor = s.readBoolEntry("/lipstikstyle/Settings/customOverHighlightColor", false);
	customOverHighlightColor->setChecked(origCustomOverHighlightColor);

	origOverHighlightColor = s.readEntry("/lipstikstyle/Settings/overHighlightColor", "/Qt/KWinPalette/activeBackground");
	overHighlightColor->setColor(origOverHighlightColor);

	origCustomMenuStripeColor = s.readBoolEntry("/lipstikstyle/Settings/customMenuStripeColor", false);
	customMenuStripeColor->setChecked(origCustomMenuStripeColor);

	origMenuStripeColor = s.readEntry("/lipstikstyle/Settings/menuStripeColor", "/Qt/KWinPalette/activeBackground");
	menuStripeColor->setColor(origMenuStripeColor);

	origCustomFocusHighlightColor = s.readBoolEntry("/lipstikstyle/Settings/customFocusHighlightColor", false);
	customFocusHighlightColor->setChecked(origCustomFocusHighlightColor);

	origFocusHighlightColor = s.readEntry("/lipstikstyle/Settings/focusHighlightColor", "/Qt/KWinPalette/activeBackground");
	focusHighlightColor->setColor(origFocusHighlightColor);

	origCustomCheckMarkColor = s.readBoolEntry("/lipstikstyle/Settings/customCheckMarkColor", false);
	customCheckMarkColor->setChecked(origCustomCheckMarkColor);

	origCheckMarkColor = s.readEntry("/lipstikstyle/Settings/checkMarkColor", "/Qt/KWinPalette/activeBackground");
	checkMarkColor->setColor(origCheckMarkColor);

	origCustomTabHighlightColor = s.readBoolEntry("/lipstikstyle/Settings/customTabHighlightColor", false);
	customTabHighlightColor->setChecked(origCustomTabHighlightColor);

	origTabHighlightColor = s.readEntry("/lipstikstyle/Settings/tabHighlightColor", "/Qt/KWinPalette/activeBackground");
	tabHighlightColor->setColor(origTabHighlightColor);

	connect(reverseGradients, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(paintGroove, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(sharperMenuGradient, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(alterMenuGradients, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(menuLeftSideBar, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(flatStripe, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(drawStatusBarFrame, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(drawToolBarHandle, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(comboboxColored, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(coloredMenuBar, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(drawTabHighlight, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(menuSpacing, SIGNAL( valueChanged(int) ), SLOT( updateChanged() ) );
	connect(toolBarSpacing, SIGNAL( valueChanged(int) ), SLOT( updateChanged() ) );
	connect(scrollBarStyle, SIGNAL( activated(int) ), SLOT( updateChanged() ) );
	connect(scrollBarWidth, SIGNAL( activated(int) ), SLOT( updateChanged() ) );
	connect(sliderStyle, SIGNAL( activated(int) ), SLOT( updateChanged() ) );
	connect(animateProgressBar, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(drawToolBarSeparator, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(drawToolBarGradient, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(invertBarGrad, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(drawToolBarItemSeparator, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
 	connect(drawFocusRect, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
 	connect(flatHeaders, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
 	connect(tickCheckMark, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(drawTriangularExpander, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(inputFocusHighlight, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(customOverHighlightColor, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(overHighlightColor, SIGNAL( pressed() ), SLOT( updateChanged() ) );
	connect(customMenuStripeColor, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(menuStripeColor, SIGNAL( pressed() ), SLOT( updateChanged() ) );
	connect(customFocusHighlightColor, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(focusHighlightColor, SIGNAL( pressed() ), SLOT( updateChanged() ) );
	connect(customCheckMarkColor, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(checkMarkColor, SIGNAL( pressed() ), SLOT( updateChanged() ) );
	connect(customTabHighlightColor, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(tabHighlightColor, SIGNAL( pressed() ), SLOT( updateChanged() ) );

	//Menu Stripe 
	if ( menuLeftSideBar->isChecked() ) {
	    customMenuStripeColor->setEnabled(true);
	    flatStripe->setEnabled(true);
	} else {
	    customMenuStripeColor->setEnabled(false);
	    flatStripe->setEnabled(false);
	}

	//Toolbar gradients.
	if ( drawToolBarGradient->isChecked() ) {
	    invertBarGrad->setEnabled(true);
	} else {
	    invertBarGrad->setEnabled(false);
	}

	if ( customMenuStripeColor->isChecked() && customMenuStripeColor->isEnabled())
	 menuStripeColor->setEnabled(true);
	else
	 menuStripeColor->setEnabled(false);

	//Input Focus 
	if ( inputFocusHighlight->isChecked() )
	    customFocusHighlightColor->setEnabled(true);
	else
	    customFocusHighlightColor->setEnabled(false);

	if ( customFocusHighlightColor->isChecked() && customFocusHighlightColor->isEnabled())
	 focusHighlightColor->setEnabled(true);
	else
	 focusHighlightColor->setEnabled(false);

	//Custom checkmark color.
	if ( customCheckMarkColor->isChecked() )
	 checkMarkColor->setEnabled(true);
	else
	 checkMarkColor->setEnabled(false);

	//Custom tab highlight color.
	if ( customTabHighlightColor->isChecked() )
	 tabHighlightColor->setEnabled(true);
	else
	 tabHighlightColor->setEnabled(false);

	if ( customOverHighlightColor->isChecked() )
	 overHighlightColor->setEnabled(true);
	else
	 overHighlightColor->setEnabled(false);
}

LipstikStyleConfig::~LipstikStyleConfig()
{
}


void LipstikStyleConfig::save()
{
	QSettings s;
	s.writeEntry("/lipstikstyle/Settings/reverseGradients", reverseGradients->isChecked());
	s.writeEntry("/lipstikstyle/Settings/paintGroove", paintGroove->isChecked());
	s.writeEntry("/lipstikstyle/Settings/sharperMenuGradient", sharperMenuGradient->isChecked());
	s.writeEntry("/lipstikstyle/Settings/alterMenuGradients", alterMenuGradients->isChecked());
	s.writeEntry("/lipstikstyle/Settings/menuLeftSideBar", menuLeftSideBar->isChecked());
	s.writeEntry("/lipstikstyle/Settings/flatStripe", flatStripe->isChecked());
	s.writeEntry("/lipstikstyle/Settings/drawStatusBarFrame", drawStatusBarFrame->isChecked());
	s.writeEntry("/lipstikstyle/Settings/drawToolBarHandle", drawToolBarHandle->isChecked());
	s.writeEntry("/lipstikstyle/Settings/comboboxColored", comboboxColored->isChecked());
	s.writeEntry("/lipstikstyle/Settings/coloredMenuBar", coloredMenuBar->isChecked());
	s.writeEntry("/lipstikstyle/Settings/drawTabHighlight", drawTabHighlight->isChecked());
	s.writeEntry("/lipstikstyle/Settings/menuSpacing", menuSpacing->value());
	s.writeEntry("/lipstikstyle/Settings/toolBarSpacing", toolBarSpacing->value());
	s.writeEntry("/lipstikstyle/Settings/scrollBarStyle", scrollBarType(scrollBarStyle->currentItem()));
	s.writeEntry("/lipstikstyle/Settings/scrollBarWidth", scrollBarWidthType(scrollBarWidth->currentItem()));
	s.writeEntry("/lipstikstyle/Settings/sliderStyle", sliderType(sliderStyle->currentItem()));
	s.writeEntry("/lipstikstyle/Settings/animateProgressBar", animateProgressBar->isChecked());
	s.writeEntry("/lipstikstyle/Settings/drawToolBarSeparator", drawToolBarSeparator->isChecked());
	s.writeEntry("/lipstikstyle/Settings/drawToolBarGradient", drawToolBarGradient->isChecked());
	s.writeEntry("/lipstikstyle/Settings/invertBarGrad", invertBarGrad->isChecked());
	s.writeEntry("/lipstikstyle/Settings/drawToolBarItemSeparator", drawToolBarItemSeparator->isChecked());
 	s.writeEntry("/lipstikstyle/Settings/drawFocusRect", drawFocusRect->isChecked());
 	s.writeEntry("/lipstikstyle/Settings/flatHeaders", flatHeaders->isChecked());
 	s.writeEntry("/lipstikstyle/Settings/tickCheckMark", tickCheckMark->isChecked());
	s.writeEntry("/lipstikstyle/Settings/drawTriangularExpander", drawTriangularExpander->isChecked());
	s.writeEntry("/lipstikstyle/Settings/inputFocusHighlight", inputFocusHighlight->isChecked());
	s.writeEntry("/lipstikstyle/Settings/customOverHighlightColor", customOverHighlightColor->isChecked());
	s.writeEntry("/lipstikstyle/Settings/overHighlightColor", QColor(overHighlightColor->color()).name());
	s.writeEntry("/lipstikstyle/Settings/customMenuStripeColor", customMenuStripeColor->isChecked());
	s.writeEntry("/lipstikstyle/Settings/menuStripeColor", QColor(menuStripeColor->color()).name());
	s.writeEntry("/lipstikstyle/Settings/customFocusHighlightColor", customFocusHighlightColor->isChecked());
	s.writeEntry("/lipstikstyle/Settings/focusHighlightColor", QColor(focusHighlightColor->color()).name());
	s.writeEntry("/lipstikstyle/Settings/customCheckMarkColor", customCheckMarkColor->isChecked());
	s.writeEntry("/lipstikstyle/Settings/checkMarkColor", QColor(checkMarkColor->color()).name());
	s.writeEntry("/lipstikstyle/Settings/customTabHighlightColor", customTabHighlightColor->isChecked());
	s.writeEntry("/lipstikstyle/Settings/tabHighlightColor", QColor(tabHighlightColor->color()).name());
}

void LipstikStyleConfig::defaults()
{
	reverseGradients->setChecked(false);
	paintGroove->setChecked(true);
	sharperMenuGradient->setChecked(false);
	alterMenuGradients->setChecked(true);
	menuLeftSideBar->setChecked(true);
	flatStripe->setChecked(false);
	menuSpacing->setValue(5);
	toolBarSpacing->setValue(0);
	drawStatusBarFrame->setChecked(false);
	drawToolBarHandle->setChecked(true);
	comboboxColored->setChecked(false);
	coloredMenuBar->setChecked(false);
	drawTabHighlight->setChecked(true);
	scrollBarStyle->setCurrentItem(0);
	scrollBarWidth->setCurrentItem(0);
	sliderStyle->setCurrentItem(1);
	animateProgressBar->setChecked(false);
	drawToolBarSeparator->setChecked(true);
	drawToolBarGradient->setChecked(false);
	invertBarGrad->setChecked(true);
	drawToolBarItemSeparator->setChecked(true);
 	drawFocusRect->setChecked(true);
 	flatHeaders->setChecked(true);
 	tickCheckMark->setChecked(true);
	drawTriangularExpander->setChecked(false);
	inputFocusHighlight->setChecked(true);
	customOverHighlightColor->setChecked(false);
	overHighlightColor->setColor("/Qt/KWinPalette/activeBackground");
	customMenuStripeColor->setChecked(false);
	menuStripeColor->setColor("/Qt/KWinPalette/activeBackground");
	customFocusHighlightColor->setChecked(false);
	focusHighlightColor->setColor("/Qt/KWinPalette/activeBackground");
	customCheckMarkColor->setChecked(false);
	checkMarkColor->setColor("/Qt/KWinPalette/activeBackground");
	customTabHighlightColor->setChecked(false);
	tabHighlightColor->setColor("/Qt/KWinPalette/activeBackground");
	//updateChanged would be done by setChecked already
}

void LipstikStyleConfig::updateChanged()
{
	if ( customOverHighlightColor->isChecked() )
	 overHighlightColor->setEnabled(true);
	else
	 overHighlightColor->setEnabled(false);

	if ( drawToolBarGradient->isChecked() )
	 invertBarGrad->setEnabled(true);
	else
	 invertBarGrad->setEnabled(false);

	//Stripe
	if ( menuLeftSideBar->isChecked() ) {
	    customMenuStripeColor->setEnabled(true);
	    flatStripe->setEnabled(true);
	    if ( customMenuStripeColor->isEnabled() )
		menuStripeColor->setEnabled(true);
	    else
		menuStripeColor->setEnabled(false);
	} else {
	    customMenuStripeColor->setEnabled(false);
	    menuStripeColor->setEnabled(false);
	    flatStripe->setEnabled(false);
	}
	
	if ( customMenuStripeColor->isChecked() && customMenuStripeColor->isEnabled() )
	 menuStripeColor->setEnabled(true);
	else
	 menuStripeColor->setEnabled(false);

	//Input Focus
	if ( inputFocusHighlight->isChecked() ) {
	    customFocusHighlightColor->setEnabled(true);
	    if ( customFocusHighlightColor->isEnabled() )
		focusHighlightColor->setEnabled(true);
	    else
		focusHighlightColor->setEnabled(false);
	} else {
	    customFocusHighlightColor->setEnabled(false);
	    focusHighlightColor->setEnabled(false);
	}

	if ( customFocusHighlightColor->isChecked() && customFocusHighlightColor->isEnabled() )
	 focusHighlightColor->setEnabled(true);
	else
	 focusHighlightColor->setEnabled(false);

	if ( customCheckMarkColor->isChecked() )
	 checkMarkColor->setEnabled(true);
	else
	 checkMarkColor->setEnabled(false);

	if ( customTabHighlightColor->isChecked() )
	 tabHighlightColor->setEnabled(true);
	else
	 tabHighlightColor->setEnabled(false);

	if ((animateProgressBar->isChecked() == origAnimProgressBar) &&
	    (reverseGradients->isChecked() == origReverseGradients) &&
	    (paintGroove->isChecked() == origPaintGroove) &&
	    (sharperMenuGradient->isChecked() == origSharperMenuGradient) &&
	    (alterMenuGradients->isChecked() == origAlterMenuGradients) &&
	    (menuLeftSideBar->isChecked() == origMenuLeftSideBar) &&
	    (flatStripe->isChecked() == origFlatStripe) &&
	    (menuSpacing->value() == origMenuSpacing) &&
	    (toolBarSpacing->value() == origToolBarSpacing) &&
	    (drawStatusBarFrame->isChecked() == origDrawStatusBarFrame) &&
	    (drawToolBarHandle->isChecked() == origDrawToolBarHandle) &&
	    (comboboxColored->isChecked() == origComboboxColored) &&
	    (coloredMenuBar->isChecked() == origColoredMenuBar) &&
	    (drawTabHighlight->isChecked() == origDrawTabHighlight) &&
	    (drawToolBarSeparator->isChecked() == origDrawToolBarSeparator) &&
	    (drawToolBarGradient->isChecked() == origDrawToolBarGradient) &&
	    (invertBarGrad->isChecked() == origInvertBarGrad) &&
	    (drawToolBarItemSeparator->isChecked() == origDrawToolBarItemSeparator) &&
 	    (drawFocusRect->isChecked() == origDrawFocusRect) &&
 	    (flatHeaders->isChecked() == origFlatHeaders) &&
 	    (tickCheckMark->isChecked() == origTickCheckMark) &&
	    (drawTriangularExpander->isChecked() == origDrawTriangularExpander) &&
	    (inputFocusHighlight->isChecked() == origInputFocusHighlight) &&
	    (customOverHighlightColor->isChecked() == origCustomOverHighlightColor) &&
	    (overHighlightColor->color() == origOverHighlightColor) &&
	    (customMenuStripeColor->isChecked() == origCustomMenuStripeColor) &&
	    (menuStripeColor->color() == origMenuStripeColor) &&
	    (customFocusHighlightColor->isChecked() == origCustomFocusHighlightColor) &&
	    (focusHighlightColor->color() == origFocusHighlightColor) &&
	    (customCheckMarkColor->isChecked() == origCustomCheckMarkColor) &&
	    (checkMarkColor->color() == origCheckMarkColor) &&
	    (customTabHighlightColor->isChecked() == origCustomTabHighlightColor) &&
	    (tabHighlightColor->color() == origTabHighlightColor) &&
	    (scrollBarStyle->currentText() == origScrollBarStyle) &&
	    (scrollBarWidth->currentText() == origScrollBarWidth) &&
	    (sliderStyle->currentText() == origSliderStyle)
	)
	    emit changed(false);
	else
	    emit changed(true);
}

//Scrollbar types
QString LipstikStyleConfig::scrollBarType( int listnr )
{
    switch ( listnr ) {
        case 0:
            return QString("WindowsStyleScrollBar");
        case 1:
            return QString("PlatinumStyleScrollBar");
	case 2:
	    return QString("ThreeButtonScrollBar");
	case 3:
	    return QString("NextStyleScrollBar");
	default:
	    return QString("PlatinumStyleScrollBar");
    }
}

int LipstikStyleConfig::scrollBarItem( QString kSBType )
{
    if( !strcmp(kSBType,"WindowsStyleScrollBar") )
        return 0;
    else if( !strcmp(kSBType,"PlatinumStyleScrollBar") )
        return 1;
    else if( !strcmp(kSBType,"ThreeButtonScrollBar") )
        return 2;
    else if( !strcmp(kSBType,"NextStyleScrollBar") )
        return 3;
    else
        return 1;
}

//Scrollbar Widths
QString LipstikStyleConfig::scrollBarWidthType( int widthListnr )
{
    switch ( widthListnr ) {
        case 0:
            return QString("Small");
        case 1:
            return QString("Normal");
	case 2:
	    return QString("Large");
	default:
	    return QString("Small");
    }
}

int LipstikStyleConfig::scrollBarWidthItem( QString kSBWidthType )
{
    if( !strcmp(kSBWidthType,"Small") )
        return 0;
    else if( !strcmp(kSBWidthType,"Normal") )
        return 1;
    else if( !strcmp(kSBWidthType,"Large") )
        return 2;
    else
        return 0;
}

//Slider types
int LipstikStyleConfig::sliderItem( QString kSliderType )
{
    if( !strcmp(kSliderType,"sliderPlastikStyle") )
        return 0;
    else if( !strcmp(kSliderType,"sliderLipstikStyle") )
        return 1;
    else if( !strcmp(kSliderType,"sliderGtkStyle") )
        return 2;
    else
        return 2;
}

QString LipstikStyleConfig::sliderType( int sliderlistnr )
{
    switch ( sliderlistnr ) {
        case 0:
            return QString("sliderPlastikStyle");
        case 1:
            return QString("sliderLipstikStyle");
	case 2:
	    return QString("sliderGtkStyle");
	default:
	    return QString("sliderGtkStyle");
    }
}

#include "lipstikconf.moc"
