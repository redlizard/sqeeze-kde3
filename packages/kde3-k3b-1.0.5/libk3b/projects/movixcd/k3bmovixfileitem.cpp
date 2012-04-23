/* 
 *
 * $Id: k3bmovixfileitem.cpp 619556 2007-01-03 17:38:12Z trueg $
 * Copyright (C) 2003 Sebastian Trueg <trueg@k3b.org>
 *
 * This file is part of the K3b project.
 * Copyright (C) 1998-2007 Sebastian Trueg <trueg@k3b.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */


#include "k3bmovixfileitem.h"
#include "k3bmovixdoc.h"

#include <k3bdiritem.h>


K3bMovixFileItem::K3bMovixFileItem( const QString& fileName, 
				    K3bMovixDoc* doc, 
				    K3bDirItem* dir, 
				    const QString& k3bName )
  : K3bFileItem( fileName, doc, dir, k3bName ),
    m_doc(doc),
    m_subTitleItem(0)
{
}


K3bMovixFileItem::~K3bMovixFileItem()
{
  if( m_subTitleItem )
    m_doc->removeSubTitleItem( this );

  // remove this from parentdir
  // it is important to do it here and not
  // rely on the K3bFileItem destructor becasue
  // otherwise the doc is not informed early enough
  if( parent() )
    parent()->takeDataItem( this );
}


void K3bMovixFileItem::setK3bName( const QString& newName )
{
  K3bFileItem::setK3bName( newName );

  // take care of the subTitle file
  if( m_subTitleItem ) {
    m_subTitleItem->setK3bName( subTitleFileName(k3bName()) );
  }
}


QString K3bMovixFileItem::subTitleFileName( const QString& name )
{
  // remove ending from k3bName
  QString subName = name;
  int pos = subName.findRev(".");
  if( pos > 0 )
    subName.truncate( pos );
  subName += ".sub";
  return subName;
}
