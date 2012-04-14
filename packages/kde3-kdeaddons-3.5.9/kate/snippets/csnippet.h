/***************************************************************************
 *   Copyright (C) 2004 by Stephan M�res                                   *
 *   Erdling@gmx.net                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef CSNIPPET_H
#define CSNIPPET_H

#include <qobject.h>
#include <qlistview.h>
#include <kaction.h>
#include <qsignalmapper.h>

/**
@author Stephan M�res
*/

class CSnippet : public QObject {
  Q_OBJECT
public:
  CSnippet(QString sKey, QString sValue, QListViewItem *lvi, QObject *parent = 0, const char *name = 0);
  ~CSnippet();
  QString getKey() 						{ return _sKey; }
  QString getValue()						{ return _sValue; }
  QListViewItem* getListViewItem() const	{ return _lvi; }
  void setKey(const QString& sKey) 				{ _sKey = sKey; }
  void setValue(const QString& sValue) 			{ _sValue = sValue; }

protected:
  QString            _sKey;
  QString            _sValue;
  QListViewItem      *_lvi;
};

#endif
