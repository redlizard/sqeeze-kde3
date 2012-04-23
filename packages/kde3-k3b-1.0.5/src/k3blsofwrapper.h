/* 
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 * Copyright (C) 2006 Sebastian Trueg <trueg@k3b.org>
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

#ifndef _K3B_LSOF_WRAPPER_H_
#define _K3B_LSOF_WRAPPER_H_

#include <qvaluelist.h>

namespace K3bDevice {
  class Device;
}


class K3bLsofWrapper
{
 public:
  K3bLsofWrapper();
  ~K3bLsofWrapper();

  /**
   * Checks which processes currently have an open file descriptor
   * to the device.
   *
   * \return true if lsof was successfully called.
   */
  bool checkDevice( K3bDevice::Device* );

  struct Process {
    QString name;
    int pid;
  };

  /**
   * \return A list of all applications that had an open
   * handle on the device used in the last successful call
   * to checkDevice.
   */
  const QValueList<Process>& usingApplications() const;

 private:
  bool findLsofExecutable();

  class Private;
  Private* d;
};

#endif
 
