/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "importdlg.h"
#include <stdlib.h>
#include <qcombobox.h>
#include <qdir.h>
#include <qfile.h>
#include <qlabel.h>
#include <qlayout.h>
#include <klineedit.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qtooltip.h>
#include <qcheckbox.h>
#include <kbuttonbox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kfiledialog.h>
#include <kinstance.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kcursor.h>
#include <kfile.h>
#include <kurlrequester.h>
#include <ktrader.h>
#include <kparts/componentfactory.h>
#include <kprocess.h>

#include "kdevcore.h"
#include "kdevversioncontrol.h"
#include "kdevplugincontroller.h"

#include "domutil.h"
#include "settings.h"
#include "profile.h"
#include "profileengine.h"

#include "appwizardfactory.h"
#include "appwizardpart.h"
#include "misc.h"


ImportDialog::ImportDialog(AppWizardPart *part, QWidget *parent, const char *name)
    : ImportDialogBase(parent, name, true), m_part(part)
{
    QString author, email;
    AppWizardUtil::guessAuthorAndEmail(&author, &email);
    author_edit->setText(author);
    email_edit->setText(email);
    QToolTip::add( urlinput_edit->button(), i18n("Choose directory to import") );
    urlinput_edit->setMode(KFile::Directory|KFile::ExistingOnly|KFile::LocalOnly);

    KStandardDirs *dirs = AppWizardFactory::instance()->dirs();
    importNames = dirs->findAllResources("appimports", QString::null, false, true);
    importNames.sort();

    QStringList::ConstIterator it;
    for (it = importNames.begin(); it != importNames.end(); ++it) {
        KConfig config(KGlobal::dirs()->findResource("appimports", *it));
        config.setGroup("General");
        QString type = config.readEntry("Comment");
        project_combo->insertItem(type);

        if (config.hasGroup("Infrastructure"))
        {
            config.setGroup("Infrastructure");
            m_infrastructure[type].isOn = true;
            m_infrastructure[type].comment = config.readEntry("Comment");
            m_infrastructure[type].command = config.readEntry("Command");
            m_infrastructure[type].existingPattern = config.readEntry("ExistingProjectPattern");
        }
        else
            m_infrastructure[type].isOn = false;
    }

    infrastructureBox->setEnabled(false);
    setProjectType("c");
    connect( name_edit, SIGNAL( textChanged ( const QString & ) ), this, SLOT( slotProjectNameChanged( const QString & ) ) );
//    scanAvailableVCS();
    connect( fetchModuleButton, SIGNAL(clicked()),
        this, SLOT(slotFetchModulesFromRepository()) );
    connect(urlinput_edit, SIGNAL(urlSelected(const QString& )), this, SLOT(dirChanged()));
    connect(urlinput_edit, SIGNAL(returnPressed(const QString& )), this, SLOT(dirChanged()));
    slotProjectNameChanged( name_edit->text() );
}


ImportDialog::~ImportDialog()
{}

void ImportDialog::slotProjectNameChanged( const QString &_text )
{
    ok_button->setEnabled( !_text.isEmpty() && !urlinput_edit->url().contains( QRegExp("\\s") ) );
}

void ImportDialog::accept()
{
    QDir dir(urlinput_edit->url());
    if (urlinput_edit->url().isEmpty() || !dir.exists()) {
        KMessageBox::sorry(this, i18n("You have to choose a directory."));
        return;
    }

    QString projectName = name_edit->text();
    if (projectName.isEmpty()) {
        KMessageBox::sorry(this, i18n("You have to choose a project name."));
        return;
    }

    for (uint i=0; i < projectName.length(); ++i)
        if (!projectName[i].isLetterOrNumber() && projectName[i] != '_') {
            KMessageBox::sorry(this, i18n("Your application name should only contain letters and numbers."));
            return;
        }

    if (infrastructureBox->isVisible() && infrastructureBox->isChecked())
        createProjectInfrastructure();

//    QString author = author_edit->text();
//    QString email = email_edit->text();

    QFileInfo finfo(importNames[project_combo->currentItem()]);
    QDir importdir(finfo.dir());
    importdir.cdUp();
    QFile src(importdir.filePath("importfiles/" + finfo.fileName() + ".kdevelop"));
    kdDebug(9010) << "Import template " << src.name() << endl;
    if (!src.open(IO_ReadOnly)) {
        KMessageBox::sorry(this, i18n("Cannot open project template."));
        return;
    }

    // Read the DOM of the newly created project
    QDomDocument projectDOM;

    int errorLine, errorCol;
    QString errorMsg;
    bool success = projectDOM.setContent( &src, &errorMsg, &errorLine, &errorCol);
    src.close();
    if ( !success )
    {
        KMessageBox::sorry( 0, i18n("This is not a valid project file.\n"
                "XML error in line %1, column %2:\n%3")
                .arg(errorLine).arg(errorCol).arg(errorMsg));
        return;
    }

    DomUtil::writeEntry( projectDOM, "/general/author", author_edit->text() );
    DomUtil::writeEntry( projectDOM, "/general/email" , email_edit->text() );
    DomUtil::writeEntry( projectDOM, "/general/projectname", name_edit->text() );
    if ( !projectVersion.isNull()){
        DomUtil::writeEntry( projectDOM, "/general/version", projectVersion );
    } else {
        DomUtil::writeEntry( projectDOM, "/general/version", "1" );
    }

    // figure out what plugins we should disable by default
    QString profileName = DomUtil::readEntry( projectDOM, "general/profile" );
    if ( profileName.isEmpty() )
    {
        QString language = DomUtil::readEntry( projectDOM, "general/primarylanguage" );
        QStringList keywords = DomUtil::readListEntry( projectDOM, "general/keywords", "keyword" );

        profileName = Settings::profileByAttributes( language, keywords );
    }

    ProfileEngine & engine = m_part->pluginController()->engine();
    Profile * profile = engine.findProfile( profileName );

    QStringList disableList;
    Profile::EntryList disableEntryList = profile->list( Profile::ExplicitDisable );
    for ( Profile::EntryList::const_iterator it = disableEntryList.constBegin(); it != disableEntryList.constEnd(); ++it )
    {
        disableList << (*it).name;
    }

    DomUtil::writeListEntry( projectDOM, "/general/ignoreparts", "part", disableList );


    // write the dom back
    QFile dest(dir.filePath(projectName + ".kdevelop"));
    if (!dest.open(IO_WriteOnly)) {
        KMessageBox::sorry(this, i18n("Cannot write the project file."));
        return;
    }
    QTextStream ts( &dest );
    ts.setEncoding(QTextStream::UnicodeUTF8);
    ts << projectDOM.toString(2);
    dest.close();


//     QTextStream srcstream(&src);
//     QTextStream deststream(&dest);
//
//     while (!srcstream.atEnd()) {
//         QString line = srcstream.readLine();
//         line.replace(QRegExp("\\$APPNAMELC\\$"), projectName);
//         line.replace(QRegExp("\\$AUTHOR\\$"), author);
//         line.replace(QRegExp("\\$EMAIL\\$"), email);
//         deststream << line << endl;
//     }
//
//     dest.close();
//     src.close();

    m_part->core()->openProject(dir.filePath(projectName + ".kdevelop"));

    kdDebug(9010) << "OPENING PROJECT: " << dir.filePath(projectName + ".kdevelop") << endl;

    QDialog::accept();
}


// Checks if the directory dir and all of its subdirectories
// (one level recursion) have files that follow patterns
// patterns is comma-separated
static bool dirHasFiles(QDir &dir, const QString &patterns)
{
    QStringList::ConstIterator pit, sit;

    QStringList patternList = QStringList::split(",", patterns);
    for (pit = patternList.begin(); pit != patternList.end(); ++pit) {
        if (!dir.entryList(*pit, QDir::Files).isEmpty()) {
            kdDebug(9010) << "Has files " << (*pit) << endl;
            return true;
        }
    }

    QStringList subdirList = dir.entryList("*", QDir::Dirs);
    for (sit = subdirList.begin(); sit != subdirList.end(); ++sit) {
        QDir subdir(dir);
        subdir.cd(*sit);
        for (pit = patternList.begin(); pit != patternList.end(); ++pit) {
            if (!subdir.entryList(*pit, QDir::Files).isEmpty()) {
                kdDebug(9010) << "Has files " << (*pit) << " in " << (*sit) << endl;
                return true;
            }
        }
    }

    return false;
}


void ImportDialog::dirChanged()
{
    kdDebug(9010) << "ImportDialog::dirChanged" << endl;
    QString dirName = urlinput_edit->url();
    QDir dir(dirName);
    if (!dir.exists())
        return;

    if ( dirName.contains( QRegExp("\\s") ) )
    {
        ok_button->setEnabled( false );
        return;
    }else
    {
        ok_button->setEnabled( true );
    }

    // KDevelop legacy project?
    QStringList files = dir.entryList("*.kdevprj");
    if (!files.isEmpty()) {
        scanLegacyKDevelopProject(dir.absFilePath(files.first()));
        return;
    }

    // Studio legacy project?
    files = dir.entryList("*.studio");
    if (!files.isEmpty()) {
        scanLegacyStudioProject(dir.absFilePath(files.first()));
        return;
    }

    // Automake based?
    if ( dir.exists("configure.in.in")|| dir.exists("configure.ac")|| dir.exists("configure.in")) {
        scanAutomakeProject(dirName);
        return;
    }

    // Remove any characters from the dirName that would be invalid in a project name
    QString projectName(dir.dirName().replace(QRegExp("[^a-zA-Z0-9_]"), "_"));

    // Set the project name
    name_edit->setText(projectName);

    // QMake based?
    files = dir.entryList("*.pro");
    if (!files.isEmpty()) {
        setProjectType("qtqmake");
        return;
    }

    // C++?
    if (dirHasFiles(dir, "*.cpp,*.c++,*.cxx,*.C,*.cc,*.ocl")) {
        setProjectType("cpp");
        return;
    }

    // Fortran?
    if (dirHasFiles(dir, "*.f77,*.f,*.for,*.ftn")) {
        setProjectType("fortran");
        return;
    }

    // Python?
    if (dirHasFiles(dir, "*.py")) {
        setProjectType("python");
        return;
    }

    // Perl?
    if (dirHasFiles(dir, "*.pl,*.pm")) {
        setProjectType("perl");
        return;
    }
}


void ImportDialog::scanLegacyKDevelopProject(const QString &fileName)
{
    kdDebug(9010) << "Scanning legacy KDevelop project file " << fileName << endl;

    KSimpleConfig config(fileName, true);
    config.setGroup("General");
    author_edit->setText(config.readEntry("author"));
    email_edit->setText(config.readEntry("email"));
    name_edit->setText(config.readEntry("project_name"));

    QString legacyType = config.readEntry("project_type");
    if (QStringList::split(",", "normal_kde,normal_kde2,kde2_normal,mdi_kde2").contains(legacyType))
        setProjectType("kde");
    else if (legacyType == "normal_gnome")
        setProjectType("gnome");
    else if (legacyType == "normal_empty")
        setProjectType("cpp-auto");
    else
        setProjectType("cpp");
}


void ImportDialog::scanLegacyStudioProject(const QString &fileName)
{
    kdDebug(9010) << "Scanning legacy studio project file " << fileName << endl;

    // Not much to do here...
    KSimpleConfig config(fileName, true);
    config.setGroup("kdestudio");
    name_edit->setText(config.readEntry("Name"));
}


void ImportDialog::scanAutomakeProject(const QString &dirName)
{
    kdDebug(9010) << "Scanning automake project directory " << dirName << endl;

    bool stop = false;
    if (QFile::exists(dirName + "/admin/am_edit")) {
        setProjectType("kde");
        stop = true;
    } else if (QFile::exists(dirName + "/macros/gnome.m4")) {
        setProjectType("gnome");
        stop = true;
    } else {
        setProjectType("c-auto");
    }

    // if we get an authors file, use it.
    QFile af(dirName + "/AUTHORS");
    if (af.open(IO_ReadOnly)){
        QTextStream astream(&af);

        QRegExp authorre("(.*)<(.*)>");
        while (!astream.atEnd()) {
            QString s = astream.readLine();
            if (authorre.search(s) != -1) {
                author_edit->setText(authorre.cap(1).stripWhiteSpace());
                email_edit->setText(authorre.cap(2).stripWhiteSpace());
                break;
            }
        }
        af.close();
    }

    // we ignore old AC_INIT that had no version..
    // only match the if there is a comma and at least two args..
    // AC_INIT (package, version, [bug-report], [tarname])
    QRegExp ac_init("^AC_INIT\\s*\\(\\s*([^,]+),([^,\\)]+)(.*)");

    // AM_INIT_AUTOMAKE([OPTIONS])
    // example: AM_INIT_AUTOMAKE([gnits 1.5 no-define dist-bzip2])
    QRegExp am_autoSpace("^AM_INIT_AUTOMAKE\\s{0,}\\(\\s{0,}([\\[\\s]{0,}[^\\s]+)\\s+([^\\s\\)\\]]+)(.*)");

    // AM_INIT_AUTOMAKE(PACKAGE, VERSION, [NO-DEFINE])
    QRegExp am_autoComma("^AM_INIT_AUTOMAKE\\s*\\(\\s*([^,]+),([^,\\)]+)(.*)");

    // look for version in a define.
    // AC_DEFINE(VERSION, "5.6")
    QRegExp ac_define("^AC_DEFINE\\s*\\(\\s*[^,]+,([^\\)]+)");
    QRegExp version("(\\bversion\\b)");
    version.setCaseSensitive(FALSE);

    QTextStream cstream;
    // try for configure.in.in, configure.in, then configure.ac
    QFile configInIn(dirName + "/configure.in.in");
    QFile configIn(dirName+"/configure.in");
    QFile configAc(dirName+"/configure.ac");
    if (configInIn.open(IO_ReadOnly)){
        cstream.setDevice(&configInIn);
        while (!cstream.atEnd()) {
            QString line = cstream.readLine();
            if ( ac_init.search(line) >= 0){
                projectVersion=ac_init.cap(2).stripWhiteSpace();
            }
            else if ( am_autoComma.search(line) >= 0 ){
                projectVersion=am_autoComma.cap(2).stripWhiteSpace();
            }
            else if ( am_autoSpace.search(line) >= 0 ){
                projectVersion=am_autoSpace.cap(2).stripWhiteSpace();
            }
            else if ( ac_define.search(line) >=0 && version.search(line) >=0) {
                projectVersion=ac_define.cap(2).stripWhiteSpace();
            }
        }
        configInIn.close();
    }

    if (configIn.open(IO_ReadOnly)){
        cstream.setDevice(&configIn);
    }
    else{
        if (configAc.open(IO_ReadOnly)){
            cstream.setDevice(&configAc);
        }
        else{
            return;
        }
    }

    QRegExp namere("\\s*AM_INIT_AUTOMAKE\\((.*),.*\\).*");
    QRegExp cppre("\\s*AC_PROG_CXX");
    QRegExp f77re("\\s*AC_PROG_F77");
    while (!cstream.atEnd()) {
        QString line = cstream.readLine();
        if ( ac_init.search(line) >= 0){
            projectVersion=ac_init.cap(2).stripWhiteSpace();
        }
        else if ( am_autoComma.search(line) >= 0 ){
            projectVersion=am_autoComma.cap(2).stripWhiteSpace();
        }
        else if ( am_autoSpace.search(line) >= 0 ){
            projectVersion=am_autoSpace.cap(2).stripWhiteSpace();
        }
        else if ( ac_define.search(line) >=0 && version.search(line) >=0) {
            projectVersion=ac_define.cap(2).stripWhiteSpace();
        }

        if (namere.search(line) == 0)
            name_edit->setText(namere.cap(1).stripWhiteSpace());
        if (!stop)
            continue;
        else if (cppre.search(line) == 0)
            setProjectType("cpp-auto");
        else if (f77re.search(line) == 0)
            setProjectType("fortran-auto");
    }

    if ( configIn.isOpen()) configIn.close();
    if ( configAc.isOpen()) configAc.close();
}


void ImportDialog::setProjectType(const QString &type)
{
    kdDebug(9010) << "Setting project type " << type << endl;
    QString suffix = "/" + type;
    int suffixLength = suffix.length();

    int i=0;
    QStringList::ConstIterator it;
    for (it = importNames.begin(); it != importNames.end(); ++it) {
        if ((*it).right(suffixLength) == suffix) {
            project_combo->setCurrentItem(i);
            break;
        }
        ++i;
    }
}
/*
void ImportDialog::scanAvailableVCS()
{
//    vcsCombo->insertStringList( m_part->registeredVersionControls() );
    int i = 0;
    KTrader::OfferList offers = KTrader::self()->query("KDevelop/VersionControl");
    KTrader::OfferList::const_iterator it = offers.begin();
    while( it != offers.end() )
    {
        vcsCombo->insertItem( (*it)->genericName(), i++ );
        ++it;
    }
}
*/
/*
void ImportDialog::slotFinishedCheckout( QString destinationDir )
{
    urlinput_edit->setURL( destinationDir );

    setCursor( KCursor::arrowCursor() );
//    setEnabled( true );
}
*/
/*
void ImportDialog::slotFetchModulesFromRepository()
{

    KDevVersionControl *vcs = m_part->versionControlByName( vcsCombo->currentText() );
    if (!vcs)
        return;

    setCursor( KCursor::waitCursor() );
//    setEnabled( false );

    connect( vcs, SIGNAL(finishedFetching(QString)),
        this, SLOT(slotFinishedCheckout(QString)) );

    //restore cursor if we can't fetch repository
    if ( !vcs->fetchFromRepository() )
        setCursor( KCursor::arrowCursor() );

}
*/
void ImportDialog::projectTypeChanged( const QString &type )
{
    if (m_infrastructure[type].isOn)
    {
        infrastructureBox->setEnabled(true);
        infrastructureBox->setText(m_infrastructure[type].comment);
    }
    else
    {
        infrastructureBox->setEnabled(false);
        infrastructureBox->setText(i18n("Generate build system infrastrucure"));
    }
}

void ImportDialog::createProjectInfrastructure( )
{
    kdDebug(9010) << "ImportDialog::createProjectInfrastructure" << endl;
    InfrastructureCmd cmd = m_infrastructure[project_combo->currentText()];
    if (!cmd.isOn)
        return;

    QDir dir (urlinput_edit->url());
    QStringList files = dir.entryList(cmd.existingPattern);
    if (!files.isEmpty()) {
        if (KMessageBox::questionYesNo(this, i18n("Project infrastrucure already exists in target directory.\nGenerate new project infrastructure and overwrite old?"), QString::null, i18n("Generate"), i18n("Do Not Generate")) == KMessageBox::No)
            return;
    }

    QString command = "cd " + urlinput_edit->url() + " && " + cmd.command;
    kdDebug(9010) << "executing " << command.ascii() << endl;
    system(command.ascii());
}

void ImportDialog::projectTypeChanged( int type )
{
    projectTypeChanged(project_combo->text(type));
}


#include "importdlg.moc"
