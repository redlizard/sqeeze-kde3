/* This file is part of the KDE project
   Copyright (C) 2002 Daniel Molkentin <molkentin@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */
#ifndef KCMKDED_H
#define KCMKDED_H

#include <qlistview.h>

#include <kcmodule.h>

class KListView;

class QStringList;
class QPushButton;

class KDEDConfig : public KCModule
{
Q_OBJECT
public:
	KDEDConfig(QWidget* parent, const char* name= 0L, const QStringList& foo = QStringList());
	~KDEDConfig() {};

	void       load();
	void       load( bool useDefaults );
	void       save();
	void       defaults();

protected slots:
	void slotReload();
	void slotStartService();
	void slotStopService();
	void slotServiceRunningToggled();
	void slotEvalItem(QListViewItem *item);
	void slotItemChecked(QCheckListItem *item);
	void getServiceStatus();

        bool autoloadEnabled(KConfig *config, const QString &filename);
        void setAutoloadEnabled(KConfig *config, const QString &filename, bool b);

private:
	KListView *_lvLoD;
	KListView *_lvStartup;
	QPushButton *_pbStart;
	QPushButton *_pbStop;
	
	QString RUNNING;
	QString NOT_RUNNING;
};

class CheckListItem : public QObject, public QCheckListItem
{
	Q_OBJECT
public:
	CheckListItem(QListView* parent, const QString &text);
	~CheckListItem() { }
signals:
	void changed(QCheckListItem*);
protected:
	virtual void stateChange(bool);
};

#endif // KCMKDED_H

