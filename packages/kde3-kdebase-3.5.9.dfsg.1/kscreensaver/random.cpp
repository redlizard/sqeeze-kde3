//-----------------------------------------------------------------------------
//
// Screen savers for KDE
//
// Copyright (c)  Martin R. Jones 1999
//
// This is an extremely simple program that starts a random screensaver.
//

#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <qtextstream.h>
#include <qlayout.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qwidget.h>

#include <kapplication.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdesktopfile.h>
#include <krandomsequence.h>
#include <kdebug.h>
#include <kcmdlineargs.h>
#include <kdialogbase.h>
#include <kconfig.h>

#include "kscreensaver_vroot.h"
#include "random.h"

#define MAX_ARGS    20

void usage(char *name)
{
	puts(i18n("Usage: %1 [-setup] [args]\n"
				"Starts a random screen saver.\n"
				"Any arguments (except -setup) are passed on to the screen saver.").arg( name ).local8Bit().data());
}

static const char appName[] = "random";

static const char description[] = I18N_NOOP("Start a random KDE screen saver");

static const char version[] = "2.0.0";

static const KCmdLineOptions options[] =
{
	{ "setup", I18N_NOOP("Setup screen saver"), 0 },
	{ "window-id wid", I18N_NOOP("Run in the specified XWindow"), 0 },
	{ "root", I18N_NOOP("Run in the root XWindow"), 0 },
	//  { "+-- [options]", I18N_NOOP("Options to pass to the screen saver"), 0 }
	KCmdLineLastOption
};

//----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
	KLocale::setMainCatalogue("kscreensaver");
	KCmdLineArgs::init(argc, argv, appName, I18N_NOOP("Random screen saver"), description, version);

	KCmdLineArgs::addCmdLineOptions(options);

	KApplication app;

	Window windowId = 0;

	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	if (args->isSet("setup"))
	{
		KRandomSetup setup;
		setup.exec();
		exit(0);
	}

	if (args->isSet("window-id"))
	{
		windowId = atol(args->getOption("window-id"));
	}

	if (args->isSet("root"))
	{
		windowId = RootWindow(qt_xdisplay(), qt_xscreen());
	}

	KGlobal::dirs()->addResourceType("scrsav",
			KGlobal::dirs()->kde_default("apps") +
			"apps/ScreenSavers/");
	KGlobal::dirs()->addResourceType("scrsav",
			KGlobal::dirs()->kde_default("apps") +
			"System/ScreenSavers/");
	QStringList tempSaverFileList = KGlobal::dirs()->findAllResources("scrsav",
			"*.desktop", false, true);

	QStringList saverFileList;

	KConfig type("krandom.kssrc");
	type.setGroup("Settings");
	bool opengl = type.readBoolEntry("OpenGL");
	bool manipulatescreen = type.readBoolEntry("ManipulateScreen");
        bool fortune = !KStandardDirs::findExe("fortune").isEmpty();

	for (uint i = 0; i < tempSaverFileList.count(); i++)
	{
		kdDebug() << "Looking at " << tempSaverFileList[i] << endl;
		KDesktopFile saver(tempSaverFileList[i], true);
		if(!saver.tryExec())
			continue;
		kdDebug() << "read X-KDE-Type" << endl;
		QString saverType = saver.readEntry("X-KDE-Type");

		if (saverType.isEmpty()) // no X-KDE-Type defined so must be OK
		{
			saverFileList.append(tempSaverFileList[i]);
		}
		else
		{
			QStringList saverTypes = QStringList::split(";", saverType);
			for (QStringList::ConstIterator it =  saverTypes.begin(); it != saverTypes.end(); ++it )
			{
				kdDebug() << "saverTypes is "<< *it << endl;
				if (*it == "ManipulateScreen")
				{
					if (manipulatescreen)
					{
						saverFileList.append(tempSaverFileList[i]);
					}
				}
				else
				if (*it == "OpenGL")
				{
					if (opengl)
					{
						saverFileList.append(tempSaverFileList[i]);
					}
				}
				if (*it == "Fortune")
				{
					if (fortune)
					{
						saverFileList.append(tempSaverFileList[i]);
					}
				}

			}
		}
	}

	KRandomSequence rnd;
	int indx = rnd.getLong(saverFileList.count());
	QString filename = *(saverFileList.at(indx));

	KDesktopFile config(filename, true);

	QString cmd;
	if (windowId && config.hasActionGroup("InWindow"))
	{
		config.setActionGroup("InWindow");
	}
	else if ((windowId == 0) && config.hasActionGroup("Root"))
	{
		config.setActionGroup("Root");
	}
	cmd = config.readPathEntry("Exec");

	QTextStream ts(&cmd, IO_ReadOnly);
	QString word;
	ts >> word;
	QString exeFile = KStandardDirs::findExe(word);

	if (!exeFile.isEmpty())
	{
		char *sargs[MAX_ARGS];
		sargs[0] = new char [strlen(word.ascii())+1];
		strcpy(sargs[0], word.ascii());

		int i = 1;
		while (!ts.atEnd() && i < MAX_ARGS-1)
		{
			ts >> word;
			if (word == "%w")
			{
				word = word.setNum(windowId);
			}

			sargs[i] = new char [strlen(word.ascii())+1];
			strcpy(sargs[i], word.ascii());
			kdDebug() << "word is " << word.ascii() << endl;

			i++;
		}

		sargs[i] = 0;

		execv(exeFile.ascii(), sargs);
	}

	// If we end up here then we couldn't start a saver.
	// If we have been supplied a window id or root window then blank it.
	Window win = windowId ? windowId : RootWindow(qt_xdisplay(), qt_xscreen());
	XSetWindowBackground(qt_xdisplay(), win,
			BlackPixel(qt_xdisplay(), qt_xscreen()));
	XClearWindow(qt_xdisplay(), win);
}


KRandomSetup::KRandomSetup( QWidget *parent, const char *name )
	: KDialogBase( parent, name, true, i18n( "Setup Random Screen Saver" ),
			Ok|Cancel, Ok, true )
{

	QFrame *main = makeMainWidget();
	QGridLayout *grid = new QGridLayout(main, 4, 2, 0, spacingHint() );

	openGL = new QCheckBox( i18n("Use OpenGL screen savers"), main );
	grid->addWidget(openGL, 0, 0);

	manipulateScreen = new QCheckBox(i18n("Use screen savers that manipulate the screen"), main);
	grid->addWidget(manipulateScreen, 1, 0);

	setMinimumSize( sizeHint() );

	KConfig config("krandom.kssrc");
	config.setGroup("Settings");
	openGL->setChecked(config.readBoolEntry("OpenGL", true));
	manipulateScreen->setChecked(config.readBoolEntry("ManipulateScreen", true));
}

void KRandomSetup::slotOk()
{
	KConfig config("krandom.kssrc");
	config.setGroup("Settings");
	config.writeEntry("OpenGL", openGL->isChecked());
	config.writeEntry("ManipulateScreen", manipulateScreen->isChecked());

	accept();
}

#include "random.moc"
