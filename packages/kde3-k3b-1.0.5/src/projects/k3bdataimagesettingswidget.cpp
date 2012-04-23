/*
 *
 * $Id: k3bdataimagesettingswidget.cpp 691413 2007-07-23 16:01:13Z trueg $
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

#include "k3bdataimagesettingswidget.h"
#include "k3bdataadvancedimagesettingswidget.h"
#include "k3bdatavolumedescwidget.h"

#include "k3bisooptions.h"

#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>
#include <qlayout.h>

#include <kmessagebox.h>
#include <klocale.h>
#include <kdialogbase.h>
#include <kdebug.h>


// indices for the filesystems combobox
static const int FS_LINUX_ONLY = 0;
static const int FS_LINUX_AND_WIN = 1;
static const int FS_UDF = 2;
static const int FS_DOS_COMP = 3;
static const int FS_CUSTOM = 4;
static const int FS_MAX = 5;

static const char* s_fsPresetNames[] = {
  I18N_NOOP("Linux/Unix only"),
  I18N_NOOP("Linux/Unix + Windows"),
  I18N_NOOP("Very large files (UDF)"),
  I18N_NOOP("DOS Compatibility"),
  I18N_NOOP("Custom")
};

static bool s_fsPresetsInitialized = false;
static K3bIsoOptions s_fsPresets[FS_CUSTOM];

// indices for the whitespace treatment combobox
static const int WS_NO_CHANGE = 0;
static const int WS_STRIP = 1;
static const int WS_EXTENDED_STRIP = 2;
static const int WS_REPLACE = 3;

// indices for the symlink handling combobox
static const int SYM_NO_CHANGE = 0;
static const int SYM_DISCARD_BROKEN = 1;
static const int SYM_DISCARD_ALL = 2;
static const int SYM_FOLLOW = 3;


//
// returns true if the part of the options that is presented in the advanced custom
// settings dialog is equal. used to determine if some preset is used.
//
static bool compareAdvancedOptions( const K3bIsoOptions& o1, const K3bIsoOptions& o2 )
{
  return ( o1.forceInputCharset() == o2.forceInputCharset() &&
	   ( !o1.forceInputCharset() || o1.inputCharset() == o2.inputCharset() ) &&
	   o1.createRockRidge() == o2.createRockRidge() &&
	   o1.createJoliet() == o2.createJoliet() &&
	   o1.createUdf() == o2.createUdf() &&
	   o1.ISOallowLowercase() == o2.ISOallowLowercase() &&
	   o1.ISOallowPeriodAtBegin() == o2.ISOallowPeriodAtBegin() &&
	   o1.ISOallow31charFilenames() == o2.ISOallow31charFilenames() &&
	   o1.ISOomitVersionNumbers() == o2.ISOomitVersionNumbers() &&
	   o1.ISOomitTrailingPeriod() == o2.ISOomitTrailingPeriod() &&
	   o1.ISOmaxFilenameLength() == o2.ISOmaxFilenameLength() &&
	   o1.ISOrelaxedFilenames() == o2.ISOrelaxedFilenames() &&
	   o1.ISOnoIsoTranslate() == o2.ISOnoIsoTranslate() &&
	   o1.ISOallowMultiDot() == o2.ISOallowMultiDot() &&
	   o1.ISOuntranslatedFilenames() == o2.ISOuntranslatedFilenames() &&
	   o1.createTRANS_TBL() == o2.createTRANS_TBL() &&
	   o1.hideTRANS_TBL() == o2.hideTRANS_TBL() &&
	   o1.jolietLong() == o2.jolietLong() &&
	   o1.ISOLevel() == o2.ISOLevel() &&
	   o1.preserveFilePermissions() == o2.preserveFilePermissions() &&
	   o1.doNotCacheInodes() == o2.doNotCacheInodes() );
}


static void initializePresets()
{
  // Linux-only
  s_fsPresets[FS_LINUX_ONLY].setCreateJoliet( false );
  s_fsPresets[FS_LINUX_ONLY].setISOallow31charFilenames( true );

  // Linux + Windows
  s_fsPresets[FS_LINUX_AND_WIN].setCreateJoliet( true );
  s_fsPresets[FS_LINUX_AND_WIN].setJolietLong( true );
  s_fsPresets[FS_LINUX_AND_WIN].setISOallow31charFilenames( true );

  // UDF
  s_fsPresets[FS_UDF].setCreateJoliet( false );
  s_fsPresets[FS_UDF].setCreateUdf( true );
  s_fsPresets[FS_UDF].setISOallow31charFilenames( true );

  // DOS comp
  s_fsPresets[FS_DOS_COMP].setCreateJoliet( false );
  s_fsPresets[FS_DOS_COMP].setCreateRockRidge( false );
  s_fsPresets[FS_DOS_COMP].setISOallow31charFilenames( false );
  s_fsPresets[FS_DOS_COMP].setISOLevel( 1 );

  s_fsPresetsInitialized = true;
}



class K3bDataImageSettingsWidget::CustomFilesystemsDialog : public KDialogBase
{
public:
  CustomFilesystemsDialog( QWidget* parent )
    : KDialogBase( parent,
		   "custom_filesystems_dialog",
		   true,
		   i18n("Custom Data Project Filesystems"),
		   Ok|Cancel,
		   Ok,
		   true ) {
    w = new K3bDataAdvancedImageSettingsWidget( this );
    setMainWidget( w );
  }

  K3bDataAdvancedImageSettingsWidget* w;
};


class K3bDataImageSettingsWidget::VolumeDescDialog : public KDialogBase
{
public:
  VolumeDescDialog( QWidget* parent )
    : KDialogBase( parent,
		   "voldesc_dialog",
		   true,
		   i18n("Volume Descriptor"),
		   Ok|Cancel,
		   Ok,
		   true ) {
    w = new K3bDataVolumeDescWidget( this );
    setMainWidget( w );

    // give ourselves a reasonable size
    QSize s = sizeHint();
    s.setWidth( QMAX(s.width(), 300) );
    resize( s );
  }

  K3bDataVolumeDescWidget* w;
};



K3bDataImageSettingsWidget::K3bDataImageSettingsWidget( QWidget* parent, const char* name )
  : base_K3bDataImageSettings( parent, name ),
    m_fileSystemOptionsShown(true)
{
  layout()->setMargin( KDialog::marginHint() );

  m_customFsDlg = new CustomFilesystemsDialog( this );
  m_volDescDlg = new VolumeDescDialog( this );

  connect( m_buttonCustomFilesystems, SIGNAL(clicked()),
	   this, SLOT(slotCustomFilesystems()) );
  connect( m_buttonMoreVolDescFields, SIGNAL(clicked()),
	   this, SLOT(slotMoreVolDescFields()) );
  connect( m_comboSpaceHandling, SIGNAL(activated(int)),
	   this, SLOT(slotSpaceHandlingChanged(int)) );

  for( int i = 0; i < FS_MAX; ++i )
    m_comboFilesystems->insertItem( i18n( s_fsPresetNames[i] ) );

  if( !s_fsPresetsInitialized )
    initializePresets();

  QWhatsThis::add( m_comboFilesystems,
		   i18n("<p><b>File System Presets</b>"
			"<p>K3b provides the following file system Presets which allow for a quick selection "
			"of the most frequently used settings.")
		   + "<p><b>" + i18n(s_fsPresetNames[0]) + "</b><br>"
		   + i18n("The file system is optimized for usage on Linux/Unix systems. This mainly means that "
			  "it uses the Rock Ridge extensions to provide long filenames, symbolic links, and POSIX "
			  "compatible file permissions.")
		   + "<p><b>" + i18n(s_fsPresetNames[1]) + "</b><br>"
		   + i18n("In addition to the settings for Linux/Unix the file system contains a Joliet tree which "
			  "allows for long file names on Windows which does not support the Rock Ridget extensions. "
			  "Be aware that the file name length is restricted to 103 characters.")
		   + "<p><b>" + i18n(s_fsPresetNames[2]) + "</b><br>"
		   + i18n("The file system has additional UDF entries attached to it. This raises the maximal file "
			  "size to 4 GB. Be aware that the UDF support in K3b is limited.")
		   + "<p><b>" + i18n(s_fsPresetNames[3]) + "</b><br>"
		   + i18n("The file system is optimized for compatibility with old systems. That means file names "
			  "are restricted to 8.3 characters and no symbolic links or file permissions are supported.") );
}


K3bDataImageSettingsWidget::~K3bDataImageSettingsWidget()
{
}


void K3bDataImageSettingsWidget::showFileSystemOptions( bool b )
{
  m_groupFileSystem->setShown(b);
  m_groupSymlinks->setShown(b);
  m_groupWhitespace->setShown(b);

  m_fileSystemOptionsShown = b;
}


void K3bDataImageSettingsWidget::slotSpaceHandlingChanged( int i )
{
  m_editReplace->setEnabled( i == WS_REPLACE );
}


void K3bDataImageSettingsWidget::slotCustomFilesystems()
{
    // load settings in custom window
    if( m_comboFilesystems->currentItem() != FS_CUSTOM ) {
        m_customFsDlg->w->load( s_fsPresets[m_comboFilesystems->currentItem()] );
    }

    // store the current settings in case the user cancels the changes
    K3bIsoOptions o;
    m_customFsDlg->w->save( o );

    if( m_customFsDlg->exec() == QDialog::Accepted ) {
        slotFilesystemsChanged();
    }
    else {
        // reload the old settings discarding any changes
        m_customFsDlg->w->load( o );
    }
}


void K3bDataImageSettingsWidget::slotFilesystemsChanged()
{
  if( !m_fileSystemOptionsShown )
    return;

  // new custom entry
  QStringList s;
  if( m_customFsDlg->w->m_checkRockRidge->isChecked() )
    s += i18n("Rock Ridge");
  if( m_customFsDlg->w->m_checkJoliet->isChecked() )
    s += i18n("Joliet");
  if( m_customFsDlg->w->m_checkUdf->isChecked() )
    s += i18n("UDF");
  if( s.isEmpty() )
    m_comboFilesystems->changeItem( i18n("Custom (ISO9660 only)"), FS_CUSTOM );
  else
    m_comboFilesystems->changeItem( i18n("Custom (%1)").arg( s.join(", ") ), FS_CUSTOM );

  // see if any of the presets is loaded
  m_comboFilesystems->setCurrentItem( FS_CUSTOM );
  K3bIsoOptions o;
  m_customFsDlg->w->save( o );
  for( int i = 0; i < FS_CUSTOM; ++i ) {
    if( compareAdvancedOptions( o, s_fsPresets[i] ) ) {
      kdDebug() << "(K3bDataImageSettingsWidget) found preset settings: " << s_fsPresetNames[i] << endl;
      m_comboFilesystems->setCurrentItem( i );
      break;
    }
  }

  if( m_comboFilesystems->currentItem() == FS_CUSTOM ) {
    if( !m_customFsDlg->w->m_checkRockRidge->isChecked() ) {
      KMessageBox::information( this,
				i18n("<p>Be aware that it is not recommended to disable the Rock Ridge "
				     "Extensions. There is no disadvantage in enabling Rock Ridge (except "
				     "for a very small space overhead) but a lot of advantages."
				     "<p>Without Rock Ridge Extensions symbolic links are not supported "
				     "and will always be followed as if the \"Follow Symbolic Links\" option "
				     "was enabled."),
				i18n("Rock Ridge Extensions Disabled"),
				"warning_about_rock_ridge" );
    }

    if( !m_customFsDlg->w->m_checkJoliet->isChecked() )
      KMessageBox::information( this,
				i18n("<p>Be aware that without the Joliet extensions Windows "
				     "systems will not be able to display long filenames. You "
				     "will only see the ISO9660 filenames."
				     "<p>If you do not intend to use the CD/DVD on a Windows "
				     "system it is safe to disable Joliet."),
				i18n("Joliet Extensions Disabled"),
				"warning_about_joliet" );
  }
}


void K3bDataImageSettingsWidget::slotMoreVolDescFields()
{
  // update dlg to current state
  m_volDescDlg->w->m_editVolumeName->setText( m_editVolumeName->text() );

  // remember old settings
  K3bIsoOptions o;
  m_volDescDlg->w->save( o );

  // exec dlg
  if( m_volDescDlg->exec() == QDialog::Accepted ) {
    // accept new entries
    m_volDescDlg->w->save( o );
    m_editVolumeName->setText( o.volumeID() );
  }
  else {
    // restore old settings
    m_volDescDlg->w->load( o );
  }
}


void K3bDataImageSettingsWidget::load( const K3bIsoOptions& o )
{
  m_customFsDlg->w->load( o );
  m_volDescDlg->w->load( o );

  slotFilesystemsChanged();

  if( o.discardBrokenSymlinks() )
    m_comboSymlinkHandling->setCurrentItem( SYM_DISCARD_BROKEN );
  else if( o.discardSymlinks() )
    m_comboSymlinkHandling->setCurrentItem( SYM_DISCARD_ALL );
  else if( o.followSymbolicLinks() )
    m_comboSymlinkHandling->setCurrentItem( SYM_FOLLOW );
  else
    m_comboSymlinkHandling->setCurrentItem( SYM_NO_CHANGE );

  switch( o.whiteSpaceTreatment() ) {
  case K3bIsoOptions::strip:
    m_comboSpaceHandling->setCurrentItem( WS_STRIP );
    break;
  case K3bIsoOptions::extended:
    m_comboSpaceHandling->setCurrentItem( WS_EXTENDED_STRIP );
    break;
  case K3bIsoOptions::replace:
    m_comboSpaceHandling->setCurrentItem( WS_REPLACE );
    break;
  default:
    m_comboSpaceHandling->setCurrentItem( WS_NO_CHANGE );
  }
  slotSpaceHandlingChanged( m_comboSpaceHandling->currentItem() );

  m_editReplace->setText( o.whiteSpaceTreatmentReplaceString() );

  m_editVolumeName->setText( o.volumeID() );
}


void K3bDataImageSettingsWidget::save( K3bIsoOptions& o )
{
  if( m_comboFilesystems->currentItem() != FS_CUSTOM )
    m_customFsDlg->w->load( s_fsPresets[m_comboFilesystems->currentItem()] );
  m_customFsDlg->w->save( o );

  m_volDescDlg->w->save( o );

  o.setDiscardSymlinks( m_comboSymlinkHandling->currentItem() == SYM_DISCARD_ALL );
  o.setDiscardBrokenSymlinks( m_comboSymlinkHandling->currentItem() == SYM_DISCARD_BROKEN );
  o.setFollowSymbolicLinks( m_comboSymlinkHandling->currentItem() == SYM_FOLLOW );

  switch( m_comboSpaceHandling->currentItem() ) {
  case WS_STRIP:
    o.setWhiteSpaceTreatment( K3bIsoOptions::strip );
    break;
  case WS_EXTENDED_STRIP:
    o.setWhiteSpaceTreatment( K3bIsoOptions::extended );
    break;
  case WS_REPLACE:
    o.setWhiteSpaceTreatment( K3bIsoOptions::replace );
    break;
  default:
    o.setWhiteSpaceTreatment( K3bIsoOptions::noChange );
  }
  o.setWhiteSpaceTreatmentReplaceString( m_editReplace->text() );

  o.setVolumeID( m_editVolumeName->text() );
}

#include "k3bdataimagesettingswidget.moc"
