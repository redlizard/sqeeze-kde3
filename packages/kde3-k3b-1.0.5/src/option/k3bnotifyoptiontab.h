/* 
 *
 * $Id: k3bnotifyoptiontab.h 619556 2007-01-03 17:38:12Z trueg $
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



#ifndef _K3B_NOTIFY_OPTIONTAB_H_
#define _K3B_NOTIFY_OPTIONTAB_H_

#include <qwidget.h>

namespace KNotify {
  class KNotifyWidget;
}


class K3bNotifyOptionTab : public QWidget
{
  Q_OBJECT

 public:
  K3bNotifyOptionTab( QWidget* parent = 0, const char* name = 0 );
  ~K3bNotifyOptionTab();

  void readSettings();
  bool saveSettings();

 private:
  KNotify::KNotifyWidget* m_notifyWidget;
};

#endif
