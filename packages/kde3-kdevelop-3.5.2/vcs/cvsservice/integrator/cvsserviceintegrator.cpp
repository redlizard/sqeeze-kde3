/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "cvsserviceintegrator.h"

#include <kdevgenericfactory.h>
#include <kdevplugininfo.h>

#include "integratordlg.h"

static const KDevPluginInfo data("kdevcvsserviceintegrator");
typedef KDevGenericFactory<CVSServiceIntegrator> CVSIntegratorFactory;
K_EXPORT_COMPONENT_FACTORY( libcvsserviceintegrator, CVSIntegratorFactory(data) )

CVSServiceIntegrator::CVSServiceIntegrator(QObject* parent, const char* name, 
    const QStringList args)
    :KDevVCSIntegrator(parent, name)
{
}

CVSServiceIntegrator::~CVSServiceIntegrator( )
{
}

VCSDialog* CVSServiceIntegrator::fetcher(QWidget* parent)
{
    return 0;
}

VCSDialog* CVSServiceIntegrator::integrator(QWidget* parent)
{
    IntegratorDlg *dlg = new IntegratorDlg(this, parent);
    return dlg;
}

#include "cvsserviceintegrator.moc"
