/* 
 *
 * $Id: k3bsystemproblemdialog.cpp 655079 2007-04-17 17:17:08Z trueg $
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


#include "k3bapplication.h"
#include "k3bsystemproblemdialog.h"
#include "k3bpassivepopup.h"
#include "k3bthemedheader.h"
#include <k3btitlelabel.h>
#include <k3bexternalbinmanager.h>
#include <k3bstdguiitems.h>
#include <k3bdevicemanager.h>
#include <k3bdevice.h>
#include <k3bversion.h>
#include <k3bglobals.h>
#include <k3bpluginmanager.h>
#include <k3bplugin.h>
#include <k3bprocess.h>
#include <k3bthememanager.h>
#include <k3bcore.h>

#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qfileinfo.h>

#include <klocale.h>
#include <kstandarddirs.h>
#include <ktextedit.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kglobal.h>

#ifdef HAVE_ICONV_H
#include <langinfo.h>
#endif

#include <fstab.h>


static QString markupString( const QString& s_ )
{
  QString s(s_);
  s.replace( '<', "&lt;" );
  s.replace( '>', "&gt;" );
  return s;
}


K3bSystemProblem::K3bSystemProblem( int t,
				    const QString& p,
				    const QString& d,
				    const QString& s,
				    bool k )
  : type(t),
    problem(p),
    details(d),
    solution(s),
    solvableByK3bSetup(k)
{
}


K3bSystemProblemDialog::K3bSystemProblemDialog( const QValueList<K3bSystemProblem>& problems,
						QWidget* parent, 
						const char* name )
  : KDialog( parent, name )
{
  setCaption( i18n("System Configuration Problems") );

  // setup the title
  // ---------------------------------------------------------------------------------------------------
  K3bThemedHeader* titleFrame = new K3bThemedHeader( this );
  titleFrame->setTitle( i18n("System Configuration Problems"), 
			i18n("1 problem", "%n problems", problems.count() ) );

  m_closeButton = new QPushButton( i18n("Close"), this );
  connect( m_closeButton, SIGNAL(clicked()), this, SLOT(close()) );
  m_checkDontShowAgain = new QCheckBox( i18n("Do not show again"), this );

#ifdef HAVE_K3BSETUP
  m_k3bsetupButton = new QPushButton( i18n("Start K3bSetup2"), this );
  connect( m_k3bsetupButton, SIGNAL(clicked()), this, SLOT(slotK3bSetup()) );
#endif

  // setup the problem view
  // ---------------------------------------------------------------------------------------------------
  KTextEdit* view = new KTextEdit( this );
  view->setReadOnly(true);
  view->setTextFormat(RichText);


  // layout everything
  QGridLayout* grid = new QGridLayout( this );
  grid->setMargin( marginHint() );
  grid->setSpacing( spacingHint() );
  grid->addMultiCellWidget( titleFrame, 0, 0, 0, 1 );
  grid->addMultiCellWidget( view, 1, 1, 0, 1 );
  grid->addWidget( m_checkDontShowAgain, 2, 0 );
  QHBoxLayout* buttonBox = new QHBoxLayout;
  buttonBox->setSpacing( spacingHint() );
  buttonBox->setMargin( 0 );
#ifdef HAVE_K3BSETUP
  buttonBox->addWidget( m_k3bsetupButton );
#endif
  buttonBox->addWidget( m_closeButton );
  grid->addLayout( buttonBox, 2, 1 );
  grid->setColStretch( 0, 1 );
  grid->setRowStretch( 1, 1 );

  QString text = "<html>";

  for( QValueList<K3bSystemProblem>::const_iterator it = problems.begin();
       it != problems.end(); ++it ) {
    const K3bSystemProblem& p = *it;

    text.append( "<p><b>" );
    if( p.type == K3bSystemProblem::CRITICAL )
      text.append( "<span style=\"color:red\">" );
    text.append( markupString( p.problem ) );
    if( p.type == K3bSystemProblem::CRITICAL )
      text.append( "</span>" );
    text.append( "</b><br>" );
    text.append( markupString( p.details ) + "<br>" );
    if( !p.solution.isEmpty() )
      text.append( "<i>" + i18n("Solution") + "</i>: " + p.solution );
#ifdef HAVE_K3BSETUP
    else if( p.solvableByK3bSetup )
      text.append( "<i>" + i18n("Solution") + "</i>: " + i18n("Use K3bSetup to solve this problem.") );
#endif
    text.append( "</p>" );
  }

  text.append( "</html>" );

  view->setText(text);
  view->setCursorPosition(0,0);
  view->ensureCursorVisible();
}


void K3bSystemProblemDialog::closeEvent( QCloseEvent* e )
{
  if( m_checkDontShowAgain->isChecked() ) {
    KConfigGroup grp( kapp->config(), "General Options" );
    grp.writeEntry( "check system config", false );
  }

  e->accept();
}


void K3bSystemProblemDialog::checkSystem( QWidget* parent, 
					  const char* name )
{
  QValueList<K3bSystemProblem> problems;

  if( k3bcore->deviceManager()->cdWriter().isEmpty() ) {
    problems.append( K3bSystemProblem( K3bSystemProblem::NON_CRITICAL,
				       i18n("No CD/DVD writer found."),
				       i18n("K3b did not find an optical writing device in your system. Thus, "
					    "you will not be able to burn CDs or DVDs. However, you can still "
					    "use other K3b features like audio track extraction or audio "
					    "transcoding or ISO9660 image creation."),
					    false ) );
  }
  else {
    // 1. cdrecord, cdrdao
    if( !k3bcore->externalBinManager()->foundBin( "cdrecord" ) ) {
      problems.append( K3bSystemProblem( K3bSystemProblem::CRITICAL,
					 i18n("Unable to find %1 executable").arg("cdrecord"),
					 i18n("K3b uses cdrecord to actually write CDs."),
					 i18n("Install the cdrtools package which contains "
					      "cdrecord."),
					 false ) );
    }
    else {
      if( k3bcore->externalBinManager()->binObject( "cdrecord" )->hasFeature( "outdated" ) ) {
	problems.append( K3bSystemProblem( K3bSystemProblem::NON_CRITICAL,
					   i18n("Used %1 version %2 is outdated").arg("cdrecord").arg(k3bcore->externalBinManager()->binObject( "cdrecord" )->version),
					   i18n("Although K3b supports all cdrtools versions since "
						"1.10 it is highly recommended to at least use "
						"version 2.0."),
					   i18n("Install a more recent version of the cdrtools."),
					   false ) );
      }
    
#ifdef Q_OS_LINUX

      //
      // Since kernel 2.6.8 older cdrecord versions are not able to use the SCSI subsystem when running suid root anymore
      // So far we ignore the suid root issue with kernel >= 2.6.8 and cdrecord < 2.01.01a02
      //
      // Kernel 2.6.16.something seems to introduce another problem which was apparently worked around in cdrecord 2.01.01a05
      //
      if( K3b::simpleKernelVersion() >= K3bVersion( 2, 6, 8 ) &&
	  k3bcore->externalBinManager()->binObject( "cdrecord" )->version < K3bVersion( 2, 1, 1, "a05" ) &&
	  !k3bcore->externalBinManager()->binObject( "cdrecord" )->hasFeature( "wodim" ) ) {
	if( k3bcore->externalBinManager()->binObject( "cdrecord" )->hasFeature( "suidroot" ) )
	  problems.append( K3bSystemProblem( K3bSystemProblem::CRITICAL,
					     i18n("%1 will be run with root privileges on kernel >= 2.6.8").arg("cdrecord <= 2.01.01a05"),
					     i18n("Since Linux kernel 2.6.8 %1 will not work when run suid "
						  "root for security reasons anymore.").arg("cdrecord <= 2.01.01a05"),
					     QString::null,
					     true ) );
      }
#ifdef CDRECORD_SUID_ROOT_CHECK
      else if( !k3bcore->externalBinManager()->binObject( "cdrecord" )->hasFeature( "suidroot" ) && getuid() != 0 ) // not root
	problems.append( K3bSystemProblem( K3bSystemProblem::CRITICAL,
					   i18n("%1 will be run without root privileges").arg("cdrecord"),
					   i18n("It is highly recommended to configure cdrecord "
						"to run with root privileges. Only then cdrecord "
						"runs with high priority which increases the overall "
						"stability of the burning process. Apart from that "
						"it allows changing the size of the used burning buffer. "
						"A lot of user problems could be solved this way. This is also "
						"true when using SuSE's resmgr."),
					   QString::null,
					   true ) );
#endif // CDRECORD_SUID_ROOT_CHECK
#endif
    }

    if( !k3bcore->externalBinManager()->foundBin( "cdrdao" ) ) {
      problems.append( K3bSystemProblem( K3bSystemProblem::CRITICAL,
					 i18n("Unable to find %1 executable").arg("cdrdao"),
					 i18n("K3b uses cdrdao to actually write CDs."),
					 i18n("Install the cdrdao package."),
					 false ) );
    }
    else {
#ifdef Q_OS_LINUX
#ifdef CDRECORD_SUID_ROOT_CHECK
      if( !k3bcore->externalBinManager()->binObject( "cdrdao" )->hasFeature( "suidroot" ) && getuid() != 0 )
	problems.append( K3bSystemProblem( K3bSystemProblem::CRITICAL,
					   i18n("%1 will be run without root privileges").arg("cdrdao"),
					   i18n("It is highly recommended to configure cdrdao "
						"to run with root privileges to increase the "
						"overall stability of the burning process."),
					   QString::null,
					   true ) );
#endif // CDRECORD_SUID_ROOT_CHECK
#endif
    }
  }


  if( !k3bcore->deviceManager()->dvdWriter().isEmpty() ) {
    if( !k3bcore->externalBinManager()->foundBin( "growisofs" ) ) {
      problems.append( K3bSystemProblem( K3bSystemProblem::CRITICAL,
					 i18n("Unable to find %1 executable").arg("growisofs"),
					 i18n("K3b uses growisofs to actually write dvds. "
					      "Without growisofs you will not be able to write dvds. "
					      "Make sure to install at least version 5.10."),
					 i18n("Install the dvd+rw-tools package."),
					 false ) );
    }
    else {
      if( k3bcore->externalBinManager()->binObject( "growisofs" )->version < K3bVersion( 5, 10 ) ) {
	problems.append( K3bSystemProblem( K3bSystemProblem::CRITICAL,
					   i18n("Used %1 version %2 is outdated").arg("growisofs").arg(k3bcore->externalBinManager()->binObject( "growisofs" )->version),
					   i18n("K3b needs at least growisofs version 5.10 to write dvds. "
						"All older versions will not work and K3b will refuse to use them."),
					   i18n("Install a more recent version of %1.").arg("growisofs"),
					   false ) );
      }
      else if( k3bcore->externalBinManager()->binObject( "growisofs" )->version < K3bVersion( 5, 12 ) ) {
	problems.append( K3bSystemProblem( K3bSystemProblem::NON_CRITICAL,
					   i18n("Used %1 version %2 is outdated").arg("growisofs").arg(k3bcore->externalBinManager()->binObject( "growisofs" )->version),
					   i18n("K3b will not be able to copy DVDs on-the-fly or write a DVD+RW in multiple "
						"sessions using a growisofs "
						"version older than 5.12."),
					   i18n("Install a more recent version of %1.").arg("growisofs"),
					   false ) );
      }
      else if( k3bcore->externalBinManager()->binObject( "growisofs" )->version < K3bVersion( 7, 0 ) ) {
	problems.append( K3bSystemProblem( K3bSystemProblem::NON_CRITICAL,
					   i18n("Used %1 version %2 is outdated").arg("growisofs").arg(k3bcore->externalBinManager()->binObject( "growisofs" )->version),
					   i18n("It is highly recommended to use growisofs 7.0 or higher. "
						"K3b won't be able to write a DVD+RW in multiple "
						"sessions using a growisofs version older than 7.0." ),
					   i18n("Install a more recent version of %1.").arg("growisofs"),
					   false ) );
      }
      // for now we ignore the suid root bit becasue of the memorylocked issue
//       else if( !k3bcore->externalBinManager()->binObject( "growisofs" )->hasFeature( "suidroot" ) ) {
// 	problems.append( K3bSystemProblem( K3bSystemProblem::CRITICAL,
// 					   i18n("%1 will be run without root privileges").arg("growisofs"),
// 					   i18n("It is highly recommended to configure growisofs "
// 						"to run with root privileges. Only then growisofs "
// 						"runs with high priority which increases the overall "
// 						"stability of the burning process."),
// 					   QString::null,
// 					   true ) );
//       }
    }

    if( !k3bcore->externalBinManager()->foundBin( "dvd+rw-format" ) ) {
      problems.append( K3bSystemProblem( K3bSystemProblem::CRITICAL,
					 i18n("Unable to find %1 executable").arg("dvd+rw-format"),
					 i18n("K3b uses dvd+rw-format to format DVD-RWs and DVD+RWs."),
					 i18n("Install the dvd+rw-tools package."),
					 false ) );
    }
  }

  if( !k3bcore->externalBinManager()->foundBin( "mkisofs" ) ) {

  }
  else if( k3bcore->externalBinManager()->binObject( "mkisofs" )->hasFeature( "outdated" ) ) {
      problems.append( K3bSystemProblem( K3bSystemProblem::CRITICAL,
					 i18n("Used %1 version %2 is outdated")
					 .arg("mkisofs")
					 .arg(k3bcore->externalBinManager()->binObject( "mkisofs" )->version),
					 i18n("K3b needs at least mkisofs version 1.14. Older versions may introduce problems "
					      "when creating data projects."),
					 i18n("Install a more recent version of %1.").arg("mkisofs"),
					 false ) );
  }

  // 2. device check
  bool atapiReader = false;
  bool atapiWriter = false;
  bool dvd_r_dl = false;
  for( QPtrListIterator<K3bDevice::Device> it( k3bcore->deviceManager()->readingDevices() );
       it.current(); ++it ) {
    if( it.current()->interfaceType() == K3bDevice::IDE )
      atapiReader = true;
    if( it.current()->type() & K3bDevice::DEVICE_DVD_R_DL )
      dvd_r_dl = true;
  }


  // check automounted devices
  QPtrList<K3bDevice::Device> automountedDevices = checkForAutomounting();
  for( QPtrListIterator<K3bDevice::Device> it( automountedDevices );
       it.current(); ++it ) {
    problems.append( K3bSystemProblem( K3bSystemProblem::NON_CRITICAL,
				       i18n("Device %1 - %2 is automounted.")
				       .arg(it.current()->vendor()).arg(it.current()->description()),
				       i18n("K3b is not able to unmount automounted devices. Thus, especially "
					    "DVD+RW rewriting might fail. There is no need to report this as "
					    "a bug or feature wish; it is not possible to solve this problem "
					    "from within K3b."),
				       i18n("Replace the automounting entries in /etc/fstab with old-fashioned "
					    "ones or use a user-space mounting solution like pmount or ivman."),
				       false ) );
  }


  if( atapiWriter ) {
    if( !K3b::plainAtapiSupport() &&
	!K3b::hackedAtapiSupport() ) {
      problems.append( K3bSystemProblem( K3bSystemProblem::CRITICAL,
					 i18n("No ATAPI writing support in kernel"),
					 i18n("Your kernel does not support writing without "
					      "SCSI emulation but there is at least one "
					      "writer in your system not configured to use "
					      "SCSI emulation."),
					 i18n("The best and recommended solution is to enable "
					      "ide-scsi (SCSI emulation) for all devices. "
					      "This way you won't have any problems."
					      "Be aware that you may still enable DMA on ide-scsi "
					      "emulated drives."),
					 false ) );
    }
    else {
      // we have atapi support in some way in the kernel

      if( k3bcore->externalBinManager()->foundBin( "cdrecord" ) ) {

	if( !( k3bcore->externalBinManager()->binObject( "cdrecord" )->hasFeature( "hacked-atapi" ) &&
	       K3b::hackedAtapiSupport() ) &&
	    !( k3bcore->externalBinManager()->binObject( "cdrecord" )->hasFeature( "plain-atapi" ) &&
	       K3b::plainAtapiSupport() ) ) {
	  problems.append( K3bSystemProblem( K3bSystemProblem::CRITICAL,
					     i18n("%1 %2 does not support ATAPI").arg("cdrecord").arg(k3bcore->externalBinManager()->binObject("cdrecord")->version),
					     i18n("The configured version of %1 does not "
						  "support writing to ATAPI devices without "
						  "SCSI emulation and there is at least one writer "
						  "in your system not configured to use "
						  "SCSI emulation.").arg("cdrecord"),
					     i18n("The best and recommended solution is to enable "
						  "ide-scsi (SCSI emulation) for all devices. "
						  "This way you won't have any problems. Or you install "
						  "(or select as the default) a more recent version of %1.").arg("cdrtools"),
					     false ) );
	}
      }

      if( k3bcore->externalBinManager()->foundBin( "cdrdao" ) ) {

	if( !k3bcore->externalBinManager()->binObject( "cdrdao" )->hasFeature( "hacked-atapi" ) &&
	    !k3bcore->externalBinManager()->binObject( "cdrdao" )->hasFeature( "plain-atapi") ) {
	  // FIXME: replace ">" with "&gt;"
	  problems.append( K3bSystemProblem( K3bSystemProblem::CRITICAL,
					     i18n("%1 %2 does not support ATAPI")
					     .arg("cdrdao").arg(k3bcore->externalBinManager()->binObject("cdrdao")->version),
					     i18n("The configured version of %1 does not "
						  "support writing to ATAPI devices without "
						  "SCSI emulation and there is at least one writer "
						  "in your system not configured to use "
						  "SCSI emulation.").arg("cdrdao"),
					     K3b::simpleKernelVersion() > K3bVersion( 2, 5, 0 )
					     ? i18n("Install cdrdao >= 1.1.8 which supports writing to "
						    "ATAPI devices directly.")
					     : i18n("The best, and recommended, solution is to use "
						    "ide-scsi (SCSI emulation) for all writer devices: "
						    "this way you will not have any problems; or, you can install "
						    "(or select as the default) a more recent version of %1.").arg("cdrdao"),
					     false ) );
	}
      }
    }
  }

  if( dvd_r_dl && k3bcore->externalBinManager()->foundBin( "growisofs" ) ) {
    if( k3bcore->externalBinManager()->binObject( "growisofs" )->version < K3bVersion( 6, 0 ) )
      problems.append( K3bSystemProblem( K3bSystemProblem::NON_CRITICAL,
					 i18n("Used %1 version %2 is outdated").arg("growisofs").arg(k3bcore->externalBinManager()->binObject( "growisofs" )->version),
					 i18n("K3b won't be able to write DVD-R Dual Layer media using a growisofs "
					      "version older than 6.0."),
					 i18n("Install a more recent version of growisofs."),
					 false ) );
  }

  for( QPtrListIterator<K3bDevice::Device> it( k3bcore->deviceManager()->allDevices() );
       it.current(); ++it ) {
    K3bDevice::Device* dev = it.current();

    if( !QFileInfo( dev->blockDeviceName() ).isWritable() )
      problems.append( K3bSystemProblem( K3bSystemProblem::CRITICAL,
					 i18n("No write access to device %1").arg(dev->blockDeviceName()),
					 i18n("K3b needs write access to all the devices to perform certain tasks. "
					      "Without it you might encounter problems with %1 - %2").arg(dev->vendor()).arg(dev->description()),
					 i18n("Make sure you have write access to %1. In case you are not using "
					      "devfs or udev K3bSetup is able to do this for you.").arg(dev->blockDeviceName()),
					 false ) );


    if( !dev->genericDevice().isEmpty() &&
	!QFileInfo( dev->genericDevice() ).isWritable() )
      problems.append( K3bSystemProblem( K3bSystemProblem::CRITICAL,
					 i18n("No write access to generic SCSI device %1").arg(dev->genericDevice()),
					 i18n("Without write access to the generic device you might "
					      "encounter problems with Audio CD ripping from %1 - %2").arg(dev->vendor()).arg(dev->description()),
					 i18n("Make sure you have write access to %1. In case you are not using "
					      "devfs or udev K3bSetup is able to do this for you.").arg(dev->genericDevice()),
					 false ) );

    if( dev->interfaceType() == K3bDevice::IDE && !dmaActivated( dev ) )
      problems.append( K3bSystemProblem( K3bSystemProblem::CRITICAL,
					 i18n("DMA disabled on device %1 - %2").arg(dev->vendor()).arg(dev->description()),
					 i18n("With most modern CD/DVD devices enabling DMA highly increases "
					      "read/write performance. If you experience very low writing speeds "
					      "this is probably the cause."),
					 i18n("Enable DMA temporarily as root with 'hdparm -d 1 %1'.").arg(dev->blockDeviceName()) ) );
  }


  //
  // Check if the user specified some user parameters and warn about it
  //
  const QMap<QString, K3bExternalProgram*>& programMap = k3bcore->externalBinManager()->programs();
  for( QMap<QString, K3bExternalProgram*>::const_iterator it = programMap.constBegin();
       it != programMap.constEnd(); ++it ) {
    const K3bExternalProgram* p = it.data();
    if( !p->userParameters().isEmpty() ) {
      problems.append( K3bSystemProblem( K3bSystemProblem::WARNING,
					 i18n("User parameters specified for external program %1").arg(p->name()),
					 i18n("Sometimes it may be nessessary to specify user parameters in addition to "
					      "the parameters generated by K3b. This is simply a warning to make sure that "
					      "these parameters are really wanted and won't be part of some bug report."),
					 i18n("To remove the user parameters for the external program %1 open the "
					      "K3b settings page 'Programs' and choose the tab 'User Parameters'.")
					 .arg(p->name()),
					 false ) );
    }
  }

  //
  // Way too many users are complaining about K3b not being able to decode mp3 files. So just warn them about
  // the legal restrictions with many distros
  //
  QPtrList<K3bPlugin> plugins = k3bcore->pluginManager()->plugins( "AudioDecoder" );
  bool haveMp3Decoder = false;
  for( QPtrListIterator<K3bPlugin> it( plugins ); *it; ++it ) {
    if( it.current()->pluginInfo().name() == "K3b MAD Decoder" ) {
      haveMp3Decoder = true;
      break;
    }
  }
  if( !haveMp3Decoder ) {
      problems.append( K3bSystemProblem( K3bSystemProblem::WARNING,
					 i18n("Mp3 Audio Decoder plugin not found."),
					 i18n("K3b could not load or find the Mp3 decoder plugin. This means that you will not "
					      "be able to create Audio CDs from Mp3 files. Many Linux distributions do not "
					      "include Mp3 support for legal reasons."),
					 i18n("To enable Mp3 support, please install the MAD Mp3 decoding library as well as the "
					      "K3b MAD Mp3 decoder plugin (the latter may already be installed but not functional "
					      "due to the missing libmad). Some distributions allow installation of Mp3 support "
					      "via an online update tool (i.e. SuSE's YOU)."),
					 false ) );
  }

#ifdef HAVE_ICONV_H
  char* codec = nl_langinfo( CODESET );
  if( strcmp( codec, "ANSI_X3.4-1968" ) == 0 ) {
    //
    // On a glibc system the system locale defaults to ANSI_X3.4-1968
    // It is very unlikely that one would set the locale to ANSI_X3.4-1968
    // intentionally
    //
    problems.append( K3bSystemProblem( K3bSystemProblem::WARNING,
				       i18n("System locale charset is ANSI_X3.4-1968"),
				       i18n("Your system's locale charset (i.e. the charset used to encode filenames) "
					    "is set to ANSI_X3.4-1968. It is highly unlikely that this has been done "
					    "intentionally. Most likely the locale is not set at all. An invalid setting "
					    "will result in problems when creating data projects."),
				       i18n("To properly set the locale charset make sure the LC_* environment variables "
					    "are set. Normally the distribution setup tools take care of this."),
				       false ) );
  }
#endif


  //
  // Never run K3b as root and especially not suid root! The latter is not possible anyway since
  // the kdelibs refuse it.
  //
  if( ::getuid() == 0 ) {
    problems.append( K3bSystemProblem( K3bSystemProblem::WARNING,
				       i18n("Running K3b as root user"),
				       i18n("It is not recommended to run K3b under the root user account. "
					    "This introduces unnecessary security risks."),
				       i18n("Run K3b from a proper user account and setup the device and "
					    "external tool permissions appropriately.")
#ifdef HAVE_K3BSETUP
				       + ' ' + i18n("The latter can be done via K3bSetup.")
#endif
				       ,
				       true ) );
  }


  kdDebug() << "(K3bCore) System problems:" << endl;
  for( QValueList<K3bSystemProblem>::const_iterator it = problems.begin();
       it != problems.end(); ++it ) {
    const K3bSystemProblem& p = *it;

    switch( p.type ) {
    case K3bSystemProblem::CRITICAL:
      kdDebug() << " CRITICAL" << endl;
      break;
    case K3bSystemProblem::NON_CRITICAL:
      kdDebug() << " NON_CRITICAL" << endl;
      break;
    case K3bSystemProblem::WARNING:
      kdDebug() << " WARNING" << endl;
      break;
    }
    kdDebug() << " PROBLEM:  " << p.problem << endl
	      << " DETAILS:  " << p.details << endl
	      << " SOLUTION: " << p.solution << endl << endl;

  }
  if( problems.isEmpty() ) {
    kdDebug() << "          - none - " << endl;
    K3bPassivePopup::showPopup( i18n("No problems found in system configuration."), i18n("System Problems") );
  }
  else {
    static K3bSystemProblemDialog* s_openDlg = 0;
    if( s_openDlg )
      s_openDlg->close();
    K3bSystemProblemDialog dlg( problems, parent, name );
    s_openDlg = &dlg;
    dlg.exec();
    s_openDlg = 0;
  }

  // remember which version of K3b checked the system the last time
  KConfigGroup cfg( k3bcore->config(), "General Options" );
  cfg.writeEntry( "Last system check version", k3bcore->version() );
}

void K3bSystemProblemDialog::slotK3bSetup()
{
  KProcess p;
  p << "kdesu" << "kcmshell k3bsetup2 --lang " + KGlobal::locale()->language();
  if( !p.start( KProcess::DontCare ) )
    KMessageBox::error( 0, i18n("Unable to start K3bSetup2.") );
}


int K3bSystemProblemDialog::dmaActivated( K3bDevice::Device* dev )
{
  QString hdparm = K3b::findExe( "hdparm" );
  if( hdparm.isEmpty() )
    return -1;

  K3bProcess p;
  K3bProcessOutputCollector out( &p );
  p << hdparm << "-d" << dev->blockDeviceName();
  if( !p.start( KProcess::Block, KProcess::AllOutput ) )
    return -1;

  // output is something like:
  //
  // /dev/hda:
  //  using_dma    =  1 (on)
  //
  // But we ignore the on/off since it might be translated
  //
  if( out.output().contains( "1 (" ) )
    return 1;
  else if( out.output().contains( "0 (" ) )
    return 0;
  else
    return -1;
}


QPtrList<K3bDevice::Device> K3bSystemProblemDialog::checkForAutomounting()
{
  QPtrList<K3bDevice::Device> l;

  ::setfsent();

  struct fstab * mountInfo = 0;
  while( (mountInfo = ::getfsent()) )
  {
    // check if the entry corresponds to a device
    QString md = K3b::resolveLink( QFile::decodeName( mountInfo->fs_spec ) );
    QString type = QFile::decodeName( mountInfo->fs_vfstype );

    if( type == "supermount" || type == "subfs" ) {
      // parse the device
      QStringList opts = QStringList::split( ",", QString::fromLocal8Bit(mountInfo->fs_mntops) );
      for( QStringList::const_iterator it = opts.begin(); it != opts.end(); ++it ) {
	if( (*it).startsWith("dev=") ) {
	  md = (*it).mid( 4 );
	  break;
	}
      }

      if( K3bDevice::Device* dev = k3bcore->deviceManager()->findDevice( md ) )
	l.append( dev );
    }
  } // while mountInfo

  ::endfsent();

  return l;
}


bool K3bSystemProblemDialog::readCheckSystemConfig()
{
  KConfigGroup cfgGrp( k3bcore->config(), "General Options" );

  K3bVersion configVersion( cfgGrp.readEntry( "Last system check version", "0.1" ) );
  if( configVersion < k3bcore->version() )
    cfgGrp.writeEntry( "check system config", true );

  return cfgGrp.readBoolEntry( "check system config", true );
}

#include "k3bsystemproblemdialog.moc"
