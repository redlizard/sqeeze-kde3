/* 
 *
 * $Id: k3bwritingmodewidget.cpp 621084 2007-01-08 09:17:21Z trueg $
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

#include "k3bwritingmodewidget.h"
#include "k3bmediacache.h"
#include "k3bapplication.h"

#include <k3bglobals.h>

#include <klocale.h>
#include <kconfig.h>

#include <qtooltip.h>
#include <qwhatsthis.h>

static const QString s_autoHelp = i18n("Let K3b select the best-suited mode. This is the recommended selection.");
static const QString s_daoHelp = i18n("<em>Disk At Once</em> or more properly <em>Session At Once</em>. "
				      "The laser is never turned off while writing the CD or DVD. "
				      "This is the preferred mode to write audio CDs since it allows "
				      "pregaps other than 2 seconds. Not all writers support DAO.<br>"
				      "DVD-R(W)s written in DAO provide the best DVD-Video compatibility.");
static const QString s_taoHelp = i18n("<em>Track At Once</em> should be supported by every CD writer. "
				      "The laser will be turned off after every track.<br>"
				      "Most CD writers need this mode for writing multisession CDs.");
// TODO: add something like: "No CD-TEXT writing in TAO mode."

static const QString s_rawHelp = i18n("RAW writing mode. The error correction data is created by the "
				      "software instead of the writer device.<br>"
				      "Try this if your CD writer fails to write in DAO and TAO.");
static const QString s_seqHelp = i18n("Incremental sequential is the default writing mode for DVD-R(W). "
				      "It allows multisession DVD-R(W)s. It only applies to DVD-R(W).");
static const QString s_ovwHelp = i18n("Restricted Overwrite allows to use a DVD-RW just like a DVD-RAM "
				      "or a DVD+RW. The media may just be overwritten. It is not possible "
				      "to write multisession DVD-RWs in this mode but K3b uses growisofs "
				      "to grow an ISO9660 filesystem within the first session, thus allowing "
				      "new files to be added to an already burned disk.");


class K3bWritingModeWidget::Private
{
public:
  // modes set via setSupportedModes
  int supportedModes;

  // filtered modes
  int selectedModes;

  K3bDevice::Device* device;
};


K3bWritingModeWidget::K3bWritingModeWidget( int modes, QWidget* parent, const char* name )
  : K3bIntMapComboBox( parent, name )
{
  init();
  setSupportedModes( modes );
}


K3bWritingModeWidget::K3bWritingModeWidget( QWidget* parent, const char* name )
  : K3bIntMapComboBox( parent, name )
{
  init();
  setSupportedModes( K3b::DAO | K3b::TAO | K3b::RAW );   // default: support all CD-R(W) modes
}


K3bWritingModeWidget::~K3bWritingModeWidget()
{
  delete d;
}


void K3bWritingModeWidget::init()
{
  d = new Private();
  d->device = 0;

  connect( this, SIGNAL(valueChanged(int)), this, SIGNAL(writingModeChanged(int)) );

  QToolTip::add( this, i18n("Select the writing mode to use") );

  initWhatsThisHelp();
}


void K3bWritingModeWidget::initWhatsThisHelp()
{
  addGlobalWhatsThisText( "<p><b>" + i18n("Writing mode") + "</b></p>", 
			  i18n("Be aware that the writing mode is ignored when writing DVD+R(W) since "
			       "there is only one way to write them.")
			  + "<p><i>"
			  + i18n("The selection of writing modes depends on the inserted burning medium.")
			  + "</i>" );
}


int K3bWritingModeWidget::writingMode() const
{
  return selectedValue();
}


void K3bWritingModeWidget::setWritingMode( int m )
{
  if( m & d->selectedModes ) {
    setSelectedValue( m );
  }
  else {
    setCurrentItem( 0 ); // WRITING_MODE_AUTO
  }
}


void K3bWritingModeWidget::setSupportedModes( int m )
{
  d->supportedModes = m|K3b::WRITING_MODE_AUTO;  // we always support the Auto mode
  updateModes();
}


void K3bWritingModeWidget::setDevice( K3bDevice::Device* dev )
{
  d->device = dev;
  updateModes();
}


void K3bWritingModeWidget::updateModes()
{
  // save current mode
  int currentMode = writingMode();

  clear();

  if( d->device )
    d->selectedModes = d->supportedModes & d->device->writingModes();
  else
    d->selectedModes = d->supportedModes;

  insertItem( 0, i18n("Auto"), s_autoHelp );
  if( d->selectedModes & K3b::DAO )
    insertItem( K3b::DAO, i18n("DAO"), s_daoHelp );
  if( d->selectedModes & K3b::TAO )
    insertItem( K3b::TAO, i18n("TAO"), s_taoHelp );
  if( d->selectedModes & K3b::RAW )
    insertItem( K3b::RAW, i18n("RAW"), s_rawHelp );
  if( d->selectedModes & K3b::WRITING_MODE_RES_OVWR )
    insertItem( K3b::WRITING_MODE_RES_OVWR, i18n("Restricted Overwrite"), s_ovwHelp );
  if( d->selectedModes & K3b::WRITING_MODE_INCR_SEQ )
    insertItem( K3b::WRITING_MODE_INCR_SEQ, i18n("Incremental"), s_seqHelp );

  setWritingMode( currentMode );
}


void K3bWritingModeWidget::saveConfig( KConfigBase* c )
{
  switch( writingMode() ) {
  case K3b::DAO:
    c->writeEntry( "writing_mode", "dao" );
    break;
  case K3b::TAO:
    c->writeEntry( "writing_mode", "tao" );
    break;
  case K3b::RAW:
    c->writeEntry( "writing_mode", "raw" );
    break;
  case K3b::WRITING_MODE_INCR_SEQ:
    c->writeEntry( "writing_mode", "incremental" );
    break;
  case K3b::WRITING_MODE_RES_OVWR:
    c->writeEntry( "writing_mode", "overwrite" );
    break;
  default:
    c->writeEntry( "writing_mode", "auto" );
    break;
  }
}

void K3bWritingModeWidget::loadConfig( KConfigBase* c )
{
  QString mode = c->readEntry( "writing_mode" );
  if ( mode == "dao" )
    setWritingMode( K3b::DAO );
  else if( mode == "tao" )
    setWritingMode( K3b::TAO );
  else if( mode == "raw" )
    setWritingMode( K3b::RAW );
  else if( mode == "incremental" )
    setWritingMode( K3b::WRITING_MODE_INCR_SEQ );
  else if( mode == "overwrite" )
    setWritingMode( K3b::WRITING_MODE_RES_OVWR );
  else
    setWritingMode( K3b::WRITING_MODE_AUTO );
}


void K3bWritingModeWidget::determineSupportedModesFromMedium( const K3bMedium& m )
{
  int modes = 0;

  if( m.diskInfo().mediaType() & (K3bDevice::MEDIA_CD_R|K3bDevice::MEDIA_CD_RW) ) {
    modes |= K3b::TAO;
    if( m.device()->supportsWritingMode( K3bDevice::WRITINGMODE_SAO ) )
      modes |= K3b::DAO;
    if( m.device()->supportsWritingMode( K3bDevice::WRITINGMODE_RAW ) )
      modes |= K3b::RAW;
  }

  if( m.diskInfo().mediaType() & K3bDevice::MEDIA_DVD_MINUS_ALL ) {
    modes |= K3b::DAO;
    if( m.device()->featureCurrent( K3bDevice::FEATURE_INCREMENTAL_STREAMING_WRITABLE ) != 0 )
      modes |= K3b::WRITING_MODE_INCR_SEQ;
  }

  if( m.diskInfo().mediaType() & (K3bDevice::MEDIA_DVD_RW|
				  K3bDevice::MEDIA_DVD_RW_SEQ|
				  K3bDevice::MEDIA_DVD_RW_OVWR) )
    modes |= K3b::WRITING_MODE_RES_OVWR;

  setSupportedModes( modes );
  setDevice( m.device() );
}


void K3bWritingModeWidget::determineSupportedModesFromMedium( K3bDevice::Device* dev )
{
  if( dev )
    determineSupportedModesFromMedium( k3bappcore->mediaCache()->medium( dev ) );
  else
    determineSupportedModesFromMedium( K3bMedium() ); // no medium
}

#include "k3bwritingmodewidget.moc"
