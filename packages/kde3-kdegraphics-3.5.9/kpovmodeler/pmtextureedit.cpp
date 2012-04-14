/*
**************************************************************************
                                 description
                             --------------------
    copyright            : (C) 2001 by Luis Carvalho
    email                : lpassos@mail.telepac.pt
**************************************************************************

**************************************************************************
*                                                                        *
*  This program is free software; you can redistribute it and/or modify  *
*  it under the terms of the GNU General Public License as published by  *
*  the Free Software Foundation; either version 2 of the License, or     *
*  (at your option) any later version.                                   *
*                                                                        *
**************************************************************************/


#include "pmtextureedit.h"
#include "pmtexture.h"
#include "pmlinkedit.h"

#include <qlayout.h>
#include <qcheckbox.h>
#include <klocale.h>


PMTextureEdit::PMTextureEdit( QWidget* parent, const char* name )
      : Base( parent, name )
{
   m_pDisplayedObject = 0;
}

void PMTextureEdit::createTopWidgets()
{
   Base::createTopWidgets();
   m_pUVMapping = new QCheckBox( i18n( "UV mapping" ), this );
   topLayout( )->addWidget( m_pUVMapping );

   connect( m_pUVMapping, SIGNAL( clicked() ), SIGNAL( dataChanged() ) );
}

void PMTextureEdit::displayObject( PMObject* o )
{
   if( o->isA( "Texture" ) )
   {
      m_pDisplayedObject = ( PMTexture* ) o;
      bool readOnly = m_pDisplayedObject->isReadOnly( );
      m_pUVMapping->setChecked( m_pDisplayedObject->uvMapping() );
      m_pUVMapping->setEnabled( !readOnly );

      Base::displayObject( o );
   }
   else
      kdError( PMArea ) << "PMTextureEdit: Can't display object\n";
}

void PMTextureEdit::saveContents()
{
   if( m_pDisplayedObject )
   {
      Base::saveContents();
      m_pDisplayedObject->setUVMapping( m_pUVMapping->isChecked() );
   }
}

#include "pmtextureedit.moc"
