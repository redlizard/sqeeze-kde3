/* 
 *
 * $Id: k3bdatamodewidget.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef _K3B_DATAMODE_WIDGET_H_
#define _K3B_DATAMODE_WIDGET_H_

#include <qcombobox.h>


class KConfigBase;


class K3bDataModeWidget : public QComboBox
{
  Q_OBJECT

 public:
  K3bDataModeWidget( QWidget* parent = 0, const char* name = 0 );
  ~K3bDataModeWidget();

  /**
   * returnes K3b::DataMode
   */
  int dataMode() const;

  void saveConfig( KConfigBase* );
  void loadConfig( KConfigBase* );

 public slots:
  void setDataMode( int );
};

#endif
