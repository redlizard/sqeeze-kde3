/***************************************************************************
                          voicesetupdlg.cpp  -  description
                             -------------------
    begin                : Son Nov 2 2003
    copyright            : (C) 2003 by Robert Vogl
    email                : voglrobe@saphir
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// #include <iostream>
// using namespace std;
 
// QT includes
#include <qpixmap.h>
#include <qstringlist.h>
#include <qcombobox.h>
#include <qcolor.h>
#include <qlayout.h>
#include <qwidget.h>

// KDE includes
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>

// App specific includes
#include "voicesetupdlg.h"
#include "fxsetupimpl.h"
#include "fxpluginhandler.h"
#include "kttsdlib.h"

VoiceSetupDlg::VoiceSetupDlg(QWidget *parent, const char *name, const QString &caption,
        bool modal,
        KConfig *config,
        FXPluginHandler *fxpluginhandler,
        KTTSDLib *ttslib)
 : KDialogBase(IconList, caption, Ok|Cancel, Ok, parent, name, modal, true),
   m_config(config),
   m_fxpluginhandler(fxpluginhandler),
   m_kttslib(ttslib)
{

  setIconListAllVisible(true);
  connect (this, SIGNAL(aboutToShowPage(QWidget*)), this, SLOT(slotPageChanged(QWidget*)));

  initVoicePage();
  initFXPage();

}


VoiceSetupDlg::~VoiceSetupDlg()
{    
}


void VoiceSetupDlg::initVoicePage()
{
    QPixmap icon = KGlobal::iconLoader()->loadIcon("ksayit", KIcon::Toolbar, KIcon::SizeMedium);
    QFrame *voicePage = addPage( i18n( "Voice" ), i18n("Voice Settings"), icon );
    QVBoxLayout *voiceLayout = new QVBoxLayout( voicePage, 0, spacingHint() );
    
    const QWidget *voiceDialog = m_kttslib->getGUI( voicePage );
    voiceLayout->addWidget( const_cast<QWidget*>(voiceDialog) );
}


void VoiceSetupDlg::initFXPage()
{
    QPixmap icon = KGlobal::iconLoader()->loadIcon("ksysguard", KIcon::Toolbar, KIcon::SizeMedium);
    m_fxPage = addPage( i18n( "Audio FX" ), i18n("Effect Stack"), icon );
    QVBoxLayout *fxLayout = new QVBoxLayout( m_fxPage, 0, spacingHint() );
    
    FX_SetupImpl *fxDialog = new FX_SetupImpl(m_fxPage, "fxsetup", m_config, m_fxpluginhandler);
    fxLayout->addWidget(fxDialog);
    connect( this, SIGNAL(signalOKWasClicked()), fxDialog, SLOT(slotSaveWasClicked()) );
    connect( this, SIGNAL(signalRemoveAllFX()), fxDialog, SLOT(slotRemoveAll()) );
    connect( this, SIGNAL(signalReloadFX()), fxDialog, SLOT(slotReload()) );
    
    // Disable/enable FX-Setup depending on TTS-libs capability
    int status = m_kttslib->getStatus() & TTS::AUDIOFILE;
    if ( status ){
        m_fxPage->setEnabled(true);
    } else {
        m_fxPage->setEnabled(false);
        emit signalRemoveAllFX();
    }
}


void VoiceSetupDlg::slotPageChanged(QWidget *page)
{
    kdDebug(100200) << "+++ entering VoiceSetupDlg::slotPageChanged: " << page << endl;
    
    if ( page != m_fxPage )
        return;
    
    int status = m_kttslib->getStatus() & TTS::AUDIOFILE;
    if ( status ){
        m_fxPage->setEnabled(true);
    } else {
        QString q = i18n("The active TTS system does not make use of aRts effects.");
        KMessageBox::information( this, q, i18n("Plugin Configuration"), "KSayIt_Audiofile" );
        m_fxPage->setEnabled(false);
    }
}


void VoiceSetupDlg::slotOk()
{
    kdDebug(100200) << "+++ entering VoiceSetupDlg::slotOK()" << endl;
    
    int status = m_kttslib->getStatus();
    if ( (status & TTS::AUDIOFILE) == 0 ){
        emit signalRemoveAllFX();
    }
    
    // Let all objects save their configuration
    emit signalOKWasClicked();

	// check if configuration was saved O.K.
    if ( m_kttslib->saveWasClicked() ){
		kdDebug(100200) << "--- leaving VoiceSetupDlg::slotOk" << endl;
		QDialog::accept();
	}
}


void VoiceSetupDlg::slotCancel()
{
    kdDebug(100200) << "VoiceSetupDlg::slotCancel()" << endl;
    QDialog::reject();
}

#include "voicesetupdlg.moc"
