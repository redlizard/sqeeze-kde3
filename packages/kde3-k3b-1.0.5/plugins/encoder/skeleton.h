/* 
 *
 * $Id: skeleton.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef _K3B_<name>_ENCODER_H_
#define _K3B_<name>_ENCODER_H_


#include <k3baudioencoder.h>
#include <k3bpluginconfigwidget.h>


class K3b<name>Encoder : public K3bAudioEncoder
{
  Q_OBJECT

 public:
  K3b<name>Encoder( QObject* parent = 0, const char* name = 0 );
  ~K3b<name>Encoder();

  QStringList extensions() const;
  
  QString fileTypeComment( const QString& ) const;

  long long fileSize( const QString&, const K3b::Msf& msf ) const;

  int pluginSystemVersion() const { return 3; }

  K3bPluginConfigWidget* createConfigWidget( QWidget* parent = 0, 
					     const char* name = 0 ) const;

 private:
  void finishEncoderInternal();
  bool initEncoderInternal( const QString& extension );
  long encodeInternal( const char* data, Q_ULONG len );
  void setMetaDataInternal( MetaDataField, const QString& );
};


class K3b<name>EncoderSettingsWidget : public K3bPluginConfigWidget
{
  Q_OBJECT

 public:
  K3b<name>EncoderSettingsWidget( QWidget* parent = 0, const char* name = 0 );
  ~K3b<name>EncoderSettingsWidget();

 public slots:
  void loadConfig();
  void saveConfig();
};

#endif
