/***************************************************************************
*   Copyright (C) 2004-2008 by Thomas Fischer                             *
*   fischer@unix-ag.uni-kl.de                                             *
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
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#include <kkeydialog.h>
#include <kencodingfiledialog.h>
#include <kconfig.h>
#include <kurl.h>
#include <kactionclasses.h>
#include <kedittoolbar.h>
#include <kdebug.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kio/netaccess.h>
#include <klibloader.h>
#include <kmessagebox.h>
#include <kstatusbar.h>
#include <klocale.h>

#include <settings.h>
#include <kbibtex_part.h>
#include "kbibtexshell.h"

// Settings settings;

KBibTeXShell::KBibTeXShell( QWidget* parentWidget, const char* name )
        : KParts::MainWindow( parentWidget, name ), m_part( NULL ), m_parentWidget( parentWidget )
{
    // set the shell's ui resource file
    setXMLFile( "kbibtex_shell.rc" );

    // then, setup our actions
    setupActions();

    // and a status bar
    statusBar() ->show();

    // this routine will find and load our Part.  it finds the Part by
    // name which is a bad idea usually.. but it's alright in this
    // case since our Part is made for this Shell
    KLibFactory *factory = KLibLoader::self() ->factory( "libkbibtexpart" );
    if ( factory )
    {
        // now that the Part is loaded, we cast it to a Part to get
        // our hands on it
        m_part = static_cast<KParts::ReadWritePart *>( factory->create( this, "kbibtex_part", "KParts::ReadWritePart" ) );

        if ( m_part )
        {
            // tell the KParts::MainWindow that this is indeed the main widget
            setCentralWidget( m_part->widget() );

            // and integrate the part's GUI with the shell's
            createGUI( m_part );
        }

    }
    else
    {
        // if we couldn't find our Part, we exit since the Shell by
        // itself can't do anything useful
        KMessageBox::error( this, i18n( "Could not find our part!" ) );
        kapp->quit();
        // we return here, cause kapp->quit() only means "exit the
        // next time we enter the event loop...
        return ;
    }

    // apply the saved mainwindow settings, if any, and ask the mainwindow
    // to automatically save settings if changed: window size, toolbar
    // position, icon size, etc.
    setAutoSaveSettings();

    readConfig();

#ifdef HAVE_LIBBIBUTILS
    kdDebug() << "Using BibUtils for extended bibliography file type support" << endl;
#else // HAVE_LIBBIBUTILS
    kdDebug() << "Library BibUtils not linked to KBibTeX, see http://www.unix-ag.uni-kl.de/~fischer/kbibtex/download.html for more information" << endl;
#endif // HAVE_LIBBIBUTILS
}

KBibTeXShell::~KBibTeXShell()
{
    // nothing
}

bool KBibTeXShell::openURL( const KURL& url )
{
    if ( url.isEmpty() )
        return false;

    // About this function, the style guide (
    // http://developer.kde.org/documentation/standards/kde/style/basics/index.html )
    // says that it should open a new window if the document is _not_
    // in its initial state.  This is what we do here.

    // If there is no part yet created, create new one...

    if ( m_part->url().isEmpty() && !m_part->isModified() )
    {
        if ( !m_part->openURL( url ) )
            return FALSE;
    }
    else
    {
        KBibTeXShell *shell = new KBibTeXShell();

        if ( !shell->part() ->openURL( url ) )
            return FALSE;
        else
            initializePart( shell->part() );
        shell->show();
    }

    m_recentFiles->addURL( url );
    return TRUE;
}

KParts::ReadWritePart * KBibTeXShell::part()
{
    return m_part;
}

void KBibTeXShell::readConfig( KConfig *config )
{
    config->setGroup( "Session" );
    m_recentFiles->loadEntries( config );
    m_recentFiles->setEnabled( true ); // force enabling
    m_recentFiles->setToolTip( i18n( "Click to open a file\nClick and hold to open a recent file" ) );
}

void KBibTeXShell::writeConfig( KConfig *config )
{
    config->setGroup( "Session" );
    m_recentFiles->saveEntries( config );
    config->sync();
}

void KBibTeXShell::readConfig()
{
    KConfig * config = kapp->config();
    readConfig( config );
}

void KBibTeXShell::writeConfig()
{
    KConfig * config = kapp->config();
    writeConfig( config );
}

QString KBibTeXShell::encoding()
{
    return "utf8";
}

void KBibTeXShell::slotAddRecentURL( const KURL&url )
{
    m_recentFiles->addURL( url );
}

void KBibTeXShell::setupActions()
{
    KAction * action;

    action = KStdAction::openNew( this, SLOT( slotFileNew() ), actionCollection() );
    action ->setToolTip( i18n( "Create a new window for a new BibTeX file" ) );
    KAction *actionOpen = KStdAction::open( this, SLOT( slotFileOpen() ), actionCollection() );
    actionOpen->setToolTip( i18n( "Open an existing BibTeX file" ) );

    m_recentFiles = KStdAction::openRecent( this, SLOT( slotFileOpen( const KURL& ) ), actionCollection() );
    m_recentFiles->setWhatsThis( i18n( "This lists files which you have opened recently, and allows you to easily open them again." ) );
    connect( m_recentFiles, SIGNAL( activated() ), actionOpen, SLOT( activate() ) );

    action = KStdAction::close( this, SLOT( slotFileClose() ), actionCollection() );
    action->setToolTip( i18n( "Close this KBibTeX window" ) );
    //     action = KStdAction::quit( kapp, SLOT( closeAllWindows() ), actionCollection() );
    //     action->setToolTip( i18n( "Close all windows and quit KBibTeX" ) );

    m_statusbarAction = KStdAction::showStatusbar( this, SLOT( optionsShowStatusbar() ), actionCollection() );
    m_statusbarAction->setToolTip( i18n( "Show or hide the window's status bar" ) );

    KStdAction::keyBindings( this, SLOT( optionsConfigureKeys() ), actionCollection() ) ->setToolTip( i18n( "Configure key bindings for KBibTeX" ) );
    KStdAction::configureToolbars( this, SLOT( optionsConfigureToolbars() ), actionCollection() ) ->setToolTip( i18n( "Configure the tool bar for KBibTeX" ) );

    connect( actionCollection(), SIGNAL( actionStatusText( const QString & ) ), this, SLOT( slotActionStatusText( const QString & ) ) );
    connect( actionCollection(), SIGNAL( clearStatusText( ) ), statusBar(), SLOT( clear() ) );
}

void KBibTeXShell::slotActionStatusText( const QString &text )
{
    KStatusBar * stb = statusBar();

    if ( stb )
        stb->message( text );
}

void KBibTeXShell::saveProperties( KConfig* /*config*/ )
{
    // the 'config' object points to the session managed
    // config file.  anything you write here will be available
    // later when this app is restored
}

void KBibTeXShell::readProperties( KConfig* /*config*/ )
{
    // the 'config' object points to the session managed
    // config file.  this function is automatically called whenever
    // the app is being restored.  read in here whatever you wrote
    // in 'saveProperties'
}

void KBibTeXShell::optionsShowStatusbar()
{
    // this is all very cut and paste code for showing/hiding the
    // statusbar
    if ( m_statusbarAction->isChecked() )
        statusBar() ->show();
    else
        statusBar() ->hide();
}

void KBibTeXShell::optionsConfigureKeys()
{
    KKeyDialog dlg( false, this );
    QPtrList<KXMLGUIClient> clients = guiFactory()->clients();
    for ( QPtrListIterator<KXMLGUIClient> it( clients ); it.current(); ++it )
    {
        dlg.insert(( *it )->actionCollection() );
    }
    dlg.configure();
}

void KBibTeXShell::optionsConfigureToolbars()
{
#if defined(KDE_MAKE_VERSION)
# if KDE_VERSION >= KDE_MAKE_VERSION(3,1,0)
    saveMainWindowSettings( KGlobal::config(), autoSaveGroup() );
# else
    saveMainWindowSettings( KGlobal::config() );
# endif
#else
    saveMainWindowSettings( KGlobal::config() );
#endif

    // use the standard toolbar editor
    KEditToolbar dlg( factory() );
    connect( &dlg, SIGNAL( newToolbarConfig() ),
             this, SLOT( applyNewToolbarConfig() ) );
    dlg.exec();
}

void KBibTeXShell::applyNewToolbarConfig()
{
#if defined(KDE_MAKE_VERSION)
# if KDE_VERSION >= KDE_MAKE_VERSION(3,1,0)
    applyMainWindowSettings( KGlobal::config(), autoSaveGroup() );
# else
    applyMainWindowSettings( KGlobal::config() );
# endif
#else
    applyMainWindowSettings( KGlobal::config() );
#endif
}

bool KBibTeXShell::queryClose()
{
    if ( m_part && !m_part ->closeURL() )
        return FALSE;

    writeConfig();

    return KParts::MainWindow::queryClose();
}

void KBibTeXShell::slotFileNew()
{
    // this slot is called whenever the File->New menu is selected,
    // the New shortcut is pressed (usually CTRL+N) or the New toolbar
    // button is clicked

    // About this function, the style guide (
    // http://developer.kde.org/documentation/standards/kde/style/basics/index.html )
    // says that it should open a new window if the document is _not_
    // in its initial state.  This is what we do here..
    if ( ! m_part->url().isEmpty() || m_part->isModified() )
    {
        ( new KBibTeXShell() ) ->show();
    };
}

/**
 * Show a file open dialog where a user can select one or several
 * files for opening.
 */
void KBibTeXShell::slotFileOpen()
{
    QString startDir = ! m_part->url().isEmpty() ? m_part->url().url() : QDir::currentDirPath();
    KURL mergeURL = KFileDialog::getOpenURL( startDir, QString( "*.bib *.ris" ) +
#ifdef HAVE_LIBBIBUTILS
                    " *.ref *.refer *.rfr *.txt *.isi *.cgi" +
#endif
                    "|"    + i18n( "Supported Bibliographies" ) + "\n*.bib|" + i18n( "BibTeX (*.bib)" ) + "\n*.ris|" + i18n( "Reference Manager (*.ris)" )
#ifdef HAVE_LIBBIBUTILS
                    + "\n*.ref *.refer *.rfr *.txt|" + i18n( "EndNote (Refer format) (*.ref *.refer *.rfr *.txt)" ) + "\n*.isi *.cgi|" + i18n( "ISI Web of Knowledge (*.isi *.cgi)" )
#endif
                    , widget() );
    slotFileOpen( mergeURL );
}


/**
 * Open a given url in a new window.
 */
void KBibTeXShell::slotFileOpen( const KURL& url )
{
    if ( url.isEmpty() ) return ;

    if ( !KIO::NetAccess::exists( url, TRUE, this ) )
    {
        m_recentFiles->removeURL( url );
        KMessageBox::error( this, i18n( "The given file could not be read, check if it exists or if it is readable for the current user." ) );
        return ;
    }

    if ( m_part->url().isEmpty() && !m_part->isModified() )
        // we open the file in this window...
        openURL( url );
    else
    {
        // we open the file in a new window...
        KBibTeXShell * shell = new KBibTeXShell();
        if ( shell->openURL( url ) )
        {
            initializePart( shell->part() );
            shell->show();
        }
        else
        {
            KMessageBox::error( this, i18n( "Could not open file '%1'." ).arg( url.path() ) );
        }
    }

}

/**
 * Close currrent window
 */
void KBibTeXShell::slotFileClose()
{
    close();
}

void KBibTeXShell::initializePart( KParts::ReadWritePart* part )
{
    if ( part )
    {
        connect( static_cast<KBibTeXPart*>( part ), SIGNAL( signalAddRecentURL( const KURL & ) ), this, SLOT( slotAddRecentURL( const KURL& ) ) );
    }
}

#include "kbibtexshell.moc"
