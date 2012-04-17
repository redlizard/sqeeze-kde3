/*
Copyright (C) 2004 Jens Luetkens <j.luetkens@limitland.de>

based on the KDE style "Plastik":

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

#include <qlayout.h>
#include <qsettings.h>
#include <kglobal.h>
#include <klocale.h>

#include <qcheckbox.h>
#include <qlabel.h>
#include <kcolorbutton.h>
#include <qcombobox.h>

#include "comixconf.h"

extern "C"
{
	QWidget* allocate_kstyle_config(QWidget* parent)
	{
		KGlobal::locale()->insertCatalogue("kstyle_comix_config");
		return new ComixStyleConfig(parent);
	}
}

ComixStyleConfig::ComixStyleConfig(QWidget* parent): QWidget(parent)
{
	//Should have no margins here, the dialog provides them
	KGlobal::locale()->insertCatalogue("kstyle_comix_config");

	QLabel* colorButtonLabel;
	QLabel* scrollBarStyleLabel;

	drawToolBarSeparator = new QCheckBox(i18n("Draw toolbar separator"), this);
	drawToolBarItemSeparator = new QCheckBox(i18n("Draw toolbar item separator"), this);
	centerTabs = new QCheckBox(i18n("Center tabs"), this);
	hideFocusRect = new QCheckBox(i18n("Hide focus rectangle"), this);
	konsoleTabColor = new QCheckBox(i18n("Use konsole tab color"), this);
	buttonSurfaceHighlight = new QCheckBox(i18n("Use button surface highlight"), this);
	useMenuFrame = new QCheckBox(i18n("Use menu frame only"), this);
	useCustomBrushColor = new QCheckBox(i18n("Use custom brush color"), this);

	colorButtonLabel = new QLabel(this);
    colorButtonLabel->setText(tr("Custom brush color:    "));
	customBrushColor = new KColorButton(i18n("Custom brush color"), this);

	scrollBarStyleLabel = new QLabel(this);
    scrollBarStyleLabel->setText(tr("Scrollbar style:       "));
	scrollBarStyle = new QComboBox(i18n("Scrollbar style"), this);
    scrollBarStyle->setEditable(false);
    scrollBarStyle->insertItem("Windows style");
    scrollBarStyle->insertItem("Platinum style");
    scrollBarStyle->insertItem("KDE style");
    scrollBarStyle->insertItem("Next style");

	QVBoxLayout* layout = new QVBoxLayout(this, 0, 0);

	layout->add(drawToolBarSeparator);
	layout->add(drawToolBarItemSeparator);
	layout->add(centerTabs);
	layout->add(hideFocusRect);
	layout->add(konsoleTabColor);
	layout->add(buttonSurfaceHighlight);
	layout->add(useMenuFrame);
	layout->add(useCustomBrushColor);
    
	QHBoxLayout* hlayout1 = new QHBoxLayout(layout);

	hlayout1->add(colorButtonLabel);
	hlayout1->add(customBrushColor);

	QHBoxLayout* hlayout2 = new QHBoxLayout(layout);

	hlayout2->add(scrollBarStyleLabel);
	hlayout2->add(scrollBarStyle);

	//layout->addLayout(hlayout);
	layout->addStretch(1);

	QSettings s;

	origDrawToolBarSeparator = s.readBoolEntry("/comixstyle/Settings/drawToolBarSeparator", false);
	origDrawToolBarItemSeparator = s.readBoolEntry("/comixstyle/Settings/drawToolBarItemSeparator", false);
	origCenterTabs = s.readBoolEntry("/comixstyle/Settings/centerTabs", false);
	origHideFocusRect = s.readBoolEntry("/comixstyle/Settings/hideFocusRect", true);
	origKonsoleTabColor = s.readBoolEntry("/comixstyle/Settings/konsoleTabColor", true);
	origButtonSurfaceHighlight = s.readBoolEntry("/comixstyle/Settings/buttonSurfaceHighlight", false);
	origUseMenuFrame = s.readBoolEntry("/comixstyle/Settings/useMenuFrame", false);
	origUseCustomBrushColor = s.readBoolEntry("/comixstyle/Settings/useCustomBrushColor", false);
	origCustomBrushColor = QColor( s.readEntry("/comixstyle/Settings/customBrushColor", "#808080") );
	origScrollBarStyle = s.readEntry("/comixstyle/Settings/scrollBarStyle", "ThreeButtonScrollBar");

	drawToolBarSeparator->setChecked(origDrawToolBarSeparator);
	drawToolBarItemSeparator->setChecked(origDrawToolBarItemSeparator);
	centerTabs->setChecked(origCenterTabs);
	hideFocusRect->setChecked(origHideFocusRect);
	konsoleTabColor->setChecked(origKonsoleTabColor);
	buttonSurfaceHighlight->setChecked(origButtonSurfaceHighlight);
	useMenuFrame->setChecked(origUseMenuFrame);
	useCustomBrushColor->setChecked(origUseCustomBrushColor);
	customBrushColor->setColor(origCustomBrushColor);
	scrollBarStyle->setCurrentItem(scrollBarItem(origScrollBarStyle));
	
	connect(drawToolBarSeparator, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(drawToolBarItemSeparator, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(centerTabs, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(hideFocusRect, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(konsoleTabColor, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(buttonSurfaceHighlight, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(useMenuFrame, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(useCustomBrushColor, SIGNAL( toggled(bool) ), SLOT( updateChanged() ) );
	connect(customBrushColor, SIGNAL( changed(const QColor &) ), SLOT( updateChanged() ) );
	connect(scrollBarStyle, SIGNAL( activated(int) ), SLOT( updateChanged() ) );
}

ComixStyleConfig::~ComixStyleConfig()
{
}


void ComixStyleConfig::save()
{
	QSettings s;
	s.writeEntry("/comixstyle/Settings/drawToolBarSeparator", drawToolBarSeparator->isChecked());
	s.writeEntry("/comixstyle/Settings/drawToolBarItemSeparator", drawToolBarItemSeparator->isChecked());
	s.writeEntry("/comixstyle/Settings/centerTabs", centerTabs->isChecked());
	s.writeEntry("/comixstyle/Settings/hideFocusRect", hideFocusRect->isChecked());
	s.writeEntry("/comixstyle/Settings/konsoleTabColor", konsoleTabColor->isChecked());
	s.writeEntry("/comixstyle/Settings/buttonSurfaceHighlight", buttonSurfaceHighlight->isChecked());
	s.writeEntry("/comixstyle/Settings/useMenuFrame", useMenuFrame->isChecked());
	s.writeEntry("/comixstyle/Settings/useCustomBrushColor", useCustomBrushColor->isChecked());
	s.writeEntry("/comixstyle/Settings/customBrushColor", customBrushColor->color().name());
	s.writeEntry("/comixstyle/Settings/scrollBarStyle", scrollBarType(scrollBarStyle->currentItem()));
}

void ComixStyleConfig::defaults()
{
	drawToolBarSeparator->setChecked(false);
	drawToolBarItemSeparator->setChecked(false);
	centerTabs->setChecked(false);
	hideFocusRect->setChecked(true);
	konsoleTabColor->setChecked(true);
	buttonSurfaceHighlight->setChecked(false);
	useMenuFrame->setChecked(false);
	useCustomBrushColor->setChecked(false);
	customBrushColor->setColor(QColor("#808080"));
	scrollBarStyle->setCurrentItem(2);
	//updateChanged would be done by setChecked already
}

void ComixStyleConfig::updateChanged()
{
	if ( 
		(drawToolBarSeparator->isChecked() == origDrawToolBarSeparator) &&
		(drawToolBarItemSeparator->isChecked() == origDrawToolBarItemSeparator) &&
		(centerTabs->isChecked() == origCenterTabs) &&
		(hideFocusRect->isChecked() == origHideFocusRect) &&
		(konsoleTabColor->isChecked() == origKonsoleTabColor) &&
		(buttonSurfaceHighlight->isChecked() == origButtonSurfaceHighlight) &&
		(useMenuFrame->isChecked() == origUseMenuFrame) &&
		(useCustomBrushColor->isChecked() == origUseCustomBrushColor) &&
		(customBrushColor->color() == origCustomBrushColor) && 
		(scrollBarStyle->currentText() == origScrollBarStyle) 
	)
		emit changed(false);
	else
		emit changed(true);
}

QString ComixStyleConfig::scrollBarType( int listnr )
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
			return QString("ThreeButtonScrollBar");
	}
}

int ComixStyleConfig::scrollBarItem( QString kSBType )
{
	if( !strcmp(kSBType,"WindowsStyleScrollBar") )
		return 0;
	else if( !strcmp(kSBType,"PlatinumStyleScrollBar") )
		return 1;
	else if( !strcmp(kSBType,"NextStyleScrollBar") )
		return 3;
	else
		return 2;
}

#include "comixconf.moc"
