/*
 *
 * $Id: k3bbootimagedialog.cpp 619556 2007-01-03 17:38:12Z trueg $
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


#include "k3bbootimagedialog.h"
#include "k3bbootimageview.h"

#include <klocale.h>


K3bBootImageDialog::K3bBootImageDialog( K3bDataDoc* doc, 
					QWidget* parent, 
					const char* name, 
					bool modal )
  : KDialogBase( parent, name, modal, i18n("Boot Images"), Ok )
{
  m_bootImageView = new K3bBootImageView( doc, this );
  setMainWidget( m_bootImageView );
}


K3bBootImageDialog::~K3bBootImageDialog()
{
}


void K3bBootImageDialog::slotOk()
{
  //  m_bootImageView->save();
  done( Ok );
}

#include "k3bbootimagedialog.moc"
