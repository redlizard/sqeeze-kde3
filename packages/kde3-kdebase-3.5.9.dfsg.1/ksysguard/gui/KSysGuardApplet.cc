/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

    KSysGuard is currently maintained by Chris Schlaeger
    <cs@kde.org>. Please do not commit any changes without consulting
    me first. Thanks!

*/

#include <qcursor.h>
#include <qdom.h>
#include <qdragobject.h>
#include <qfile.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qtooltip.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksavefile.h>
#include <kstandarddirs.h>
#include <kpopupmenu.h>

#include <ksgrd/SensorClient.h>
#include <ksgrd/SensorManager.h>
#include <ksgrd/StyleEngine.h>

#include "DancingBars.h"
#include "FancyPlotter.h"
#include "KSGAppletSettings.h"
#include "MultiMeter.h"

#include "KSysGuardApplet.h"

extern "C"
{
  KDE_EXPORT KPanelApplet* init( QWidget *parent, const QString& configFile )
  {
    KGlobal::locale()->insertCatalogue( "ksysguard" );
    return new KSysGuardApplet( configFile, KPanelApplet::Normal,
                                KPanelApplet::Preferences, parent,
                                "ksysguardapplet" );
  }
}

KSysGuardApplet::KSysGuardApplet( const QString& configFile, Type type,
                                  int actions, QWidget *parent,
                                  const char *name )
  : KPanelApplet( configFile, type, actions, parent, name)
{
  mSettingsDlg = 0;

  KSGRD::SensorMgr = new KSGRD::SensorManager();

  KSGRD::Style = new KSGRD::StyleEngine();

  mDockCount = 1;
  mDockList = new QWidget*[ mDockCount ];

  mSizeRatio = 1.0;
  addEmptyDisplay( mDockList, 0 );

  updateInterval( 2 );

  load();

  setAcceptDrops( true );
}

KSysGuardApplet::~KSysGuardApplet()
{
  save();

  delete [] mDockList;
  mDockList = 0;

  delete mSettingsDlg;
  mSettingsDlg = 0;

	delete KSGRD::Style;
	delete KSGRD::SensorMgr;
	KSGRD::SensorMgr = 0;
}

int KSysGuardApplet::widthForHeight( int height ) const
{
  return ( (int) ( height * mSizeRatio + 0.5 ) * mDockCount );
}

int KSysGuardApplet::heightForWidth( int width ) const
{
  return ( (int) ( width * mSizeRatio + 0.5 ) * mDockCount );
}

void KSysGuardApplet::resizeEvent( QResizeEvent* )
{
  layout();
}

void KSysGuardApplet::preferences()
{
  if(mSettingsDlg) {
    return;
  }
  mSettingsDlg = new KSGAppletSettings( this );

  connect( mSettingsDlg, SIGNAL( applyClicked() ), SLOT( applySettings() ) );
  connect( mSettingsDlg, SIGNAL( okClicked() ), SLOT( applySettings() ) );
  connect( mSettingsDlg, SIGNAL( finished() ), SLOT( preferencesFinished() ) );

  mSettingsDlg->setNumDisplay( mDockCount );
  mSettingsDlg->setSizeRatio( (int) ( mSizeRatio * 100.0 + 0.5 ) );
  mSettingsDlg->setUpdateInterval( updateInterval() );

  mSettingsDlg->show();
}
void KSysGuardApplet::preferencesFinished()
{
  mSettingsDlg->delayedDestruct();
  mSettingsDlg = 0;
}
void KSysGuardApplet::applySettings()
{
  updateInterval( mSettingsDlg->updateInterval() );
  mSizeRatio = mSettingsDlg->sizeRatio() / 100.0;
  resizeDocks( mSettingsDlg->numDisplay() );

  for ( uint i = 0; i < mDockCount; ++i )
    if ( !mDockList[ i ]->isA( "QFrame" ) )
      ((KSGRD::SensorDisplay*)mDockList[ i ])->setUpdateInterval( updateInterval() );

  save();
}

void KSysGuardApplet::sensorDisplayModified( bool modified )
{
  if ( modified )
    save();
}

void KSysGuardApplet::layout()
{
  if ( orientation() == Horizontal ) {
    int h = height();
    int w = (int) ( h * mSizeRatio + 0.5 );
    for ( uint i = 0; i < mDockCount; ++i )
      if ( mDockList[ i ] )
        mDockList[ i ]->setGeometry( i * w, 0, w, h );
  } else {
    int w = width();
    int h = (int) ( w * mSizeRatio + 0.5 );
    for ( uint i = 0; i < mDockCount; ++i )
      if ( mDockList[ i ] )
        mDockList[ i ]->setGeometry( 0, i * h, w, h );
  }
}

int KSysGuardApplet::findDock( const QPoint& point )
{
  if ( orientation() == Horizontal )
    return ( point.x() / (int) ( height() * mSizeRatio + 0.5 ) );
  else
    return ( point.y() / (int) ( width() * mSizeRatio + 0.5 ) );
}

void KSysGuardApplet::dragEnterEvent( QDragEnterEvent *e )
{
  e->accept( QTextDrag::canDecode( e ) );
}

void KSysGuardApplet::dropEvent( QDropEvent *e )
{
  QString dragObject;

  if ( QTextDrag::decode( e, dragObject ) ) {
    // The host name, sensor name and type are seperated by a ' '.
    QStringList parts = QStringList::split( ' ', dragObject );

    QString hostName = parts[ 0 ];
    QString sensorName = parts[ 1 ];
    QString sensorType = parts[ 2 ];
    QString sensorDescr = parts[ 3 ];

    if ( hostName.isEmpty() || sensorName.isEmpty() || sensorType.isEmpty() )
      return;

    int dock = findDock( e->pos() );
    if ( mDockList[ dock ]->isA( "QFrame" ) ) {
      if ( sensorType == "integer" || sensorType == "float" ) {
        KPopupMenu popup;
        QWidget *wdg = 0;

        popup.insertTitle( i18n( "Select Display Type" ) );
        popup.insertItem( i18n( "&Signal Plotter" ), 1 );
        popup.insertItem( i18n( "&Multimeter" ), 2 );
        popup.insertItem( i18n( "&Dancing Bars" ), 3 );
        switch ( popup.exec( QCursor::pos() ) ) {
          case 1:
            wdg = new FancyPlotter( this, "FancyPlotter", sensorDescr,
                                    100.0, 100.0, true );
						break;

          case 2:
            wdg = new MultiMeter( this, "MultiMeter", sensorDescr,
                                  100.0, 100.0, true );
            break;

          case 3:
            wdg = new DancingBars( this, "DancingBars", sensorDescr,
                                   100, 100, true );
            break;
				}

        if ( wdg ) {
          delete mDockList[ dock ];
          mDockList[ dock ] = wdg;
          layout();

          connect( wdg, SIGNAL( modified( bool ) ),
                   SLOT( sensorDisplayModified( bool ) ) );

          mDockList[ dock ]->show();
        }
      } else {
        KMessageBox::sorry( this,
          i18n( "The KSysGuard applet does not support displaying of "
                "this type of sensor. Please choose another sensor." ) );

        layout();
      }
    }

    if ( !mDockList[ dock ]->isA( "QFrame" ) )
      ((KSGRD::SensorDisplay*)mDockList[ dock ])->
                  addSensor( hostName, sensorName, sensorType, sensorDescr );
  }

  save();
}

void KSysGuardApplet::customEvent( QCustomEvent *e )
{
  if ( e->type() == QEvent::User ) {
    // SensorDisplays send out this event if they want to be removed.
    removeDisplay( (KSGRD::SensorDisplay*)e->data() );
    save();
  }
}

void KSysGuardApplet::removeDisplay( KSGRD::SensorDisplay *display )
{
  for ( uint i = 0; i < mDockCount; ++i )
    if ( display == mDockList[i] ) {
      delete mDockList[ i ];

      addEmptyDisplay( mDockList, i );
      return;
    }
}

void KSysGuardApplet::resizeDocks( uint newDockCount )
{
  /* This function alters the number of available docks. The number of
   * docks can be increased or decreased. We try to preserve existing
   * docks if possible. */

  if ( newDockCount == mDockCount ) {
    emit updateLayout();
    return;
  }

  // Create and initialize new dock array.
  QWidget** tmp = new QWidget*[ newDockCount ];

  uint i;
  for ( i = 0; ( i < newDockCount ) && ( i < mDockCount ); ++i )
    tmp[ i ] = mDockList[ i ];

  for ( i = newDockCount; i < mDockCount; ++i )
    if ( mDockList[ i ] )
      delete mDockList[ i ];

  for ( i = mDockCount; i < newDockCount; ++i )
    addEmptyDisplay( tmp, i );

  delete [] mDockList;

  mDockList = tmp;
  mDockCount = newDockCount;

  emit updateLayout();
}

bool KSysGuardApplet::load()
{
  KStandardDirs* kstd = KGlobal::dirs();
  kstd->addResourceType( "data", "share/apps/ksysguard" );
  QString fileName = kstd->findResource( "data", "KSysGuardApplet.xml" );

  QFile file( fileName );
  if ( !file.open( IO_ReadOnly ) ) {
    KMessageBox::sorry( this, i18n( "Cannot open the file %1." ).arg( fileName ) );
    return false;
  }

  // Parse the XML file.
  QDomDocument doc;

  // Read in file and check for a valid XML header.
  if ( !doc.setContent( &file ) ) {
    KMessageBox::sorry( this, i18n( "The file %1 does not contain valid XML." )
                        .arg( fileName ) );
    return false;
  }

	// Check for proper document type.
  if ( doc.doctype().name() != "KSysGuardApplet" ) {
    KMessageBox::sorry( this, i18n( "The file %1 does not contain a valid applet "
                        "definition, which must have a document type 'KSysGuardApplet'." )
                        .arg( fileName ) );
    return false;
  }

  QDomElement element = doc.documentElement();
  bool ok;
  uint count = element.attribute( "dockCnt" ).toUInt( &ok );
  if ( !ok )
		count = 1;

  mSizeRatio = element.attribute( "sizeRatio" ).toDouble( &ok );
  if ( !ok )
    mSizeRatio = 1.0;

  updateInterval( element.attribute( "interval" ).toUInt( &ok ) );
  if ( !ok )
    updateInterval( 2 );

  resizeDocks( count );

  /* Load lists of hosts that are needed for the work sheet and try
   * to establish a connection. */
  QDomNodeList dnList = element.elementsByTagName( "host" );
  uint i;
  for ( i = 0; i < dnList.count(); ++i ) {
    QDomElement element = dnList.item( i ).toElement();
    int port = element.attribute( "port" ).toInt( &ok );
    if ( !ok )
      port = -1;

    KSGRD::SensorMgr->engage( element.attribute( "name" ),
                              element.attribute( "shell" ),
                              element.attribute( "command" ), port );
  }
  //if no hosts are specified, at least connect to localhost
  if(dnList.count() == 0)
    KSGRD::SensorMgr->engage( "localhost", "", "ksysguardd", -1);

  // Load the displays and place them into the work sheet.
  dnList = element.elementsByTagName( "display" );
  for ( i = 0; i < dnList.count(); ++i ) {
    QDomElement element = dnList.item( i ).toElement();
    uint dock = element.attribute( "dock" ).toUInt();
    if ( i >= mDockCount ) {
      kdDebug (1215) << "Dock number " << i << " out of range "
                     << mDockCount << endl;
      return false;
    }

    QString classType = element.attribute( "class" );
    KSGRD::SensorDisplay* newDisplay;
    if ( classType == "FancyPlotter" )
      newDisplay = new FancyPlotter( this, "FancyPlotter", "Dummy", 100.0, 100.0, true /*no frame*/, true /*run ksysguard menu*/);
    else if ( classType == "MultiMeter" )
      newDisplay = new MultiMeter( this, "MultiMeter", "Dummy", 100.0, 100.0, true /*no frame*/, true /*run ksysguard menu*/ );
    else if ( classType == "DancingBars" )
      newDisplay = new DancingBars( this, "DancingBars", "Dummy", 100, 100, true /*no frame*/, true /*run ksysguard menu*/);
    else {
      KMessageBox::sorry( this, i18n( "The KSysGuard applet does not support displaying of "
                          "this type of sensor. Please choose another sensor." ) );
      return false;
    }

    newDisplay->setUpdateInterval( updateInterval() );

    // load display specific settings
    if ( !newDisplay->restoreSettings( element ) )
      return false;

    delete mDockList[ dock ];
    mDockList[ dock ] = newDisplay;

    connect( newDisplay, SIGNAL( modified( bool ) ),
             SLOT( sensorDisplayModified( bool ) ) );
  }

  return true;
}

bool KSysGuardApplet::save()
{
  // Parse the XML file.
  QDomDocument doc( "KSysGuardApplet" );
  doc.appendChild( doc.createProcessingInstruction(
                   "xml", "version=\"1.0\" encoding=\"UTF-8\"" ) );

  // save work sheet information
  QDomElement ws = doc.createElement( "WorkSheet" );
  doc.appendChild( ws );
  ws.setAttribute( "dockCnt", mDockCount );
  ws.setAttribute( "sizeRatio", mSizeRatio );
  ws.setAttribute( "interval", updateInterval() );

  QStringList hosts;
  uint i;
  for ( i = 0; i < mDockCount; ++i )
    if ( !mDockList[ i ]->isA( "QFrame" ) )
      ((KSGRD::SensorDisplay*)mDockList[ i ])->hosts( hosts );

  // save host information (name, shell, etc.)
  QStringList::Iterator it;
  for ( it = hosts.begin(); it != hosts.end(); ++it ) {
    QString shell, command;
    int port;

    if ( KSGRD::SensorMgr->hostInfo( *it, shell, command, port ) ) {
      QDomElement host = doc.createElement( "host" );
      ws.appendChild( host );
      host.setAttribute( "name", *it );
      host.setAttribute( "shell", shell );
      host.setAttribute( "command", command );
      host.setAttribute( "port", port );
    }
  }

  for ( i = 0; i < mDockCount; ++i )
    if ( !mDockList[ i ]->isA( "QFrame" ) ) {
      QDomElement element = doc.createElement( "display" );
      ws.appendChild( element );
      element.setAttribute( "dock", i );
      element.setAttribute( "class", mDockList[ i ]->className() );

      ((KSGRD::SensorDisplay*)mDockList[ i ])->saveSettings( doc, element );
    }

  KStandardDirs* kstd = KGlobal::dirs();
  kstd->addResourceType( "data", "share/apps/ksysguard" );
  QString fileName = kstd->saveLocation( "data", "ksysguard" );
  fileName += "/KSysGuardApplet.xml";

  KSaveFile file( fileName, 0644 );

  if ( file.status() == 0 )
  {
    file.textStream()->setEncoding( QTextStream::UnicodeUTF8 );
    *(file.textStream()) << doc;
    file.close();
  }
  else
  {
    KMessageBox::sorry( this, i18n( "Cannot save file %1" ).arg( fileName ) );
    return false;
  }

  return true;
}

void KSysGuardApplet::addEmptyDisplay( QWidget **dock, uint pos )
{
  dock[ pos ] = new QFrame( this );
  ((QFrame*)dock[ pos ])->setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
  QToolTip::add( dock[ pos ],
                 i18n( "Drag sensors from the KDE System Guard into this cell." ) );

  layout();
  if ( isVisible() )
    dock[ pos ]->show();
}

#include "KSysGuardApplet.moc"
