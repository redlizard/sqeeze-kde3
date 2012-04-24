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

// #define UNIQUEAPP 1

#include <qframe.h>
#include <qpixmap.h>
#ifdef UNIQUEAPP
#include <kuniqueapplication.h>
#else // UNIQUEAPP
#include <kapplication.h>
#endif // UNIQUEAPP
#include <kmessagebox.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include "kbibtexshell.h"

static const char description[] =
    I18N_NOOP( "A BibTeX editor for KDE" );

static const char version[] = "0.2.1";

static KCmdLineOptions options[] =
{
    { "+[URL]", I18N_NOOP( "Document to open." ), 0
    },
    KCmdLineLastOption
};

#ifdef UNIQUEAPP
class KBibTeXApplication: public KUniqueApplication
#else // UNIQUEAPP
class KBibTeXApplication: public KApplication
#endif // UNIQUEAPP
{
public:
#ifdef UNIQUEAPP
    KBibTeXApplication() : KUniqueApplication()
    {
// nothing
    }

    int newInstance()
    {
#else // UNIQUEAPP
    KBibTeXApplication() : KApplication()
    {
#endif // UNIQUEAPP
        // see if we are starting with session management
        if ( isRestored() )
        {
            RESTORE( KBibTeXShell );
        }
        else
        {
            // no session.. just start up normally
            KCmdLineArgs * args = KCmdLineArgs::parsedArgs();

            if ( args->count() == 0 )
            {
                KBibTeXShell * shell = new KBibTeXShell();
                shell->show();
            }
            else
            {
                for ( int i = 0 ; i < args->count(); i++ )
                {
                    KBibTeXShell *shell = new KBibTeXShell();
                    if ( shell->openURL( args->url( i ) ) )
                        shell->show();
                    else
                    {
                        kdDebug() << "Couldn't open file " << args->url( i ).path() << endl;
                        KMessageBox::error( NULL, i18n( "Could not open file '%1'." ).arg( args->url( i ).path() ) );
                    }
                }
            }
            args->clear();
        }

#ifdef UNIQUEAPP
        return 0;
#endif // UNIQUEAPP

    }
};

int main( int argc, char **argv )
{
    KAboutData about( "kbibtex", I18N_NOOP( "KBibTeX" ), version, description,
                      KAboutData::License_GPL, "(C) 2004-2008 Thomas Fischer", 0, "http://www.unix-ag.uni-kl.de/~fischer/kbibtex/", "fischer@unix-ag.uni-kl.de" );
    about.addAuthor( "Thomas Fischer", 0, "fischer@unix-ag.uni-kl.de" );
    about.setTranslator( I18N_NOOP( "NAME OF TRANSLATORS" ), I18N_NOOP( "EMAIL OF TRANSLATORS" ) );
    KCmdLineArgs::init( argc, argv, &about );
    KCmdLineArgs::addCmdLineOptions( options );

#ifdef UNIQUEAPP
    if ( !KUniqueApplication::start() )
    {
        kdDebug() << "Reusing existing KBibTeX instance" << endl;
        return 0;
    }
#endif // UNIQUEAPP

    KBibTeXApplication app;
    return app.exec();
}
