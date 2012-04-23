/* 
 *
 * $Id: k3bvideodvdburndialog.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef _K3B_VIDEODVD_BURNDIALOG_H_
#define _K3B_VIDEODVD_BURNDIALOG_H_

#include "k3bprojectburndialog.h"


class K3bVideoDvdDoc;
class K3bDataImageSettingsWidget;
class QCheckBox;


class K3bVideoDvdBurnDialog : public K3bProjectBurnDialog
{
 Q_OBJECT

 public:
   K3bVideoDvdBurnDialog( K3bVideoDvdDoc*, QWidget *parent = 0, const char *name = 0, bool modal = true );
   ~K3bVideoDvdBurnDialog();

 protected slots:
   void slotStartClicked();
   void saveSettings();
   void readSettings();

 protected:
   void loadK3bDefaults();
   void loadUserDefaults( KConfigBase* );
   void saveUserDefaults( KConfigBase* );
   void toggleAll();

 private:
   K3bDataImageSettingsWidget* m_imageSettingsWidget;

   QCheckBox* m_checkVerify;

   K3bVideoDvdDoc* m_doc;
};

#endif
