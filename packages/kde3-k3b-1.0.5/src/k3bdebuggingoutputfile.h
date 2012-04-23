/* 
 *
 * $Id: k3bdebuggingoutputfile.h 619556 2007-01-03 17:38:12Z trueg $
 * Copyright (C) 2005 Sebastian Trueg <trueg@k3b.org>
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

#ifndef _K3B_DEBUGGING_OUTPUT_FILE_H_
#define _K3B_DEBUGGING_OUTPUT_FILE_H_

#include <qfile.h>
#include <qobject.h>

class K3bDebuggingOutputFile : public QObject, public QFile
{
  Q_OBJECT

 public:
  K3bDebuggingOutputFile();

  /**
   * Open the default output file and write some system information.
   */
  bool open();

 public slots:
  void addOutput( const QString&, const QString& );
};


#endif
