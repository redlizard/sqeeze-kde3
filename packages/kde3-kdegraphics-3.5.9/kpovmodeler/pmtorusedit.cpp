/***************************************************************************
                          pmtorusedit.cpp  -  description
                             -------------------
    begin                : Sun Jul 1 2001
    copyright            : (C) 2001 by Van Hecke Philippe
    email                : lephiloux@tiscalinet.be
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "pmtorusedit.h"
#include "pmtorus.h"
#include "pmvectoredit.h"
#include "pmlineedits.h"

#include <qlayout.h>
#include <qlabel.h>
#include <klocale.h>
#include <qcheckbox.h>


PMTorusEdit::PMTorusEdit( QWidget* parent, const char* name )
      : Base( parent, name )
{
   m_pDisplayedObject = 0;
}

void PMTorusEdit::createTopWidgets( )
{
   Base::createTopWidgets( );

   QGridLayout* gl;
   QHBoxLayout* hl;

   m_pMinorRadius = new PMFloatEdit( this );
   m_pMajorRadius = new PMFloatEdit( this );
   m_pSturm = new QCheckBox( i18n( "Sturm" ), this );

   hl = new QHBoxLayout( topLayout( ) );
   gl = new QGridLayout( hl, 2, 2 );
   gl->addWidget( new QLabel( i18n( "Minor radius:" ), this ), 0, 0 );
   gl->addWidget( m_pMinorRadius, 0, 1 );
   gl->addWidget( new QLabel( i18n( "Major radius:" ), this ), 1, 0 );
   gl->addWidget( m_pMajorRadius, 1, 1 );
   hl->addStretch( 1 );

   topLayout( )->addWidget( m_pSturm );


   connect( m_pMinorRadius, SIGNAL( dataChanged( ) ), SIGNAL( dataChanged( ) ) );
   connect( m_pMajorRadius, SIGNAL( dataChanged( ) ), SIGNAL( dataChanged( ) ) );
   connect( m_pSturm, SIGNAL( clicked( ) ), SIGNAL( dataChanged( ) ) );
}

void PMTorusEdit::displayObject( PMObject* o )
{
   if( o->isA( "Torus" ) )
   {
      bool readOnly = o->isReadOnly( );
      m_pDisplayedObject = ( PMTorus* ) o;

      m_pMajorRadius->setValue( m_pDisplayedObject->majorRadius( ) );
      m_pMinorRadius->setValue( m_pDisplayedObject->minorRadius( ) );
      m_pSturm->setChecked( m_pDisplayedObject->sturm( ) );

      m_pMajorRadius->setReadOnly( readOnly );
      m_pMinorRadius->setReadOnly( readOnly );
      m_pSturm->setEnabled( !readOnly );

      Base::displayObject( o );
   }
   else
      kdError( PMArea ) << "PMTorusEdit: Can't display object\n";
}

void PMTorusEdit::saveContents( )
{
   if( m_pDisplayedObject )
   {
      Base::saveContents( );

      m_pDisplayedObject->setMajorRadius( m_pMajorRadius->value( ) );
      m_pDisplayedObject->setMinorRadius( m_pMinorRadius->value( ) );
      m_pDisplayedObject->setSturm( m_pSturm->isChecked( ) );
   }
}

bool PMTorusEdit::isDataValid( )
{
   if( m_pMinorRadius->isDataValid( ) )
      if( m_pMajorRadius->isDataValid( ) )
         return Base::isDataValid( );
   return false;
}


#include "pmtorusedit.moc"

