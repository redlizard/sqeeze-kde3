/* 
 *
 * $Id: k3baudioencoder.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef _K3B_AUDIO_ENCODER_H_
#define _K3B_AUDIO_ENCODER_H_

#include <k3bplugin.h>

#include <k3bmsf.h>
#include "k3b_export.h"


/**
 * The base class for all audio encoders.
 * Do not be alarmed by the number of methods since most of them
 * do not need to be touched. They are just there to keep the API 
 * clean and extendable.
 *
 * see the skeleton files for further help.
 */
class LIBK3B_EXPORT K3bAudioEncoder : public K3bPlugin
{
  Q_OBJECT

 public:
  K3bAudioEncoder( QObject* parent = 0, const char* name = 0 );
  virtual ~K3bAudioEncoder();

  // TODO: if the following methods are to be activated the config methods in
  //       K3bPluginConfigWidget also need to be changed since they do not allow
  //       to use an extern config object yet.
  //       Perhaps these two methods should even go into K3bPlugin.
  /**
   * This calls readConfig using the k3bcore config object
   */
  // void readConfig();

  /**
   * Force the plugin to read it's configuration
   */
  // virtual void readConfig( KConfig* );

  QString group() const { return "AudioEncoder"; }

  /**
   * This should return the fileextensions supported by the filetype written in the
   * encoder.
   * May return an empty list in which case the encoder will not be usable (this may come
   * in handy if the encoder is based on some external program or lib which is not 
   * available on runtime.)
   */
  virtual QStringList extensions() const = 0;

  /**
   * The filetype as presented to the user.
   */
  virtual QString fileTypeComment( const QString& extension ) const = 0;

  /**
   * Determine the filesize of the encoded file (~)
   * default implementation returnes -1 (unknown)
   * First parameter is the extension to be used
   */
  virtual long long fileSize( const QString&, const K3b::Msf& ) const { return -1; }

  /**
   * The default implementation openes the file for writing with 
   * writeData. Normally this does not need to be reimplemented.
   * @param extension the filetype to be used.
   *
   */
  virtual bool openFile( const QString& extension, const QString& filename, const K3b::Msf& length );


  /**
   * The default implementation returnes true if openFile (default implementation) has been
   * successfully called. Normally this does not need to be reimplemented but it has to be 
   * if openFile is reimplemented.
   */
  virtual bool isOpen() const;

  /**
   * The default implementation closes the file opened by openFile
   * (default implementation) 
   * Normally this does not need to be reimplemented but it has to be 
   * if openFile is reimplemented.
   */
  virtual void closeFile();

  /**
   * The default implementation returnes the filename set in openFile
   * or QString::null if no file has been opened.
   * Normally this does not need to be reimplemented but it has to be 
   * if openFile is reimplemented.
   */
  virtual const QString& filename() const;

  enum MetaDataField {
    META_TRACK_TITLE,
    META_TRACK_ARTIST,
    META_TRACK_COMMENT,
    META_TRACK_NUMBER,
    META_ALBUM_TITLE,
    META_ALBUM_ARTIST,
    META_ALBUM_COMMENT,
    META_YEAR,
    META_GENRE };

  /**
   * Calling this method does only make sense after successfully
   * calling openFile and before calling encode.
   * This calls setMetaDataInternal.
   */
  void setMetaData( MetaDataField, const QString& );

  /**
   * Returnes the amount of actually written bytes or -1 if an error
   * occurred.
   *
   * Be aware that the returned amount of written data may very well differ
   * from len since the data is encoded.
   */
  long encode( const char*, Q_ULONG len );

  /**
   * Use this signal in case of an error to provide the user with information
   * about the problem.
   */
  virtual QString lastErrorString() const;

 protected:
  /**
   * Called by the default implementation of openFile
   * This calls initEncoderInternal.
   */
  bool initEncoder( const QString& extension, const K3b::Msf& length );

  /**
   * Called by the deafult implementation of openFile
   * This calls finishEncoderInternal.
   */
  void finishEncoder();

  /**
   * Use this to write the data to the file when
   * using the default implementation of openFile
   * Returnes the number of bytes actually written.
   */
  Q_LONG writeData( const char*, Q_ULONG len );

  /**
   * initzialize the decoder structures.
   * default implementation does nothing
   * this may already write data.
   */
  virtual bool initEncoderInternal( const QString& extension, const K3b::Msf& length );

  /**
   * reimplement this if the encoder needs to do some
   * finishing touch.
   */
  virtual void finishEncoderInternal();

  /**
   * encode the data and write it with writeData (when using
   * the default)
   * The data will always be 16bit 44100 Hz stereo little endian samples.
   * Should return the amount of actually written bytes (may be 0) and -1
   * on error.
   */
  // TODO: use Q_INT16* instead of char*
  // FIXME: why little endian while CDs use big endian???
  virtual long encodeInternal( const char*, Q_ULONG len ) = 0;

  /**
   * default implementation does nothing
   * this may already write data.
   */
  virtual void setMetaDataInternal( MetaDataField, const QString& );

  /**
   * Use this in combination with the default implementation of lastError()
   */
  void setLastError( const QString& );

 private:
  class Private;
  Private* d;
};

#endif
