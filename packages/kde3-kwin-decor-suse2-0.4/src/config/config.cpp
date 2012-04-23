/* SUSE2 KWin window decoration
  Copyright (C) 2005 Gerd Fleischer <gerdfleischer@web.de>

  based on the window decoration "Plastik"
  Copyright (C) 2003 Sandro Giessl <ceebx@users.sourceforge.net>

  based on the window decoration "Web":
  Copyright (C) 2001 Rik Hemsley (rikkus) <rik@kde.org>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; see the file COPYING.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
  Boston, MA  02110-1301  USA.
*/

//#include <kdebug.h>

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qlabel.h>
#include <qcombobox.h>

#include <kdeversion.h>
#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>
#include <kurlrequester.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kfiledialog.h>
#include <knuminput.h>
#include <kfileitem.h>
#include <kcolorbutton.h>

#include "config.h"
#include "configdialog.h"

SUSE2Config::SUSE2Config(KConfig *config, QWidget *parent)
    : QObject(parent), m_config(0), m_dialog(0)
{
    m_parent = parent;
    // create the configuration object
    m_config = new KConfig("kwinSUSE2rc");
    KGlobal::locale()->insertCatalogue("kwin_clients");
    KGlobal::locale()->insertCatalogue("kwin_SUSE2");

    // create and show the configuration dialog
    m_dialog = new ConfigDialog(parent);

    m_dialog->show();

    // load the configuration
    load(config);
    toggleIconSettings(m_dialog->useTitleProps->isChecked());

    // setup the connections
    connect(m_dialog->titleAlign, SIGNAL(clicked(int)), SIGNAL(changed()));
    connect(m_dialog->roundCorners, SIGNAL(clicked(int)), SIGNAL(changed()));
    connect(m_dialog->menuClose, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(m_dialog->titleShadow, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(m_dialog->addSpace, SIGNAL(valueChanged(int)), SIGNAL(changed()));
    connect(m_dialog->titlebarStyle, SIGNAL(activated(int)), SIGNAL(changed()));

    connect(m_dialog->buttonType, SIGNAL(activated(int)), SIGNAL(changed()));
    connect(m_dialog->customColors, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(m_dialog->useTitleProps, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(m_dialog->animateButtons, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(m_dialog->redCloseButton, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(m_dialog->iconSize, SIGNAL(valueChanged(int)), SIGNAL(changed()));
    connect(m_dialog->customIconColors, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(m_dialog->activeFgColor, SIGNAL(changed(const QColor &)), SIGNAL(changed()));
    connect(m_dialog->activeBgColor, SIGNAL(changed(const QColor &)), SIGNAL(changed()));
    connect(m_dialog->inactiveFgColor, SIGNAL(changed(const QColor &)), SIGNAL(changed()));
    connect(m_dialog->inactiveBgColor, SIGNAL(changed(const QColor &)), SIGNAL(changed()));
    connect(m_dialog->iconShadow, SIGNAL(toggled(bool)), SIGNAL(changed()));

    connect(m_dialog->titleBarLogo, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(m_dialog->titleBarLogoOffset, SIGNAL(valueChanged(int)), SIGNAL(changed()));

    connect(m_dialog->useTitleProps, SIGNAL(toggled(bool)), SLOT(toggleIconSettings(bool)));
    connect(m_dialog->selectButton, SIGNAL(clicked()), SLOT(selectImage()));
}

SUSE2Config::~SUSE2Config()
{
    if (m_dialog) delete m_dialog;
    if (m_config) delete m_config;
}

void SUSE2Config::load(KConfig *)
{
    m_config->setGroup("General");

    QString alignValue = m_config->readEntry("TitleAlignment", "AlignLeft");
    QRadioButton *alignButton = (QRadioButton*)m_dialog->titleAlign->child(alignValue.latin1());
    if (alignButton) alignButton->setChecked(true);

    QString roundValue = m_config->readEntry("RoundCorners", "NotMaximized");
    QRadioButton *roundButton = (QRadioButton*)m_dialog->roundCorners->child(roundValue.latin1());
    if (roundButton) roundButton->setChecked(true);

    bool menuClose = m_config->readBoolEntry("CloseOnMenuDoubleClick", true);
    m_dialog->menuClose->setChecked(menuClose);

    bool titleShadow = m_config->readBoolEntry("TitleShadow", true);
    m_dialog->titleShadow->setChecked(titleShadow);

    int addSpace = m_config->readNumEntry("AddSpace", 4);
    m_dialog->addSpace->setValue(addSpace);

    int titlebarStyle = m_config->readNumEntry("TitleBarStyle", 0);
    m_dialog->titlebarStyle->setCurrentItem(titlebarStyle);

    int titleButtonType = m_config->readNumEntry("TitleBarButtonType", 0);
    m_dialog->buttonType->setCurrentItem(titleButtonType);

    bool customColors = m_config->readBoolEntry("CustomColors", false);
    m_dialog->customColors->setChecked(customColors);

    bool useTitleProps = m_config->readBoolEntry("UseTitleProps", false);
    m_dialog->useTitleProps->setChecked(useTitleProps);

    bool animateButtons = m_config->readBoolEntry("AnimateButtons", true);
    m_dialog->animateButtons->setChecked(animateButtons);

    bool redCloseButton = m_config->readBoolEntry("RedCloseButton", false);
    m_dialog->redCloseButton->setChecked(redCloseButton);

    int iconSize = m_config->readNumEntry("IconSize", 45);
    m_dialog->iconSize->setValue(iconSize);

    bool customIconColors = m_config->readBoolEntry("CustomIconColors", false);
    m_dialog->customIconColors->setChecked(customIconColors);

    QColor afgcolor = QColor(10, 20, 40);
    QColor aFgColor = m_config->readColorEntry("AFgColor", &afgcolor);
    m_dialog->activeFgColor->setColor(aFgColor);

    QColor abgcolor = QColor(210, 220, 240);
    QColor aBgColor = m_config->readColorEntry("ABgColor", &abgcolor);
    m_dialog->activeBgColor->setColor(aBgColor);

    QColor ifgcolor = QColor(40, 40, 40);
    QColor iFgColor = m_config->readColorEntry("IFgColor", &ifgcolor);
    m_dialog->inactiveFgColor->setColor(iFgColor);

    QColor ibgcolor = QColor(240, 240, 240);
    QColor iBgColor = m_config->readColorEntry("AFgColor", &ibgcolor);
    m_dialog->inactiveBgColor->setColor(iBgColor);

    bool iconShadow = m_config->readBoolEntry("IconShadow", false);
    m_dialog->iconShadow->setChecked(iconShadow);

    bool titleBarLogo = m_config->readBoolEntry("TitleBarLogo", true);
    m_dialog->titleBarLogo->setChecked(titleBarLogo);

    int titleBarLogoOffset = m_config->readNumEntry("TitleBarLogoOffset", 3);
    m_dialog->titleBarLogoOffset->setValue(titleBarLogoOffset);

    QString titleBarImage = locate("data", "kwin/pics/titlebar_decor.png");
    titlebarLogoURL = m_config->readEntry("TitleBarLogoURL", titleBarImage);
    QImage tmpLogo = QImage::QImage(titlebarLogoURL);
    m_dialog->logoImage->setPixmap(QPixmap(tmpLogo.smoothScale(120, 20, QImage::ScaleMin)));

}

void SUSE2Config::save(KConfig *) const
{
    m_config->setGroup("General");

    QRadioButton *alignButton = (QRadioButton*)m_dialog->titleAlign->selected();
    if (alignButton) m_config->writeEntry("TitleAlignment", QString(alignButton->name()));
    QRadioButton *roundButton = (QRadioButton*)m_dialog->roundCorners->selected();
    if (roundButton) m_config->writeEntry("RoundCorners", QString(roundButton->name()));
    m_config->writeEntry("CloseOnMenuDoubleClick", m_dialog->menuClose->isChecked());
    m_config->writeEntry("TitleShadow", m_dialog->titleShadow->isChecked());
    m_config->writeEntry("AddSpace", m_dialog->addSpace->value());
    m_config->writeEntry("TitleBarStyle", m_dialog->titlebarStyle->currentItem());

    m_config->writeEntry("TitleBarButtonType", m_dialog->buttonType->currentItem());
    m_config->writeEntry("CustomColors", m_dialog->customColors->isChecked());
    m_config->writeEntry("UseTitleProps", m_dialog->useTitleProps->isChecked());
    m_config->writeEntry("AnimateButtons", m_dialog->animateButtons->isChecked());
    m_config->writeEntry("RedCloseButton", m_dialog->redCloseButton->isChecked());
    m_config->writeEntry("IconSize", m_dialog->iconSize->value());
    m_config->writeEntry("CustomIconColors", m_dialog->customIconColors->isChecked());
    m_config->writeEntry("AFgColor", m_dialog->activeFgColor->color());
    m_config->writeEntry("ABgColor", m_dialog->activeBgColor->color());
    m_config->writeEntry("IFgColor", m_dialog->inactiveFgColor->color());
    m_config->writeEntry("IBgColor", m_dialog->inactiveBgColor->color());
    m_config->writeEntry("IconShadow", m_dialog->iconShadow->isChecked());

    m_config->writeEntry("TitleBarLogo", m_dialog->titleBarLogo->isChecked());
    m_config->writeEntry("TitleBarLogoOffset", m_dialog->titleBarLogoOffset->value());
    m_config->writeEntry("TitleBarLogoURL", QString(titlebarLogoURL));
    m_config->sync();
}

void SUSE2Config::defaults()
{
    QRadioButton *alignButton = (QRadioButton*)m_dialog->titleAlign->child("AlignHCenter");
    if (alignButton) alignButton->setChecked(true);
    QRadioButton *roundButton = (QRadioButton*)m_dialog->roundCorners->child("NotMaximized");
    if (roundButton) roundButton->setChecked(true);
    m_dialog->menuClose->setChecked(false);
    m_dialog->titleShadow->setChecked(true);
    m_dialog->addSpace->setValue(4);
    m_dialog->titlebarStyle->setCurrentItem(0);

    m_dialog->buttonType->setCurrentItem(0);
    m_dialog->customColors->setChecked(false);
    m_dialog->useTitleProps->setChecked(false);
    m_dialog->animateButtons->setChecked(true);
    m_dialog->redCloseButton->setChecked(false);
    m_dialog->iconSize->setValue(45);
    m_dialog->customIconColors->setChecked(false);
    m_dialog->activeFgColor->setColor(QColor(10, 20, 40));
    m_dialog->activeBgColor->setColor(QColor(210, 220, 240));
    m_dialog->inactiveFgColor->setColor(QColor(40, 40, 40));
    m_dialog->inactiveBgColor->setColor(QColor(240, 240, 240));
    m_dialog->iconShadow->setChecked(false);

    m_dialog->titleBarLogo->setChecked(true);
    m_dialog->titleBarLogoOffset->setValue(3);
    titlebarLogoURL = locate("data", "kwin/pics/titlebar_decor.png");
    QImage tmpLogo = QImage::QImage(titlebarLogoURL);
    m_dialog->logoImage->setPixmap(QPixmap(tmpLogo.smoothScale(120, 20, QImage::ScaleMin)));
}

void SUSE2Config::toggleIconSettings(bool checked) const
{
    m_dialog->customIconColors->setEnabled(!checked);
    m_dialog->activeLbl->setEnabled(!checked);
    m_dialog->activeDownLbl->setEnabled(!checked);
    m_dialog->inactiveLbl->setEnabled(!checked);
    m_dialog->inactiveDownLbl->setEnabled(!checked);
    m_dialog->activeFgColor->setEnabled(!checked);
    m_dialog->activeBgColor->setEnabled(!checked);
    m_dialog->inactiveFgColor->setEnabled(!checked);
    m_dialog->inactiveBgColor->setEnabled(!checked);
    m_dialog->iconShadow->setEnabled(!checked);
}

void SUSE2Config::selectImage()
{
    KURL logoURL = KFileDialog::getImageOpenURL(titlebarLogoURL, m_parent, i18n("Select Logo Image"));
    KFileItem tmpFileItem = KFileItem(KFileItem::Unknown, KFileItem::Unknown, logoURL);
    if (!logoURL.isEmpty() && tmpFileItem.isFile() && tmpFileItem.isReadable()) {
        titlebarLogoURL = logoURL.path();
        QImage tmpLogo = QImage::QImage(titlebarLogoURL);
        m_dialog->logoImage->setPixmap(QPixmap(tmpLogo.smoothScale(120, 20, QImage::ScaleMin)));
        emit changed();
    }
}

//////////////////////////////////////////////////////////////////////////////
// Plugin Stuff                                                             //
//////////////////////////////////////////////////////////////////////////////

extern "C"
{
    KDE_EXPORT QObject *allocate_config(KConfig *config, QWidget *parent) {
        return (new SUSE2Config(config, parent));
    }
}

#include "config.moc"

// kate: space-indent on; indent-width 4; replace-tabs on;
