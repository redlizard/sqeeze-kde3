/***************************************************************************
 *   Copyright (C) 2004 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qwhatsthis.h>
#include <qpopupmenu.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qregexp.h>

#include <klineedit.h>
#include <klistview.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kparts/part.h>
#include <ktexteditor/document.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/viewcursorinterface.h>
#include <kprocess.h>
#include <kdebug.h>
#include <kstringhandler.h>
#include <kdialogbase.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kaction.h>

#include "kdevappfrontend.h"
#include <kdevgenericfactory.h>
#include <kdevcore.h>
#include <kdevmainwindow.h>
#include <kdevproject.h>
#include <kdevpartcontroller.h>
#include <kdevplugininfo.h>
#include "configwidgetproxy.h"
#include "domutil.h"
#include "kdeveditorutil.h"

#include "ctags2_settingswidget.h"
#include "ctags2_widget.h"
#include "ctags2_part.h"
#include "tags.h"

#define CTAGSSETTINGSPAGE 1

namespace ctags
{
#include "readtags.h"
}

typedef KDevGenericFactory<CTags2Part> CTags2Factory;
static const KDevPluginInfo data("kdevctags2");
K_EXPORT_COMPONENT_FACTORY( libkdevctags2, CTags2Factory( data ) )

CTags2Part::CTags2Part(QObject *parent, const char *name, const QStringList& )
  : KDevPlugin(&data, parent, name ? name : "ctags2Part" )
{
	setInstance(CTags2Factory::instance());
	setXMLFile("kdevpart_ctags2.rc");

	QDomDocument & dom = *projectDom();
	QString customTagFile = DomUtil::readEntry( dom, "/ctagspart/customTagfilePath" );
	if ( customTagFile.isEmpty() )
	{
		customTagFile =  project()->projectDirectory() + "/tags";
	}
	QStringList tagFiles = DomUtil::readListEntry(dom, "/ctagspart/activeTagsFiles", "file");
	tagFiles.push_front(customTagFile);

	Tags::setTagFiles(tagFiles);

	m_widget = new CTags2Widget(this);

	QWhatsThis::add(m_widget, i18n("<b>CTags</b><p>Result view for a tag lookup. Click a line to go to the corresponding place in the code."));
	m_widget->setCaption(i18n("CTags Lookup"));
	mainWindow()->embedOutputView( m_widget, i18n( "CTags" ), i18n( "CTags lookup results" ) );

	connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)), this, SLOT(contextMenu(QPopupMenu *, const Context *)) );

	_configProxy = new ConfigWidgetProxy( core() );
	_configProxy->createProjectConfigPage( i18n("CTags"), CTAGSSETTINGSPAGE, info()->icon() );
	connect( _configProxy, SIGNAL(insertConfigWidget(const KDialogBase*, QWidget*, unsigned int )),
		this, SLOT(insertConfigWidget(const KDialogBase*, QWidget*, unsigned int )) );

	new KAction( i18n("Lookup Current Text"), 0, CTRL+Key_Underscore, this, SLOT(slotLookup()), actionCollection(), "ctags_lookup_shortcut");
	new KAction( i18n("Lookup Current Text as Declaration"), 0, CTRL+Key_Semicolon, this, SLOT(slotLookupDeclaration()), actionCollection(), "ctags_declaration_shortcut");
	new KAction( i18n("Lookup Current Text as Definition"), 0, CTRL+Key_Colon, this, SLOT(slotLookupDefinition()), actionCollection(), "ctags_definition_shortcut");
	new KAction( i18n("Jump to Next Match"), 0, 0, this, SLOT(slotGoToNext()), actionCollection(), "ctags_jump_to_next");
	new KAction( i18n("Open Lookup Dialog"), 0, 0, this, SLOT(slotOpenLookup()), actionCollection(), "ctags_input_shortcut");
}


CTags2Part::~CTags2Part()
{
	if ( m_widget )
	{
		mainWindow()->removeView( m_widget );
	}
	delete m_widget;
	delete _configProxy;
}

void CTags2Part::insertConfigWidget( const KDialogBase * dlg, QWidget * page, unsigned int pagenumber )
{
	if ( pagenumber == CTAGSSETTINGSPAGE )
	{
		CTags2SettingsWidget * w = new CTags2SettingsWidget( this, page );
		connect( dlg, SIGNAL(okClicked()), w, SLOT(slotAccept()) );
		connect( w, SIGNAL(newTagsfileName(const QString& )), this, SLOT(updateTagsfileName(const QString& )) );
	}
}

void CTags2Part::updateTagsfileName( const QString & )
{
	m_widget->updateDBDateLabel();
}

// wrapper for creating a tag file for the current project
bool CTags2Part::createTagsFile()
{
	// check if user specified a custom tag file name
	QDomDocument & dom = *projectDom();
	QString tagsFileCustom = DomUtil::readEntry( dom, "/ctagspart/customTagfilePath" ).stripWhiteSpace();

	return createTagsFile(tagsFileCustom, project()->projectDirectory());
}

// creates a new tag file with the specified name for the specified source directory
bool CTags2Part::createTagsFile(const QString& tagFile, const QString& dir)
{
/*
	KProcess proc;
	proc.setWorkingDirectory( project()->projectDirectory() );

	proc << "ctags";
	proc << "-R" << "--c++-types=+px" << "--excmd=pattern" << "--exclude=Makefile";

	bool success = proc.start(KProcess::Block);

	return success;
*/

	// get name of the ctags binary
	KConfig * config = kapp->config();
	config->setGroup( "CTAGS" );
	QString ctagsBinary = config->readEntry( "ctags binary" ).stripWhiteSpace();
	if ( ctagsBinary.isEmpty() )
	{
		ctagsBinary = "ctags";
	}

	// set a default argument list
	QString argsDefault = "-R --c++-types=+px --excmd=pattern --exclude=Makefile --exclude=.";

	QDomDocument & dom = *projectDom();
	QString argsCustom = DomUtil::readEntry( dom, "/ctagspart/customArguments" ).stripWhiteSpace();
	QString commandline = ctagsBinary + " " + ( argsCustom.isEmpty() ? argsDefault : argsCustom ) + ( tagFile.isEmpty() ? "" : " -f " + tagFile );
	commandline += " ";
	commandline += dir;

	if (KDevAppFrontend *appFrontend = extension<KDevAppFrontend>("KDevelop/AppFrontend"))
		appFrontend->startAppCommand(dir, commandline, false);

	return true;
}

void CTags2Part::contextMenu(QPopupMenu *popup, const Context *context)
{
	if (!context->hasType( Context::EditorContext ))
		return;

	const EditorContext *econtext = static_cast<const EditorContext*>(context);
	QString ident = econtext->currentWord();
	if (ident.isEmpty())
		return;

	KConfig * config = kapp->config();
	config->setGroup( "CTAGS" );
	bool showDeclaration = config->readBoolEntry( "ShowDeclaration", true );
	bool showDefinition = config->readBoolEntry( "ShowDefinition", true );
	bool showLookup = config->readBoolEntry( "ShowLookup", true );

	if ( Tags::hasTag( ident ) && ( showDefinition || showDeclaration || showLookup  ) )
	{
		m_contextString = ident;
	    QString squeezed = KStringHandler::csqueeze(ident, 30);

		popup->insertSeparator();

		if ( showDeclaration )
			popup->insertItem( i18n("CTags - Go to Declaration: %1").arg(squeezed), this, SLOT(slotGotoDeclaration()) );

		if ( showDefinition )
			popup->insertItem( i18n("CTags - Go to Definition: %1").arg(squeezed), this, SLOT(slotGotoDefinition()) );

		if ( showLookup )
			popup->insertItem( i18n("CTags - Lookup: %1").arg(squeezed), this, SLOT(slotGotoTag()) );
	}
}

void CTags2Part::showHits( Tags::TagList const & tags )
{
	m_widget->displayHitsAndClear( tags );

	mainWindow()->raiseView( m_widget );
	m_widget->output_view->setFocus();
}

void CTags2Part::slotGotoTag( )
{
	showHits( Tags::getExactMatches( m_contextString ) );
}

void CTags2Part::gotoTagForTypes( QStringList const & types )
{
	Tags::TagList list = Tags::getMatches( m_contextString, false, types );

	if ( list.count() < 1 ) return;

	KConfig * config = kapp->config();
	config->setGroup("CTAGS");
	bool jumpToFirst = config->readBoolEntry( "JumpToFirst", false );

	if ( list.count() == 1 || jumpToFirst )
	{
		Tags::TagEntry tag = list.first();
		KURL url;
		QString fileWithTagInside;
		// assume relative path to project directory if path does not start with slash
		if (tag.file[0] != '/') {
			fileWithTagInside = project()->projectDirectory() + "/" + tag.file;
		}
		else {
			fileWithTagInside = tag.file;
		}
		url.setPath(fileWithTagInside);
		partController()->editDocument( url, getFileLineFromPattern( url, tag.pattern ) );
		m_widget->displayHitsAndClear( list );
	}
	else
	{
		showHits( list );
	}
}

void CTags2Part::slotGotoDefinition( )
{
	QStringList types;
	types << "S" << "d" << "f" << "t" << "v";
	gotoTagForTypes( types );
}

void CTags2Part::slotGotoDeclaration( )
{
	QStringList types;
	types << "L" << "c" << "e" << "g" << "m" << "n" << "p" << "s" << "u" << "x";
	gotoTagForTypes( types );
}


int CTags2Part::getFileLineFromStream( QTextStream & istream, QString const & pattern )
{
	if ( pattern.isEmpty() ) return -1;

	// ctags interestingly escapes "/", but apparently nothing else. lets revert that
	QString unescaped = pattern;
	unescaped.replace( "\\/", "/" );

	// most of the time, the ctags pattern has the form /^foo$/
	// but this isn't true for some macro definitions
	// where the form is only /^foo/
	// I have no idea if this is a ctags bug or not, but we have to deal with it

	QString reduced, escaped, re_string;
	if ( unescaped.endsWith( "$/" ) )
	{
		reduced = unescaped.mid( 2, unescaped.length() -4 );
		escaped = QRegExp::escape( reduced );
		re_string = QString( "^" + escaped + "$" );
	}
	else
	{
		reduced = unescaped.mid( 2, unescaped.length() -3 );
		escaped = QRegExp::escape( reduced );
		re_string = QString( "^" + escaped );
	}

	QRegExp re( re_string );

	int n = 0;
	while ( !istream.atEnd() )
	{
		if ( re.search( istream.readLine() ) > -1 )
		{
			return n;
		}
		n++;
	}
	return -1;
}

int CTags2Part::getFileLineFromPattern( KURL const & url, QString const & pattern )
{
	// if the file is open - get the line from the editor buffer
	if ( KTextEditor::EditInterface * ei = dynamic_cast<KTextEditor::EditInterface*>( partController()->partForURL( url ) ) )
	{
		QString ibuffer = ei->text();
		QTextStream istream( &ibuffer, IO_ReadOnly );
		return getFileLineFromStream( istream, pattern );
	}
	else // else the file is not open - get the line from the file on disk
	{
		QFile file( url.path() );
		QString buffer;

		if ( file.open( IO_ReadOnly ) )
		{
			QTextStream istream( &file );
			return getFileLineFromStream( istream, pattern );
		}
	}
	return -1;
}

void CTags2Part::slotLookupDeclaration( )
{
 	m_contextString = KDevEditorUtil::currentWord( dynamic_cast<KTextEditor::Document*>( partController()->activePart() ) );
	if ( !m_contextString.isEmpty() )
	{
		slotGotoDeclaration();
	}
}

void CTags2Part::slotLookupDefinition( )
{
 	m_contextString = KDevEditorUtil::currentWord( dynamic_cast<KTextEditor::Document*>( partController()->activePart() ) );
	if ( !m_contextString.isEmpty() )
	{
		slotGotoDefinition();
	}
}

void CTags2Part::slotLookup( )
{
 	m_contextString = KDevEditorUtil::currentWord( dynamic_cast<KTextEditor::Document*>( partController()->activePart() ) );
	if ( !m_contextString.isEmpty() )
	{
		slotGotoTag();
	}
}

void CTags2Part::slotOpenLookup( )
{
	mainWindow()->raiseView( m_widget );
	m_widget->input_edit->setFocus();
}

void CTags2Part::slotGoToNext( )
{
	m_widget->goToNext();
}

#include "ctags2_part.moc"

// kate: space-indent off; indent-width 4; tab-width 4; show-tabs off;
