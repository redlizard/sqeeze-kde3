/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include <qstringlist.h>
#include <qdir.h>
#include <kstandarddirs.h>
#include <kglobal.h>

#include <config.h>
#include LCMS_HEADER

#include <kis_colorspace_factory_registry.h>
#include <kis_math_toolbox.h>
#include <kis_meta_registry.h>

KisMetaRegistry * KisMetaRegistry::m_singleton = 0;

KisMetaRegistry::KisMetaRegistry()
{
    // Create the colorspaces and load the profiles

    KGlobal::instance()->dirs()->addResourceType("kis_profiles",
                                                     KStandardDirs::kde_default("data") + "krita/profiles/");
                          
    // Add those things here as well, since we are not yet using KisDoc's KisFactory instance (which inits these as well)
    KGlobal::instance()->dirs()->addResourceType("kis_profiles", KStandardDirs::kde_default("data") + "krita/profiles/");
    KGlobal::instance()->dirs()->addResourceDir("kis_profiles", "/usr/share/color/icc");
    KGlobal::instance()->dirs()->addResourceDir("kis_profiles", QDir::homeDirPath() + QString("/.icc/"));
    KGlobal::instance()->dirs()->addResourceDir("kis_profiles", QDir::homeDirPath() + QString("/.color/icc/"));

    QStringList profileFilenames;
    profileFilenames += KGlobal::instance()->dirs()->findAllResources("kis_profiles", "*.icm", true /* recursive */);
    profileFilenames += KGlobal::instance()->dirs()->findAllResources("kis_profiles", "*.ICM", true);
    profileFilenames += KGlobal::instance()->dirs()->findAllResources("kis_profiles", "*.ICC", true);
    profileFilenames += KGlobal::instance()->dirs()->findAllResources("kis_profiles", "*.icc", true);
    // Set lcms to return NUll/false etc from failing calls, rather than aborting the app.
    cmsErrorAction(LCMS_ERROR_SHOW);

    m_csRegistry = new KisColorSpaceFactoryRegistry(profileFilenames);
    m_mtRegistry = new KisMathToolboxFactoryRegistry();
}

KisMetaRegistry::~KisMetaRegistry()
{
}

KisMetaRegistry * KisMetaRegistry::instance()
{
    if ( KisMetaRegistry::m_singleton == 0 ) {
        KisMetaRegistry::m_singleton = new KisMetaRegistry();
    }
    return KisMetaRegistry::m_singleton;
}

