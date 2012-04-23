/* 
 *
 * $Id: k3bdatavolumedescwidget.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef K3B_DATA_VOLUMEDESC_WIDGET_H
#define K3B_DATA_VOLUMEDESC_WIDGET_H


#include "base_k3bdatavolumedescwidget.h"

class K3bIsoOptions;


class K3bDataVolumeDescWidget : public base_K3bDataVolumeDescWidget
{
  Q_OBJECT

 public:
  K3bDataVolumeDescWidget( QWidget* parent = 0, const char* name =  0 );
  ~K3bDataVolumeDescWidget();

  void load( const K3bIsoOptions& );
  void save( K3bIsoOptions& );

 private slots:
  void slotVolumeSetSizeChanged( int );
};

#endif
