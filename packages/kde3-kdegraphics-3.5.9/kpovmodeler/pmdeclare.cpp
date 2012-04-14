/*
**************************************************************************
                                 description
                             --------------------
    copyright            : (C) 2000-2002 by Andreas Zehender
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

#include "pmdeclare.h"
#include "pmdeclareedit.h"
#include "pmxmlhelper.h"
#include "pmmemento.h"
#include "pmprototypemanager.h"
#include "pmpart.h"

#include <qvaluelist.h>
#include <klocale.h>

PMDefinePropertyClass( PMDeclare, PMDeclareProperty );

PMMetaObject* PMDeclare::s_pMetaObject;
PMObject* createNewDeclare( PMPart* part )
{
   return new PMDeclare( part );
}

PMDeclare::PMDeclare( PMPart* part )
      : Base( part )
{
   m_pDeclareType = 0;
}

PMDeclare::PMDeclare( const PMDeclare& d )
      : Base( d )
{
   m_id = d.m_id; // CAUTION! Duplication of the id which has to be unique
   m_pDeclareType = 0; // will be set automatically in the
   // base constuctor when the children are copied
}

PMMetaObject* PMDeclare::metaObject( ) const
{
   if( !s_pMetaObject )
   {
      s_pMetaObject = new PMMetaObject( "Declare", Base::metaObject( ),
                                        createNewDeclare );
      s_pMetaObject->addProperty(
         new PMDeclareProperty( "id", &PMDeclare::setID, &PMDeclare::id ) );
   }
   return s_pMetaObject;
}

void PMDeclare::cleanUp( ) const
{
   if( s_pMetaObject )
   {
      delete s_pMetaObject;
      s_pMetaObject = 0;
   }
   Base::cleanUp( );
}

PMDeclare::~PMDeclare( )
{
}

QString PMDeclare::description( ) const
{
   PMPart* pPart = part( );
   QString d = i18n( "declaration" );

   if( m_pDeclareType && pPart )
   {
      const QValueList<PMDeclareDescription>& descriptions
         = pPart->prototypeManager( )->declarationTypes( );
      QValueList<PMDeclareDescription>::const_iterator it;
      bool found = false;
      for( it = descriptions.begin( ); it != descriptions.end( ) && !found; ++it )
      {
         if( ( *it ).type == m_pDeclareType )
         {
            d = ( *it ).description;
            found = true;
         }
      }
   }
   return d;
}

QString PMDeclare::pixmap( ) const
{
   PMPart* pPart = part( );
   QString d = "pmdeclare";

   if( m_pDeclareType && pPart )
   {
      const QValueList<PMDeclareDescription>& descriptions
         = pPart->prototypeManager( )->declarationTypes( );
      QValueList<PMDeclareDescription>::const_iterator it;
      bool found = false;
      for( it = descriptions.begin( ); it != descriptions.end( ) && !found; ++it )
      {
         if( ( *it ).type == m_pDeclareType )
         {
            d = ( *it ).pixmap;
            found = true;
         }
      }
   }
   return d;
}

void PMDeclare::serialize( QDomElement& e, QDomDocument& doc ) const
{
   e.setAttribute( "id", m_id );
   Base::serialize( e, doc );
}

void PMDeclare::readAttributes( const PMXMLHelper& h )
{
   m_id = h.stringAttribute( "id", "object" );
   Base::readAttributes( h );
}

PMDialogEditBase* PMDeclare::editWidget( QWidget* parent ) const
{
   return new PMDeclareEdit( parent );
}

void PMDeclare::setID( const QString& newID )
{
   if( newID != m_id )
   {
      if( m_pMemento )
      {
         m_pMemento->addIDChange( s_pMetaObject, PMIDID, m_id );
         m_pMemento->setDescriptionChanged( );
      }

      m_id = newID;
   }
}

QString PMDeclare::declareType( ) const
{
   if( !m_pDeclareType )
      return QString( "None" );
   return m_pDeclareType->className( );
}

void PMDeclare::setDeclareType( PMMetaObject* t )
{
   if( m_pDeclareType != t )
   {
      if( m_pMemento )
         m_pMemento->setDescriptionChanged( );

      m_pDeclareType = t;
   }
}

void PMDeclare::updateDeclareType( )
{
   PMPart* pPart = part( );
   if( !pPart )
      return;

   PMMetaObject* type = 0;
   PMObject* o = firstChild( );
   PMPrototypeManager* m = pPart->prototypeManager( );
   if( o )
   {
      if( o->isA( "GraphicalObject" ) )
         type = m->metaObject( "GraphicalObject" );
      else
         type = o->metaObject( );
   }
   setDeclareType( type );
}

void PMDeclare::restoreMemento( PMMemento* s )
{
   PMMementoDataIterator it( s );
   PMMementoData* data;

   for( ; it.current( ); ++it )
   {
      data = it.current( );
      if( data->objectType( ) == s_pMetaObject )
      {
         switch( data->valueID( ) )
         {
            case PMIDID:
               setID( data->stringData( ) );
               break;
            default:
               kdError( PMArea ) << "Wrong ID in PMDeclare::restoreMemento\n";
               break;
         }
      }
   }
   Base::restoreMemento( s );
}

void PMDeclare::addLinkedObject( PMObject* o )
{
   m_linkedObjects.append( o );
}

void PMDeclare::removeLinkedObject( PMObject* o )
{
   m_linkedObjects.removeRef( o );
}

void PMDeclare::childAdded( PMObject* o )
{
   if( !m_pDeclareType )
      updateDeclareType( );

   Base::childAdded( o );
}

void PMDeclare::childRemoved( PMObject* o )
{
   if( !firstChild( ) )
      setDeclareType( 0 );

   Base::childRemoved( o );
}
