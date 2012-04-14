#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <qpopupmenu.h>
#include <qfile.h>
#include <qlayout.h>
#include <qmap.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qdom.h>

#include <kmimetype.h>
#include <kservice.h>
#include <ktrader.h>
#include <kapplication.h>
#include <krun.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kparts/part.h>
#include <kparts/factory.h>
#include <kparts/partmanager.h>
#include <kparts/browserextension.h>
#include <kfiledialog.h>
#include <kmainwindow.h>
#include <kaction.h>
#include <kmessagebox.h>
#include <kstatusbar.h>
#include <khtml_part.h>
#include <kpopupmenu.h>
#include <kio/netaccess.h>
#include <kdialogbase.h>
#include <klineedit.h>
#include <kshortcut.h>
#include <kcompletion.h>
#include <kdirwatch.h>
#include <kdeversion.h>
#include <kiconloader.h>
#include <kuserprofile.h>
#include <kencodingfiledialog.h>
#include <ksqueezedtextlabel.h>

#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/encodinginterface.h>

#include "toplevel.h"
#include "api.h"
#include "core.h"
#include "editorproxy.h"
#include "documentationpart.h"
#include "ksavealldialog.h"

#include "kdevproject.h"
#include "urlutil.h"
#include "mimewarningdialog.h"
#include "domutil.h"
#include "kdevjobtimer.h"

#include "designer.h"
#include "kdevlanguagesupport.h"

#include "multibuffer.h"
#include "partcontroller.h"


class QDomDocument;

PartController *PartController::s_instance = 0;

using namespace MainWindowUtils;

struct HistoryEntry
{
    KURL url;
    QString context;

    HistoryEntry( const KURL& u, const QString& c ): url( u ), context( c ) {}
};

struct ModificationData
{
	KTextEditor::Document * doc;
	bool isModified;
	unsigned char reason;
};


PartController::PartController(QWidget *parent)
  : KDevPartController(parent), _editorFactory(0L), m_currentActivePart(0), m_removingActivePart(false)
{
  connect(this, SIGNAL(partRemoved(KParts::Part*)), this, SLOT(slotPartRemoved(KParts::Part* )) );
  connect(this, SIGNAL(partAdded(KParts::Part*)), this, SLOT(slotPartAdded(KParts::Part* )) );
  connect(this, SIGNAL(activePartChanged(KParts::Part*)), this, SLOT(slotActivePartChanged(KParts::Part*)));

  setupActions();

  m_isJumping = false;

  m_openNextAsText = false;
}


PartController::~PartController()
{
}


void PartController::createInstance(QWidget *parent)
{
  if (!s_instance)
    s_instance = new PartController(parent);
}


PartController *PartController::getInstance()
{
  return s_instance;
}


void PartController::setupActions()
{
  KActionCollection *ac = TopLevel::getInstance()->main()->actionCollection();

  KAction* newAction = KStdAction::open(this, SLOT(slotOpenFile()), ac, "file_open");
  newAction->setToolTip( i18n("Open file") );
  newAction->setWhatsThis( i18n("<b>Open file</b><p>Opens an existing file without adding it to the project.</p>") );

  m_openRecentAction = KStdAction::openRecent( this, SLOT(slotOpenRecent(const KURL&) ), ac, "file_open_recent" );
  m_openRecentAction->setWhatsThis(QString("<b>%1</b><p>%2").arg(beautifyToolTip(m_openRecentAction->text())).arg(i18n("Opens recently opened file.")));
  m_openRecentAction->loadEntries( kapp->config(), "RecentFiles" );

  m_saveAllFilesAction = new KAction(i18n("Save Al&l"), 0, this, SLOT(slotSaveAllFiles()), ac, "file_save_all");
  m_saveAllFilesAction->setToolTip( i18n("Save all modified files") );
  m_saveAllFilesAction->setWhatsThis(i18n("<b>Save all</b><p>Saves all modified files."));
  m_saveAllFilesAction->setEnabled(false);

  m_revertAllFilesAction = new KAction(i18n("Rever&t All"), 0, this, SLOT(slotRevertAllFiles()), ac, "file_revert_all");
  m_revertAllFilesAction->setToolTip(i18n("Revert all changes"));
  m_revertAllFilesAction->setWhatsThis(i18n("<b>Revert all</b><p>Reverts all changes in opened files. Prompts to save changes so the reversion can be canceled for each modified file."));
  m_revertAllFilesAction->setEnabled(false);

  m_closeWindowAction = KStdAction::close(this, SLOT(slotCloseWindow()), ac, "file_close");
  m_closeWindowAction->setToolTip( i18n("Close current file") );
  m_closeWindowAction->setWhatsThis(QString("<b>%1</b><p>%2").arg(beautifyToolTip(m_closeWindowAction->text())).arg(i18n("Closes current file.")));
  m_closeWindowAction->setEnabled(false);

  m_closeAllWindowsAction = new KAction(i18n("Close All"), 0, this, SLOT(slotCloseAllWindows()), ac, "file_close_all");
  m_closeAllWindowsAction->setToolTip( i18n("Close all files") );
  m_closeAllWindowsAction->setWhatsThis(i18n("<b>Close all</b><p>Close all opened files."));
  m_closeAllWindowsAction->setEnabled(false);

  m_closeOtherWindowsAction = new KAction(i18n("Close All Others"), 0, this, SLOT(slotCloseOtherWindows()), ac, "file_closeother");
  m_closeOtherWindowsAction->setToolTip( i18n("Close other files") );
  m_closeOtherWindowsAction->setWhatsThis(i18n("<b>Close all others</b><p>Close all opened files except current."));
  m_closeOtherWindowsAction->setEnabled(false);

  new KActionSeparator(ac, "dummy_separator");

  m_backAction = new KToolBarPopupAction(i18n("Back"), "back", 0, this, SLOT(slotBack()), ac, "history_back");
  m_backAction->setEnabled( false );
  m_backAction->setToolTip(i18n("Back"));
  m_backAction->setWhatsThis(i18n("<b>Back</b><p>Moves backwards one step in the navigation history."));
  connect(m_backAction->popupMenu(), SIGNAL(aboutToShow()), this, SLOT(slotBackAboutToShow()));
  connect(m_backAction->popupMenu(), SIGNAL(activated(int)), this, SLOT(slotBackPopupActivated(int)));

  m_forwardAction = new KToolBarPopupAction(i18n("Forward"), "forward", 0, this, SLOT(slotForward()), ac, "history_forward");
  m_forwardAction->setEnabled( false );
  m_forwardAction->setToolTip(i18n("Forward"));
  m_forwardAction->setWhatsThis(i18n("<b>Forward</b><p>Moves forward one step in the navigation history."));
  connect(m_forwardAction->popupMenu(), SIGNAL(aboutToShow()), this, SLOT(slotForwardAboutToShow()));
  connect(m_forwardAction->popupMenu(), SIGNAL(activated(int)), this, SLOT(slotForwardPopupActivated(int)));

  m_gotoLastEditPosAction = new KAction( i18n("Goto Last Edit Position"), "bottom", 0, this, SLOT(gotoLastEditPos()), ac, "goto_last_edit_pos" );
  m_gotoLastEditPosAction->setEnabled( false );
  m_gotoLastEditPosAction->setToolTip( i18n("Goto Last Edit Position") );
  m_gotoLastEditPosAction->setWhatsThis( i18n("<b>Goto Last Edit Position</b><p>Open the last edited file and position cursor at the point of edit") );
}

void PartController::setEncoding(const QString &encoding)
{
  m_presetEncoding = encoding;
}

KParts::Part* PartController::findOpenDocument(const KURL& url)
{
	// if we find it this way, all is well
	KParts::Part * part = partForURL( url );
	if ( part )
	{
		return part;
	}

	// ok, let's see if we can try harder
	if ( API::getInstance()->project() )
	{
		KURL partURL = findURLInProject( url );
		partURL.cleanPath();
		return partForURL( partURL );
	}

	return 0L;
}

KURL PartController::findURLInProject(const KURL& url)
{
  QStringList fileList = API::getInstance()->project()->allFiles();

  bool filenameOnly = (url.url().find('/') == -1);
  QString filename = filenameOnly ? "/" : "";
  filename += url.url();

  for (QStringList::Iterator it = fileList.begin(); it != fileList.end(); ++it) {
    if ((*it).endsWith(filename)) {
      // Match! The first one is as good as any one, I guess...
      return KURL( API::getInstance()->project()->projectDirectory() + "/" + *it );
    }
  }

  return url;
}

void PartController::editDocument(const KURL &inputUrl, int lineNum, int col)
{
  editDocumentInternal(inputUrl, lineNum, col);
}

void PartController::splitCurrentDocument(const KURL &inputUrl,
                                          int lineNum, int col)
{
  editDocumentInternal(inputUrl, lineNum, col, true, true);
}

void PartController::scrollToLineColumn(const KURL &inputUrl,
                                        int lineNum, int col, bool storeHistory )
{
    if ( KParts::ReadOnlyPart *existingPart = partForURL( inputUrl ) )
    {
        if( storeHistory ) addHistoryEntry( existingPart );
        EditorProxy::getInstance()->setLineNumber( existingPart, lineNum, col );
        return;
    }
}

void PartController::editDocumentInternal( const KURL & inputUrl, int lineNum,
                                           int col, bool activate,
                                           bool addToCurrentBuffer )
{
    kdDebug(9000) << k_funcinfo << "\n " << inputUrl.prettyURL()
        << " linenum " << lineNum << " activate? " << activate
        << " addToCurrentBuffer? " << addToCurrentBuffer << endl;

	KURL url = inputUrl;

	// is it already open?
	// (Try this once before verifying the URL, we could be dealing with a file that no longer exists on disc)
	if ( KParts::Part *existingPart = partForURL( url ) )
	{
		// if we've been asked to OpenAs an open file with a specific encoding, assume the user wants to change encoding
		if ( !m_presetEncoding.isNull() )
		{
			if ( KTextEditor::EncodingInterface * ei = dynamic_cast<KTextEditor::EncodingInterface*>( existingPart ) )
			{
				ei->setEncoding( m_presetEncoding );
			}
			m_presetEncoding = QString::null;
		}

		addHistoryEntry();
		activatePart( existingPart );
    	EditorProxy::getInstance()->setLineNumber( existingPart, lineNum, col );
		return;
	}

	// Make sure the URL exists
	if ( !url.isValid() || !KIO::NetAccess::exists(url, false, 0) )
	{
		bool done = false;

		// Try to find this file in the current project's list instead
		if ( API::getInstance()->project() )
		{
			if (url.isRelativeURL(url.url())) {
				KURL relURL(API::getInstance()->project()->projectDirectory());
				relURL.addPath( url.url() );

				kdDebug() << k_funcinfo << "Looking for file in project dir: " << API::getInstance()->project()->projectDirectory() << " url " << url.url() << " transformed to " << relURL.url() << ": " << done << endl;
				if (relURL.isValid() && KIO::NetAccess::exists(relURL, false, 0)) {
					url = relURL;
					done = true;
				}
				else {
					KURL relURL(API::getInstance()->project()->buildDirectory());
					relURL.addPath( url.url() );
					kdDebug() << k_funcinfo << "Looking for file in build dir: " << API::getInstance()->project()->buildDirectory() << " url " << url.url() << " transformed to " << relURL.url() << ": " << done << endl;
					if (relURL.isValid() && KIO::NetAccess::exists(relURL, false, 0)) {
						url = relURL;
						done = true;
					}
				}
			}

			if (!done) {
				url = findURLInProject(url);

				if ( !url.isValid() || !KIO::NetAccess::exists(url, false, 0) )
					// See if this url is relative to the current project's directory
					url = API::getInstance()->project()->projectDirectory() + "/" + url.path();

				else
					done = true;
			}
		}

 		if ( !done && ( !url.isValid() || !KIO::NetAccess::exists(url, false, 0) ))
		{
			// Not found - prompt the user to find it?
			kdDebug(9000) << "cannot find URL: " << url.url() << endl;
			return;
		}
	}

  // We now have a url that exists ;)

  // clean it and resolve possible symlink
	url.cleanPath(true);
	if (url.isLocalFile())
	{
		QString path = url.path();
		path = URLUtil::canonicalPath(path);
		if ( !path.isEmpty() )
			url.setPath(path);
	}

	// is it already open?
	KParts::Part *existingPart = partForURL(url);
	if (existingPart)
	{
		addHistoryEntry();
		activatePart(existingPart);
		EditorProxy::getInstance()->setLineNumber(existingPart, lineNum, col);
		return;
	}

    if ( !addToCurrentBuffer )
    {
        if ( KDevLanguageSupport *lang =
             API::getInstance()->languageSupport() )
        {
            // Let the language part override the addToCurrentBuffer flag
            // if it decides to...
            addToCurrentBuffer = lang->shouldSplitDocument( inputUrl );

            if ( addToCurrentBuffer )
            {
                kdDebug(9000) <<
                    "languagePart() insists addToCurrentBuffer = true" << endl;
                // Set activate = true, otherwise we have hard to fix
                // multi-buffer delayed activation.
                // I'll re-look at this later...
                activate = true;
            }
        }
    }

	KMimeType::Ptr MimeType = KMimeType::findByURL( url );

	kdDebug(9000) << "mimeType = " << MimeType->name() << endl;

	// is the URL pointing to a directory?
	if ( MimeType->is( "inode/directory" ) )
	{
		return;
	}

	if ( !m_presetEncoding.isNull() )
	{
		m_openNextAsText = true;
	}

	KConfig *config = kapp->config();
	config->setGroup("General Options");

	// we don't trust KDE with designer files, let's handle it ourselves
	if ( !m_openNextAsText && MimeType->is( "application/x-designer" ) )
	{
		QString DesignerSetting = config->readEntry( "DesignerSetting", "ExternalDesigner" );
		QString designerExec = "designer";
		QStringList designerPluginPaths;
		QDomDocument* dom = API::getInstance()->projectDom();
		if ( dom != 0 )
		{
			// The global option specifies a fallback if the project
			// has no setting or no project is open. However for Qt4
			// projects we want to use ExternalDesigner in any case.
			if ( DomUtil::readIntEntry( *dom, "/kdevcppsupport/qt/version", 3 ) == 4 )
			{
				designerPluginPaths = DomUtil::readListEntry(*dom, "/kdevcppsupport/qt/designerpluginpaths", "path" );
				DesignerSetting = "ExternalDesigner";
			}

			DesignerSetting = DomUtil::readEntry(*dom, "/kdevcppsupport/qt/designerintegration", DesignerSetting  );
			designerExec = DomUtil::readEntry(*dom, "/kdevcppsupport/qt/designer", designerExec );
		}
		if ( DesignerSetting == "ExternalKDevDesigner" )
		{
			designerExec = "kdevdesigner";
		}
		else if ( DesignerSetting == "EmbeddedKDevDesigner" )
		{
			if ( KParts::ReadOnlyPart *designerPart = qtDesignerPart() )
			{
				addHistoryEntry();
				activatePart(designerPart);
				designerPart->openURL(url);
				return;
			}
			else if ( KParts::Factory * KDevDesignerFactory = static_cast<KParts::Factory*>( KLibLoader::self()->factory( QFile::encodeName( "libkdevdesignerpart" ) ) ) )
			{
				KParts::ReadWritePart * kdevpart = static_cast<KParts::ReadWritePart*>( KDevDesignerFactory->createPart( TopLevel::getInstance()->main(), 0, 0, 0, "KParts::ReadWritePart"  ) );
				kdevpart->openURL( url );
				addHistoryEntry();
				integratePart( kdevpart, url );
				m_openRecentAction->addURL( url );
				m_openRecentAction->saveEntries( kapp->config(), "RecentFiles" );
				return;
			}
		}

		if( designerPluginPaths.isEmpty() )
			KRun::runCommand( designerExec+" "+url.pathOrURL() );
		else
			KRun::runCommand( "QT_PLUGIN_PATH=\""+designerPluginPaths.join(":")+"\" "+designerExec+" "+url.pathOrURL() );

		return;
	}

	config->setGroup("General");

	QStringList texttypeslist = config->readListEntry( "TextTypes" );
	if ( texttypeslist.contains( MimeType->name() ) )
	{
		m_openNextAsText = true;
	}

	bool isText = false;
  	QVariant v = MimeType->property("X-KDE-text");
     	if (v.isValid())
           isText = v.toBool();
	// is this regular text - open in editor
	if ( m_openNextAsText || isText || MimeType->is( "application/x-zerosize" ) )
	{
        KTextEditor::Editor *editorpart =
            createEditorPart( activate, addToCurrentBuffer, url );
        if ( editorpart )
        {
            if ( m_presetEncoding.isNull() && API::getInstance()->projectDom() )
            {
                QDomDocument * projectDom = API::getInstance()->projectDom();
                m_presetEncoding = DomUtil::readEntry( *projectDom, "/general/defaultencoding", QString::null );
            }

            if ( !m_presetEncoding.isNull() )
            {
                if ( KTextEditor::EncodingInterface * ei = dynamic_cast<KTextEditor::EncodingInterface*>( editorpart ) )
                {
                    ei->setEncoding( m_presetEncoding );
                }
                m_presetEncoding = QString::null;
            }

            addHistoryEntry();

            QWidget * widget = EditorProxy::getInstance()->topWidgetForPart( editorpart );

            integratePart(editorpart, url, widget, true, activate, addToCurrentBuffer);
            EditorProxy::getInstance()->setLineNumber(editorpart, lineNum, col);

            m_openNextAsText = false;

            m_openRecentAction->addURL( url );
            m_openRecentAction->saveEntries( kapp->config(), "RecentFiles" );

            return;
        }
	}

	// OK, it's not text and it's not a designer file.. let's see what else we can come up with..

	KParts::Factory *factory = 0;
	QString className;

	QString services[] = { "KParts/ReadWritePart", "KParts/ReadOnlyPart" };
	QString classnames[] = { "KParts::ReadWritePart", "KParts::ReadOnlyPart" };
	for (uint i=0; i<2; ++i)
	{
		factory = findPartFactory( MimeType->name(), services[i] );
		if (factory)
		{
			className = classnames[i];
			break;
		}
	}

	kdDebug(9000) << "factory = " << factory << endl;

	if (factory)
	{
		// create the object of the desired class
		KParts::ReadOnlyPart *part = static_cast<KParts::ReadOnlyPart*>( factory->createPart( TopLevel::getInstance()->main(), 0, 0, 0, className.latin1() ) );
		if ( part )
		{
			part->openURL( url );
			addHistoryEntry();

			if ( dynamic_cast<KTextEditor::Editor*>( part ) ) // we can have ended up with a texteditor, in which case need to treat it as such
			{
				integratePart(part, url, part->widget(), true, activate);
				EditorProxy::getInstance()->setLineNumber(part, lineNum, col);
			}
			else
			{
				integratePart( part, url );
			}

			m_openRecentAction->addURL( url );
			m_openRecentAction->saveEntries( kapp->config(), "RecentFiles" );
		}
	}
	else
	{
		MimeWarningDialog dlg;
		dlg.text2->setText( QString( "<qt><b>%1</b></qt>" ).arg(url.path()));
		dlg.text3->setText( dlg.text3->text().arg(MimeType->name()) );

		if ( dlg.exec() == QDialog::Accepted )
		{
			if ( dlg.open_with_kde->isChecked() )
			{
				KRun::runURL(url, MimeType->name() );
			}
			else
			{
				if ( dlg.always_open_as_text->isChecked() )
				{
					KConfig *config = kapp->config();
					config->setGroup("General");
					QStringList texttypeslist = config->readListEntry( "TextTypes" );
					texttypeslist << MimeType->name();
					config->writeEntry( "TextTypes", texttypeslist );
				}
				m_openNextAsText = true;
				editDocument( url, lineNum, col );
			}
		}
	}
}


void PartController::showDocument(const KURL &url, bool newWin)
{
  QString fixedPath = HTMLDocumentationPart::resolveEnvVarsInURL(url.url()); // possibly could env vars
  KURL docUrl(fixedPath);
  kdDebug(9000) << "SHOW: " << docUrl.url() << endl;

  if ( docUrl.isLocalFile() && KMimeType::findByURL(docUrl)->name() != "text/html" ) {
    // a link in a html-file pointed to a local text file - display
    // it in the editor instead of a html-view to avoid uglyness
    editDocument( docUrl );
    return;
  }

  addHistoryEntry();

  HTMLDocumentationPart *part = dynamic_cast<HTMLDocumentationPart*>(activePart());
  if (!part || newWin)
  {
    part = new HTMLDocumentationPart;
    integratePart(part,docUrl);
    connect(part, SIGNAL(fileNameChanged(KParts::ReadOnlyPart* )),
        this, SIGNAL(partURLChanged(KParts::ReadOnlyPart* )));
  }
  else
  {
    activatePart(part);
  }
  part->openURL(docUrl);
}

KParts::Factory *PartController::findPartFactory(const QString &mimeType, const QString &partType, const QString &preferredName)
{
  KTrader::OfferList offers = KTrader::self()->query(mimeType, QString("'%1' in ServiceTypes").arg(partType));

  if (offers.count() > 0)
  {
    KService::Ptr ptr = 0;
    // if there is a preferred plugin we'll take it
    if ( !preferredName.isEmpty() ) {
      KTrader::OfferList::Iterator it;
      for (it = offers.begin(); it != offers.end(); ++it) {
        if ((*it)->desktopEntryName() == preferredName) {
          ptr = (*it);
        }
      }
    }
    // else we just take the first in the list
    if ( !ptr ) {
      ptr = offers.first();
    }
    return static_cast<KParts::Factory*>(KLibLoader::self()->factory(QFile::encodeName(ptr->library())));
  }

  return 0;
}

KTextEditor::Editor * PartController::createEditorPart( bool activate,
                                                        bool addToCurrentBuffer,
                                                        const KURL &url )
{
    MultiBuffer *multiBuffer = 0;
    if ( addToCurrentBuffer )
    {
        multiBuffer =
            dynamic_cast<MultiBuffer*>(
                    EditorProxy::getInstance()->topWidgetForPart( activePart() )
                                      );
    }
    if ( !multiBuffer )
    {
        kdDebug(9000) << "Creating a new MultiBuffer for "
            << url.fileName() << endl;
        multiBuffer = new MultiBuffer( TopLevel::getInstance()->main() );
    }

    static bool alwaysActivate = true;

    kapp->config()->setGroup("Editor");
    QString preferred = kapp->config()->readPathEntry("EmbeddedKTextEditor");
    // If we are not using kyzis...
    // Don't create non-wrapped views for now,
    // avoid two paths (== two chances for bad bugs)
    if ( preferred != "kyzispart" )
        alwaysActivate = false;

    KTextEditor::Editor *editorpart =
        dynamic_cast<KTextEditor::Editor*>(multiBuffer->createPart( "text/plain",
            "KTextEditor/Document",
            alwaysActivate | activate ?
            "KTextEditor::Editor" : "KTextEditor::Document",
            preferred
            ));

    if ( url.isValid() )
        editorpart->openURL( url );

    multiBuffer->registerURL( url, editorpart );
    multiBuffer->setDelayedActivation( !activate );
    return editorpart;
}

void PartController::integratePart(KParts::Part *part, const KURL &url,
                                   QWidget* widget, bool isTextEditor,
                                   bool activate, bool addToCurrentBuffer )
{
  if (!widget) widget = part->widget();

  if (!widget) {
    /// @todo error handling
      kdDebug(9000) << "no widget for this part!!" << endl;
      return; // to avoid later crash
  }

  if ( !addToCurrentBuffer )
    TopLevel::getInstance()->embedPartView(widget, url.fileName(), url.url());

  addPart(part, activate);

  // tell the parts we loaded a document
  KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(part);
  if ( !ro_part ) return;

  emit loadedFile( ro_part->url() );

  connect( part, SIGNAL(modifiedOnDisc(Kate::Document*, bool, unsigned char)), this, SLOT(slotDocumentDirty(Kate::Document*, bool, unsigned char)) );

  // let's get notified when a document has been changed
  connect(part, SIGNAL(completed()), this, SLOT(slotUploadFinished()));

  // yes, we're cheating again. this signal exists for katepart's
  // Document object and our HTMLDocumentationPart
//  connect(part, SIGNAL(fileNameChanged()), this, SLOT(slotFileNameChanged()));

  // Connect to the document's views newStatus() signal in order to keep track of the
  // modified-status of the document.

  if (isTextEditor)
    integrateTextEditorPart(static_cast<KTextEditor::Document*>(part));

  KInterfaceDesigner::Designer *designerPart = dynamic_cast<KInterfaceDesigner::Designer *>(part);
  if (designerPart && API::getInstance()->languageSupport())
  {
      kdDebug() << "integrating designer part with language support" << endl;
      connect(designerPart, SIGNAL(addedFunction(DesignerType, const QString&, Function )),
          API::getInstance()->languageSupport(),
          SLOT(addFunction(DesignerType, const QString&, Function )));
      connect(designerPart, SIGNAL(editedFunction(DesignerType, const QString&, Function, Function )), API::getInstance()->languageSupport(),
      SLOT(editFunction(DesignerType, const QString&, Function, Function )));
      connect(designerPart, SIGNAL(removedFunction(DesignerType, const QString&, Function )),
          API::getInstance()->languageSupport(),
          SLOT(removeFunction(DesignerType, const QString&, Function )));
      connect(designerPart, SIGNAL(editFunction(DesignerType, const QString&, const QString& )),
          API::getInstance()->languageSupport(),
          SLOT(openFunction(DesignerType, const QString&, const QString& )));
      connect(designerPart, SIGNAL(editSource(DesignerType, const QString& )),
          API::getInstance()->languageSupport(),
          SLOT(openSource(DesignerType, const QString& )));
      connect(designerPart, SIGNAL(newStatus(const QString &, int)),
          this, SLOT(slotNewDesignerStatus(const QString &, int)));
  }
}

void PartController::integrateTextEditorPart(KTextEditor::Document* doc)
{
  // There's shortcut conflict between Kate and the debugger, resolve
  // it here. Ideally, the should be some standard mechanism, configurable
  // by config files.
  // However, it does not exists and situation here some rare commands
  // like "Dynamic word wrap" or "Show line numbers" from Kate take
  // all possible shortcuts, leaving us with IDE that has no shortcuts for
  // debugger, is very bad.
  //
  // We could try to handle this in debugger, but that would require
  // the debugger to intercept all new KTextEditor::View creations, which is
  // not possible.


  if ( !doc ) return;

  connect( doc, SIGNAL(textChanged()), this, SLOT(textChanged()) );
  connect( doc, SIGNAL(fileNameChanged()),
    this, SLOT(slotDocumentUrlChanged()));

  if( doc->widget() )
  {
      connect( doc->widget(), SIGNAL(dropEventPass(QDropEvent *)),
        TopLevel::getInstance()->main(), SLOT(slotDropEvent(QDropEvent *)) );
  }

  if ( KTextEditor::View * view = dynamic_cast<KTextEditor::View*>( doc->widget() ) )
  {
    KActionCollection* c = view->actionCollection();
    // Be extra carefull, in case the part either don't provide those
    // action, or uses different shortcuts.

    if (KAction* a = c->action("view_folding_markers"))
    {
      if (a->shortcut() == KShortcut(Key_F9))
        a->setShortcut(KShortcut());
    }

    if (KAction* a = c->action("view_dynamic_word_wrap"))
    {
      if (a->shortcut() == KShortcut(Key_F10))
        a->setShortcut(KShortcut());
    }

    if (KAction* a = c->action("view_line_numbers"))
    {
      if (a->shortcut() == KShortcut(Key_F11))
        a->setShortcut(KShortcut());
    }
  }

  //EditorProxy::getInstance()->installPopup(doc, contextPopupMenu());

  // What's potentially problematic is that this signal isn't officially part of the
  // KTextEditor::View interface. It is nevertheless there, and used in kate and kwrite.
  // There doesn't seem to be any othere way of making this work with katepart, and since
  // signals are dynamic, if we try to connect to an editorpart that lacks this signal,
  // all we get is a runtime warning. At this point in time we are only really supported
  // by katepart anyway so IMHO this hack is justified. //teatime
  QPtrList<KTextEditor::View> list = doc->views();
  QPtrListIterator<KTextEditor::View> it( list );
  while ( it.current() )
  {
    connect( it, SIGNAL( newStatus() ), this, SLOT( slotNewStatus() ) );
    ++it;
  }
}

void PartController::slotPartAdded( KParts::Part * part )
{
	kdDebug(9000) << k_funcinfo << endl;

	if ( KParts::ReadOnlyPart * ro_part = dynamic_cast<KParts::ReadOnlyPart*>( part ) )
	{
		updatePartURL( ro_part );
	}

	updateMenuItems();
}

void PartController::slotPartRemoved( KParts::Part * part )
{
	kdDebug(9000) << k_funcinfo << endl;

	_partURLMap.remove( static_cast<KParts::ReadOnlyPart*>(part) );

	if ( part == m_currentActivePart )
	{
		m_removingActivePart = true;
	}

	updateMenuItems();
}

void PartController::updatePartURL( KParts::ReadOnlyPart * ro_part )
{
	if ( ro_part->url().isEmpty() )
	{
		kdDebug(9000) << "updatePartURL() called with empty URL for part: " << ro_part << endl;
		return;
	}
	_partURLMap[ ro_part ] = ro_part->url();
}

bool PartController::partURLHasChanged( KParts::ReadOnlyPart * ro_part )
{
	if ( _partURLMap.contains( ro_part ) && !ro_part->url().isEmpty() )
	{
		if ( _partURLMap[ ro_part ] != ro_part->url() )
		{
			return true;
		}
	}
	return false;
}

KURL PartController::storedURLForPart( KParts::ReadOnlyPart * ro_part )
{
	if ( _partURLMap.contains( ro_part ) )
	{
		return _partURLMap[ ro_part ];
	}
	return KURL();
}


void PartController::slotUploadFinished()
{
	KParts::ReadOnlyPart *ro_part = const_cast<KParts::ReadOnlyPart*>( dynamic_cast<const KParts::ReadOnlyPart*>(sender()) );
	if ( !ro_part ) return;

	if ( partURLHasChanged( ro_part ) )
	{
		emit partURLChanged( ro_part );
		updatePartURL( ro_part );
	}
}

KParts::ReadOnlyPart *PartController::partForURL(const KURL &url)
{
	QPtrListIterator<KParts::Part> it(*parts());
	for ( ; it.current(); ++it)
	{
		KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(it.current());
		if (ro_part && url.path() == ro_part->url().path())
			return ro_part;
	}
	return 0;
}

KParts::Part * PartController::partForWidget( const QWidget * widget )
{
	QPtrListIterator<KParts::Part> it(*parts());
	for ( ; it.current(); ++it)
	{
		if ( it.current()->widget() == widget )
		{
			return *it;
		}
	}
	return 0;
}


void PartController::activatePart(KParts::Part *part)
{
  if ( !part ) return;

  QWidget * widget = EditorProxy::getInstance()->widgetForPart( part );
  if (widget)
  {
    TopLevel::getInstance()->raiseView( widget );
    widget->show();
    widget->setFocus();
  }

  setActivePart(part);

  QWidget* w2 = EditorProxy::getInstance()->widgetForPart( part );
  if (w2 != widget)
    w2->setFocus();
}

bool PartController::closePart(KParts::Part *part)
{
    KParts::ReadOnlyPart * ro_part =
        dynamic_cast<KParts::ReadOnlyPart*>( part );

    if ( !ro_part ) return true;

    KURL url = ro_part->url();

    if (QWidget* w = EditorProxy::getInstance()->topWidgetForPart( part ) )
    {
        if ( MultiBuffer *multiBuffer = dynamic_cast<MultiBuffer*>( w ) )
        {
            kdDebug(9000) << "About to delete MultiBuffered document..."
                << " numberOfBuffers: " << multiBuffer->numberOfBuffers()
                << " isActivated: " << multiBuffer->isActivated()
                << endl;
            if ( !multiBuffer->closeURL( url ) )
                return false;

            if ( multiBuffer->numberOfBuffers() == 0
                 || !multiBuffer->isActivated() )
            {
                TopLevel::getInstance()->removeView( w );
                _dirtyDocuments.remove( static_cast<KParts::ReadWritePart*>( ro_part ) );
                emit closedFile( url );
/*                kdDebug(9000) << "Deleting MultiBuffer Part" << endl;*/
                TopLevel::getInstance()->main()->guiFactory()->removeClient( part );
                delete part;
/*                kdDebug(9000) << "DeleteLater Actual MultiBuffer" << endl;*/
                multiBuffer->deleteLater();
                return true;
            }
            else
            {
/*                kdDebug(9000) << "Deleting MultiBuffer Part" << endl;*/
                _dirtyDocuments.remove( static_cast<KParts::ReadWritePart*>( ro_part ) );
                TopLevel::getInstance()->main()->guiFactory()->removeClient( part );
                emit closedFile( url );
                delete part;
                // Switch to a remaining buffer
                setActivePart( multiBuffer->activeBuffer() );
                return true;
            }
        }
        else if ( !ro_part->closeURL() )
            return false;
	TopLevel::getInstance()->removeView( w );
    }
    else if ( !ro_part->closeURL() )
        return false;
    TopLevel::getInstance()->main()->guiFactory()->removeClient( part );

    _dirtyDocuments.remove( static_cast<KParts::ReadWritePart*>( ro_part ) );
    emit closedFile( url );

/*    kdDebug(9000) << "Deleting Regular Part" << endl;*/
    delete part;
    return true;
}


void PartController::updateMenuItems()
{
  bool hasWriteParts = false;
  bool hasReadOnlyParts = false;

  QPtrListIterator<KParts::Part> it(*parts());
  for ( ; it.current(); ++it)
  {
    if (it.current()->inherits("KParts::ReadWritePart"))
      hasWriteParts = true;
    if (it.current()->inherits("KParts::ReadOnlyPart"))
      hasReadOnlyParts = true;
  }

  m_saveAllFilesAction->setEnabled(hasWriteParts);
  m_revertAllFilesAction->setEnabled(hasWriteParts);
  m_closeWindowAction->setEnabled(hasReadOnlyParts);
  m_closeAllWindowsAction->setEnabled(hasReadOnlyParts);
  m_closeOtherWindowsAction->setEnabled(hasReadOnlyParts);

  m_backAction->setEnabled( !m_backHistory.isEmpty() );
}

void PartController::slotRevertAllFiles()
{
	revertAllFiles();
}

void PartController::reloadFile( const KURL & url )
{
	KParts::ReadWritePart * part = dynamic_cast<KParts::ReadWritePart*>( partForURL( url ) );
	if ( part )
	{
		if ( part->isModified() )
		{
			if ( KMessageBox::warningYesNo( TopLevel::getInstance()->main(),
				i18n( "The file \"%1\" is modified in memory. Are you sure you want to reload it? (Local changes will be lost.)" ).arg( url.path() ),
				i18n( "File is Modified" ), i18n("Reload"), i18n("Do Not Reload") ) == KMessageBox::Yes )
			{
				part->setModified( false );
			}
			else
			{
				return;
			}
		}

		unsigned int line = 0; unsigned int col = 0;
		KTextEditor::ViewCursorInterface * iface = dynamic_cast<KTextEditor::ViewCursorInterface*>( part->widget() );
		if (iface)
		{
			iface->cursorPositionReal( &line, &col );
		}

		part->openURL( url );

		_dirtyDocuments.remove( part );
		emit documentChangedState( url, Clean );

		if ( iface )
		{
			iface->setCursorPositionReal( line, col );
		}
	}
}

void PartController::revertFiles( const KURL::List & list  )
{
	KURL::List::ConstIterator it = list.begin();
	while ( it != list.end() )
	{
		reloadFile( *it );
		++it;
	}
}

void PartController::revertAllFiles()
{
	revertFiles( openURLs() );
}

void PartController::slotCloseWindow()
{
	closePart( activePart() );
}

KURL::List PartController::modifiedDocuments()
{
	KURL::List modFiles;

	QPtrListIterator<KParts::Part> it( *parts() );
	while( it.current() )
	{
		KParts::ReadWritePart *rw_part = dynamic_cast<KParts::ReadWritePart*>(it.current());
		if ( rw_part && rw_part->isModified() )
		{
			modFiles << rw_part->url();
		}
		++it;
	}
	return modFiles;
}

void PartController::slotSave()
{
	kdDebug(9000) << k_funcinfo << endl;

	if ( KParts::ReadWritePart * part = dynamic_cast<KParts::ReadWritePart*>( activePart() ) )
	{
		saveFile( part->url() );
	}
}

void PartController::slotReload()
{
	kdDebug(9000) << k_funcinfo << endl;

	if ( KParts::ReadWritePart * part = dynamic_cast<KParts::ReadWritePart*>( activePart() ) )
	{
		reloadFile( part->url() );
	}
}

void PartController::slotSaveAllFiles()
{
  saveAllFiles();
}

bool PartController::saveFile( const KURL & url, bool force )
{
	KParts::ReadWritePart * part = dynamic_cast<KParts::ReadWritePart*>( partForURL( url ) );
	if ( !part ) return true;

	switch( documentState( url ) )
	{
		case Clean:
			if ( !force )
			{
				return true;
			}
			kdDebug(9000) << "Forced save" << endl;
			break;

		case Modified:
			kdDebug(9000) << "Normal save" << endl;
			break;

		case Dirty:
		case DirtyAndModified:
			{
				int code = KMessageBox::warningYesNoCancel( TopLevel::getInstance()->main(),
					i18n("The file \"%1\" is modified on disk.\n\nAre you sure you want to overwrite it? (External changes will be lost.)").arg( url.path() ),
					i18n("File Externally Modified"), i18n("Overwrite"), i18n("Do Not Overwrite") );
				if ( code == KMessageBox::Yes )
				{
					kdDebug(9000) << "Dirty save!!" << endl;
				}
				else if ( code == KMessageBox::No )
				{
					return true;
				}
				else
				{
					return false; // a 'false' return means to interrupt the process that caused the save
				}
			}
			break;

		default:
			;
	}

	if ( part->save() )
	{
		_dirtyDocuments.remove( part );
		emit documentChangedState( url, Clean );
		emit savedFile( url );
	}

	return true;
}

bool PartController::saveAllFiles()
{
	return saveFiles( openURLs() );
}

bool PartController::saveFiles( KURL::List const & filelist )
{
	KURL::List::ConstIterator it = filelist.begin();
	while ( it != filelist.end() )
	{
                if (saveFile( *it )==false)
                   return false; //user cancelled
		++it;
	}
        return true;
}

bool PartController::querySaveFiles()
{
	return saveFilesDialog( KURL::List() );
}

void PartController::clearModified( KURL::List const & filelist )
{
	KURL::List::ConstIterator it = filelist.begin();
	while ( it != filelist.end() )
	{
		KParts::ReadWritePart * rw_part = dynamic_cast<KParts::ReadWritePart*>( partForURL( *it ) );
		if ( rw_part )
		{
			rw_part->setModified( false );
		}
		++it;
	}
}

bool PartController::saveFilesDialog( KURL::List const & ignoreList )
{
	KURL::List modList = modifiedDocuments();

	if ( modList.count() > 0 && modList != ignoreList )
	{
		KSaveSelectDialog dlg( modList, ignoreList, TopLevel::getInstance()->main() );
		if ( dlg.exec() == QDialog::Accepted )
		{
			saveFiles( dlg.filesToSave() );
			clearModified( dlg.filesNotToSave() );
		}
		else
		{
			return false;
		}
	}
	return true;
}

bool PartController::closeFilesDialog( KURL::List const & ignoreList )
{
	if ( !saveFilesDialog( ignoreList ) ) return false;

	QPtrList<KParts::Part> partList( *parts() );
	QPtrListIterator<KParts::Part> it( partList );
	while ( KParts::Part* part = it.current() )
	{
		KParts::ReadOnlyPart * ro_part = dynamic_cast<KParts::ReadOnlyPart*>( part );
		if ( ro_part && !ignoreList.contains( ro_part->url() ) || !ro_part )
		{
			closePart( part );
		}
		++it;
	}
	return true;
}

bool PartController::closeFiles( const KURL::List & list )
{
	KURL::List::ConstIterator it = list.begin();
	while ( it != list.end() )
	{
		if ( !closePart( partForURL( *it ) ) )
		{
			return false;
		}
		++it;
	}
	return true;
}

bool PartController::closeFile( const KURL & url )
{
	return closePart( partForURL( url ) );
}

bool PartController::closeAllFiles()
{
	return closeFilesDialog( KURL::List() );
}

void PartController::slotCloseAllWindows()
{
	closeAllFiles();
}

bool PartController::closeAllOthers( const KURL & url )
{
	KURL::List ignoreList;
	ignoreList.append( url );

	return closeFilesDialog( ignoreList );
}


void PartController::slotCloseOtherWindows()
{
	if ( KParts::ReadOnlyPart * active = dynamic_cast<KParts::ReadOnlyPart*>( activePart() ) )
	{
		closeAllOthers( active->url() );
	}
}

void PartController::slotOpenFile()
{
	QString DefaultEncoding;
	if ( QDomDocument * projectDom = API::getInstance()->projectDom() )
	{
		DefaultEncoding = DomUtil::readEntry( *projectDom, "/general/defaultencoding", QString::null );
	}

	if ( DefaultEncoding.isEmpty() )
	{
		// have a peek at katepart's settings:
		KConfig * config = kapp->config();
		config->setGroup("Kate Document Defaults");
		DefaultEncoding = config->readEntry("Encoding", QString::null );
	}

	KEncodingFileDialog::Result result = KEncodingFileDialog::getOpenURLsAndEncoding( DefaultEncoding, QString::null,
		QString::null, TopLevel::getInstance()->main(), QString::null );

	for ( KURL::List::Iterator it = result.URLs.begin(); it != result.URLs.end(); ++it )
	{
		m_presetEncoding = result.encoding;
		editDocument( *it );
	}
}

void PartController::slotOpenRecent( const KURL& url )
{
  editDocument( url );
  // stupid bugfix - don't allow an active item in the list
  m_openRecentAction->setCurrentItem( -1 );
}

bool PartController::readyToClose()
{
	blockSignals( true );

	closeAllFiles(); // this should never return false, as the files are already saved

	return true;
}

void PartController::slotActivePartChanged( KParts::Part *part )
{
	kdDebug(9000) << k_funcinfo << part << endl;

	if ( !m_isJumping && !m_removingActivePart )
	{
		if ( KParts::ReadOnlyPart * ro_part = dynamic_cast<KParts::ReadOnlyPart*>(m_currentActivePart) )
		{
			addHistoryEntry( ro_part );
		}
	}

	m_currentActivePart = part;
	m_removingActivePart = false;

    if (part) {
        KXMLGUIClient* client = dynamic_cast<KXMLGUIClient*>(part->widget());
        if (client) Core::setupShourtcutTips(client);
    }

    updateMenuItems();
    QTimer::singleShot( 100, this, SLOT(slotWaitForFactoryHack()) );
}

void PartController::showPart( KParts::Part* part, const QString& name, const QString& shortDescription )
{
  if (!part->widget()) {
    /// @todo error handling
    return; // to avoid later crash
  }

  QPtrListIterator<KParts::Part> it(*parts());
  for ( ; it.current(); ++it)
  {
    if( it.current() == part ){
        // part already embedded
	activatePart( it.current() );
	return;
    }
  }

  // embed the part
  TopLevel::getInstance()->embedPartView( part->widget(), name, shortDescription );
  addPart( part );
}

void PartController::slotDocumentDirty( Kate::Document * d, bool isModified, unsigned char reason )
{
	kdDebug(9000) << k_funcinfo << endl;

	//	KTextEditor::Document * doc = reinterpret_cast<KTextEditor::Document*>( d ); // theoretically unsafe in MI scenario
	KTextEditor::Document * doc = 0;

	QPtrListIterator<KParts::Part> it( *parts() );
	while( it.current() )
	{
		if ( (void*)it.current() == (void*)d )
		{
			doc = dynamic_cast<KTextEditor::Document*>( it.current() );
			break;
		}
		++it;
	}

	// this is a bit strange, but in order to avoid weird crashes
	// down in KDirWatcher, we want to step off the call chain before
	// opening any messageboxes
	if ( doc )
	{
		ModificationData * p = new ModificationData;
		p->doc = doc;
		p->isModified = isModified;
		p->reason = reason;
		KDevJobTimer::singleShot( 0, this, SLOT(slotDocumentDirtyStepTwo(void*)), p );
	}
}

void PartController::slotDocumentDirtyStepTwo( void * payload )
{
	if ( !payload ) return;

	ModificationData * p = reinterpret_cast<ModificationData*>( payload );
	KTextEditor::Document * doc = p->doc;

	// let's make sure the document is still loaded
	bool haveDocument = false;
	if( const QPtrList<KParts::Part> * partlist = parts() )
	{
		QPtrListIterator<KParts::Part> it( *partlist );
		while ( KParts::Part * part = it.current() )
		{
			if ( p->doc == dynamic_cast<KTextEditor::Document*>( part ) )
			{
				haveDocument = true;
				break;
			}
			++it;
		}
	}
	if ( !haveDocument ) return;

	bool isModified = p->isModified;
	unsigned char reason = p->reason;
	delete p;

	KURL url = storedURLForPart( doc );
	if ( url.isEmpty() )
	{
		kdDebug(9000) << "Warning!! the stored url is empty. Bailing out!" << endl;
	}

	if ( reason > 0 )
	{
		if ( !_dirtyDocuments.contains( doc ) )
		{
			_dirtyDocuments.append( doc );
		}

		if ( reactToDirty( url, reason ) )
		{
			// file has been reloaded
			emit documentChangedState( url, Clean );
			_dirtyDocuments.remove( doc );
		}
		else
		{
			doEmitState( url );
		}
	}
	else
	{
		_dirtyDocuments.remove( doc );
		emit documentChangedState( url, Clean );
	}

	kdDebug(9000) << doc->url().url() << endl;
	kdDebug(9000) << isModified << endl;
	kdDebug(9000) << reason << endl;
}

bool PartController::isDirty( KURL const & url )
{
	return _dirtyDocuments.contains( static_cast<KTextEditor::Document*>( partForURL( url ) ) );
}

bool PartController::reactToDirty( KURL const & url, unsigned char reason )
{
    KConfig *config = kapp->config();
    config->setGroup("Editor");
	QString dirtyAction = config->readEntry( "DirtyAction" );

	if ( dirtyAction == "nothing" ) return false;

	bool isModified = true;
	if( KParts::ReadWritePart * part = dynamic_cast<KParts::ReadWritePart*>( partForURL( url ) ) )
	{
		isModified = part->isModified();
	}
	else
	{
		kdDebug(9000) << k_funcinfo << " Warning. Not a ReadWritePart." << endl;
		return false;
	}

	if ( isModified )
	{
		KMessageBox::sorry( TopLevel::getInstance()->main(),
			i18n("Conflict: The file \"%1\" has changed on disk while being modified in memory.\n\n"
					"You should investigate before saving to make sure you are not losing data.").arg( url.path() ),
			i18n("Conflict") );
		return false;
	}

	if ( reason == 3 ) // means the file was deleted
	{
		KMessageBox::sorry( TopLevel::getInstance()->main(),
			i18n("Warning: The file \"%1\" has been deleted on disk.\n\n"
					"If this was not your intention, make sure to save this file now.").arg( url.path() ),
			i18n("File Deleted") );
		return false;
	}

	if ( dirtyAction == "alert" )
	{
		if ( KMessageBox::warningYesNo( TopLevel::getInstance()->main(),
	   		i18n("The file \"%1\" has changed on disk.\n\nDo you want to reload it?").arg( url.path() ),
			i18n("File Changed"), i18n("Reload"), i18n("Do Not Reload") ) == KMessageBox::No )
		{
			return false;
		}
	}

	// here we either answered yes above or are in autoreload mode
	reloadFile( url );

	return true;
}

void PartController::slotNewDesignerStatus(const QString &formName, int status)
{
	kdDebug(9000) << k_funcinfo << endl;
	kdDebug(9000) << " formName: " << formName << ", status: " << status << endl;
	emit documentChangedState( KURL::fromPathOrURL(formName), DocumentState(status) );
}

void PartController::slotNewStatus( )
{
	kdDebug(9000) << k_funcinfo << endl;

	QObject * senderobj = const_cast<QObject*>( sender() );
	KTextEditor::View * view = dynamic_cast<KTextEditor::View*>( senderobj );
	if ( view )
	{
		doEmitState( view->document()->url() );
	}
}

DocumentState PartController::documentState( KURL const & url )
{
	KParts::ReadWritePart * rw_part = dynamic_cast<KParts::ReadWritePart*>( partForURL( url ) );
	if ( !rw_part ) return Clean;

	DocumentState state = Clean;
	if ( rw_part->isModified() )
	{
		state = Modified;
	}

	if ( isDirty( url ) )
	{
		if ( state == Modified )
		{
			state = DirtyAndModified;
		}
		else
		{
			state = Dirty;
		}
	}

	return state;
}

void PartController::doEmitState( KURL const & url )
{
	emit documentChangedState( url, documentState( url ) );
}

KURL::List PartController::openURLs( )
{
	KURL::List list;
	QPtrListIterator<KParts::Part> it(*parts());
	for ( ; it.current(); ++it)
	{
		if ( KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(it.current()) )
		{
			list << ro_part->url();
		}
	}
	return list;
}

/////////////////////////////////////////////////////////////////////////////

//BEGIN History methods

PartController::HistoryEntry::HistoryEntry( const KURL & u, int l, int c)
	: url(u), line(l), col(c)
{
	id = abs( QTime::currentTime().msecsTo( QTime() ) );	// should provide a reasonably unique number
}

void PartController::slotBack()
{
	HistoryEntry thatEntry = m_backHistory.front();
	m_backHistory.pop_front();
	m_backAction->setEnabled( !m_backHistory.isEmpty() );

	HistoryEntry thisEntry = createHistoryEntry();
	if ( !thisEntry.url.isEmpty() )
	{
		m_forwardHistory.push_front( thisEntry );
		m_forwardAction->setEnabled( true );
	}

	jumpTo( thatEntry );
}

void PartController::slotForward()
{
	HistoryEntry thatEntry = m_forwardHistory.front();
	m_forwardHistory.pop_front();
	m_forwardAction->setEnabled( !m_forwardHistory.isEmpty() );

	HistoryEntry thisEntry = createHistoryEntry();
	if ( !thisEntry.url.isEmpty() )
	{
		m_backHistory.push_front( thisEntry );
		m_backAction->setEnabled( true );
	}

	jumpTo( thatEntry );
}

void PartController::slotBackAboutToShow()
{
	KPopupMenu *popup = m_backAction->popupMenu();
	popup->clear();

	if ( m_backHistory.isEmpty()) return;

	int i = 0;
	QValueList<HistoryEntry>::ConstIterator it = m_backHistory.begin();
	while( i < 10 && it != m_backHistory.end() )
	{
		popup->insertItem( (*it).url.fileName() + QString(" (%1)").arg( (*it).line +1), (*it).id );
		++i;
		++it;
	}
}

void PartController::slotForwardAboutToShow( )
{
	KPopupMenu * popup = m_forwardAction->popupMenu();
	popup->clear();

	if ( m_forwardHistory.isEmpty() ) return;

	int i = 0;
	QValueList<HistoryEntry>::ConstIterator it = m_forwardHistory.begin();
	while( i < 10 && it != m_forwardHistory.end() )
	{
		popup->insertItem( (*it).url.fileName() + QString(" (%1)").arg( (*it).line +1), (*it).id );
		++i;
		++it;
	}
}

void PartController::slotBackPopupActivated( int id )
{
	QValueList<HistoryEntry>::Iterator it = m_backHistory.begin();
	while( it != m_backHistory.end() )
	{
		if ( (*it).id == id )
		{
			HistoryEntry entry = *it;
			m_backHistory.erase( m_backHistory.begin(), ++it );
			m_backAction->setEnabled( !m_backHistory.isEmpty() );

			HistoryEntry thisEntry = createHistoryEntry();
			if ( !thisEntry.url.isEmpty() )
			{
				m_forwardHistory.push_front( thisEntry );
				m_forwardAction->setEnabled( true );
			}

			jumpTo( entry );
			return;
		}
		++it;
	}
}

void PartController::slotForwardPopupActivated( int id )
{
	QValueList<HistoryEntry>::Iterator it = m_forwardHistory.begin();
	while( it != m_forwardHistory.end() )
	{
		if ( (*it).id == id )
		{
			HistoryEntry entry = *it;
			m_forwardHistory.erase( m_forwardHistory.begin(), ++it );
			m_forwardAction->setEnabled( !m_forwardHistory.isEmpty() );

			HistoryEntry thisEntry = createHistoryEntry();
			if ( !thisEntry.url.isEmpty() )
			{
				m_backHistory.push_front( thisEntry );
				m_backAction->setEnabled( true );
			}

			jumpTo( entry );
			return;
		}
		++it;
	}
}

void PartController::jumpTo( const HistoryEntry & entry )
{
	m_isJumping = true;
	editDocument( entry.url, entry.line, entry.col );
	m_isJumping = false;
}


PartController::HistoryEntry PartController::createHistoryEntry( KParts::ReadOnlyPart * ro_part ) {
    if( ro_part == 0 )
        ro_part = dynamic_cast<KParts::ReadOnlyPart*>( activePart() );

    if ( !ro_part ) return HistoryEntry();
    KTextEditor::ViewCursorInterface * cursorIface = dynamic_cast<KTextEditor::ViewCursorInterface*>( ro_part->widget() );
    if ( !cursorIface ) return HistoryEntry();

    uint line = 0;
    uint col = 0;
    cursorIface->cursorPositionReal( &line, &col );

    return HistoryEntry( ro_part->url(), line, col );
}


// this should be called _before_ a jump is made
void PartController::addHistoryEntry( KParts::ReadOnlyPart * part )
{
	if ( m_isJumping ) return;

	HistoryEntry thisEntry = createHistoryEntry( part );
	if ( !thisEntry.url.isEmpty() )
	{
		HistoryEntry lastEntry = m_backHistory.front();
		if ( (lastEntry.url.path() != thisEntry.url.path()) || (lastEntry.line != thisEntry.line) )
		{
			m_backHistory.push_front( thisEntry );
			m_backAction->setEnabled( true );

			m_forwardHistory.clear();
			m_forwardAction->setEnabled( false );
		}
		else
		{
			kdDebug(9000) << "** avoiding to create duplicate history entry **" << endl;
		}
	}
}

//END History methods

void PartController::slotWaitForFactoryHack( )
{
	//kdDebug(9000) << k_funcinfo << endl;

	if ( !activePart() ) return;

	if ( dynamic_cast<KTextEditor::View*>( activePart()->widget() ) )
	{
		if ( !activePart()->factory() )
		{
			QTimer::singleShot( 100, this, SLOT(slotWaitForFactoryHack()) );
    		return;
		}
		else
		{
			EditorProxy::getInstance()->installPopup( activePart() );
		}
	}

    if ( MultiBuffer *multiBuffer =
         dynamic_cast<MultiBuffer*>(
            EditorProxy::getInstance()->topWidgetForPart( activePart() ) )
       )
    {
        KURL url = dynamic_cast<KParts::ReadOnlyPart*>( activePart() )->url();
        multiBuffer->activePartChanged( url );
        // Really unfortunate, but the mainWindow relies upon this
        // to set the tab's icon
        emit documentChangedState( url, documentState( url ) );
    }
}

KParts::ReadOnlyPart *PartController::qtDesignerPart()
{
	QPtrListIterator<KParts::Part> it(*parts());
	for ( ; it.current(); ++it)
	{
		KInterfaceDesigner::Designer *des =  dynamic_cast<KInterfaceDesigner::Designer*>(it.current());
		if (des && des->designerType() == KInterfaceDesigner::QtDesigner)
			return des;
	}
	return 0;
}

KTextEditor::Editor *PartController::openTextDocument( bool activate )
{
    KTextEditor::Editor *editorpart =
        createEditorPart( activate, false, KURL( i18n("unnamed") ) );

    if ( editorpart )
    {
        if ( !m_presetEncoding.isNull() )
        {
            KParts::BrowserExtension * extension =
                KParts::BrowserExtension::childObject( editorpart );
            if ( extension )
            {
                KParts::URLArgs args;
                args.serviceType = QString( "text/plain;" ) + m_presetEncoding;
                extension->setURLArgs(args);
            }
            m_presetEncoding = QString::null;
        }

        QWidget * widget =
            EditorProxy::getInstance()->topWidgetForPart( editorpart );

        addHistoryEntry();
        integratePart(editorpart, KURL(i18n("unnamed")), widget, true, true);

        EditorProxy::getInstance()->setLineNumber(editorpart, 0, 0);
    }

    return editorpart;
}

void PartController::textChanged()
{
	if ( KTextEditor::Document * doc = dynamic_cast<KTextEditor::Document*>( activePart() ) )
	{
		if ( KTextEditor::ViewCursorInterface * vci = dynamic_cast<KTextEditor::ViewCursorInterface*>( doc->widget() ) )
		{
			m_gotoLastEditPosAction->setEnabled( true );

			m_lastEditPos.url = doc->url();
			vci->cursorPosition( &m_lastEditPos.pos.first, &m_lastEditPos.pos.second );
		}
	}
}

void PartController::gotoLastEditPos()
{
	editDocument( m_lastEditPos.url, m_lastEditPos.pos.first, m_lastEditPos.pos.second );
}

void PartController::slotDocumentUrlChanged()
{
    QObject *obj = const_cast<QObject*>(sender());
    KTextEditor::Document *doc = dynamic_cast<KTextEditor::Document*>( obj );
    if (!doc)
        return;

    MultiBuffer *multiBuffer = dynamic_cast<MultiBuffer*>(
        EditorProxy::getInstance()->findPartWidget( doc ));
    if (!multiBuffer)
        return;

    multiBuffer->updateUrlForPart(doc, doc->url());
    updatePartURL(doc);
    emit partURLChanged(doc);
}

#include "partcontroller.moc"

