/***************************************************************************
                          fxsetupimpl.h  -  description
                             -------------------
    begin                : Mo Nov 24 2003
    copyright            : (C) 2003 by voglrobe
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

#ifndef FXSETUPIMPL_H
#define FXSETUPIMPL_H

// QT includes
#include <qwidget.h>
#include <qlistbox.h>


// KDE includes
#include <kconfig.h>

// App specific includes
#include "fx_setup.h"

// forward declaration
class FXPluginHandler;

/**
  *@author voglrobe
  */
class FX_SetupImpl : public FX_Setup  {
   Q_OBJECT

public: 
  FX_SetupImpl(QWidget *parent=0, const char *name=0,
          KConfig *config=0,
          FXPluginHandler *fxpluginhandler=0);
  ~FX_SetupImpl();

public slots:
  /** Adds an effect to the list of active effects.
   */
  void slotAdd();

  /** Removes an effect from the list of active effects.
   */
  void slotRemove();

  /** Removes all effects from the list of active effects.
   */
  void slotRemoveAll();
  
  /** Recovers last saved state.
   */
  void slotReload();

  /** Called by a double click on an effect in the list of active effects.
      Opens a modal dialog to configure the efffect.
   \param item The double clicked item.
   */
  void slotConfigureEffect(QListBoxItem *item);
  
  /** Saves the configuration
   */
  void slotSaveWasClicked();

private: // Methods
  /** Loads the configuration and initializes the widget.
   \param c_avail List of available plugins.
   */
  void Init(QStringList c_avail);

private:
  KConfig *m_config;
  FXPluginHandler *m_fxpluginhandler;
  QStringList pluginlist;
  
};



#endif

