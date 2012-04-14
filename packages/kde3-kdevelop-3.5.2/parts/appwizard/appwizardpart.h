/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _APPWIZARDPART_H_
#define _APPWIZARDPART_H_

#include <qguardedptr.h>
#include "kdevplugin.h"
#include <qstring.h>
#include <qstringlist.h>
#include <kurl.h>

class AppWizardDialog;


class AppWizardPart : public KDevPlugin
{
    Q_OBJECT

public:
    AppWizardPart( QObject *parent, const char *name, const QStringList & );
    ~AppWizardPart();

    void openFilesAfterGeneration(const KURL::List urlsToOpen);

public slots:
    void openFilesAfterGeneration();

private slots:
    void slotNewProject();
    void slotImportProject();

private:
    KURL::List m_urlsToOpen;
};

#endif
