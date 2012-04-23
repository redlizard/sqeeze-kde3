/***************************************************************************
 *   Copyright (C) 2005-2007 by Stephen Leaf <smileaf@gmail.com>           *
 *   Copyright (C) 2006 by Oswald Buddenhagen <ossi@kde.org>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#ifndef _KDMTHEME_H_
#define _KDMTHEME_H_

#include <kconfig.h>
#include <kcmodule.h>

#include <qwidget.h>

class ThemeData;

class QLabel;
class QPushButton;
class QListView;
class QCheckBox;

class KDMThemeWidget : public KCModule {
	Q_OBJECT

  public:
	KDMThemeWidget( QWidget *parent = 0, const char *name=0, const QStringList& = QStringList() );

	virtual void load();
	virtual void save();
	virtual void defaults();
	void setReadOnly( bool );

  private:
	void selectTheme( const QString & );
	void insertTheme( const QString & );
	void updateInfoView( ThemeData * );

	QListView *themeWidget;
	QLabel *preview;
	QLabel *info;
	QPushButton *bInstallTheme;
	QPushButton *bRemoveTheme;
	QCheckBox *cUseTheme;

	ThemeData *defaultTheme;
	QString themeDir;
	KConfig *config;

  protected slots:
	void themeSelected();
	void removeSelectedThemes();
	void installNewTheme();
	void toggleUseTheme(bool);

};

#endif
