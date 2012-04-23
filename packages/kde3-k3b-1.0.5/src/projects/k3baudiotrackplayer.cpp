/* 
 *
 * $Id: k3baudiotrackplayer.cpp 619556 2007-01-03 17:38:12Z trueg $
 * Copyright (C) 2004 Sebastian Trueg <trueg@k3b.org>
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

#include "k3baudiotrackplayer.h"
#include <k3baudiodoc.h>
#include <k3baudiotrack.h>
#include <k3baudioserver.h>

#include <kactionclasses.h>
#include <klocale.h>

#include <qslider.h>
#include <qtimer.h>
#include <qmutex.h>
#include <qtooltip.h>


class K3bAudioTrackPlayer::Private
{
public:
  KAction* actionPlay;
  KAction* actionPause;
  KAction* actionPlayPause;
  KAction* actionStop;
  KAction* actionNext;
  KAction* actionPrev;
  KAction* actionSeek;

  // just to handle them easily;
  KActionCollection* actionCollection;

  QSlider* seekSlider;
  QTimer sliderTimer;

  // used to make sure that no seek and read operation occur in parallel
  QMutex mutex;

  bool playing;
  bool paused;
};


K3bAudioTrackPlayer::K3bAudioTrackPlayer( K3bAudioDoc* doc, QObject* parent, const char* name )
  : QObject( parent, name ),
    K3bAudioClient(),
    m_doc( doc ),
    m_currentTrack( 0 )
{
  d = new Private;
  d->paused = false;
  d->playing = false;

  // TODO: handle the shortcuts: pass a widget to the action collection (perhaps the trackview?)
  d->actionCollection = new KActionCollection( 0, this );

  // create the actions
  // TODO: create shortcuts (is there a way to let the user change them?)
  d->actionPlay = new KAction( i18n("Play"), 
			       "player_play", 
			       KShortcut(), 
			       this, SLOT(playPause()), 
			       d->actionCollection,
			       "play" );
  d->actionPause = new KAction( i18n("Pause"), 
				"player_pause", 
				KShortcut(), 
				this, SLOT(playPause()), 
				d->actionCollection,
				"pause" );
  d->actionPlayPause = new KAction( i18n("Play/Pause"), 
				    "player_play", 
				    KShortcut(), 
				    this, SLOT(playPause()), 
				    d->actionCollection,
				    "play_pause" );

  d->actionStop = new KAction( i18n("Stop"), 
			       "player_stop", 
			       KShortcut(), 
			       this, SLOT(stop()), 
			       d->actionCollection,
			       "stop" );
  d->actionNext = new KAction( i18n("Next"), 
			       "player_end", 
			       KShortcut(), 
			       this, SLOT(next()), 
			       d->actionCollection,
			       "next" );
  d->actionPrev = new KAction( i18n("Prev"), 
			       "player_start", 
			       KShortcut(), 
			       this, SLOT(prev()), 
			       d->actionCollection,
			       "prev" );

  d->seekSlider = new QSlider( 0, 100, 1, 0, Qt::Horizontal, 0, "audiotrackplayerslider" );
  connect( d->seekSlider, SIGNAL(sliderMoved(int)), this, SLOT(slotSeek(int)) );
  // FIXME: maybe it's not such a good idea to use a KWidgetAction here since this way the player
  // can only be used once in one widget. If the action would always create a new slider we could plug
  // the action into several toolboxes and also use it in some resizing or track splitting dialogs.
  d->actionSeek = new KWidgetAction( d->seekSlider,
				     i18n("Seek"),
				     KShortcut(),
				     0,
				     0,
				     d->actionCollection,
				     "seek" );
  // this should be done in KWidgetAction but is not yet
  connect( d->actionSeek, SIGNAL(enabled(bool)), d->seekSlider, SLOT(setEnabled(bool)) );

  d->actionStop->setEnabled(false);
  d->actionPause->setEnabled(false);
  d->actionNext->setEnabled(false);
  d->actionPrev->setEnabled(false);
  d->actionSeek->setEnabled(false);

  connect( m_doc, SIGNAL(changed()), 
	   this, SLOT(slotDocChanged()) );
  connect( m_doc, SIGNAL(trackChanged(K3bAudioTrack*)),
	   this, SLOT(slotTrackChanged(K3bAudioTrack*)) );
  connect( m_doc, SIGNAL(trackRemoved(K3bAudioTrack*)),
	   this, SLOT(slotTrackRemoved(K3bAudioTrack*)) );
  connect( &d->sliderTimer, SIGNAL(timeout()),
	   this, SLOT(slotUpdateSlider()) );

  // we just stop the player if the audio server has an error. K3bMainWindow will show the error message
  // This is all very hacky and has to be improved for K3b 2.0. But then we will probably use Phonon anyway...
  connect( K3bAudioServer::instance(), SIGNAL(error(const QString&)), this, SLOT(stop()) );

  // tooltips
  d->actionPlay->setToolTip( i18n("Play") );
  d->actionStop->setToolTip( i18n("Stop") );
  d->actionPause->setToolTip( i18n("Pause") );
  d->actionNext->setToolTip( i18n("Next") );
  d->actionPrev->setToolTip( i18n("Previous") );
}


K3bAudioTrackPlayer::~K3bAudioTrackPlayer()
{
  stop();
  delete d->seekSlider;
  delete d;
}


KAction* K3bAudioTrackPlayer::action( int action ) const
{
  switch( action ) {
  case ACTION_PLAY:
    return d->actionPlay;
  case ACTION_PAUSE:
    return d->actionPause;
  case ACTION_PLAY_PAUSE:
    return d->actionPlayPause;
  case ACTION_STOP:
    return d->actionStop;
  case ACTION_NEXT:
    return d->actionNext;
  case ACTION_PREV:
    return d->actionPrev;
  case ACTION_SEEK:
    return d->actionSeek;
  default:
    return 0;
  }
}

  
void K3bAudioTrackPlayer::playTrack( K3bAudioTrack* track )
{
  if( track ) {
    // we show the currently playing track as a tooltip on the slider
    QToolTip::remove( d->seekSlider );
    QToolTip::add( d->seekSlider, i18n("Playing track %1: %2 - %3")
		   .arg(track->trackNumber())
		   .arg(track->artist())
		   .arg(track->title()) );
    d->seekSlider->setMaxValue( track->length().totalFrames() );
    m_currentTrack = track;
    d->paused = true;

    d->actionNext->setEnabled( m_currentTrack->next() != 0 );
    d->actionPrev->setEnabled( m_currentTrack->prev() != 0 );

    seek(0);
    playPause();

    emit playingTrack( track );
  }
}


void K3bAudioTrackPlayer::playPause()
{
  if( !m_currentTrack ) {
    playTrack( m_doc->firstTrack() );
  }
  else {
    if( !d->playing ) {
      seek( m_currentPosition );
      d->playing = true;
      d->actionPlayPause->setIcon( "player_pause" );
      d->actionPause->setEnabled(true);
      d->actionPlay->setEnabled(false);
      d->actionSeek->setEnabled(true);
      startStreaming();
      d->sliderTimer.start(1000);
    }
    else if( d->paused ) {
      d->paused = false;
      d->actionPlayPause->setIcon( "player_pause" );
      d->actionPause->setEnabled(true);
      d->actionPlay->setEnabled(false);
      startStreaming();
      d->sliderTimer.start(1000);

      emit paused( false );
    }
    else {
      d->paused = true;
      d->actionPlayPause->setIcon( "player_play" );
      d->actionPause->setEnabled(false);
      d->actionPlay->setEnabled(true);
      stopStreaming();
      d->sliderTimer.stop();

      emit paused( true );
    }

    d->actionStop->setEnabled(true);
  }
}


void K3bAudioTrackPlayer::stop()
{
  m_currentTrack = 0;
  m_currentPosition = 0;
  stopStreaming();
  d->paused = false;
  d->playing = false;
  d->actionStop->setEnabled(false);
  d->actionPause->setEnabled(false);
  d->actionPlay->setEnabled(true);
  d->actionSeek->setEnabled(false);
  d->actionNext->setEnabled(false);
  d->actionPrev->setEnabled(false);

  d->actionPlayPause->setIcon( "player_play" );

  emit stopped();
}


void K3bAudioTrackPlayer::next()
{
  if( m_currentTrack && m_currentTrack->next() ) {
    playTrack( m_currentTrack->next() );
  }
}


void K3bAudioTrackPlayer::prev()
{
  if( m_currentTrack && m_currentTrack->prev() ) {
    playTrack( m_currentTrack->prev() );
  }
}


void K3bAudioTrackPlayer::seek( const K3b::Msf& msf )
{
  if( m_currentTrack ) {
    if( msf < m_currentTrack->length() ) {
      d->mutex.lock();
      m_currentTrack->seek( msf );
      m_currentPosition = msf;
      slotUpdateSlider();
      d->mutex.unlock();
    }
    else
      next();
  }
}


void K3bAudioTrackPlayer::slotSeek( int frames )
{
  seek( K3b::Msf( frames ) );
}


int K3bAudioTrackPlayer::read( char* data, int maxlen )
{
  if( m_currentTrack ) {
    d->mutex.lock();
    int len = m_currentTrack->read( data, maxlen );
    d->mutex.unlock();
    if( len > 0 ) {
      m_currentPosition += (int)( (double)len / 2352.0 + 0.5 );
    }
    else if( m_currentTrack->next() ) {
      // play the next track
      next();
      return read( data, maxlen );
    }
    else {
      stop();
      return -1; // no more tracks
    }

    return len;
  }
  else
    return -1;
}


void K3bAudioTrackPlayer::slotTrackRemoved( K3bAudioTrack* track )
{
  if( m_currentTrack == track ) {
    stop();
    m_currentTrack = 0;
  }
}


void K3bAudioTrackPlayer::slotTrackChanged( K3bAudioTrack* track )
{
  if( m_currentTrack == track ) {
    d->seekSlider->setMaxValue( track->length().totalFrames() );
  }
}


void K3bAudioTrackPlayer::slotUpdateSlider()
{
  d->seekSlider->setValue( m_currentPosition.totalFrames() );
}


void K3bAudioTrackPlayer::slotDocChanged()
{
  // update the controls in case a new track has been added before or after
  // the current one and it has been the first or last track
  if( m_currentTrack ) {
    d->actionNext->setEnabled( m_currentTrack->next() != 0 );
    d->actionPrev->setEnabled( m_currentTrack->prev() != 0 );
  }
}

#include "k3baudiotrackplayer.moc"
