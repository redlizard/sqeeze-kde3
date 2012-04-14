/***************************************************************************
                          kstartuplogo.h  -  description
                             -------------------
    begin                : Fri Dec 14 2001
    copyright            : (C) 2001 by Arnold Kraschinski
    email                : arnold.k67@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KSTARTUPLOGO_H
#define KSTARTUPLOGO_H

#include <qwidget.h>

/**This class creates the splash screen at the beginning of the program.
  *@author Arnold Kraschinski
  */

class KStartupLogo : public QWidget  {
   Q_OBJECT
public:
	KStartupLogo(QWidget *parent=0, const char *name=0);
	~KStartupLogo();
protected:
  virtual bool event(QEvent*);
};


#endif
