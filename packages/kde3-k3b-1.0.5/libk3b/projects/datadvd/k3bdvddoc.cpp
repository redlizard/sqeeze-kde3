/* 
 *
 * $Id: k3bdvddoc.cpp 619556 2007-01-03 17:38:12Z trueg $
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


#include "k3bdvddoc.h"
#include "k3bdvdjob.h"

#include <k3bisooptions.h>

#include <kconfig.h>


K3bDvdDoc::K3bDvdDoc( QObject* parent )
  : K3bDataDoc( parent )
{
}

K3bDvdDoc::~K3bDvdDoc()
{
}

K3bBurnJob* K3bDvdDoc::newBurnJob( K3bJobHandler* hdl, QObject* parent )
{
  return new K3bDvdJob( this, hdl, parent );
}

//#include "k3bdvddoc.moc"
