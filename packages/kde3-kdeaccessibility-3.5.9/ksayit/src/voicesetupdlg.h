/***************************************************************************
                          voicesetupdlg.h  -  description
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

#ifndef VOICESETUPDLG_H
#define VOICESETUPDLG_H

// QT includes
#include <qframe.h>

// KDE includes
#include <kdialogbase.h>
#include <kconfig.h>

// forward declaration
class FXPluginHandler;
class KTTSDLib;
/**
  *@author Robert Vogl
  */
class VoiceSetupDlg : public KDialogBase  {
  Q_OBJECT

public: 
  VoiceSetupDlg(QWidget *parent=0, const char *name=0, const QString &caption=NULL,
        bool modal=true,
        KConfig *config=0,
        FXPluginHandler *fxpluginhandler=0,
        KTTSDLib *kttslib=0);
                  
  ~VoiceSetupDlg();
  
signals:
    /** Emitted when OK was cliecked;
     */
    void signalOKWasClicked();
    
    /** Signals to remove all effects from the active list.
     */
    void signalRemoveAllFX();
    
    /** Signals to relaod the last saved state.
     */
    void signalReloadFX();

private: // Methods  
    /** Initializes the configuration page to setup the properties of
     * of the TTS system.
     */
    void initVoicePage();
    
    /** Initializes the configuration page to setup the effects.
     */
    void initFXPage();
    
private slots:
    /** Called when the Page changes.
     */
    void slotPageChanged(QWidget *page);

    /** Reimplementation from base class.
     */
    void slotCancel();
    
    /** Reimplementation from base class.
     */
    void slotOk();
       
private:
    KConfig *m_config;
    FXPluginHandler *m_fxpluginhandler;
    KTTSDLib *m_kttslib;
    QFrame *m_fxPage;
    
};

#endif
