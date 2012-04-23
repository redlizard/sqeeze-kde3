//
// C++ Implementation: embeddata
//
// Description:
//
//
// Author: Remi Villatel <maxilys@tele2.fr>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "embeddata.h"

void qInitImages_KWinPowder()
{
    if ( !factory ) {
	factory = new MimeSourceFactory_KWinPowder;
	QMimeSourceFactory::defaultFactory()->addFactory( factory );
    }
}

void qCleanupImages_KWinPowder()
{
    if ( factory ) {
	QMimeSourceFactory::defaultFactory()->removeFactory( factory );
	delete factory;
	factory = 0;
    }
}
