/* This file is part of the KDE project
   Copyright (C)  2001 Montel Laurent <lmontel@mandrakesoft.com>

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
 * Boston, MA 02110-1301, USA.
*/

#include <klocale.h>

#include <qvbox.h>
#include <qwhatsthis.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

#include "KoChangeCaseDia.h"


KoChangeCaseDia::KoChangeCaseDia( QWidget *parent, const char *name )
    : KDialogBase( parent, name , true, "", Ok|Cancel, Ok, true )
{

    setCaption( i18n("Change Case") );
    QVBox *page = makeVBoxMainWidget();

    QButtonGroup *grp = new QButtonGroup( 1, QGroupBox::Horizontal, i18n( "Case" ),page );
    grp->setRadioButtonExclusive( TRUE );
    grp->layout();
    m_upperCase=new QRadioButton( i18n("&Uppercase"), grp );
    m_lowerCase=new QRadioButton( i18n("&Lowercase"), grp );

    m_titleCase=new QRadioButton( i18n("T&itle case"), grp );

    m_toggleCase=new QRadioButton( i18n("&Toggle case"), grp );
    m_sentenceCase=new QRadioButton( i18n("Sentence case"), grp );
    QWhatsThis::add( m_sentenceCase, i18n("Convert first letter of a sentence to uppercase."));

    m_upperCase->setChecked(true);
    grp->setRadioButtonExclusive( TRUE );

}

KoChangeCaseDia::TypeOfCase KoChangeCaseDia::getTypeOfCase()
{
    TypeOfCase type=UpperCase;
    if( m_upperCase->isChecked())
        type=UpperCase;
    else if(m_lowerCase->isChecked())
        type=LowerCase;
    else if(m_titleCase->isChecked())
        type=TitleCase;
    else if( m_toggleCase->isChecked())
        type=ToggleCase;
    else if( m_sentenceCase->isChecked())
        type=SentenceCase;
    return type;
}

#include "KoChangeCaseDia.moc"
