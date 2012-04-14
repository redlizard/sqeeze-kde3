/***************************************************************************
*   Copyright (C) 1999 by Jonas Nordin                                    *
*   jonas.nordin@syncom.se                                                *
*   Copyright (C) 2000-2001 by Bernd Gehrmann                             *
*   bernd@kdevelop.org                                                    *
*   Copyright (C) 2002-2003 by Roberto Raggi                              *
*   roberto@kdevelop.org                                                  *
*   Copyright (C) 2003-2004 by Alexander Dymo                             *
*   adymo@mksat.net                                                       *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/


#include "cppsupportpart.h"
#include "cppsupport_events.h"
#include "problemreporter.h"
#include "backgroundparser.h"
#include "store_walker.h"
#include "ast.h"
#include "ast_utils.h"
#include "cppcodecompletion.h"
#include "ccconfigwidget.h"
#include "KDevCppSupportIface.h"
#include "cppsupportfactory.h"
#include "catalog.h"
#include "cpp_tags.h"
#include "kdevdriver.h"
#include "cppcodecompletionconfig.h"
#include "cppsplitheadersourceconfig.h"
#include "tag_creator.h"
#include "cppsupport_utils.h"
#include "classgeneratorconfig.h"
#include "urlutil.h"
#include "creategettersetterconfiguration.h"
#include "kdevsourceformatter.h"
#include "kdevcreatefile.h"
#include "qtbuildconfig.h"
#include "kdeveditorutil.h"
#include <ktexteditor/viewcursorinterface.h>
#include <kpopupmenu.h>
// wizards
#include "cppnewclassdlg.h"
#include "subclassingdlg.h"
#include "addmethoddialog.h"
#include "addattributedialog.h"
#include "creategettersetterdialog.h"
// designer integration
#include "qtdesignercppintegration.h"
#include "cppimplementationwidget.h"
#include "configproblemreporter.h"
#include "codeinformationrepository.h"

#include <qeventloop.h>
#include <qheader.h>
#include <qdir.h>
#include <qdom.h>
#include <qfileinfo.h>
#include <qguardedptr.h>
#include <qpopupmenu.h>
#include <qprogressdialog.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qstatusbar.h>
#include <qprogressbar.h>
#include <qregexp.h>
#include <qlabel.h>
#include <qvbox.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kgenericfactory.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmainwindow.h>
#include <kstatusbar.h>
#include <kconfig.h>
#include <kdeversion.h>
#include <kstandarddirs.h>
#include <kiconloader.h>

#include <ktexteditor/document.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/view.h>
#include <ktexteditor/selectioninterface.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/clipboardinterface.h>
#include <ktexteditor/texthintinterface.h>

#include <kdevcore.h>
#include <kdevproject.h>
#include <kdevmainwindow.h>
#include <kdevpartcontroller.h>
#include <kdevmakefrontend.h>
#include <kdevcoderepository.h>
#include <codemodel_utils.h>
#include <kdevplugininfo.h>

#include <domutil.h>
#include <config.h>

const bool alwaysParseInBackground = true;

enum { KDEV_DB_VERSION = 21 };
enum { KDEV_PCS_VERSION = 18 };

QStringList CppSupportPart::m_sourceMimeTypes = QStringList() << "text/x-csrc" << "text/x-c++src";
QStringList CppSupportPart::m_headerMimeTypes = QStringList() << "text/x-chdr" << "text/x-c++hdr";

QStringList CppSupportPart::m_sourceExtensions = QStringList::split( ",", "c,C,cc,cpp,c++,cxx,m,mm,M" );
QStringList CppSupportPart::m_headerExtensions = QStringList::split( ",", "h,H,hh,h++,hxx,hpp,inl,tlh,diff,ui.h" );

class CppDriver: public KDevDriver
{
public:
	CppDriver( CppSupportPart* cppSupport ) : KDevDriver( cppSupport, true )
	{}

	void fileParsed( ParsedFile& fileName )
	{
		//kdDebug(9007) << "-----> file " << fileName << " parsed!" << endl;

		ParsedFilePointer ast = takeTranslationUnit( fileName.fileName() );

		if ( cppSupport() ->problemReporter() )
		{
			cppSupport() ->problemReporter() ->removeAllProblems( fileName.fileName() );

			QValueList<Problem> pl = problems( fileName.fileName() );
			QValueList<Problem>::ConstIterator it = pl.begin();
			while ( it != pl.end() )
			{
				const Problem & p = *it++;
				cppSupport() ->problemReporter() ->reportProblem( fileName.fileName(), p );
			}
		}

		StoreWalker walker( fileName.fileName(), cppSupport() ->codeModel() );

		if ( cppSupport() ->codeModel() ->hasFile( fileName.fileName() ) )
		{
			FileDom file = cppSupport() ->codeModel() ->fileByName( fileName.fileName() );
			cppSupport() ->removeWithReferences( fileName.fileName() );
		}

		walker.parseTranslationUnit( *ast );
		cppSupport() ->codeModel() ->addFile( walker.file() );
		remove
			( fileName.fileName() );

		if( cppSupport()->_jd ) {
			cppSupport()->_jd->backgroundState ++;
			cppSupport()->_jd->lastParse = QTime::currentTime();
		}

		QFileInfo fileInfo( fileName.fileName() );
		QString path = URLUtil::canonicalPath( fileName.fileName() );

		cppSupport()->m_timestamp[ path ] = fileInfo.lastModified();

		cppSupport()->emitSynchronousParseReady( fileName.fileName(), ast );
	}
};

// ProblemReporter doesn't really depend on background parsing, so it's a bit of a mixup to
// handle them together, but it's the same config widget so...
class BackgroundParserConfig
{
	bool m_useProblemReporter;
	bool m_useBackgroundParser;
	int m_backgroundParseDelay;
public:
	void readConfig()
	{
		KConfig* config = kapp->config();
		config->setGroup( "General Options" );
		m_useProblemReporter = config->readBoolEntry( "EnableProblemReporter", true );
		m_useBackgroundParser = config->readBoolEntry( "EnableCppBgParser", true );
		m_backgroundParseDelay = config->readNumEntry( "BgParserDelay", 500 );
	}

	bool useProblemReporter() { return m_useProblemReporter; }
	bool useBackgroundParser() { return m_useBackgroundParser; }
	int backgroudParseDelay() { return m_backgroundParseDelay; }
};


CppSupportPart::CppSupportPart( QObject *parent, const char *name, const QStringList &args )
: KDevLanguageSupport( CppSupportFactory::info(), parent, name ? name : "KDevCppSupport" ), m_backgroundParser(0),
	m_activeDocument( 0 ), m_activeView( 0 ), m_activeSelection( 0 ), m_activeEditor( 0 ), m_activeViewCursor( 0 ),
	m_projectClosed( true ), m_projectClosing( false ), m_valid( false ), m_isTyping( false ), m_hadErrors( false ),
	_jd(0)
{
	setInstance( CppSupportFactory::instance() );

	m_pCompletionConfig = new CppCodeCompletionConfig( this, projectDom() );
	m_pSplitHeaderSourceConfig = new CppSplitHeaderSourceConfig( this, projectDom() );
	m_pCreateGetterSetterConfiguration = new CreateGetterSetterConfiguration( this );	connect( m_pSplitHeaderSourceConfig, SIGNAL( stored() ),
		this, SLOT( splitHeaderSourceConfigStored() ) );
	connect( m_pCompletionConfig, SIGNAL( stored() ),
	         this, SLOT( codeCompletionConfigStored() ) );
	m_qtBuildConfig = new QtBuildConfig( this, projectDom() );
    m_qtBuildConfig->store();

	m_backgroundParserConfig = new BackgroundParserConfig;
	m_backgroundParserConfig->readConfig();

	m_driver = new CppDriver( this );
	m_problemReporter = 0;

	m_textChangedTimer = new QTimer( this );
	connect( m_textChangedTimer, SIGNAL(timeout()), this, SLOT(slotParseCurrentFile()) );

	m_cursorMovedTimer = new QTimer( this );
	connect( m_cursorMovedTimer, SIGNAL(timeout()), this, SLOT(slotCursorPositionChanged()) );


// 	m_deleteParserStoreTimer = new QTimer( this );
	m_saveMemoryTimer = new QTimer( this );
	m_buildSafeFileSetTimer = new QTimer( this );
// 	m_functionHintTimer = new QTimer( this );
	connect( m_buildSafeFileSetTimer, SIGNAL(timeout()), this, SLOT(buildSafeFileSet()) );
	connect( m_saveMemoryTimer, SIGNAL(timeout()), this, SLOT(slotSaveMemory()) );
// 	connect( m_deleteParserStoreTimer, SIGNAL(timeout()), this, SLOT(slotDeleteParserStore()) );
	resetParserStoreTimer();
	m_saveMemoryTimer->start( 240000, false ); //Free some memory every 4 minutes
	//    connect( m_functionHintTimer, SIGNAL(timeout()), this, SLOT(slotFunctionHint()) );

	setXMLFile( "kdevcppsupport.rc" );

	m_catalogList.setAutoDelete( true );

	connect( core(), SIGNAL( projectOpened() ), this, SLOT( projectOpened() ) );
	connect( core(), SIGNAL( projectClosed() ), this, SLOT( projectClosed() ) );
	connect( core(), SIGNAL( languageChanged() ), this, SLOT( projectOpened() ) );
	connect( partController(), SIGNAL( savedFile( const KURL& ) ),
	         this, SLOT( savedFile( const KURL& ) ) );
	connect( core(), SIGNAL( contextMenu( QPopupMenu *, const Context * ) ),
	         this, SLOT( contextMenu( QPopupMenu *, const Context * ) ) );
	connect( partController(), SIGNAL( activePartChanged( KParts::Part* ) ),
	         this, SLOT( activePartChanged( KParts::Part* ) ) );
	connect( partController(), SIGNAL( partRemoved( KParts::Part* ) ),
	         this, SLOT( partRemoved( KParts::Part* ) ) );

	connect( core(), SIGNAL( configWidget( KDialogBase* ) ),
	         this, SLOT( configWidget( KDialogBase* ) ) );

	m_switchHeaderSourceAction = new KAction( i18n( "Switch Header/Implementation" ), SHIFT + Key_F12,
	                      this, SLOT( slotSwitchHeader() ),
	                      actionCollection(), "edit_switchheader" );
	m_switchHeaderSourceAction->setToolTip( i18n( "Switch between header and implementation files" ) );
	m_switchHeaderSourceAction->setWhatsThis( i18n( "<b>Switch Header/Implementation</b><p>"
	                            "If you are currently looking at a header file, this "
	                            "brings you to the corresponding implementation file. "
	                            "If you are looking at an implementation file (.cpp etc.), "
	                            "this brings you to the corresponding header file." ) );
	m_switchHeaderSourceAction->setEnabled( false );

	KAction *action;

	action = new KAction( i18n( "Complete Text" ), CTRL + Key_Space,
	                      this, SLOT( slotCompleteText() ),
	                      actionCollection(), "edit_complete_text" );
	action->setToolTip( i18n( "Complete current expression" ) );
	action->setWhatsThis( i18n( "<b>Complete Text</p><p>Completes current expression using "
	                            "memory class store for the current project and persistent class stores "
	                            "for external libraries." ) );
	action->setEnabled( false );

	m_createGetterSetterAction = new KAction( i18n( "Create Accessor Methods" ), 0,
	                                          this, SLOT( slotCreateAccessMethods() ), actionCollection(),
	                                          "edit_create_getter_setter" );

	action = new KAction( i18n( "Make Member" ), 0, Key_F2,
	                      this, SLOT( slotMakeMember() ),
	                      actionCollection(), "edit_make_member" );
	action->setToolTip( i18n( "Make member" ) );
	action->setWhatsThis( i18n( "<b>Make member</b><p>Creates a class member function in implementation file "
	                            "based on the member declaration at the current line." ) );
	action->plug( &m_DummyActionWidget );

	action = new KAction( i18n( "Navigation Menu" ), 0, CTRL + ALT + Key_Space,
	                      this, SLOT( slotNavigate() ),
	                      actionCollection(), "edit_navigate" );
	action->setToolTip( i18n( "Show the navigation-menu" ) );
	action->setWhatsThis( i18n( "<b>Navigate</b><p>Shows a navigation-menu based on the type-evaluation of the item under the cursor." ) );
	action->plug( &m_DummyActionWidget );


	action = new KAction( i18n( "New Class..." ), "classnew", 0,
	                      this, SLOT( slotNewClass() ),
	                      actionCollection(), "project_newclass" );
	action->setToolTip( i18n( "Generate a new class" ) );
	action->setWhatsThis( i18n( "<b>New Class</b><p>Calls the <b>New Class</b> wizard." ) );

	m_pCompletion = 0;

	withcpp = false;
	if ( args.count() == 1 && args[ 0 ] == "Cpp" )
		withcpp = true;

	// daniel
	connect( core( ), SIGNAL( projectConfigWidget( KDialogBase* ) ), this,
	         SLOT( projectConfigWidget( KDialogBase* ) ) );

	new KDevCppSupportIface( this );
	//(void) dcopClient();

	m_lockupTester = new UIBlockTester( 100 );
}


CppSupportPart::~CppSupportPart()
{
	delete m_lockupTester;

	if ( !m_projectClosed )
		projectClosed();

	delete( m_driver );
	m_driver = 0;

	if ( m_backgroundParser )
	{
		m_backgroundParser->close();
		//	m_backgroundParser->wait();
		delete m_backgroundParser;
		m_backgroundParser = 0;
	}

	codeRepository() ->setMainCatalog( 0 );

	QPtrListIterator<Catalog> it( m_catalogList );
	while ( Catalog * catalog = it.current() )
	{
		++it;
		codeRepository() ->unregisterCatalog( catalog );
	}


	delete m_backgroundParserConfig;
	m_backgroundParserConfig = 0;

	delete m_pCompletion;
	m_pCompletion = 0;

/*	mainWindow()->removeView( m_problemReporter );
	delete m_problemReporter;
	m_problemReporter = 0;
*/
	delete _jd;
	_jd = 0;

	kdDebug( 9007 ) << k_funcinfo << endl;
}


void CppSupportPart::customEvent( QCustomEvent* ev )
{
	kdDebug( 9007 ) << "CppSupportPart::customEvent(" << ev->type() << ")" << endl;

	QTime t;
	t.start();
	bool fromDisk = false;

	if ( ev->type() == int( Event_FileParsed ) )
	{
		resetParserStoreTimer();

		FileParsedEvent * event = ( FileParsedEvent* ) ev;
		fromDisk = event->fromDisk();
		QString fileName = event->fileName();
		bool hasErrors = false;
		if ( m_problemReporter )
		{
			m_problemReporter->removeAllProblems( fileName );

			QValueList<Problem> problems = event->problems();
			QValueList<Problem>::ConstIterator it = problems.begin();
			while ( it != problems.end() )
			{
				const Problem & p = *it++;
				if ( p.level() == Problem::Level_Error )
					hasErrors = true;

				m_problemReporter->reportProblem( fileName, p );
			}
		}
		ParsedFilePointer p = m_backgroundParser->translationUnit( fileName );
		if( p && !p->includedFrom().isEmpty() ) {
			kdDebug( 9007 ) << "customEvent() parsed included file \"" << fileName << "\" included from \"" << p->includedFrom() << "\"" << endl;
		} else {
			kdDebug( 9007 ) << "customEvent() parsed file \"" << fileName << "\"" <<  endl;
		}

		if( p && !p->includedFrom().isEmpty() ) {
			if( !project()->isProjectFile( fileName ) ) {
				//The file was parsed to resolve a dependency, and is not a project file
				addToRepository( p );
			} else {
				//It is a project-file that was parsed for whatever reason to resolve a dependency(currently it isn't handled this way)
			}
		} else if( !project()->isProjectFile( fileName ) || !m_parseEmitWaiting.reject( fileName ) ) {
            ParseEmitWaiting::Processed p = m_parseEmitWaiting.processFile( fileName, ( !m_hadErrors && hasErrors && !fromDisk && m_isTyping && fileName == m_activeFileName ) ? ParseEmitWaiting::HadErrors : ParseEmitWaiting::None );
			parseEmit( p );

	        //Increase status-bar
			if( p.hasFlag( ParseEmitWaiting::Silent ) && _jd ) {
				_jd->backgroundState ++;
				_jd->lastParse = QTime::currentTime();
			}

        } else {
	        ParseEmitWaiting::Processed p = m_fileParsedEmitWaiting.processFile( fileName );
	        if( !p.hasFlag( ParseEmitWaiting::Silent ) )
		        emitFileParsed( p );

	        //Increase status-bar
			if( p.hasFlag( ParseEmitWaiting::Silent ) && _jd ) {
				_jd->backgroundState ++;
				_jd->lastParse = QTime::currentTime();
			}
        }
	}
}


void CppSupportPart::projectConfigWidget( KDialogBase* dlg )
{
	QVBox * vbox = 0;

	vbox = dlg->addVBoxPage( i18n( "C++ Support" ), i18n( "C++ Support" ),
	                         BarIcon( info() ->icon(), KIcon::SizeMedium ) );
	CCConfigWidget* w = new CCConfigWidget( this, vbox );
	connect( dlg, SIGNAL( okClicked( ) ), w, SLOT( accept( ) ) );
}

void CppSupportPart::configWidget( KDialogBase *dlg )
{
	QVBox * vbox = dlg->addVBoxPage( i18n( "C++ Class Generator" ), i18n( "C++ Class Generator" ),
	                                 BarIcon( info() ->icon(), KIcon::SizeMedium ) );
	ClassGeneratorConfig *w = new ClassGeneratorConfig( vbox, "classgenerator config widget" );
	connect( dlg, SIGNAL( okClicked() ), w, SLOT( storeConfig() ) );

	vbox = dlg->addVBoxPage(i18n("C++ Parsing"), i18n("C++ Parsing"),
	                               BarIcon( "source_cpp", KIcon::SizeMedium) );
	ConfigureProblemReporter* ww = new ConfigureProblemReporter( vbox );
	ww->setPart( this );
	connect(dlg, SIGNAL(okClicked()), ww, SLOT(accept()));
}

void CppSupportPart::activePartChanged( KParts::Part *part )
{
	kdDebug( 9032 ) << "CppSupportPart::activePartChanged()" << endl;

	bool enabled = false;

// 	m_functionHintTimer->stop();

	if ( m_activeView )
	{
		disconnect( m_activeView, SIGNAL( cursorPositionChanged() ), this, 0 );
	}
	if ( m_activeDocument )
	{
		disconnect( m_activeDocument, SIGNAL(textChanged()), this, 0 );
	}

	m_isTyping = false;
	m_hadErrors = true;
	m_activeDocument = dynamic_cast<KTextEditor::Document*>( part );
	m_activeView = part ? dynamic_cast<KTextEditor::View*>( part->widget() ) : 0;
	m_activeEditor = dynamic_cast<KTextEditor::EditInterface*>( part );
	m_activeSelection = dynamic_cast<KTextEditor::SelectionInterface*>( part );
	m_activeViewCursor = dynamic_cast<KTextEditor::ViewCursorInterface*>( m_activeView );

	m_activeFileName = QString::null;

	if ( m_activeDocument )
	{
		m_activeFileName = URLUtil::canonicalPath( m_activeDocument->url().path() );
		QFileInfo fi( m_activeFileName );
		QString ext = fi.extension();
		if ( isSource( m_activeFileName ) || isHeader( m_activeFileName ) )
			enabled = true;
	}

	actionCollection() ->action( "edit_switchheader" ) ->setEnabled( enabled );
	actionCollection() ->action( "edit_complete_text" ) ->setEnabled( enabled );
	actionCollection() ->action( "edit_make_member" ) ->setEnabled( enabled );

	if ( !part || !part->widget() )
		return ;

	if ( m_activeDocument )
	{
		connect( m_activeDocument, SIGNAL(textChanged()), this, SLOT(slotTextChanged()) );
		m_textChangedTimer->start( 250, true ); // kick the parse timer, we might want to parse the current file
	}

	if ( m_activeViewCursor )
	{
		connect( m_activeView, SIGNAL( cursorPositionChanged() ), this, SLOT(slotCursorMoved()) );
// 		         this, SLOT( slotCursorPositionChanged() ) );
	}


#if 0
	KTextEditor::TextHintInterface* textHintIface = dynamic_cast<KTextEditor::TextHintInterface*>( m_activeView );
	if ( !textHintIface )
		return ;

	connect( view, SIGNAL( needTextHint( int, int, QString& ) ),
	         this, SLOT( slotNeedTextHint( int, int, QString& ) ) );

	textHintIface->enableTextHints( 1000 );
#endif
}


void CppSupportPart::setTyping( bool typing ) {
	m_isTyping = typing;
	if( m_problemReporter) {
		m_hadErrors &= m_problemReporter->hasErrors(m_activeFileName);///m_hadErrors generally stores whether there was an error-free state of the file.
	}
}


void CppSupportPart::projectOpened( )
{
	kdDebug( 9007 ) << "projectOpened( )" << endl;

	m_backgroundParser = new BackgroundParser( this, &m_eventConsumed );
	m_backgroundParser->start( QThread::IdlePriority );

	// setup the driver
	QString conf_file_name = specialHeaderName();
	if ( QFile::exists( conf_file_name ) )
		m_driver->parseFile( conf_file_name, true, true, true );

	m_projectDirectory = URLUtil::canonicalPath( project() ->projectDirectory() );
	m_projectFileList = project() ->allFiles();

	setupCatalog();

	embedProblemReporter();

	connect( core(), SIGNAL( configWidget( KDialogBase* ) ),
	         m_problemReporter, SLOT( configWidget( KDialogBase* ) ) );

	connect( project( ), SIGNAL( addedFilesToProject( const QStringList & ) ),
	         this, SLOT( addedFilesToProject( const QStringList & ) ) );
	connect( project( ), SIGNAL( removedFilesFromProject( const QStringList & ) ),
	         this, SLOT( removedFilesFromProject( const QStringList & ) ) );
	connect( project( ), SIGNAL( changedFilesInProject( const QStringList & ) ),
	         this, SLOT( changedFilesInProject( const QStringList & ) ) );
	connect( project(), SIGNAL( projectCompiled() ),
	         this, SLOT( slotProjectCompiled() ) );

	m_timestamp.clear();
	m_parseEmitWaiting.clear();
	m_fileParsedEmitWaiting.clear();

	m_pCompletion = new CppCodeCompletion( this );
	m_projectClosed = false;

	m_buildSafeFileSetTimer->start( 500, true );
	updateParserConfiguration(); //Necessary to respect custom include-paths and such

	QTimer::singleShot( 500, this, SLOT( initialParse( ) ) );
}

void CppSupportPart::embedProblemReporter( bool force )
{
	if ( force || m_backgroundParserConfig->useProblemReporter() )
	{
		m_problemReporter = new ProblemReporter( this, 0, "problemReporterWidget" );
		m_problemReporter->setIcon( SmallIcon( "info" ) );
		m_problemReporter->setCaption( i18n( "Problem Reporter" ) );
		mainWindow( ) ->embedOutputView( m_problemReporter, i18n( "Problems" ), i18n( "Problem reporter" ) );
	}
}

void CppSupportPart::removeProblemReporter()
{
	mainWindow()->removeView( m_problemReporter );
	delete m_problemReporter;
	m_problemReporter = 0;
}


void CppSupportPart::projectClosed( )
{
	kdDebug( 9007 ) << "projectClosed( )" << endl;

	m_projectClosing = true;

	QStringList enabledPCSs;
	QValueList<Catalog*> catalogs = codeRepository() ->registeredCatalogs();
	for ( QValueList<Catalog*>::Iterator it = catalogs.begin(); it != catalogs.end(); ++it )
	{
		Catalog* c = *it;
		if ( c->enabled() )
			enabledPCSs.push_back( QFileInfo( c->dbName() ).baseName(true) );
	}
	DomUtil::writeListEntry( *project() ->projectDom(), "kdevcppsupport/references", "pcs", enabledPCSs );

	for ( QMap<KInterfaceDesigner::DesignerType, KDevDesignerIntegration*>::const_iterator it = m_designers.begin();
	      it != m_designers.end(); ++it )
	{
		kdDebug( 9007 ) << "calling save settings fro designer integration" << endl;
		it.data() ->saveSettings( *project() ->projectDom(), "kdevcppsupport/designerintegration" );
	}

	saveProjectSourceInfo();

	m_pCompletionConfig->store();

	delete _jd;
	_jd = 0;

	removeProblemReporter();

	delete m_pCompletion;
	m_parseEmitWaiting.clear();
	m_fileParsedEmitWaiting.clear();
	m_pCompletion = 0;
	m_projectClosed = true;
	m_projectClosing = false;
}


void CppSupportPart::slotNavigate() {
	if( codeCompletion() && m_activeView && m_activeViewCursor ) {
		unsigned int curLine = 0, curCol = 0;
		m_activeViewCursor->cursorPositionReal( &curLine, &curCol );

		if( m_navigationMenu ) delete (KPopupMenu*)m_navigationMenu;

		m_navigationMenu = new KPopupMenu( m_activeView );

		codeCompletion()->contextEvaluationMenus( m_navigationMenu, 0, curLine, curCol );

		m_navigationMenu->move( m_activeView->mapToGlobal( m_activeViewCursor->cursorCoordinates() ) );
		if ( m_navigationMenu->count() > 0 )
		{
			m_navigationMenu->show();
		}
	}
}


void CppSupportPart::contextMenu( QPopupMenu *popup, const Context *context )
{
	m_activeClass = 0;
	m_activeFunction = 0;
	m_activeVariable = 0;
	m_curAttribute = 0;
	m_curClass = 0;

	if ( context->hasType( Context::EditorContext ) )
	{
		int id;

		m_switchHeaderSourceAction->plug( popup );

		// 	CodeModelItemContext
		if ( context->type() == Context::EditorContext )
		{
			m_curClass = currentClass();
			if ( m_curClass != 0 )
			{
				m_curAttribute = currentAttribute( m_curClass );
				if ( m_curAttribute != 0 )
					m_createGetterSetterAction->plug( popup );
			}
		}

		QString text;
		int atline, atcol;
		MakeMemberHelper( text, atline, atcol );
		if ( !text.isEmpty() )
		{
			id = popup->insertItem( i18n( "Make Member" ), this, SLOT( slotMakeMember() ) );
			popup->setWhatsThis( id, i18n( "<b>Make member</b><p>Creates a class member function in implementation file "
			                               "based on the member declaration at the current line." ) );
		}

		kdDebug( 9007 ) << "======> code model has the file: " << m_activeFileName << " = " << codeModel() ->hasFile( m_activeFileName ) << endl;

		bool showContextMenuExplosion = false;
		bool showContextTypeEvaluation = false;
		KConfig *config = CppSupportFactory::instance() ->config();
		if ( config )
		{
			config->setGroup( "General" );
			showContextMenuExplosion = config->readBoolEntry( "ShowContextMenuExplosion", false );
			config->setGroup( "General" );
			showContextTypeEvaluation = config->readBoolEntry( "ShowContextTypeEvaluation", true );
		}


		if( codeModel() ->hasFile( m_activeFileName ) ) {

		if( showContextTypeEvaluation && m_activeViewCursor != 0 ) {
			if( codeCompletion() ) {
				unsigned int curLine = 0, curCol = 0;
				m_activeViewCursor->cursorPositionReal( &curLine, &curCol );

				codeCompletion()->contextEvaluationMenus( popup, context, curLine, curCol );
			}
		}


		if ( showContextMenuExplosion )
		{
			//kdDebug( 9007 ) << "CppSupportPart::contextMenu 1" << endl;
			QString candidate;
			if ( isSource( m_activeFileName ) )
				candidate = sourceOrHeaderCandidate();
			else
				candidate = m_activeFileName;

			unsigned int curLine = 0, curCol = 0;
			if ( m_activeViewCursor != 0 )
				m_activeViewCursor->cursorPositionReal( &curLine, &curCol );

			//kdDebug( 9007 ) << "CppSupportPart::contextMenu 2: candidate: " << candidate << endl;

			if ( !candidate.isEmpty() && codeModel() ->hasFile( candidate ) )
			{
				QPopupMenu * m2 = new QPopupMenu( popup );
				id = popup->insertItem( i18n( "Go to Declaration" ), m2 );
				popup->setWhatsThis( id, i18n( "<b>Go to declaration</b><p>Provides a menu to select available function declarations "
				                               "in the current file and in the corresponding header (if the current file is an implementation) or source (if the current file is a header) file." ) );

				FileDom file2 = codeModel() ->fileByName( candidate );
				//kdDebug( 9007 ) << "CppSupportPart::contextMenu 3: " << file2->name() << endl;

				FunctionList functionList2 = CodeModelUtils::allFunctions( file2 );
				for ( FunctionList::ConstIterator it = functionList2.begin(); it != functionList2.end(); ++it )
				{
					QString text = ( *it ) ->scope().join( "::" );
					//kdDebug( 9007 ) << "CppSupportPart::contextMenu 3 text: " << text << endl;
					if ( !text.isEmpty() )
					{
						text += "::";
					}
					text += formatModelItem( *it, true );
					text = text.replace( QString::fromLatin1( "&" ), QString::fromLatin1( "&&" ) );
					int id = m2->insertItem( text, this, SLOT( gotoDeclarationLine( int ) ) );
					int line, column;
					( *it ) ->getStartPosition( &line, &column );
					m2->setItemParameter( id, line );
				}

				if ( m2->count() == 0 )
				{
					popup->removeItem( id );
				}
				//kdDebug( 9007 ) << "CppSupportPart::contextMenu 4" << endl;
			}

			QString candidate1;
			if ( isHeader( m_activeFileName ) )
			{
				candidate1 = sourceOrHeaderCandidate();
			}
			else
			{
				candidate1 = m_activeFileName;
			}
			//kdDebug( 9007 ) << "CppSupportPart::go to definition in " << candidate1 << endl;
			if ( codeModel() ->hasFile( candidate1 ) )
			{
				QPopupMenu * m = new QPopupMenu( popup );
				id = popup->insertItem( i18n( "Go to Definition" ), m );
				popup->setWhatsThis( id, i18n( "<b>Go to definition</b><p>Provides a menu to select available function definitions "
				                               "in the current file and in the corresponding header (if the current file is an implementation) or source (if the current file is a header) file." ) );

				const FileDom file = codeModel() ->fileByName( candidate1 );
				const FunctionDefinitionList functionDefinitionList = CodeModelUtils::allFunctionDefinitionsDetailed( file ).functionList;
				for ( FunctionDefinitionList::ConstIterator it = functionDefinitionList.begin(); it != functionDefinitionList.end(); ++it )
				{
					QString text = ( *it ) ->scope().join( "::" );
					if ( !text.isEmpty() )
					{
						text += "::";
					}
					text += formatModelItem( *it, true );
					text = text.replace( QString::fromLatin1( "&" ), QString::fromLatin1( "&&" ) );
					int id = m->insertItem( text, this, SLOT( gotoLine( int ) ) );
					int line, column;
					( *it ) ->getStartPosition( &line, &column );
					m->setItemParameter( id, line );
				}
				if ( m->count() == 0 )
				{
					popup->removeItem( id );
				}

			}
		}
		}

		const EditorContext *econtext = static_cast<const EditorContext*>( context );
		QString str = econtext->currentLine();
		if ( str.isEmpty() )
			return ;
	}
	else if ( context->hasType( Context::CodeModelItemContext ) )
	{
		const CodeModelItemContext * mcontext = static_cast<const CodeModelItemContext*>( context );

		if ( mcontext->item() ->isClass() )
		{
			m_activeClass = ( ClassModel* ) mcontext->item();
			int id = popup->insertItem( i18n( "Extract Interface..." ), this, SLOT( slotExtractInterface() ) );
			popup->setWhatsThis( id, i18n( "<b>Extract interface</b><p>Extracts interface from the selected class and creates a new class with this interface. "
			                               "No implementation code is extracted and no implementation code is created." ) );
		}
		else if ( mcontext->item() ->isFunction() )
		{
			m_activeFunction = ( FunctionModel* ) mcontext->item();
		}
	}
	else if ( context->hasType( Context::FileContext ) )
	{
		const FileContext * fc = static_cast<const FileContext*>( context );
		//this is a .ui file and only selection contains only one such file
		KURL url = fc->urls().first();
		kdDebug( 9007 ) << "file context with " << url.path() << endl;
		if ( url.fileName().endsWith( ".ui" ) )
		{
			m_contextFileName = url.path();
			int id = popup->insertItem( i18n( "Create or Select Implementation..." ), this, SLOT( slotCreateSubclass() ) );
			popup->setWhatsThis( id, i18n( "<b>Create or select implementation</b><p>Creates or selects a subclass of selected form for use with integrated KDevDesigner." ) );
		}
	}
}


QStringList makeListUnique( const QStringList& rhs ) {
	QMap<QString, bool> map;
	QStringList ret;
	for( QStringList::const_iterator it = rhs.begin(); it != rhs.end(); ++it ) {
		if( map.find( *it ) == map.end() ) {
			ret << *it;
			map.insert( *it, true );
		}
	}
	return ret;
}

// Makes sure that header files come first
QStringList CppSupportPart::reorder( const QStringList &list )
{
	QStringList headers, others;

	QStringList headerExtensions = QStringList::split( ",", "h,H,hh,hxx,hpp,tlh" );

	QString projectPath = project()->projectDirectory();

	QStringList::ConstIterator it;
	for ( it = list.begin(); it != list.end(); ++it )
	{
		QString filePath = *it;
		// brilliant stuff.. this method is apparently called both with
		// relative and absolute paths..
		if ( !filePath.startsWith("/") )
		{
			filePath = projectPath + "/" + filePath;
		}
		if( !isValidSource( filePath ) ) continue;
		if ( headerExtensions.contains( QFileInfo( filePath ).extension() ) )
			headers << ( filePath );
		else
			others << ( filePath );
	}

	return makeListUnique( headers + others );
}

void CppSupportPart::addedFilesToProject( const QStringList &fileList )
{
	m_projectFileList = project() ->allFiles();
	QStringList files = reorder( fileList );

	for ( QStringList::ConstIterator it = files.begin(); it != files.end(); ++it )
	{
		QString path = *it;
		if (!path.startsWith("/"))
			path = URLUtil::canonicalPath( m_projectDirectory + "/" + ( *it ) );

		maybeParse( path );
		//emit addedSourceInfo( path );
	}
	m_buildSafeFileSetTimer->start( 500, true );
}

void CppSupportPart::removedFilesFromProject( const QStringList &fileList )
{
	m_projectFileList = project() ->allFiles();
	for ( QStringList::ConstIterator it = fileList.begin(); it != fileList.end(); ++it )
	{
		QString path = URLUtil::canonicalPath( m_projectDirectory + "/" + *it );
		kdDebug( 9007 ) << "=====================> remove file: " << path << endl;

		removeWithReferences( path );
		m_backgroundParser->removeFile( path );
	}
	m_buildSafeFileSetTimer->start( 500, true );
}

void CppSupportPart::changedFilesInProject( const QStringList & fileList )
{
	QStringList files = reorder( fileList );

	for ( QStringList::ConstIterator it = files.begin(); it != files.end(); ++it )
	{
		QString path = URLUtil::canonicalPath( m_projectDirectory + "/" + *it );

		maybeParse( path );
		//emit addedSourceInfo( path );
	}
}

void CppSupportPart::savedFile( const KURL &file )
{
	if( file.path() == m_activeFileName ) {
		m_isTyping = false;
		m_hadErrors = false;
		maybeParse( file.path() );
	}

	Q_UNUSED( file.path() );

#if 0  // not needed anymore

	kdDebug( 9007 ) << "savedFile(): " << fileName.mid ( m_projectDirectory.length() + 1 ) << endl;

	if ( m_projectFileList.contains( fileName.mid ( m_projectDirectory.length() + 1 ) ) )
	{
		maybeParse( fileName );
		emit addedSourceInfo( fileName );
	}
#endif
}

QString CppSupportPart::findSourceFile()
{
	// get the path of the currently active document
	QFileInfo fi( m_activeFileName );
	QString path = fi.filePath();
	QString ext = fi.extension();
	// extract the base path (full path without '.' and extension)
	QString base = path.left( path.length() - ext.length() - 1 );
	QStringList candidates;
	if ( QStringList::split( ',', "h,H,hh,hxx,hpp,tlh" ).contains( ext ) )
	{
		candidates << ( base + ".c" );
		candidates << ( base + ".cc" );
		candidates << ( base + ".cpp" );
		candidates << ( base + ".c++" );
		candidates << ( base + ".cxx" );
		candidates << ( base + ".C" );
		candidates << ( base + ".m" );
		candidates << ( base + ".mm" );
		candidates << ( base + ".M" );
		candidates << ( base + ".inl" );
		candidates << ( base + "_impl.h" );
	}

	QStringList::ConstIterator it;
	for ( it = candidates.begin(); it != candidates.end(); ++it )
	{
		kdDebug( 9007 ) << "Trying " << ( *it ) << endl;
		if ( QFileInfo( *it ).exists() )
		{
			return * it;
		}
	}
	return m_activeFileName;
}

QString CppSupportPart::sourceOrHeaderCandidate( const KURL &url )
{
	QString urlPath;
	if ( url.isEmpty() )
	{
		KTextEditor::Document * doc =
			dynamic_cast<KTextEditor::Document*>( partController() ->activePart() );
		if ( !doc )
			return QString::null;
		urlPath = doc->url().path();
	}
	else
	{
		urlPath = url.path();
	}
	// get the path of the currently active document
	QFileInfo fi( urlPath );
	QString path = fi.filePath();
	// extract the exension
	QString ext = fi.extension();
	if ( ext.isEmpty() )
		return QString::null;
	// extract the base path (full path without '.' and extension)
	QString base = path.left( path.length() - ext.length() - 1 );
	//kdDebug( 9007 ) << "base: " << base << ", ext: " << ext << endl;
	// just the filename without the extension
	QString fileNameWoExt = fi.fileName();
	if ( !ext.isEmpty() )
		fileNameWoExt.replace( "." + ext, "" );
	QString possibleExts;
	// depending on the current extension assemble a list of
	// candidate files to look for
	QStringList candidates;
	// special case for template classes created by the new class dialog
	if ( path.endsWith( "_impl.h" ) )
	{
		QString headerpath = path;
		headerpath.replace( "_impl.h", ".h" );
		candidates << headerpath;
		fileNameWoExt.replace( "_impl", "" );
		possibleExts = "h";
	}
	// if file is a header file search for implementation file
	else if ( QStringList::split( ',', "h,H,hh,hxx,hpp,tlh" ).contains( ext ) )
	{
		candidates << ( base + ".c" );
		candidates << ( base + ".cc" );
		candidates << ( base + ".cpp" );
		candidates << ( base + ".c++" );
		candidates << ( base + ".cxx" );
		candidates << ( base + ".C" );
		candidates << ( base + ".m" );
		candidates << ( base + ".mm" );
		candidates << ( base + ".M" );
		candidates << ( base + ".inl" );
		candidates << ( base + "_impl.h" );
		possibleExts = "c,cc,cpp,c++,cxx,C,m,mm,M,inl,_impl.h";
	}
	// if file is an implementation file, search for header file
	else if ( QStringList::split( ',', "c,cc,cpp,c++,cxx,C,m,mm,M,inl" ).contains( ext ) )
	{
		candidates << ( base + ".h" );
		candidates << ( base + ".H" );
		candidates << ( base + ".hh" );
		candidates << ( base + ".hxx" );
		candidates << ( base + ".hpp" );
		candidates << ( base + ".tlh" );
		possibleExts = "h,H,hh,hxx,hpp,tlh";
	}
        // search for files from the assembled candidate lists, return the first
        // candidate file that actually exists or QString::null if nothing is found.
        QStringList::ConstIterator it;
        for ( it = candidates.begin(); it != candidates.end(); ++it )
        {
	        //kdDebug( 9007 ) << "Trying " << ( *it ) << endl;
                if ( QFileInfo( *it ).exists() )
                {
			kdDebug( 9007 ) << "using: " << *it << endl;
                        return * it;
                }
        }
	//kdDebug( 9007 ) << "Now searching in project files." << endl;
	// Our last resort: search the project file list for matching files
	QStringList::iterator fileIt;
	QFileInfo candidateFileWoExt;
	QString candidateFileWoExtString;
	QStringList possibleExtsList = QStringList::split( ',', possibleExts );
	for ( fileIt = m_projectFileList.begin(); fileIt != m_projectFileList.end(); ++fileIt )
	{
		candidateFileWoExt.setFile(*fileIt);
		//kdDebug( 9007 ) << "candidate file: " << *fileIt << endl;
		if( !candidateFileWoExt.extension().isEmpty() )
			candidateFileWoExtString = candidateFileWoExt.fileName().replace( "." + candidateFileWoExt.extension(), "" );
		if ( candidateFileWoExtString == fileNameWoExt )
		{
			if ( possibleExtsList.contains( candidateFileWoExt.extension() ) || candidateFileWoExt.extension().isEmpty() )
			{
				//kdDebug( 9007 ) << "checking if " << *fileIt << " exists" << endl;
				if ( QFileInfo( *fileIt ).exists() )
					kdDebug( 9007 ) << "using: " << *fileIt << endl;
					return *fileIt;
			}
		}
	}
	return QString::null;
}

void CppSupportPart::slotSaveMemory() {
	if( m_backgroundParser ) {
		///This is done so the caches are completely empty after kdevelop was idle for some time(else it would be waste of memory). The background-parsers internal lexer-cache-manager just cares about keeping the count of cached files under a specific count, but doesn't decrease that count when kdevelop is idle.
		m_backgroundParser->lock();
		m_backgroundParser->saveMemory();
		m_backgroundParser->unlock();
	}
}

void CppSupportPart::slotSwitchHeader( bool scrollOnly )
{
	bool attemptMatch = true;
	KConfig *config = CppSupportFactory::instance() ->config();
	if ( config )
	{
		config->setGroup( "General" );
		attemptMatch = config->readBoolEntry( "SwitchShouldMatch", true );
	}

	// ok, both files exist. Do the codemodel have them?
	if ( codeModel() ->hasFile( m_activeFileName ) && m_activeViewCursor && attemptMatch )
	{
		unsigned int currentline, column;
		m_activeViewCursor->cursorPositionReal( &currentline, &column );

		if ( switchHeaderImpl( m_activeFileName, currentline, column, scrollOnly ) )
			return;
	}

	// last chance
	KURL url;
	url.setPath( sourceOrHeaderCandidate() );

	if ( scrollOnly )
		return;
	else if ( !splitHeaderSourceConfig()->splitEnabled() )
		partController() ->editDocument( url );
	else
		partController() ->splitCurrentDocument( url );
}

bool CppSupportPart::switchHeaderImpl( const QString& file, int line, int col, bool scrollOnly )
{
	bool handled = false;

	FunctionDom d;
	FileDom fd = codeModel() ->fileByName( file );
	if ( fd ) {
		CodeModelUtils::CodeModelHelper h( codeModel(), fd );
		d = h.functionAt( line, col );
	}
	if ( d ) {
		if( d->isFunctionDefinition() ) {
			FunctionDom decl = findFunction( d );
			if ( decl ) {
				if ( (void*)&decl != (void*)d.data() && ( !scrollOnly || decl->fileName() != file ) ) {
					jumpToCodeModelItem( model_cast<ItemDom>(decl), scrollOnly );
					handled = true;
				}
			}
		} else {
			FunctionDom def = findFunctionDefinition( d );
			if ( def ) {
				if ( def != d && ( !scrollOnly || def->fileName() != file ) ) {
					jumpToCodeModelItem( model_cast<ItemDom>(def), scrollOnly );
					handled = true;
				}
			}
		}
	}

	return handled;
}

FunctionDom CppSupportPart::findFunction( const FunctionDom& def )
{
	// We have a definition so we're looking for a declaration. The declaration will either be the child of a namespace node (non class members)
	// or the child of a class node (class member).  Search recursively until we find a declaration that matches.
	FunctionDom bestMatch;
	FunctionDom decl = findFunctionInNamespace( codeModel()->globalNamespace(), def, codeModel()->globalNamespace()->namespaceImports(),
	                                            sourceOrHeaderCandidate( def->fileName() ), 0, bestMatch );
	return decl ? decl : bestMatch;
}

FunctionDom CppSupportPart::findFunctionInNamespace( const NamespaceDom& ns, const FunctionDom& def, const std::set<NamespaceImportModel>& nsImports,
                                                     const QString& candidateFile, int scopeIndex, FunctionDom& bestMatch )
{
	FunctionDom d;
	QStringList scope = def->scope();
	if ( !(scopeIndex >= (signed) scope.size()) ) {
		NamespaceDom ns_next = ns->namespaceByName( scope[ scopeIndex ] );
		if ( ns_next ) {
			d = findFunctionInNamespace( ns_next, def, ns_next->namespaceImports(), candidateFile, scopeIndex+1, bestMatch );
		}
		if ( !d ) {
			for ( std::set<NamespaceImportModel>::const_iterator it_ns = nsImports.begin(); it_ns != nsImports.end(); ++it_ns ) {
				if ( (*it_ns).fileName().str() == def->fileName() ) {
					ns_next = ns->namespaceByName( (*it_ns).name() );
					if ( ns_next ) {
						if ( d = findFunctionInNamespace( ns_next, def, nsImports, candidateFile, scopeIndex, bestMatch ) ) break;
					}
				}
			}
		}
		if ( !d ) {
			ClassList classList = ns->classByName( scope[ scopeIndex ] );
			for ( ClassList::ConstIterator it_cs = classList.begin(); it_cs != classList.end(); ) {
				if ( d = findFunctionInClass( *(it_cs++), def, nsImports, candidateFile, scopeIndex+1, bestMatch ) ) break;
			}
		}
	}
	if ( !d ) {
		FunctionList functionList = ns->functionByName( def->name() );
		for ( FunctionList::ConstIterator it_decl = functionList.begin(); it_decl != functionList.end(); ++it_decl ) {
			if ( CodeModelUtils::compareDeclarationToDefinition( *it_decl, (FunctionDefinitionModel*) def.data(), nsImports ) ) {
				ParsedFile* p = dynamic_cast<ParsedFile*>( def->file()->parseResult().data() );
				if ( p ) {
					if ( p->includeFiles()[ (*it_decl)->fileName() ] ) {
						d = *it_decl;
						break;
					} else if ( (*it_decl)->fileName() == candidateFile ) {
						d = *it_decl;
						break;
					}
				}
				if ( !bestMatch ) {
					bestMatch = *it_decl;
				}
			}
		}
	}
	return d;
}

FunctionDom CppSupportPart::findFunctionInClass( const ClassDom& cs, const FunctionDom& def, const std::set<NamespaceImportModel>& nsImports,
                                                 const QString& candidateFile, int scopeIndex, FunctionDom& bestMatch )
{
	FunctionDom d;
	QStringList scope = def->scope();
	if ( !(scopeIndex >= (signed) scope.size()) ) {
		ClassList classList = cs->classByName( scope[ scopeIndex ] );
		for ( ClassList::ConstIterator it_cs = classList.begin(); it_cs != classList.end(); ) {
			if ( d = findFunctionInClass( *(it_cs++), def, nsImports, candidateFile, scopeIndex+1, bestMatch ) ) break;
		}
	}
	if ( !d ) {
		FunctionList functionList = cs->functionByName( def->name() );
		for ( FunctionList::ConstIterator it_decl = functionList.begin(); it_decl != functionList.end(); ++it_decl ) {
			if ( CodeModelUtils::compareDeclarationToDefinition( *it_decl, (FunctionDefinitionModel*) def.data(), nsImports ) ) {
				ParsedFile* p = dynamic_cast<ParsedFile*>( def->file()->parseResult().data() );
				if ( p ) {
					if ( p->includeFiles()[ (*it_decl)->fileName() ] ) {
						d = *it_decl;
						break;
					} else if ( (*it_decl)->fileName() == candidateFile ) {
						d = *it_decl;
						break;
					}
				}
				if ( !bestMatch ) {
					bestMatch = *it_decl;
				}
			}
		}
	}
	return d;
}

FunctionDom CppSupportPart::findFunctionDefinition( const FunctionDom& decl )
{
	// We have a declaration so we're looking for a definition. The definition will be the child of some namespace node (never a class node).
	// Since the definition can be the child of any namespace in its scope depending on syntax, we have to check every one.
	FunctionDom def, bestMatch;
	NamespaceDom ns = codeModel()->globalNamespace();
	QString candidateFile = sourceOrHeaderCandidate( decl->fileName() );
	FunctionDefinitionList functionList = ns->functionDefinitionByName( decl->name() );
	for ( FunctionDefinitionList::ConstIterator it_def = functionList.begin(); it_def != functionList.end() && !def; ++it_def ) {
		if ( CodeModelUtils::compareDeclarationToDefinition( decl, *it_def, ns->namespaceImports() ) ) {
			ParsedFile* p = dynamic_cast<ParsedFile*>( (*it_def)->file()->parseResult().data() );
			if ( p ) {
				if ( p->includeFiles()[ decl->fileName() ] ) {
					def = *it_def;
				} else if ( (*it_def)->fileName() == candidateFile ) {
					def = *it_def;
					break;
				}
			}
			if ( !bestMatch ) {
				bestMatch = *it_def;
			}
		}
	}
	QStringList scope = decl->scope();
	for ( QStringList::ConstIterator it_scope = scope.begin(); it_scope != scope.end() && !def; ++it_scope ) {
		NamespaceDom ns_next = ns->namespaceByName( *it_scope );
		if ( ns_next ) {
			ns = ns_next;
			FunctionDefinitionList functionList = ns->functionDefinitionByName( decl->name() );
			for ( FunctionDefinitionList::ConstIterator it_def = functionList.begin(); it_def != functionList.end() && !def; ++it_def ) {
				if ( CodeModelUtils::compareDeclarationToDefinition( decl, *it_def, ns->namespaceImports() ) ) {
					ParsedFile* p = dynamic_cast<ParsedFile*>( (*it_def)->file()->parseResult().data() );
					if ( p ) {
						if ( p->includeFiles()[ decl->fileName() ] ) {
							def = *it_def;
						} else if ( (*it_def)->fileName() == candidateFile ) {
							def = *it_def;
							break;
						}
					}
					if ( !bestMatch ) {
						bestMatch = *it_def;
					}
				}
			}
		}
	}
	return def ? def : bestMatch;
}

void CppSupportPart::jumpToCodeModelItem( const ItemDom& item, bool scrollOnly )
{
	static KURL lastSyncedUrl;
	static int lastSyncedLine = -1;

	int line, col;
	item->getStartPosition( &line, &col );

	KURL url( item->fileName() );

	if ( scrollOnly ) {
		KParts::ReadOnlyPart* part = partController()->partForURL( url );
		int currentLine = lastSyncedLine;
		if ( part ) {
			KTextEditor::ViewCursorInterface *iface = dynamic_cast<KTextEditor::ViewCursorInterface*>(part->widget());
			if( iface )
				iface->cursorPosition( (uint*) &currentLine, (uint*) &col );
		}
		partController() ->scrollToLineColumn( url, line, -1, lastSyncedLine != currentLine || lastSyncedUrl != url );
	} else if ( !splitHeaderSourceConfig()->splitEnabled() )
		partController() ->editDocument( url, line );
	else
		partController() ->splitCurrentDocument( url, line );
	lastSyncedLine = line;
	lastSyncedUrl = url;
}

KDevLanguageSupport::Features CppSupportPart::features()
{
	if ( withcpp )
		return Features( Classes | Structs | Functions | Variables | Namespaces | Declarations
		                 | Signals | Slots | AddMethod | AddAttribute | NewClass | CreateAccessMethods );
	else
		return Features ( Structs | Functions | Variables | Declarations );
}

QString CppSupportPart::formatClassName( const QString &name )
{
	QString n = name;
	return n.replace( ".", "::" );
}

QString CppSupportPart::unformatClassName( const QString &name )
{
	QString n = name;
	return n.replace( "::", "." );
}

bool CppSupportPart::shouldSplitDocument(const KURL &url)
{
	if ( !splitHeaderSourceConfig()->splitEnabled() )
		return false;

	KURL::List list = partController()->openURLs();
	KURL::List::ConstIterator it = list.begin();
	while ( it != list.end() )
	{
		QString candidate = sourceOrHeaderCandidate( ( *it ) );
		if ( candidate.isEmpty() )
		{
			++it;
			continue;
		}

		KURL urlCandidate;
		urlCandidate.setPath( candidate );
		if ( url == urlCandidate )
		{
			// It is already open, so switch to it so
			// our split view will open with it
			partController() ->editDocument( ( *it ) );
			return true;
		}
		++it;
	}
	return false;
}

Qt::Orientation CppSupportPart::splitOrientation() const
{
	QString o = splitHeaderSourceConfig()->orientation();
	if ( o == "Vertical" )
		return Qt::Vertical;
	else
		return Qt::Horizontal;
}

void CppSupportPart::slotNewClass()
{
	CppNewClassDialog dlg( this );
	dlg.exec();
}

void CppSupportPart::addMethod( ClassDom klass )
{
	if ( !klass )
	{
		KMessageBox::error( 0, i18n( "Please select a class." ), i18n( "Error" ) );
		return ;
	}

	AddMethodDialog dlg( this, klass, mainWindow() ->main() );
	dlg.exec();
}

void CppSupportPart::addAttribute( ClassDom klass )
{
	if ( !klass )
	{
		KMessageBox::error( 0, i18n( "Please select a class." ), i18n( "Error" ) );
		return ;
	}

	AddAttributeDialog dlg( this, klass, mainWindow() ->main() );
	dlg.exec();
}

void CppSupportPart::slotCompleteText()
{
	if ( !m_pCompletion )
		return ;
	m_pCompletion->completeText( true );
}

/**
 * parsing stuff for project persistent classstore and code completion
 */
void CppSupportPart::initialParse( )
{
	// For debugging
	if ( !project( ) )
	{
		// messagebox ?
		kdDebug( 9007 ) << "No project" << endl;
		return ;
	}

	parseProject( );
	m_valid = true;
	return ;
}

bool CppSupportPart::parseProject( bool force )
{
	if( _jd )
		delete _jd->progressBar; ///Make sure the progress-bar is open

	mainWindow() ->statusBar() ->message( i18n( "Updating..." ) );

	kapp->setOverrideCursor( waitCursor );

	_jd = new JobData;
	if( QFileInfo( project() ->projectDirectory() + "/" + project()->projectName().lower()
				+ ".kdevelop.pcs" ).exists())
	{
		QDir d( project() ->projectDirectory());
		d.rename(project() ->projectName().lower() + ".kdevelop.pcs",
					project() ->projectName() +".kdevelop.pcs");
	}
	_jd->file.setName( project() ->projectDirectory() + "/" + project()->projectName()
			+ ".kdevelop.pcs" );

	QString skip_file_name = project() ->projectDirectory() + "/" +
		project() ->projectName() + ".kdevelop.ignore_pcs";
	QString skip_lower_file_name = project() ->projectDirectory() + "/" +
		project() ->projectName().lower() + ".kdevelop.ignore_pcs";

	if ( !force && !QFile::exists( skip_file_name ) &&
			!QFile::exists( skip_lower_file_name ) && _jd->file.open( IO_ReadOnly ) )
	{
		_jd->stream.setDevice( &( _jd->file ) );

		createIgnorePCSFile();

		QString sig;
		int pcs_version = 0;
		_jd->stream >> sig >> pcs_version;
		if ( sig == "PCS" && pcs_version == KDEV_PCS_VERSION )
		{

			int numFiles = 0;
			_jd->stream >> numFiles;
			kdDebug( 9007 ) << "Read " << numFiles << " files from pcs" << endl;

			for ( int i = 0; i < numFiles; ++i )
			{
				QString fn;
				uint ts;
				uint offset;

				_jd->stream >> fn >> ts >> offset;
				_jd->pcs[ fn ] = qMakePair( ts, offset );
			}
		}
	}

	_jd->files = reorder( modifiedFileList() );

	QProgressBar* bar = new QProgressBar( _jd->files.count( ), mainWindow( ) ->statusBar( ) );
	bar->setMinimumWidth( 120 );
	bar->setCenterIndicator( true );
	mainWindow( ) ->statusBar( ) ->addWidget( bar );
	bar->show( );

	_jd->progressBar = bar;
	_jd->dir.setPath( m_projectDirectory );
	_jd->it = _jd->files.begin();
	_jd->reparseList = QStringList();
	_jd->backgroundCount = 0;
	_jd->cycle = 0;

	QTimer::singleShot( 0, this, SLOT( slotParseFiles() ) );

	m_saveMemoryTimer->stop(); //Do not regularly remove cached files that may still be needed while parsing(the cache anyway be full for the whole parsing-process)
	return true;
}

void CppSupportPart::slotParseFiles()
{
	// NOTE: The checking for m_projectClosed is actually (currently) not needed.
	// When the project is closed, the language support plugin is destroyed
	// and as a consequence, the timer job signal never arrives at this method

	if ( !_jd ) return; // how can this possibly happen?!

	if ( _jd->cycle == 0 && !m_projectClosed && _jd->it != _jd->files.end() )
	{
		_jd->progressBar->setProgress( _jd->progressBar->progress() + 1 );

		QFileInfo fileInfo( _jd->dir, *( _jd->it ) );

		if ( fileInfo.exists() && fileInfo.isFile() && fileInfo.isReadable() )
		{
			QString absFilePath = URLUtil::canonicalPath( fileInfo.absFilePath() );

			if ( isValidSource( absFilePath ) )
			{
				QDateTime t = fileInfo.lastModified();

				if ( ! ( m_timestamp.contains( absFilePath ) && m_timestamp[ absFilePath ] == t ) )
				{
					if ( _jd->pcs.contains( absFilePath ) )
					{
						_jd->stream.device() ->at( _jd->pcs[ absFilePath ].second );
						FileDom file = codeModel() ->create<FileModel>();
						file->read( _jd->stream );
						codeModel() ->addFile( file );

						if( t.toTime_t() != _jd->pcs[ absFilePath ].first ) {
							///The FileDom had to be created first, so the dependencies are known
							_jd->reparseList << file->name();
/*							kdDebug( 9007 ) << "File timestamp: " << ": " <<  t.toTime_t()   << endl;
							kdDebug( 9007 ) << "Stored timestamp: " << ": " <<  _jd->pcs[ absFilePath ].first << endl;*/
						} else {
							m_timestamp[ absFilePath ] = t;
/*							kdDebug( 9007 ) << "timestamp ok" << endl;*/
						}
					} else {
					  _jd->reparseList <<  absFilePath;
	/*				  kdDebug( 9007 ) << absFilePath << " put into reparse-list"  << endl;
	*/				}
				} else {
/*				  kdDebug( 9007 ) << absFilePath << " is already in code-model"  << endl;*/
				}
			}
		}

		++( _jd->it );
		QTimer::singleShot( 0, this, SLOT( slotParseFiles() ) );

		if( _jd->it == _jd->files.end()) {
			if( _jd->reparseList.isEmpty() ) {
				_jd->backgroundCount = 0;
			} else {
				if( alwaysParseInBackground ) {
					_jd->backgroundCount = parseFilesAndDependencies( _jd->reparseList, true, false, true );
				} else {
					_jd->reparseList = reorder( _jd->reparseList );
					_jd->it = _jd->reparseList.begin();
					_jd->backgroundCount = _jd->reparseList.count();
				}
				_jd->progressBar->setProgress( 0 ); ///restart progress-bar for reparsing
				_jd->progressBar->setTotalSteps( _jd->backgroundCount );
			}

			_jd->lastBackgroundState = -1;
			_jd->backgroundState = 0;
			_jd->cycle = 1;
			_jd->lastParse = QTime::currentTime();
			kapp->restoreOverrideCursor( );
		}
	}
	else // finished or interrupted
	{
		if( _jd->backgroundCount <= _jd->backgroundState || m_projectClosed ) {
			mainWindow( ) ->statusBar( ) ->removeWidget( _jd->progressBar );

			if ( !m_projectClosed )
			{
				kdDebug( 9007 ) << "updating sourceinfo" << endl;
				kapp->restoreOverrideCursor( );
				emit updatedSourceInfo();
				mainWindow( ) ->statusBar( ) ->message( i18n( "Done" ), 2000 );
				QFile::remove( project() ->projectDirectory()
						+ "/" + project() ->projectName()
						+ ".kdevelop.ignore_pcs" );
				QFile::remove( project() ->projectDirectory()
						+ "/" + project() ->projectName().lower()
						+ ".kdevelop.ignore_pcs" );

			}
			else
			{
				kdDebug( 9007 ) << "ABORT" << endl;
			}

			delete _jd;
			_jd = 0;
			m_saveMemoryTimer->start( 240000, false );
		} else {
			_jd->progressBar->setProgress( _jd->backgroundState ); ///restart
			_jd->progressBar->setTotalSteps( _jd->backgroundCount );
			if( _jd->lastParse.msecsTo( QTime::currentTime()) > 60000 && !m_backgroundParser->filesInQueue()) {
				_jd->backgroundCount = _jd->backgroundState;   ///Stop waiting if there is no progress and no file in the background-parser
				QTimer::singleShot( 0, this, SLOT( slotParseFiles() ) );
			} else {
				int timeStep = 0;
				if( alwaysParseInBackground ) {
					QTimer::singleShot( 10, this, SLOT( slotParseFiles() ) );
				} else {
					if( _jd->it == _jd->reparseList.end() ) {
						/*_jd->it = _jd->files.end();
						_jd->backgroundCount = _jd->backgroundState; ///finish processing*/
						timeStep = 1;
					} else {
						/*///Parse the files one by one
						if( _jd->lastParse.msecsTo( QTime::currentTime()) > 100 || _jd->backgroundState != _jd->lastBackgroundState ) {*/
							maybeParse( *_jd->it, false );
							++(_jd->it);
							_jd->lastBackgroundState = _jd->backgroundState;
						/*}else{
							timeStep = 1;
						}*/
					}
					QTimer::singleShot( timeStep, this, SLOT( slotParseFiles() ) );
				}
			}
		}
	}
}

void CppSupportPart::maybeParse( const QString& fn, bool background  )
{
	if ( !isValidSource( fn ) )
		return ;

	QFileInfo fileInfo( fn );
	QString path = URLUtil::canonicalPath( fn );
	QDateTime t = fileInfo.lastModified();

	if ( !fileInfo.exists() )
		return;

	QMap<QString, QDateTime>::Iterator it = m_timestamp.find( path );
	if ( codeModel()->hasFile( fn ) && it != m_timestamp.end() && *it == t )
		return;

	QStringList l;
	l << fn;
	parseFilesAndDependencies( l, background );
}

bool CppSupportPart::isQueued( const QString& file ) const {
	//int c = m_backgroundParser->countInQueue( file );
	//if( c == 0 ) return false;
	return m_parseEmitWaiting.waiting( file, ParseEmitWaiting::Silent, 2 ); //Since it may be possible that the background-parser is currently parsing the file(in an obselete state), it is allowed to have the file in the queue twice.
}

void CppSupportPart::slotNeedTextHint( int line, int column, QString& textHint )
{
	if ( 1 || !m_activeEditor )
		return ;

	m_backgroundParser->lock();
	TranslationUnitAST* ast = *m_backgroundParser->translationUnit( m_activeFileName );
	AST* node = 0;
	if ( ast && ( node = findNodeAt( ast, line, column ) ) )
	{

		while ( node && node->nodeType() != NodeType_FunctionDefinition )
			node = node->parent();

		if ( node )
		{
			int startLine, startColumn;
			int endLine, endColumn;
			node->getStartPosition( &startLine, &startColumn );
			node->getEndPosition( &endLine, &endColumn );

			if ( !node->text().isNull() )
				textHint = node->text();
			else
				textHint = m_activeEditor->textLine( startLine ).simplifyWhiteSpace();
		}
	}
	m_backgroundParser->unlock();
}

void CppSupportPart::MakeMemberHelper( QString& text, int& atLine, int& atColumn )
{
	if ( !m_activeViewCursor || !m_valid )
		return ;

	atLine = -2;
	atColumn = 0;

	QString implFile = findSourceFile();

	m_backgroundParser->lock();
	TranslationUnitAST* translationUnit = *m_backgroundParser->translationUnit( m_activeFileName );
	if ( translationUnit )
	{
		bool fail = false;
		unsigned int line, column;
		m_activeViewCursor->cursorPositionReal( &line, &column );

		AST* currentNode = findNodeAt( translationUnit, line, column );
		DeclaratorAST* declarator = 0;
		while ( currentNode && currentNode->nodeType() != NodeType_SimpleDeclaration )
		{
			if ( currentNode->nodeType() == NodeType_Declarator )
				declarator = ( DeclaratorAST* ) currentNode;
			currentNode = currentNode->parent();
		}
		SimpleDeclarationAST* decl = currentNode ? ( SimpleDeclarationAST* ) currentNode : 0;

		if ( decl && decl->storageSpecifier() && decl->storageSpecifier()->text().contains("friend") )
		{
			kdDebug(9007) << "this is a friend declaration, don't create any definition" << endl;
			fail = true;
		}

		if ( !fail && decl && decl->initDeclaratorList() && !declarator )
		{
			InitDeclaratorAST * i = decl->initDeclaratorList() ->initDeclaratorList().at( 0 );
			if ( i )
				declarator = i->declarator();
		}

		if ( !fail && decl && declarator && declarator->parameterDeclarationClause() )
		{

			QStringList scope;
			scopeOfNode( decl, scope );

			QString scopeStr = scope.join( "::" );
			if ( !scopeStr.isEmpty() )
				scopeStr += "::";

			QString declStr = declaratorToString( declarator, scopeStr ).simplifyWhiteSpace();
			if ( declarator->exceptionSpecification() )
			{
				declStr += QString::fromLatin1( " throw( " );
				QPtrList<AST> l = declarator->exceptionSpecification() ->nodeList();
				QPtrListIterator<AST> type_it( l );
				while ( type_it.current() )
				{
					declStr += type_it.current() ->text();
					++type_it;

					if ( type_it.current() )
						declStr += QString::fromLatin1( ", " );
				}

				declStr += QString::fromLatin1( " )" );
			}

			text += "\n\n";
			QString type = typeSpecToString( decl->typeSpec() );
			text += type;
			if ( !type.isNull() )
				text += + " ";

			text += declStr + "\n{\n}";
		}

		if ( !fail )
		{
			translationUnit = *m_backgroundParser->translationUnit( implFile );
			if ( translationUnit )
				translationUnit->getEndPosition( &atLine, &atColumn );
		}

		kdDebug( 9007 ) << "at line in mm: " << atLine << endl;
	}
	m_backgroundParser->unlock();
}

void CppSupportPart::slotMakeMember()
{
	QString text;
	int atColumn, atLine;
	MakeMemberHelper( text, atLine, atColumn );

	if ( !text.isEmpty() )
	{
		QString implFile = findSourceFile();

		if ( !implFile.isEmpty() )
		{
			partController() ->editDocument( KURL( implFile ) );
			kapp->eventLoop()->processEvents( QEventLoop::ExcludeUserInput | QEventLoop::ExcludeSocketNotifiers, 500 );
		}
		if ( atLine == -2 )
			atLine = m_activeEditor->numLines() - 1;

		m_backgroundParser->lock ()
			;

		kdDebug( 9007 ) << "at line in mm: " << atLine << " atCol: " << atColumn << endl;
		kdDebug( 9007 ) << "text: " << text << endl;
		if ( m_activeEditor )
			m_activeEditor->insertText( atLine, atColumn, text );
		if ( m_activeViewCursor )
			m_activeViewCursor->setCursorPositionReal( atLine + 3, 1 );

		m_backgroundParser->unlock();
	}
}

QStringList CppSupportPart::subclassWidget( const QString& formName )
{
	QStringList newFileNames;
	SubclassingDlg *dlg = new SubclassingDlg( this, formName, newFileNames );
	dlg->exec();
	return newFileNames;
}

QStringList CppSupportPart::updateWidget( const QString& formName, const QString& fileName )
{
	QStringList dummy;
	SubclassingDlg *dlg = new SubclassingDlg( this, formName, fileName, dummy );
	dlg->exec();
	return dummy;
}

void CppSupportPart::partRemoved( KParts::Part* part )
{
	kdDebug( 9032 ) << "CppSupportPart::partRemoved()" << endl;

	if ( KTextEditor::Document * doc = dynamic_cast<KTextEditor::Document*>( part ) )
	{

		QString fileName = doc->url().path();
		if ( !isValidSource( fileName ) )
			return ;

		QString canonicalFileName = URLUtil::canonicalPath( fileName );
		m_backgroundParser->removeFile( canonicalFileName );
		m_backgroundParser->addFile( canonicalFileName, true );
	}
}

void CppSupportPart::slotProjectCompiled()
{
	kdDebug( 9007 ) << "CppSupportPart::slotProjectCompiled()" << endl;
	parseProject();
}

QStringList CppSupportPart::modifiedFileList()
{
	QStringList lst;

	QStringList fileList = m_projectFileList;
	QStringList::Iterator it = fileList.begin();
	while ( it != fileList.end() )
	{
		QString fileName = *it;
		++it;

		QFileInfo fileInfo( m_projectDirectory, fileName );
		QString path = URLUtil::canonicalPath( fileInfo.absFilePath() );

		if ( !( isSource( path ) || isHeader( path ) ) )
			continue;

		QDateTime t = fileInfo.lastModified();

		QMap<QString, QDateTime>::Iterator dictIt = m_timestamp.find( path );
		if ( fileInfo.exists() && dictIt != m_timestamp.end() && *dictIt == t )
			continue;

		lst << fileName;
	}

	return lst;
}

KTextEditor::Document * CppSupportPart::findDocument( const KURL & url )
{
	if ( !partController() ->parts() )
		return 0;

	QPtrList<KParts::Part> parts( *partController() ->parts() );
	QPtrListIterator<KParts::Part> it( parts );
	while ( KParts::Part * part = it.current() )
	{
		KTextEditor::Document * doc = dynamic_cast<KTextEditor::Document*>( part );
		if ( doc && doc->url() == url )
			return doc;
		++it;
	}

	return 0;
}

void CppSupportPart::setupCatalog( )
{
	kdDebug( 9007 ) << "CppSupportPart::setupCatalog()" << endl;

	KStandardDirs *dirs = CppSupportFactory::instance() ->dirs();
	QStringList pcsList = dirs->findAllResources( "pcs", "*.db", false, true );
	QStringList pcsIdxList = dirs->findAllResources( "pcs", "*.idx", false, true );

	QStringList enabledPCSs;
	if ( DomUtil::elementByPath( *project() ->projectDom(), "kdevcppsupport/references" ).isNull() )
	{
		for ( QStringList::Iterator it = pcsList.begin(); it != pcsList.end(); ++it )
		{
			kdDebug( 9007 ) << "CppSupportPart::setupCatalog()1 " << *it << endl;
			enabledPCSs.push_back( QFileInfo( *it ).baseName(true) );
		}
	}
	else
	{
		enabledPCSs = DomUtil::readListEntry( *project() ->projectDom(), "kdevcppsupport/references", "pcs" );
	}

	QStringList indexList = QStringList() << "kind" << "name" << "scope" << "fileName" << "prefix";

	if ( pcsList.size() && pcsVersion() < KDEV_DB_VERSION )
	{
		QStringList l = pcsList + pcsIdxList;
		int rtn = KMessageBox::questionYesNoList( 0, i18n( "Persistent class store will be disabled: you have a wrong version of pcs installed.\nRemove old pcs files?" ), l, i18n( "C++ Support" ), KStdGuiItem::del(), KStdGuiItem::cancel() );
		if ( rtn == KMessageBox::Yes )
		{
			QStringList::Iterator it = l.begin();
			while ( it != l.end() )
			{
				QFile::remove
					( *it );
				++it;
			}
			// @todo regenerate the pcs list
			pcsList.clear();
		}
		else
		{
			return ;
		}
	}

	QStringList::Iterator it = pcsList.begin();
	while ( it != pcsList.end() )
	{
		kdDebug( 9007 ) << "CppSupportPart::setupCatalog()2 " << *it << endl;
		Catalog * catalog = new Catalog();
		catalog->open( *it );
		catalog->setEnabled( enabledPCSs.contains( QFileInfo( *it ).baseName(true) ) );
		++it;

		for ( QStringList::Iterator idxIt = indexList.begin(); idxIt != indexList.end(); ++idxIt )
			catalog->addIndex( ( *idxIt ).utf8() );

		m_catalogList.append( catalog );
		codeRepository() ->registerCatalog( catalog );
	}

	setPcsVersion( KDEV_DB_VERSION );
}

KMimeType::List CppSupportPart::mimeTypes( )
{
	QStringList mimeList;
	mimeList += m_headerMimeTypes;
	mimeList += m_sourceMimeTypes;

	KMimeType::List list;
	for ( QStringList::Iterator it = mimeList.begin(); it != mimeList.end(); ++it )
	{
		if ( KMimeType::Ptr mime = KMimeType::mimeType( *it ) )
			list << mime;
	}

	return list;
}

int CppSupportPart::pcsVersion()
{
	KConfig * config = CppSupportFactory::instance() ->config();
	KConfigGroupSaver cgs( config, "PCS" );
	return config->readNumEntry( "Version", 0 );
}

void CppSupportPart::setPcsVersion( int version )
{
	KConfig * config = CppSupportFactory::instance() ->config();
	KConfigGroupSaver cgs( config, "PCS" );
	config->writeEntry( "Version", version );
	config->sync();
}

QString CppSupportPart::formatTag( const Tag & inputTag )
{
	Tag tag = inputTag;

	switch ( tag.kind() )
	{
	case Tag::Kind_Namespace:
		return QString::fromLatin1( "namespace " ) + tag.name();

	case Tag::Kind_Class:
		return QString::fromLatin1( "class " ) + tag.name();

	case Tag::Kind_Function:
	case Tag::Kind_FunctionDeclaration:
		{
			CppFunction<Tag> tagInfo( tag );
			return tagInfo.name() + "( " + tagInfo.arguments().join( ", " ) + " ) : " + tagInfo.type();
		}
		break;

	case Tag::Kind_Variable:
	case Tag::Kind_VariableDeclaration:
		{
			CppVariable<Tag> tagInfo( tag );
			return tagInfo.name() + " : " + tagInfo.type();
		}
		break;
	}
	return tag.name();
}

void CppSupportPart::codeCompletionConfigStored( )
{
	if ( m_projectClosing ) return;
	updateParserConfiguration();
	/*
	m_backgroundParser->updateParserConfiguration();

	KDevDriver* d = dynamic_cast<KDevDriver*>( m_driver ); //The foreground-parse isn't used anymore, and could be removed
	if( d ) {
		d->setup();
		d->makeMacrosPersistent();
	}*/
	partController() ->setActivePart( partController()->activePart() );
}

void CppSupportPart::splitHeaderSourceConfigStored( )
{
	QString o = splitHeaderSourceConfig()->orientation();
	if ( o == "Vertical" )
		emit splitOrientationChanged( Qt::Vertical );
	else if ( o == "Horizontal" )
		emit splitOrientationChanged( Qt::Horizontal );
}

void CppSupportPart::removeWithReferences( const QString & fileName )
{
kdDebug( 9007 ) << "remove with references: " << fileName << endl;
	m_timestamp.remove( fileName );
	if ( !codeModel() ->hasFile( fileName ) )
		return ;

	emit aboutToRemoveSourceInfo( fileName );

	codeModel() ->removeFile( codeModel() ->fileByName( fileName ) );
}

bool CppSupportPart::isValidSource( const QString& fileName ) const
{
	QFileInfo fileInfo( fileName );
	QString path = URLUtil::canonicalPath( fileInfo.absFilePath() );

	return /*project() && project() ->isProjectFile( path )
		&&*/ ( isSource( path ) || isHeader( path ) )
		&& !QFile::exists( fileInfo.dirPath( true ) + "/.kdev_ignore" );
}

QString CppSupportPart::formatModelItem( const CodeModelItem *item, bool shortDescription )
{
	if ( item->isFunction() || item->isFunctionDefinition() )
	{
		const FunctionModel * model = static_cast<const FunctionModel*>( item );
		QString function;
		QString args;
		ArgumentList argumentList = model->argumentList();
		for ( ArgumentList::const_iterator it = argumentList.begin(); it != argumentList.end(); ++it )
		{
			args.isEmpty() ? args += "" : args += ", " ;
			args += formatModelItem( ( *it ).data() );
		}
		if ( !shortDescription )
			function += ( model->isVirtual() ? QString( "virtual " ) : QString( "" ) ) + model->resultType() + " ";

		function += model->name() + "(" + args + ")" + ( model->isConstant() ? QString( " const" ) : QString( "" ) ) +
			( model->isAbstract() ? QString( " = 0" ) : QString( "" ) );

		return function;
	}
	else if ( item->isVariable() )
	{
		const VariableModel * model = static_cast<const VariableModel*>( item );
		if ( shortDescription )
			return model->name();
		return model->type() + " " + model->name();
	}
	else if ( item->isArgument() )
	{
		const ArgumentModel * model = static_cast<const ArgumentModel*>( item );
		QString arg;
		if ( !shortDescription )
			arg += model->type() + " ";
		arg += model->name();
		if ( !shortDescription )
			arg += model->defaultValue().isEmpty() ? QString( "" ) : QString( " = " ) + model->defaultValue();
		return arg.stripWhiteSpace();
	}
	else
		return KDevLanguageSupport::formatModelItem( item, shortDescription );
}

void CppSupportPart::addClass()
{
	slotNewClass();
}

void CppSupportPart::saveProjectSourceInfo()
{
	const FileList fileList = codeModel() ->fileList();

	if ( !project() || fileList.isEmpty() )
		return ;

	QFile f( project() ->projectDirectory() + "/"
			+ project() ->projectName() + ".kdevelop.pcs" );
	if ( !f.open( IO_WriteOnly ) )
		return ;

	m_backgroundParser->lock();

	createIgnorePCSFile();

	QDataStream stream( &f );
	QMap<QString, uint> offsets;

	QString pcs( "PCS" );
	stream << pcs << KDEV_PCS_VERSION;

	stream << int( fileList.size() );
	for ( FileList::ConstIterator it = fileList.begin(); it != fileList.end(); ++it )
	{
		const FileDom dom = ( *it );
		stream << dom->name() << m_timestamp[ dom->name() ].toTime_t();
		if( m_timestamp.find( dom->name() ) == m_timestamp.end() ) {
		  kdDebug( 9007 ) << dom->name() << ": timestamp is missing "  << endl;
		}
		offsets.insert( dom->name(), stream.device() ->at() );
		stream << ( uint ) 0; // dummy offset
	}

	for ( FileList::ConstIterator it = fileList.begin(); it != fileList.end(); ++it )
	{
		const FileDom dom = ( *it );
		int offset = stream.device() ->at();

		dom->write( stream );

		int end = stream.device() ->at();

		stream.device() ->at( offsets[ dom->name() ] );
		stream << offset;
		stream.device() ->at( end );
	}

	QFile::remove(  project() ->projectDirectory() + "/"
			+ project() ->projectName() + ".kdevelop.ignore_pcs" );
	QFile::remove(  project() ->projectDirectory() + "/"
			+ project() ->projectName().lower() + ".kdevelop.ignore_pcs" );

	m_backgroundParser->unlock();
}

QString CppSupportPart::extractInterface( const ClassDom& klass )
{
	QString txt;
	QTextStream stream( &txt, IO_WriteOnly );

	QString name = klass->name() + "Interface";
	QString ind;
	ind.fill( QChar( ' ' ), 4 );

	stream
		<< "class " << name << "\n"
		<< "{" << "\n"
		<< "public:" << "\n"
		<< ind << name << "() {}" << "\n"
		<< ind << "virtual ~" << name << "() {}" << "\n"
		<< "\n";

	const FunctionList functionList = klass->functionList();
	for ( FunctionList::ConstIterator it = functionList.begin(); it != functionList.end(); ++it )
	{
		const FunctionDom& fun = *it;

		if ( !fun->isVirtual() || fun->name().startsWith( "~" ) )
			continue;

		stream << ind << formatModelItem( fun );
		if ( !fun->isAbstract() )
			stream << " = 0";

		stream << ";\n";
	}

	stream
		<< "\n"
		<< "private:" << "\n"
		<< ind << name << "( const " << name << "& source );" << "\n"
		<< ind << "void operator = ( const " << name << "& source );" << "\n"
		<< "};" << "\n\n";

	return txt;
}

void CppSupportPart::slotExtractInterface( )
{
	if ( !m_activeClass )
		return ;

	QFileInfo fileInfo( m_activeClass->fileName() );
	QString ifaceFileName = fileInfo.dirPath( true ) + "/" + m_activeClass->name().lower() + "_interface.h";
	if ( QFile::exists( ifaceFileName ) )
	{
		KMessageBox::error( mainWindow() ->main(), i18n( "File %1 already exists" ).arg( ifaceFileName ),
		                    i18n( "C++ Support" ) );
	}
	else
	{
		QString text = extractInterface( m_activeClass );

		QFile f( ifaceFileName );
		if ( f.open( IO_WriteOnly ) )
		{
			QTextStream stream( &f );
			stream
				<< "#ifndef __" << m_activeClass->name().upper() << "_INTERFACE_H" << "\n"
				<< "#define __" << m_activeClass->name().upper() << "_INTERFACE_H" << "\n"
				<< "\n"
				<< extractInterface( m_activeClass )
				<< "\n"
				<< "#endif // __" << m_activeClass->name().upper() << "_INTERFACE_H" << "\n";
			f.close();

			project() ->addFile( ifaceFileName );
		}
	}

	m_activeClass = 0;
}

void CppSupportPart::gotoLine( int line )
{
	if ( isHeader( m_activeFileName ) )
	{
		KURL url;
		url.setPath( sourceOrHeaderCandidate() );
		partController() ->editDocument( url, line );
	}
	else
		m_activeViewCursor->setCursorPositionReal( line, 0 );
}

FileDom CppSupportPart::fileByName( const QString& name) {
	return codeModel()->fileByName( name );
}


int CppSupportPart::parseFilesAndDependencies( QStringList files, bool background, bool parseFirst, bool silent ) {
	QMap<QString, int> fileGroups;
	int nextGroup = 0;

	for( QStringList::iterator it = files.begin(); it != files.end(); ++it ) {
		FileDom d = fileByName( *it );

		QStringList lst;
		if( !d ) {
			lst << *it;
		}else{
			lst = codeModel()->getGroupStrings( d->groupId() );
/*			kdDebug( 9007 ) << "adding group of: " << *it << ":\n" << " which is " << lst.join("\n") << "\n\n";*/
			if( lst.count() > 10 ) {
				lst = codeModel()->getGroupStrings( d->groupId() );
			}
		}
		int cgroup = nextGroup;
		nextGroup++;

		if( fileGroups.find( *it ) != fileGroups.end() )
			cgroup = fileGroups[*it];

		for( QStringList::iterator lit = lst.begin(); lit != lst.end(); ++lit )
			fileGroups[*lit] = cgroup;
	}

	QValueVector<QStringList> groups;
	groups.resize( nextGroup );

	///put the groups together
	for( QMap<QString, int>::iterator it = fileGroups.begin(); it != fileGroups.end(); ++it ) {
		groups[*it] << it.key();
	}

	for( int a = 0; a < nextGroup; a++ ) {
		QStringList group = reorder( groups[a] );


/*		kdDebug( 9007 ) << "reparsing the following group: " << ":\n" << group.join("\n") << "\n\n";*/
		if( background ) {

			m_backgroundParser->lock();

			if( !group.isEmpty() ) {
				if( !parseFirst )
					m_parseEmitWaiting.addGroup( group, silent ? ParseEmitWaiting::Silent : ParseEmitWaiting::None  );
				else
					m_parseEmitWaiting.addGroupFront( group, silent ? ParseEmitWaiting::Silent : ParseEmitWaiting::None  );
				if( !silent ) {
					if( !parseFirst )
						m_fileParsedEmitWaiting.addGroup( group, silent ? ParseEmitWaiting::Silent : ParseEmitWaiting::None  );
					else
						m_fileParsedEmitWaiting.addGroupFront( group, silent ? ParseEmitWaiting::Silent : ParseEmitWaiting::None  );
				}
			}

			if( parseFirst && !group.empty() ) {
				for(QStringList::iterator it = --group.end(); it != group.end(); ) {
					backgroundParser()->addFileFront(*it);
					if( it == group.begin() ) {
						it = group.end();
					} else {
						--it;
					}
				}
			} else {
				for(QStringList::iterator it = group.begin(); it != group.end(); ++it) {
				  backgroundParser()->addFile(*it);
				}
			}

			m_backgroundParser->unlock();

		} else {
			for(QStringList::iterator it = group.begin(); it != group.end(); ++it) {
				m_driver->parseFile( *it );
			}
		}
	}

	return fileGroups.count();
}

int CppSupportPart::parseFileAndDependencies( const QString & fileName, bool background, bool parseFirst, bool silent ) {
	if(! isValidSource( fileName ) ) return 0;

// 	kdDebug( 9007 ) << "reparsing dependencies of " << fileName << "\n";

	return parseFilesAndDependencies( fileName, background, parseFirst, silent );
}

void CppSupportPart::parseEmit( ParseEmitWaiting::Processed files ) {
	if( files.res.isEmpty() ) return;

	bool modelHasFiles = true;

	for( QStringList::iterator it = files.res.begin(); it != files.res.end(); ++it ) {
		if( !codeModel()->hasFile( *it ) ) modelHasFiles = false;
	}

	int oldFileCount = codeModel()->fileList().count();

	if( (files.flag & ParseEmitWaiting::HadErrors) && modelHasFiles && !files.hasFlag( ParseEmitWaiting::Silent ) ) {
		mainWindow() ->statusBar() ->message( "File parsed, but not updating code-model because of errors", 2000 );
		kdDebug( 9007 ) << "not updating code-model because at least one file has errors" << endl;
		//		for( QStringList::iterator it = files.res.begin(); it != files.res.end(); ++it )
		  //			m_backgroundParser->removeFile( *it );
	} else {
	  ///update timestamps
	  for( QStringList::iterator it = files.res.begin(); it != files.res.end(); ++it ) {
	    if( !codeModel()->hasFile( *it ) ) modelHasFiles = false;
	    QString& fileName = *it;

	    QFileInfo fileInfo( fileName );
	    QString path = URLUtil::canonicalPath( fileName );

	    if ( !fileInfo.exists() ) {
	      removeWithReferences( path );
	      continue ;
	    }

	    m_timestamp[ path ] = fileInfo.lastModified();
	  }

		if( files.hasFlag( ParseEmitWaiting::Silent ) && !alwaysParseInBackground )
		  return;

		m_backgroundParser->lock();

		QStringList l = files.res;

		QMap<QString, bool> wholeResult;
		QStringList missing;

		QMap<QString, FileDom> newFiles;

		while(!l.isEmpty() ) {
			QString fileName = l.front();

			if( !m_backgroundParser->hasTranslationUnit( fileName ) ) {
			  kdDebug( 9007 ) << "error: translation-unit is missing: " << fileName << endl;
				missing << fileName;
			} else {
				if ( ParsedFilePointer ast = m_backgroundParser->translationUnit( fileName ) )
				{
					if ( true /*!hasErrors*/ )
					{
						FileDom oldFile = codeModel()->fileByName( fileName );

						StoreWalker walker( fileName, codeModel() );
						walker.setOverrides( newFiles );

						walker.parseTranslationUnit( *ast );

						if( oldFile ) {
							newFiles[fileName] = walker.file();

							///update timestamps
							QFileInfo fileInfo( fileName );
							QString path = URLUtil::canonicalPath( fileName );

							m_timestamp[ path ] = fileInfo.lastModified();
						} else {
							codeModel() ->addFile( walker.file() );
						}

						if( walker.file() ) {
							QStringList grp = walker.file()->wholeGroupStrings();
							for( QStringList::const_iterator it = grp.begin(); it != grp.end(); ++it )
								wholeResult[*it] = true;
						}
					}
				} else {
					kdDebug( 9007 ) << "failed to parse " << fileName << endl;
				}
			}


			l.pop_front();
		}

		bool canUpdate = true;
		for( QMap<QString, FileDom>::const_iterator it = newFiles.begin(); it != newFiles.end(); ++it ) {
			FileDom oldFile = codeModel()->fileByName( it.key() );

			if( !oldFile || !oldFile->canUpdate( *it ) ) {
				canUpdate = false;
				break;
			}
		}

		if( canUpdate ) {
			///Update the code-model
			for( QMap<QString, FileDom>::const_iterator it = newFiles.begin(); it != newFiles.end(); ++it ) {
				FileDom oldFile = codeModel()->fileByName( it.key() );
				oldFile->update( *it );
				codeModel()->mergeGroups( oldFile->groupId(), (*it)->groupId() ); ///Merge parsing-groups together
			}
		} else {
			///Remove the current files and replace them with the new ones
			for( QMap<QString, FileDom>::const_iterator it = newFiles.begin(); it != newFiles.end(); ++it ) {
				removeWithReferences( it.key() );
				codeModel()->addFile( *it );
			}
		}
		/*
		///make the list unique

		l.clear();
		for( QMap<QString, bool>::const_iterator it = wholeResult.begin(); it != wholeResult.end(); ++it )
			l << it.key();*/

		m_backgroundParser->unlock();

		if( !missing.isEmpty() ) {
			kdDebug( 9007 ) << "error: translation-units were missing: " << missing << endl;
  	           //don't reparse missing units, because it may cause the whole project to be reparsed
		  //			parseFilesAndDependencies( missing, true, false, files.hasFlag( ParseEmitWaiting::Silent ) );
		}

		if( files.hasFlag( ParseEmitWaiting::Silent ) ) {
			if( alwaysParseInBackground )
			for( QStringList::iterator it = files.res.begin(); it != files.res.end(); ++it )
				m_backgroundParser->removeFile( *it );
		} else {
			if( !canUpdate ) {  ///If the current model could be updated, do not emit addedSourceInfo(..) and remove the units from the parser, because nobody will be using them
				QStringList l = files.res;
				while(!l.isEmpty() ) {
					emit aboutToRemoveSourceInfo( l.front() );
					emit removedSourceInfo( l.front() );
					emit addedSourceInfo( l.front() );
					l.pop_front();
				}

				if( !files.hasFlag( ParseEmitWaiting::Silent ) )
					emitFileParsed( files );
			} else {
				QStringList l = files.res;
				while( !l.isEmpty() ) {
					emit codeModelUpdated( l.front() );
					emit aboutToRemoveSourceInfo( l.front() );
					emit removedSourceInfo( l.front() );
					emit addedSourceInfo( l.front() );
					l.pop_front();
				}
			}
		}
		kdDebug( 9007 ) << "files in code-model after parseEmit: " << codeModel()->fileList().count() << " before: " << oldFileCount << endl;
	}
}

/*void CppSupportPart::recomputeCodeModel( const QString& fileName )
{*/

//}

void CppSupportPart::emitSynchronousParseReady( const QString& file, ParsedFilePointer unit ) {
	emit synchronousParseReady( file, unit );
}

void CppSupportPart::emitFileParsed( QStringList l )
{
	while( !l.isEmpty() ) {
		emit fileParsed( l.front() );
		l.pop_front();
	}
}

bool CppSupportPart::isHeader( const QString& fileName ) const
{
	/*KMimeType::Ptr ptr = KMimeType::findByPath( fileName );
	if ( ptr && m_headerMimeTypes.contains( ptr->name() ) )
		return true;*/

	return ( m_headerExtensions.findIndex( QFileInfo( fileName ).extension() ) != -1 );
}

bool CppSupportPart::isSource( const QString& fileName ) const
{
	/*KMimeType::Ptr ptr = KMimeType::findByPath( fileName );
	if ( ptr && m_sourceMimeTypes.contains( ptr->name() ) )
		return true;*/

	return ( m_sourceExtensions.findIndex( QFileInfo( fileName ).extension() ) != -1 );
}

void CppSupportPart::gotoDeclarationLine( int line )
{
	if ( isHeader( m_activeFileName ) )
		m_activeViewCursor->setCursorPositionReal( line, 0 );
	else
	{
		KURL url;
		url.setPath( sourceOrHeaderCandidate() );
		partController() ->editDocument( url, line );
	}
}

void CppSupportPart::removeCatalog( const QString & dbName )
{
	if ( !QFile::exists( dbName ) )
		return ;

	QValueList<Catalog*> catalogs = codeRepository() ->registeredCatalogs();
	Catalog* c = 0;
	for ( QValueList<Catalog*>::Iterator it = catalogs.begin(); it != catalogs.end(); ++it )
	{
		if ( ( *it ) ->dbName() == dbName )
		{
			c = *it;
			break;
		}
	}

	if ( c )
	{
		codeRepository() ->unregisterCatalog( c );
		m_catalogList.remove( c );
	}

	QFileInfo fileInfo( dbName );
	QDir dir( fileInfo.dir( true ) );
	QStringList indexList = QStringList() << "kind" << "name" << "scope" << "fileName" << "prefix";
	for(QStringList::Iterator iter = indexList.begin(); iter != indexList.end(); iter++)
	{
		QStringList fileList = dir.entryList( fileInfo.baseName(true) +"." +(*iter) + ".idx" );
		for ( QStringList::Iterator it = fileList.begin(); it != fileList.end(); ++it )
		{
			QString idxName = fileInfo.dirPath( true ) + "/" + *it;
		kdDebug( 9007 ) << "=========> remove db index: " << idxName << endl;
			dir.remove( *it );
		}
	}
	dir.remove( fileInfo.fileName() );
}

void CppSupportPart::addCatalog( Catalog * catalog )
{
	m_catalogList.append( catalog );
	codeRepository() ->registerCatalog( catalog );
}

FunctionDefinitionDom CppSupportPart::functionDefinitionAt( int line, int column )
{
	if ( !codeModel() ->hasFile( m_activeFileName ) )
		return FunctionDefinitionDom();

	CodeModelUtils::CodeModelHelper h( codeModel(), codeModel()->fileByName( m_activeFileName ) );

	FunctionDom d = h.functionAt( line, column, CodeModelUtils::CodeModelHelper::Definition );
	if( d ) {
		FunctionDefinitionModel* m = dynamic_cast<FunctionDefinitionModel*>( d.data() );
		if( m ) return FunctionDefinitionDom( m );
	}
	return FunctionDefinitionDom();
}

FunctionDefinitionDom CppSupportPart::currentFunctionDefinition( )
{
	if ( !this->m_activeViewCursor )
		return FunctionDefinitionDom();

	unsigned int line, column;
	this->m_activeViewCursor->cursorPositionReal( &line, &column );
	return functionDefinitionAt( line, column );
}

void CppSupportPart::slotCursorPositionChanged()
{
	if ( codeCompletion() )
	{
		unsigned int line = 0;
		unsigned int column = 0;
		if ( KDevEditorUtil::currentPositionReal( &line, &column, dynamic_cast<KTextEditor::Document*>( partController()->activePart() ) ) )
		{
			QString typeInfoString = codeCompletion()->createTypeInfoString( line, column );
			mainWindow()->statusBar()->message( typeInfoString );
		}
	}

	//    m_functionHintTimer->changeInterval( 1000 );
	if ( splitHeaderSourceConfig()->splitEnabled()
	     && splitHeaderSourceConfig()->autoSync() )
		slotSwitchHeader( true );
}

/*
void CppSupportPart::slotFunctionHint( )
{
	kdDebug( 9007 ) << "=======> compute current function definition" << endl;
// 	m_functionHintTimer->stop();
	if ( FunctionDefinitionDom fun = currentFunctionDefinition() )
	{
		QStringList scope = fun->scope();
		QString funName = scope.join( "::" );
		if ( !funName.isEmpty() )
			funName += "::";

		funName += formatModelItem( fun, true );

		mainWindow() ->statusBar() ->message( funName, 2000 );
	}
}
*/

void CppSupportPart::createIgnorePCSFile( )
{
	static QCString skip_me( "ignore me\n" );

	QString skip_file_name = project() ->projectDirectory() + "/"
		+ project() ->projectName() + ".kdevelop.ignore_pcs";
	QFile skip_pcs_file( skip_file_name );
	if ( skip_pcs_file.open( IO_WriteOnly ) )
	{
		skip_pcs_file.writeBlock( skip_me );
		skip_pcs_file.close();
	}
}

QString CppSupportPart::specialHeaderName( bool local ) const
{
	if ( local )
		return ::locateLocal( "data", "kdevcppsupport/configuration", CppSupportFactory::instance() );

	return ::locate( "data", "kdevcppsupport/configuration", CppSupportFactory::instance() );
}

void CppSupportPart::updateParserConfiguration()
{
	m_backgroundParser->updateParserConfiguration();

	QString conf_file_name = specialHeaderName();

	m_driver->removeAllMacrosInFile( conf_file_name );
	dynamic_cast<KDevDriver*>(m_driver)->setup();
	m_driver->parseFile( conf_file_name, true, true, true );

	m_buildSafeFileSetTimer->start( 500, true );
	parseProject( true );
}

const Driver* CppSupportPart::driver() const {
	return m_driver;
}

Driver* CppSupportPart::driver() {
	return m_driver;
}

KDevDesignerIntegration * CppSupportPart::designer( KInterfaceDesigner::DesignerType type )
{
	KDevDesignerIntegration * des = 0;
	switch ( type )
	{
	case KInterfaceDesigner::Glade:
	case KInterfaceDesigner::QtDesigner:
		des = m_designers[ type ];
		if ( des == 0 )
		{
			CppImplementationWidget * impl = new CppImplementationWidget( this );
			des = new QtDesignerCppIntegration( this, impl );
			des->loadSettings( *project() ->projectDom(), "kdevcppsupport/designerintegration" );
			m_designers[ type ] = des;
		}
		break;
	}
	return des;
}


void CppSupportPart::resetParserStoreTimer() {
  //	m_deleteParserStoreTimer->start(10000); ///try to empty the store regularly
}

void CppSupportPart::slotDeleteParserStore() {
  /*        if( !m_backgroundParser->filesInQueue() )
	   m_backgroundParser->removeAllFiles();
	else
	resetParserStoreTimer();*/
}


void CppSupportPart::slotCreateSubclass()
{
	QFileInfo fi( m_contextFileName );
	if ( fi.extension( false ) != "ui" )
		return ;
	QtDesignerCppIntegration *des = dynamic_cast<QtDesignerCppIntegration*>( designer( KInterfaceDesigner::QtDesigner ) );
	if ( des )
		des->selectImplementation( m_contextFileName );
}

void CppSupportPart::addMethod( ClassDom aClass, const QString& name, const QString type,
                                const QString& parameters, CodeModelItem::Access accessType,
                                bool isConst, bool isInline, bool isVirtual, bool isPureVirtual,
                                const QString& implementation )
{
	partController() ->editDocument( KURL( aClass->fileName() ) );
	KTextEditor::EditInterface* editIface = dynamic_cast<KTextEditor::EditInterface*>( partController() ->activePart() );
	if ( !editIface )
	{
		/// @fixme show messagebox
		return ;
	}
	QString declarationString = type + " " + name + "(" + parameters + ")" + ( isConst ? " const" : "" );

	KDevSourceFormatter* sourceFormatter = extension<KDevSourceFormatter>( "KDevelop/SourceFormatter" );

	QString finalDeclaration = ( ( isVirtual || isPureVirtual ) ? "\nvirtual " : "\n" + declarationString +
	                             ( isPureVirtual ? " = 0 " : "" ) +
	                             ( isInline ? "\n{\n" + implementation + "\n}\n" : ";" ) );

	if ( sourceFormatter != 0 )
		finalDeclaration = sourceFormatter->formatSource( finalDeclaration );

	QString indentString = "\t";

	if ( sourceFormatter != 0 )
		indentString = sourceFormatter->indentString();

	editIface->insertText( findInsertionLineMethod( aClass, accessType ), 0,
	                       finalDeclaration.replace( "\n", "\n\t" ) + "\n" );

	backgroundParser() ->addFile( aClass->fileName() );
	if ( isInline || isPureVirtual )
		return ;

	// construct fully qualified name for method definition
	QString fullyQualifiedName = aClass->scope().join("::");
	if (! fullyQualifiedName.isEmpty())
	{
		fullyQualifiedName += "::";
	}
	fullyQualifiedName += aClass->name() + "::" + name;

	QString definitionString = "\n" + type + " " + fullyQualifiedName + "(" + parameters + ")" + ( isConst ? " const" : "" ) + "\n{\n" + implementation + "\n}\n";

	if ( sourceFormatter != 0 )
		definitionString = sourceFormatter->formatSource( definitionString );

	QFileInfo info( aClass->fileName() );
	QString implementationFile = info.dirPath( true ) + "/" + info.baseName() + ".cpp" ;
	QFileInfo fileInfo( implementationFile );
	KDevCreateFile* createFileSupport = extension<KDevCreateFile>( "KDevelop/CreateFile" );
	if ( !QFile::exists( fileInfo.absFilePath() ) && createFileSupport != 0 )
		createFileSupport->createNewFile( fileInfo.extension(), fileInfo.dirPath( true ), fileInfo.baseName() );

	partController() ->editDocument( KURL( implementationFile ) );
	editIface = dynamic_cast<KTextEditor::EditInterface*>( partController() ->activePart() );
	if ( !editIface )
		return ; //@fixme errorverdoedelung

	editIface->insertLine( editIface->numLines(), QString::fromLatin1( "" ) );
	editIface->insertText( editIface->numLines() - 1, 0, definitionString );
	backgroundParser() ->addFile( implementationFile );
}

ClassDom CppSupportPart::currentClass( ) const
{
	FileDom file = codeModel() ->fileByName( m_activeFileName );
	if ( file == 0 || m_activeViewCursor == 0 )
		return 0;

	unsigned int curLine, curCol;
	m_activeViewCursor->cursorPositionReal( &curLine, &curCol );

	CodeModelUtils::CodeModelHelper h( codeModel(), file );

	return h.classAt( curLine, curCol );
}

VariableDom CppSupportPart::currentAttribute( ClassDom curClass ) const
{
	if ( m_activeViewCursor == 0 || curClass == 0 )
		return 0;

	unsigned int line, col;
	m_activeViewCursor->cursorPositionReal( &line, &col );

	VariableList vars = curClass->variableList();

	for ( VariableList::iterator i = vars.begin(); i != vars.end(); ++i )
	{
		int startLine, startCol;
		( *i ) ->getStartPosition( &startLine, &startCol );
		if ( startLine < (int)line || ( startLine == (int)line && startCol <= (int)col ) )
		{
			int endLine, endCol;
			( *i ) ->getEndPosition( &endLine, &endCol );
			if ( endLine > (int)line || ( endLine == (int)line && endCol >= (int)col ) )
				return * i;
		}
	}
	return 0;
}

void CppSupportPart::slotCreateAccessMethods( )
{
	if ( m_curAttribute == 0 || m_curClass == 0 )
		return ;

	CreateGetterSetterDialog dlg ( this, m_curClass, m_curAttribute );
	dlg.exec();
}

int CppSupportPart::findInsertionLineMethod( ClassDom aClass, CodeModelItem::Access access )
{
	int line, column;
	aClass->getEndPosition( &line, &column );

	int point = CodeModelUtils::findLastMethodLine( aClass, access );

	if ( point == -1 )
	{
		KTextEditor::EditInterface * editIface = dynamic_cast<KTextEditor::EditInterface*>( partController() ->activePart() );
		if ( !editIface )
			return -1;

		editIface->insertLine( line - 1, CodeModelUtils::accessSpecifierToString( access ) + ":\n" );
		return line;
	}

	return point + 1;
}

int CppSupportPart::findInsertionLineVariable( ClassDom aClass, CodeModelItem::Access access )
{
	int line, column;
	aClass->getEndPosition( &line, &column );

	int point = CodeModelUtils::findLastVariableLine( aClass, access );

	if ( point == -1 )
	{
		KTextEditor::EditInterface * editIface = dynamic_cast<KTextEditor::EditInterface*>( partController() ->activePart() );
		if ( !editIface )
			return -1;

		editIface->insertLine( line - 1, CodeModelUtils::accessSpecifierToString( access ) + ":\n" );
		return line;
	}

	return point;
}

void CppSupportPart::createAccessMethods( ClassDom theClass, VariableDom theVariable )
{
	m_curClass = theClass;
	m_curAttribute = theVariable;

	slotCreateAccessMethods();
}

void CppSupportPart::slotCursorMoved()
{
	m_cursorMovedTimer->start( 250, true );
}

void CppSupportPart::slotTextChanged()
{
	setTyping( true );	///@todo check if this is really needed

	if ( m_backgroundParserConfig->useBackgroundParser() )
	{
		m_textChangedTimer->start( m_backgroundParserConfig->backgroudParseDelay(), true );
	}
}

void CppSupportPart::slotParseCurrentFile()
{
 	if( isValid() && !isQueued( m_activeFileName ) )
	{
		parseFileAndDependencies( m_activeFileName, true, true );
	}
}

void CppSupportPart::updateBackgroundParserConfig()
{
	BackgroundParserConfig config;
	config.readConfig();

	if ( m_backgroundParserConfig->useProblemReporter() && !config.useProblemReporter() )
	{
		removeProblemReporter();
	}
	else if ( !m_backgroundParserConfig->useProblemReporter() && config.useProblemReporter() )
	{
		embedProblemReporter( true );
	}

	*m_backgroundParserConfig = config;
}

const SynchronizedFileSet& CppSupportPart::safeFileSet() const {
	return m_safeProjectFiles;
}

SynchronizedFileSet& CppSupportPart::safeFileSet() {
	return m_safeProjectFiles;
}

void CppSupportPart::buildSafeFileSet() {
        if( codeCompletion() == 0 ) //probably the project has already been closed
          return;
	SynchronizedFileSet::SetType files; //everything that goes into this set must be deep-copied

	kdDebug( 9007 ) << "CppSupportPart:: rebuilding safe-file-set" << endl;
	for( QStringList::const_iterator it = m_projectFileList.begin(); it != m_projectFileList.end(); ++it ) {
		QFileInfo fi( *it );
		QString file = *it;
		if( fi.isRelative() ) {
			fi.setFile( QDir(m_projectDirectory), *it );
			file = fi.absFilePath();
		}

		//deep-copy
		files.insert( QString::fromUtf8(file.utf8()) );
	}

	///Now get all translation-units from the code-repository
 	QValueList<Catalog::QueryArgument> args;

	args << Catalog::QueryArgument( "kind", Tag::Kind_TranslationUnit );

	QValueList<Tag> tags( codeCompletion()->repository()->query( args ) );

	for( QValueList<Tag>::const_iterator it = tags.begin(); it != tags.end(); ++it ) {
		files.insert( (*it).fileName() + "||" + (*it).attribute("macroValueHash").toString() + "||" + (*it).attribute("macroIdHash").toString() );
	}
	m_safeProjectFiles.setFiles( files );
}

void CppSupportPart::addToRepository( ParsedFilePointer file ) {
	QString catalogString( "automatic_" + KURL::encode_string_no_slash(m_projectDirectory) );

	KStandardDirs *dirs = CppSupportFactory::instance() ->dirs();

	QString dbName = dirs->saveLocation( "data", "kdevcppsupport/pcs" ) + catalogString + ".db";

	Catalog* catalog = 0;
	///First check if the catalog is already there
	QValueList<Catalog*> catalogs = codeRepository()->registeredCatalogs();
	for( QValueList<Catalog*>::const_iterator it = catalogs.begin(); it != catalogs.end(); ++it ) {
		if( (*it)->dbName() == dbName ) {
			catalog = *it;
			break;
		}
	}

	if( !catalog ) {
		kdDebug( 9007 ) << "creating new catalog named " << catalogString << " for automatic filling" << endl;
		//QStringList indexList = QStringList() << "kind" << "name" << "scope" << "fileName" << "prefix";
		catalog = new Catalog;
		catalog->open( dbName );
		catalog->addIndex( "kind" );
		catalog->addIndex( "name" );
		catalog->addIndex( "scope" );
		catalog->addIndex( "prefix" );
		catalog->addIndex( "fileName" );
		/*
		for ( QStringList::Iterator idxIt = indexList.begin(); idxIt != indexList.end(); ++idxIt )
			catalog->addIndex( ( *idxIt ).utf8() );*/
		addCatalog( catalog );
	}
	catalog->setEnabled( true );

	///Now check if the file was already parsed with the same parameters, if yes don't parse again(auto-update is currently not supported, when major changes have been done in the libraries, the repository should be deleted)
    QValueList<Catalog::QueryArgument> args;

	bool compatibleParsed = false;
	Tag compatibleParsedTag;

	args << Catalog::QueryArgument( "kind", Tag::Kind_TranslationUnit );
	args << Catalog::QueryArgument( "fileName", file->fileName() );
	QValueList<Tag> tags( catalog->query( args ) );
	if( !tags.isEmpty() ) {
		for( QValueList<Tag>::const_iterator it = tags.begin(); it != tags.end(); ++it ) {
			if( (*it).hasAttribute( "cppparsedfile" ) ) {
				QVariant v = (*it).attribute( "cppparsedfile" );
				///@todo reenable this
				/*QByteArray b = v.toByteArray();
				if( !b.isEmpty() ) {
					//Would be much more efficient not to do this deserialization
					ParsedFile f(b);
					if( f.usedMacros().valueHash() == file->usedMacros().valueHash() && f.usedMacros().idHash() == file->usedMacros().idHash() && f.includeFiles().hash() == file->includeFiles().hash() ) {
						///Do not reparse the file, it seems to already be in the repository in a similar state
						if( (*it).attribute( "includedFrom" ).toString() == file->includedFrom() ) return;

						///It is probable that the same state has already been parsed, but there seems to be no such tag yet(the tag will be added)
						compatibleParsed = true;
						compatibleParsedTag = *it;
						break;
					}
				}*/
			}
		}
 }

	if( compatibleParsed ) {
		///Add a Tag that makes sure that the file will not be parsed again
		compatibleParsedTag.setAttribute( "includedFrom", file->includedFrom() );
		QByteArray data;
		QDataStream s( data, IO_WriteOnly );
		file->write( s );
		compatibleParsedTag.setAttribute( "cppparsedfile", data );
		catalog->addItem( compatibleParsedTag );
		return;
	}

	kdDebug( 9007 ) << "parsing translation-unit " << file->fileName() << " into catalog " << catalogString << endl;
	TagCreator w( file->fileName(), catalog );
	w.parseTranslationUnit( *file );
	codeRepository()->touchCatalog( catalog );

	m_safeProjectFiles.insert( file->fileName() + "||" + QString("%1").arg(file->usedMacros().valueHash()) + "||" + QString("%1").arg(file->usedMacros().idHash()) );
}

QString CppSupportPart::findHeaderSimple( const QString &header )
{
        QStringList::ConstIterator it;
        for ( it = m_projectFileList.begin(); it != m_projectFileList.end(); ++it )
        {
                QString s = *it;
                if (s == header)
                    return s;
                if ( ( s.right( header.length() ) == header ) && ( s[s.length() - header.length() - 1] == '/' ) )
                        return s;
        }

        return QString::null;
}

UIBlockTester::UIBlockTesterThread::UIBlockTesterThread( UIBlockTester& parent ) : QThread(), m_parent( parent ), m_stop(false) {
}

void UIBlockTester::UIBlockTesterThread::run() {
  while(!m_stop) {
	  msleep( m_parent.m_msecs / 10 );
	  m_parent.m_timeMutex.lock();
	  QDateTime t = QDateTime::currentDateTime();
	  uint msecs = m_parent.m_lastTime.time().msecsTo( t.time() );
	  if( msecs > m_parent.m_msecs ) {
		  m_parent.lockup();
		  m_parent.m_lastTime = t;
	  }
	  m_parent.m_timeMutex.unlock();
  }
}

void UIBlockTester::UIBlockTesterThread::stop() {
	m_stop = true;
}

UIBlockTester::UIBlockTester( uint milliseconds ) : m_thread( *this ), m_msecs( milliseconds ) {
	m_timer = new QTimer( this );
	m_timer->start( milliseconds/10 );
	connect( m_timer, SIGNAL(timeout()), this, SLOT(timer()) );
	timer();
	m_thread.start();
}
UIBlockTester::~UIBlockTester() {
  m_thread.stop();
  m_thread.wait();
}

void UIBlockTester::timer() {
	m_timeMutex.lock();
	m_lastTime = QDateTime::currentDateTime();
	m_timeMutex.unlock();
}

void UIBlockTester::lockup() {
	//std::cout << "UIBlockTester: lockup of the UI for " << m_msecs << endl; ///kdDebug(..) is not thread-safe..
	int a = 1; ///Place breakpoint here
}

#include "cppsupportpart.moc"
//kate: indent-mode csands; tab-width 4; space-indent off;


