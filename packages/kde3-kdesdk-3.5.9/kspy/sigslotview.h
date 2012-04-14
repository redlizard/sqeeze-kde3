/***************************************************************************
                          sigslotview.h  -  description
                             -------------------
    begin                : Tue May 1 2001
    copyright            : (C) 2001 by Richard Moore
    email                : rich@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SIGSLOTVIEW_H
#define SIGSLOTVIEW_H

#include <qwidget.h>
#include <klistview.h>

/**
  *@author Richard Moore
  */

class SigSlotView : public KListView  {
   Q_OBJECT
public: 
   SigSlotView(QWidget *parent=0, const char *name=0);
   ~SigSlotView();

  void buildList( QObject *o );

public slots:
  void setTarget( QObject * );
};

#endif
