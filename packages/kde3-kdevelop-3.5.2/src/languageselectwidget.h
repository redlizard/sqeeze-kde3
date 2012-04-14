/***************************************************************************
 *   Copyright (C) 2003 by Harald Fernengel                                *
 *   harry@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _LANGUAGESELECTWIDGET_H_
#define _LANGUAGESELECTWIDGET_H_

#include <qdialog.h>
#include <qdom.h>
#include <qstringlist.h>

class QListView;

class LanguageSelectWidget : public QWidget
{
    Q_OBJECT

public:
    /* for selection of project parts */
    LanguageSelectWidget( QDomDocument &projectDom, QWidget *parent=0, const char *name=0 );
    ~LanguageSelectWidget();

public slots:
   void accept();

signals:
   void accepted();

private slots:
    void itemSelected( QListViewItem * );

private:
    void init();
    void readProjectConfig();
    void saveProjectConfig();

    QDomDocument m_projectDom;
    QListView * _pluginList;
    QLabel * _pluginDescription;
    QLabel * _currentLanguage;

};

#endif
