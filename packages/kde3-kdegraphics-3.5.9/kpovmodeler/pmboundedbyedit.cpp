/*
**************************************************************************
                                 description
                             --------------------
    copyright            : (C) 2002 by Andreas Zehender
    email                : zehender@kde.org
**************************************************************************

**************************************************************************
*                                                                        *
*  This program is free software; you can redistribute it and/or modify  *
*  it under the terms of the GNU General Public License as published by  *
*  the Free Software Foundation; either version 2 of the License, or     *
*  (at your option) any later version.                                   *
*                                                                        *
**************************************************************************/


#include "pmboundedbyedit.h"
#include "pmboundedby.h"

#include <qlayout.h>
#include <qlabel.h>
#include <klocale.h>

PMBoundedByEdit::PMBoundedByEdit( QWidget* parent, const char* name )
      : Base( parent, name )
{
   m_pDisplayedObject = 0;
}

void PMBoundedByEdit::createTopWidgets( )
{
   Base::createTopWidgets( );

   m_pChildLabel = new QLabel( i18n( "No child objects" ), this );
   topLayout( )->addWidget( m_pChildLabel );
   m_pClippedByLabel = new QLabel( i18n( "(= clipped by)" ), this );
   topLayout( )->addWidget( m_pClippedByLabel );   
}

void PMBoundedByEdit::displayObject( PMObject* o )
{
   if( o->isA( "BoundedBy" ) )
   {
      m_pDisplayedObject = ( PMBoundedBy* ) o;

      if( m_pDisplayedObject->clippedBy( ) )
      {
         m_pChildLabel->show( );
         m_pClippedByLabel->show( );
      }
      else
      {
         m_pChildLabel->hide( );
         m_pClippedByLabel->hide( );
      }
      
      Base::displayObject( o );
   }
   else
      kdError( PMArea ) << "PMBoundedByEdit: Can't display object\n";
}

#include "pmboundedbyedit.moc"
