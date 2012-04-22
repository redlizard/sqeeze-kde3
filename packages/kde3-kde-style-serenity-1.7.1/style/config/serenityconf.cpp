/*
	Serenity Style for KDE 3+
	
		Copyright (C) 2006 Remi Villatel <maxilys@tele2.fr>
	
	Based on Lipstik engine:
	
		Copyright (C) 2004 Patrice Tremblay <tremblaypatrice@yahoo.fr>
	
	Also based on numerous open source styles that helped me to not 
	re-invent the wheel for every line of code. Thanks to everyone.  ;-)
	
	This library is free software; you can redistribute it and/or 
	modify it under the terms of the GNU Library General Public 
	License version 2 as published by the Free Software Foundation.
	
	This library is distributed in the hope that it will be useful, 
	but WITHOUT ANY WARRANTY; without even the implied warranty of 
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
	Library General Public License for more details.
	
	You should have received a copy of the GNU Library General Public 
	License along with this library. (See the file COPYING.) If not, 
	write to:
	The Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
	Boston, MA 02111-1307, USA.
*/

#include <qapplication.h>
#include <qcheckbox.h>
#include <qcolor.h>
#include <qcombobox.h>
#include <qframe.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qsettings.h>
#include <qslider.h>
#include <qtabwidget.h>
#include <qvgroupbox.h>

#include <kcolorbutton.h>
#include <kglobal.h>
#include <klocale.h>
#include <kseparator.h>
#include <kurllabel.h>
#include <kdemacros.h>

#include "serenityconf.h"
#include "configdialog.h"

extern "C"
{
	KDE_EXPORT QWidget* allocate_kstyle_config(QWidget* parent)
	{
		KGlobal::locale()->insertCatalogue("kstyle_serenity_config");
		return new SerenityStyleConfig(parent);
	}
}

SerenityStyleConfig::SerenityStyleConfig(QWidget* parent): QWidget(parent)
{
	//Should have no margins here, the dialog provides them.
	QVBoxLayout* layout = new QVBoxLayout(this, 0, 0);
	dialog_ = new ConfigDialog(this);
	KGlobal::locale()->insertCatalogue("kstyle_serenity_config");

	layout->addWidget(dialog_);
	dialog_->show();

	// Read config and populate!
	QSettings s;

	origActiveTabStyle = limitedTo(0, 3, s.readNumEntry("/serenitystyle/Settings/activeTabStyle", 0));
	dialog_->activeTabStyle->setCurrentItem(origActiveTabStyle);
	origAlternateSinking = s.readBoolEntry("/serenitystyle/Settings/alternateSunkenEffect", false);
	dialog_->alternateSinking->setChecked(origAlternateSinking);
	origAnimProgressBar = s.readBoolEntry("/serenitystyle/Settings/animateProgressBar", true);
	dialog_->animateProgressBar->setChecked(origAnimProgressBar);
	origCenterTabs = s.readBoolEntry("/serenitystyle/Settings/centerTabs", false);
	dialog_->centerTabs->setChecked(origCenterTabs);
	origCustomGlobalColor = s.readBoolEntry("/serenitystyle/Settings/customGlobalColor", false);
	dialog_->customGlobalColor->setChecked(origCustomGlobalColor);
	origCustomOverHighlightColor = s.readBoolEntry("/serenitystyle/Settings/customOverHighlightColor", false);
	dialog_->customOverHighlightColor->setChecked(origCustomOverHighlightColor);
	origDrawFocusRect = s.readBoolEntry("/serenitystyle/Settings/drawFocusRect", true);
	dialog_->drawFocusRect->setChecked(origDrawFocusRect);
	origDrawToolBarHandle = s.readBoolEntry("/serenitystyle/Settings/drawToolBarHandle", true);
	dialog_->drawToolBarHandle->setChecked(origDrawToolBarHandle);
	origDrawToolBarSeparator = s.readBoolEntry("/serenitystyle/Settings/drawToolBarSeparator", false);
	dialog_->drawToolBarSeparator->setChecked(origDrawToolBarSeparator);
	origDrawTriangularExpander = s.readBoolEntry("/serenitystyle/Settings/drawTriangularExpander", true);
	dialog_->drawTriangularExpander->setChecked(origDrawTriangularExpander);
	origFlatProgressBar = s.readBoolEntry("/serenitystyle/Settings/flatProgressBar", false);
	dialog_->flatProgressBar->setChecked(origFlatProgressBar);
	origGlobalColor = s.readEntry("/serenitystyle/Settings/globalColor", "#800000");
	dialog_->globalColor->setColor(origGlobalColor);
	origGlobalStyle = limitedTo(0, 2, s.readNumEntry("/serenitystyle/Settings/globalStyle", 1));
	dialog_->globalStyle->setCurrentItem(origGlobalStyle);
	origHiliteRubber = s.readBoolEntry("/serenitystyle/Settings/highlightedRubberband", false);
	dialog_->hiliteRubber->setChecked(origHiliteRubber);
	origMenubarHack = s.readBoolEntry("/serenitystyle/Settings/menubarHack", false);
	dialog_->menubarHack->setChecked(origMenubarHack);
	origMenuGrooveStyle = limitedTo(0, 5, s.readNumEntry("/serenitystyle/Settings/menuGrooveStyle", 0));
	dialog_->menuGrooveStyle->setCurrentItem(origMenuGrooveStyle);
	//origMouseOverLabel = s.readBoolEntry("/serenitystyle/Settings/mouseOverLabel", true);
	//dialog_->mouseOverLabel->setChecked(origMouseOverLabel);
	origOverHighlightColor = s.readEntry("/serenitystyle/Settings/overHighlightColor", "#00d000");
	dialog_->overHighlightColor->setColor(origOverHighlightColor);
	origPassiveTabStyle = limitedTo(0, 3, s.readNumEntry("/serenitystyle/Settings/passiveTabStyle", 0));
	dialog_->passiveTabStyle->setCurrentItem(origPassiveTabStyle);
	origProgressBarPattern = limitedTo(0, 3, s.readNumEntry("/serenitystyle/Settings/progressBarPattern", 0));
	dialog_->progressBarPattern->setCurrentItem(origProgressBarPattern);
	origPurerHover = s.readBoolEntry("/serenitystyle/Settings/purerHover", false);
	dialog_->purerHover->setChecked(origPurerHover);
	origResizerStyle = limitedTo(0, 2, s.readNumEntry("/serenitystyle/Settings/resizerStyle", 0));
	dialog_->resizerStyle->setCurrentItem(origResizerStyle);
	origScrollBarStyle = limitedTo(0, 4, s.readNumEntry("/serenitystyle/Settings/scrollBarStyle", 0));
	dialog_->scrollBarStyle->setCurrentItem(origScrollBarStyle);
	origScrollerScheme = limitedTo(0, 2, s.readNumEntry("/serenitystyle/Settings/scrollerScheme", 0));
	dialog_->scrollerScheme->setCurrentItem(origScrollerScheme);
	origSubArrowFrame = s.readBoolEntry("/serenitystyle/Settings/submenuArrowFrame", false);
	dialog_->subArrowFrame->setChecked(origSubArrowFrame);
	origSplitTabs = s.readBoolEntry("/serenitystyle/Settings/splitTabs", true);
	dialog_->splitTabs->setChecked(origSplitTabs);
	origSubmenuSpeed = limitedTo(-16, 16, s.readNumEntry("/serenitystyle/Settings/submenuSpeed", 0));
	dialog_->submenuSpeed->setValue(origSubmenuSpeed);
	origTabWarnStyle = limitedTo(0, 3, s.readNumEntry("/serenitystyle/Settings/tabWarningStyle", 0));
	dialog_->tabWarnStyle->setCurrentItem(origTabWarnStyle);
	origTextboxHilite = s.readBoolEntry("/serenitystyle/Settings/textboxFocusHighlight", true);
	dialog_->textboxHilite->setChecked(origTextboxHilite);
	origTipTint = limitedTo(0, 7, s.readNumEntry("/serenitystyle/Settings/tipTint", 0));
	dialog_->tipTint->setCurrentItem(origTipTint);
	origTreeViewStyle = limitedTo(0, 3, s.readNumEntry("/serenitystyle/Settings/treeViewStyle", 0));
	dialog_->treeViewStyle->setCurrentItem(origTreeViewStyle);
	origWideSlider = s.readBoolEntry("/serenitystyle/Settings/wideSlider", false);
	dialog_->wideSlider->setChecked(origWideSlider);
	
	connect(dialog_->activeTabStyle, SIGNAL(activated(int)), SLOT(updateChanged()));
	connect(dialog_->alternateSinking, SIGNAL(toggled(bool)), SLOT(updateChanged()));
	connect(dialog_->animateProgressBar, SIGNAL(toggled(bool)), SLOT(updateChanged()));
	connect(dialog_->centerTabs, SIGNAL(toggled(bool)), SLOT(updateChanged()));
	connect(dialog_->customGlobalColor, SIGNAL(toggled(bool)), SLOT(updateChanged()));
	connect(dialog_->customOverHighlightColor, SIGNAL(toggled(bool)), SLOT(updateChanged()));
 	connect(dialog_->drawFocusRect, SIGNAL(toggled(bool)), SLOT(updateChanged()));
	connect(dialog_->drawToolBarHandle, SIGNAL(toggled(bool)), SLOT(updateChanged()));
	connect(dialog_->drawToolBarSeparator, SIGNAL(toggled(bool)), SLOT(updateChanged()));
	connect(dialog_->drawTriangularExpander, SIGNAL(toggled(bool)), SLOT(updateChanged()));
	connect(dialog_->flatProgressBar, SIGNAL(toggled(bool)), SLOT(updateChanged()));
	connect(dialog_->globalStyle, SIGNAL(activated(int)), SLOT(updateChanged()));
	connect(dialog_->hiliteRubber, SIGNAL(toggled(bool)), SLOT(updateChanged()));
	connect(dialog_->menubarHack, SIGNAL(toggled(bool)), SLOT(updateChanged()));
	connect(dialog_->globalColor, SIGNAL(pressed()), SLOT(updateChanged()));
	connect(dialog_->menuGrooveStyle, SIGNAL(activated(int)), SLOT(updateChanged()));
	//connect(dialog_->mouseOverLabel, SIGNAL(toggled(bool)), SLOT(updateChanged()));
	connect(dialog_->overHighlightColor, SIGNAL(pressed()), SLOT(updateChanged()));
	connect(dialog_->passiveTabStyle, SIGNAL(activated(int)), SLOT(updateChanged()));
	connect(dialog_->progressBarPattern, SIGNAL(activated(int)), SLOT(updateChanged()));
	connect(dialog_->purerHover, SIGNAL(toggled(bool)), SLOT(updateChanged()));
	connect(dialog_->resizerStyle, SIGNAL(activated(int)), SLOT(updateChanged()));
	connect(dialog_->scrollBarStyle, SIGNAL(activated(int)), SLOT(updateChanged()));
	connect(dialog_->scrollerScheme, SIGNAL(activated(int)), SLOT(updateChanged()));
	connect(dialog_->splitTabs, SIGNAL(toggled(bool)), SLOT(updateChanged()));
	connect(dialog_->subArrowFrame, SIGNAL(toggled(bool)), SLOT(updateChanged()));
	connect(dialog_->submenuSpeed, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
	connect(dialog_->tabWarnStyle, SIGNAL(activated(int)), SLOT(updateChanged()));
	connect(dialog_->textboxHilite, SIGNAL(toggled(bool)), SLOT(updateChanged()));
	connect(dialog_->tipTint, SIGNAL(activated(int)), SLOT(updateChanged()));
	connect(dialog_->treeViewStyle, SIGNAL(activated(int)), SLOT(updateChanged()));
	connect(dialog_->wideSlider, SIGNAL(toggled(bool)), SLOT(updateChanged()));

	if (dialog_->globalStyle->currentItem() >= 2)
	{
		dialog_->alternateSinking->setEnabled(false);
	}
	else
	{
		dialog_->alternateSinking->setEnabled(true);
	}
	if ( dialog_->customOverHighlightColor->isChecked() )
	{
		dialog_->overHighlightColor->setEnabled(true);
	}
	else
	{
		dialog_->overHighlightColor->setEnabled(false);
	}
	if ( dialog_->customGlobalColor->isChecked() )
	{
		dialog_->globalColor->setEnabled(true);
	}
	else
	{
		dialog_->globalColor->setEnabled(false);
	}
	if (dialog_->treeViewStyle->currentItem() == 3)
	{
		dialog_->drawTriangularExpander->setEnabled(false);
	}
	else
	{
		dialog_->drawTriangularExpander->setEnabled(true);
	}
}

SerenityStyleConfig::~SerenityStyleConfig()
{
}


void SerenityStyleConfig::save()
{
	QSettings s;

	s.writeEntry("/serenitystyle/Settings/activeTabStyle", dialog_->activeTabStyle->currentItem());
	s.writeEntry("/serenitystyle/Settings/alternateSunkenEffect", dialog_->alternateSinking->isChecked());
	s.writeEntry("/serenitystyle/Settings/animateProgressBar", dialog_->animateProgressBar->isChecked());
	s.writeEntry("/serenitystyle/Settings/centerTabs", dialog_->centerTabs->isChecked());
	s.writeEntry("/serenitystyle/Settings/customGlobalColor", dialog_->customGlobalColor->isChecked());
	s.writeEntry("/serenitystyle/Settings/customOverHighlightColor", dialog_->customOverHighlightColor->isChecked());
	s.writeEntry("/serenitystyle/Settings/drawFocusRect", dialog_->drawFocusRect->isChecked());
	s.writeEntry("/serenitystyle/Settings/drawToolBarHandle", dialog_->drawToolBarHandle->isChecked());
	s.writeEntry("/serenitystyle/Settings/drawToolBarSeparator", dialog_->drawToolBarSeparator->isChecked());
	s.writeEntry("/serenitystyle/Settings/drawTriangularExpander", dialog_->drawTriangularExpander->isChecked());
	s.writeEntry("/serenitystyle/Settings/flatProgressBar", dialog_->flatProgressBar->isChecked());
	s.writeEntry("/serenitystyle/Settings/globalColor", QColor(dialog_->globalColor->color()).name());
	s.writeEntry("/serenitystyle/Settings/globalStyle", dialog_->globalStyle->currentItem());
	s.writeEntry("/serenitystyle/Settings/highlightedRubberband", dialog_->hiliteRubber->isChecked());
	s.writeEntry("/serenitystyle/Settings/menubarHack", dialog_->menubarHack->isChecked());
	s.writeEntry("/serenitystyle/Settings/menuGrooveStyle", dialog_->menuGrooveStyle->currentItem());
	//s.writeEntry("/serenitystyle/Settings/mouseOverLabel", dialog_->mouseOverLabel->isChecked());
	s.writeEntry("/serenitystyle/Settings/overHighlightColor", QColor(dialog_->overHighlightColor->color()).name());
	s.writeEntry("/serenitystyle/Settings/passiveTabStyle", dialog_->passiveTabStyle->currentItem());
	s.writeEntry("/serenitystyle/Settings/progressBarPattern", dialog_->progressBarPattern->currentItem());
	s.writeEntry("/serenitystyle/Settings/purerHover", dialog_->purerHover->isChecked());
	s.writeEntry("/serenitystyle/Settings/resizerStyle", dialog_->resizerStyle->currentItem());
	s.writeEntry("/serenitystyle/Settings/scrollBarStyle", dialog_->scrollBarStyle->currentItem());
	s.writeEntry("/serenitystyle/Settings/scrollerScheme", dialog_->scrollerScheme->currentItem());
	s.writeEntry("/serenitystyle/Settings/splitTabs", dialog_->splitTabs->isChecked());
	s.writeEntry("/serenitystyle/Settings/submenuArrowFrame", dialog_->subArrowFrame->isChecked());
	s.writeEntry("/serenitystyle/Settings/submenuSpeed", dialog_->submenuSpeed->value());
	s.writeEntry("/serenitystyle/Settings/tabWarningStyle", dialog_->tabWarnStyle->currentItem());
	s.writeEntry("/serenitystyle/Settings/textboxFocusHighlight", dialog_->textboxHilite->isChecked());
	s.writeEntry("/serenitystyle/Settings/tipTint", dialog_->tipTint->currentItem());
	s.writeEntry("/serenitystyle/Settings/treeViewStyle", dialog_->treeViewStyle->currentItem());
	s.writeEntry("/serenitystyle/Settings/wideSlider", dialog_->wideSlider->isChecked());
}

void SerenityStyleConfig::defaults()
{
	dialog_->activeTabStyle->setCurrentItem(0);
	dialog_->alternateSinking->setChecked(false);
	dialog_->animateProgressBar->setChecked(true);
	dialog_->centerTabs->setChecked(false);
	dialog_->customGlobalColor->setChecked(false);
	dialog_->customOverHighlightColor->setChecked(false);
	dialog_->drawFocusRect->setChecked(true);
	dialog_->drawToolBarHandle->setChecked(true);
	dialog_->drawToolBarSeparator->setChecked(false);
	dialog_->drawTriangularExpander->setChecked(true);
	dialog_->flatProgressBar->setChecked(false);
	dialog_->globalStyle->setCurrentItem(1);
	dialog_->hiliteRubber->setChecked(false);
	dialog_->menubarHack->setChecked(false);
	dialog_->globalColor->setColor("#800000");
	dialog_->menuGrooveStyle->setCurrentItem(0);
	//dialog_->mouseOverLabel->setChecked(true);
	dialog_->overHighlightColor->setColor("#00d000");
	dialog_->passiveTabStyle->setCurrentItem(0);
	dialog_->progressBarPattern->setCurrentItem(0);
	dialog_->purerHover->setChecked(false);
	dialog_->resizerStyle->setCurrentItem(0);
	dialog_->scrollBarStyle->setCurrentItem(0);
	dialog_->scrollerScheme->setCurrentItem(0);
	dialog_->splitTabs->setChecked(true);
	dialog_->subArrowFrame->setChecked(false);
	dialog_->submenuSpeed->setValue(0);
	dialog_->tabWarnStyle->setCurrentItem(0);
	dialog_->textboxHilite->setChecked(true);
	dialog_->tipTint->setCurrentItem(0);
	dialog_->treeViewStyle->setCurrentItem(0);
	dialog_->wideSlider->setChecked(false);
}

void SerenityStyleConfig::updateChanged()
{
	if (dialog_->globalStyle->currentItem() >= 2)
	{
		dialog_->alternateSinking->setEnabled(false);
	}
	else
	{
		dialog_->alternateSinking->setEnabled(true);
	}
	if ( dialog_->customOverHighlightColor->isChecked() )
	{
		dialog_->overHighlightColor->setEnabled(true);
	}
	else
	{
		dialog_->overHighlightColor->setEnabled(false);
	}
	if ( dialog_->customGlobalColor->isChecked() )
	{
		dialog_->globalColor->setEnabled(true);
	}
	else
	{
		dialog_->globalColor->setEnabled(false);
	}
	if (dialog_->treeViewStyle->currentItem() == 3)
	{
		dialog_->drawTriangularExpander->setEnabled(false);
	}
	else
	{
		dialog_->drawTriangularExpander->setEnabled(true);
	}
	emit changed(true);
}

// Sanity check while reading config file
int SerenityStyleConfig::limitedTo(int lowest, int highest, int variable)
{
	if ( (variable < lowest) || (variable > highest) )
		return (lowest < 0) ? 0 : lowest;
	else
		return variable;
}


#include "serenityconf.moc"
