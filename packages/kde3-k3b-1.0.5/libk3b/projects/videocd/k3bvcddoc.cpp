/*
*
* $Id: k3bvcddoc.cpp 619556 2007-01-03 17:38:12Z trueg $
* Copyright (C) 2003-2005 Christian Kvasny <chris@k3b.org>
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

// QT-includes
#include <qstring.h>
#include <qstringlist.h>
#include <qfile.h>
#include <qdatastream.h>
#include <qdom.h>
#include <qdatetime.h>
#include <qtimer.h>
#include <qtextstream.h>

// KDE-includes
#include <kprocess.h>
#include <kurl.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kio/global.h>
#include <kdebug.h>
#include <kstdguiitem.h>

// K3b-includes
#include "k3bvcddoc.h"
#include "k3bvcdtrack.h"
#include "k3bvcdjob.h"
#include <k3bglobals.h>
#include <k3bmsf.h>


bool desperate_mode = false;
bool preserve_header = false;
bool print_progress = true;
bool aspect_correction = false;
byte forced_sequence_header = 0;

K3bVcdDoc::K3bVcdDoc( QObject* parent )
        : K3bDoc( parent )
{
    m_tracks = 0L;
    m_vcdOptions = new K3bVcdOptions();

    m_docType = VCD;
    m_vcdType = NONE;

    m_urlAddingTimer = new QTimer( this );
    connect( m_urlAddingTimer, SIGNAL( timeout() ), this, SLOT( slotWorkUrlQueue() ) );

    // FIXME: remove the newTracks() signal and replace it with the changed signal
    connect( this, SIGNAL( newTracks() ), this, SIGNAL( changed() ) );
    connect( this, SIGNAL( trackRemoved( K3bVcdTrack* ) ), this, SIGNAL( changed() ) );
}

K3bVcdDoc::~K3bVcdDoc()
{
    if ( m_tracks ) {
        m_tracks->setAutoDelete( true );
        delete m_tracks;
    }

    delete m_vcdOptions;
}

bool K3bVcdDoc::newDocument()
{
    if ( m_tracks )
        while ( m_tracks->first() )
            removeTrack( m_tracks->first() );
    else
        m_tracks = new QPtrList<K3bVcdTrack>;
    m_tracks->setAutoDelete( false );

    return K3bDoc::newDocument();
}


QString K3bVcdDoc::name() const
{
  return m_vcdOptions->volumeId();
}


KIO::filesize_t K3bVcdDoc::calcTotalSize() const
{
    unsigned long long sum = 0;
    if ( m_tracks ) {
        for ( K3bVcdTrack * track = m_tracks->first(); track; track = m_tracks->next() ) {
            sum += track->size();
        }
    }
    return sum;
}

KIO::filesize_t K3bVcdDoc::size() const
{
    // mode2 -> mode1 int(( n+2047 ) / 2048) * 2352
    // mode1 -> mode2 int(( n+2351 ) / 2352) * 2048
    long tracksize = long( ( calcTotalSize() + 2351 ) / 2352 ) * 2048;
    return tracksize + ISOsize();
}

KIO::filesize_t K3bVcdDoc::ISOsize() const
{
    // 136000b for vcd iso reseved
    long long iso_size = 136000;
    if ( vcdOptions() ->CdiSupport() ) {
        iso_size += vcdOptions() ->CDIsize();
    }

    return iso_size;
}

K3b::Msf K3bVcdDoc::length() const
{
    return K3b::Msf( size() / 2048 );
}


bool K3bVcdDoc::isImage( const KURL& url )
{
    QImage p;
    return p.load( QFile::encodeName( url.path() ) );
}

void K3bVcdDoc::addUrls( const KURL::List& urls )
{
    // make sure we add them at the end even if urls are in the queue
    addTracks( urls, 99 );
}

void K3bVcdDoc::addTracks( const KURL::List& urls, uint position )
{
    KURL::List::ConstIterator end( urls.end() );
    for ( KURL::List::ConstIterator it = urls.begin(); it != end; ++it ) {
        urlsToAdd.enqueue( new PrivateUrlToAdd( K3b::convertToLocalUrl(*it), position++ ) );
    }

    m_urlAddingTimer->start( 0 );
}

void K3bVcdDoc::slotWorkUrlQueue()
{
    if ( !urlsToAdd.isEmpty() ) {
        PrivateUrlToAdd * item = urlsToAdd.dequeue();
        lastAddedPosition = item->position;

        // append at the end by default
        if ( lastAddedPosition > m_tracks->count() )
            lastAddedPosition = m_tracks->count();

        if ( !item->url.isLocalFile() ) {
            kdDebug() << item->url.path() << " no local file" << endl;
            return ;
        }

        if ( !QFile::exists( item->url.path() ) ) {
            kdDebug() << "(K3bVcdDoc) file not found: " << item->url.path() << endl;
            m_notFoundFiles.append( item->url.path() );
            return ;
        }

        if ( K3bVcdTrack * newTrack = createTrack( item->url ) )
            addTrack( newTrack, lastAddedPosition );

        delete item;

        emit newTracks();
    } else {
        m_urlAddingTimer->stop();

        emit newTracks();

        // reorder pbc tracks
        setPbcTracks();

        informAboutNotFoundFiles();
    }
}

K3bVcdTrack* K3bVcdDoc::createTrack( const KURL& url )
{
    char filename[ 255 ];
    QString error_string = "";
    strcpy( filename, QFile::encodeName( url.path() ) );
    K3bMpegInfo* Mpeg = new K3bMpegInfo( filename );

    if ( Mpeg ) {
        int mpegVersion = Mpeg->version();
        if ( mpegVersion > 0 ) {

            if ( vcdType() == NONE && mpegVersion < 2 ) {
                m_urlAddingTimer->stop();
                setVcdType( vcdTypes( mpegVersion ) );
                vcdOptions() ->setMpegVersion( mpegVersion );
                KMessageBox::information( kapp->mainWidget(),
                                          i18n( "K3b will create a %1 image from the given MPEG "
                                                "files, but these files must already be in %2 "
                                                "format. K3b does not yet resample MPEG files." )
                                          .arg( i18n( "VCD" ) )
                                          .arg( i18n( "VCD" ) ),
                                          i18n( "Information" ) );
                m_urlAddingTimer->start( 0 );
            } else if ( vcdType() == NONE ) {
                m_urlAddingTimer->stop();
                vcdOptions() ->setMpegVersion( mpegVersion );
                bool force = false;
                force = ( KMessageBox::questionYesNo( kapp->mainWidget(),
                                                      i18n( "K3b will create a %1 image from the given MPEG "
                                                            "files, but these files must already be in %2 "
                                                            "format. K3b does not yet resample MPEG files." )
                                                      .arg( i18n( "SVCD" ) )
                                                      .arg( i18n( "SVCD" ) )
                                                      + "\n\n"
                                                      + i18n( "Note: Forcing MPEG2 as VCD is not supported by "
                                                              "some standalone DVD players." ),
                                                      i18n( "Information" ),
                                                      KStdGuiItem::ok().text(),
                                                      i18n( "Forcing VCD" ) ) == KMessageBox::No );
                if ( force ) {
                    setVcdType( vcdTypes( 1 ) );
                    vcdOptions() ->setAutoDetect( false );
                } else
                    setVcdType( vcdTypes( mpegVersion ) );

                m_urlAddingTimer->start( 0 );
            }


            if ( numOfTracks() > 0 && vcdOptions() ->mpegVersion() != mpegVersion ) {
                KMessageBox::error( kapp->mainWidget(), "(" + url.path() + ")\n" +
                                    i18n( "You cannot mix MPEG1 and MPEG2 video files.\nPlease start a new Project for this filetype.\nResample not implemented in K3b yet." ),
                                    i18n( "Wrong File Type for This Project" ) );

                delete Mpeg;
                return 0;
            }

            K3bVcdTrack* newTrack = new K3bVcdTrack( m_tracks, url.path() );
            *( newTrack->mpeg_info ) = *( Mpeg->mpeg_info );

            if ( newTrack->isSegment() && !vcdOptions()->PbcEnabled() ) {
                KMessageBox::information( kapp->mainWidget(),
                                          i18n( "PBC (Playback control) enabled.\n"
                                                "Videoplayers can not reach Segments (Mpeg Still Pictures) without Playback control ." ) ,
                                          i18n( "Information" ) );

                  vcdOptions()->setPbcEnabled( true );
             }

            // set defaults;
            newTrack->setPlayTime( vcdOptions() ->PbcPlayTime() );
            newTrack->setWaitTime( vcdOptions() ->PbcWaitTime() );
            newTrack->setPbcNumKeys( vcdOptions() ->PbcNumkeysEnabled() );
            delete Mpeg;

            // debugging output
            newTrack->PrintInfo();

            return newTrack;
        }
    } else if ( isImage( url ) ) { // image track
        // woking on ...
        // for future use
        // photoalbum starts here
        // return here the new photoalbum track
    }

    if ( Mpeg ) {
        error_string = Mpeg->error_string();
        delete Mpeg;
    }

    // error (unsupported files)
    KMessageBox::error( kapp->mainWidget(), "(" + url.path() + ")\n" +
                        i18n( "Only MPEG1 and MPEG2 video files are supported.\n" ) + error_string ,
                        i18n( "Wrong File Format" ) );


    return 0;
}

void K3bVcdDoc::addTrack( const KURL& url, uint position )
{
    urlsToAdd.enqueue( new PrivateUrlToAdd( url, position ) );

    m_urlAddingTimer->start( 0 );
}


void K3bVcdDoc::addTrack( K3bVcdTrack* track, uint position )
{
    if ( m_tracks->count() >= 98 ) {
        kdDebug() << "(K3bVcdDoc) VCD Green Book only allows 98 tracks." << endl;
        // TODO: show some messagebox
        delete track;
        return ;
    }

    lastAddedPosition = position;

    if ( !m_tracks->insert( position, track ) ) {
        lastAddedPosition = m_tracks->count();
        m_tracks->insert( m_tracks->count(), track );
    }

    if ( track->isSegment() )
        vcdOptions() ->increaseSegments( );
    else
        vcdOptions() ->increaseSequence( );

    emit newTracks();

    setModified( true );
}


void K3bVcdDoc::removeTrack( K3bVcdTrack* track )
{
    if ( !track ) {
        return ;
    }

    // set the current item to track
    if ( m_tracks->findRef( track ) >= 0 ) {
        // take the current item
        track = m_tracks->take();

        // remove all pbc references to us?
        if ( track->hasRevRef() )
            track->delRefToUs();

        // remove all pbc references from us?
        track->delRefFromUs();

        // emit signal before deleting the track to avoid crashes
        // when the view tries to call some of the tracks' methods
        emit trackRemoved( track );

        if ( track->isSegment() )
            vcdOptions() ->decreaseSegments( );
        else
            vcdOptions() ->decreaseSequence( );

        delete track;

        if ( numOfTracks() == 0 ) {
            setVcdType( NONE );
            vcdOptions() ->setAutoDetect( true );
        }

        // reorder pbc tracks
        setPbcTracks();
    }
}

void K3bVcdDoc::moveTrack( const K3bVcdTrack* track, const K3bVcdTrack* after )
{
    if ( track == after )
        return ;

    // set the current item to track
    m_tracks->findRef( track );
    // take the current item
    track = m_tracks->take();

    // if after == 0 findRef returnes -1
    int pos = m_tracks->findRef( after );
    m_tracks->insert( pos + 1, track );

    // reorder pbc tracks
    setPbcTracks();

    emit changed();
}


QString K3bVcdDoc::typeString() const
{
    return "vcd";
}


K3bBurnJob* K3bVcdDoc::newBurnJob( K3bJobHandler* hdl, QObject* parent )
{
    return new K3bVcdJob( this, hdl, parent );
}

void K3bVcdDoc::informAboutNotFoundFiles()
{
    if ( !m_notFoundFiles.isEmpty() ) {
        KMessageBox::informationList( view(), i18n( "Could not find the following files:" ),
                                      m_notFoundFiles, i18n( "Not Found" ) );

        m_notFoundFiles.clear();
    }
}

void K3bVcdDoc::setVcdType( int type )
{
    m_vcdType = type;
    switch ( type ) {
        case 0:
            //vcd 1.1
            vcdOptions() ->setVcdClass( "vcd" );
            vcdOptions() ->setVcdVersion( "1.1" );
            break;
        case 1:
            //vcd 2.0
            vcdOptions() ->setVcdClass( "vcd" );
            vcdOptions() ->setVcdVersion( "2.0" );
            break;
        case 2:
            //svcd 1.0
            vcdOptions() ->setVcdClass( "svcd" );
            vcdOptions() ->setVcdVersion( "1.0" );
            break;
        case 3:
            //hqvcd 1.0
            vcdOptions() ->setVcdClass( "hqvcd" );
            vcdOptions() ->setVcdVersion( "1.0" );
            break;

    }
}

void K3bVcdDoc::setPbcTracks()
{
    // reorder pbc tracks
    /*
    if ( !vcdOptions()->PbcEnabled() )
        return;
    */

    if ( m_tracks ) {
        int count = m_tracks->count();
        kdDebug() << QString( "K3bVcdDoc::setPbcTracks() - we have %1 tracks in list." ).arg( count ) << endl;

        QPtrListIterator<K3bVcdTrack> iterTrack( *m_tracks );
        K3bVcdTrack* track;
        while ( ( track = iterTrack.current() ) != 0 ) {
            ++iterTrack;
            for ( int i = 0; i < K3bVcdTrack::_maxPbcTracks; i++ ) {
                // do not change userdefined tracks
                if ( !track->isPbcUserDefined( i ) ) {
                    if ( track->getPbcTrack( i ) )
                        track->getPbcTrack( i ) ->delFromRevRefList( track );

                    K3bVcdTrack* t = 0L;
                    int index = track->index();

                    // we are the last track
                    if ( index == count - 1 ) {
                        switch ( i ) {
                            case K3bVcdTrack::PREVIOUS:
                                // we are not alone :)
                                if ( count > 1 ) {
                                    t = at( index - 1 );
                                    t->addToRevRefList( track );
                                    track->setPbcTrack( i, t );
                                } else {
                                    track->setPbcTrack( i );
                                    track->setPbcNonTrack( i, K3bVcdTrack::VIDEOEND );
                                }
                                break;
                            case K3bVcdTrack::AFTERTIMEOUT:
                            case K3bVcdTrack::NEXT:
                                track->setPbcTrack( i );
                                track->setPbcNonTrack( i, K3bVcdTrack::VIDEOEND );
                                break;
                            case K3bVcdTrack::RETURN:
                                track->setPbcTrack( i );
                                track->setPbcNonTrack( i, K3bVcdTrack::VIDEOEND );
                                break;
                            case K3bVcdTrack::DEFAULT:
                                track->setPbcTrack( i );
                                track->setPbcNonTrack( i, K3bVcdTrack::DISABLED );
                                break;
                        }
                    }
                    // we are the first track
                    else if ( index == 0 ) {
                        switch ( i ) {
                            case K3bVcdTrack::PREVIOUS:
                                track->setPbcTrack( i );
                                track->setPbcNonTrack( i, K3bVcdTrack::VIDEOEND );
                                break;
                            case K3bVcdTrack::AFTERTIMEOUT:
                            case K3bVcdTrack::NEXT:
                                t = at( index + 1 );
                                t->addToRevRefList( track );
                                track->setPbcTrack( i, t );
                                break;
                            case K3bVcdTrack::RETURN:
                                track->setPbcTrack( i );
                                track->setPbcNonTrack( i, K3bVcdTrack::VIDEOEND );
                                break;
                            case K3bVcdTrack::DEFAULT:
                                track->setPbcTrack( i );
                                track->setPbcNonTrack( i, K3bVcdTrack::DISABLED );
                                break;
                        }
                    }
                    // we are one of the other tracks and have PREVIOUS and NEXT Track
                    else {
                        switch ( i ) {
                            case K3bVcdTrack::PREVIOUS:
                                t = at( index - 1 );
                                t->addToRevRefList( track );
                                track->setPbcTrack( i, t );
                                break;
                            case K3bVcdTrack::AFTERTIMEOUT:
                            case K3bVcdTrack::NEXT:
                                t = at( index + 1 );
                                t->addToRevRefList( track );
                                track->setPbcTrack( i, t );
                                break;
                            case K3bVcdTrack::RETURN:
                                track->setPbcTrack( i );
                                track->setPbcNonTrack( i, K3bVcdTrack::VIDEOEND );
                                break;
                            case K3bVcdTrack::DEFAULT:
                                track->setPbcTrack( i );
                                track->setPbcNonTrack( i, K3bVcdTrack::DISABLED );
                                break;
                        }
                    }
                }
            }
        }
    }
}


bool K3bVcdDoc::loadDocumentData( QDomElement* root )
{
    newDocument();

    QDomNodeList nodes = root->childNodes();

    if ( nodes.length() < 3 )
        return false;

    if ( nodes.item( 0 ).nodeName() != "general" )
        return false;
    if ( !readGeneralDocumentData( nodes.item( 0 ).toElement() ) )
        return false;

    if ( nodes.item( 1 ).nodeName() != "vcd" )
        return false;

    if ( nodes.item( 2 ).nodeName() != "contents" )
        return false;


    // vcd Label
    QDomNodeList vcdNodes = nodes.item( 1 ).childNodes();

    for ( uint i = 0; i < vcdNodes.count(); i++ ) {
        QDomNode item = vcdNodes.item( i );
        QString name = item.nodeName();

        kdDebug() << QString( "(K3bVcdDoc::loadDocumentData) nodeName = '%1'" ).arg( name ) << endl;

        if ( name == "volumeId" )
            vcdOptions() ->setVolumeId( item.toElement().text() );
        else if ( name == "albumId" )
            vcdOptions() ->setAlbumId( item.toElement().text() );
        else if ( name == "volumeSetId" )
            vcdOptions() ->setVolumeSetId( item.toElement().text() );
        else if ( name == "preparer" )
            vcdOptions() ->setPreparer( item.toElement().text() );
        else if ( name == "publisher" )
            vcdOptions() ->setPublisher( item.toElement().text() );
        else if ( name == "vcdType" )
            setVcdType( vcdTypes( item.toElement().text().toInt() ) );
        else if ( name == "mpegVersion" )
            vcdOptions() ->setMpegVersion( item.toElement().text().toInt() );
        else if ( name == "PreGapLeadout" )
            vcdOptions() ->setPreGapLeadout( item.toElement().text().toInt() );
        else if ( name == "PreGapTrack" )
            vcdOptions() ->setPreGapTrack( item.toElement().text().toInt() );
        else if ( name == "FrontMarginTrack" )
            vcdOptions() ->setFrontMarginTrack( item.toElement().text().toInt() );
        else if ( name == "RearMarginTrack" )
            vcdOptions() ->setRearMarginTrack( item.toElement().text().toInt() );
        else if ( name == "FrontMarginTrackSVCD" )
            vcdOptions() ->setFrontMarginTrackSVCD( item.toElement().text().toInt() );
        else if ( name == "RearMarginTrackSVCD" )
            vcdOptions() ->setRearMarginTrackSVCD( item.toElement().text().toInt() );
        else if ( name == "volumeCount" )
            vcdOptions() ->setVolumeCount( item.toElement().text().toInt() );
        else if ( name == "volumeNumber" )
            vcdOptions() ->setVolumeNumber( item.toElement().text().toInt() );
        else if ( name == "AutoDetect" )
            vcdOptions() ->setAutoDetect( item.toElement().text().toInt() );
        else if ( name == "CdiSupport" )
            vcdOptions() ->setCdiSupport( item.toElement().text().toInt() );
        else if ( name == "NonCompliantMode" )
            vcdOptions() ->setNonCompliantMode( item.toElement().text().toInt() );
        else if ( name == "Sector2336" )
            vcdOptions() ->setSector2336( item.toElement().text().toInt() );
        else if ( name == "UpdateScanOffsets" )
            vcdOptions() ->setUpdateScanOffsets( item.toElement().text().toInt() );
        else if ( name == "RelaxedAps" )
            vcdOptions() ->setRelaxedAps( item.toElement().text().toInt() );
        else if ( name == "UseGaps" )
            vcdOptions() ->setUseGaps( item.toElement().text().toInt() );
        else if ( name == "PbcEnabled" )
            vcdOptions() ->setPbcEnabled( item.toElement().text().toInt() );
        else if ( name == "SegmentFolder" )
            vcdOptions() ->setSegmentFolder( item.toElement().text().toInt() );
        else if ( name == "Restriction" )
            vcdOptions() ->setRestriction( item.toElement().text().toInt() );
    }

    // vcd Tracks
    QDomNodeList trackNodes = nodes.item( 2 ).childNodes();

    for ( uint i = 0; i < trackNodes.length(); i++ ) {

        // check if url is available
        QDomElement trackElem = trackNodes.item( i ).toElement();
        QString url = trackElem.attributeNode( "url" ).value();
        if ( !QFile::exists( url ) )
            m_notFoundFiles.append( url );
        else {
            KURL k;
            k.setPath( url );
            if ( K3bVcdTrack * track = createTrack( k ) ) {
                track ->setPlayTime( trackElem.attribute( "playtime", "1" ).toInt() );
                track ->setWaitTime( trackElem.attribute( "waittime", "2" ).toInt() );
                track ->setReactivity( trackElem.attribute( "reactivity", "0" ).toInt() );
                track -> setPbcNumKeys( ( trackElem.attribute( "numkeys", "yes" ).contains( "yes" ) ) ? true : false );
                track -> setPbcNumKeysUserdefined( ( trackElem.attribute( "userdefinednumkeys", "no" ).contains( "yes" ) ) ? true : false );

                addTrack( track, m_tracks->count() );
            }
        }
    }

    emit newTracks();

    // do not add saved pbcTrack links when one ore more files missing.
    // TODO: add info message to informAboutNotFoundFiles();
    if ( m_notFoundFiles.isEmpty() ) {
        int type;
        int val;
        bool pbctrack;
        for ( uint trackId = 0; trackId < trackNodes.length(); trackId++ ) {
            QDomElement trackElem = trackNodes.item( trackId ).toElement();
            QDomNodeList trackNodes = trackElem.childNodes();
            for ( uint i = 0; i < trackNodes.length(); i++ ) {
                QDomElement trackElem = trackNodes.item( i ).toElement();
                QString name = trackElem.tagName();
                if ( name.contains( "pbc" ) ) {
                    if ( trackElem.hasAttribute ( "type" ) ) {
                        type = trackElem.attribute ( "type" ).toInt();
                        if ( trackElem.hasAttribute ( "pbctrack" ) ) {
                            pbctrack = ( trackElem.attribute ( "pbctrack" ) == "yes" );
                            if ( trackElem.hasAttribute ( "val" ) ) {
                                val = trackElem.attribute ( "val" ).toInt();
                                K3bVcdTrack* track = m_tracks->at( trackId );
                                K3bVcdTrack* pbcTrack = m_tracks->at( val );
                                if ( pbctrack ) {
                                    pbcTrack->addToRevRefList( track );
                                    track->setPbcTrack( type, pbcTrack );
                                    track->setUserDefined( type, true );
                                } else {
                                    track->setPbcTrack( type );
                                    track->setPbcNonTrack( type, val );
                                    track->setUserDefined( type, true );
                                }
                            }
                        }
                    }
                } else if ( name.contains( "numkeys" ) ) {
                    if ( trackElem.hasAttribute ( "key" ) ) {
                        int key = trackElem.attribute ( "key" ).toInt();
                        if ( trackElem.hasAttribute ( "val" ) ) {
                            int val = trackElem.attribute ( "val" ).toInt() - 1;
                            K3bVcdTrack* track = m_tracks->at( trackId );
                            if ( val >= 0 ) {
                                K3bVcdTrack * numkeyTrack = m_tracks->at( val );
                                track->setDefinedNumKey( key, numkeyTrack );
                            } else {
                                track->setDefinedNumKey( key, 0L );
                            }
                        }
                    }
                }

            }

        }
        setPbcTracks();
        setModified( false );
    }

    informAboutNotFoundFiles();
    return true;
}



bool K3bVcdDoc::saveDocumentData( QDomElement * docElem )
{
    QDomDocument doc = docElem->ownerDocument();
    saveGeneralDocumentData( docElem );

    // save Vcd Label
    QDomElement vcdMain = doc.createElement( "vcd" );

    QDomElement vcdElem = doc.createElement( "volumeId" );
    vcdElem.appendChild( doc.createTextNode( vcdOptions() ->volumeId() ) );
    vcdMain.appendChild( vcdElem );

    vcdElem = doc.createElement( "albumId" );
    vcdElem.appendChild( doc.createTextNode( vcdOptions() ->albumId() ) );
    vcdMain.appendChild( vcdElem );

    vcdElem = doc.createElement( "volumeSetId" );
    vcdElem.appendChild( doc.createTextNode( vcdOptions() ->volumeSetId() ) );
    vcdMain.appendChild( vcdElem );

    vcdElem = doc.createElement( "preparer" );
    vcdElem.appendChild( doc.createTextNode( vcdOptions() ->preparer() ) );
    vcdMain.appendChild( vcdElem );

    vcdElem = doc.createElement( "publisher" );
    vcdElem.appendChild( doc.createTextNode( vcdOptions() ->publisher() ) );
    vcdMain.appendChild( vcdElem );

    // applicationId()
    // systemId()

    vcdElem = doc.createElement( "vcdType" );
    vcdElem.appendChild( doc.createTextNode( QString::number( vcdType() ) ) );
    vcdMain.appendChild( vcdElem );

    vcdElem = doc.createElement( "mpegVersion" );
    vcdElem.appendChild( doc.createTextNode( QString::number( vcdOptions() ->mpegVersion() ) ) );
    vcdMain.appendChild( vcdElem );

    vcdElem = doc.createElement( "PreGapLeadout" );
    vcdElem.appendChild( doc.createTextNode( QString::number( vcdOptions() ->PreGapLeadout() ) ) );
    vcdMain.appendChild( vcdElem );

    vcdElem = doc.createElement( "PreGapTrack" );
    vcdElem.appendChild( doc.createTextNode( QString::number( vcdOptions() ->PreGapTrack() ) ) );
    vcdMain.appendChild( vcdElem );

    vcdElem = doc.createElement( "FrontMarginTrack" );
    vcdElem.appendChild( doc.createTextNode( QString::number( vcdOptions() ->FrontMarginTrack() ) ) );
    vcdMain.appendChild( vcdElem );

    vcdElem = doc.createElement( "RearMarginTrack" );
    vcdElem.appendChild( doc.createTextNode( QString::number( vcdOptions() ->RearMarginTrack() ) ) );
    vcdMain.appendChild( vcdElem );

    vcdElem = doc.createElement( "FrontMarginTrackSVCD" );
    vcdElem.appendChild( doc.createTextNode( QString::number( vcdOptions() ->FrontMarginTrackSVCD() ) ) );
    vcdMain.appendChild( vcdElem );

    vcdElem = doc.createElement( "RearMarginTrackSVCD" );
    vcdElem.appendChild( doc.createTextNode( QString::number( vcdOptions() ->RearMarginTrackSVCD() ) ) );
    vcdMain.appendChild( vcdElem );

    vcdElem = doc.createElement( "volumeCount" );
    vcdElem.appendChild( doc.createTextNode( QString::number( vcdOptions() ->volumeCount() ) ) );
    vcdMain.appendChild( vcdElem );

    vcdElem = doc.createElement( "volumeNumber" );
    vcdElem.appendChild( doc.createTextNode( QString::number( vcdOptions() ->volumeNumber() ) ) );
    vcdMain.appendChild( vcdElem );

    vcdElem = doc.createElement( "AutoDetect" );
    vcdElem.appendChild( doc.createTextNode( QString::number( vcdOptions() ->AutoDetect() ) ) );
    vcdMain.appendChild( vcdElem );

    vcdElem = doc.createElement( "CdiSupport" );
    vcdElem.appendChild( doc.createTextNode( QString::number( vcdOptions() ->CdiSupport() ) ) );
    vcdMain.appendChild( vcdElem );

    vcdElem = doc.createElement( "NonCompliantMode" );
    vcdElem.appendChild( doc.createTextNode( QString::number( vcdOptions() ->NonCompliantMode() ) ) );
    vcdMain.appendChild( vcdElem );

    vcdElem = doc.createElement( "Sector2336" );
    vcdElem.appendChild( doc.createTextNode( QString::number( vcdOptions() ->Sector2336() ) ) );
    vcdMain.appendChild( vcdElem );

    vcdElem = doc.createElement( "UpdateScanOffsets" );
    vcdElem.appendChild( doc.createTextNode( QString::number( vcdOptions() ->UpdateScanOffsets() ) ) );
    vcdMain.appendChild( vcdElem );

    vcdElem = doc.createElement( "RelaxedAps" );
    vcdElem.appendChild( doc.createTextNode( QString::number( vcdOptions() ->RelaxedAps() ) ) );
    vcdMain.appendChild( vcdElem );

    vcdElem = doc.createElement( "UseGaps" );
    vcdElem.appendChild( doc.createTextNode( QString::number( vcdOptions() ->UseGaps() ) ) );
    vcdMain.appendChild( vcdElem );

    vcdElem = doc.createElement( "PbcEnabled" );
    vcdElem.appendChild( doc.createTextNode( QString::number( vcdOptions() ->PbcEnabled() ) ) );
    vcdMain.appendChild( vcdElem );

    vcdElem = doc.createElement( "SegmentFolder" );
    vcdElem.appendChild( doc.createTextNode( QString::number( vcdOptions() ->SegmentFolder() ) ) );
    vcdMain.appendChild( vcdElem );

    vcdElem = doc.createElement( "Restriction" );
    vcdElem.appendChild( doc.createTextNode( QString::number( vcdOptions() ->Restriction() ) ) );
    vcdMain.appendChild( vcdElem );

    docElem->appendChild( vcdMain );

    // save the tracks
    // -------------------------------------------------------------
    QDomElement contentsElem = doc.createElement( "contents" );

    QPtrListIterator<K3bVcdTrack> iterTrack( *m_tracks );
    K3bVcdTrack* track;

    while ( ( track = iterTrack.current() ) != 0 ) {
        ++iterTrack;

        QDomElement trackElem = doc.createElement( "track" );
        trackElem.setAttribute( "url", KIO::decodeFileName( track->absPath() ) );
        trackElem.setAttribute( "playtime", track->getPlayTime() );
        trackElem.setAttribute( "waittime", track->getWaitTime() );
        trackElem.setAttribute( "reactivity", track->Reactivity() );
        trackElem.setAttribute( "numkeys", ( track->PbcNumKeys() ) ? "yes" : "no" );
        trackElem.setAttribute( "userdefinednumkeys", ( track->PbcNumKeysUserdefined() ) ? "yes" : "no" );

        for ( int i = 0;
                i < K3bVcdTrack::_maxPbcTracks;
                i++ ) {
            if ( track->isPbcUserDefined( i ) ) {
                // save pbcTracks
                QDomElement pbcElem = doc.createElement( "pbc" );
                pbcElem.setAttribute( "type", i );
                if ( track->getPbcTrack( i ) ) {
                    pbcElem.setAttribute( "pbctrack", "yes" );
                    pbcElem.setAttribute( "val", track->getPbcTrack( i ) ->index() );
                } else {
                    pbcElem.setAttribute( "pbctrack", "no" );
                    pbcElem.setAttribute( "val", track->getNonPbcTrack( i ) );
                }
                trackElem.appendChild( pbcElem );
            }
        }
        QMap<int, K3bVcdTrack*> numKeyMap = track->DefinedNumKey();
        QMap<int, K3bVcdTrack*>::const_iterator trackIt;

        for ( trackIt = numKeyMap.begin();
                trackIt != numKeyMap.end();
                ++trackIt ) {
            QDomElement numElem = doc.createElement( "numkeys" );
            if ( trackIt.data() ) {
                numElem.setAttribute( "key", trackIt.key() );
                numElem.setAttribute( "val", trackIt.data() ->index() + 1 );
            } else {
                numElem.setAttribute( "key", trackIt.key() );
                numElem.setAttribute( "val", 0 );
            }
            trackElem.appendChild( numElem );
        }

        contentsElem.appendChild( trackElem );
    }
    // -------------------------------------------------------------

    docElem->appendChild( contentsElem );

    return true;
}

#include "k3bvcddoc.moc"
