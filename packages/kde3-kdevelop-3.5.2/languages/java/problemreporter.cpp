/*
   Copyright (C) 2002 by Roberto Raggi <roberto@kdevelop.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   version 2, License as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "problemreporter.h"
#include "javasupportpart.h"
#include "configproblemreporter.h"
#include "backgroundparser.h"

#include <kdevpartcontroller.h>
#include <kdevmainwindow.h>

#include <kdeversion.h>
#include <kparts/part.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/document.h>
#include <ktexteditor/markinterface.h>

#include <ktexteditor/markinterfaceextension.h>
#include <ktexteditor/view.h>

#include <kdebug.h>
#include <klocale.h>
#include <kstatusbar.h>
#include <kurl.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kdialogbase.h>

#include <kconfig.h>

#include <qtimer.h>
#include <qregexp.h>
#include <qvbox.h>
#include <qfileinfo.h>
#include <qwhatsthis.h>
#include <qgroupbox.h>


class ProblemItem: public KListViewItem
{
public:
    ProblemItem( QListView* parent, const QString& level, const QString& problem,
		 const QString& file, const QString& line, const QString& column  )
	: KListViewItem( parent, level, problem, file, line, column ) {}

    ProblemItem( QListViewItem* parent, const QString& level, const QString& problem,
		 const QString& file, const QString& line, const QString& column  )
	: KListViewItem( parent, level, problem, file, line, column ) {}

    int compare( QListViewItem* item, int column, bool ascending ) const {
	if( column == 2 || column == 3 ){
	    int a = text( column ).toInt();
	    int b = item->text( column ).toInt();
	    if( a == b )
		return 0;
	    return( a > b ? 1 : -1 );
	}
	return KListViewItem::compare( item, column, ascending );
    }

};

ProblemReporter::ProblemReporter( JavaSupportPart* part, QWidget* parent, const char* name )
    : KListView( parent, name ? name : "problemreporter" ),
      m_javaSupport( part ),
      m_document( 0 ),
      m_markIface( 0 )
{
    QWhatsThis::add(this, i18n("<b>Problem reporter</b><p>This window shows various \"problems\" in your project. "
        "It displays TODO entries, FIXME's and errors reported by a language parser. "
        "To add a TODO or FIXME entry, just type<br>"
        "<tt>//@todo my todo</tt><br>"
        "<tt>//TODO: my todo</tt><br>"
        "<tt>//FIXME fix this</tt>"));

    addColumn( i18n("Level") );
    addColumn( i18n("File") );
    addColumn( i18n("Line") );
    addColumn( i18n("Column") );
    addColumn( i18n("Problem") );
    setAllColumnsShowFocus( TRUE );

    m_timer = new QTimer( this );

    connect( part->partController(), SIGNAL(activePartChanged(KParts::Part*)),
             this, SLOT(slotActivePartChanged(KParts::Part*)) );
    connect( part->partController(), SIGNAL(partAdded(KParts::Part*)),
             this, SLOT(slotPartAdded(KParts::Part*)) );
    connect( part->partController(), SIGNAL(partRemoved(KParts::Part*)),
             this, SLOT(slotPartRemoved(KParts::Part*)) );

    connect( m_timer, SIGNAL(timeout()), this, SLOT(reparse()) );

    connect( this, SIGNAL(executed(QListViewItem*)),
             this, SLOT(slotSelected(QListViewItem*)) );

    configure();
}

ProblemReporter::~ProblemReporter()
{
}

void ProblemReporter::slotActivePartChanged( KParts::Part* part )
{
    if( !part )
	return;

    m_timer->stop();

    if( m_document )
	disconnect( m_document, 0, this, 0 );

    m_document = dynamic_cast<KTextEditor::Document*>( part );
    m_markIface = 0;

    if( !m_document )
        return;

    m_fileName = m_document->url().path();

    if( !m_javaSupport->isValidSource(m_fileName) )
        return;

    connect( m_document, SIGNAL(textChanged()), this, SLOT(slotTextChanged()) );
    m_markIface = dynamic_cast<KTextEditor::MarkInterface*>( part );

    if( !m_javaSupport->backgroundParser() )
        return;

    m_javaSupport->backgroundParser()->lock();
    bool needReparse = false;
    if( !m_javaSupport->backgroundParser()->translationUnit(m_fileName) )
        needReparse = true;
    m_javaSupport->backgroundParser()->unlock();

    if( needReparse )
        reparse();
}

void ProblemReporter::slotTextChanged()
{
    if( m_active )
        m_timer->changeInterval( m_delay );
}

void ProblemReporter::removeAllProblems( const QString& filename )
{
    QListViewItem* current = firstChild();
    while( current ){
	QListViewItem* i = current;
	current = current->nextSibling();

	if( i->text(1) == filename )
	    delete( i );
    }

    if( m_document && m_markIface ){
	QPtrList<KTextEditor::Mark> marks = m_markIface->marks();
	QPtrListIterator<KTextEditor::Mark> it( marks );
	while( it.current() ){
	    m_markIface->removeMark( it.current()->line, KTextEditor::MarkInterface::markType07 );
	    ++it;
	}
    }
}

void ProblemReporter::reparse()
{
    if( !m_javaSupport->isValid() )
	return;

    // @todo use the project database to decide which files to parse instead of this!
    // ugly hack: do not parse non .java ending files
    if ( !m_fileName.endsWith(".java") )
        return;

    m_timer->stop();

    kdDebug(9013) << "ProblemReporter::reparse()" << endl;
    m_javaSupport->backgroundParser()->addFile( m_fileName );
    kdDebug(9013) << "---> file added " << m_fileName << endl;
}

void ProblemReporter::slotSelected( QListViewItem* item )
{
    KURL url( item->text(1) );
    int line = item->text( 2 ).toInt();
    // int column = item->text( 3 ).toInt();
    m_javaSupport->partController()->editDocument( url, line-1 );
//    m_javaSupport->mainWindow()->lowerView( this );
}

void ProblemReporter::reportProblem( const QString& fileName, const Problem& p )
{
    int markType = levelToMarkType( p.level() );
    if( markType != -1 && m_document && m_markIface && m_fileName == fileName ){
	m_markIface->addMark( p.line(), markType );
    }

    QString msg = p.text();
    msg = msg.replace( QRegExp("\n"), "" );

    new ProblemItem( this,
		     levelToString( p.level() ),
		     fileName,
		     QString::number( p.line() + 1 ),
		     QString::number( p.column() + 1 ),
		     msg );
}

void ProblemReporter::configure()
{
    kdDebug(9013) << "ProblemReporter::configure()" << endl;
    KConfig* config = kapp->config();
    config->setGroup( "General Options" );
    m_active = config->readBoolEntry( "EnableJavaBgParser", TRUE );
    m_delay = config->readNumEntry( "BgParserDelay", 500 );
}

void ProblemReporter::configWidget( KDialogBase* dlg )
{
    QVBox *vbox = dlg->addVBoxPage(i18n("Java Parsing"));
    ConfigureProblemReporter* w = new ConfigureProblemReporter( vbox );
    //FIXME adymo: unused functionality
    w->groupBox3->hide();
    connect(dlg, SIGNAL(okClicked()), w, SLOT(accept()));
    connect(dlg, SIGNAL(okClicked()), this, SLOT(configure()));
}

void ProblemReporter::slotPartAdded( KParts::Part* part )
{
    KTextEditor::MarkInterfaceExtension* iface = dynamic_cast<KTextEditor::MarkInterfaceExtension*>( part );

    if( !iface )
	return;

    iface->setPixmap( KTextEditor::MarkInterface::markType07, SmallIcon("stop") );
}

void ProblemReporter::slotPartRemoved( KParts::Part* part )
{
    kdDebug(9013) << "ProblemReporter::slotPartRemoved()" << endl;
    if( part == m_document ){
	m_document = 0;
	m_timer->stop();
    }
}

QString ProblemReporter::levelToString( int level ) const
{
    switch( level )
    {
    case Problem::Level_Error:
	return QString::fromLatin1( "Error" );
    case Problem::Level_Warning:
	return QString::fromLatin1( "Warning" );
    case Problem::Level_Todo:
	return QString::fromLatin1( "Todo" );
    case Problem::Level_Fixme:
	return QString::fromLatin1( "Fixme" );
    default:
        return QString::null;
    }
}

int ProblemReporter::levelToMarkType( int level ) const
{
    switch( level )
    {
    case Problem::Level_Error:
	return KTextEditor::MarkInterface::markType07;
    case Problem::Level_Warning:
        return -1;
    case Problem::Level_Todo:
        return -1;
    case Problem::Level_Fixme:
        return -1;
    default:
        return -1;
    }
}

#include "problemreporter.moc"
