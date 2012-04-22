//
// C++ Implementation: embeddata
//
// Description:
//
//
// Author: Remi Villatel <maxilys@tele2.fr>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "embeddata.h"

void qInitImages_KWinSerenity()
{
	if (!factory)
	{
		factory = new MimeSourceFactory_KWinSerenity;
		QMimeSourceFactory::defaultFactory()->addFactory( factory );
	}
}

void qCleanupImages_KWinSerenity()
{
	if (factory)
	{
		QMimeSourceFactory::defaultFactory()->removeFactory( factory );
		delete factory;
		factory = 0;
	}
}
