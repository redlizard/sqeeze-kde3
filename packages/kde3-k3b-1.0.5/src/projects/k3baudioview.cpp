/* 
 *
 * $Id: k3baudioview.cpp 619556 2007-01-03 17:38:12Z trueg $
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

#include <config.h>

#include "k3baudioview.h"
#include "k3baudiotrackview.h"
#include "k3baudioburndialog.h"
#include "k3baudiotrackplayer.h"
#include "k3baudioburndialog.h"
#include "k3baudiotrackaddingdialog.h"
#include <k3bapplication.h>

#include <k3baudiodoc.h>
#include <k3baudiotrack.h>
#include <k3baudiofile.h>
#include <k3bpluginmanager.h>

// this is not here becasue of base_*.ui troubles
#include "../rip/k3baudioprojectconvertingdialog.h"

#include <k3bfillstatusdisplay.h>
#include <k3bmsf.h>
#include <k3btoolbox.h>
#include <kactionclasses.h>
#include <k3bprojectplugin.h>

// QT-includes
#include <qlayout.h>
#include <qstring.h>

// KDE-includes
#include <klocale.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kmessagebox.h>



K3bAudioView::K3bAudioView( K3bAudioDoc* pDoc, QWidget* parent, const char *name )
  : K3bView( pDoc, parent, name )
{
  m_doc = pDoc;

  m_songlist = new K3bAudioTrackView( m_doc, this );
  setMainWidget( m_songlist );
  fillStatusDisplay()->showTime();

  // add button for the audio conversion
  KAction* conversionAction = new KAction( i18n("Convert Tracks"), "redo", 0, this, SLOT(slotAudioConversion()), 
					   actionCollection(), "project_audio_convert" );
  conversionAction->setToolTip( i18n("Convert audio tracks to other audio formats." ) );

  toolBox()->addButton( conversionAction );
  toolBox()->addSeparator();

  toolBox()->addButton( m_songlist->player()->action( K3bAudioTrackPlayer::ACTION_PLAY ) );
  toolBox()->addButton( m_songlist->player()->action( K3bAudioTrackPlayer::ACTION_PAUSE ) );
  toolBox()->addButton( m_songlist->player()->action( K3bAudioTrackPlayer::ACTION_STOP ) );
  toolBox()->addSpacing();
  toolBox()->addButton( m_songlist->player()->action( K3bAudioTrackPlayer::ACTION_PREV ) );
  toolBox()->addButton( m_songlist->player()->action( K3bAudioTrackPlayer::ACTION_NEXT ) );
  toolBox()->addSpacing();
  toolBox()->addWidgetAction( static_cast<KWidgetAction*>(m_songlist->player()->action( K3bAudioTrackPlayer::ACTION_SEEK )) );
  toolBox()->addSeparator();

#ifdef HAVE_MUSICBRAINZ
  kdDebug() << "(K3bAudioView) m_songlist->actionCollection()->actions().count() " << m_songlist->actionCollection()->actions().count() << endl;
  toolBox()->addButton( m_songlist->actionCollection()->action( "project_audio_musicbrainz" ) );
  toolBox()->addSeparator();
#endif

  addPluginButtons( K3bProjectPlugin::AUDIO_CD );

  toolBox()->addStretch();

  // this is just for testing (or not?)
  // most likely every project type will have it's rc file in the future
  // we only add the additional actions since K3bView already added the default actions
  setXML( "<!DOCTYPE kpartgui SYSTEM \"kpartgui.dtd\">"
	  "<kpartgui name=\"k3bproject\" version=\"1\">"
	  "<MenuBar>"
	  " <Menu name=\"project\"><text>&amp;Project</text>"
	  "  <Action name=\"project_audio_convert\"/>"
#ifdef HAVE_MUSICBRAINZ
	  "  <Action name=\"project_audio_musicbrainz\"/>"
#endif
	  " </Menu>"
	  "</MenuBar>"
	  "</kpartgui>", true );
}

K3bAudioView::~K3bAudioView()
{
}


void K3bAudioView::init()
{
  if( k3bcore->pluginManager()->plugins( "AudioDecoder" ).isEmpty() )
    KMessageBox::error( this, i18n("No audio decoder plugins found. You will not be able to add any files "
				   "to the audio project!") );
}


K3bProjectBurnDialog* K3bAudioView::newBurnDialog( QWidget* parent, const char* name )
{
  return new K3bAudioBurnDialog( m_doc, parent, name, true );
}


void K3bAudioView::slotAudioConversion()
{
  K3bAudioProjectConvertingDialog dlg( m_doc, this );
  dlg.exec();
}


void K3bAudioView::addUrls( const KURL::List& urls )
{
  K3bAudioTrackAddingDialog::addUrls( urls, m_doc, 0, 0, 0, this );
}

#include "k3baudioview.moc"
