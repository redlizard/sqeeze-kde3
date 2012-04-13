/*
  This file is part of the KDE Display Manager Configuration package
  Copyright (C) 1997-1998 Thomas Tanghus (tanghus@earthling.net)

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/


#include <unistd.h>
#include <sys/types.h>


#include <qbuttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qwhatsthis.h>
#include <qvalidator.h>
#include <qstylefactory.h>
#include <qstyle.h>

#include <klocale.h>
#include <klineedit.h>
#include <kimageio.h>
#include <kcombobox.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>
#include <kiconloader.h>
#include <kurldrag.h>
#include <kimagefilepreview.h>

#include "kdm-appear.h"
#include "kbackedcombobox.h"

extern KSimpleConfig *config;


KDMAppearanceWidget::KDMAppearanceWidget(QWidget *parent, const char *name)
  : QWidget(parent, name)
{
  QString wtstr;

  QVBoxLayout *vbox = new QVBoxLayout(this, KDialog::marginHint(),
                      KDialog::spacingHint(), "vbox");
  QGroupBox *group = new QGroupBox(i18n("Appearance"), this);
  vbox->addWidget(group);

  QGridLayout *grid = new QGridLayout( group, 5, 2, KDialog::marginHint(),
                       KDialog::spacingHint(), "grid");
  grid->addRowSpacing(0, group->fontMetrics().height());
  grid->setColStretch(0, 1);
  grid->setColStretch(1, 1);

  QHBoxLayout *hlay = new QHBoxLayout( KDialog::spacingHint() );
  grid->addMultiCellLayout(hlay, 1,1, 0,1);
  greetstr_lined = new KLineEdit(group);
  QLabel *label = new QLabel(greetstr_lined, i18n("&Greeting:"), group);
  hlay->addWidget(label);
  connect(greetstr_lined, SIGNAL(textChanged(const QString&)),
      SLOT(changed()));
  hlay->addWidget(greetstr_lined);
  wtstr = i18n("This is the \"headline\" for KDM's login window. You may want to "
           "put some nice greeting or information about the operating system here.<p>"
           "KDM will substitute the following character pairs with the "
           "respective contents:<br><ul>"
           "<li>%d -> current display</li>"
           "<li>%h -> host name, possibly with domain name</li>"
           "<li>%n -> node name, most probably the host name without domain name</li>"
           "<li>%s -> the operating system</li>"
           "<li>%r -> the operating system's version</li>"
           "<li>%m -> the machine (hardware) type</li>"
           "<li>%% -> a single %</li>"
           "</ul>" );
  QWhatsThis::add( label, wtstr );
  QWhatsThis::add( greetstr_lined, wtstr );


  QGridLayout *hglay = new QGridLayout( 3, 4, KDialog::spacingHint() );
  grid->addMultiCellLayout(hglay, 2,4, 0,0);

  label = new QLabel(i18n("Logo area:"), group);
  hglay->addWidget(label, 0, 0);
  QVBoxLayout *vlay = new QVBoxLayout( KDialog::spacingHint() );
  hglay->addMultiCellLayout(vlay, 0,0, 1,2);
  noneRadio = new QRadioButton( i18n("logo area", "&None"), group );
  clockRadio = new QRadioButton( i18n("Show cloc&k"), group );
  logoRadio = new QRadioButton( i18n("Sho&w logo"), group );
  QButtonGroup *buttonGroup = new QButtonGroup( group );
  label->setBuddy( buttonGroup );
  connect( buttonGroup, SIGNAL(clicked(int)), SLOT(slotAreaRadioClicked(int)) );
  connect( buttonGroup, SIGNAL(clicked(int)), SLOT(changed()) );
  buttonGroup->hide();
  buttonGroup->insert(noneRadio, KdmNone);
  buttonGroup->insert(clockRadio, KdmClock);
  buttonGroup->insert(logoRadio, KdmLogo);
  vlay->addWidget(noneRadio);
  vlay->addWidget(clockRadio);
  vlay->addWidget(logoRadio);
  wtstr = i18n("You can choose to display a custom logo (see below), a clock or no logo at all.");
  QWhatsThis::add( label, wtstr );
  QWhatsThis::add( noneRadio, wtstr );
  QWhatsThis::add( logoRadio, wtstr );
  QWhatsThis::add( clockRadio, wtstr );

  logoLabel = new QLabel(i18n("&Logo:"), group);
  logobutton = new QPushButton(group);
  logoLabel->setBuddy( logobutton );
  logobutton->setAutoDefault(false);
  logobutton->setAcceptDrops(true);
  logobutton->installEventFilter(this); // for drag and drop
  connect(logobutton, SIGNAL(clicked()), SLOT(slotLogoButtonClicked()));
  hglay->addWidget(logoLabel, 1, 0);
  hglay->addWidget(logobutton, 1, 1, AlignCenter);
  hglay->addRowSpacing(1, 110);
  wtstr = i18n("Click here to choose an image that KDM will display. "
	       "You can also drag and drop an image onto this button "
	       "(e.g. from Konqueror).");
  QWhatsThis::add( logoLabel, wtstr );
  QWhatsThis::add( logobutton, wtstr );
  hglay->addRowSpacing( 2, KDialog::spacingHint());
  hglay->setColStretch( 3, 1);


  hglay = new QGridLayout( 2, 3, KDialog::spacingHint() );
  grid->addLayout(hglay, 2, 1);

  label = new QLabel(i18n("Position:"), group);
  hglay->addMultiCellWidget(label, 0,1, 0,0, AlignVCenter);
  QValidator *posValidator = new QIntValidator(0, 100, group);
  QLabel *xLineLabel = new QLabel(i18n("&X:"), group);
  hglay->addWidget(xLineLabel, 0, 1);
  xLineEdit = new QLineEdit (group);
  connect( xLineEdit, SIGNAL( textChanged(const QString&) ), SLOT( changed() ));
  hglay->addWidget(xLineEdit, 0, 2);
  xLineLabel->setBuddy(xLineEdit);
  xLineEdit->setValidator(posValidator);
  QLabel *yLineLabel = new QLabel(i18n("&Y:"), group);
  hglay->addWidget(yLineLabel, 1, 1);
  yLineEdit = new QLineEdit (group);
  connect( yLineEdit, SIGNAL( textChanged(const QString&) ), SLOT( changed() ));
  hglay->addWidget(yLineEdit, 1, 2);
  yLineLabel->setBuddy(yLineEdit);
  yLineEdit->setValidator(posValidator);
  wtstr = i18n("Here you specify the relative coordinates (in percent) of the login dialog's <em>center</em>.");
  QWhatsThis::add( label, wtstr );
  QWhatsThis::add( xLineLabel, wtstr );
  QWhatsThis::add( xLineEdit, wtstr );
  QWhatsThis::add( yLineLabel, wtstr );
  QWhatsThis::add( yLineEdit, wtstr );
  hglay->setColStretch( 3, 1);
  hglay->setRowStretch( 2, 1);


  hglay = new QGridLayout( 2, 3, KDialog::spacingHint() );
  grid->addLayout(hglay, 3, 1);
  hglay->setColStretch(3, 1);

  guicombo = new KBackedComboBox(group);
  guicombo->insertItem( "", i18n("<default>") );
  loadGuiStyles(guicombo);
  guicombo->listBox()->sort();
  label = new QLabel(guicombo, i18n("GUI s&tyle:"), group);
  connect(guicombo, SIGNAL(activated(int)), SLOT(changed()));
  hglay->addWidget(label, 0, 0);
  hglay->addWidget(guicombo, 0, 1);
  wtstr = i18n("You can choose a basic GUI style here that will be "
        "used by KDM only.");
  QWhatsThis::add( label, wtstr );
  QWhatsThis::add( guicombo, wtstr );

  colcombo = new KBackedComboBox(group);
  colcombo->insertItem( "", i18n("<default>") );
  loadColorSchemes(colcombo);
  colcombo->listBox()->sort();
  label = new QLabel(colcombo, i18n("&Color scheme:"), group);
  connect(colcombo, SIGNAL(activated(int)), SLOT(changed()));
  hglay->addWidget(label, 1, 0);
  hglay->addWidget(colcombo, 1, 1);
  wtstr = i18n("You can choose a basic Color Scheme here that will be "
        "used by KDM only.");
  QWhatsThis::add( label, wtstr );
  QWhatsThis::add( colcombo, wtstr );

  echocombo = new KBackedComboBox(group);
  echocombo->insertItem("NoEcho", i18n("No Echo"));
  echocombo->insertItem("OneStar", i18n("One Star"));
  echocombo->insertItem("ThreeStars", i18n("Three Stars"));
  label = new QLabel(echocombo, i18n("Echo &mode:"), group);
  connect(echocombo, SIGNAL(activated(int)), SLOT(changed()));
  hglay->addWidget(label, 2, 0);
  hglay->addWidget(echocombo, 2, 1);
  wtstr = i18n("You can choose whether and how KDM shows your password when you type it.");
  QWhatsThis::add( label, wtstr );
  QWhatsThis::add( echocombo, wtstr );


  // The Language group box
  group = new QGroupBox(0, Vertical, i18n("Locale"), this);
  vbox->addWidget(group);

  langcombo = new KLanguageButton(group);
  loadLanguageList(langcombo);
  connect(langcombo, SIGNAL(activated(const QString &)), SLOT(changed()));
  label = new QLabel(langcombo, i18n("Languag&e:"), group);
  QGridLayout *hbox = new QGridLayout( group->layout(), 2, 2, KDialog::spacingHint() );
  hbox->setColStretch(1, 1);
  hbox->addWidget(label, 1, 0);
  hbox->addWidget(langcombo, 1, 1);
  wtstr = i18n("Here you can choose the language used by KDM. This setting does not affect"
    " a user's personal settings; that will take effect after login.");
  QWhatsThis::add( label, wtstr );
  QWhatsThis::add( langcombo, wtstr );


  vbox->addStretch(1);

}

void KDMAppearanceWidget::makeReadOnly()
{
    disconnect( logobutton, SIGNAL(clicked()),
		this, SLOT(slotLogoButtonClicked()) );
    logobutton->setAcceptDrops(false);
    greetstr_lined->setReadOnly(true);
    noneRadio->setEnabled(false);
    clockRadio->setEnabled(false);
    logoRadio->setEnabled(false);
    xLineEdit->setEnabled(false);
    yLineEdit->setEnabled(false);
    guicombo->setEnabled(false);
    colcombo->setEnabled(false);
    echocombo->setEnabled(false);
    langcombo->setEnabled(false);
}

void KDMAppearanceWidget::loadLanguageList(KLanguageButton *combo)
{
  QStringList langlist = KGlobal::dirs()->findAllResources("locale",
			QString::fromLatin1("*/entry.desktop"));
  langlist.sort();
  for ( QStringList::ConstIterator it = langlist.begin();
	it != langlist.end(); ++it )
  {
    QString fpath = (*it).left((*it).length() - 14);
    int index = fpath.findRev('/');
    QString nid = fpath.mid(index + 1);

    KSimpleConfig entry(*it);
    entry.setGroup(QString::fromLatin1("KCM Locale"));
    QString name = entry.readEntry(QString::fromLatin1("Name"), i18n("without name"));
    combo->insertLanguage(nid, name, QString::fromLatin1("l10n/"), QString::null);
  }
}

void KDMAppearanceWidget::loadColorSchemes(KBackedComboBox *combo)
{
  // XXX: Global + local schemes
  QStringList list = KGlobal::dirs()->
      findAllResources("data", "kdisplay/color-schemes/*.kcsrc", false, true);
  for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it)
  {
    KSimpleConfig config(*it, true);
    config.setGroup("Color Scheme");

    QString str;
    if (!(str = config.readEntry("Name")).isEmpty() ||
	!(str = config.readEntry("name")).isEmpty())
    {
	QString str2 = (*it).mid( (*it).findRev( '/' ) + 1 ); // strip off path
	str2.setLength( str2.length() - 6 ); // strip off ".kcsrc
        combo->insertItem( str2, str );
    }
  }
}

void KDMAppearanceWidget::loadGuiStyles(KBackedComboBox *combo)
{
  // XXX: Global + local schemes
  QStringList list = KGlobal::dirs()->
      findAllResources("data", "kstyle/themes/*.themerc", false, true);
  for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it)
  {
    KSimpleConfig config(*it, true);

    if (!(config.hasGroup("KDE") && config.hasGroup("Misc")))
	continue;

    config.setGroup("Desktop Entry");
    if (config.readBoolEntry("Hidden", false))
	continue;

    config.setGroup("KDE");
    QString str2 = config.readEntry("WidgetStyle");
    if (str2.isNull())
	continue;

    config.setGroup("Misc");
    combo->insertItem( str2, config.readEntry("Name") );
  }
}

bool KDMAppearanceWidget::setLogo(QString logo)
{
    QString flogo = logo.isEmpty() ?
                    locate("data", QString::fromLatin1("kdm/pics/kdelogo.png") ) :
                    logo;
    QImage p(flogo);
    if (p.isNull())
        return false;
    if (p.width() > 100 || p.height() > 100)
        p = p.smoothScale(100, 100, QImage::ScaleMin);
    logobutton->setPixmap(p);
    uint bd = style().pixelMetric( QStyle::PM_ButtonMargin ) * 2;
    logobutton->setFixedSize(p.width() + bd, p.height() + bd);
    logopath = logo;
    return true;
}


void KDMAppearanceWidget::slotLogoButtonClicked()
{
    KImageIO::registerFormats();
    KFileDialog dialogue(locate("data", QString::fromLatin1("kdm/pics/")),
			 KImageIO::pattern( KImageIO::Reading ),
			 this, 0, true);
    dialogue.setOperationMode( KFileDialog::Opening );
    dialogue.setMode( KFile::File | KFile::LocalOnly );

    KImageFilePreview* imagePreview = new KImageFilePreview( &dialogue );
    dialogue.setPreviewWidget( imagePreview );
    if (dialogue.exec() == QDialog::Accepted) {
	if ( setLogo(dialogue.selectedFile()) ) {
	    changed();
	}
    }
}


void KDMAppearanceWidget::slotAreaRadioClicked(int id)
{
    logobutton->setEnabled( id == KdmLogo );
    logoLabel->setEnabled( id == KdmLogo );
}


bool KDMAppearanceWidget::eventFilter(QObject *, QEvent *e)
{
  if (e->type() == QEvent::DragEnter) {
    iconLoaderDragEnterEvent((QDragEnterEvent *) e);
    return true;
  }

  if (e->type() == QEvent::Drop) {
    iconLoaderDropEvent((QDropEvent *) e);
    return true;
  }

  return false;
}

void KDMAppearanceWidget::iconLoaderDragEnterEvent(QDragEnterEvent *e)
{
  e->accept(KURLDrag::canDecode(e));
}


KURL *decodeImgDrop(QDropEvent *e, QWidget *wdg);

void KDMAppearanceWidget::iconLoaderDropEvent(QDropEvent *e)
{
    KURL pixurl;
    bool istmp;

    KURL *url = decodeImgDrop(e, this);
    if (url) {

	// we gotta check if it is a non-local file and make a tmp copy at the hd.
	if(!url->isLocalFile()) {
	    pixurl.setPath(KGlobal::dirs()->resourceDirs("data").last() +
		     "kdm/pics/" + url->fileName());
	    KIO::NetAccess::copy(*url, pixurl, parentWidget());
	    istmp = true;
	} else {
	    pixurl = *url;
	    istmp = false;
	}

	// By now url should be "file:/..."
	if (!setLogo(pixurl.path())) {
	    KIO::NetAccess::del(pixurl, parentWidget());
	    QString msg = i18n("There was an error loading the image:\n"
			       "%1\n"
			       "It will not be saved.")
			       .arg(pixurl.path());
	    KMessageBox::sorry(this, msg);
	}

	delete url;
    }
}


void KDMAppearanceWidget::save()
{
  config->setGroup("X-*-Greeter");

  config->writeEntry("GreetString", greetstr_lined->text());

  config->writeEntry("LogoArea", noneRadio->isChecked() ? "None" :
			    logoRadio->isChecked() ? "Logo" : "Clock" );

  config->writeEntry("LogoPixmap", KGlobal::iconLoader()->iconPath(logopath, KIcon::Desktop, true));

  config->writeEntry("GUIStyle", guicombo->currentId());

  config->writeEntry("ColorScheme", colcombo->currentId());

  config->writeEntry("EchoMode", echocombo->currentId());

  config->writeEntry("GreeterPos", xLineEdit->text() + ',' + yLineEdit->text());

  config->writeEntry("Language", langcombo->current());
}


void KDMAppearanceWidget::load()
{
  config->setGroup("X-*-Greeter");

  // Read the greeting string
  greetstr_lined->setText(config->readEntry("GreetString", i18n("Welcome to %s at %n")));

  // Regular logo or clock
  QString logoArea = config->readEntry("LogoArea", "Logo" );
  if (logoArea == "Clock") {
    clockRadio->setChecked(true);
    slotAreaRadioClicked(KdmClock);
  } else if (logoArea == "Logo") {
    logoRadio->setChecked(true);
    slotAreaRadioClicked(KdmLogo);
  } else {
    noneRadio->setChecked(true);
    slotAreaRadioClicked(KdmNone);
  }

  // See if we use alternate logo
  setLogo(config->readEntry("LogoPixmap"));

  // Check the GUI type
  guicombo->setCurrentId(config->readEntry("GUIStyle"));

  // Check the Color Scheme
  colcombo->setCurrentId(config->readEntry("ColorScheme"));

  // Check the echo mode
  echocombo->setCurrentId(config->readEntry("EchoMode", "OneStar"));

  QStringList sl = config->readListEntry( "GreeterPos" );
  if (sl.count() != 2) {
    xLineEdit->setText( "50" );
    yLineEdit->setText( "50" );
  } else {
    xLineEdit->setText( sl.first() );
    yLineEdit->setText( sl.last() );
  }

  // get the language
  langcombo->setCurrentItem(config->readEntry("Language", "C"));
}


void KDMAppearanceWidget::defaults()
{
  greetstr_lined->setText( i18n("Welcome to %s at %n") );
  logoRadio->setChecked( true );
  slotAreaRadioClicked( KdmLogo );
  setLogo( "" );
  guicombo->setCurrentId( "" );
  colcombo->setCurrentId( "" );
  echocombo->setCurrentItem( "OneStar" );

  xLineEdit->setText( "50" );
  yLineEdit->setText( "50" );

  langcombo->setCurrentItem( "en_US" );
}

QString KDMAppearanceWidget::quickHelp() const
{
  return i18n("<h1>KDM - Appearance</h1> Here you can configure the basic appearance"
    " of the KDM login manager, i.e. a greeting string, an icon etc.<p>"
    " For further refinement of KDM's appearance, see the \"Font\" and \"Background\" "
    " tabs.");
}


void KDMAppearanceWidget::changed()
{
  emit changed(true);
}

#include "kdm-appear.moc"
