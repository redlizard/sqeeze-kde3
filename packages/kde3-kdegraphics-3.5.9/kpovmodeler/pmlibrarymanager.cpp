/*
**************************************************************************
                                 description
                             --------------------
    copyright            : (C) 2003 by Luis Carvalho
    email                : lpassos@oninetspeed.pt
**************************************************************************

**************************************************************************
*                                                                        *
*  This program is free software; you can redistribute it and/or modify  *
*  it under the terms of the GNU General Public License as published by  *
*  the Free Software Foundation; either version 2 of the License, or     *
*  (at your option) any later version.                                   *
*                                                                        *
**************************************************************************/

#include "pmlibrarymanager.h"

#include <kconfig.h>
#include <kstandarddirs.h>
#include <kglobal.h>

#include <qfile.h>
#include <qdir.h>

#include "pmdebug.h"

PMLibraryManager* PMLibraryManager::s_pInstance = 0;
KStaticDeleter<PMLibraryManager> PMLibraryManager::s_staticDeleter;

PMLibraryHandle* PMLibraryManager::getLibraryHandle( const QString& libraryName ) 
{
   QPtrListIterator<PMLibraryHandle> it( m_libraries );

   for( ; it.current( ); ++it )
      if( it.current( )->name( ) == libraryName )
         return it.current( );

   return NULL;
}

PMLibraryManager::PMLibraryManager( )
{
   m_libraries.setAutoDelete( true );
   scanLibraries( );
}

PMLibraryManager::~PMLibraryManager( )
{
   m_libraries.clear( );
}

void PMLibraryManager::saveConfig( KConfig* /*cfg*/ )
{
}

void PMLibraryManager::restoreConfig( KConfig* /*cfg*/ )
{
}

QValueList<QString> PMLibraryManager::availableLibraries( )
{
   QValueList<QString> result;
   QPtrListIterator<PMLibraryHandle> it( m_libraries );

   for( ; it.current( ); ++it )
      result.push_back( it.current( )->name( ) );

   return result;
}

PMLibraryManager* PMLibraryManager::theManager( )
{
   if( !s_pInstance )
      s_staticDeleter.setObject( s_pInstance, new PMLibraryManager( ) );
   return s_pInstance;
}

void PMLibraryManager::scanLibraries( )
{
   QStringList libraryDirectories;
   
   // Search for sub directories in /usr/share/kpovmodeler/library
   libraryDirectories = KGlobal::dirs( )->findDirs( "data", "kpovmodeler/library" );

   for( QStringList::Iterator i = libraryDirectories.begin( ); i != libraryDirectories.end( ); ++i )
   {
      QDir curDir( *i );
      curDir.setFilter( QDir::Dirs );
      QFileInfoListIterator it( *( curDir.entryInfoList( ) ) );

      // For each sub directory
      QFileInfo* fi;
      for( ; ( fi = it.current( ) ) != NULL; ++it ) 
      {
         // check for the existance of library_index.xml
         // If it's there it's a library
         if( QFile::exists( fi->absFilePath( ) + "/library_index.xml" ) )
         {
            // Create the corresponding PMLibraryHandle
            PMLibraryHandle* h;

            h = new PMLibraryHandle( fi->absFilePath( ) );
            if( !getLibraryHandle( h->name( ) ) )
               m_libraries.append( h );
            else
               // a library with that name already exists
               delete h;
         }
      }
   }
}

void PMLibraryManager::refresh( )
{
   // TODO: Manage the list incrementaly so that previouly handed out 
   // PMLibraryHandle pointers are kept valid
   m_libraries.clear( );
   scanLibraries( );
}
