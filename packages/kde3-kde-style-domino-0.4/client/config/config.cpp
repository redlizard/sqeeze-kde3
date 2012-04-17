/*
	Copyright (C) 2006 Michael Lentner <michaell@gmx.net>
	
	based on KDE2 Default configuration widget:
	Copyright (c) 2001
		Karol Szwed <gallium@kde.org>
		http://gallium.n3.net/
	
	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public
	License version 2 as published by the Free Software Foundation.
	
	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
	Library General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "config.h"
#include <kglobal.h>
#include <klocale.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qsettings.h>

extern "C"
{
	KDE_EXPORT QObject* allocate_config( KConfig* config, QWidget* parent )
	{
		return(new DominoConfig(config, parent));
	}
}

// NOTE:
// 'config' is a pointer to the kwindecoration modules open kwin config,
//		  and is by default set to the "Style" group.
// 'parent' is the parent of the QObject, which is a VBox inside the
//		  Configure tab in kwindecoration

DominoConfig::DominoConfig( KConfig* config, QWidget* parent )
	: QObject( parent )
{
	conf = new KConfig("kwindominorc");
	
	KGlobal::locale()->insertCatalogue("kwin_clients");
	
	vBox = new QVBox( parent );

	customBorderColor = new QCheckBox( i18n("Custom border color"), vBox );
	QHBox *hbox1 = new QHBox(vBox);
	hbox1->layout()->addItem(new QSpacerItem(20, 0, QSizePolicy::Fixed, QSizePolicy::Minimum) );
	borderColor = new KColorButton(hbox1);
	
	customButtonColor = new QCheckBox( i18n("Custom button color"), vBox );
	QHBox *hbox2 = new QHBox(vBox);
	hbox2->layout()->addItem(new QSpacerItem(20, 0, QSizePolicy::Fixed, QSizePolicy::Minimum) );
	buttonColor = new KColorButton(hbox2);
	
	customButtonIconColor = new QCheckBox( i18n("Custom button icon color"), vBox );
	QHBox *hbox3 = new QHBox(vBox);
	hbox3->layout()->addItem(new QSpacerItem(20, 0, QSizePolicy::Fixed, QSizePolicy::Minimum) );
	buttonIconColor = new KColorButton(hbox3);
	
	customGradientColors = new QCheckBox( i18n("Custom gradient colors"), vBox);
	QHBox *hbox4 = new QHBox(vBox);
	hbox4->layout()->addItem(new QSpacerItem(22, 0, QSizePolicy::Fixed, QSizePolicy::Minimum) );
	
	QVBox *vbox1 = new QVBox(hbox4);
	new QLabel( i18n("Top gradient color"), vbox1);
	topGradientColor = new KColorButton(vbox1);
	new QLabel(i18n("Bottom gradient color"), vbox1);
	bottomGradientColor = new KColorButton(vbox1);
	
	showInactiveButtons = new QCheckBox( i18n("Show inactive buttons"), vBox );
	showButtonIcons  = new QCheckBox( i18n("Show button icons"), vBox );
	darkFrame  = new QCheckBox( i18n("Dark window frame"), vBox );
	
	
	///////////////////////////////////////
	
	
	QGroupBox* colorGb = new QGroupBox(vBox);
	colorGb->setTitle(i18n("Button contour colors:"));
	colorGb->setColumnLayout(0, Qt::Vertical );
	colorGb->layout()->setSpacing( 6 );
	colorGb->layout()->setMargin( 11 );
	
	QGridLayout* gbLayout = new QGridLayout( colorGb->layout(), 4, 2 );
	gbLayout->setAlignment( Qt::AlignTop );
	
	QLabel* labelSC = new QLabel(i18n("Standard color:"), colorGb);
	buttonContourColor = new KColorButton(colorGb);
	buttonContourColor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	QLabel* labelMC = new QLabel(i18n("Mouseover color:"), colorGb);
	buttonMouseOverContourColor = new KColorButton(colorGb);
	QLabel* labelPC = new QLabel(i18n("Pressed color:"), colorGb);
	buttonPressedContourColor = new KColorButton(colorGb);
	
	useDominoStyleContourColors = new QCheckBox( i18n("Use domino style contour colors"), colorGb );
	gbLayout->addMultiCellWidget(useDominoStyleContourColors, 0, 0, 0, 1);
	
	gbLayout->addWidget(labelSC, 1, 0);
	gbLayout->addItem(new QSpacerItem(20, 0, QSizePolicy::Minimum, QSizePolicy::Minimum) );
	gbLayout->addWidget(buttonContourColor, 1, 1);
	
	gbLayout->addWidget(labelMC, 2, 0);
	gbLayout->addItem(new QSpacerItem(20, 0, QSizePolicy::Minimum, QSizePolicy::Minimum) );
	gbLayout->addWidget(buttonMouseOverContourColor, 2, 1);
	
	gbLayout->addWidget(labelPC, 3, 0);
	gbLayout->addItem(new QSpacerItem(20, 0, QSizePolicy::Minimum, QSizePolicy::Minimum) );
	gbLayout->addWidget(buttonPressedContourColor, 3, 1);
	
	
	
	
	
	
	
	
	
	///////////////////////////////////////
	
	// Load configuration options
	load(config);

	// Ensure we track user changes properly
	connect(customGradientColors, SIGNAL(toggled(bool)), topGradientColor, SLOT(setEnabled(bool)));
	connect(customGradientColors, SIGNAL(toggled(bool)), bottomGradientColor, SLOT(setEnabled(bool)));
	
	connect(customBorderColor, SIGNAL(toggled(bool)), borderColor, SLOT(setEnabled(bool)));
	connect(customButtonColor, SIGNAL(toggled(bool)), buttonColor, SLOT(setEnabled(bool)));
	connect(customButtonIconColor, SIGNAL(toggled(bool)), buttonIconColor, SLOT(setEnabled(bool)));
	
	connect(customGradientColors, SIGNAL(clicked()), this, SLOT(slotSelectionChanged()));
	connect(topGradientColor, SIGNAL(changed(const QColor&)), this, SLOT(slotSelectionChanged()));
	connect(bottomGradientColor, SIGNAL(changed(const QColor&)), this, SLOT(slotSelectionChanged()));
	
	
	connect(customBorderColor, SIGNAL(clicked()), this, SLOT(slotSelectionChanged()));
	connect(borderColor, SIGNAL(changed(const QColor&)), this, SLOT(slotSelectionChanged()));
	
	connect(customButtonColor, SIGNAL(clicked()), this, SLOT(slotSelectionChanged()));
	connect(buttonColor, SIGNAL(changed(const QColor&)), this, SLOT(slotSelectionChanged()));
	connect(customButtonIconColor, SIGNAL(clicked()), this, SLOT(slotSelectionChanged()));
	connect(buttonIconColor, SIGNAL(changed(const QColor&)), this, SLOT(slotSelectionChanged()));
	
	connect(showInactiveButtons, SIGNAL(clicked()), this, SLOT(slotSelectionChanged()));
	connect(showButtonIcons, SIGNAL(clicked()), this, SLOT(slotSelectionChanged()));
	connect(darkFrame, SIGNAL(clicked()), this, SLOT(slotSelectionChanged()));
	
	connect(useDominoStyleContourColors, SIGNAL(clicked()), this, SLOT(loadButtonContourColors()));
	connect(useDominoStyleContourColors, SIGNAL(clicked()), this, SLOT(slotSelectionChanged()));
	connect(buttonContourColor, SIGNAL(changed(const QColor&)), this, SLOT(slotSelectionChanged()));
	connect(buttonMouseOverContourColor, SIGNAL(changed(const QColor&)), this, SLOT(slotSelectionChanged()));
	connect(buttonPressedContourColor, SIGNAL(changed(const QColor&)), this, SLOT(slotSelectionChanged()));
	
	
	// Make the widgets visible in kwindecoration valueChanged
	vBox->show();
}


DominoConfig::~DominoConfig()
{
	delete vBox;
	delete conf;
}


void DominoConfig::slotSelectionChanged()
{
	emit changed();
}


// Loads the configurable options from the kwinrc config file
// It is passed the open config from kwindecoration to improve efficiency
void DominoConfig::load(KConfig*)
{
	conf->setGroup("General");
	
	customBorderColor->setChecked(conf->readBoolEntry("customBorderColor", true));
	borderColor->setEnabled(customBorderColor->isChecked());
	borderColor->setColor(conf->readEntry("borderColor", "#777B7F"));
	customButtonColor->setChecked(conf->readBoolEntry( "customButtonColor", false));
	buttonColor->setEnabled(customButtonColor->isChecked());
	buttonColor->setColor(conf->readEntry("buttonColor", "#212121"));
	customButtonIconColor->setChecked(conf->readBoolEntry( "customButtonIconColor", false));
	buttonIconColor->setEnabled(customButtonIconColor->isChecked());
	buttonIconColor->setColor(conf->readEntry("buttonIconColor", "#000000"));
	
	customGradientColors->setChecked(conf->readBoolEntry( "customGradientColors", false));
	topGradientColor->setEnabled(customGradientColors->isChecked());
	bottomGradientColor->setEnabled(customGradientColors->isChecked());
	topGradientColor->setColor(conf->readEntry("topGradientColor", "#ffffff"));
	bottomGradientColor->setColor(conf->readEntry("bottomGradientColor", "#000000"));
	
	showInactiveButtons->setChecked(conf->readBoolEntry("showInactiveButtons", false));
	showButtonIcons->setChecked(conf->readBoolEntry("showButtonIcons", false));
	darkFrame->setChecked(conf->readBoolEntry("darkFrame", false));
	
	useDominoStyleContourColors->setChecked(conf->readBoolEntry("useDominoStyleContourColors", true));
	
	loadButtonContourColors();
	
}


// Saves the configurable options to the kwinrc config file
void DominoConfig::save(KConfig*)
{
	conf->setGroup("General");
	
	conf->writeEntry("customBorderColor", customBorderColor->isChecked());
	conf->writeEntry("borderColor", borderColor->color().name());
	conf->writeEntry("customButtonColor", customButtonColor->isChecked());
	conf->writeEntry("buttonColor", buttonColor->color().name());
	conf->writeEntry("customButtonIconColor", customButtonIconColor->isChecked());
	conf->writeEntry("buttonIconColor", buttonIconColor->color().name());
	conf->writeEntry("showInactiveButtons", showInactiveButtons->isChecked());
	conf->writeEntry("showButtonIcons", showButtonIcons->isChecked());
	
	conf->writeEntry("customGradientColors", customGradientColors->isChecked());
	conf->writeEntry("topGradientColor", topGradientColor->color().name());
	conf->writeEntry("bottomGradientColor", bottomGradientColor->color().name());
	conf->writeEntry("darkFrame", darkFrame->isChecked());
	
	conf->writeEntry("useDominoStyleContourColors", useDominoStyleContourColors->isChecked());
	conf->writeEntry("buttonContourColor", buttonContourColor->color().name());
	conf->writeEntry("buttonMouseOverContourColor", buttonMouseOverContourColor->color().name());
	conf->writeEntry("buttonPressedContourColor", buttonPressedContourColor->color().name());
	
	conf->sync();
}


// Sets UI widget defaults which must correspond to style defaults
void DominoConfig::defaults()
{
	
	QColor background = vBox->paletteBackgroundColor();
	
	customBorderColor->setChecked(true);
	borderColor->setColor("#777B7F");
	customButtonColor->setChecked(false);
	buttonColor->setColor("#212121");
	customButtonIconColor->setChecked(false);
	buttonIconColor->setColor("#000000");
	showInactiveButtons->setChecked(false);
	showButtonIcons->setChecked(false);
	customGradientColors->setChecked(false);
	topGradientColor->setColor("#ffffff");
	bottomGradientColor->setColor("#000000");
	darkFrame->setChecked(false);
	
	useDominoStyleContourColors->setChecked(true);
	buttonContourColor->setColor(background.dark(250).name());
	buttonMouseOverContourColor->setColor(background.dark(250).name());
	buttonPressedContourColor->setColor(background.dark(250).name());
	
}

void DominoConfig::loadButtonContourColors() {
	
	QColor background = vBox->paletteBackgroundColor();
	if(useDominoStyleContourColors->isChecked()) {
		QSettings s;
		buttonContourColor->setColor(s.readEntry("/domino/Settings/buttonContourColor", background.dark(250).name()));
		buttonMouseOverContourColor->setColor(s.readEntry("/domino/Settings/buttonMouseOverContourColor", background.dark(250).name()));
		buttonPressedContourColor->setColor(s.readEntry("/domino/Settings/buttonPressedContourColor", background.dark(250).name()));
	}
	else {
		buttonContourColor->setColor(conf->readEntry("buttonContourColor", background.dark(250).name()));
		buttonMouseOverContourColor->setColor(conf->readEntry("buttonMouseOverContourColor", background.dark(250).name()));
		buttonPressedContourColor->setColor(conf->readEntry("buttonPressedContourColor", background.dark(250).name()));
	}
	
}

#include "config.moc"

