/*
 *
 * $Id: k3bmkisofshandler.cpp 802340 2008-04-29 07:43:07Z trueg $
 * Copyright (C) 2005 Sebastian Trueg <trueg@k3b.org>
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

#include "k3bmkisofshandler.h"

#include <k3bexternalbinmanager.h>
#include <k3bcore.h>
#include <k3bjob.h>

#include <kdebug.h>
#include <klocale.h>

#include <cmath>



class K3bMkisofsHandler::Private
{
public:
  const K3bExternalBin* mkisofsBin;
  double firstProgressValue;
  bool readError;
};


K3bMkisofsHandler::K3bMkisofsHandler()
{
  d = new Private;
  d->mkisofsBin = 0;
}


K3bMkisofsHandler::~K3bMkisofsHandler()
{
  delete d;
}


bool K3bMkisofsHandler::mkisofsReadError() const
{
  return d->readError;
}


const K3bExternalBin* K3bMkisofsHandler::initMkisofs()
{
  d->mkisofsBin = k3bcore->externalBinManager()->binObject( "mkisofs" );

  if( d->mkisofsBin ) {
    if( !d->mkisofsBin->copyright.isEmpty() )
      handleMkisofsInfoMessage( i18n("Using %1 %2 - Copyright (C) %3")
				.arg("mkisofs").arg(d->mkisofsBin->version).arg(d->mkisofsBin->copyright),
				K3bJob::INFO );

    d->firstProgressValue = -1;
    d->readError = false;
  }
  else {
    kdDebug() << "(K3bMkisofsHandler) could not find mkisofs executable" << endl;
    handleMkisofsInfoMessage( i18n("Mkisofs executable not found."), K3bJob::ERROR );
  }

  return d->mkisofsBin;
}


void K3bMkisofsHandler::parseMkisofsOutput( const QString& line )
{
  if( !line.isEmpty() ) {
    if( line.startsWith( d->mkisofsBin->path ) ) {
      // error or warning
      QString errorLine = line.mid( d->mkisofsBin->path.length() + 2 );
      if( errorLine.startsWith( "Input/output error. Cannot read from" ) ) {
	handleMkisofsInfoMessage( i18n("Read error from file '%1'").arg( errorLine.mid( 38, errorLine.length()-40 ) ),
				  K3bJob::ERROR );
	d->readError = true;
      }
      else if( errorLine.startsWith( "Value too large for defined data type" ) ) {
	handleMkisofsInfoMessage( i18n("Used version of mkisofs does not have large file support."), K3bJob::ERROR );
	handleMkisofsInfoMessage( i18n("Files bigger than 2 GB cannot be handled."), K3bJob::ERROR );
	d->readError = true;
      }
    }
    else if( line.contains( "done, estimate" ) ) {
      int p = parseMkisofsProgress( line );
      if( p != -1 )
	handleMkisofsProgress( p );
    }
    else if( line.contains( "extents written" ) ) {
      handleMkisofsProgress( 100 );
    }
    else if( line.startsWith( "Incorrectly encoded string" ) ) {
      handleMkisofsInfoMessage( i18n("Encountered an incorrectly encoded filename '%1'")
				.arg(line.section( QRegExp("[\\(\\)]"), 1, 1 )), K3bJob::ERROR );
      handleMkisofsInfoMessage( i18n("This may be caused by a system update which changed the local character set."), K3bJob::ERROR );
      handleMkisofsInfoMessage( i18n("You may use convmv (http://j3e.de/linux/convmv/) to fix the filename encoding."), K3bJob::ERROR );
      d->readError = true;
    }
    else if( line.endsWith( "has not an allowable size." ) ) {
      handleMkisofsInfoMessage( i18n("The boot image has an invalid size."), K3bJob::ERROR );
      d->readError = true;
    }
    else if( line.endsWith( "has multiple partitions." ) ) {
      handleMkisofsInfoMessage( i18n("The boot image contains multiple partitions.."), K3bJob::ERROR );
      handleMkisofsInfoMessage( i18n("A hard-disk boot image has to contain a single partition."), K3bJob::ERROR );
      d->readError = true;
    }
    else {
      kdDebug() << "(mkisofs) " << line << endl;
    }
  }
}


int K3bMkisofsHandler::parseMkisofsProgress( const QString& line )
{
  //
  // in multisession mode mkisofs' progress does not start at 0 but at (X+Y)/X
  // where X is the data already on the cd and Y the data to create
  // This is not very dramatic but kind or ugly.
  // We just save the first emitted progress value and to some math ;)
  //

  QString perStr = line;
  perStr.truncate( perStr.find('%') );
  bool ok;
  double p = perStr.toDouble( &ok );
  if( !ok ) {
    kdDebug() << "(K3bMkisofsHandler) Parsing did not work for " << perStr << endl;
    return -1;
  }
  else {
    if( d->firstProgressValue < 0 )
      d->firstProgressValue = p;

    return( (int)::ceil( (p - d->firstProgressValue)*100.0/(100.0 - d->firstProgressValue) ) );
  }
}
