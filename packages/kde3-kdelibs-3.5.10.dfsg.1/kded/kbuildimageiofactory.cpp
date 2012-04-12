/*  This file is part of the KDE libraries
 *  Copyright (C) 2000 Waldo Bastian <bastian@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "ksycoca.h"
#include "ksycocadict.h"
#include "kresourcelist.h"

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <klocale.h>
#include <assert.h>

#include <kimageiofactory.h>
#include <kbuildimageiofactory.h>

KBuildImageIOFactory::KBuildImageIOFactory() :
  KImageIOFactory()
{
   m_resourceList = new KSycocaResourceList();
   m_resourceList->add( "services", "*.kimgio" );
}

// return all service types for this factory
// i.e. first arguments to m_resourceList->add() above
QStringList KBuildImageIOFactory::resourceTypes()
{
    return QStringList() << "services";
}

KBuildImageIOFactory::~KBuildImageIOFactory() 
{
   delete m_resourceList;
}

KSycocaEntry *
KBuildImageIOFactory::createEntry( const QString& file, const char *resource )
{
   QString fullPath = locate( resource, file);

   KImageIOFormat *format = new KImageIOFormat(fullPath);
   return format;
}

void
KBuildImageIOFactory::addEntry(KSycocaEntry *newEntry, const char *resource)
{
   KSycocaFactory::addEntry(newEntry, resource);

   KImageIOFormat *format = (KImageIOFormat *) newEntry;
   rPath += format->rPaths;

   // Since Qt doesn't allow us to unregister image formats
   // we have to make sure not to add them a second time.
   // This typically happens when the sycoca database is updated 
   // incremenatally
   for( KImageIOFormatList::ConstIterator it = formatList->begin(); 
           it != formatList->end();
           ++it )
   {
      KImageIOFormat *_format = (*it);
      if (format->mType == _format->mType)
      {
         // Already in list
         format = 0;
         break;
      }
   }
   if (format)
      formatList->append( format );
}


void
KBuildImageIOFactory::saveHeader(QDataStream &str)
{
   KSycocaFactory::saveHeader(str);

   str << mReadPattern << mWritePattern << rPath;
}

void
KBuildImageIOFactory::save(QDataStream &str)
{
   rPath.sort();
   // Remove duplicates from rPath:
   QString last;
   for(QStringList::Iterator it = rPath.begin();
       it != rPath.end(); )
   {
      QStringList::Iterator it2 = it++;
      if (*it2 == last)
      {
         // remove duplicate
         rPath.remove(it2);
      }
      else
      {
         last = *it2;
      }
   }
   mReadPattern = createPattern( KImageIO::Reading );
   mWritePattern = createPattern( KImageIO::Writing );

   KSycocaFactory::save(str);
}

