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

#include "pmxmlparser.h"

#include <qbuffer.h>
#include <klocale.h>

#include "pmpart.h"
#include "pmscene.h"
#include "pmxmlhelper.h"
#include "pmprototypemanager.h"
#include "pmdocumentformat.h"
#include "pmdebug.h"

PMXMLParser::PMXMLParser( PMPart* part, QIODevice* dev )
      : PMParser( part, dev )
{
   init( );
}

PMXMLParser::PMXMLParser( PMPart* part, const QByteArray& array )
      : PMParser( part, array )
{
   init( );
}

void PMXMLParser::init( )
{
   m_pDoc = 0;
   m_majorDocumentFormat = 1;
   m_minorDocumentFormat = 0;
}

PMXMLParser::~PMXMLParser( )
{
   if( m_pDoc )
      delete m_pDoc;
}

bool PMXMLParser::initDocument( )
{
   if( !m_pDoc )
   {
      m_pDoc = new QDomDocument( "KPOVMODELER" );
      if( m_pDoc->setContent( m_pDevice ) )
         return true;
      else
      {
         printError( i18n( "Could not load the documents data!" ) );
         setFatalError( );
         return false;
      }
   }
   return true;
}

void PMXMLParser::topParse( )
{
   if( initDocument( ) )
   {
      QDomElement e = m_pDoc->documentElement( );
      // read the format number
      // assume 1.0 on error
      QString fstring = e.attribute( "majorFormat", "1" );
      bool ok = true;
      int format = fstring.toInt( &ok );
      if( !ok || ( format < 1 ) )
         format = 1;
      m_majorDocumentFormat = format;

      fstring = e.attribute( "minorFormat", "0" );
      ok = true;
      format = fstring.toInt( &ok );
      if( !ok || ( format < 0 ) )
         format = 0;
      m_minorDocumentFormat = format;

      if( ( m_majorDocumentFormat > c_majorDocumentFormat )
          || ( m_majorDocumentFormat == c_majorDocumentFormat )
          && ( m_minorDocumentFormat > c_minorDocumentFormat ) )
         printWarning( i18n( "This document was created with a newer version of KPovModeler. "
                             "The whole document may not be loaded correctly." ) );

      if( e.tagName( ) == "objects" )
      {
         parseChildObjects( e, 0 );
      }
      else if( e.tagName( ) == "scene" )
      {
         PMScene* scene = new PMScene( m_pPart );
         insertChild( scene, 0 );
         PMXMLHelper hlp( e, m_pPart, this,
                          m_majorDocumentFormat, m_minorDocumentFormat );
         scene->readAttributes( hlp );
         parseChildObjects( e, scene );
      }
      else
      {
         printError( i18n( "Wrong top level tag" ) );
         setFatalError( );
      }
   }
}

void PMXMLParser::parseChildObjects( QDomElement& e, PMObject* parent )
{
   QDomNode c = e.firstChild( );
   while( !c.isNull( ) )
   {
      if( c.isElement( ) )
      {
         QDomElement ce = c.toElement( );
         PMPrototypeManager* m = m_pPart->prototypeManager( );
         PMObject* obj = m->newObject( m->className( ce.tagName( ) ) );
         if( obj )
         {
            PMXMLHelper hlp( ce, m_pPart, this,
                             m_majorDocumentFormat, m_minorDocumentFormat );
            obj->readAttributes( hlp );
            if( insertChild( obj, parent ) )
            {
               parseChildObjects( ce, obj );

               if( obj->isA( "Declare" ) )
                  checkID( ( PMDeclare* ) obj );
            }
            else
               delete obj;
         }
         else if( ce.tagName( ) != "extra_data" )
            printError( i18n( "Unknown object %1" ).arg( ce.tagName( ) ) );
      }
      c = c.nextSibling( );
   }
}


void PMXMLParser::quickParse( QStringList& list )
{
   if( initDocument( ) )
   {
      QDomElement e = m_pDoc->documentElement( );
      if( ( e.tagName( ) == "objects" ) || ( e.tagName( ) == "scene" ) )
      {
         QDomNode c = e.firstChild( );

         while( !c.isNull( ) )
         {
            if( c.isElement( ) )
            {
               QDomElement ce = c.toElement( );
               QString type = m_pPart->prototypeManager( )->className( ce.tagName( ) );
               if( !type.isNull( ) )
                  list.append( type );
            }
            c = c.nextSibling( );
         }
      }
      else
         printError( i18n( "Wrong top level tag" ) );
   }
}
