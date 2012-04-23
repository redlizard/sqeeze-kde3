/* 
 *
 * $Id: k3bsessionimportitem.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef _K3B_SESSION_IMPORT_ITEM_H_
#define _K3B_SESSION_IMPORT_ITEM_H_


#include "k3bdataitem.h"


class K3bDataDoc;
class K3bFileItem;
class K3bDirItem;
class K3bIso9660File;


class K3bSessionImportItem : public K3bDataItem
{
 public:
  K3bSessionImportItem( const K3bIso9660File*, K3bDataDoc* doc, K3bDirItem* );
  K3bSessionImportItem( const K3bSessionImportItem& );
  ~K3bSessionImportItem();

  K3bDataItem* copy() const;

  K3bFileItem* replaceItem() const { return m_replaceItem; }
  void setReplaceItem( K3bFileItem* item ) { m_replaceItem = item; }

  bool isFile() const { return false; }
  bool isFromOldSession() const { return true; }

  bool isRemoveable() const { return false; }
  bool isMoveable() const { return false; }
  bool isRenameable() const { return false; }
  bool isHideable() const { return false; }
  bool writeToCd() const { return false; }

 protected:
  // the size of an item from an imported session does not depend
  // on the value of followSymlinks
  /**
   * Normally one does not use this method but K3bDataItem::size()
   */
  KIO::filesize_t itemSize( bool ) const { return m_size; }

 private:
  K3bFileItem* m_replaceItem;
  KIO::filesize_t m_size;
};

#endif
