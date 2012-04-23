/*
 *
 * $Id: k3bcdparanoialib.cpp 621693 2007-01-09 14:38:25Z trueg $
 * Copyright (C) 2003-2007 Sebastian Trueg <trueg@k3b.org>
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

#include "k3bcdparanoialib.h"

#include <k3bdevice.h>
#include <k3btoc.h>
#include <k3bmsf.h>

#include <kdebug.h>

#include <dlfcn.h>

#include <qfile.h>
#include <qmutex.h>


static bool s_haveLibCdio = false;


void* K3bCdparanoiaLib::s_libInterface = 0;
void* K3bCdparanoiaLib::s_libParanoia = 0;
int K3bCdparanoiaLib::s_counter = 0;


#define CDDA_IDENTIFY          s_haveLibCdio ? "cdio_cddap_identify" : "cdda_identify"
#define CDDA_CLOSE             s_haveLibCdio ? "cdio_cddap_close" : "cdda_close"
#define CDDA_OPEN              s_haveLibCdio ? "cdio_cddap_open" : "cdda_open"
#define CDDA_TRACK_FIRSTSECTOR s_haveLibCdio ? "cdio_cddap_track_firstsector" : "cdda_track_firstsector"
#define CDDA_TRACK_LASTSECTOR  s_haveLibCdio ? "cdio_cddap_track_lastsector" : "cdda_track_lastsector"
#define CDDA_VERBOSE_SET       s_haveLibCdio ? "cdio_cddap_verbose_set" : "cdda_verbose_set"
#define CDDA_DISC_FIRSTSECTOR  s_haveLibCdio ? "cdio_cddap_disc_firstsector" : "cdda_disc_firstsector"

#define PARANOIA_INIT          s_haveLibCdio ? "cdio_paranoia_init" : "paranoia_init"
#define PARANOIA_FREE          s_haveLibCdio ? "cdio_paranoia_free" : "paranoia_free"
#define PARANOIA_MODESET       s_haveLibCdio ? "cdio_paranoia_modeset" : "paranoia_modeset"
#define PARANOIA_SEEK          s_haveLibCdio ? "cdio_paranoia_seek" : "paranoia_seek"
#define PARANOIA_READ_LIMITED  s_haveLibCdio ? "cdio_paranoia_read_limited" : "paranoia_read_limited"


// from cdda_paranoia.h
#define PARANOIA_CB_READ           0
#define PARANOIA_CB_VERIFY         1
#define PARANOIA_CB_FIXUP_EDGE     2
#define PARANOIA_CB_FIXUP_ATOM     3
#define PARANOIA_CB_SCRATCH        4
#define PARANOIA_CB_REPAIR         5
#define PARANOIA_CB_SKIP           6
#define PARANOIA_CB_DRIFT          7
#define PARANOIA_CB_BACKOFF        8
#define PARANOIA_CB_OVERLAP        9
#define PARANOIA_CB_FIXUP_DROPPED 10
#define PARANOIA_CB_FIXUP_DUPED   11
#define PARANOIA_CB_READERR       12



static void paranoiaCallback( long, int status )
{
  // do nothing so far....
  return;

  switch( status ) {
  case -1:
    break;
  case -2:
    break;
  case PARANOIA_CB_READ:
    // no problem
    // does only this mean that the sector has been read?
//     m_lastReadSector = sector;  // this seems to be rather useless
//     m_readSectors++;
    break;
  case PARANOIA_CB_VERIFY:
    break;
  case PARANOIA_CB_FIXUP_EDGE:
    break;
  case PARANOIA_CB_FIXUP_ATOM:
    break;
  case PARANOIA_CB_SCRATCH:
    // scratch detected
    break;
  case PARANOIA_CB_REPAIR:
    break;
  case PARANOIA_CB_SKIP:
    // skipped sector
    break;
  case PARANOIA_CB_DRIFT:
    break;
  case PARANOIA_CB_BACKOFF:
    break;
  case PARANOIA_CB_OVERLAP:
    // sector does not seem to contain the current
    // sector but the amount of overlapped data
    //    m_overlap = sector;
    break;
  case PARANOIA_CB_FIXUP_DROPPED:
    break;
  case PARANOIA_CB_FIXUP_DUPED:
    break;
  case PARANOIA_CB_READERR:
    break;
  } 
}



extern "C" {
  struct cdrom_drive;
  struct cdrom_paranoia;

  // HINT: these pointers must NOT have the same name like the actual methods!
  //       I added "cdda_" as prefix
  //       Before doing that K3b crashed in cdda_open!
  //       Can anyone please explain that to me?

  // cdda_interface
  cdrom_drive* (*cdda_cdda_identify)(const char*, int, char**);
  int (*cdda_cdda_open)(cdrom_drive *d);
  int (*cdda_cdda_close)(cdrom_drive *d);
  long (*cdda_cdda_track_firstsector)( cdrom_drive*, int );
  long (*cdda_cdda_track_lastsector)( cdrom_drive*, int );
  long (*cdda_cdda_disc_firstsector)(cdrom_drive *d);
  void (*cdda_cdda_verbose_set)(cdrom_drive *d,int err_action, int mes_action);

  // cdda_paranoia
  cdrom_paranoia* (*cdda_paranoia_init)(cdrom_drive*);
  void (*cdda_paranoia_free)(cdrom_paranoia *p);
  void (*cdda_paranoia_modeset)(cdrom_paranoia *p, int mode);
  int16_t* (*cdda_paranoia_read_limited)(cdrom_paranoia *p, void(*callback)(long,int), int);
  long (*cdda_paranoia_seek)(cdrom_paranoia *p,long seek,int mode);
}

// from cdda_paranoia.h
#define PARANOIA_MODE_FULL        0xff
#define PARANOIA_MODE_DISABLE     0

#define PARANOIA_MODE_VERIFY      1
#define PARANOIA_MODE_FRAGMENT    2
#define PARANOIA_MODE_OVERLAP     4
#define PARANOIA_MODE_SCRATCH     8
#define PARANOIA_MODE_REPAIR      16
#define PARANOIA_MODE_NEVERSKIP   32



/**
 * Internal class used by K3bCdparanoiaLib
 */
class K3bCdparanoiaLibData
{
 public:
  K3bCdparanoiaLibData( K3bDevice::Device* dev )
    : m_device(dev),
      m_drive(0),
      m_paranoia(0),
      m_currentSector(0) {
    s_dataMap.insert( dev, this );
  }

  ~K3bCdparanoiaLibData() {
    paranoiaFree();

    s_dataMap.erase( m_device );
  }

  K3bDevice::Device* device() const { return m_device; }
  void paranoiaModeSet( int );
  bool paranoiaInit();
  void paranoiaFree();
  int16_t* paranoiaRead( void(*callback)(long,int), int maxRetries );
  long paranoiaSeek( long, int );
  long firstSector( int );
  long lastSector( int );
  long sector() const { return m_currentSector; }

  static K3bCdparanoiaLibData* data( K3bDevice::Device* dev ) {
    QMap<K3bDevice::Device*, K3bCdparanoiaLibData*>::const_iterator it = s_dataMap.find( dev );
    if( it == s_dataMap.constEnd() )
      return new K3bCdparanoiaLibData( dev );
    else
      return *it;
  }

  static void freeAll() {
    // clean up all K3bCdparanoiaLibData instances
    for( QMap<K3bDevice::Device*, K3bCdparanoiaLibData*>::iterator it = s_dataMap.begin();
	 it != s_dataMap.end(); ++it )
      delete it.data();
  }

 private:
  //
  // We have exactly one instance of K3bCdparanoiaLibData per device
  //
  static QMap<K3bDevice::Device*, K3bCdparanoiaLibData*> s_dataMap;

  K3bDevice::Device* m_device;

  cdrom_drive* m_drive;
  cdrom_paranoia* m_paranoia;

  long m_currentSector;

  QMutex mutex;
};


QMap<K3bDevice::Device*, K3bCdparanoiaLibData*> K3bCdparanoiaLibData::s_dataMap;

bool K3bCdparanoiaLibData::paranoiaInit()
{
  mutex.lock();

  if( m_drive )
    paranoiaFree();

  // since we use cdparanoia to open the device it is important to close
  // the device here
  m_device->close();

  m_drive = cdda_cdda_identify( QFile::encodeName(m_device->blockDeviceName()), 0, 0 );
  if( m_drive == 0 ) {
    mutex.unlock();
    return false;
  }

  //  cdda_cdda_verbose_set( m_drive, 1, 1 );

  cdda_cdda_open( m_drive );
  m_paranoia = cdda_paranoia_init( m_drive );
  if( m_paranoia == 0 ) {
    mutex.unlock();
    paranoiaFree();
    return false;
  }

  m_currentSector = 0;

  mutex.unlock();

  return true;
}


void K3bCdparanoiaLibData::paranoiaFree()
{
  mutex.lock();

  if( m_paranoia ) {
    cdda_paranoia_free( m_paranoia );
    m_paranoia = 0;
  }
  if( m_drive ) {
    cdda_cdda_close( m_drive );
    m_drive = 0;
  }

  mutex.unlock();
}


void K3bCdparanoiaLibData::paranoiaModeSet( int mode )
{
  mutex.lock();
  cdda_paranoia_modeset( m_paranoia, mode );
  mutex.unlock();
}


int16_t* K3bCdparanoiaLibData::paranoiaRead( void(*callback)(long,int), int maxRetries )
{
  if( m_paranoia ) {
    mutex.lock();
    int16_t* data = cdda_paranoia_read_limited( m_paranoia, callback, maxRetries );
    if( data )
      m_currentSector++;
    mutex.unlock();
    return data;
  }
  else
    return 0;
}


long K3bCdparanoiaLibData::firstSector( int track )
{
  if( m_drive ) {
    mutex.lock();
    long sector = cdda_cdda_track_firstsector( m_drive, track );
    mutex.unlock();
    return sector;
  }
  else
    return -1;
}

long K3bCdparanoiaLibData::lastSector( int track )
{
  if( m_drive ) {
    mutex.lock();
    long sector = cdda_cdda_track_lastsector(m_drive, track );
    mutex.unlock();
    return sector;
  }
  else
   return -1;
}


long K3bCdparanoiaLibData::paranoiaSeek( long sector, int mode )
{
  if( m_paranoia ) {
    mutex.lock();
    m_currentSector = cdda_paranoia_seek( m_paranoia, sector, mode );
    mutex.unlock();
    return m_currentSector;
  }
  else
    return -1;
}



class K3bCdparanoiaLib::Private
{
public:
  Private() 
    : device(0),
      currentSector(0),
      startSector(0),
      lastSector(0),
      status(S_OK),
      paranoiaLevel(0),
      neverSkip(true),
      maxRetries(5),
      data(0) {
  }

  ~Private() {
  }

  void updateParanoiaMode() {
    // from cdrdao 1.1.7
    int paranoiaMode = PARANOIA_MODE_FULL^PARANOIA_MODE_NEVERSKIP;

    switch( paranoiaLevel ) {
    case 0:
      paranoiaMode = PARANOIA_MODE_DISABLE;
      break;

    case 1:
      paranoiaMode |= PARANOIA_MODE_OVERLAP;
      paranoiaMode &= ~PARANOIA_MODE_VERIFY;
      break;

    case 2:
      paranoiaMode &= ~(PARANOIA_MODE_SCRATCH|PARANOIA_MODE_REPAIR);
      break;
    }

    if( neverSkip )
      paranoiaMode |= PARANOIA_MODE_NEVERSKIP;

    data->paranoiaModeSet( paranoiaMode );
  }

  // high-level api
  K3bDevice::Device* device;
  K3bDevice::Toc toc;
  long currentSector;
  long startSector;
  long lastSector;
  int status;
  unsigned int currentTrack;
  int paranoiaLevel;
  bool neverSkip;
  int maxRetries;

  K3bCdparanoiaLibData* data;
};


K3bCdparanoiaLib::K3bCdparanoiaLib()
{
  d = new Private();
  s_counter++;
}


K3bCdparanoiaLib::~K3bCdparanoiaLib()
{
  delete d;
  s_counter--;
  if( s_counter == 0 ) {
    K3bCdparanoiaLibData::freeAll();

    // cleanup the dynamically loaded lib
    dlclose( s_libInterface );
    dlclose( s_libParanoia );
    s_libInterface = 0;
    s_libParanoia = 0;
  }
}


bool K3bCdparanoiaLib::load()
{
  cdda_cdda_identify = (cdrom_drive* (*) (const char*, int, char**))dlsym( s_libInterface, CDDA_IDENTIFY );
  cdda_cdda_open = (int (*) (cdrom_drive*))dlsym( s_libInterface, CDDA_OPEN );
  cdda_cdda_close = (int (*) (cdrom_drive*))dlsym( s_libInterface, CDDA_CLOSE );
  cdda_cdda_track_firstsector = (long (*)(cdrom_drive*, int))dlsym( s_libInterface, CDDA_TRACK_FIRSTSECTOR );
  cdda_cdda_track_lastsector = (long (*)(cdrom_drive*, int))dlsym( s_libInterface, CDDA_TRACK_LASTSECTOR );
  cdda_cdda_verbose_set = (void (*)(cdrom_drive *d,int err_action, int mes_action))dlsym( s_libInterface, CDDA_VERBOSE_SET );
  cdda_cdda_disc_firstsector = (long (*)(cdrom_drive *d))dlsym( s_libInterface, CDDA_DISC_FIRSTSECTOR );

  cdda_paranoia_init = (cdrom_paranoia* (*)(cdrom_drive*))dlsym( s_libParanoia, PARANOIA_INIT );
  cdda_paranoia_free = (void (*)(cdrom_paranoia *p))dlsym( s_libParanoia, PARANOIA_FREE );
  cdda_paranoia_modeset = (void (*)(cdrom_paranoia *p, int mode))dlsym( s_libParanoia, PARANOIA_MODESET );
  cdda_paranoia_read_limited = (int16_t* (*)(cdrom_paranoia *p, void(*callback)(long,int), int))dlsym( s_libParanoia, PARANOIA_READ_LIMITED );
  cdda_paranoia_seek = (long (*)(cdrom_paranoia *p,long seek,int mode))dlsym( s_libParanoia, PARANOIA_SEEK );

  // check if all symbols could be resoled
  if( cdda_cdda_identify == 0 ) {
    kdDebug() << "(K3bCdparanoiaLib) Error: could not resolve 'cdda_identify'" << endl;
    return false;
  }
  if( cdda_cdda_open == 0 ) {
    kdDebug() << "(K3bCdparanoiaLib) Error: could not resolve 'cdda_open'" << endl;
    return false;
  }
  if( cdda_cdda_close == 0 ) {
    kdDebug() << "(K3bCdparanoiaLib) Error: could not resolve 'cdda_close'" << endl;
    return false;
  }
  if( cdda_cdda_track_firstsector == 0 ) {
    kdDebug() << "(K3bCdparanoiaLib) Error: could not resolve 'cdda_track_firstsector'" << endl;
    return false;
  }
  if( cdda_cdda_track_lastsector == 0 ) {
    kdDebug() << "(K3bCdparanoiaLib) Error: could not resolve 'cdda_track_lastsector'" << endl;
    return false;
  }
  if( cdda_cdda_disc_firstsector == 0 ) {
    kdDebug() << "(K3bCdparanoiaLib) Error: could not resolve 'cdda_disc_firstsector'" << endl;
    return false;
  }
  if( cdda_cdda_verbose_set == 0 ) {
    kdDebug() << "(K3bCdparanoiaLib) Error: could not resolve 'cdda_verbose_set'" << endl;
    return false;
  }

  if( cdda_paranoia_init == 0 ) {
    kdDebug() << "(K3bCdparanoiaLib) Error: could not resolve 'paranoia_init'" << endl;
    return false;
  }
  if( cdda_paranoia_free == 0 ) {
    kdDebug() << "(K3bCdparanoiaLib) Error: could not resolve 'paranoia_free'" << endl;
    return false;
  }
  if( cdda_paranoia_modeset == 0 ) {
    kdDebug() << "(K3bCdparanoiaLib) Error: could not resolve 'paranoia_modeset'" << endl;
    return false;
  }
  if( cdda_paranoia_read_limited == 0 ) {
    kdDebug() << "(K3bCdparanoiaLib) Error: could not resolve 'paranoia_read_limited'" << endl;
    return false;
  }
  if( cdda_paranoia_seek == 0 ) {
    kdDebug() << "(K3bCdparanoiaLib) Error: could not resolve 'paranoia_seek'" << endl;
    return false;
  }

  return true;
}



K3bCdparanoiaLib* K3bCdparanoiaLib::create()
{
  // check if libcdda_interface is avalilable
  if( s_libInterface == 0 ) {
    s_haveLibCdio = false;

    s_libInterface = dlopen( "libcdda_interface.so.0", RTLD_NOW|RTLD_GLOBAL );

    // try the redhat & Co. location
    if( s_libInterface == 0 )
      s_libInterface = dlopen( "cdda/libcdda_interface.so.0", RTLD_NOW|RTLD_GLOBAL );

    // try the new cdio lib
    if( s_libInterface == 0 ) {
      s_libInterface = dlopen( "libcdio_cdda.so", RTLD_NOW|RTLD_GLOBAL );
      s_haveLibCdio = true;
    }

    if( s_libInterface == 0 ) {
      kdDebug() << "(K3bCdparanoiaLib) Error while loading libcdda_interface. " << endl;
      return 0;
    }


    s_libParanoia = dlopen( "libcdda_paranoia.so.0", RTLD_NOW );

    // try the redhat & Co. location
    if( s_libParanoia == 0 )
      s_libParanoia = dlopen( "cdda/libcdda_paranoia.so.0", RTLD_NOW );

    // try the new cdio lib
    if( s_haveLibCdio && s_libParanoia == 0 )
      s_libParanoia = dlopen( "libcdio_paranoia.so.0", RTLD_NOW );

    if( s_libParanoia == 0 ) {
      kdDebug() << "(K3bCdparanoiaLib) Error while loading libcdda_paranoia. " << endl;
      dlclose( s_libInterface );
      s_libInterface = 0;
      return 0;
    }
  }

  K3bCdparanoiaLib* lib = new K3bCdparanoiaLib();
  if( !lib->load() ) {
    kdDebug() << "(K3bCdparanoiaLib) Error: could not resolve all symbols!" << endl;
    delete lib;
    return 0;
  }
  return lib;
}


bool K3bCdparanoiaLib::initParanoia( K3bDevice::Device* dev, const K3bDevice::Toc& toc )
{
  if( !dev ) {
    kdError() << "(K3bCdparanoiaLib::initParanoia) dev = 0!" << endl;
    return false;
  }

  close();

  d->device = dev;
  d->toc = toc;
  if( d->toc.isEmpty() ) {
    kdDebug() << "(K3bCdparanoiaLib) empty toc." << endl;
    cleanup();
    return false;
  }

  if( d->toc.contentType() == K3bDevice::DATA ) {
    kdDebug() << "(K3bCdparanoiaLib) No audio tracks found." << endl;
    cleanup();
    return false;
  }

  //
  // Get the appropriate data instance for this device
  //
  d->data = K3bCdparanoiaLibData::data( dev );

  if( d->data->paranoiaInit() ) {
    d->startSector = d->currentSector = d->lastSector = 0;

    return true;
  }
  else {
    cleanup();
    return false;
  }
}


bool K3bCdparanoiaLib::initParanoia( K3bDevice::Device* dev )
{
  return initParanoia( dev, dev->readToc() );
}


void K3bCdparanoiaLib::close()
{
  cleanup();
}


void K3bCdparanoiaLib::cleanup()
{
  if( d->data )
    d->data->paranoiaFree();
  d->device = 0;
  d->currentSector = 0;
}


bool K3bCdparanoiaLib::initReading()
{
  if( d->device ) {
    // find first audio track
    K3bDevice::Toc::const_iterator trackIt = d->toc.begin();
    while( (*trackIt).type() != K3bDevice::Track::AUDIO ) {
      ++trackIt;
    }

    long start = (*trackIt).firstSector().lba();

    // find last audio track
    while( trackIt != d->toc.end() && (*trackIt).type() == K3bDevice::Track::AUDIO )
      ++trackIt;
    --trackIt;

    long end = (*trackIt).lastSector().lba();

    return initReading( start, end );
  }
  else {
    kdDebug() << "(K3bCdparanoiaLib) initReading without initParanoia." << endl;
    return false;
  }
}


bool K3bCdparanoiaLib::initReading( unsigned int track )
{
  if( d->device ) {
    if( track <= d->toc.count() ) {
      const K3bDevice::Track& k3bTrack = d->toc[track-1];
      if( k3bTrack.type() == K3bDevice::Track::AUDIO ) {
	return initReading( k3bTrack.firstSector().lba(), k3bTrack.lastSector().lba() );
      }
      else {
	kdDebug() << "(K3bCdparanoiaLib) Track " << track << " no audio track." << endl;
	return false;
      }
    }
    else {
      kdDebug() << "(K3bCdparanoiaLib) Track " << track << " too high." << endl;
      return false;
    }
  }
  else {
    kdDebug() << "(K3bCdparanoiaLib) initReading without initParanoia." << endl;
    return false;
  }
}


bool K3bCdparanoiaLib::initReading( long start, long end )
{
  kdDebug() << "(K3bCdparanoiaLib) initReading( " << start << ", " << end << " )" << endl;

  if( d->device ) {
    if( d->toc.firstSector().lba() <= start &&
	d->toc.lastSector().lba() >= end ) {
      d->startSector = d->currentSector = start;
      d->lastSector = end;

      // determine track number
      d->currentTrack = 1;
      while( d->toc[d->currentTrack-1].lastSector() < start )
	d->currentTrack++;

      // let the paranoia stuff point to the startSector
      d->data->paranoiaSeek( start, SEEK_SET );
      return true;
    }
    else {
      kdDebug() << "(K3bCdparanoiaLib) " << start << " and " << end << " out of range." << endl;
      return false;
    }
  }
  else {
    kdDebug() << "(K3bCdparanoiaLib) initReading without initParanoia." << endl;
    return false;
  }
}


char* K3bCdparanoiaLib::read( int* statusCode, unsigned int* track, bool littleEndian )
{
  if( d->currentSector > d->lastSector ) {
    kdDebug() << "(K3bCdparanoiaLib) finished ripping. read " 
	      << (d->currentSector - d->startSector) << " sectors." << endl
	      << "                   current sector: " << d->currentSector << endl;
    d->status = S_OK;
    if( statusCode )
      *statusCode = d->status;
    return 0;
  }

  if( d->currentSector != d->data->sector() ) {
    kdDebug() << "(K3bCdparanoiaLib) need to seek before read. Looks as if we are reusing the paranoia instance." << endl;
    if( !d->data->paranoiaSeek( d->currentSector, SEEK_SET ) )
      return 0;
  }

  //
  // The paranoia data could have been used by someone else before
  // and setting the paranoia mode is fast
  //
  d->updateParanoiaMode();

  Q_INT16* data = d->data->paranoiaRead( paranoiaCallback, d->maxRetries );

  char* charData = reinterpret_cast<char*>(data);

#ifdef WORDS_BIGENDIAN // __BYTE_ORDER == __BIG_ENDIAN
  if( littleEndian ) {
#else
  if( !littleEndian ) {
#endif
    for( int i = 0; i < CD_FRAMESIZE_RAW-1; i+=2 ) {
      char b = charData[i];
      charData[i] = charData[i+1];
      charData[i+1] = b;
    }
  }


  if( data )
    d->status = S_OK;
  else
    d->status = S_ERROR; // We may skip this sector if we'd like...

  if( statusCode )
    *statusCode = d->status;

  if( track )
    *track = d->currentTrack;

  d->currentSector++;

  if( d->toc[d->currentTrack-1].lastSector() < d->currentSector )
    d->currentTrack++;

  return charData;
}


int K3bCdparanoiaLib::status() const
{
  return d->status;
}


const K3bDevice::Toc& K3bCdparanoiaLib::toc() const
{
  return d->toc;
}


long K3bCdparanoiaLib::rippedDataLength() const
{
  return d->lastSector - d->startSector + 1;
}


void K3bCdparanoiaLib::setParanoiaMode( int m )
{
  d->paranoiaLevel = m;
}


void K3bCdparanoiaLib::setNeverSkip( bool b )
{
  d->neverSkip = b;
}


void K3bCdparanoiaLib::setMaxRetries( int r )
{
  d->maxRetries = r;
}
