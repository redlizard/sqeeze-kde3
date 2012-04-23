/* 
 *
 * $Id: k3bmovixdocpreparer.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef _K3B_MOVIX_DOC_PREPARER_H_
#define _K3B_MOVIX_DOC_PREPARER_H_

#include <k3bjob.h>

class K3bMovixDoc;
class K3bFileItem;
class K3bDirItem;


/**
 * This class creates the needed eMovix structures in an eMovix doc
 * and removes them after creating the image.
 */
class K3bMovixDocPreparer : public K3bJob
{
  Q_OBJECT

 public:
  explicit K3bMovixDocPreparer( K3bMovixDoc* doc, K3bJobHandler*, QObject* parent = 0, const char* name = 0 );
  ~K3bMovixDocPreparer();

  K3bMovixDoc* doc() const;

  bool createMovixStructures();
  void removeMovixStructures();

 public slots:
  /**
   * use createMovixStructures and removeMovixStructures instead.
   */
  void start();

  /**
   * Useless since this job works syncronously
   */
  void cancel();

 private:
  bool writePlaylistFile();
  bool writeIsolinuxConfigFile( const QString& );
  bool writeMovixRcFile();
  bool addMovixFiles();
  bool addMovixFilesNew();
  K3bFileItem* createItem( const QString& localPath, const QString& docPath );
  K3bDirItem* createDir( const QString& docPath );

  class Private;
  Private* d;
};

#endif
