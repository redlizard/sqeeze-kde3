/***************************************************************************
 *   Copyright (C) 2003 by Roberto Raggi                                   *
 *   roberto@kdevelop.org                                                  *
 *                                                                         *
 *   Copyright (C) 2006 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <klistbox.h>
#include <kcombobox.h>
#include <kurlrequester.h>
#include <kdebug.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <klocale.h>

#include <qfile.h>
#include <qdir.h>
#include <qregexp.h>
#include <cstdlib>

#include "settingsdialog.h"

QListBoxItem* QListBox_selectedItem(QListBox* cpQListBox)
{
    if ( cpQListBox->selectionMode() != QListBox::Single )
    return 0;
    if ( cpQListBox->isSelected( cpQListBox->currentItem() ) )
    return  cpQListBox->item(cpQListBox->currentItem());
    return 0;
}

SettingsDialog::SettingsDialog(QWidget* parent, const char* name, WFlags fl)
    : SettingsDialogBase(parent,name,fl)
{
    QStringList qtdirs;
	qtdirs.push_back( ::getenv("QTDIR") + QString("/include") );
	qtdirs.push_back( QString::fromLocal8Bit(::getenv("HOME")) + "/dev/qt/include" );
	qtdirs.push_back( QString::fromLocal8Bit(::getenv("HOME")) + "/dev/qt-main/include" );
	qtdirs.push_back( "/usr/qt/4/include" );

    for( QStringList::Iterator it=qtdirs.begin(); it!=qtdirs.end(); ++it )
    {
        QString qtdir = *it;
        if( !qtdir.isEmpty() && isValidQtDir(qtdir) )
            if (!qtListBox->findItem(qtdir, ExactMatch))
                qtListBox->insertItem( qtdir );
    }

	qtUrl->setMode( KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly );

	connect( addUrlButton, SIGNAL(clicked()), this, SLOT(addUrlButton_clicked()) );
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::slotSelectionChanged(QListBoxItem* item)
{
    if( !qtListBox->selectedItem() )
	{
        emit enabled( false );
        return;
    }


    emit enabled( true );
}

bool SettingsDialog::isValidQtDir( const QString & path ) const
{
    return QFile::exists( path + "/QtCore/QtCore" );
}

QString SettingsDialog::qtDir( ) const
{
    return qtListBox->currentText();
}

QString SettingsDialog::configuration( ) const
{
    return "";
}

void SettingsDialog::addUrlButton_clicked( )
{
	kdDebug(9000) << k_funcinfo << endl;

	if ( isValidQtDir( qtUrl->url() ) )
	{
		qtListBox->insertItem( qtUrl->url() );
		if ( QListBoxItem * item = qtListBox->findItem( qtUrl->url(), ExactMatch ) )
		{
			qtListBox->setSelected( item, true );
		}
		qtUrl->lineEdit()->clear();
	}
	else
	{
		KMessageBox::error( this, i18n("This doesn't appear to be a valid Qt4 include directory.\nPlease select a different directory."), i18n("Invalid Directory") );
	}
}


#include "settingsdialog.moc"
//kate: indent-mode csands; tab-width 4; space-indent off;


