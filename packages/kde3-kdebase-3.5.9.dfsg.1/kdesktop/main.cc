/* This file is part of the KDE project
   Copyright (C) 1999 David Faure (maintainer)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <config.h>
#include <kuniqueapplication.h>
#include <klocale.h>
#include <dcopclient.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kopenwith.h>
#include <kcrash.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kmanagerselection.h>

#include "desktop.h"
#include "lockeng.h"
#include "init.h"
#include "krootwm.h"
#include "kdesktopsettings.h"

#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

static const char description[] =
        I18N_NOOP("The KDE desktop");

static const char version[] = VERSION;

static KCmdLineOptions options[] =
{
   { "x-root", I18N_NOOP("Use this if the desktop window appears as a real window"), 0 },
   { "noautostart", I18N_NOOP("Obsolete"), 0 },
   { "waitforkded", I18N_NOOP("Wait for kded to finish building database"), 0 },
   KCmdLineLastOption
};

// -----------------------------------------------------------------------------

int kdesktop_screen_number = 0;
QCString kdesktop_name, kicker_name, kwin_name;

static void crashHandler(int sigId)
{
    DCOPClient::emergencyClose(); // unregister DCOP
    sleep( 1 );
    system("kdesktop &"); // try to restart
    fprintf(stderr, "*** kdesktop (%ld) got signal %d\n", (long) getpid(), sigId);
    ::exit(1);
}

static void signalHandler(int sigId)
{
    fprintf(stderr, "*** kdesktop got signal %d (Exiting)\n", sigId);
    KCrash::setEmergencySaveFunction(0); // No restarts any more
    // try to cleanup all windows
    signal(SIGTERM, SIG_DFL); // next one kills
    signal(SIGHUP,  SIG_DFL); // next one kills
    if (kapp)
        kapp->quit(); // turn catchable signals into clean shutdown
}

void KDesktop::slotUpAndRunning()
{
    // Activate crash recovery
    if (getenv("KDE_DEBUG") == NULL)
        KCrash::setEmergencySaveFunction(crashHandler); // Try to restart on crash
}

extern "C" KDE_EXPORT int kdemain( int argc, char **argv )
{
    //setup signal handling
    signal(SIGTERM, signalHandler);
    signal(SIGHUP,  signalHandler);

    {
        if (KGlobalSettings::isMultiHead())
        {
       	    Display *dpy = XOpenDisplay(NULL);
	    if (! dpy) {
		fprintf(stderr,
			"%s: FATAL ERROR: couldn't open display '%s'\n",
			argv[0], XDisplayName(NULL));
		exit(1);
	    }

	    int number_of_screens = ScreenCount(dpy);
	    kdesktop_screen_number = DefaultScreen(dpy);
	    int pos;
	    QCString display_name = XDisplayString(dpy);
	    XCloseDisplay(dpy);
	    dpy = 0;

	    if ((pos = display_name.findRev('.')) != -1)
		display_name.remove(pos, 10);

            QCString env;
	    if (number_of_screens != 1) {
		for (int i = 0; i < number_of_screens; i++) {
		    if (i != kdesktop_screen_number && fork() == 0) {
			kdesktop_screen_number = i;
			// break here because we are the child process, we don't
			// want to fork() anymore
			break;
		    }
		}

		env.sprintf("DISPLAY=%s.%d", display_name.data(),
			    kdesktop_screen_number);

		if (putenv(strdup(env.data()))) {
		    fprintf(stderr,
			    "%s: WARNING: unable to set DISPLAY environment variable\n",
			    argv[0]);
		    perror("putenv()");
		}
	    }
	}
    }

    KGlobal::locale()->setMainCatalogue("kdesktop");

    if (kdesktop_screen_number == 0) {
        kdesktop_name = "kdesktop";
        kicker_name = "kicker";
        kwin_name = "kwin";
    } else {
        kdesktop_name.sprintf("kdesktop-screen-%d", kdesktop_screen_number);
        kicker_name.sprintf("kicker-screen-%d", kdesktop_screen_number);
        kwin_name.sprintf("kwin-screen-%d", kdesktop_screen_number);
    }

    KAboutData aboutData( kdesktop_name, I18N_NOOP("KDesktop"),
			  version, description, KAboutData::License_GPL,
			  "(c) 1998-2000, The KDesktop Authors");
    aboutData.addAuthor("David Faure", 0, "faure@kde.org");
    aboutData.addAuthor("Martin Koller", 0, "m.koller@surfeu.at");
    aboutData.addAuthor("Waldo Bastian", 0, "bastian@kde.org");
    aboutData.addAuthor("Luboš Luňák", 0, "l.lunak@kde.org");
    aboutData.addAuthor("Joseph Wenninger", 0, "kde@jowenn.at");
    aboutData.addAuthor("Tim Jansen", 0, "tim@tjansen.de");
    aboutData.addAuthor("Benoit Walter", 0, "b.walter@free.fr");
    aboutData.addAuthor("Torben Weis", 0, "weis@kde.org");
    aboutData.addAuthor("Matthias Ettrich", 0, "ettrich@kde.org");

    KCmdLineArgs::init( argc, argv, &aboutData );
    KCmdLineArgs::addCmdLineOptions( options );

    if (!KUniqueApplication::start()) {
        fprintf(stderr, "kdesktop is already running!\n");
        exit(0);
    }
    DCOPClient* cl = new DCOPClient;
    cl->attach();
    DCOPRef r( "ksmserver", "ksmserver" );
    r.setDCOPClient( cl );
    r.send( "suspendStartup", QCString( "kdesktop" ));
    delete cl;
    KUniqueApplication app;
    app.disableSessionManagement(); // Do SM, but don't restart.

    KDesktopSettings::instance(kdesktop_name + "rc");
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    bool x_root_hack = args->isSet("x-root");
    bool wait_for_kded = args->isSet("waitforkded");

    // This MUST be created before any widgets are created
    SaverEngine saver;

    // Do this before forking so that if a dialog box appears it won't
    // be covered by other apps.
    // And before creating KDesktop too, of course.
    testLocalInstallation();

    // Mark kdeskop as immutable if all of its config modules have been disabled
    if (!app.config()->isImmutable() && 
        kapp->authorizeControlModules(KRootWm::configModules()).isEmpty())
    {
       app.config()->setReadOnly(true);
       app.config()->reparseConfiguration();
    }

    // for the KDE-already-running check in startkde
    KSelectionOwner kde_running( "_KDE_RUNNING", 0 );
    kde_running.claim( false );

    KDesktop desktop( x_root_hack, wait_for_kded );

    args->clear();

    app.dcopClient()->setDefaultObject( "KDesktopIface" );

    return app.exec();
}
