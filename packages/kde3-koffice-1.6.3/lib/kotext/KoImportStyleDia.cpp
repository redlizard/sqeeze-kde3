/* This file is part of the KDE project
   Copyright (C)  2002 Montel Laurent <lmontel@mandrakesoft.com>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <klocale.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include "KoImportStyleDia.h"

#include <kdebug.h>
#include <qlabel.h>

KoImportStyleDia::KoImportStyleDia( KoStyleCollection* currentCollection, QWidget *parent, const char *name )
    : KDialogBase( parent, name , true, "", Ok|Cancel|User1, Ok, true )
{
    setCaption( i18n("Import Styles") );
    m_currentCollection = currentCollection;
    QVBox *page = makeVBoxMainWidget();
    new QLabel(i18n("Select styles to import:"), page);
    m_listStyleName = new QListBox( page );
    m_listStyleName->setSelectionMode( QListBox::Multi );
    enableButtonOK( m_listStyleName->count() != 0 );
    setButtonText( KDialogBase::User1, i18n("Load...") );
    connect( this, SIGNAL( user1Clicked() ), this, SLOT(slotLoadFile()));
    setInitialSize( QSize( 300, 400 ) );
    setFocus();
}

KoImportStyleDia::~KoImportStyleDia()
{
}

void KoImportStyleDia::generateStyleList()
{
    for (uint i = 0; i< m_listStyleName->count();i++)
    {
        if ( !m_listStyleName->isSelected( i ) )
        {
            //remove this style from list
            KoParagStyle* style = m_styleList.styleAt( i );
            updateFollowingStyle( style );
            m_styleList.removeStyle( style );
            break;
        }
    }
}

void KoImportStyleDia::updateFollowingStyle( KoParagStyle* removedStyle )
{
    QValueList<KoUserStyle *> lst = m_styleList.styleList();
    for( QValueList<KoUserStyle *>::Iterator it = lst.begin(); it != lst.end(); ++it ) {
        KoParagStyle* style = static_cast<KoParagStyle *>( *it );
        if ( style->followingStyle() == removedStyle )
        {
            style->setFollowingStyle( style );
        }
    }
}

void KoImportStyleDia::slotLoadFile()
{
    loadFile();
    enableButtonOK( m_listStyleName->count() != 0 );
}

void KoImportStyleDia::initList()
{
    m_listStyleName->insertStringList( m_styleList.displayNameList() );
}

void KoImportStyleDia::slotOk()
{
    generateStyleList();
    KDialogBase::slotOk();
}

QString KoImportStyleDia::generateStyleName( const QString & templateName ) const
{
    QString name;
    int num = 1;
    bool exists;
    do {
        name = templateName.arg( num );
        exists = m_currentCollection->findStyle( name ) != 0;
        ++num;
    } while ( exists );
    return name;
}

QString KoImportStyleDia::generateStyleDisplayName( const QString & templateName ) const
{
    QString name;
    int num = 1;
    bool exists;
    do {
        name = templateName.arg( num );
        exists = m_currentCollection->findStyleByDisplayName( name ) != 0;
        ++num;
    } while ( exists );
    return name;
}

void KoImportStyleDia::clear()
{
    m_styleList.clear();
}

#include "KoImportStyleDia.moc"
