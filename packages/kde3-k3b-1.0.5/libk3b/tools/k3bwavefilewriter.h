/*
 *
 * $Id: k3bwavefilewriter.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef K3BWAVEFILEWRITER_H
#define K3BWAVEFILEWRITER_H

#include <qstring.h>
#include <qfile.h>
#include <qdatastream.h>
#include "k3b_export.h"
/**
 * @author Sebastian Trueg
 * Creates wave files from 16bit stereo little or big endian
 * sound samples
 */
class LIBK3B_EXPORT K3bWaveFileWriter 
{
 public: 

  enum Endianess { BigEndian, LittleEndian };

  K3bWaveFileWriter();
  ~K3bWaveFileWriter();

  /**
   * open a new wave file.
   * closes any opened file.
   */
  bool open( const QString& filename );

  bool isOpen();
  const QString& filename() const;

  /**
   * closes the file.
   * Length of the wave file will be written into the header.
   * If no data has been written to the file except the header
   * it will be removed.
   */
  void close();

  /**
   * write 16bit samples to the file.
   * @param e the endianess of the data
   *          (it will be swapped to little endian byte order if necessary)
   */
  void write( const char* data, int len, Endianess e = BigEndian );

  /**
   * returnes a filedescriptor with the already opened file
   * or -1 if isOpen() is false
   */
  int fd() const;

 private:
  void writeEmptyHeader();
  void updateHeader();
  void padTo2352();

  QFile m_outputFile;
  QDataStream m_outputStream;
  QString m_filename;
};

#endif
