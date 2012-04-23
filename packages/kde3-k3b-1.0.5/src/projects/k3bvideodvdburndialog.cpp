/*
 *
 * $Id: k3bvideodvdburndialog.cpp 619556 2007-01-03 17:38:12Z trueg $
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

#include "k3bvideodvdburndialog.h"
#include "k3bvideodvddoc.h"

#include <k3bdevice.h>
#include <k3bwriterselectionwidget.h>
#include <k3btempdirselectionwidget.h>
#include <k3bcore.h>
#include <k3bwritingmodewidget.h>
#include <k3bglobals.h>
#include <k3bdataimagesettingswidget.h>
#include <k3bisooptions.h>
#include <k3bstdguiitems.h>
#include <k3bglobalsettings.h>

#include <kconfig.h>
#include <klocale.h>
#include <kio/global.h>
#include <kmessagebox.h>

#include <qlayout.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qtooltip.h>
#include <qwhatsthis.h>


K3bVideoDvdBurnDialog::K3bVideoDvdBurnDialog( K3bVideoDvdDoc* doc, QWidget *parent, const char *name, bool modal )
  : K3bProjectBurnDialog( doc, parent, name, modal, true ),
    m_doc( doc )
{
  prepareGui();

  setTitle( i18n("Video DVD Project"), i18n("Size: %1").arg( KIO::convertSize(doc->size()) ) );

  // for now we just put the verify checkbox on the main page...
  m_checkVerify = K3bStdGuiItems::verifyCheckBox( m_optionGroup );
  m_optionGroupLayout->addWidget( m_checkVerify );

  QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  m_optionGroupLayout->addItem( spacer );

  // create image settings tab
  m_imageSettingsWidget = new K3bDataImageSettingsWidget( this );
  m_imageSettingsWidget->showFileSystemOptions( false );

  addPage( m_imageSettingsWidget, i18n("Filesystem") );

  m_tempDirSelectionWidget->setSelectionMode( K3bTempDirSelectionWidget::FILE );

  QString path = m_doc->tempDir();
  if( path.isEmpty() ) {
    path = K3b::defaultTempPath();
    if( m_doc->isoOptions().volumeID().isEmpty() )
      path.append( "image.iso" );
    else
      path.append( m_doc->isoOptions().volumeID() + ".iso" );
  }
  m_tempDirSelectionWidget->setTempPath( path );
}


K3bVideoDvdBurnDialog::~K3bVideoDvdBurnDialog()
{
}


void K3bVideoDvdBurnDialog::saveSettings()
{
  K3bProjectBurnDialog::saveSettings();

  // save iso image settings
  K3bIsoOptions o = m_doc->isoOptions();
  m_imageSettingsWidget->save( o );
  m_doc->setIsoOptions( o );

  // save image file path
  m_doc->setTempDir( m_tempDirSelectionWidget->tempPath() );

  m_doc->setVerifyData( m_checkVerify->isChecked() );
}


void K3bVideoDvdBurnDialog::readSettings()
{
  K3bProjectBurnDialog::readSettings();

  if( !doc()->tempDir().isEmpty() )
    m_tempDirSelectionWidget->setTempPath( doc()->tempDir() );
  else
    m_tempDirSelectionWidget->setTempPath( K3b::defaultTempPath() + doc()->name() + ".iso" );

  m_checkVerify->setChecked( m_doc->verifyData() );

  m_imageSettingsWidget->load( m_doc->isoOptions() );

  // in case overburn is enabled we allow some made up max size 
  // before we force a DL medium
  if( doc()->size() > 4700372992LL &&
      ( !k3bcore->globalSettings()->overburn() ||
	doc()->size() > 4900000000LL ) )
    m_writerSelectionWidget->setWantedMediumType( K3bDevice::MEDIA_WRITABLE_DVD_DL );
  else
    m_writerSelectionWidget->setWantedMediumType( K3bDevice::MEDIA_WRITABLE_DVD );

  toggleAll();
}


void K3bVideoDvdBurnDialog::toggleAll()
{
  K3bProjectBurnDialog::toggleAll();

  if( m_checkSimulate->isChecked() || m_checkOnlyCreateImage->isChecked() ) {
    m_checkVerify->setChecked(false);
    m_checkVerify->setEnabled(false);
  }
  else
    m_checkVerify->setEnabled(true);
}


void K3bVideoDvdBurnDialog::loadK3bDefaults()
{
  K3bProjectBurnDialog::loadK3bDefaults();

  m_imageSettingsWidget->load( K3bIsoOptions::defaults() );
  m_checkVerify->setChecked( false );

  toggleAll();
}


void K3bVideoDvdBurnDialog::loadUserDefaults( KConfigBase* c )
{
  K3bProjectBurnDialog::loadUserDefaults( c );

  K3bIsoOptions o = K3bIsoOptions::load( c );
  m_imageSettingsWidget->load( o );

  m_checkVerify->setChecked( c->readBoolEntry( "verify data", false ) );

  toggleAll();
}


void K3bVideoDvdBurnDialog::saveUserDefaults( KConfigBase* c )
{
  K3bProjectBurnDialog::saveUserDefaults(c);

  K3bIsoOptions o;
  m_imageSettingsWidget->save( o );
  o.save( c );

  c->writeEntry( "verify data", m_checkVerify->isChecked() );
}


void K3bVideoDvdBurnDialog::slotStartClicked()
{
  if( m_checkOnlyCreateImage->isChecked() ||
      m_checkCacheImage->isChecked() ) {
    QFileInfo fi( m_tempDirSelectionWidget->tempPath() );
    if( fi.isDir() )
      m_tempDirSelectionWidget->setTempPath( fi.filePath() + "/image.iso" );

    if( QFile::exists( m_tempDirSelectionWidget->tempPath() ) ) {
      if( KMessageBox::warningContinueCancel( this,
					      i18n("Do you want to overwrite %1?").arg(m_tempDirSelectionWidget->tempPath()),
					      i18n("File Exists"), i18n("Overwrite") )
	  == KMessageBox::Continue ) {
	// delete the file here to avoid problems with free space in K3bProjectBurnDialog::slotStartClicked
	QFile::remove( m_tempDirSelectionWidget->tempPath() );
      }
      else
	return;
    }
  }

  K3bProjectBurnDialog::slotStartClicked();
}

#include "k3bvideodvdburndialog.moc"
