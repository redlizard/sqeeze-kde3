/* This file is part of the KDE project
   Copyright (C) 2004 Alexander Dymo <adymo@kdevelop.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#include "kdevplugincontroller.h"

#include "kdevplugin.h"

KDevPluginController::KDevPluginController()
{
}

KTrader::OfferList KDevPluginController::query(const QString &serviceType,
    const QString &constraint)
{
    return KTrader::self()->query(serviceType,
        QString("%1 and [X-KDevelop-Version] == %2").arg(constraint).arg(KDEVELOP_PLUGIN_VERSION));
}

KTrader::OfferList KDevPluginController::queryPlugins(const QString &constraint)
{
    return query("KDevelop/Plugin", constraint);
}

KURL::List KDevPluginController::profileResources(const QString &/*nameFilter*/)
{
    return KURL::List();
}

KURL::List KDevPluginController::profileResourcesRecursive(const QString &/*nameFilter*/)
{
    return KURL::List();
}

#include "kdevplugincontroller.moc"
