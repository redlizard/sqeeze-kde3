/* 
 *
 * $Id: k3bprojectinterface.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef _K3B_PROJECT_INTERFACE_H_
#define _K3B_PROJECT_INTERFACE_H_

#include <dcopobject.h>
#include <kio/global.h>
#include <qstringlist.h>

class K3bDoc;

/**
 * Base class for all project interfaces
 */
class K3bProjectInterface : public DCOPObject
{
  K_DCOP

 public:
  K3bProjectInterface( K3bDoc*, const char* name = 0 );
  virtual ~K3bProjectInterface();

  // Generate a name for this interface. Automatically used if name=0 is
  // passed to the constructor
  static QCString newIfaceName();

 k_dcop:
  virtual void addUrls( const QStringList& urls );
  virtual void addUrl( const QString& url );

  /**
   * Opens the burn dialog
   */
  virtual void burn();

  /**
   * Starts the burning immedeately
   * \return true if the burning could be started. Be aware that the return
   *         value does not say anything about the success of the burning
   *         process.
   */
  virtual bool directBurn();

  virtual void setBurnDevice( const QString& blockdevicename );

  /**
   * \return the length of the project in blocks (frames).
   */
  virtual int length() const;

  /**
   * \return size of the project in bytes.
   */
  virtual KIO::filesize_t size() const;

  virtual const QString& imagePath() const;

  /**
   * \return A string representation of the project type. One of:
   * \li "datacd" - Data CD
   * \li "audiocd" - Audio CD
   * \li "mixedcd" - Mixed Mode CD
   * \li "videocd" - Video
   * \li "emovixcd" - eMovix CD
   * \li "datadvd" - Data DVD
   * \li "videodvd" - Video DVD
   * \li "emovixdvd" - eMovix DVD
   *
   * Be aware that this is not the same as K3bDoc::documentType for historical reasons.
   */
  virtual QString projectType() const;

 private:
  K3bDoc* m_doc;
};

#endif
