/* 
 *
 * $Id: k3bmixeddoc.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef K3B_MIXED_DOC_H
#define K3B_MIXED_DOC_H

#include <k3bdoc.h>
#include <k3bdatadoc.h>
#include <k3baudiodoc.h>

#include <k3btoc.h>
#include "k3b_export.h"
class QDomDocument;
class QDomElement;
class K3bBurnJob;
//class K3bView;
class QWidget;
class KConfig;


class LIBK3B_EXPORT K3bMixedDoc : public K3bDoc
{
  Q_OBJECT

 public: 
  K3bMixedDoc( QObject* parent = 0 );
  ~K3bMixedDoc();

  QString name() const;

  bool newDocument();

  void setModified( bool m = true );
  bool isModified() const;

  KIO::filesize_t size() const;
  K3b::Msf length() const;

  int numOfTracks() const;

  K3bBurnJob* newBurnJob( K3bJobHandler*, QObject* parent = 0 );

  K3bAudioDoc* audioDoc() const { return m_audioDoc; }
  K3bDataDoc* dataDoc() const { return m_dataDoc; }

  enum MixedType { DATA_FIRST_TRACK,
		   DATA_LAST_TRACK,
		   DATA_SECOND_SESSION };

  int mixedType() const { return m_mixedType; }
  int type() const { return MIXED; }

  void setURL( const KURL& url );

  /**
   * Represent the structure of the doc as CD Table of Contents.
   * Be aware that the length of the data track is just an estimate
   * and needs to be corrected if not specified here.
   *
   * @param dataMode mode of the data track (MODE1 or XA_FORM1)
   * @param dataTrackLength exact length of the dataTrack
   */

  K3bDevice::Toc toToc( int dataMode, const K3b::Msf& dataTrackLength = 0 ) const;

 public slots:
  void setMixedType( MixedType t ) { m_mixedType = t; }
  void addUrls( const KURL::List& urls );

 protected:
  bool loadDocumentData( QDomElement* );
  bool saveDocumentData( QDomElement* );
  QString typeString() const { return "mixed"; }
  
 private:
  K3bDataDoc* m_dataDoc;
  K3bAudioDoc* m_audioDoc;

  int m_mixedType;
};


#endif
