/* 
 *
 * $Id: k3bmkisofshandler.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef _K3B_MKISOfS_HANDLER_H_
#define _K3B_MKISOfS_HANDLER_H_

#include <qstring.h>

class K3bExternalBin;


/**
 * Derive from this to handle mkisofs.
 */
class K3bMkisofsHandler
{
 public:
  K3bMkisofsHandler();
  virtual ~K3bMkisofsHandler();

  /**
   * \return true if there was a read error.
   */
  bool mkisofsReadError() const;

 protected:
  /**
   * Initialize the MkisofsHandler.
   * This method emits copyright information and an error message in case mkisofs is not installed
   * through handleMkisofsInfoMessage.
   *
   * \return A mkisofs bin object to be used or 0 if mkisofs is not installed.
   */
  const K3bExternalBin* initMkisofs();

  void parseMkisofsOutput( const QString& line );

  /**
   * Used internally by handleMkisofsOutput.
   * May be used in case handleMkisofsOutput is not sufficient.
   */
  int parseMkisofsProgress( const QString& line );

  /**
   * Called by handleMkisofsOutput
   */
  virtual void handleMkisofsProgress( int ) = 0;

  /**
   * Called by handleMkisofsOutput
   *
   * Uses K3bJob::MessageType
   */
  virtual void handleMkisofsInfoMessage( const QString&, int ) = 0;

 private:
  class Private;
  Private* d;
};


#endif
