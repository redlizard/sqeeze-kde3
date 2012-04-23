/* 
 *
 * $Id: k3bdataitem.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef K3BDATAITEM_H
#define K3BDATAITEM_H


class K3bDirItem;
class K3bDataDoc;

#include <qstring.h>

#include <kio/global.h>

#include <k3bmsf.h>
#include "k3b_export.h"


/**
  *@author Sebastian Trueg
  */
class LIBK3B_EXPORT K3bDataItem
{
 public: 
  K3bDataItem( K3bDataDoc* doc, K3bDataItem* parent = 0, int flags = 0 );

  /**
   * Default copy constructor.
   *
   * The result is an exact copy except that no parent dir it set and, thus, also no doc.
   */
  K3bDataItem( const K3bDataItem& );

  virtual ~K3bDataItem();

  /**
   * Return an exact copy of this data item.
   *
   * The result is an exact copy except that no parent dir it set and, thus, also no doc.
   *
   * Implementations should use the default constructor.
   */
  virtual K3bDataItem* copy() const = 0;
	
  K3bDirItem* parent() { return m_parentDir; }
  K3bDirItem* getParent() const { return m_parentDir; }

  /**
   * Remove this item from it's parent and return a pointer to it.
   */
  K3bDataItem* take();
	
  K3bDataDoc* doc() const { return m_doc; }
  virtual const QString& k3bName() const;
  virtual void setK3bName( const QString& );

  /** 
   * returns the path as defined by the k3b-hierachy, NOT starting with a slash
   * (since this is used for graft-points!) 
   * directories have a trailing "/"
   */
  virtual QString k3bPath() const;

  /**
   * Returns the name of the item as used on the CD or DVD image.
   *
   * This is only valid after a call to @p K3bDataDoc::prepareFilenames()
   */
  const QString& writtenName() const { return m_writtenName; }

  /**
   * \return the pure name used in the Iso9660 tree.
   *
   * This is only valid after a call to @p K3bDataDoc::prepareFilenames()
   */
  const QString& iso9660Name() const { return m_rawIsoName; }

  /**
   * Returns the path of the item as written to the CD or DVD image.
   *
   * This is suited to be used for mkisofs graftpoints.
   *
   * This is only valid after a call to @p K3bDataDoc::prepareFilenames()
   */
  virtual QString writtenPath() const;

  virtual QString iso9660Path() const;

  /**
   * Used to set the written name by @p K3bDataDoc::prepareFilenames()
   */
  void setWrittenName( const QString& s ) { m_writtenName = s; }

  /**
   * Used to set the pure Iso9660 name by @p K3bDataDoc::prepareFilenames()
   */
  void setIso9660Name( const QString& s ) { m_rawIsoName = s; }

  virtual K3bDataItem* nextSibling() const;
	
  /** returns the path to the file on the local filesystem */
  virtual QString localPath() const { return QString::null; }

  /**
   * The size of the item
   */
  KIO::filesize_t size() const;

  /**
   * \return The number of blocks (2048 bytes) occupied by this item.
   *         This value equals to ceil(size()/2048)
   */
  K3b::Msf blocks() const;

  /** 
   * \returne the dir of the item (or the item itself if it is a dir)
   */
  virtual K3bDirItem* getDirItem() const { return getParent(); }

  virtual void reparent( K3bDirItem* );

  // FIXME: use all these flags and make the isXXX methods
  // non-virtual. Then move the parent()->addDataItem call
  // to the K3bDataItem constructor
  enum ItemFlags {
    DIR = 0x1,
    FILE = 0x2,
    SPECIALFILE = 0x4,
    SYMLINK = 0x8,
    OLD_SESSION = 0x10,
    BOOT_IMAGE = 0x11
  };

  int flags() const;

  virtual bool isDir() const { return false; }
  virtual bool isFile() const { return false; }
  virtual bool isSpecialFile() const { return false; }
  virtual bool isSymLink() const { return false; }	
  virtual bool isFromOldSession() const { return false; }
  bool isBootItem() const;

  bool hideOnRockRidge() const;
  bool hideOnJoliet() const;

  virtual void setHideOnRockRidge( bool b );
  virtual void setHideOnJoliet( bool b );

  virtual long sortWeight() const { return m_sortWeight; }
  virtual void setSortWeight( long w ) { m_sortWeight = w; }

  virtual int depth() const;

  virtual bool isValid() const { return true; }

  // these are all needed for special fileitems like
  // imported sessions or the movix filesystem
  virtual bool isRemoveable() const { return m_bRemoveable; }
  virtual bool isMoveable() const { return m_bMovable; }
  virtual bool isRenameable() const { return m_bRenameable; }
  virtual bool isHideable() const { return m_bHideable; }
  virtual bool writeToCd() const { return m_bWriteToCd; }
  virtual const QString& extraInfo() const { return m_extraInfo; }

  void setRenameable( bool b ) { m_bRenameable = b; }
  void setMoveable( bool b ) { m_bMovable = b; }
  void setRemoveable( bool b ) { m_bRemoveable = b; }
  void setHideable( bool b ) { m_bHideable = b; }
  void setWriteToCd( bool b ) { m_bWriteToCd = b; }
  void setExtraInfo( const QString& i ) { m_extraInfo = i; }

 protected:
  virtual KIO::filesize_t itemSize( bool followSymlinks ) const = 0;

  /**
   * \param followSymlinks If true symlinks will be followed and their
   *                       size equals the size of the file they are
   *                       pointing to.
   *
   * \return The number of blocks (2048 bytes) occupied by this item.
   */
  virtual K3b::Msf itemBlocks( bool followSymlinks ) const;

  QString m_k3bName;

  void setFlags( int flags );

 private:
  class Private;
  Private* d;

  QString m_writtenName;
  QString m_rawIsoName;

  K3bDataDoc* m_doc;
  K3bDirItem* m_parentDir;

  bool m_bHideOnRockRidge;
  bool m_bHideOnJoliet;
  bool m_bRemoveable;
  bool m_bRenameable;
  bool m_bMovable;
  bool m_bHideable;
  bool m_bWriteToCd;
  QString m_extraInfo;

  long m_sortWeight;

  friend class K3bDirItem;
};

#endif
