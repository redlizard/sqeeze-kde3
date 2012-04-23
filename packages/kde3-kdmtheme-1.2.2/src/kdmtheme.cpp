/***************************************************************************
 *   Copyright (C) 2005-2007 by Stephen Leaf <smileaf@gmail.com>           *
 *   Copyright (C) 2006 by Oswald Buddenhagen <ossi@kde.org>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#include "kdmtheme.h"

#include <kdialog.h>
#include <kglobal.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprogress.h>
#include <kstandarddirs.h>
#include <ktar.h>
#include <kurlrequester.h>
#include <kurlrequesterdlg.h>
#include <kparts/genericfactory.h>

#include <qcheckbox.h>
#include <qdir.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlist.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qlistview.h>
#include <qwhatsthis.h>
#include <qwidget.h>

#include <unistd.h>

class ThemeData : public QListViewItem {
  public:
	ThemeData( QListView *parent = 0 ) : QListViewItem( parent ) {}

	QString name;
	QString path;
	QString screenShot;
	QString copyright;
	QString description;
};

typedef KGenericFactory<KDMThemeWidget, QWidget> kdmthemeFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kdmtheme, kdmthemeFactory("kcmkdmtheme"))

KDMThemeWidget::KDMThemeWidget( QWidget *parent, const char *name, const QStringList& )
	: KCModule(parent, name), config( 0L )
{
	QGridLayout *ml = new QGridLayout( this );
	ml->setSpacing( KDialog::spacingHint() );
	ml->setMargin( KDialog::marginHint() );

	cUseTheme = new QCheckBox( this );
	cUseTheme->setText( i18n( "En&able KDM Themes" ) );
	ml->addMultiCellWidget( cUseTheme, 0, 0, 0, 2 );

	themeWidget = new QListView( this );
    themeWidget->addColumn( i18n( "Theme" ) );
    themeWidget->addColumn( i18n( "Author" ) );
	themeWidget->setAllColumnsShowFocus( true );
	themeWidget->setShowSortIndicator( true );
	themeWidget->setRootIsDecorated( false );
	QWhatsThis::add( themeWidget, i18n("This is a list of installed themes.\n"
	                                "Click the one to be used.") );

	ml->addMultiCellWidget( themeWidget, 1, 3, 0, 1 );

	preview = new QLabel( this );
	preview->setFixedSize( QSize( 200, 150 ) );
	preview->setScaledContents( true );
	QWhatsThis::add(preview, i18n("This is a screen shot of what KDM will look like.") );

	ml->addWidget( preview, 1, 2 );

	info = new QLabel( this );
	info->setMaximumWidth( 200 );
	info->setAlignment( int( QLabel::AlignTop | QLabel::WordBreak) );
	QWhatsThis::add(info, i18n("This contains information about the selected theme.") );

	ml->addMultiCellWidget( info, 3, 4, 2, 2 );

	bInstallTheme = new QPushButton( i18n("Install &new theme"), this );
	QWhatsThis::add(bInstallTheme, i18n("This will install a theme into the theme directory.") );

	ml->addWidget( bInstallTheme, 4, 0 );

	bRemoveTheme = new QPushButton( i18n("&Remove theme"), this );
	QWhatsThis::add(bRemoveTheme, i18n("This will remove the selected theme.") );

	ml->addWidget( bRemoveTheme, 4, 1 );

	connect( themeWidget, SIGNAL(selectionChanged()), SLOT(themeSelected()) );
	connect( bInstallTheme, SIGNAL(clicked()), SLOT(installNewTheme()) );
	connect( bRemoveTheme, SIGNAL(clicked()), SLOT(removeSelectedThemes()) );
	connect( cUseTheme, SIGNAL( toggled( bool ) ), SLOT( toggleUseTheme( bool ) ) );

	QStringList themeDirs = KGlobal::dirs()->findDirs("data","kdm/");
	themeDir = KGlobal::dirs()->findDirs("data","kdm/").last();
	QDir d;
	for ( QStringList::Iterator dirs = themeDirs.begin(); dirs != themeDirs.end(); ++dirs ) {
		kdDebug() << "Loading themes... ( " + *dirs + "themes/" + " )" << endl;
		d.setPath( *dirs + "themes/" );
		d.setFilter( QDir::Dirs );
		QStringList list = d.entryList();
		for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
			if (*it == "." || *it == "..") continue;
			insertTheme( *dirs + "themes/" + *it );
		}
	}

	load();

	if (getuid() != 0) {
		cUseTheme->setEnabled( false );
		setReadOnly(false);
	}
}

void KDMThemeWidget::selectTheme( const QString &path )
{
	for ( QListViewItemIterator tdi( themeWidget ); tdi.current(); tdi++ ) {
		ThemeData * td = ((ThemeData *)*tdi);
		if (td->path == path) {
			themeWidget->clearSelection();
			themeWidget->setSelected(td, true );
			updateInfoView( td );
		}
	}
}

void KDMThemeWidget::load()
{
	QString kdmrc = KGlobal::dirs()->findResource("config", "kdm/kdmrc");
	if ( kdmrc.isEmpty() ) {
		kdError() << "Failed to find kdm resource file kdmrc!" << endl;

		KMessageBox msgBox;
		msgBox.sorry(0, i18n("I can't find the configuration file for the Login Manager, kdm. Check you have set up the Login Manager from the Control Center, and that you have permision to read and write the Login Manager's configuration file.\n\n You must set-up the Login Manager before you can use its themes."), i18n("Can't find Configuration File"));

		delete config;
		config = NULL;
	} else {
		kdDebug() << "Loading... ( " + kdmrc + " )" << endl;

		delete config;
		config = new KConfig( kdmrc );
		config->setGroup( "X-*-Greeter" );
		cUseTheme->setChecked( config->readBoolEntry( "UseTheme", false ) );

		selectTheme( config->readEntry( "Theme", themeDir + "circles" ) );
	}
}

void KDMThemeWidget::save()
{
	kdDebug() << "Saving: Theme " << defaultTheme->path << " - UseTheme " << cUseTheme->isChecked() << endl;
	config->writeEntry( "Theme", defaultTheme->path );
	config->writeEntry( "UseTheme", cUseTheme->isChecked() );
	config->sync();
}

void KDMThemeWidget::defaults()
{
	selectTheme( themeDir + "circles" );

	emit changed(true);
}

void KDMThemeWidget::toggleUseTheme(bool useTheme) {
	setReadOnly( useTheme );
	emit changed(true);
}

void KDMThemeWidget::setReadOnly(bool ro)
{
	themeWidget->setEnabled( ro );
	bInstallTheme->setEnabled( ro );
	bRemoveTheme->setEnabled( ro );
}

void KDMThemeWidget::insertTheme( const QString &_theme )
{
	KConfig * themeConfig;
	QString name;

	kdDebug() << "Looking for " << _theme << "/KdmGreeterTheme.desktop" << endl;
	themeConfig = new KConfig( _theme + "/KdmGreeterTheme.desktop");
	themeConfig->setGroup( "GdmGreeterTheme" );
	name = themeConfig->readEntry( "Name" );
	if (name.isEmpty()) {
		kdDebug() << "Looking for " << _theme << "/GdmGreeterTheme.desktop" << endl;
		themeConfig = new KConfig( _theme + "/GdmGreeterTheme.desktop");
		themeConfig->setGroup( "GdmGreeterTheme" );

		name = themeConfig->readEntry( "Name" );
		if (name.isEmpty())
			return;
	}

	for ( QListViewItemIterator tdi( themeWidget ); tdi.current(); tdi++ ) {
		ThemeData * td = ((ThemeData *)*tdi);
		if (td->name == name)
			return;
	}
	kdDebug() << "Adding theme " << name << endl;

	ThemeData *child = new ThemeData( themeWidget );
	child->setText( 0, name );
	child->setText( 1, themeConfig->readEntry( "Author" ) );
	child->name = name;
	child->path = _theme;
	child->screenShot = themeConfig->readEntry( "Screenshot" );
	child->copyright = themeConfig->readEntry( "Copyright" );
	child->description = themeConfig->readEntry( "Description" );
}

void KDMThemeWidget::updateInfoView( ThemeData *theme )
{
	info->setText(
		((theme->copyright.length() > 0) ?
			i18n("<qt><strong>Copyright:</strong> %1<br/></qt>",
				theme->copyright) : "") +
		((theme->description.length() > 0) ?
			i18n("<qt><strong>Description:</strong> %1</qt>",
				theme->description) : "") );
	preview->setPixmap( theme->path + '/' + theme->screenShot );
	preview->setText( theme->screenShot.isEmpty() ?
		"Screenshot not available" : QString() );
}

void KDMThemeWidget::installNewTheme()
{
	KURLRequesterDlg fileRequester( QString::null, this, i18n("Drag or Type Theme URL") );
	fileRequester.urlRequester()->setMode( KFile::File | KFile::Directory | KFile::ExistingOnly );
	KURL themeURL = fileRequester.getURL();
	if (themeURL.isEmpty())
		return;

	QString themeTmpFile;

	if (!KIO::NetAccess::download( themeURL, themeTmpFile, this )) {
		QString sorryText;
		if (themeURL.isLocalFile())
			sorryText = i18n("Unable to find the KDM theme archive %1.",themeURL.prettyURL());
		else
			sorryText = i18n("Unable to download the KDM theme archive;\n"
			                 "please check that address %1 is correct.",themeURL.prettyURL());
		KMessageBox::sorry( this, sorryText );
		return;
	}

	QList<KArchiveDirectory> foundThemes;

	KTar archive( themeTmpFile );
	archive.open( IO_ReadOnly );

	const KArchiveDirectory *archDir = archive.directory();
	QStringList entries = archDir->entries();
	for (QStringList::Iterator ent = entries.begin(); ent != entries.end(); ++ent) {
		const KArchiveEntry *possibleDir = archDir->entry( *ent );
		if (possibleDir->isDirectory()) {
			const KArchiveDirectory *subDir =
				static_cast<const KArchiveDirectory *>( possibleDir );
			if (subDir->entry( "KdmGreeterTheme.desktop" ))
				foundThemes.append( subDir );
			else if (subDir->entry( "GdmGreeterTheme.desktop" ))
				foundThemes.append( subDir );
		}
	}

	if (foundThemes.isEmpty())
		KMessageBox::error( this, i18n("The file is not a valid KDM theme archive.") );
	else {
		KProgressDialog progressDiag( this,
			i18n("Installing KDM themes"), QString() );
		progressDiag.setModal( true );
		progressDiag.setAutoClose( true );
		progressDiag.progressBar()->setTotalSteps( foundThemes.count() );
		progressDiag.show();

		for ( KArchiveDirectory * ard = foundThemes.first(); foundThemes.current(); foundThemes.next() ) {
			progressDiag.setLabel(
				i18n("<qt>Installing <strong>%1</strong> theme</qt>", ard->name() ) );

			QString path = themeDir + "themes/" + ard->name();
			kdDebug() << "Unpacking new theme to " << path << endl;
			ard->copyTo( path, true );
			if (QDir( path ).exists())
				insertTheme( path );

			progressDiag.progressBar()->setValue( progressDiag.progressBar()->value() + 1 );
			if (progressDiag.wasCancelled())
				break;
		}
		emit changed(true);
	}

	archive.close();

	KIO::NetAccess::removeTempFile( themeTmpFile );
}

void KDMThemeWidget::themeSelected()
{
	if (themeWidget->selectedItem()) {
		defaultTheme = (ThemeData *)(themeWidget->selectedItem());
		updateInfoView( defaultTheme );
	} else
		updateInfoView( 0 );
	emit changed(true);
}

void KDMThemeWidget::removeSelectedThemes()
{
	QListViewItem * themes = themeWidget->selectedItem();
	if (!themes)
		return;
	if (KMessageBox::questionYesNoList( this,
			i18n("Are you sure you want to remove this KDM theme?"),
			themes->text(0), i18n("Remove theme?") ) != KMessageBox::Yes)
		return;
	KIO::del( ((ThemeData *)themes)->path ); // XXX error check

	themeWidget->takeItem( themes );
}

#include "kdmtheme.moc"
