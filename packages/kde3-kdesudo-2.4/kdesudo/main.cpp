/***************************************************************************
                          kdesudo.cpp  -  description
                             -------------------
    begin                : Sam Feb 15 15:42:12 CET 2003
    copyright            : (C) 2003 by Robert Gruber <rgruber@users.sourceforge.net>
                           (C) 2007 by Martin Böhm <martin.bohm@kubuntu.org>
                                       Anthony Mercatante <tonio@kubuntu.org>
                                       Canonical Ltd (Jonathan Riddell <jriddell@ubuntu.com>)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>

#include "kdesudo.h"
#include <kmessagebox.h>
#include <kdesktopfile.h>
#include <kiconloader.h>
#include <kicontheme.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kprocess.h>
#include <kdebug.h>
#include <qfile.h>
#include <qfileinfo.h>

static const char *description =
	I18N_NOOP("KdeSudo");
// INSERT A DESCRIPTION FOR YOUR APPLICATION HERE
	
	
static KCmdLineOptions options[] =
{
	{ "u <runas>", I18N_NOOP("sets a runas user"), 0 },
	{ "c <command>", I18N_NOOP("The command to execute"), 0 },
	{ "s", I18N_NOOP("Forget passwords"), 0 },
	{ "i <icon name>", I18N_NOOP("Specify icon to use in the password dialog"), 0},
	{ "d", I18N_NOOP("Do not show the command to be run in the dialog"), 0},
	{ "p <priority>", I18N_NOOP("Process priority, between 0 and 100, 0 the lowest [50]"), 0},
	{ "r", I18N_NOOP("Use realtime scheduling"), 0},
	{ "f <file>", I18N_NOOP("Use target UID if <file> is not writeable"), 0},
	{ "t", I18N_NOOP("Fake option for KDE's KdeSu compatibility"), 0 },
	{ "n", I18N_NOOP("Do not keep password"), 0},
	{ "nonewdcop", I18N_NOOP("Use existing DCOP server"), 0},
	{ "comment <dialog text>", I18N_NOOP("The comment that should be displayed in the dialog"),  0},
	{ "noignorebutton", I18N_NOOP("Do not display « ignore » button"), 0 },
	{ "+command", I18N_NOOP("The command to execute"), 0 },
	KCmdLineLastOption
};

int main(int argc, char **argv)
{
	KAboutData aboutData("kdesudo", I18N_NOOP("KdeSudo"),
		VERSION, description, KAboutData::License_GPL,
		"(c) 2007-2008, Anthony Mercatante", 0, 0, "tonio@ubuntu.com");
	aboutData.addAuthor("Robert Gruber",0, "rgruber@users.sourceforge.net");
	aboutData.addAuthor("Anthony Mercatante",0, "tonio@ubuntu.com");
	KCmdLineArgs::init(argc, argv, &aboutData);
	KCmdLineArgs::addCmdLineOptions(options); // Add our own options.

	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	KApplication a;

	QString strCmd;
	QString executable;
	QString arg;
	QString command;
	QStringList commandlist;
	QString icon;

	bool withIgnoreButton = args->isSet("ignorebutton");

	if (args->count())
	{
		if (executable.isEmpty())
		{
			command = args->arg(0);
			commandlist = QStringList::split(QChar(' '), command);
			executable = commandlist[0];
		}
	}

	/* Kubuntu has a bug in it - this is a workaround for it */  
	KGlobal::dirs()->addResourceDir("apps","/usr/share/applications/kde");
	KGlobal::dirs()->addResourceDir("apps","/usr/share/applications");
	QString deskFilePath = KGlobal::dirs()->findResource("apps",executable + ".desktop");
	KDesktopFile desktopFile(deskFilePath,true);

	/* icon parsing */
	if (args->isSet("i"))
		icon = args->getOption("i");
	else
	{
		QString iconName = desktopFile.readIcon();
		icon = KGlobal::iconLoader()->iconPath(iconName, -1* KIcon::StdSizes(KIcon::SizeHuge), true);
	}

	/* generic name parsing */
	QString name = desktopFile.readName();
	QString genericName = desktopFile.readGenericName();

	if (!name.isEmpty())
	{
		if (!genericName.isEmpty())
		name += " - " + genericName;
	}
	else if (!genericName.isEmpty())
		name = genericName;

	KdeSudo *kdesudo = new KdeSudo(0,0,icon,name,withIgnoreButton);

	a.setMainWidget(kdesudo);
	args->clear();

	return a.exec();
}
