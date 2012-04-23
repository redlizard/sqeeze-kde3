/*
 *
 * $Id: k3bdvdcopydialog.cpp 733470 2007-11-06 12:10:29Z trueg $
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

#include "k3bdvdcopydialog.h"
#include "k3bdvdcopyjob.h"

#include <k3btempdirselectionwidget.h>
#include <k3bwriterselectionwidget.h>
#include <k3bglobals.h>
#include <k3bstdguiitems.h>
#include <k3bmediaselectioncombobox.h>
#include <k3bdevice.h>
#include <k3bdevicemanager.h>
#include <k3bexternalbinmanager.h>
#include <k3bburnprogressdialog.h>
#include <k3bwritingmodewidget.h>
#include <k3bthememanager.h>
#include <k3bapplication.h>
#include <k3bmediacache.h>
#include <k3biso9660.h>

#include <qlayout.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qtabwidget.h>
#include <qspinbox.h>
#include <qptrlist.h>
#include <qfile.h>
#include <qpushbutton.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qhbox.h>

#include <kdebug.h>
#include <klocale.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kglobal.h>



K3bDvdCopyDialog::K3bDvdCopyDialog( QWidget* parent, const char* name, bool modal )
  : K3bInteractionDialog( parent, name,
			  i18n("DVD Copy"),
			  i18n("No video transcoding!"),
			  START_BUTTON|CANCEL_BUTTON,
			  START_BUTTON,
			  "default dvd copy settings",
			  modal )
{
  QWidget* w = mainWidget();

  //
  // Source group
  // //////////////////////////////////////////////////////////////////////////
  QGroupBox* groupSource = new QGroupBox( 1, Qt::Vertical, i18n("Source Medium"), w );
  groupSource->setInsideSpacing( spacingHint() );
  groupSource->setInsideMargin( marginHint() );
  m_comboSourceDevice = new K3bMediaSelectionComboBox( groupSource );
  m_comboSourceDevice->setWantedMediumType( K3bDevice::MEDIA_WRITABLE_DVD|K3bDevice::MEDIA_DVD_ROM );
  m_comboSourceDevice->setWantedMediumState( K3bDevice::STATE_COMPLETE|K3bDevice::STATE_INCOMPLETE );
  // //////////////////////////////////////////////////////////////////////////

  //
  // Writer group
  // //////////////////////////////////////////////////////////////////////////
  m_writerSelectionWidget = new K3bWriterSelectionWidget( w );
  m_writerSelectionWidget->setSupportedWritingApps( K3b::GROWISOFS );
  m_writerSelectionWidget->setWantedMediumType( K3bDevice::MEDIA_WRITABLE_DVD );
  m_writerSelectionWidget->setWantedMediumState( K3bDevice::STATE_EMPTY );
  // //////////////////////////////////////////////////////////////////////////

  //
  // Option tab
  // //////////////////////////////////////////////////////////////////////////
  QTabWidget* tabWidget = new QTabWidget( w );

  // option tab --------------------
  QWidget* optionTab = new QWidget( tabWidget );
  QGridLayout* optionTabGrid = new QGridLayout( optionTab );
  optionTabGrid->setSpacing( spacingHint() );
  optionTabGrid->setMargin( marginHint() );

  QGroupBox* groupWritingMode = new QGroupBox( 1, Qt::Vertical, i18n("Writing Mode"), optionTab );
  groupWritingMode->setInsideMargin( marginHint() );
  m_writingModeWidget = new K3bWritingModeWidget( groupWritingMode );

  QGroupBox* groupOptions = new QGroupBox( 5, Qt::Vertical, i18n("Settings"), optionTab );
  groupOptions->setInsideSpacing( spacingHint() );
  groupOptions->setInsideMargin( marginHint() );
  m_checkSimulate = K3bStdGuiItems::simulateCheckbox( groupOptions );
  m_checkCacheImage = K3bStdGuiItems::createCacheImageCheckbox( groupOptions );
  m_checkOnlyCreateImage = K3bStdGuiItems::onlyCreateImagesCheckbox( groupOptions );
  m_checkDeleteImages = K3bStdGuiItems::removeImagesCheckbox( groupOptions );
  m_checkVerifyData = K3bStdGuiItems::verifyCheckBox( groupOptions );

  QGroupBox* groupCopies = new QGroupBox( 2, Qt::Horizontal, i18n("Copies"), optionTab );
  groupCopies->setInsideSpacing( spacingHint() );
  groupCopies->setInsideMargin( marginHint() );
  QLabel* pixLabel = new QLabel( groupCopies );
  pixLabel->setPixmap( SmallIcon( "cdcopy", KIcon::SizeMedium ) );
  pixLabel->setScaledContents( false );
  m_spinCopies = new QSpinBox( groupCopies );
  m_spinCopies->setMinValue( 1 );
  m_spinCopies->setMaxValue( 999 );

  optionTabGrid->addWidget( groupWritingMode, 0, 0 );
  optionTabGrid->addMultiCellWidget( groupOptions, 0, 1, 1, 1 );
  optionTabGrid->addWidget( groupCopies, 1, 0 );
  optionTabGrid->setRowStretch( 1, 1 );
  optionTabGrid->setColStretch( 1, 1 );

  tabWidget->addTab( optionTab, i18n("&Options") );


  //
  // Image tab
  // //////////////////////////////////////////////////////////////////////////
  QWidget* imageTab = new QWidget( tabWidget );
  QGridLayout* imageTabGrid = new QGridLayout( imageTab );
  imageTabGrid->setSpacing( spacingHint() );
  imageTabGrid->setMargin( marginHint() );

  m_tempDirSelectionWidget = new K3bTempDirSelectionWidget( imageTab );
  m_tempDirSelectionWidget->setSelectionMode( K3bTempDirSelectionWidget::FILE );

  imageTabGrid->addWidget( m_tempDirSelectionWidget, 0, 0 );

  tabWidget->addTab( imageTab, i18n("&Image") );


  //
  // advanced tab ------------------
  // //////////////////////////////////////////////////////////////////////////
  QWidget* advancedTab = new QWidget( tabWidget );
  QGridLayout* advancedTabGrid = new QGridLayout( advancedTab );
  advancedTabGrid->setSpacing( spacingHint() );
  advancedTabGrid->setMargin( marginHint() );

  QGroupBox* groupGeneral = new QGroupBox( 2, Qt::Vertical, i18n("General"), advancedTab );
  groupGeneral->setInsideSpacing( spacingHint() );
  groupGeneral->setInsideMargin( marginHint() );
  QHBox* box = new QHBox( groupGeneral );
  box->setSpacing( spacingHint() );
  box->setStretchFactor( new QLabel( i18n("Read retries:"), box ), 1 );
  m_spinRetries = new QSpinBox( 1, 128, 1, box );
  m_checkIgnoreReadErrors = new QCheckBox( i18n("Ignore read errors"), groupGeneral );

  advancedTabGrid->addWidget( groupGeneral, 0, 0 );

  tabWidget->addTab( advancedTab, i18n("&Advanced") );
  // //////////////////////////////////////////////////////////////////////////


  //
  // setup layout
  // //////////////////////////////////////////////////////////////////////////
  QGridLayout* grid = new QGridLayout( w );
  grid->setMargin( 0 );
  grid->setSpacing( spacingHint() );

  grid->addWidget( groupSource, 0, 0 );
  grid->addWidget( m_writerSelectionWidget, 1, 0 );
  grid->addWidget( tabWidget, 2, 0 );
  grid->setRowStretch( 2, 1 );
  // //////////////////////////////////////////////////////////////////////////


  // tab order
  setTabOrder( m_writingModeWidget, m_spinCopies );
  setTabOrder( m_spinCopies, groupOptions );


  //
  // setup connections
  // //////////////////////////////////////////////////////////////////////////
  connect( m_writerSelectionWidget, SIGNAL(writerChanged()), this, SLOT(slotToggleAll()) );
  connect( m_comboSourceDevice, SIGNAL(selectionChanged(K3bDevice::Device*)), this, SLOT(slotToggleAll()) );
  connect( m_comboSourceDevice, SIGNAL(selectionChanged(K3bDevice::Device*)),
	   this, SLOT(slotSourceMediumChanged(K3bDevice::Device*)) );
  connect( m_checkSimulate, SIGNAL(toggled(bool)), this, SLOT(slotToggleAll()) );
  connect( m_checkCacheImage, SIGNAL(toggled(bool)), this, SLOT(slotToggleAll()) );
  connect( m_checkOnlyCreateImage, SIGNAL(toggled(bool)), this, SLOT(slotToggleAll()) );
  connect( m_writingModeWidget, SIGNAL(writingModeChanged(int)), this, SLOT(slotToggleAll()) );

  QToolTip::add( m_checkIgnoreReadErrors, i18n("Skip unreadable sectors") );
  QWhatsThis::add( m_checkIgnoreReadErrors, i18n("<p>If this option is checked and K3b is not able to read a sector from the "
						 "source CD/DVD it will be replaced with zeros on the resulting copy.") );
}


K3bDvdCopyDialog::~K3bDvdCopyDialog()
{
}


void K3bDvdCopyDialog::init()
{
  slotSourceMediumChanged( m_comboSourceDevice->selectedDevice() );
}


void K3bDvdCopyDialog::setReadingDevice( K3bDevice::Device* dev )
{
  m_comboSourceDevice->setSelectedDevice( dev );
}


K3bDevice::Device* K3bDvdCopyDialog::readingDevice() const
{
  return m_comboSourceDevice->selectedDevice();
}


void K3bDvdCopyDialog::slotStartClicked()
{
  //
  // Let's check the available size
  //
  if( m_checkCacheImage->isChecked() || m_checkOnlyCreateImage->isChecked() ) {
    if( neededSize()/1024 > m_tempDirSelectionWidget->freeTempSpace() ) {
      if( KMessageBox::warningContinueCancel( this, i18n("There seems to be not enough free space in temporary directory. "
							 "Write anyway?") ) == KMessageBox::Cancel )
	return;
    }
  }

  //
  // The job asks if we want to overwrite.
  //

  K3bJobProgressDialog* dlg = 0;
  if( m_checkOnlyCreateImage->isChecked() ) {
    dlg = new K3bJobProgressDialog( kapp->mainWidget() );
  }
  else {
    dlg = new K3bBurnProgressDialog( kapp->mainWidget() );
  }

  K3bDvdCopyJob* job = new K3bDvdCopyJob( dlg, this );

  job->setWriterDevice( m_writerSelectionWidget->writerDevice() );
  job->setReaderDevice( m_comboSourceDevice->selectedDevice() );
  job->setImagePath( m_tempDirSelectionWidget->tempPath() );
  job->setRemoveImageFiles( m_checkDeleteImages->isChecked() && !m_checkOnlyCreateImage->isChecked() );
  job->setOnlyCreateImage( m_checkOnlyCreateImage->isChecked() );
  job->setSimulate( m_checkSimulate->isChecked() );
  job->setOnTheFly( !m_checkCacheImage->isChecked() );
  job->setWriteSpeed( m_writerSelectionWidget->writerSpeed() );
  job->setCopies( m_checkSimulate->isChecked() ? 1 : m_spinCopies->value() );
  job->setWritingMode( m_writingModeWidget->writingMode() );
  job->setIgnoreReadErrors( m_checkIgnoreReadErrors->isChecked() );
  job->setReadRetries( m_spinRetries->value() );
  job->setVerifyData( m_checkVerifyData->isChecked() );

  if( !exitLoopOnHide() )
    hide();

  dlg->startJob( job );

  delete dlg;
  delete job;

  if( KConfigGroup( k3bcore->config(), "General Options" ).readBoolEntry( "keep action dialogs open", false ) &&
      !exitLoopOnHide() )
    show();
  else
    close();
}


void K3bDvdCopyDialog::loadUserDefaults( KConfigBase* c )
{
  m_comboSourceDevice->setSelectedDevice( k3bcore->deviceManager()->findDevice( c->readEntry( "source_device" ) ) );

  m_writerSelectionWidget->loadConfig( c );

  m_writingModeWidget->loadConfig( c );

  m_tempDirSelectionWidget->readConfig( c );

  m_checkSimulate->setChecked( c->readBoolEntry( "simulate", false ) );
  m_checkCacheImage->setChecked( !c->readBoolEntry( "on_the_fly", false ) );
  m_checkOnlyCreateImage->setChecked( c->readBoolEntry( "only_create_image", false ) );
  m_checkDeleteImages->setChecked( c->readBoolEntry( "remove_image", true ) );
  m_checkIgnoreReadErrors->setChecked( c->readBoolEntry( "ignore read errors", false ) );
  m_spinRetries->setValue( c->readNumEntry( "retries", 128 ) );
  m_spinCopies->setValue( c->readNumEntry( "copies", 1 ) );
  m_checkVerifyData->setChecked( c->readBoolEntry( "verify data", false ) );

  slotToggleAll();
}


void K3bDvdCopyDialog::saveUserDefaults( KConfigBase* c )
{
  m_tempDirSelectionWidget->saveConfig();

  m_writingModeWidget->saveConfig( c );
  m_tempDirSelectionWidget->saveConfig( c );

  c->writeEntry( "source_device", m_comboSourceDevice->selectedDevice() ? m_comboSourceDevice->selectedDevice()->devicename() : QString() );

  c->writeEntry( "simulate", m_checkSimulate->isChecked() );
  c->writeEntry( "on_the_fly", !m_checkCacheImage->isChecked() );
  c->writeEntry( "only_create_image", m_checkOnlyCreateImage->isChecked() );
  c->writeEntry( "remove_image", m_checkDeleteImages->isChecked() );
  c->writeEntry( "ignore read errors", m_checkIgnoreReadErrors->isChecked() );
  c->writeEntry( "retries", m_spinRetries->value() );
  c->writeEntry( "copies", m_spinCopies->value() );
  c->writeEntry( "verify data", m_checkVerifyData->isChecked() );

  m_writerSelectionWidget->saveConfig( c );

  if( m_tempDirSelectionWidget->isEnabled() )
    m_tempDirSelectionWidget->saveConfig();
}


void K3bDvdCopyDialog::loadK3bDefaults()
{
  m_writerSelectionWidget->loadDefaults();
  m_tempDirSelectionWidget->setTempPath( K3b::defaultTempPath() );

  m_writingModeWidget->setWritingMode( K3b::WRITING_MODE_AUTO );

  m_checkSimulate->setChecked( false );
  m_checkCacheImage->setChecked( true );
  m_checkOnlyCreateImage->setChecked( false );
  m_checkDeleteImages->setChecked( true );
  m_checkIgnoreReadErrors->setChecked(false);
  m_spinCopies->setValue( 1 );
  m_spinRetries->setValue(128);
  m_checkVerifyData->setChecked( false );

  slotToggleAll();
}


void K3bDvdCopyDialog::toggleAll()
{
  updateOverrideDevice();

  m_checkSimulate->setDisabled( m_checkOnlyCreateImage->isChecked() );
  m_checkCacheImage->setDisabled( m_checkOnlyCreateImage->isChecked() );

  K3bDevice::Device* dev = m_writerSelectionWidget->writerDevice();
  if( dev ) {
    // select the proper writing modes
    // if writing and reading devices are the same we cannot use
    // K3bWritingModeWidget::determineSupportedModesFromMedium since the inserted medium is not the one we
    // will be using for burning. In that case we go the old fashioned way.
    if( dev == m_comboSourceDevice->selectedDevice() ) {
      int modes = 0;
      if( dev->type() & (K3bDevice::DVDR|K3bDevice::DVDRW) ) {
	modes |= K3b::DAO|K3b::WRITING_MODE_RES_OVWR;
	if( dev->featureCurrent( K3bDevice::FEATURE_INCREMENTAL_STREAMING_WRITABLE ) != 0 )
	  modes |= K3b::WRITING_MODE_INCR_SEQ;
      }

      m_writingModeWidget->setSupportedModes( modes );
      m_checkSimulate->setDisabled( m_checkOnlyCreateImage->isChecked() );
    }
    else {
      m_writingModeWidget->determineSupportedModesFromMedium( dev );

      if( k3bappcore->mediaCache()->diskInfo( dev ).mediaType() & K3bDevice::MEDIA_DVD_PLUS_ALL ) {
	// no simulation support for DVD+R(W) media
	m_checkSimulate->setChecked(false);
	m_checkSimulate->setEnabled(false);
      }
      else {
	m_checkSimulate->setDisabled( m_checkOnlyCreateImage->isChecked() );
      }
    }
  }



  m_writingModeWidget->setDisabled( m_checkOnlyCreateImage->isChecked() );
  m_writerSelectionWidget->setDisabled( m_checkOnlyCreateImage->isChecked() );
  m_tempDirSelectionWidget->setDisabled( !m_checkCacheImage->isChecked() && !m_checkOnlyCreateImage->isChecked() );
  m_writingModeWidget->setDisabled( m_checkOnlyCreateImage->isChecked() );
  m_checkDeleteImages->setDisabled( m_checkOnlyCreateImage->isChecked() || !m_checkCacheImage->isChecked() );
  m_spinCopies->setDisabled( m_checkSimulate->isChecked() || m_checkOnlyCreateImage->isChecked() );
  m_checkVerifyData->setDisabled( m_checkOnlyCreateImage->isChecked() || m_checkSimulate->isChecked() );

  setButtonEnabled( START_BUTTON, m_comboSourceDevice->selectedDevice() &&
		    (dev || m_checkOnlyCreateImage->isChecked()) );
}


void K3bDvdCopyDialog::slotSourceMediumChanged( K3bDevice::Device* dev )
{
  updateOverrideDevice();

  K3bMedium medium = k3bappcore->mediaCache()->medium( dev );

  m_writerSelectionWidget->setWantedMediumType( k3bappcore->mediaCache()->diskInfo( dev ).numLayers() > 1 &&
						k3bappcore->mediaCache()->diskInfo( dev ).size().mode1Bytes() > 4700372992LL
						? K3bDevice::MEDIA_WRITABLE_DVD_DL
						: K3bDevice::MEDIA_WRITABLE_DVD_SL );

  m_tempDirSelectionWidget->setNeededSize( neededSize() );

  m_tempDirSelectionWidget->setDefaultImageFileName( medium.volumeId().lower() + ".iso" );

  toggleAll();
}


void K3bDvdCopyDialog::updateOverrideDevice()
{
    if( !m_checkCacheImage->isChecked() ) {
        m_writerSelectionWidget->setOverrideDevice( 0 );
        m_writerSelectionWidget->setIgnoreDevice( m_comboSourceDevice->selectedDevice() );
    }
    else {
        m_writerSelectionWidget->setIgnoreDevice( 0 );
        m_writerSelectionWidget->setOverrideDevice( m_comboSourceDevice->selectedDevice(),
                                                    i18n("Use the same device for burning"),
                                                    i18n("<qt>Use the same device for burning <i>(Or insert another medium)</i>") );
    }
}


KIO::filesize_t K3bDvdCopyDialog::neededSize() const
{
  K3bMedium medium = k3bappcore->mediaCache()->medium( m_comboSourceDevice->selectedDevice() );

  if( medium.diskInfo().diskState() == K3bDevice::STATE_NO_MEDIA )
    return 0;
  else if( medium.diskInfo().mediaType() & (K3bDevice::MEDIA_DVD_RW_OVWR|K3bDevice::MEDIA_DVD_PLUS_RW) )
    return (KIO::filesize_t)medium.iso9660Descriptor().volumeSpaceSize * (KIO::filesize_t)2048;
  else
    return medium.diskInfo().size().mode1Bytes();
}

#include "k3bdvdcopydialog.moc"
