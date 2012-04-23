/* 
 *
 * $Id: k3bmovixdvddoc.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef _K3B_MOVIX_DVD_DOC_H_
#define _K3B_MOVIX_DVD_DOC_H_

#include <k3bmovixdoc.h>
#include "k3b_export.h"
class KConfig;


class LIBK3B_EXPORT K3bMovixDvdDoc : public K3bMovixDoc
{
  Q_OBJECT

 public:
  K3bMovixDvdDoc( QObject* parent = 0 );
  ~K3bMovixDvdDoc();

  int type() const { return MOVIX_DVD; }

  K3bBurnJob* newBurnJob( K3bJobHandler* hdl, QObject* parent );

 protected:
  QString typeString() const { return "movixdvd"; }
};

#endif
