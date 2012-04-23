/*
 *
 * $Id: k3bdatadoc.cpp 619556 2007-01-03 17:38:12Z trueg $
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


#include "k3bdatadoc.h"
#include "k3bfileitem.h"
#include "k3bdiritem.h"
#include "k3bsessionimportitem.h"
#include "k3bdatajob.h"
#include "k3bbootitem.h"
#include "k3bspecialdataitem.h"
#include "k3bfilecompilationsizehandler.h"
#include "k3bmkisofshandler.h"
#include <k3bcore.h>
#include <k3bglobals.h>
#include <k3bmsf.h>
#include <k3biso9660.h>
#include <k3bdevicehandler.h>
#include <k3bdevice.h>
#include <k3btoc.h>
#include <k3btrack.h>
#include <k3bmultichoicedialog.h>
#include <k3bvalidators.h>

#include <qdir.h>
#include <qstring.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <qdom.h>
#include <qptrlist.h>

#include <kstandarddirs.h>
#include <kurl.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kprogress.h>
#include <kconfig.h>
#include <kapplication.h>


#include <string.h>
#include <stdlib.h>
#include <ctype.h>


/**
 * There are two ways to fill a data project with files and folders:
 * \li Use the addUrl and addUrls methods
 * \li or create your own K3bDirItems and K3bFileItems. The doc will be properly updated
 *     by the constructors of the items.
 */
K3bDataDoc::K3bDataDoc( QObject* parent )
  : K3bDoc( parent )
{
  m_root = 0;

  m_sizeHandler = new K3bFileCompilationSizeHandler();
}

K3bDataDoc::~K3bDataDoc()
{
  delete m_root;
  delete m_sizeHandler;
  //  delete m_oldSessionSizeHandler;
}


bool K3bDataDoc::newDocument()
{
  clearImportedSession();

  m_bootCataloge = 0;
  m_oldSessionSize = 0;
  m_bExistingItemsReplaceAll = m_bExistingItemsIgnoreAll = false;

  if( m_root ) {
    while( m_root->children().getFirst() )
      removeItem( m_root->children().getFirst() );
  }
  else
    m_root = new K3bRootItem( this );

  m_sizeHandler->clear();

  m_multisessionMode = AUTO;
  m_dataMode = K3b::DATA_MODE_AUTO;

  m_isoOptions = K3bIsoOptions();

  return K3bDoc::newDocument();
}


QString K3bDataDoc::name() const
{
  return m_isoOptions.volumeID();
}


void K3bDataDoc::setIsoOptions( const K3bIsoOptions& o )
{
  m_isoOptions = o;
  emit changed();
}


void K3bDataDoc::setVolumeID( const QString& v )
{
  m_isoOptions.setVolumeID( v );
  emit changed();
}


void K3bDataDoc::addUrls( const KURL::List& urls )
{
  addUrls( urls, root() );
}


void K3bDataDoc::addUrls( const KURL::List& l, K3bDirItem* dir )
{
  if( !dir )
    dir = root();

  KURL::List urls = K3b::convertToLocalUrls(l);

  for( KURL::List::ConstIterator it = urls.begin(); it != urls.end(); ++it ) {
    const KURL& url = *it;
    QFileInfo f( url.path() );
    QString k3bname = f.absFilePath().section( "/", -1 );

    // filenames cannot end in backslashes (mkisofs problem. See comments in k3bisoimager.cpp (escapeGraftPoint()))
    while( k3bname[k3bname.length()-1] == '\\' )
      k3bname.truncate( k3bname.length()-1 );

    // backup dummy name
    if( k3bname.isEmpty() )
      k3bname = "1";

    K3bDirItem* newDirItem = 0;

    // rename the new item if an item with that name already exists
    int cnt = 0;
    bool ok = false;
    while( !ok ) {
      ok = true;
      QString name( k3bname );
      if( cnt > 0 )
	name += QString("_%1").arg(cnt);
      if( K3bDataItem* oldItem = dir->find( name ) ) {
	if( f.isDir() && oldItem->isDir() ) {
	  // ok, just reuse the dir
	  newDirItem = static_cast<K3bDirItem*>(oldItem);
	}
	// directories cannot replace files in an old session (I think)
	// and also directories can for sure never be replaced (only be reused as above)
	// so we always rename if the old item is a dir.
	else if( !oldItem->isFromOldSession() ||
		 f.isDir() ||
		 oldItem->isDir() ) {
	  ++cnt;
	  ok = false;
	}
      }
    }
    if( cnt > 0 )
      k3bname += QString("_%1").arg(cnt);

    // QFileInfo::exists and QFileInfo::isReadable return false for broken symlinks :(
    if( f.isDir() && !f.isSymLink() ) {
      if( !newDirItem ) {
	newDirItem = new K3bDirItem( k3bname, this, dir );
      	newDirItem->setLocalPath( url.path() ); // HACK: see k3bdiritem.h
      }

      // recursively add all the files in the directory
      QStringList dlist = QDir( f.absFilePath() ).entryList( QDir::All|QDir::System|QDir::Hidden );
      dlist.remove(".");
      dlist.remove("..");
      KURL::List newUrls;
      for( QStringList::Iterator it = dlist.begin(); it != dlist.end(); ++it )
	newUrls.append( KURL::fromPathOrURL( f.absFilePath() + "/" + *it ) );
      addUrls( newUrls, newDirItem );
    }
    else if( f.isSymLink() || f.isFile() )
      (void)new K3bFileItem( url.path(), this, dir, k3bname );
  }

  emit changed();

  setModified( true );
}


bool K3bDataDoc::nameAlreadyInDir( const QString& name, K3bDirItem* dir )
{
  if( !dir )
    return false;
  else
    return ( dir->find( name ) != 0 );
}


K3bDirItem* K3bDataDoc::addEmptyDir( const QString& name, K3bDirItem* parent )
{
  K3bDirItem* item = new K3bDirItem( name, this, parent );

  setModified( true );

  return item;
}


KIO::filesize_t K3bDataDoc::size() const
{
  if( m_isoOptions.doNotCacheInodes() )
    return root()->blocks().mode1Bytes() + m_oldSessionSize;
  else
    return m_sizeHandler->blocks( m_isoOptions.followSymbolicLinks() ||
				  !m_isoOptions.createRockRidge() ).mode1Bytes() + m_oldSessionSize;
}


KIO::filesize_t K3bDataDoc::burningSize() const
{
  return size() - m_oldSessionSize; //m_oldSessionSizeHandler->size();
}


K3b::Msf K3bDataDoc::length() const
{
  // 1 block consists of 2048 bytes real data
  // and 1 block equals to 1 audio frame
  // so this is the way to calculate:

  return K3b::Msf( size() / 2048 );
}


K3b::Msf K3bDataDoc::burningLength() const
{
  return K3b::Msf( burningSize() / 2048 );
}


QString K3bDataDoc::typeString() const
{
  return QString::fromLatin1("data");
}


bool K3bDataDoc::loadDocumentData( QDomElement* rootElem )
{
  if( !root() )
    newDocument();

  QDomNodeList nodes = rootElem->childNodes();

  if( nodes.item(0).nodeName() != "general" ) {
    kdDebug() << "(K3bDataDoc) could not find 'general' section." << endl;
    return false;
  }
  if( !readGeneralDocumentData( nodes.item(0).toElement() ) )
    return false;


  // parse options
  // -----------------------------------------------------------------
  if( nodes.item(1).nodeName() != "options" ) {
    kdDebug() << "(K3bDataDoc) could not find 'options' section." << endl;
    return false;
  }
  if( !loadDocumentDataOptions( nodes.item(1).toElement() ) )
    return false;
  // -----------------------------------------------------------------



  // parse header
  // -----------------------------------------------------------------
  if( nodes.item(2).nodeName() != "header" ) {
    kdDebug() << "(K3bDataDoc) could not find 'header' section." << endl;
    return false;
  }
  if( !loadDocumentDataHeader( nodes.item(2).toElement() ) )
    return false;
  // -----------------------------------------------------------------



  // parse files
  // -----------------------------------------------------------------
  if( nodes.item(3).nodeName() != "files" ) {
    kdDebug() << "(K3bDataDoc) could not find 'files' section." << endl;
    return false;
  }

  if( m_root == 0 )
    m_root = new K3bRootItem( this );

  QDomNodeList filesList = nodes.item(3).childNodes();
  for( uint i = 0; i < filesList.count(); i++ ) {

    QDomElement e = filesList.item(i).toElement();
    if( !loadDataItem( e, root() ) )
      return false;
  }

  // -----------------------------------------------------------------

  //
  // Old versions of K3b do not properly save the boot catalog location
  // and name. So to ensure we have one around even if loading an old project
  // file we create a default one here.
  //
  if( !m_bootImages.isEmpty() && !m_bootCataloge )
    createBootCatalogeItem( m_bootImages.first()->parent() );


  informAboutNotFoundFiles();

  return true;
}


bool K3bDataDoc::loadDocumentDataOptions( QDomElement elem )
{
  QDomNodeList headerList = elem.childNodes();
  for( uint i = 0; i < headerList.count(); i++ ) {

    QDomElement e = headerList.item(i).toElement();
    if( e.isNull() )
      return false;

    if( e.nodeName() == "rock_ridge")
      m_isoOptions.setCreateRockRidge( e.attributeNode( "activated" ).value() == "yes" );

    else if( e.nodeName() == "joliet")
      m_isoOptions.setCreateJoliet( e.attributeNode( "activated" ).value() == "yes" );

    else if( e.nodeName() == "udf")
      m_isoOptions.setCreateUdf( e.attributeNode( "activated" ).value() == "yes" );

    else if( e.nodeName() == "joliet_allow_103_characters")
      m_isoOptions.setJolietLong( e.attributeNode( "activated" ).value() == "yes" );

    else if( e.nodeName() == "iso_allow_lowercase")
      m_isoOptions.setISOallowLowercase( e.attributeNode( "activated" ).value() == "yes" );

    else if( e.nodeName() == "iso_allow_period_at_begin")
      m_isoOptions.setISOallowPeriodAtBegin( e.attributeNode( "activated" ).value() == "yes" );

    else if( e.nodeName() == "iso_allow_31_char")
      m_isoOptions.setISOallow31charFilenames( e.attributeNode( "activated" ).value() == "yes" );

    else if( e.nodeName() == "iso_omit_version_numbers")
      m_isoOptions.setISOomitVersionNumbers( e.attributeNode( "activated" ).value() == "yes" );

    else if( e.nodeName() == "iso_omit_trailing_period")
      m_isoOptions.setISOomitTrailingPeriod( e.attributeNode( "activated" ).value() == "yes" );

    else if( e.nodeName() == "iso_max_filename_length")
      m_isoOptions.setISOmaxFilenameLength( e.attributeNode( "activated" ).value() == "yes" );

    else if( e.nodeName() == "iso_relaxed_filenames")
      m_isoOptions.setISOrelaxedFilenames( e.attributeNode( "activated" ).value() == "yes" );

    else if( e.nodeName() == "iso_no_iso_translate")
      m_isoOptions.setISOnoIsoTranslate( e.attributeNode( "activated" ).value() == "yes" );

    else if( e.nodeName() == "iso_allow_multidot")
      m_isoOptions.setISOallowMultiDot( e.attributeNode( "activated" ).value() == "yes" );

    else if( e.nodeName() == "iso_untranslated_filenames")
      m_isoOptions.setISOuntranslatedFilenames( e.attributeNode( "activated" ).value() == "yes" );

    else if( e.nodeName() == "follow_symbolic_links")
      m_isoOptions.setFollowSymbolicLinks( e.attributeNode( "activated" ).value() == "yes" );

    else if( e.nodeName() == "create_trans_tbl")
      m_isoOptions.setCreateTRANS_TBL( e.attributeNode( "activated" ).value() == "yes" );

    else if( e.nodeName() == "hide_trans_tbl")
      m_isoOptions.setHideTRANS_TBL( e.attributeNode( "activated" ).value() == "yes" );

    else if( e.nodeName() == "iso_level")
      m_isoOptions.setISOLevel( e.text().toInt() );

    else if( e.nodeName() == "discard_symlinks")
      m_isoOptions.setDiscardSymlinks( e.attributeNode( "activated" ).value() == "yes" );

    else if( e.nodeName() == "discard_broken_symlinks")
      m_isoOptions.setDiscardBrokenSymlinks( e.attributeNode( "activated" ).value() == "yes" );

    else if( e.nodeName() == "preserve_file_permissions")
      m_isoOptions.setPreserveFilePermissions( e.attributeNode( "activated" ).value() == "yes" );

    else if( e.nodeName() == "force_input_charset")
      m_isoOptions.setForceInputCharset( e.attributeNode( "activated" ).value() == "yes" );

    else if( e.nodeName() == "input_charset")
      m_isoOptions.setInputCharset( e.text() );

    else if( e.nodeName() == "do_not_cache_inodes" )
      m_isoOptions.setDoNotCacheInodes( e.attributeNode( "activated" ).value() == "yes" );

    else if( e.nodeName() == "whitespace_treatment" ) {
      if( e.text() == "strip" )
	m_isoOptions.setWhiteSpaceTreatment( K3bIsoOptions::strip );
      else if( e.text() == "extended" )
	m_isoOptions.setWhiteSpaceTreatment( K3bIsoOptions::extended );
      else if( e.text() == "extended" )
	m_isoOptions.setWhiteSpaceTreatment( K3bIsoOptions::replace );
      else
	m_isoOptions.setWhiteSpaceTreatment( K3bIsoOptions::noChange );
    }

    else if( e.nodeName() == "whitespace_replace_string")
      m_isoOptions.setWhiteSpaceTreatmentReplaceString( e.text() );

    else if( e.nodeName() == "data_track_mode" ) {
      if( e.text() == "mode1" )
	m_dataMode = K3b::MODE1;
      else if( e.text() == "mode2" )
	m_dataMode = K3b::MODE2;
      else
	m_dataMode = K3b::DATA_MODE_AUTO;
    }

    else if( e.nodeName() == "multisession" ) {
      QString mode = e.text();
      if( mode == "start" )
	setMultiSessionMode( START );
      else if( mode == "continue" )
	setMultiSessionMode( CONTINUE );
      else if( mode == "finish" )
	setMultiSessionMode( FINISH );
      else if( mode == "none" )
	setMultiSessionMode( NONE );
      else
	setMultiSessionMode( AUTO );
    }

    else if( e.nodeName() == "verify_data" )
      setVerifyData( e.attributeNode( "activated" ).value() == "yes" );

    else
      kdDebug() << "(K3bDataDoc) unknown option entry: " << e.nodeName() << endl;
  }

  return true;
}


bool K3bDataDoc::loadDocumentDataHeader( QDomElement headerElem )
{
  QDomNodeList headerList = headerElem.childNodes();
  for( uint i = 0; i < headerList.count(); i++ ) {

    QDomElement e = headerList.item(i).toElement();
    if( e.isNull() )
      return false;

    if( e.nodeName() == "volume_id" )
      m_isoOptions.setVolumeID( e.text() );

    else if( e.nodeName() == "application_id" )
      m_isoOptions.setApplicationID( e.text() );

    else if( e.nodeName() == "publisher" )
      m_isoOptions.setPublisher( e.text() );

    else if( e.nodeName() == "preparer" )
      m_isoOptions.setPreparer( e.text() );

    else if( e.nodeName() == "volume_set_id" )
      m_isoOptions.setVolumeSetId( e.text() );

    else if( e.nodeName() == "volume_set_size" )
      m_isoOptions.setVolumeSetSize( e.text().toInt() );

    else if( e.nodeName() == "volume_set_number" )
      m_isoOptions.setVolumeSetNumber( e.text().toInt() );

    else if( e.nodeName() == "system_id" )
      m_isoOptions.setSystemId( e.text() );

    else
      kdDebug() << "(K3bDataDoc) unknown header entry: " << e.nodeName() << endl;
  }

  return true;
}


bool K3bDataDoc::loadDataItem( QDomElement& elem, K3bDirItem* parent )
{
  K3bDataItem* newItem = 0;

  if( elem.nodeName() == "file" ) {
    QDomElement urlElem = elem.firstChild().toElement();
    if( urlElem.isNull() ) {
      kdDebug() << "(K3bDataDoc) file-element without url!" << endl;
      return false;
    }

    QFileInfo f( urlElem.text() );

    // We canot use exists() here since this always disqualifies broken symlinks
    if( !f.isFile() && !f.isSymLink() )
      m_notFoundFiles.append( urlElem.text() );

    // broken symlinks are not readable according to QFileInfo which is wrong in our case
    else if( f.isFile() && !f.isReadable() )
      m_noPermissionFiles.append( urlElem.text() );

    else if( !elem.attribute( "bootimage" ).isEmpty() ) {
      K3bBootItem* bootItem = new K3bBootItem( urlElem.text(),
					       this,
					       parent,
					       elem.attributeNode( "name" ).value() );
      if( elem.attribute( "bootimage" ) == "floppy" )
	bootItem->setImageType( K3bBootItem::FLOPPY );
      else if( elem.attribute( "bootimage" ) == "harddisk" )
	bootItem->setImageType( K3bBootItem::HARDDISK );
      else
	bootItem->setImageType( K3bBootItem::NONE );
      bootItem->setNoBoot( elem.attribute( "no_boot" ) == "yes" );
      bootItem->setBootInfoTable( elem.attribute( "boot_info_table" ) == "yes" );
      bootItem->setLoadSegment( elem.attribute( "load_segment" ).toInt() );
      bootItem->setLoadSize( elem.attribute( "load_size" ).toInt() );

      newItem = bootItem;
    }

    else {
      newItem = new K3bFileItem( urlElem.text(),
				 this,
				 parent,
				 elem.attributeNode( "name" ).value() );
    }
  }
  else if( elem.nodeName() == "special" ) {
    if( elem.attributeNode( "type" ).value() == "boot cataloge" )
      createBootCatalogeItem( parent )->setK3bName( elem.attributeNode( "name" ).value() );
  }
  else if( elem.nodeName() == "directory" ) {
    // This is for the VideoDVD project which already contains the *_TS folders
    K3bDirItem* newDirItem = 0;
    if( K3bDataItem* item = parent->find( elem.attributeNode( "name" ).value() ) ) {
      if( item->isDir() ) {
	newDirItem = static_cast<K3bDirItem*>(item);
      }
      else {
	kdError() << "(K3bDataDoc) INVALID DOCUMENT: item " << item->k3bPath() << " saved twice" << endl;
	return false;
      }
    }

    if( !newDirItem )
      newDirItem = new K3bDirItem( elem.attributeNode( "name" ).value(), this, parent );
    QDomNodeList childNodes = elem.childNodes();
    for( uint i = 0; i < childNodes.count(); i++ ) {

      QDomElement e = childNodes.item(i).toElement();
      if( !loadDataItem( e, newDirItem ) )
	return false;
    }

    newItem = newDirItem;
  }
  else {
    kdDebug() << "(K3bDataDoc) wrong tag in files-section: " << elem.nodeName() << endl;
    return false;
  }

  // load the sort weight
  if( newItem )
    newItem->setSortWeight( elem.attribute( "sort_weight", "0" ).toInt() );

  return true;
}


bool K3bDataDoc::saveDocumentData( QDomElement* docElem )
{
  QDomDocument doc = docElem->ownerDocument();

  saveGeneralDocumentData( docElem );

  // all options
  // ----------------------------------------------------------------------
  QDomElement optionsElem = doc.createElement( "options" );
  saveDocumentDataOptions( optionsElem );
  docElem->appendChild( optionsElem );
  // ----------------------------------------------------------------------

  // the header stuff
  // ----------------------------------------------------------------------
  QDomElement headerElem = doc.createElement( "header" );
  saveDocumentDataHeader( headerElem );
  docElem->appendChild( headerElem );


  // now do the "real" work: save the entries
  // ----------------------------------------------------------------------
  QDomElement topElem = doc.createElement( "files" );

  QPtrListIterator<K3bDataItem> it( root()->children() );
  for( ; it.current(); ++it ) {
    saveDataItem( it.current(), &doc, &topElem );
  }

  docElem->appendChild( topElem );
  // ----------------------------------------------------------------------

  return true;
}


void K3bDataDoc::saveDocumentDataOptions( QDomElement& optionsElem )
{
  QDomDocument doc = optionsElem.ownerDocument();

  QDomElement topElem = doc.createElement( "rock_ridge" );
  topElem.setAttribute( "activated", isoOptions().createRockRidge() ? "yes" : "no" );
  optionsElem.appendChild( topElem );

  topElem = doc.createElement( "joliet" );
  topElem.setAttribute( "activated", isoOptions().createJoliet() ? "yes" : "no" );
  optionsElem.appendChild( topElem );

  topElem = doc.createElement( "udf" );
  topElem.setAttribute( "activated", isoOptions().createUdf() ? "yes" : "no" );
  optionsElem.appendChild( topElem );

  topElem = doc.createElement( "joliet_allow_103_characters" );
  topElem.setAttribute( "activated", isoOptions().jolietLong() ? "yes" : "no" );
  optionsElem.appendChild( topElem );

  topElem = doc.createElement( "iso_allow_lowercase" );
  topElem.setAttribute( "activated", isoOptions().ISOallowLowercase() ? "yes" : "no" );
  optionsElem.appendChild( topElem );

  topElem = doc.createElement( "iso_allow_period_at_begin" );
  topElem.setAttribute( "activated", isoOptions().ISOallowPeriodAtBegin() ? "yes" : "no" );
  optionsElem.appendChild( topElem );

  topElem = doc.createElement( "iso_allow_31_char" );
  topElem.setAttribute( "activated", isoOptions().ISOallow31charFilenames() ? "yes" : "no" );
  optionsElem.appendChild( topElem );

  topElem = doc.createElement( "iso_omit_version_numbers" );
  topElem.setAttribute( "activated", isoOptions().ISOomitVersionNumbers() ? "yes" : "no" );
  optionsElem.appendChild( topElem );

  topElem = doc.createElement( "iso_omit_trailing_period" );
  topElem.setAttribute( "activated", isoOptions().ISOomitTrailingPeriod() ? "yes" : "no" );
  optionsElem.appendChild( topElem );

  topElem = doc.createElement( "iso_max_filename_length" );
  topElem.setAttribute( "activated", isoOptions().ISOmaxFilenameLength() ? "yes" : "no" );
  optionsElem.appendChild( topElem );

  topElem = doc.createElement( "iso_relaxed_filenames" );
  topElem.setAttribute( "activated", isoOptions().ISOrelaxedFilenames() ? "yes" : "no" );
  optionsElem.appendChild( topElem );

  topElem = doc.createElement( "iso_no_iso_translate" );
  topElem.setAttribute( "activated", isoOptions().ISOnoIsoTranslate() ? "yes" : "no" );
  optionsElem.appendChild( topElem );

  topElem = doc.createElement( "iso_allow_multidot" );
  topElem.setAttribute( "activated", isoOptions().ISOallowMultiDot() ? "yes" : "no" );
  optionsElem.appendChild( topElem );

  topElem = doc.createElement( "iso_untranslated_filenames" );
  topElem.setAttribute( "activated", isoOptions().ISOuntranslatedFilenames() ? "yes" : "no" );
  optionsElem.appendChild( topElem );

  topElem = doc.createElement( "follow_symbolic_links" );
  topElem.setAttribute( "activated", isoOptions().followSymbolicLinks() ? "yes" : "no" );
  optionsElem.appendChild( topElem );

  topElem = doc.createElement( "create_trans_tbl" );
  topElem.setAttribute( "activated", isoOptions().createTRANS_TBL() ? "yes" : "no" );
  optionsElem.appendChild( topElem );

  topElem = doc.createElement( "hide_trans_tbl" );
  topElem.setAttribute( "activated", isoOptions().hideTRANS_TBL() ? "yes" : "no" );
  optionsElem.appendChild( topElem );

  topElem = doc.createElement( "iso_level" );
  topElem.appendChild( doc.createTextNode( QString::number(isoOptions().ISOLevel()) ) );
  optionsElem.appendChild( topElem );

  topElem = doc.createElement( "discard_symlinks" );
  topElem.setAttribute( "activated", isoOptions().discardSymlinks() ? "yes" : "no" );
  optionsElem.appendChild( topElem );

  topElem = doc.createElement( "discard_broken_symlinks" );
  topElem.setAttribute( "activated", isoOptions().discardBrokenSymlinks() ? "yes" : "no" );
  optionsElem.appendChild( topElem );

  topElem = doc.createElement( "preserve_file_permissions" );
  topElem.setAttribute( "activated", isoOptions().preserveFilePermissions() ? "yes" : "no" );
  optionsElem.appendChild( topElem );

  topElem = doc.createElement( "force_input_charset" );
  topElem.setAttribute( "activated", isoOptions().forceInputCharset() ? "yes" : "no" );
  optionsElem.appendChild( topElem );

  topElem = doc.createElement( "do_not_cache_inodes" );
  topElem.setAttribute( "activated", isoOptions().doNotCacheInodes() ? "yes" : "no" );
  optionsElem.appendChild( topElem );

  topElem = doc.createElement( "input_charset" );
  topElem.appendChild( doc.createTextNode( isoOptions().inputCharset() ) );
  optionsElem.appendChild( topElem );


  topElem = doc.createElement( "whitespace_treatment" );
  switch( isoOptions().whiteSpaceTreatment() ) {
  case K3bIsoOptions::strip:
    topElem.appendChild( doc.createTextNode( "strip" ) );
    break;
  case K3bIsoOptions::extended:
    topElem.appendChild( doc.createTextNode( "extended" ) );
    break;
  case K3bIsoOptions::replace:
    topElem.appendChild( doc.createTextNode( "replace" ) );
    break;
  default:
    topElem.appendChild( doc.createTextNode( "noChange" ) );
    break;
  }
  optionsElem.appendChild( topElem );

  topElem = doc.createElement( "whitespace_replace_string" );
  topElem.appendChild( doc.createTextNode( isoOptions().whiteSpaceTreatmentReplaceString() ) );
  optionsElem.appendChild( topElem );

  topElem = doc.createElement( "data_track_mode" );
  if( m_dataMode == K3b::MODE1 )
    topElem.appendChild( doc.createTextNode( "mode1" ) );
  else if( m_dataMode == K3b::MODE2 )
    topElem.appendChild( doc.createTextNode( "mode2" ) );
  else
    topElem.appendChild( doc.createTextNode( "auto" ) );
  optionsElem.appendChild( topElem );


  // save multisession
  topElem = doc.createElement( "multisession" );
  switch( m_multisessionMode ) {
  case START:
    topElem.appendChild( doc.createTextNode( "start" ) );
    break;
  case CONTINUE:
    topElem.appendChild( doc.createTextNode( "continue" ) );
    break;
  case FINISH:
    topElem.appendChild( doc.createTextNode( "finish" ) );
    break;
  case NONE:
    topElem.appendChild( doc.createTextNode( "none" ) );
    break;
  default:
    topElem.appendChild( doc.createTextNode( "auto" ) );
    break;
  }
  optionsElem.appendChild( topElem );

  topElem = doc.createElement( "verify_data" );
  topElem.setAttribute( "activated", verifyData() ? "yes" : "no" );
  optionsElem.appendChild( topElem );
  // ----------------------------------------------------------------------
}


void K3bDataDoc::saveDocumentDataHeader( QDomElement& headerElem )
{
  QDomDocument doc = headerElem.ownerDocument();

  QDomElement topElem = doc.createElement( "volume_id" );
  topElem.appendChild( doc.createTextNode( isoOptions().volumeID() ) );
  headerElem.appendChild( topElem );

  topElem = doc.createElement( "volume_set_id" );
  topElem.appendChild( doc.createTextNode( isoOptions().volumeSetId() ) );
  headerElem.appendChild( topElem );

  topElem = doc.createElement( "volume_set_size" );
  topElem.appendChild( doc.createTextNode( QString::number(isoOptions().volumeSetSize()) ) );
  headerElem.appendChild( topElem );

  topElem = doc.createElement( "volume_set_number" );
  topElem.appendChild( doc.createTextNode( QString::number(isoOptions().volumeSetNumber()) ) );
  headerElem.appendChild( topElem );

  topElem = doc.createElement( "system_id" );
  topElem.appendChild( doc.createTextNode( isoOptions().systemId() ) );
  headerElem.appendChild( topElem );

  topElem = doc.createElement( "application_id" );
  topElem.appendChild( doc.createTextNode( isoOptions().applicationID() ) );
  headerElem.appendChild( topElem );

  topElem = doc.createElement( "publisher" );
  topElem.appendChild( doc.createTextNode( isoOptions().publisher() ) );
  headerElem.appendChild( topElem );

  topElem = doc.createElement( "preparer" );
  topElem.appendChild( doc.createTextNode( isoOptions().preparer() ) );
  headerElem.appendChild( topElem );
  // ----------------------------------------------------------------------
}


void K3bDataDoc::saveDataItem( K3bDataItem* item, QDomDocument* doc, QDomElement* parent )
{
  if( K3bFileItem* fileItem = dynamic_cast<K3bFileItem*>( item ) ) {
    if( m_oldSession.contains( fileItem ) ) {
      kdDebug() << "(K3bDataDoc) ignoring fileitem " << fileItem->k3bName() << " from old session while saving..." << endl;
    }
    else {
      QDomElement topElem = doc->createElement( "file" );
      topElem.setAttribute( "name", fileItem->k3bName() );
      QDomElement subElem = doc->createElement( "url" );
      subElem.appendChild( doc->createTextNode( fileItem->localPath() ) );
      topElem.appendChild( subElem );

      if( item->sortWeight() != 0 )
	topElem.setAttribute( "sort_weight", QString::number(item->sortWeight()) );

      parent->appendChild( topElem );

      // add boot options as attributes to preserve compatibility to older K3b versions
      if( K3bBootItem* bootItem = dynamic_cast<K3bBootItem*>( fileItem ) ) {
	if( bootItem->imageType() == K3bBootItem::FLOPPY )
	  topElem.setAttribute( "bootimage", "floppy" );
	else if( bootItem->imageType() == K3bBootItem::HARDDISK )
	  topElem.setAttribute( "bootimage", "harddisk" );
	else
	  topElem.setAttribute( "bootimage", "none" );

	topElem.setAttribute( "no_boot", bootItem->noBoot() ? "yes" : "no" );
	topElem.setAttribute( "boot_info_table", bootItem->bootInfoTable() ? "yes" : "no" );
	topElem.setAttribute( "load_segment", QString::number( bootItem->loadSegment() ) );
	topElem.setAttribute( "load_size", QString::number( bootItem->loadSize() ) );
      }
    }
  }
  else if( item == m_bootCataloge ) {
    QDomElement topElem = doc->createElement( "special" );
    topElem.setAttribute( "name", m_bootCataloge->k3bName() );
    topElem.setAttribute( "type", "boot cataloge" );

    parent->appendChild( topElem );
  }
  else if( K3bDirItem* dirItem = dynamic_cast<K3bDirItem*>( item ) ) {
    QDomElement topElem = doc->createElement( "directory" );
    topElem.setAttribute( "name", dirItem->k3bName() );

    if( item->sortWeight() != 0 )
      topElem.setAttribute( "sort_weight", QString::number(item->sortWeight()) );

    QPtrListIterator<K3bDataItem> it( dirItem->children() );
    for( ; it.current(); ++it ) {
      saveDataItem( it.current(), doc, &topElem );
    }

    parent->appendChild( topElem );
  }
}


void K3bDataDoc::removeItem( K3bDataItem* item )
{
  if( !item )
    return;

  if( item->isRemoveable() ) {
    delete item;
  }
  else
    kdDebug() << "(K3bDataDoc) tried to remove non-removable entry!" << endl;
}


void K3bDataDoc::itemRemovedFromDir( K3bDirItem*, K3bDataItem* removedItem )
{
  // update the project size
  if( !removedItem->isFromOldSession() )
    m_sizeHandler->removeFile( removedItem );

  // update the boot item list
  if( removedItem->isBootItem() ) {
    m_bootImages.removeRef( static_cast<K3bBootItem*>( removedItem ) );
    if( m_bootImages.isEmpty() ) {
      delete m_bootCataloge;
      m_bootCataloge = 0;
    }
  }

  emit itemRemoved( removedItem );
  emit changed();
}


void K3bDataDoc::itemAddedToDir( K3bDirItem*, K3bDataItem* item )
{
  // update the project size
  if( !item->isFromOldSession() )
    m_sizeHandler->addFile( item );

  // update the boot item list
  if( item->isBootItem() )
    m_bootImages.append( static_cast<K3bBootItem*>( item ) );

  emit itemAdded( item );
  emit changed();
}


void K3bDataDoc::moveItem( K3bDataItem* item, K3bDirItem* newParent )
{
  if( !item || !newParent ) {
    kdDebug() << "(K3bDataDoc) item or parentitem was NULL while moving." << endl;
    return;
  }

  if( !item->isMoveable() ) {
    kdDebug() << "(K3bDataDoc) item is not movable! " << endl;
    return;
  }

  item->reparent( newParent );
}


void K3bDataDoc::moveItems( QPtrList<K3bDataItem> itemList, K3bDirItem* newParent )
{
  if( !newParent ) {
    kdDebug() << "(K3bDataDoc) tried to move items to nowhere...!" << endl;
    return;
  }

  QPtrListIterator<K3bDataItem> it( itemList );
  for( ; it.current(); ++it ) {
    // check if newParent is subdir of item
    if( K3bDirItem* dirItem = dynamic_cast<K3bDirItem*>( it.current() ) ) {
      if( dirItem->isSubItem( newParent ) ) {
	continue;
      }
    }

    if( it.current()->isMoveable() )
      it.current()->reparent( newParent );
  }
}


K3bBurnJob* K3bDataDoc::newBurnJob( K3bJobHandler* hdl, QObject* parent )
{
  return new K3bDataJob( this, hdl, parent );
}


QString K3bDataDoc::treatWhitespace( const QString& path )
{

  // TODO:
  // It could happen that two files with different names
  // will have the same name after the treatment
  // Perhaps we should add a number at the end or something
  // similar (s.a.)


  if( isoOptions().whiteSpaceTreatment() != K3bIsoOptions::noChange ) {
    QString result = path;

    if( isoOptions().whiteSpaceTreatment() == K3bIsoOptions::replace ) {
      result.replace( ' ', isoOptions().whiteSpaceTreatmentReplaceString() );
    }
    else if( isoOptions().whiteSpaceTreatment() == K3bIsoOptions::strip ) {
      result.remove( ' ' );
    }
    else if( isoOptions().whiteSpaceTreatment() == K3bIsoOptions::extended ) {
      result.truncate(0);
      for( uint i = 0; i < path.length(); i++ ) {
	if( path[i] == ' ' ) {
	  if( path[i+1] != ' ' )
	    result.append( path[++i].upper() );
	}
	else
	  result.append( path[i] );
      }
    }

    kdDebug() << "(K3bDataDoc) converted " << path << " to " << result << endl;
    return result;
  }
  else
    return path;
}


void K3bDataDoc::prepareFilenames()
{
  m_needToCutFilenames = false;
  m_needToCutFilenameItems.clear();

  //
  // if joliet is used cut the names and rename if necessary
  // 64 characters for standard joliet and 103 characters for long joliet names
  //
  // Rockridge supports the full 255 UNIX chars and in case Rockridge is disabled we leave
  // it to mkisofs for now since handling all the options to alter the ISO9660 standard it just
  // too much.
  //

  K3bDataItem* item = root();
  unsigned int maxlen = ( isoOptions().jolietLong() ? 103 : 64 );
  while( (item = item->nextSibling()) ) {
    item->setWrittenName( treatWhitespace( item->k3bName() ) );
    
    if( isoOptions().createJoliet() && item->writtenName().length() > maxlen ) {
      m_needToCutFilenames = true;
      item->setWrittenName( K3b::cutFilename( item->writtenName(), maxlen ) );
      m_needToCutFilenameItems.append( item );
    }

    // TODO: check the Joliet charset
  }

  //
  // 3. check if a directory contains items with the same name
  //
  prepareFilenamesInDir( root() );
}


void K3bDataDoc::prepareFilenamesInDir( K3bDirItem* dir )
{
  if( !dir )
    return;

  QPtrList<K3bDataItem> sortedChildren;
  QPtrListIterator<K3bDataItem> it( dir->children() );

  for( it.toLast(); it.current(); --it ) {
    K3bDataItem* item = it.current();

    if( item->isDir() )
      prepareFilenamesInDir( dynamic_cast<K3bDirItem*>( item ) );
    
    // insertion sort
    unsigned int i = 0;
    while( i < sortedChildren.count() && item->writtenName() > sortedChildren.at(i)->writtenName() )
      ++i;
    
    sortedChildren.insert( i, item );
  }


  if( isoOptions().createJoliet() || isoOptions().createRockRidge() ) {
    QPtrList<K3bDataItem> sameNameList;
    while( !sortedChildren.isEmpty() ) {

      sameNameList.clear();

      do {
	sameNameList.append( sortedChildren.first() );
	sortedChildren.removeFirst();
      } while( !sortedChildren.isEmpty() &&
	       sortedChildren.first()->writtenName() == sameNameList.first()->writtenName() );

      if( sameNameList.count() > 1 ) {
	// now we need to rename the items
	unsigned int maxlen = 255;
	if( isoOptions().createJoliet() ) {
	  if( isoOptions().jolietLong() )
	    maxlen = 103;
	  else
	    maxlen = 64;
	}

	int cnt = 1;
	for( QPtrListIterator<K3bDataItem> it( sameNameList );
	     it.current(); ++it ) {
	  it.current()->setWrittenName( K3b::appendNumberToFilename( it.current()->writtenName(), cnt++, maxlen ) );
	}
      }
    }
  }
}


void K3bDataDoc::informAboutNotFoundFiles()
{
  if( !m_notFoundFiles.isEmpty() ) {
    KMessageBox::informationList( qApp->activeWindow(), i18n("Could not find the following files:"),
 				  m_notFoundFiles, i18n("Not Found") );
    m_notFoundFiles.clear();
  }

  if( !m_noPermissionFiles.isEmpty() ) {
    KMessageBox::informationList( qApp->activeWindow(), i18n("No permission to read the following files:"),
				  m_noPermissionFiles, i18n("No Read Permission") );

    m_noPermissionFiles.clear();
  }
}


void K3bDataDoc::setMultiSessionMode( K3bDataDoc::MultiSessionMode mode )
{
  if( m_multisessionMode == NONE || m_multisessionMode == START )
    clearImportedSession();

  m_multisessionMode = mode;
}


bool K3bDataDoc::importSession( K3bDevice::Device* device )
{
  K3bDevice::DiskInfo diskInfo = device->diskInfo();
  // DVD+RW media is reported as non-appendable
  if( !diskInfo.appendable() &&
      !(diskInfo.mediaType() & (K3bDevice::MEDIA_DVD_PLUS_RW|K3bDevice::MEDIA_DVD_RW_OVWR)) )
    return false;

  K3bDevice::Toc toc = device->readToc();
  if( toc.isEmpty() || 
      toc.last().type() != K3bDevice::Track::DATA )
    return false;

  long startSec = toc.last().firstSector().lba();
  K3bIso9660 iso( device, startSec );

  if( iso.open() ) {
    // remove previously imported sessions
    clearImportedSession();
    
    // set multisession option
    if( m_multisessionMode != FINISH && m_multisessionMode != AUTO )
      m_multisessionMode = CONTINUE;
    
    // since in iso9660 it is possible that two files share it's data
    // simply summing the file sizes could result in wrong values
    // that's why we use the size from the toc. This is more accurate
    // anyway since there might be files overwritten or removed
    m_oldSessionSize = toc.last().lastSector().mode1Bytes();
    
    kdDebug() << "(K3bDataDoc) imported session size: " << KIO::convertSize(m_oldSessionSize) << endl;
    
    // the track size for DVD+RW media and DVD-RW Overwrite media has nothing to do with the filesystem 
    // size. in that case we need to use the filesystem's size (which is ok since it's one track anyway,
    // no real multisession)
    if( diskInfo.mediaType() & (K3bDevice::MEDIA_DVD_PLUS_RW|K3bDevice::MEDIA_DVD_RW_OVWR) ) {
      m_oldSessionSize = iso.primaryDescriptor().volumeSpaceSize 
	* iso.primaryDescriptor().logicalBlockSize;
    }
    
    // import some former settings
    m_isoOptions.setCreateRockRidge( iso.firstRRDirEntry() != 0 );
    m_isoOptions.setCreateJoliet( iso.firstJolietDirEntry() != 0 );
    m_isoOptions.setVolumeID( iso.primaryDescriptor().volumeId );
    // TODO: also import some other pd fields
    
    const K3bIso9660Directory* rootDir = iso.firstRRDirEntry();
    // J�rg Schilling says that it is impossible to import the joliet tree for multisession
//     if( !rootDir )
//       rootDir = iso.firstJolietDirEntry();
    if( !rootDir )
      rootDir = iso.firstIsoDirEntry();
    
    if( rootDir ) {
      createSessionImportItems( rootDir, root() );
      emit changed();
      return true;
    }
    else {
      kdDebug() << "(K3bDataDoc::importSession) Could not find primary volume desc." << endl;
      return false;
    }
  }
  else {
    kdDebug() << "(K3bDataDoc) unable to read toc." << endl;
    return false;
  }
}


void K3bDataDoc::createSessionImportItems( const K3bIso9660Directory* importDir, K3bDirItem* parent )
{
  Q_ASSERT(importDir);
  QStringList entries = importDir->entries();
  entries.remove( "." );
  entries.remove( ".." );
  for( QStringList::const_iterator it = entries.begin();
       it != entries.end(); ++it ) {
    const K3bIso9660Entry* entry = importDir->entry( *it );
    K3bDataItem* oldItem = parent->find( entry->name() );
    if( entry->isDirectory() ) {
      K3bDirItem* dir = 0;
      if( oldItem && oldItem->isDir() ) {
	dir = (K3bDirItem*)oldItem;
      }
      else {
	// we overwrite without warning!
	if( oldItem )
	  removeItem( oldItem );
	dir = new K3bDirItem( entry->name(), this, parent );
      }

      dir->setRemoveable(false);
      dir->setRenameable(false);
      dir->setMoveable(false);
      dir->setHideable(false);
      dir->setWriteToCd(false);
      dir->setExtraInfo( i18n("From previous session") );
      m_oldSession.append( dir );

      createSessionImportItems( static_cast<const K3bIso9660Directory*>(entry), dir );
    }
    else {
      const K3bIso9660File* file = static_cast<const K3bIso9660File*>(entry);

      // we overwrite without warning!
      if( oldItem )
	removeItem( oldItem );

      K3bSessionImportItem* item = new K3bSessionImportItem( file, this, parent );
      item->setExtraInfo( i18n("From previous session") );
      m_oldSession.append( item );
    }
  }
}


void K3bDataDoc::clearImportedSession()
{
  //  m_oldSessionSizeHandler->clear();
  m_oldSessionSize = 0;
  m_oldSession.setAutoDelete(false);
  K3bDataItem* item = m_oldSession.first();
  while( !m_oldSession.isEmpty() ) {
    if( item == 0 )
      item = m_oldSession.first();

    if( item->isDir() ) {
      K3bDirItem* dir = (K3bDirItem*)item;
      if( dir->numDirs() + dir->numFiles() == 0 ) {
	// this imported dir is not needed anymore
	// since it is empty
	m_oldSession.remove();
	delete dir;
      }
      else {
	for( QPtrListIterator<K3bDataItem> it( dir->children() ); it.current(); ++it ) {
	  if( !m_oldSession.contains(it.current()) ) {
	    m_oldSession.remove();
	    // now the dir becomes a totally normal dir
	    dir->setRemoveable(true);
	    dir->setRenameable(true);
	    dir->setMoveable(true);
	    dir->setHideable(true);
	    dir->setWriteToCd(true);
	    dir->setExtraInfo( "" );
	    break;
	  }
	}
      }
    }
    else {
      m_oldSession.remove();
      delete item;
    }

    item = m_oldSession.next();
  }

  m_multisessionMode = AUTO;

  emit changed();
}


K3bDirItem* K3bDataDoc::bootImageDir()
{
  K3bDataItem* b = m_root->find( "boot" );
  if( !b ) {
    b = new K3bDirItem( "boot", this, m_root );
    setModified( true );
  }

  // if we cannot create the dir because there is a file named boot just use the root dir
  if( !b->isDir() )
    return m_root;
  else
    return static_cast<K3bDirItem*>(b);
}


K3bBootItem* K3bDataDoc::createBootItem( const QString& filename, K3bDirItem* dir )
{
  if( !dir )
    dir = bootImageDir();

  K3bBootItem* boot = new K3bBootItem( filename, this, dir );

  if( !m_bootCataloge )
    createBootCatalogeItem(dir);

  return boot;
}


K3bDataItem* K3bDataDoc::createBootCatalogeItem( K3bDirItem* dir )
{
  if( !m_bootCataloge ) {
    QString newName = "boot.catalog";
    int i = 0;
    while( dir->alreadyInDirectory( "boot.catalog" ) ) {
      ++i;
      newName = QString( "boot%1.catalog" ).arg(i);
    }

    K3bSpecialDataItem* b = new K3bSpecialDataItem( this, 0, dir, newName );
    m_bootCataloge = b;
    m_bootCataloge->setRemoveable(false);
    m_bootCataloge->setHideable(false);
    m_bootCataloge->setWriteToCd(false);
    m_bootCataloge->setExtraInfo( i18n("El Torito boot catalog file") );
    b->setMimeType( i18n("Boot catalog") );
  }
  else
    m_bootCataloge->reparent( dir );

  return m_bootCataloge;
}


QValueList<K3bDataItem*> K3bDataDoc::findItemByLocalPath( const QString& path ) const
{
  Q_UNUSED( path );
  return QValueList<K3bDataItem*>();
}


bool K3bDataDoc::sessionImported() const
{
  return !m_oldSession.isEmpty();
}

#include "k3bdatadoc.moc"
