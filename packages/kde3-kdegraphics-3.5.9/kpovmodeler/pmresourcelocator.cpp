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

#include "pmresourcelocator.h"
#include "pmpovrayrenderwidget.h"
#include "pmdebug.h"

#include <qfileinfo.h>
#include <qdir.h>

PMResourceLocator* PMResourceLocator::s_pInstance = 0;
KStaticDeleter<PMResourceLocator> PMResourceLocator::s_staticDeleter;

PMResourceLocator::PMResourceLocator( )
      : m_cache( 100, 109, true )
{
   m_cache.setAutoDelete( true );
}

PMResourceLocator::~PMResourceLocator( )
{
   m_cache.clear( );
}

QString PMResourceLocator::findFile( const QString& file )
{
   if( !s_pInstance )
      s_staticDeleter.setObject( s_pInstance, new PMResourceLocator( ) );
   return s_pInstance->lookUp( file );
}

void PMResourceLocator::clearCache( )
{
   if( s_pInstance )
      s_pInstance->m_cache.clear( );
}

QString PMResourceLocator::lookUp( const QString& file )
{
   if( file.isEmpty( ) )
      return QString::null;

   kdDebug( PMArea ) << "LookUp: " << file << endl;
   
   QString* ps = m_cache.find( file );
   if( ps )
      return *ps;

   bool found = false;
   QString fullPath = QString::null;
   
   if( file[0] == '/' )
   {
      // absolute path, library paths are not used
      QFileInfo info( file );
      if( info.exists( ) && info.isReadable( ) && info.isFile( ) )
      {
         found = true;
         fullPath = file;
      }
   }
   else
   {
      QStringList plist = PMPovrayRenderWidget::povrayLibraryPaths( );
      QStringList::ConstIterator it = plist.begin( );
      for( ; ( it != plist.end( ) ) && !found; ++it )
      {
         QDir dir( *it );
         QFileInfo info( dir, file );
         if( info.exists( ) && info.isReadable( ) && info.isFile( ) )
         {
            found = true;
            fullPath = info.absFilePath( );
         }
      }
   }

   if( found )
   {
      QString* ni = new QString( fullPath );
      if( !m_cache.insert( file, ni ) )
         delete ni;
      kdDebug( PMArea ) << "File \"" << file << "\" found in "
                        << fullPath << endl;
   }

   return fullPath;
}

