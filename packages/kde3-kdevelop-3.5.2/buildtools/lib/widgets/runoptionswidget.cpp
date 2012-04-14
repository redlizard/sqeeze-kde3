/* This file is part of the KDE project
   Copyright (C) 2001-2002 Bernd Gehrmann <bernd@kdevelop.org>

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

#include "runoptionswidget.h"

#include <klocale.h>
#include <kfiledialog.h>
#include <kurlrequester.h>
#include <kurlcompletion.h>
#include <klineedit.h>
#include <urlutil.h>

#include <qlistview.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qlabel.h>

#include "domutil.h"
#include "environmentvariableswidget.h"


RunOptionsWidget::RunOptionsWidget(QDomDocument &dom, const QString &configGroup,
                                    const QString &buildDirectory, QWidget *parent, const char *name)
    : RunOptionsWidgetBase(parent, name),
      m_dom(dom), m_configGroup(configGroup)
{
    // Create the "Environment Variables" GUI
    env_var_group->setColumnLayout( 1, Qt::Vertical );
    m_environmentVariablesWidget = new EnvironmentVariablesWidget( dom, configGroup + "/run/envvars", env_var_group );

    mainprogram_edit->completionObject()->setMode(KURLCompletion::FileCompletion);
    mainprogram_edit->setMode( KFile::File | KFile::ExistingOnly | KFile::LocalOnly );
    if( DomUtil::readEntry(dom, configGroup + "/run/mainprogram").isEmpty() && QFileInfo( buildDirectory ).exists() )
    {
        mainprogram_edit->setURL( buildDirectory );
        mainprogram_edit->fileDialog()->setURL( KURL::fromPathOrURL(buildDirectory) );
    }else if ( QFileInfo( DomUtil::readEntry(dom, configGroup + "/run/mainprogram") ).exists() )
    {
        QString program = DomUtil::readEntry(dom, configGroup + "/run/mainprogram");
        if( QDir::isRelativePath(program) )
            program = buildDirectory + "/" + program;
        mainprogram_edit->setURL(program);
        mainprogram_edit->fileDialog()->setURL( program );
    }else
    {
        mainprogram_edit->setURL(QString());
        mainprogram_edit->fileDialog()->setURL(QString());
    }

    cwd_edit->completionObject()->setMode(KURLCompletion::DirCompletion);
    cwd_edit->setMode( KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly );
    if( DomUtil::readEntry(dom, configGroup + "/run/globalcwd").isEmpty() && QFileInfo( buildDirectory ).exists() )
    {
        cwd_edit->setURL( buildDirectory );
        cwd_edit->fileDialog()->setURL( KURL::fromPathOrURL(buildDirectory) );
    }else if( QFileInfo( DomUtil::readEntry(dom, configGroup + "/run/globalcwd") ).exists() )
    {
        cwd_edit->setURL(DomUtil::readEntry(dom, configGroup + "/run/globalcwd"));
        cwd_edit->fileDialog()->setURL( KURL::fromPathOrURL( DomUtil::readEntry(dom, configGroup + "/run/globalcwd") ) );
    }else
    {
        cwd_edit->setURL(QString());
        cwd_edit->fileDialog()->setURL(QString());
    }

    if( configGroup == "/kdevautoproject" || configGroup == "/kdevtrollproject" )
    {
        mainProgramGroupBox->setCheckable(true);
        mainProgramGroupBox->setChecked( DomUtil::readBoolEntry(dom, configGroup+"/run/useglobalprogram", false ) );
    }else
    {
        delete notelabel;
    }

    // Read the main program command line arguments and store them in the edit box

    runargs_edit->setText(DomUtil::readEntry(dom, configGroup + "/run/programargs"));
    debugargs_edit->setText(DomUtil::readEntry(dom, configGroup + "/run/globaldebugarguments"));

    startinterminal_box->setChecked(DomUtil::readBoolEntry(dom, configGroup + "/run/terminal"));
    autocompile_box->setChecked(DomUtil::readBoolEntry(dom, configGroup + "/run/autocompile", false));
    autoinstall_box->setChecked(DomUtil::readBoolEntry(dom, configGroup + "/run/autoinstall", false));
    autokdesu_box->setChecked(DomUtil::readBoolEntry(dom, configGroup + "/run/autokdesu", false));
}


RunOptionsWidget::~RunOptionsWidget()
{}


void RunOptionsWidget::accept()
{
    DomUtil::writeEntry(m_dom, m_configGroup + "/run/mainprogram", mainprogram_edit->url());
    DomUtil::writeEntry(m_dom, m_configGroup + "/run/programargs", runargs_edit->text());
    DomUtil::writeEntry(m_dom, m_configGroup + "/run/globaldebugarguments", debugargs_edit->text());
    DomUtil::writeEntry(m_dom, m_configGroup + "/run/globalcwd", cwd_edit->url());
    DomUtil::writeBoolEntry(m_dom, m_configGroup + "/run/useglobalprogram", mainProgramGroupBox->isChecked());
    DomUtil::writeBoolEntry(m_dom, m_configGroup + "/run/terminal", startinterminal_box->isChecked());
    DomUtil::writeBoolEntry(m_dom, m_configGroup + "/run/autocompile", autocompile_box->isChecked());
    DomUtil::writeBoolEntry(m_dom, m_configGroup + "/run/autoinstall", autoinstall_box->isChecked());
    DomUtil::writeBoolEntry(m_dom, m_configGroup + "/run/autokdesu", autokdesu_box->isChecked());

    m_environmentVariablesWidget->accept();
}

void RunOptionsWidget::mainProgramChanged( )
{

    if( mainProgramGroupBox->isChecked() && mainprogram_edit->url().isEmpty() )
    {
        mainprogram_label->setPaletteForegroundColor(QColor("#ff0000"));
    }
    else
    {
        mainprogram_label->unsetPalette();
    }
}

#include "runoptionswidget.moc"
