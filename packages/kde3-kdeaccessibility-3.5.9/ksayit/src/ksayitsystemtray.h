/***************************************************************************
                          ksayitsystemtray.h  -  description
                             -------------------
    begin                : Die Sep 2 2003
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

#ifndef KSAYITSYSTEMTRAY_H
#define KSAYITSYSTEMTRAY_H

// QT includes
#include <qwidget.h>
#include <qpixmap.h>

// KDE includes
#include <kapplication.h>
#include <ksystemtray.h>
#include <kpopupmenu.h>
#include <kaction.h>
#include <kmenubar.h>
#include <khelpmenu.h>
#include <kiconloader.h>

// forward declarations
class State;

/**
  *@author Robert Vogl
  */
class KSayItSystemTray : public KSystemTray  {
   Q_OBJECT

  friend class State;

public: 
    KSayItSystemTray(QWidget *parent=0, const char *name=0);
    ~KSayItSystemTray();

signals:
  /** Signal emitted if configuration has been changed
   */
  void signalCallPreferences();

  /** Signal emitted if the Tray icon was clicked.
   */
  void signalSayClipboard();
  
  /** Signal emitted by the control actions
   */
  void signalSayActivated();

  void signalShutUpActivated();
    
  void signalPauseActivated();
  
  void signalNextActivated();
  
  void signalPrevActivated();
  
public slots:
  /** opens the Preferences-Dialog
   */
  void slotPreferences();

  /** activated if "say" was clicked
   */
  void slotSayActivated();

  /** activated if "shut up" was clicked
   */
  void slotStopActivated();

  /** activated if "pause" was clicked
   */
  void slotPauseActivated();
  
  /** activated if "next sentence" was clicked
   */
  void slotNextSentenceActivated();
  
  /** activated if "previous sentence" was clicked
   */
  void slotPrevSentenceActivated();

public:  
  /** Changes the state of the statemachine
   */
  void changeState(State *state);

  /** Enables/Diasabled the actions in the menu
   */
  void setActions(bool sayEnabled, bool pauseEnabled, bool shutupEnabled,
      bool nextEnabled, bool prevEnabled);
  
protected: // Methods
  /** Reimplementation from base class
   */
  void mousePressEvent(QMouseEvent *);

  /** Reimplementation from base class
   */  
  void mouseReleaseEvent(QMouseEvent *);

  /** KSystemTray default mouse handling
   */
  void normalMousePressEvent(QMouseEvent *e);

  /** KSystemTray default mouse handling
   */
  void normalMouseReleaseEvent(QMouseEvent *e);

  /** Called if the Tray icon was clicked. 
   */
  void sayClipboard();
    
private: // Methods
  /** initializes the KActions of the application */
  void initActions();

public:
  KAction *say;
  KAction *shutup;
  KAction *pause;
  KAction *next_sentence;
  KAction *prev_sentence;

private:
  KPopupMenu *menu;
  KHelpMenu *help;
  KAction *settings;
  KAction *help_about;
  KAction *help_kde;
  State *_state;
  
};




class State : public KSystemTray {
  Q_OBJECT
public:
  State();
  ~State();

  virtual void mousePressEvent(KSayItSystemTray *caller, QMouseEvent *e);
  virtual void mouseReleaseEvent(KSayItSystemTray *caller, QMouseEvent *e);
  virtual void setContext(KSayItSystemTray *caller);

protected:
  void changeState(KSayItSystemTray *caller, State *state);
  void say(KSayItSystemTray *caller);  
  void mousePressEventCall(KSayItSystemTray *caller, QMouseEvent *e);
  void mouseReleaseEventCall(KSayItSystemTray *caller, QMouseEvent *e);
};




class StateWAIT : public State {

public:
  static StateWAIT* Instance();
  ~StateWAIT();

protected:
  StateWAIT();
  void mousePressEvent(KSayItSystemTray *caller, QMouseEvent *e);
  void mouseReleaseEvent(KSayItSystemTray *caller, QMouseEvent *e);
  void setContext(KSayItSystemTray *caller);

private:
  static StateWAIT *_instance;
  QPixmap m_traypixmap;
};




class StateSAY : public State {

public:
  static StateSAY* Instance();
  ~StateSAY();

protected:
  StateSAY();
  void mousePressEvent(KSayItSystemTray *caller, QMouseEvent *e);
  void mouseReleaseEvent(KSayItSystemTray *caller, QMouseEvent *e);
  void setContext(KSayItSystemTray *caller);

private:
  static StateSAY *_instance;
  QPixmap m_traypixmap;
};




class StateCLIPEMPTY : public State {

public:
  static StateCLIPEMPTY* Instance();
  ~StateCLIPEMPTY();

protected:
  StateCLIPEMPTY();
  void mousePressEvent(KSayItSystemTray *caller, QMouseEvent *e);
  void mouseReleaseEvent(KSayItSystemTray *caller, QMouseEvent *e);
  void setContext(KSayItSystemTray *caller);

private:
  static StateCLIPEMPTY *_instance;
  QPixmap m_traypixmap;
};


#endif
