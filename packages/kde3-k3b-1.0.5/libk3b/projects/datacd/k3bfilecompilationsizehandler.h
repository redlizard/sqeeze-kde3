/* 
 *
 * $Id: k3bfilecompilationsizehandler.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef _K3B_FILECOMPILATION_SIZE_HANDLER_H_
#define _K3B_FILECOMPILATION_SIZE_HANDLER_H_


#include <kio/global.h>
#include <k3bmsf.h>

class K3bDataItem;


/**
 * This class maintains a map of indoes and the number
 * of files in the doc that belong to that inode.
 * This way a more accurate size calculation is possible
 *
 * It has to be noted that the sizes of the directories 
 * are only locally true. That means that in some cases 
 * the root directory of the project may show a much 
 * higher size than calculated by this class.
 */
class K3bFileCompilationSizeHandler
{
 public:
  K3bFileCompilationSizeHandler();
  ~K3bFileCompilationSizeHandler();

  /**
   * This does NOT equal blocks() * 2048.
   * This is the sum of the actual file sizes.
   */
  const KIO::filesize_t& size( bool followSymlinks = false ) const;

  /**
   * Number of blocks the files will occupy.
   */
  const K3b::Msf& blocks( bool followSymlinks = false ) const;

  /**
   * This will increase the counter for the inode of
   * the file in url and update the totel size.
   */
  void addFile( K3bDataItem* );

  /**
   * This will decrease the counter for the inode of
   * the file in url and update the totel size.
   */
  void removeFile( K3bDataItem* );

  void clear();

 private:
  class Private;
  Private* d_symlinks;
  Private* d_noSymlinks;
};

#endif
