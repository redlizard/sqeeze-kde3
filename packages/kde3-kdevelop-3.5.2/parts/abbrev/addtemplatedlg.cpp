/*
 *  Copyright (C) 2002 Roberto Raggi (roberto@kdevelop.org)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */
#include "addtemplatedlg.h"
#include <qcombobox.h>
#include <klineedit.h>
#include <qpushbutton.h>

AddTemplateDialog::AddTemplateDialog( QStringList suffixesList, QWidget *parent, const char *name )
    : AddTemplateDialogBase( parent, name )
{
    setFocusProxy( editTemplate );
    comboSuffixes->insertStringList(suffixesList);
    connect( editTemplate,  SIGNAL(textChanged ( const QString & )), this, SLOT( textChanged()));
    connect( editDescription,  SIGNAL(textChanged ( const QString & )), this, SLOT( textChanged()));
    buttonOk->setEnabled(false);
}

AddTemplateDialog::~AddTemplateDialog()
{
}

void AddTemplateDialog::textChanged()
{
    buttonOk->setEnabled( !editTemplate->text().isEmpty() && !editDescription->text().isEmpty() );
}

QString AddTemplateDialog::templ() const
{
    return editTemplate->text();
}

QString AddTemplateDialog::description() const
{
    return editDescription->text();
}

QString AddTemplateDialog::suffixes() const
{
    return comboSuffixes->currentText();
}

#include "addtemplatedlg.moc"
