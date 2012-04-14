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

#include "scriptprojectpart.h"

#include <qdir.h>
#include <qregexp.h>
#include <qstringlist.h>
#include <qvaluestack.h>
#include <qvbox.h>
#include <qwhatsthis.h>
#include <kaction.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdevgenericfactory.h>
#include <kdevcreatefile.h>
#include <kdirwatch.h>

#include "domutil.h"
#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevpartcontroller.h"
#include "kdevlanguagesupport.h"
#include "scriptoptionswidget.h"
#include "scriptnewfiledlg.h"
#include "kdevplugininfo.h"

typedef KDevGenericFactory<ScriptProjectPart> ScriptProjectFactory;
static const KDevPluginInfo data("kdevscriptproject");
K_EXPORT_COMPONENT_FACTORY( libkdevscriptproject, ScriptProjectFactory( data ) )

ScriptProjectPart::ScriptProjectPart(QObject *parent, const char *name, const QStringList &)
    : KDevBuildTool(&data, parent, name ? name : "ScriptProjectPart")
{
    setInstance(ScriptProjectFactory::instance());

    setXMLFile("kdevscriptproject.rc");

    // only create new file action if file creation part not available
    if (!extension<KDevCreateFile>("KDevelop/CreateFile")) {
      KAction *action;
      action = new KAction( i18n("New File..."), 0,
                            this, SLOT(slotNewFile()),
                            actionCollection(), "file_newfile" );
      action->setWhatsThis( i18n("<b>New file</b><p>Creates a new file.") );
      action->setToolTip( i18n("Create a new file") );
    }
    new KAction( i18n("Rescan Project"), 0, CTRL+ALT+Key_R,
                            this, SLOT(rescan()),
                            actionCollection(), "rescan" );

    connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
             this, SLOT(projectConfigWidget(KDialogBase*)) );
}


ScriptProjectPart::~ScriptProjectPart()
{}


void ScriptProjectPart::projectConfigWidget(KDialogBase *dlg)
{
    QVBox *vbox;
    vbox = dlg->addVBoxPage(i18n("Script Project Options"), i18n("Script Project Options"), BarIcon("kdevelop", KIcon::SizeMedium));
    ScriptOptionsWidget *w = new ScriptOptionsWidget(this, vbox);
    connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}


/**
 * @todo This should really be merged with FileTreeWidget::matchesHidePattern()
 * and put in its own class. Currently this is repeated in scriptprojectpart.cpp, pascalproject_part.cpp, adaproject_part.cpp
 */
static bool matchesPattern(const QString &fileName, const QStringList &patternList)
{
    QStringList::ConstIterator it;
    for (it = patternList.begin(); it != patternList.end(); ++it) {
        QRegExp re(*it, true, true);
        if (re.search(fileName) == 0 && re.matchedLength() == (int)fileName.length())
            return true;
    }

    return false;
}


void ScriptProjectPart::openProject(const QString &dirName, const QString &projectName)
{
    if (!languageSupport())
        kdDebug(9015) << "ScriptProjectPart::openProject: no language support found!" << endl;

    m_projectDirectory = dirName;
    m_projectName = projectName;

    QDomDocument &dom = *projectDom();

    // Set the default directory radio to "executable"
    if (DomUtil::readEntry(dom, "/kdevscriptproject/run/directoryradio") == "" ) {
        DomUtil::writeEntry(dom, "/kdevscriptproject/run/directoryradio", "executable");
    }

    // Put all files from all subdirectories into file list
    QValueStack<QString> s;
    int prefixlen = m_projectDirectory.length()+1;
    s.push(m_projectDirectory);

    QDir dir;
    do {
        dir.setPath(s.pop());
        kdDebug(9015) << "Examining: " << dir.path() << endl;
        const QFileInfoList *dirEntries = dir.entryInfoList();
        if ( dirEntries )
        {
            QPtrListIterator<QFileInfo> it(*dirEntries);
            for (; it.current(); ++it) {
                QString fileName = it.current()->fileName();
                if (fileName == "." || fileName == "..")
                   continue;
                QString path = it.current()->absFilePath();
                if (it.current()->isDir()) {
                    //do not follow symlinks which point to the themselves
                    if (it.current()->isSymLink())
                    {
                        QString symLink = it.current()->readLink();
                        if ((symLink == path) || (symLink == "./"))
                            continue;
                    } else if (canAddDirectoryToProject(path)) {
                        kdDebug(9015) << "Pushing: " << path << endl;
                        s.push(path);
                    }
                }
                else {
                    if (canAddToProject(path))
                        m_sourceFiles.append(path.mid(prefixlen));
                }
            }
        }
    } while (!s.isEmpty());

    KDevProject::openProject( dirName, projectName );
}


void ScriptProjectPart::closeProject()
{
}


QString ScriptProjectPart::projectDirectory() const
{
    return m_projectDirectory;
}


QString ScriptProjectPart::buildDirectory() const
{
    return m_projectDirectory;
}

QString ScriptProjectPart::projectName() const
{
    return m_projectName;
}


/** Retuns a PairList with the run environment variables */
DomUtil::PairList ScriptProjectPart::runEnvironmentVars() const
{
    return DomUtil::readPairListEntry(*projectDom(), "/kdevscriptproject/run/envvars", "envvar", "name", "value");
}


/** Retuns the currently selected run directory
  * The returned string can be:
  *   if run/directoryradio == executable
  *        The directory where the executable is
  *   if run/directoryradio == build
  *        The directory where the executable is relative to build directory
  *   if run/directoryradio == custom
  *        The custom directory absolute path
  */
QString ScriptProjectPart::runDirectory() const
{
    QString cwd = defaultRunDirectory("kdevscriptproject");
    if (cwd.isEmpty())
      cwd = buildDirectory();
    return cwd;
}


/** Retuns the currently selected main program
  * The returned string can be:
  *   if run/directoryradio == executable
  *        The executable name
  *   if run/directoryradio == build
  *        The path to executable relative to build directory
  *   if run/directoryradio == custom or relative == false
  *        The absolute path to executable
  */
QString ScriptProjectPart::mainProgram() const
{
    QDomDocument * dom = projectDom();

    if ( !dom ) return QString();

    QString DomMainProgram = DomUtil::readEntry( *dom, "/kdevscriptproject/run/mainprogram");

    if ( DomMainProgram.isEmpty() ) return QString();

    if ( DomMainProgram.startsWith("/") )   // assume absolute path
    {
        return DomMainProgram;
    }
    else // assume project relative path
    {
        return projectDirectory() + "/" + DomMainProgram;
    }

    return QString();
}

/** Retuns a QString with the debug command line arguments */
QString ScriptProjectPart::debugArguments() const
{
    return DomUtil::readEntry(*projectDom(), "/kdevscriptproject/run/globaldebugarguments");
}


/** Retuns a QString with the run command line arguments */
QString ScriptProjectPart::runArguments() const
{
    return DomUtil::readEntry(*projectDom(), "/kdevscriptproject/run/programargs");
}


QString ScriptProjectPart::activeDirectory() const
{
    QDomDocument &dom = *projectDom();

    return DomUtil::readEntry(dom, "/kdevscriptproject/general/activedir");
}


QStringList ScriptProjectPart::allFiles() const
{
/*    QStringList res;

    QStringList::ConstIterator it;
    for (it = m_sourceFiles.begin(); it != m_sourceFiles.end(); ++it)
        res += (m_projectDirectory + "/" + (*it));

    return res;*/

	// return all files relative to the project directory!
	return m_sourceFiles;
}

void ScriptProjectPart::addFile(const QString &fileName)
{
    kdDebug(9015) << "AddFile2" << fileName << endl;

	QStringList fileList;
	fileList.append ( fileName );

	this->addFiles ( fileList );
}

void ScriptProjectPart::addFiles ( const QStringList& fileList )
{
	QStringList::ConstIterator it;

	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
		m_sourceFiles.append ( ( *it ) );
	}

	emit addedFilesToProject ( fileList );
}

void ScriptProjectPart::removeFile(const QString &fileName)
{
	QStringList fileList;
	fileList.append ( fileName );

	this->addFiles ( fileList );
}

void ScriptProjectPart::removeFiles ( const QStringList& fileList )
{
	emit removedFilesFromProject ( fileList );

	QStringList::ConstIterator it;

	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
		m_sourceFiles.remove ( ( *it ) );
	}
}

void ScriptProjectPart::slotNewFile()
{
    ScriptNewFileDialog dlg(this);
    dlg.exec();
}

/*!
    \fn ScriptProjectPart::distFiles() const
 */
QStringList ScriptProjectPart::distFiles() const
{
   	QStringList sourceList = allFiles();
	// Scan current source directory for any .pro files.
	QString projectDir = projectDirectory();
	QDir dir(projectDir);
	QStringList files = dir.entryList( "*README*");
	return sourceList + files;
}

void ScriptProjectPart::rescan()
{
//     kdDebug() << "Directory " << path << " changed, scanning for new files in the project" << endl;

    // Put all files from all subdirectories into file list
    QValueStack<QString> s;
    int prefixlen = m_projectDirectory.length()+1;
    s.push(m_projectDirectory);

    QDir dir;
    do {
        dir.setPath(s.pop());
        kdDebug(9015) << "Examining: " << dir.path() << endl;
        const QFileInfoList *dirEntries = dir.entryInfoList();
        if ( dirEntries )
        {
            QPtrListIterator<QFileInfo> it(*dirEntries);
            for (; it.current(); ++it) {
                QString fileName = it.current()->fileName();
                if (fileName == "." || fileName == "..")
                   continue;
                QString path = it.current()->absFilePath();
                if (it.current()->isDir()) {
                    //do not follow symlinks which point to the themselves
                    if (it.current()->isSymLink())
                    {
                        QString symLink = it.current()->readLink();
                        if ((symLink == path) || (symLink == "./"))
                            continue;
                    } else if (canAddDirectoryToProject(path)) {
                        kdDebug(9015) << "Pushing: " << path << endl;
                        s.push(path);
                    }
                }
                else {
                    if (!isProjectFile(path) && canAddToProject(path))
                        addFile(path.mid(prefixlen));
//                         m_sourceFiles.append(path.mid(prefixlen));
                }
            }
        }
    } while (!s.isEmpty());

/*    const QFileInfoList *dirEntries = QDir(path).entryInfoList();
    if ( dirEntries )
    {
        kdDebug() << "1" << endl;
        QPtrListIterator<QFileInfo> it(*dirEntries);
        for (; it.current(); ++it) {
            kdDebug() << "2" << endl;
            QString fileName = it.current()->fileName();
            if (fileName == "." || fileName == "..")
                continue;
            kdDebug() << "3" << endl;
            QString filePath = it.current()->absFilePath();
            if (!it.current()->isDir() && canAddToProject(filePath) && !isProjectFile(filePath)) {
                kdDebug() << "=== adding " << filePath << endl;
                addFile(filePath);
            }
        }
    }*/
}

bool ScriptProjectPart::canAddToProject(const QString & path)
{
    QDomDocument &dom = *projectDom();
    QString includepatterns
        = DomUtil::readEntry(dom, "/kdevscriptproject/general/includepatterns");
    QStringList includepatternList;
    if ( includepatterns.isNull() ) {
    if ( languageSupport() ){
        KMimeType::List list = languageSupport()->mimeTypes();
        KMimeType::List::Iterator it = list.begin();
        while( it != list.end() ){
        includepatternList += (*it)->patterns();
        ++it;
        }
    }
    } else {
        includepatternList = QStringList::split(",", includepatterns);
    }

    QString excludepatterns
        = DomUtil::readEntry(dom, "/kdevscriptproject/general/excludepatterns");
    if (excludepatterns.isNull())
        excludepatterns = "*~";
    QStringList excludepatternList = QStringList::split(",", excludepatterns);

    if (matchesPattern(path, includepatternList)
        && !matchesPattern(path, excludepatternList)) {
        kdDebug(9015) << "Adding: " << path << endl;
        return true;
    } else {
        kdDebug(9015) << "Ignoring: " << path << endl;
        return false;
    }
}

bool ScriptProjectPart::canAddDirectoryToProject(const QString & path)
{
    QDomDocument &dom = *projectDom();
    QString excludepatterns
        = DomUtil::readEntry(dom, "/kdevscriptproject/general/excludepatterns");
    if (excludepatterns.isNull()) {
        return true;
    }
    QStringList excludepatternList = QStringList::split(",", excludepatterns);

    if (!matchesPattern(path, excludepatternList)) {
        return true;
    } else {
        kdDebug(9015) << "Ignoring Directory: " << path << endl;
        return false;
    }
}

#include "scriptprojectpart.moc"
