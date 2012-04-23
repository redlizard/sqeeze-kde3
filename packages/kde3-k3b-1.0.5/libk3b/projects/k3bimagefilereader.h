/* 
 *
 * $Id: k3bimagefilereader.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef _K3B_IMAGE_FILE_READER_H_
#define _K3B_IMAGE_FILE_READER_H_

#include <qstring.h>
#include "k3b_export.h"

class LIBK3B_EXPORT K3bImageFileReader
{
 public:
  K3bImageFileReader();
  virtual ~K3bImageFileReader();

  /**
   * Open a file. In most cases the TOC file
   */
  void openFile( const QString& filename );

  virtual bool isValid() const;

  /**
   * Return the current set filename;
   */
  const QString& filename() const;

  /**
   * returns the name of the corresponding image file.
   */
  virtual const QString& imageFilename() const;

 protected:
  virtual void readFile() = 0;
  void setValid( bool );
  void setImageFilename( const QString& );

 private:
  class Private;
  Private* d;
};

#endif
