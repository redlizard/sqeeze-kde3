/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

/*
  Where it all began ...
  begin:     Die Jan 15 05:59:05 CET 2002
  copyright: (C) 2002 by Dario Abatianni
  email:     eisfuchs@tigress.com
*/

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <dcopclient.h>
#include <dcopref.h>
#include <kdebug.h>

#include "konversationapplication.h"
#include "version.h"
#include "commit.h"

/*
  Don't use i18n() here, use I18N_NOOP() instead!
  i18n() will only work as soon as a kapplication object was made.
*/

static const char* shortDescription=I18N_NOOP("A user friendly IRC client");

static const KCmdLineOptions options[] =
{
    { "+[url]", I18N_NOOP("irc:// URL or server hostname"), 0 },
    { "server <server>", I18N_NOOP("Server to connect"), 0 },
    { "port <port>", I18N_NOOP("Port to use"), "6667"},
    { "channel <channel>", I18N_NOOP("Channel to join after connection"), ""},
    { "nick <nickname>", I18N_NOOP("Nickname to use"),""},
    { "password <password>", I18N_NOOP("Password for connection"),""},
    { "ssl", I18N_NOOP("Use SSL for connection"),"false"},
    KCmdLineLastOption
};

int main(int argc, char* argv[])
{
    KAboutData aboutData("konversation",
        I18N_NOOP("Konversation"),
        KONVI_VERSION,
        shortDescription,
        KAboutData::License_GPL,
        I18N_NOOP("(C) 2002-2008 by the Konversation team"),
        I18N_NOOP("Konversation is a client for the Internet Relay Chat (IRC) protocol.\n"
        "Meet friends on the net, make new acquaintances and lose yourself in\n"
        "talk about your favorite subject."),
        "http://konversation.kde.org/");

    aboutData.addAuthor("Dario Abatianni",I18N_NOOP("Original Author, Project Founder"),"eisfuchs@tigress.com");
    aboutData.addAuthor("Peter Simonsson",I18N_NOOP("Maintainer"),"psn@linux.se");
    aboutData.addAuthor("Eike Hein",I18N_NOOP("Maintainer, Release Manager, User interface, Protocol handling"),"hein@kde.org");
    aboutData.addAuthor("Shintaro Matsuoka",I18N_NOOP("DCC, Encoding handling, OSD positioning"),"shin@shoegazed.org");
    aboutData.addAuthor("Eli MacKenzie",I18N_NOOP("Protocol handling, Input line"),"argonel@gmail.com");
    aboutData.addAuthor("İsmail Dönmez",I18N_NOOP("Blowfish, SSL support, KNetwork port, Colored nicks, Nicklist themes"),"ismail@kde.org");
    aboutData.addAuthor("John Tapsell",I18N_NOOP("Refactoring, KAddressBook/Kontact integration"), "john@geola.co.uk");

    aboutData.addCredit("Olivier Bédard",I18N_NOOP("Website hosting"));
    aboutData.addCredit("Jędrzej Lisowski",I18N_NOOP("Website maintenance"),"yesoos@gmail.com");
    aboutData.addCredit("Christian Muehlhaeuser",I18N_NOOP("Multiple modes extension, Close widget placement, OSD functionality"),"chris@chris.de");
    aboutData.addCredit("Gary Cramblitt",I18N_NOOP("Documentation, Watched nicks online improvements, Custom web browser extension"),"garycramblitt@comcast.net");
    aboutData.addCredit("Matthias Gierlings",I18N_NOOP("Color configurator, Highlight dialog"),"gismore@users.sourceforge.net");
    aboutData.addCredit("Alex Zepeda",I18N_NOOP("DCOP interface"),"garbanzo@hooked.net");
    aboutData.addCredit("Stanislav Karchebny",I18N_NOOP("Non-Latin1-Encodings"),"berkus@users.sourceforge.net");
    aboutData.addCredit("Mickael Marchand",I18N_NOOP("Konsole part view"),"marchand@kde.org");
    aboutData.addCredit("Michael Goettsche",I18N_NOOP("Quick connect, Ported new OSD, other features and bugfixes"),"michael.goettsche@kdemail.net");
    aboutData.addCredit("Benjamin Meyer",I18N_NOOP("A Handful of fixes and code cleanup"),"ben+konversation@meyerhome.net");
    aboutData.addCredit("Jakub Stachowski",I18N_NOOP("Drag&Drop improvements"),"qbast@go2.pl");
    aboutData.addCredit("Sebastian Sariego",I18N_NOOP("Artwork"),"segfault@kde.cl");
    aboutData.addCredit("Renchi Raju",I18N_NOOP("Firefox style searchbar"));
    aboutData.addCredit("Michael Kreitzer",I18N_NOOP("Raw modes, Tab grouping per server, Ban list"),"mrgrim@gr1m.org");
    aboutData.addCredit("Frauke Oster",I18N_NOOP("System tray patch"),"frauke@frsv.de");
    aboutData.addCredit("Lucijan Busch",I18N_NOOP("Bug fixes"),"lucijan@kde.org");
    aboutData.addCredit("Sascha Cunz",I18N_NOOP("Extended user modes patch"),"mail@sacu.de");
    aboutData.addCredit("Steve Wollkind",I18N_NOOP("Close visible tab with shortcut patch"),"steve@njord.org");
    aboutData.addCredit("Thomas Nagy",I18N_NOOP("Cycle tabs with mouse scroll wheel"),"thomas.nagy@eleve.emn.fr");
    aboutData.addCredit("Tobias Olry",I18N_NOOP("Channel ownership mode patch"),"tobias.olry@web.de");
    aboutData.addCredit("Ruud Nabben",I18N_NOOP("Option to enable IRC color filtering"),"r.nabben@gawab.com");
    aboutData.addCredit("Lothar Braun",I18N_NOOP("Bug fixes"),"mail@lobraun.de");
    aboutData.addCredit("Ivor Hewitt",I18N_NOOP("Bug fixes, OSD work, clearing topics"),"ivor@ivor.org");
    aboutData.addCredit("Emil Obermayr",I18N_NOOP("Sysinfo script"),"nobs@tigress.com");
    aboutData.addCredit("Stanislav Nikolov",I18N_NOOP("Bug fixes"),"valsinats@gmail.com");
    aboutData.addCredit("Juan Carlos Torres",I18N_NOOP("Auto-join context menu"),"carlosdgtorres@gmail.com");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineArgs::addCmdLineOptions(options);
    KApplication::addCmdLineOptions();

    if (!KUniqueApplication::start()) return 0;

    KonversationApplication app;

    return app.exec();
}
