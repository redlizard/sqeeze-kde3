#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qdom.h>
#include <qstringlist.h>
#include <qptrlist.h>
#include <qvbox.h>
#include <qsize.h>
#include <qtimer.h>

class QDomDocument;

#include <kmessagebox.h>
#include <kdebug.h>
#include <klocale.h>
#include <kservice.h>
#include <ktrader.h>
#include <kfiledialog.h>
#include <kmainwindow.h>
#include <kparts/componentfactory.h>
#include <kaction.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kprocess.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>
#include <ktempfile.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <kiconloader.h>

#include "kdevproject.h"
#include "kdevlanguagesupport.h"
#include "kdevplugin.h"
#include "kdevcreatefile.h"
#include "kdevversioncontrol.h"


#include "toplevel.h"
#include "core.h"
#include "api.h"
#include "plugincontroller.h"
#include "partcontroller.h"
#include "codemodel.h"
// #include "partselectwidget.h"
#include "languageselectwidget.h"
#include "generalinfowidget.h"
#include "projectsession.h"
#include "domutil.h"
#include "settings.h"

#include "projectmanager.h"

QString ProjectInfo::sessionFile() const
{
    QString sf = m_projectURL.path(-1);
    sf.truncate(sf.length() - 8); // without ".kdevelop"
    sf += "kdevses"; // suffix for a KDevelop session file
    return sf;
}

QString ProjectManager::projectDirectory( const QString& path, bool absolute ) {
    if(absolute)
        return path;
    KURL url(ProjectManager::getInstance()->projectFile(), path);
    url.cleanPath();
    return url.path(-1);
}

ProjectManager *ProjectManager::s_instance = 0;

ProjectManager::ProjectManager()
: m_info(0L)
 ,m_pProjectSession(new ProjectSession)
{
}

ProjectManager::~ProjectManager()
{
  delete m_pProjectSession;
  delete m_info;
}

ProjectManager *ProjectManager::getInstance()
{
  if (!s_instance)
    s_instance = new ProjectManager;
  return s_instance;
}

void ProjectManager::createActions( KActionCollection* ac )
{
  KAction *action;

  action = new KAction(i18n("&Open Project..."), "project_open", 0,
                       this, SLOT(slotOpenProject()),
                       ac, "project_open");
  action->setToolTip( i18n("Open project"));
  action->setWhatsThis(i18n("<b>Open project</b><p>Opens a KDevelop3 or KDevelop2 project."));

  m_openRecentProjectAction =
    new KRecentFilesAction(i18n("Open &Recent Project"), 0,
                          this, SLOT(loadProject(const KURL &)),
                          ac, "project_open_recent");
  m_openRecentProjectAction->setToolTip(i18n("Open recent project"));
  m_openRecentProjectAction->setWhatsThis(i18n("<b>Open recent project</b><p>Opens recently opened project."));
  m_openRecentProjectAction->loadEntries(kapp->config(), "RecentProjects");

  m_closeProjectAction =
    new KAction(i18n("C&lose Project"), "fileclose",0,
                this, SLOT(closeProject()),
                ac, "project_close");
  m_closeProjectAction->setEnabled(false);
  m_closeProjectAction->setToolTip(i18n("Close project"));
  m_closeProjectAction->setWhatsThis(i18n("<b>Close project</b><p>Closes the current project."));

  m_projectOptionsAction = new KAction(i18n("Project &Options"), "configure", 0,
                this, SLOT(slotProjectOptions()),
                ac, "project_options" );
  m_projectOptionsAction->setToolTip(i18n("Project options"));
  m_projectOptionsAction->setWhatsThis(i18n("<b>Project options</b><p>Lets you customize project options."));
  m_projectOptionsAction->setEnabled(false);
}

void ProjectManager::slotOpenProject()
{
	KConfig *config = kapp->config();
	config->setGroup("General Options");
	QString defaultProjectsDir = config->readPathEntry("DefaultProjectsDir", QDir::homeDirPath()+"/");

  KURL url = KFileDialog::getOpenURL(defaultProjectsDir,
		i18n("*.kdevelop|KDevelop 3 Project Files\n"
		     "*.kdevprj|KDevelop 2 Project Files"),
		TopLevel::getInstance()->main(), i18n("Open Project") );

  if( url.isEmpty() )
      return;

  if (url.path().endsWith("kdevprj"))
      loadKDevelop2Project( url );
  else
      loadProject( url );
}

void ProjectManager::slotProjectOptions()
{
	KDialogBase dlg(KDialogBase::IconList, i18n("Project Options"),
                  KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, TopLevel::getInstance()->main(),
                  "project options dialog");

	QVBox *box = dlg.addVBoxPage( i18n("General"), i18n("General"), BarIcon( "kdevelop", KIcon::SizeMedium ) );
    GeneralInfoWidget *g = new GeneralInfoWidget(*API::getInstance()->projectDom(), box, "general informations widget");
    connect (&dlg, SIGNAL(okClicked()), g, SLOT(accept()));

  KConfig *config = kapp->config();
  config->setGroup("Project Settings Dialog");
  int height = config->readNumEntry( "Height", 600 );
  int width = config->readNumEntry( "Width", 800 );

  dlg.resize( width, height );

  Core::getInstance()->doEmitProjectConfigWidget(&dlg);
  dlg.exec();

  saveProjectFile();

  config->setGroup("Project Settings Dialog");
  config->writeEntry( "Height", dlg.size().height() );
  config->writeEntry( "Width", dlg.size().width() );
}

void ProjectManager::loadSettings()
{
}

void ProjectManager::saveSettings()
{
  KConfig *config = kapp->config();

  if (projectLoaded())
  {
    config->setGroup("General Options");
    config->writePathEntry("Last Project", ProjectManager::getInstance()->projectFile().url());
  }

  m_openRecentProjectAction->saveEntries(config, "RecentProjects");
}

void ProjectManager::loadDefaultProject()
{
  KConfig *config = kapp->config();
  config->setGroup("General Options");
  QString project = config->readPathEntry("Last Project");
  bool readProject = config->readBoolEntry("Read Last Project On Startup", true);
  if (!project.isEmpty() && readProject)
  {
      loadProject(KURL(project));
  }
}

bool ProjectManager::loadProject(const KURL &projectURL)
{
  KURL url = projectURL;

  if (!url.isValid())
    return false;

  if (url.isLocalFile())
  {
    QDir dir(url.path());
    QString can_path = dir.canonicalPath();
               //if the directory doesn't exist, the returned string is null which confuses the user later on. so better short-cut here
    if(can_path.isNull())
      return false;
    else
      url.setPath(can_path);
  }

  // reopen the already opened project?
  if( projectLoaded() && url.path() == projectFile().path() )
  {
    if (KMessageBox::questionYesNo(TopLevel::getInstance()->main(),
        i18n("Are you sure you want to reopen the current project?"), QString::null, i18n("Reopen"), i18n("Do Not Reopen")) == KMessageBox::No)
      return false;
  }

  TopLevel::getInstance()->main()->menuBar()->setEnabled( false );
  kapp->setOverrideCursor( waitCursor );

  if( projectLoaded() && !closeProject() )
  {
	m_openRecentProjectAction->setCurrentItem( -1 );
	TopLevel::getInstance()->main()->menuBar()->setEnabled( true );
	kapp->restoreOverrideCursor();
	return false;
  }

  m_info = new ProjectInfo;
  m_info->m_projectURL = url;

  QTimer::singleShot( 0, this, SLOT(slotLoadProject()) );

  // no one cares about this value
  return true;
}

void ProjectManager::slotLoadProject( )
{
  if( !loadProjectFile() )
  {
    m_openRecentProjectAction->removeURL(m_info->m_projectURL);
    delete m_info; m_info = 0;
    saveSettings();
	TopLevel::getInstance()->main()->menuBar()->setEnabled( true );
	kapp->restoreOverrideCursor();
    return;
  }

  getGeneralInfo();

  if( !loadLanguageSupport(m_info->m_language) ) {
    delete m_info; m_info = 0;
	TopLevel::getInstance()->main()->menuBar()->setEnabled( true );
	kapp->restoreOverrideCursor();
    return;
  }

  if( !loadProjectPart() ) {
    unloadLanguageSupport();
    delete m_info; m_info = 0;
    API::getInstance()->setProjectDom( 0 );
	TopLevel::getInstance()->main()->menuBar()->setEnabled( true );
	kapp->restoreOverrideCursor();
    return;
  }

  TopLevel::getInstance()->statusBar()->message( i18n("Changing plugin profile...") );
  m_oldProfileName = PluginController::getInstance()->changeProfile(m_info->m_profileName);

  TopLevel::getInstance()->statusBar()->message( i18n("Loading project plugins...") );
  loadLocalParts();

  // shall we try to load a session file from network?? Probably not.
	if (m_info->m_projectURL.isLocalFile())
	{
		// first restore the project session stored in a .kdevses file
		if (!m_pProjectSession->restoreFromFile(m_info->sessionFile(), PluginController::getInstance()->loadedPlugins() ))
		{
			kdWarning() << i18n("error during restoring of the KDevelop session !") << endl;
		}
	}
#if KDE_IS_VERSION(3,5,0)
  m_openRecentProjectAction->addURL(projectFile(), projectName());  // KDE >= 3.5.x
#else
  m_openRecentProjectAction->addURL(projectFile());                 // KDE 3.4.x
#endif 
  m_closeProjectAction->setEnabled(true);
  m_projectOptionsAction->setEnabled(true);

  Core::getInstance()->doEmitProjectOpened();

  TopLevel::getInstance()->main()->menuBar()->setEnabled( true );
  kapp->restoreOverrideCursor();

  TopLevel::getInstance()->statusBar()->message( i18n("Project loaded."), 3000 );

  return;
}


bool ProjectManager::closeProject( bool exiting )
{
  if( !projectLoaded() )
    return true;

  // save the session if it is a local file
	if (m_info->m_projectURL.isLocalFile())
	{
		m_pProjectSession->saveToFile(m_info->sessionFile(), PluginController::getInstance()->loadedPlugins() );
	}

  if ( !PartController::getInstance()->querySaveFiles() )
    return false;

  Core::getInstance()->doEmitProjectClosed();

  PluginController::getInstance()->unloadProjectPlugins();
  PluginController::getInstance()->changeProfile(m_oldProfileName);
  unloadLanguageSupport();
  unloadProjectPart();

  /// @todo if this fails, user is screwed
  saveProjectFile();

  API::getInstance()->setProjectDom(0);
  API::getInstance()->codeModel()->wipeout();

  delete m_info;
  m_info = 0;

  m_closeProjectAction->setEnabled(false);
  m_projectOptionsAction->setEnabled(false);

  if ( !exiting )
  {
    PartController::getInstance()->slotCloseAllWindows();
  }

  return true;
}

bool ProjectManager::loadProjectFile()
{
  QString path;
  if (!KIO::NetAccess::download(m_info->m_projectURL, path, 0)) {
    KMessageBox::sorry(TopLevel::getInstance()->main(),
        i18n("Could not read project file: %1").arg(m_info->m_projectURL.prettyURL()));
    return false;
  }

  QFile fin(path);
  if (!fin.open(IO_ReadOnly))
  {
    KMessageBox::sorry(TopLevel::getInstance()->main(),
        i18n("Could not read project file: %1").arg(m_info->m_projectURL.prettyURL()));
    return false;
  }

  int errorLine, errorCol;
  QString errorMsg;
  if (!m_info->m_document.setContent(&fin, &errorMsg, &errorLine, &errorCol))
  {
    KMessageBox::sorry(TopLevel::getInstance()->main(),
        i18n("This is not a valid project file.\n"
             "XML error in line %1, column %2:\n%3")
             .arg(errorLine).arg(errorCol).arg(errorMsg));
    fin.close();
    KIO::NetAccess::removeTempFile(path);
    return false;
  }
  if (m_info->m_document.documentElement().nodeName() != "kdevelop")
  {
    KMessageBox::sorry(TopLevel::getInstance()->main(),
        i18n("This is not a valid project file."));
    fin.close();
    KIO::NetAccess::removeTempFile(path);
    return false;
  }

  fin.close();
  KIO::NetAccess::removeTempFile(path);

  API::getInstance()->setProjectDom(&m_info->m_document);

  return true;
}

bool ProjectManager::saveProjectFile()
{
  Q_ASSERT( API::getInstance()->projectDom() );

  if (m_info->m_projectURL.isLocalFile()) {
    QFile fout(m_info->m_projectURL.path());
    if( !fout.open(IO_WriteOnly) ) {
      KMessageBox::sorry(TopLevel::getInstance()->main(), i18n("Could not write the project file."));
      return false;
    }

    QTextStream stream(&fout);
    API::getInstance()->projectDom()->save(stream, 2);
    fout.close();
  } else {
    KTempFile fout(QString::fromLatin1("kdevelop3"));
    fout.setAutoDelete(true);
    if (fout.status() != 0) {
      KMessageBox::sorry(TopLevel::getInstance()->main(), i18n("Could not write the project file."));
      return false;
    }
    API::getInstance()->projectDom()->save(*(fout.textStream()), 2);
    fout.close();
    KIO::NetAccess::upload(fout.name(), m_info->m_projectURL, 0);
  }

  return true;
}

static QString getAttribute(QDomElement elem, QString attr)
{
  QDomElement el = elem.namedItem(attr).toElement();
  return el.firstChild().toText().data();
}

static void getAttributeList(QDomElement elem, QString attr, QString tag, QStringList &list)
{
  list.clear();

  QDomElement el = elem.namedItem(attr).toElement();
  QDomElement item = el.firstChild().toElement();
  while (!item.isNull())
  {
    if (item.tagName() == tag)
      list << item.firstChild().toText().data();
    item = item.nextSibling().toElement();
  }
}

void ProjectManager::getGeneralInfo()
{
  QDomElement docEl = m_info->m_document.documentElement();
  QDomElement generalEl = docEl.namedItem("general").toElement();

  m_info->m_projectPlugin = getAttribute(generalEl, "projectmanagement");
  m_info->m_vcsPlugin = getAttribute(generalEl, "versioncontrol");
  m_info->m_language = getAttribute(generalEl, "primarylanguage");
  m_info->m_projectName = getAttribute(generalEl, "projectname");
  if( m_info->m_projectName.isEmpty() )
  {
    m_info->m_projectName = m_info->m_projectURL.filename();
    m_info->m_projectName = m_info->m_projectName.left(m_info->m_projectName.length()-QString(".kdevelop").length());
    QDomElement prjname = m_info->m_document.createElement("projectname");
    prjname.appendChild( m_info->m_document.createTextNode( m_info->m_projectName) );
    generalEl.appendChild( prjname );
  }

  getAttributeList(generalEl, "ignoreparts", "part", m_info->m_ignoreParts);
  getAttributeList(generalEl, "keywords", "keyword", m_info->m_keywords);

  //FIXME: adymo: workaround for those project templates without "profile" element
//  m_info->m_profileName = getAttribute(generalEl, "profile");
  QDomElement el = generalEl.namedItem("profile").toElement();
  if (el.isNull())
//      m_info->m_profileName = profileByAttributes(m_info->m_language, m_info->m_keywords);
      m_info->m_profileName = Settings::profileByAttributes(m_info->m_language, m_info->m_keywords);
  else
      m_info->m_profileName = el.firstChild().toText().data();
}

bool ProjectManager::loadProjectPart()
{
  KService::Ptr projectService = KService::serviceByDesktopName(m_info->m_projectPlugin);
  if (!projectService) {
    // this is for backwards compatibility with pre-alpha6 projects
    projectService = KService::serviceByDesktopName(m_info->m_projectPlugin.lower());
  }
  if (!projectService) {
    KMessageBox::sorry(TopLevel::getInstance()->main(),
        i18n("No project management plugin %1 found.")
            .arg(m_info->m_projectPlugin));
    return false;
  }

  KDevProject *projectPart = KParts::ComponentFactory
    ::createInstanceFromService< KDevProject >( projectService, API::getInstance(), 0,
                                                  PluginController::argumentsFromService( projectService ) );
  if ( !projectPart ) {
    KMessageBox::sorry(TopLevel::getInstance()->main(),
        i18n("Could not create project management plugin %1.")
            .arg(m_info->m_projectPlugin));
    return false;
  }

  API::getInstance()->setProject( projectPart );

  QDomDocument& dom = *API::getInstance()->projectDom();
  QString path = DomUtil::readEntry(dom,"/general/projectdirectory", ".");
  bool absolute = DomUtil::readBoolEntry(dom,"/general/absoluteprojectpath",false);
  QString projectDir = projectDirectory( path, absolute );
  kdDebug(9000) << "projectDir: " << projectDir << "  projectName: " << m_info->m_projectName << endl;

  projectPart->openProject(projectDir, m_info->m_projectName);

  PluginController::getInstance()->integratePart( projectPart );

  return true;
}

void ProjectManager::unloadProjectPart()
{
  KDevProject *projectPart = API::getInstance()->project();
  if( !projectPart ) return;
  PluginController::getInstance()->removePart( projectPart );
  projectPart->closeProject();
  delete projectPart;
  API::getInstance()->setProject(0);
}

bool ProjectManager::loadLanguageSupport(const QString& lang)
{
  kdDebug(9000) << "Looking for language support for " << lang << endl;

  if (lang == m_info->m_activeLanguage)
  {
    kdDebug(9000) << "Language support already loaded" << endl;
    return true;
  }

  KTrader::OfferList languageSupportOffers =
    KTrader::self()->query(QString::fromLatin1("KDevelop/LanguageSupport"),
                           QString::fromLatin1("[X-KDevelop-Language] == '%1' and [X-KDevelop-Version] == %2").arg(lang).arg(KDEVELOP_PLUGIN_VERSION));

  if (languageSupportOffers.isEmpty()) {
    KMessageBox::sorry(TopLevel::getInstance()->main(),
        i18n("No language plugin for %1 found.")
            .arg(lang));
    return false;
  }

  KService::Ptr languageSupportService = *languageSupportOffers.begin();
  KDevLanguageSupport *langSupport = KParts::ComponentFactory
      ::createInstanceFromService<KDevLanguageSupport>( languageSupportService,
                                                        API::getInstance(),
                                                        0,
                                                        PluginController::argumentsFromService(  languageSupportService ) );

  if ( !langSupport ) {
    KMessageBox::sorry(TopLevel::getInstance()->main(),
        i18n("Could not create language plugin for %1.")
            .arg(lang));
    return false;
  }

  API::getInstance()->setLanguageSupport( langSupport );
  PluginController::getInstance()->integratePart( langSupport );
  m_info->m_activeLanguage = lang;
  kdDebug(9000) << "Language support for " << lang << " successfully loaded." << endl;
  return true;
}

void ProjectManager::unloadLanguageSupport()
{
  KDevLanguageSupport *langSupport = API::getInstance()->languageSupport();
  if( !langSupport ) return;
  kdDebug(9000) << "Language support for " << langSupport->name() << " unloading..." << endl;
  PluginController::getInstance()->removePart( langSupport );
  delete langSupport;
  API::getInstance()->setLanguageSupport(0);
}

void ProjectManager::loadLocalParts()
{
	// Make sure to refresh load/ignore lists
	getGeneralInfo();

  PluginController::getInstance()->unloadPlugins( m_info->m_ignoreParts );
  PluginController::getInstance()->loadProjectPlugins( m_info->m_ignoreParts );
	PluginController::getInstance()->loadGlobalPlugins( m_info->m_ignoreParts );
}

KURL ProjectManager::projectFile() const
{
  if (!m_info)
    return KURL();
  return m_info->m_projectURL;
}

QString ProjectManager::projectName() const
{
  if (!m_info) return QString();

  return m_info->m_projectName;
}

bool ProjectManager::projectLoaded() const
{
  return m_info != 0;
}

ProjectSession* ProjectManager::projectSession() const
{
  return m_pProjectSession;
}

bool ProjectManager::loadKDevelop2Project( const KURL & url )
{
    if( !url.isValid() || !url.isLocalFile() ){
        KMessageBox::sorry(0, i18n("Invalid URL."));
        return false;
    }

    QString cmd = KGlobal::dirs()->findExe( "kdevprj2kdevelop" );
    if (cmd.isEmpty()) {
        KMessageBox::sorry(0, i18n("You do not have 'kdevprj2kdevelop' installed."));
        return false;
    }

    QFileInfo fileInfo( url.path() );

    KShellProcess proc( "/bin/sh" );
    proc.setWorkingDirectory( fileInfo.dirPath(true) );
    proc << "perl" << cmd << KShellProcess::quote( url.path() );
    proc.start( KProcess::Block );

    QString projectFile = fileInfo.dirPath( true ) + "/" + fileInfo.baseName() + ".kdevelop";
    return loadProject( KURL(projectFile) );
}

// QString ProjectManager::profileByAttributes(const QString &language, const QStringList &keywords)
// {
//     KConfig config(locate("data", "kdevelop/profiles/projectprofiles"));
//     config.setGroup(language);
//
//     QStringList profileKeywords = QStringList::split("/", "Empty");
//     if (config.hasKey("Keywords"))
//         profileKeywords = config.readListEntry("Keywords");
//
//     int idx = 0;
//     for (QStringList::const_iterator it = profileKeywords.constBegin();
//         it != profileKeywords.constEnd(); ++it)
//     {
//         if (keywords.contains(*it))
//         {
//             idx = profileKeywords.findIndex(*it);
//             break;
//         }
//     }
//
//     QStringList profiles;
//     if (config.hasKey("Profiles"))
//     {
//         profiles = config.readListEntry("Profiles");
//         kdDebug() << "IDX: " << idx << "    PROFILE: " << profiles[idx] << endl;
//         return profiles[idx];
//     }
//     return "KDevelop";
// }

#include "projectmanager.moc"
