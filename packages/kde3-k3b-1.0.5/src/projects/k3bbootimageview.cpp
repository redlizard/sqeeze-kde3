/*
 *
 * $Id: k3bbootimageview.cpp 619556 2007-01-03 17:38:12Z trueg $
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

#include "k3bbootimageview.h"

#include "k3bdatadoc.h"
#include "k3bbootitem.h"
#include <k3bintvalidator.h>

#include <klocale.h>
#include <klistview.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include <qpushbutton.h>
#include <qstring.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qregexp.h>



class K3bBootImageView::PrivateBootImageViewItem : public KListViewItem
{
public:
  PrivateBootImageViewItem( K3bBootItem* image, QListView* parent ) 
    : KListViewItem( parent ), 
      m_image( image ) {

  }

  PrivateBootImageViewItem( K3bBootItem* image, QListView* parent, QListViewItem* after )
    : KListViewItem( parent, after ),
      m_image( image ) {

  }

  QString text( int col ) const {
    if( col == 0 ) {
      if( m_image->imageType() == K3bBootItem::FLOPPY )
	return i18n("Floppy");
      else if( m_image->imageType() == K3bBootItem::HARDDISK )
	return i18n("Harddisk");
      else
	return i18n("None");
    }
    else if( col == 1 )
      return QString( "%1 KB" ).arg( m_image->size()/1024 );
    else if( col == 2 )
      return m_image->localPath();
    else
      return QString::null;
  }

  K3bBootItem* bootImage() const { return m_image; }

private:
  K3bBootItem* m_image;
};


K3bBootImageView::K3bBootImageView( K3bDataDoc* doc, QWidget* parent, const char* name )
  : base_K3bBootImageView( parent, name ),
    m_doc(doc)
{
  connect( m_buttonNew, SIGNAL(clicked()), 
	   this, SLOT(slotNewBootImage()) );
  connect( m_buttonDelete, SIGNAL(clicked()), 
	   this, SLOT(slotDeleteBootImage()) );
  connect( m_buttonToggleOptions, SIGNAL(clicked()),
	   this, SLOT(slotToggleOptions()) );
  connect( m_viewImages, SIGNAL(selectionChanged()),
	   this, SLOT(slotSelectionChanged()) );
  connect( m_radioNoEmulation, SIGNAL(toggled(bool)),
	   this, SLOT(slotNoEmulationToggled(bool)) );

  K3bIntValidator* v = new K3bIntValidator( this );
  m_editLoadSegment->setValidator( v );
  m_editLoadSize->setValidator( v );

  updateBootImages();

  showAdvancedOptions( false );
  loadBootItemSettings(0);
}

K3bBootImageView::~K3bBootImageView()
{
}


void K3bBootImageView::slotToggleOptions()
{
  showAdvancedOptions( !m_groupOptions->isVisible() );
}


void K3bBootImageView::showAdvancedOptions( bool show )
{
  if( show ) {
    m_groupOptions->show();
    m_buttonToggleOptions->setText( i18n("Hide Advanced Options") );
  }
  else {
    m_groupOptions->hide();
    m_buttonToggleOptions->setText( i18n("Show Advanced Options") );
  }
}


void K3bBootImageView::slotNewBootImage()
{
  QString file = KFileDialog::getOpenFileName( QString::null, QString::null, this, i18n("Please Choose Boot Image") );
  if( !file.isEmpty() ) {
    KIO::filesize_t fsize = K3b::filesize( file );
    int boottype = K3bBootItem::FLOPPY;
    if( fsize != 1200*1024 &&
	fsize != 1440*1024 &&
	fsize != 2880*1024 ) {
      switch( KMessageBox::warningYesNoCancel( this, 
					       i18n("<p>The file you selected is not a floppy image (floppy images are "
						    "of size 1200 KB, 1440 KB, or 2880 KB). You may still use boot images "
						    "of other sizes by emulating a harddisk or disabling emulation completely. "
						    "<p>If you are not familiar with terms like 'harddisk emulation' you most "
						    "likely want to use a floppy image here. Floppy images can be created by "
						    "directly extracting them from a real floppy disk:"
						    "<pre>dd if=/dev/floppy of=/tmp/floppy.img</pre>"
						    "or by using one of the many boot floppy generators that can be found on "
						    "<a href=\"http://www.google.com/search?q=linux+boot+floppy&ie=UTF-8&oe=UTF-8\">the internet</a>."),
					       i18n("No Floppy image selected"),
					       i18n("Use harddisk emulation"),
					       i18n("Use no emulation"),
					       QString::null,
					       KMessageBox::AllowLink ) ) {
      case KMessageBox::Yes:
	boottype = K3bBootItem::HARDDISK;
	break;
      case KMessageBox::No:
	boottype = K3bBootItem::NONE;
	break;
      default:
	return;
      }
    }

    m_doc->createBootItem( file )->setImageType( boottype );
    updateBootImages();
  }
}


void K3bBootImageView::slotDeleteBootImage()
{
  QListViewItem* item = m_viewImages->selectedItem();
  if( item ) {
    K3bBootItem* i = ((PrivateBootImageViewItem*)item)->bootImage();
    delete item;
    m_doc->removeItem( i );
  }
}


void K3bBootImageView::slotSelectionChanged()
{
  QListViewItem* item = m_viewImages->selectedItem();
  if( item )
    loadBootItemSettings( ((PrivateBootImageViewItem*)item)->bootImage() );
  else
    loadBootItemSettings( 0 );
}


void K3bBootImageView::updateBootImages()
{
  m_viewImages->clear();
  for( QPtrListIterator<K3bBootItem> it( m_doc->bootImages() ); it.current(); ++it ) {
    (void)new PrivateBootImageViewItem( *it, m_viewImages, 
					m_viewImages->lastItem() );
  }
}


void K3bBootImageView::loadBootItemSettings( K3bBootItem* item )
{
  // this is needed to prevent the slots to change stuff
  m_loadingItem = true;

  if( item ) {
    m_groupOptions->setEnabled(true);
    m_groupImageType->setEnabled(true);

    m_checkNoBoot->setChecked( item->noBoot() );
    m_checkInfoTable->setChecked( item->bootInfoTable() );
    m_editLoadSegment->setText( "0x" + QString::number( item->loadSegment(), 16 ) );
    m_editLoadSize->setText( "0x" + QString::number( item->loadSize(), 16 ) );

    if( item->imageType() == K3bBootItem::FLOPPY )
      m_radioFloppy->setChecked(true);
    else if( item->imageType() == K3bBootItem::HARDDISK )
      m_radioHarddisk->setChecked(true);      
    else
      m_radioNoEmulation->setChecked(true);

    // force floppy size
    KIO::filesize_t fsize = K3b::filesize( item->localPath() );
    m_radioFloppy->setDisabled( fsize != 1200*1024 &&
				fsize != 1440*1024 &&
				fsize != 2880*1024 );	
  }
  else {
    m_groupOptions->setEnabled(false);
    m_groupImageType->setEnabled(false);
  }

  m_loadingItem = false;
}


void K3bBootImageView::slotOptionsChanged()
{
  if( !m_loadingItem ) {
    QListViewItem* item = m_viewImages->selectedItem();
    if( item ) {
      K3bBootItem* i = ((PrivateBootImageViewItem*)item)->bootImage();
      
      i->setNoBoot( m_checkNoBoot->isChecked() );
      i->setBootInfoTable( m_checkInfoTable->isChecked() );

      // TODO: create some class K3bIntEdit : public QLineEdit
      bool ok = true;
      i->setLoadSegment( K3bIntValidator::toInt( m_editLoadSegment->text(), &ok ) );
      if( !ok )
	kdDebug() << "(K3bBootImageView) parsing number failed: " << m_editLoadSegment->text().lower() << endl;
      i->setLoadSize( K3bIntValidator::toInt( m_editLoadSize->text(), &ok ) );
      if( !ok )
	kdDebug() << "(K3bBootImageView) parsing number failed: " << m_editLoadSize->text().lower() << endl;

      if( m_radioFloppy->isChecked() )
	i->setImageType( K3bBootItem::FLOPPY );
      else if( m_radioHarddisk->isChecked() )
	i->setImageType( K3bBootItem::HARDDISK );
      else
	i->setImageType( K3bBootItem::NONE );
    }
  }
}


void K3bBootImageView::slotNoEmulationToggled( bool on )
{
  // it makes no sense to combine no emulation and no boot!
  // the base_widget takes care of the disabling
  if( on )
    m_checkNoBoot->setChecked(false);
}

#include "k3bbootimageview.moc"
