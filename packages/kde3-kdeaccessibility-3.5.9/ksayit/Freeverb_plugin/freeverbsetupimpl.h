/***************************************************************************
                          freeverbsetupimpl.h  -  description
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

#ifndef FREEVERBSETUPIMPL_H
#define FREEVERBSETUPIMPL_H

// QT includes
#include <qwidget.h>

// KDE includes
#include <kconfig.h>

// App specific includes
#include <freeverb_setup.h>

/**
  *@author Robert Vogl
  */

class FreeverbSetupImpl : public Freeverb_Setup  {
   Q_OBJECT
public: 
	FreeverbSetupImpl(QWidget *parent=0, const char *name=0, bool modal=true, KConfig *config=0);
	~FreeverbSetupImpl();

public slots:
  /** saves the configuration */
  void saveWasClicked();

protected slots:
  void sliderRoomChanged(int value);
  void sliderDampChanged(int value);
  void sliderWetChanged(int value);
  void sliderDryChanged(int value);
  void sliderWidthChanged(int value);
  void slotOKwasClicked();

private: // Methods
  void Init();


private:
  KConfig *m_config;


};

#endif
