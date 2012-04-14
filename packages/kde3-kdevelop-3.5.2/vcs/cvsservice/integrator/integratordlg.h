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
#ifndef INTEGRATORDLG_H
#define INTEGRATORDLG_H

#include "integratordlgbase.h"
#include "cvsserviceintegrator.h"

class QDomDocument;

class IntegratorDlg: public IntegratorDlgBase, public VCSDialog {
Q_OBJECT
public:
    IntegratorDlg(CVSServiceIntegrator *integrator, QWidget *parent = 0, const char *name = 0);

    virtual QWidget *self();
    virtual void init(const QString &projectName, const QString &projectLocation);

public slots:
    virtual void login_clicked();
    virtual void init_clicked();
    virtual void accept();
    virtual void createModule_clicked();

private:
    CVSServiceIntegrator *m_integrator;
    QString m_projectLocation;
    QString m_projectName;
};

#endif
