/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Son Aug 10 13:26:57 EDT 2003
    copyright            : (C) 2003 by Robert Vogl
    email                : voglrobe@saphir
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h> 

// QT includes

// KDE includes
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kglobal.h>
#include <klocale.h>
#include <arts/kartsdispatcher.h>
#include <dcopclient.h>

// App specific includes
#include "version.h"
#include "ksayit.h"

static const char *description =
	I18N_NOOP("KSayIt - A Text To Speech frontend for KDE");
// INSERT A DESCRIPTION FOR YOUR APPLICATION HERE


int main(int argc, char *argv[])
{

  KAboutData aboutData( "ksayit", I18N_NOOP("KSayIt"),
    KSAYIT_VERSION, description, KAboutData::License_GPL,
    "(c) 1996 - 2005, Robert Vogl", 0, 0, "voglrobe@web.de");
  aboutData.addAuthor("Robert Vogl",0, "voglrobe@web.de");
  KCmdLineArgs::init( argc, argv, &aboutData );

  KApplication app;

    // setup MCOP
    KArtsDispatcher dispatcher;
 
    // setup DCOP
    QCString appID = app.dcopClient()->registerAs( app.name(), false );
    
    KSayItApp *ksayit = new KSayItApp(0, "MainWindow", 0, appID);    
    // ksayit->hide();
    ksayit->show();
    
    return app.exec();
}
