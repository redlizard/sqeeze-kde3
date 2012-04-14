/***************************************************************************
 *   Copyright (C) 2003 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CHOOSESUBPROJECTDLG_H
#define CHOOSESUBPROJECTDLG_H

#include <klistview.h>

#include "choosesubprojectdlgbase.h"

class QMakeScopeItem;
class TrollProjectWidget;

class ChooseItem: public KListViewItem
{
public:
    ChooseItem(QMakeScopeItem *spitem, QListViewItem *parent, QString text);
    ChooseItem(QMakeScopeItem *spitem, QListView *parent, QString text);

    QMakeScopeItem *subproject();

private:
    QMakeScopeItem *m_spitem;
};

class ChooseSubprojectDlg : public ChooseSubprojectDlgBase
{
  Q_OBJECT

public:
  ChooseSubprojectDlg(TrollProjectWidget *widget, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
  ~ChooseSubprojectDlg();
  /*$PUBLIC_FUNCTIONS$*/

  QMakeScopeItem *selectedSubproject();

public slots:
  /*$PUBLIC_SLOTS$*/

protected:
  /*$PROTECTED_FUNCTIONS$*/
  TrollProjectWidget *m_widget;

protected slots:
  /*$PROTECTED_SLOTS$*/
  virtual void accept();
  virtual void itemSelected(QListViewItem *it);

private:
    void fillSubprojectsView(ChooseItem *item);

};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
