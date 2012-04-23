/*
 *
 * $Id: k3bwriterselectionwidget.cpp 690635 2007-07-21 16:47:29Z trueg $
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


#include "k3bwriterselectionwidget.h"
#include "k3bapplication.h"
#include "k3bmediacache.h"

#include <k3bmediaselectioncombobox.h>
#include <k3bdevice.h>
#include <k3bdevicemanager.h>
#include <k3bglobals.h>
#include <k3bcore.h>

#include <klocale.h>
#include <kdialog.h>
#include <kconfig.h>
#include <kcombobox.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kinputdialog.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qtooltip.h>
#include <qtoolbutton.h>
#include <qwhatsthis.h>
#include <qmap.h>
#include <qptrvector.h>
#include <qcursor.h>
#include <qapplication.h>


class K3bWriterSelectionWidget::MediaSelectionComboBox : public K3bMediaSelectionComboBox
{
public:
  MediaSelectionComboBox( QWidget* parent )
    : K3bMediaSelectionComboBox( parent ),
      m_overrideDevice( 0 ) {
  }

  void setOverrideDevice( K3bDevice::Device* dev, const QString& s, const QString& t ) {
    m_overrideDevice = dev;
    m_overrideString = s;
    m_overrideToolTip = t;
    updateMedia();
  }

  K3bDevice::Device* overrideDevice() const {
    return m_overrideDevice;
  }

 protected:
  bool showMedium( const K3bMedium& m ) const {
    return ( m.device() == m_overrideDevice ||
	     K3bMediaSelectionComboBox::showMedium( m ) );
  }

  QString mediumString( const K3bMedium& m ) const {
    if( m.device() == m_overrideDevice )
      return m_overrideString;
    else
      return K3bMediaSelectionComboBox::mediumString( m );
  }

  QString mediumToolTip( const K3bMedium& m ) const {
    if( m.device() == m_overrideDevice )
      return m_overrideToolTip;
    else {
      QString s = K3bMediaSelectionComboBox::mediumToolTip( m );
      if( !m.diskInfo().empty() && !(wantedMediumState() & m.diskInfo().diskState()) )
	s.append( "<p><i>" + i18n("Medium will be overwritten.") + "</i>" );
      return s;
    }
  }

private:
  K3bDevice::Device* m_overrideDevice;
  QString m_overrideString;
  QString m_overrideToolTip;
};


class K3bWriterSelectionWidget::Private
{
public:
  bool forceAutoSpeed;
  bool haveIgnoreSpeed;
  bool haveManualSpeed;

  int supportedWritingApps;

  int lastSetSpeed;

  QMap<int, int> indexSpeedMap;
  QMap<int, int> speedIndexMap;
};


K3bWriterSelectionWidget::K3bWriterSelectionWidget( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  d = new Private;
  d->forceAutoSpeed = false;
  d->supportedWritingApps = K3b::CDRECORD|K3b::CDRDAO|K3b::GROWISOFS;
  d->lastSetSpeed = -1;

  QGroupBox* groupWriter = new QGroupBox( this );
  groupWriter->setTitle( i18n( "Burn Medium" ) );
  groupWriter->setColumnLayout(0, Qt::Vertical );
  groupWriter->layout()->setSpacing( 0 );
  groupWriter->layout()->setMargin( 0 );

  QGridLayout* groupWriterLayout = new QGridLayout( groupWriter->layout() );
  groupWriterLayout->setAlignment( Qt::AlignTop );
  groupWriterLayout->setSpacing( KDialog::spacingHint() );
  groupWriterLayout->setMargin( KDialog::marginHint() );

  QLabel* labelSpeed = new QLabel( groupWriter, "TextLabel1" );
  labelSpeed->setText( i18n( "Speed:" ) );

  m_comboSpeed = new KComboBox( false, groupWriter, "m_comboSpeed" );
  m_comboSpeed->setAutoMask( false );
  m_comboSpeed->setDuplicatesEnabled( false );

  m_comboMedium = new MediaSelectionComboBox( groupWriter );

  m_writingAppLabel = new QLabel( i18n("Writing app:"), groupWriter );
  m_comboWritingApp = new KComboBox( groupWriter );

  groupWriterLayout->addWidget( m_comboMedium, 0, 0 );
  groupWriterLayout->addWidget( labelSpeed, 0, 1 );
  groupWriterLayout->addWidget( m_comboSpeed, 0, 2 );
  groupWriterLayout->addWidget( m_writingAppLabel, 0, 3 );
  groupWriterLayout->addWidget( m_comboWritingApp, 0, 4 );
  groupWriterLayout->setColStretch( 0, 1 );


  QGridLayout* mainLayout = new QGridLayout( this );
  mainLayout->setAlignment( Qt::AlignTop );
  mainLayout->setSpacing( KDialog::spacingHint() );
  mainLayout->setMargin( 0 );

  mainLayout->addWidget( groupWriter, 0, 0 );

  // tab order
  setTabOrder( m_comboMedium, m_comboSpeed );
  setTabOrder( m_comboSpeed, m_comboWritingApp );

  connect( m_comboMedium, SIGNAL(selectionChanged(K3bDevice::Device*)), this, SIGNAL(writerChanged()) );
  connect( m_comboMedium, SIGNAL(selectionChanged(K3bDevice::Device*)),
	   this, SIGNAL(writerChanged(K3bDevice::Device*)) );
  connect( m_comboMedium, SIGNAL(newMedia()), this, SIGNAL(newMedia()) );
  connect( m_comboMedium, SIGNAL(newMedium(K3bDevice::Device*)), this, SIGNAL(newMedium(K3bDevice::Device*)) );
  connect( m_comboMedium, SIGNAL(newMedium(K3bDevice::Device*)), this, SLOT(slotNewBurnMedium(K3bDevice::Device*)) );
  connect( m_comboWritingApp, SIGNAL(activated(int)), this, SLOT(slotWritingAppSelected(int)) );
  connect( this, SIGNAL(writerChanged()), SLOT(slotWriterChanged()) );
  connect( m_comboSpeed, SIGNAL(activated(int)), this, SLOT(slotSpeedChanged(int)) );


  QToolTip::add( m_comboMedium, i18n("The medium that will be used for burning") );
  QToolTip::add( m_comboSpeed, i18n("The speed at which to burn the medium") );
  QToolTip::add( m_comboWritingApp, i18n("The external application to actually burn the medium") );

  QWhatsThis::add( m_comboMedium, i18n("<p>Select the medium that you want to use for burning."
				       "<p>In most cases there will only be one medium available which "
				       "does not leave much choice.") );
  QWhatsThis::add( m_comboSpeed, i18n("<p>Select the speed with which you want to burn."
				      "<p><b>Auto</b><br>"
				      "This will choose the maximum writing speed possible with the used "
				      "medium. "
				      "This is the recommended selection for most media.</p>"
				      "<p><b>Ignore</b> (DVD only)<br>"
				      "This will leave the speed selection to the writer device. "
				      "Use this if K3b is unable to set the writing speed."
				      "<p>1x refers to 1385 KB/s for DVD and 175 KB/s for CD.</p>"
				      "<p><b>Caution:</b> Make sure your system is able to send the data "
				      "fast enough to prevent buffer underruns.") );
  QWhatsThis::add( m_comboWritingApp, i18n("<p>K3b uses the command line tools cdrecord, growisofs, and cdrdao "
					   "to actually write a CD or DVD."
					   "<p>Normally K3b chooses the best "
					   "suited application for every task automatically but in some cases it "
					   "may be possible that one of the applications does not work as intended "
					   "with a certain writer. In this case one may select the "
					   "application manually.") );

  clearSpeedCombo();

  slotConfigChanged(k3bcore->config());
  slotWriterChanged();
}


K3bWriterSelectionWidget::~K3bWriterSelectionWidget()
{
  delete d;
}


void K3bWriterSelectionWidget::setWantedMediumType( int type )
{
  m_comboMedium->setWantedMediumType( type );
}


void K3bWriterSelectionWidget::setWantedMediumState( int state )
{
  m_comboMedium->setWantedMediumState( state );
}


int K3bWriterSelectionWidget::wantedMediumType() const
{
  return m_comboMedium->wantedMediumType();
}


int K3bWriterSelectionWidget::wantedMediumState() const
{
  return m_comboMedium->wantedMediumState();
}


void K3bWriterSelectionWidget::slotConfigChanged( KConfigBase* c )
{
  KConfigGroup g( c, "General Options" );
  if( g.readBoolEntry( "Manual writing app selection", false ) ) {
    m_comboWritingApp->show();
    m_writingAppLabel->show();
  }
  else {
    m_comboWritingApp->hide();
    m_writingAppLabel->hide();
  }
}


void K3bWriterSelectionWidget::slotRefreshWriterSpeeds()
{
  if( writerDevice() ) {
    QValueList<int> speeds = k3bappcore->mediaCache()->writingSpeeds( writerDevice() );

    int lastSpeed = writerSpeed();

    clearSpeedCombo();

    m_comboSpeed->insertItem( i18n("Auto") );
    if( k3bappcore->mediaCache()->diskInfo( writerDevice() ).isDvdMedia() ) {
      m_comboSpeed->insertItem( i18n("Ignore") );
      d->haveIgnoreSpeed = true;
    }
    else
      d->haveIgnoreSpeed = false;

    if( !d->forceAutoSpeed ) {
      if( speeds.isEmpty() || writerDevice() == m_comboMedium->overrideDevice() ) {
	//
	// In case of the override device we do not know which medium will actually be used
	// So this is the only case in which we need to use the device's max writing speed
	//
	// But we need to know if it will be a CD or DVD medium. Since the override device
	// is only used for CD/DVD copy anyway we simply reply on the inserted medium's type.
	//
	int i = 1;
	int speed = ( k3bappcore->mediaCache()->diskInfo( writerDevice() ).isDvdMedia() ? 1385 : 175 );
	int max = writerDevice()->maxWriteSpeed();
	while( i*speed <= max ) {
	  insertSpeedItem( i*speed );
	  // a little hack to handle the stupid 2.4x DVD speed
	  if( i == 2 && speed == 1385 )
	    insertSpeedItem( (int)(2.4*1385.0) );
	  i = ( i == 1 ? 2 : i+2 );
	}

	//
	// Since we do not know the exact max writing speed if an override device is set (we can't becasue
	// the writer always returns the speed relative to the inserted medium) we allow the user to specify
	// the speed manually
	//
	m_comboSpeed->insertItem( i18n("More...") );
	d->haveManualSpeed = true;
      }
      else {
	for( QValueList<int>::iterator it = speeds.begin(); it != speeds.end(); ++it )
	  insertSpeedItem( *it );
      }
    }

    // try to reload last set speed
    if( d->lastSetSpeed == -1 )
      setSpeed( lastSpeed );
    else
      setSpeed( d->lastSetSpeed );
  }

  m_comboSpeed->setEnabled( writerDevice() != 0 );
}


void K3bWriterSelectionWidget::clearSpeedCombo()
{
  m_comboSpeed->clear();
  d->indexSpeedMap.clear();
  d->speedIndexMap.clear();
  d->haveManualSpeed = false;
  d->haveIgnoreSpeed = false;
}


void K3bWriterSelectionWidget::insertSpeedItem( int speed )
{
  if( !d->speedIndexMap.contains( speed ) ) {
    d->indexSpeedMap[m_comboSpeed->count()] = speed;
    d->speedIndexMap[speed] = m_comboSpeed->count();

    if( k3bappcore->mediaCache()->diskInfo( writerDevice() ).isDvdMedia() )
      m_comboSpeed->insertItem( ( speed%1385 > 0
				? QString::number( (float)speed/1385.0, 'f', 1 )  // example: DVD+R(W): 2.4x
				: QString::number( speed/1385 ) )
			      + "x" );
    else
      m_comboSpeed->insertItem( QString("%1x").arg(speed/175) );
  }
}


void K3bWriterSelectionWidget::slotWritingAppSelected( int )
{
  emit writingAppChanged( selectedWritingApp() );
}


K3bDevice::Device* K3bWriterSelectionWidget::writerDevice() const
{
  return m_comboMedium->selectedDevice();
}


QValueList<K3bDevice::Device*> K3bWriterSelectionWidget::allDevices() const
{
  return m_comboMedium->allDevices();
}


void K3bWriterSelectionWidget::setWriterDevice( K3bDevice::Device* dev )
{
  m_comboMedium->setSelectedDevice( dev );
}


void K3bWriterSelectionWidget::setSpeed( int s )
{
  d->lastSetSpeed = -1;

  if( d->haveIgnoreSpeed && s < 0 )
    m_comboSpeed->setCurrentItem( 1 ); // Ignore
  else if( d->speedIndexMap.contains( s ) )
    m_comboSpeed->setCurrentItem( d->speedIndexMap[s] );
  else {
    m_comboSpeed->setCurrentItem( 0 ); // Auto
    d->lastSetSpeed = s; // remember last set speed
  }
}


void K3bWriterSelectionWidget::setWritingApp( int app )
{
  switch( app ) {
  case K3b::CDRECORD:
    m_comboWritingApp->setCurrentItem( "cdrecord" );
    break;
  case K3b::CDRDAO:
    m_comboWritingApp->setCurrentItem( "cdrdao" );
    break;
  case K3b::DVDRECORD:
    m_comboWritingApp->setCurrentItem( "dvdrecord" );
    break;
  case K3b::GROWISOFS:
    m_comboWritingApp->setCurrentItem( "growisofs" );
    break;
  case K3b::DVD_RW_FORMAT:
    m_comboWritingApp->setCurrentItem( "dvd+rw-format" );
    break;
  default:
    m_comboWritingApp->setCurrentItem( 0 );  // Auto
    break;
  }
}


int K3bWriterSelectionWidget::writerSpeed() const
{
  if( m_comboSpeed->currentItem() == 0 )
    return 0; // Auto
  else if( d->haveIgnoreSpeed && m_comboSpeed->currentItem() == 1 )
    return -1; // Ignore
  else
    return d->indexSpeedMap[m_comboSpeed->currentItem()];
}


int K3bWriterSelectionWidget::writingApp() const
{
  KConfigGroup g( k3bcore->config(), "General Options" );
  if( g.readBoolEntry( "Manual writing app selection", false ) ) {
    return selectedWritingApp();
  }
  else
    return K3b::DEFAULT;
}


int K3bWriterSelectionWidget::selectedWritingApp() const
{
  return K3b::writingAppFromString( m_comboWritingApp->currentText() );
}


void K3bWriterSelectionWidget::slotSpeedChanged( int s )
{
  // the last item is the manual speed selection item
  if( d->haveManualSpeed && s == m_comboSpeed->count() - 1 ) {
    slotManualSpeed();
  }
  else {
    d->lastSetSpeed = d->indexSpeedMap[s];

    if( K3bDevice::Device* dev = writerDevice() )
      dev->setCurrentWriteSpeed( writerSpeed() );
  }
}


void K3bWriterSelectionWidget::slotWriterChanged()
{
  slotRefreshWriterSpeeds();
  slotRefreshWritingApps();

  // save last selected writer
  if( K3bDevice::Device* dev = writerDevice() ) {
    KConfigGroup g( k3bcore->config(), "General Options" );
    g.writeEntry( "current_writer", dev->devicename() );
  }
}


void K3bWriterSelectionWidget::setSupportedWritingApps( int i )
{
  int oldApp = writingApp();

  d->supportedWritingApps = i;

  slotRefreshWritingApps();

  setWritingApp( oldApp );
}


void K3bWriterSelectionWidget::slotRefreshWritingApps()
{
  int i = 0;

  // select the ones that make sense
  if( k3bappcore->mediaCache()->diskInfo( writerDevice() ).isDvdMedia() )
    i = K3b::GROWISOFS|K3b::DVD_RW_FORMAT|K3b::DVDRECORD;
  else
    i = K3b::CDRDAO|K3b::CDRECORD;

  // now strip it down to the ones we support
  i &= d->supportedWritingApps;

  m_comboWritingApp->clear();
  m_comboWritingApp->insertItem( i18n("Auto") );

  if( i & K3b::CDRDAO )
    m_comboWritingApp->insertItem( "cdrdao" );
  if( i & K3b::CDRECORD )
    m_comboWritingApp->insertItem( "cdrecord" );
  if( i & K3b::DVDRECORD )
    m_comboWritingApp->insertItem( "dvdrecord" );
  if( i & K3b::GROWISOFS )
    m_comboWritingApp->insertItem( "growisofs" );
  if( i & K3b::DVD_RW_FORMAT )
    m_comboWritingApp->insertItem( "dvd+rw-format" );

  m_comboWritingApp->setEnabled( writerDevice() != 0 );
}


void K3bWriterSelectionWidget::loadConfig( KConfigBase* c )
{
  setWriterDevice( k3bcore->deviceManager()->findDevice( c->readEntry( "writer_device" ) ) );
  setSpeed( c->readNumEntry( "writing_speed",  0 ) );
  setWritingApp( K3b::writingAppFromString( c->readEntry( "writing_app" ) ) );
}


void K3bWriterSelectionWidget::saveConfig( KConfigBase* c )
{
  c->writeEntry( "writing_speed", writerSpeed() );
  c->writeEntry( "writer_device", writerDevice() ? writerDevice()->devicename() : QString::null );
  c->writeEntry( "writing_app", m_comboWritingApp->currentText() );
}

void K3bWriterSelectionWidget::loadDefaults()
{
  // ignore the writer
  m_comboSpeed->setCurrentItem( 0 ); // Auto
  setWritingApp( K3b::DEFAULT );
}


void K3bWriterSelectionWidget::setForceAutoSpeed( bool b )
{
  d->forceAutoSpeed = b;
  slotRefreshWriterSpeeds();
}


void K3bWriterSelectionWidget::setOverrideDevice( K3bDevice::Device* dev, const QString& overrideString, const QString& tooltip )
{
  m_comboMedium->setOverrideDevice( dev, overrideString, tooltip );
}


void K3bWriterSelectionWidget::slotNewBurnMedium( K3bDevice::Device* dev )
{
  //
  // Try to select a medium that is better suited than the current one
  //
  if( dev && dev != writerDevice() ) {
    K3bMedium medium = k3bappcore->mediaCache()->medium( dev );

    //
    // Always prefer newly inserted media over the override device
    //
    if( writerDevice() == m_comboMedium->overrideDevice() ) {
      setWriterDevice( dev );
    }

    //
    // Prefer an empty medium over one that has to be erased
    //
    else if( wantedMediumState() & K3bDevice::STATE_EMPTY &&
	     !k3bappcore->mediaCache()->diskInfo( writerDevice() ).empty() &&
	     medium.diskInfo().empty() ) {
      setWriterDevice( dev );
    }
  }
}


void K3bWriterSelectionWidget::slotManualSpeed()
{
  //
  // We need to know if it will be a CD or DVD medium. Since the override device
  // is only used for CD/DVD copy anyway we simply reply on the inserted medium's type.
  //
  int speedFactor = ( k3bappcore->mediaCache()->diskInfo( writerDevice() ).isDvdMedia() ? 1385 : 175 );

  bool ok = true;
  int newSpeed = KInputDialog::getInteger( i18n("Set writing speed manually"),
					   i18n("<p>K3b is not able to perfectly determine the maximum "
						"writing speed of an optical writer. Writing speed is always "
						"reported subject to the inserted medium."
						"<p>Please enter the writing speed here and K3b will remember it "
						"for future sessions (Example: 16x)."),
					   writerDevice()->maxWriteSpeed()/speedFactor,
					   1,
					   10000,
					   1,
					   10,
					   &ok,
					   this ) * speedFactor;
  if( ok ) {
    writerDevice()->setMaxWriteSpeed( QMAX( newSpeed, writerDevice()->maxWriteSpeed() ) );
    slotRefreshWriterSpeeds();
    setSpeed( newSpeed );
  }
  else {
    if( d->lastSetSpeed == -1 )
      m_comboSpeed->setCurrentItem( 0 ); // Auto
    else
      setSpeed( d->lastSetSpeed );
  }
}


void K3bWriterSelectionWidget::setIgnoreDevice( K3bDevice::Device* dev )
{
    m_comboMedium->setIgnoreDevice( dev );
}

#include "k3bwriterselectionwidget.moc"
