///////////////////////////////////////////////////////////////////////
// -------------------
// DeKorator window decoration for KDE
// -------------------
// Copyright (C) 2005  moty rahamim <moty.rahamim@gmail.com>
//
// Based on Example-0.8, some ideas and code have been taken from plastik and KCMicons
//
// Example window decoration for KDE
// Copyright (c) 2003, 2004 David Johnson <david@usermode.org>
//
// Plastik KWin window decoration
// Copyright (C) 2003-2005 Sandro Giessl <sandro@giessl.com>
//
// KCMicons for KDE
// Copyright (c) 2000 Antonio Larrosa <larrosa@kde.org>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; see the file COPYING.  If not, write to
// the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
// Boston, MA 02110-1301, USA.
///////////////////////////////////////////////////////////////////////

#ifndef EXAMPLECONFIG_H
#define EXAMPLECONFIG_H

#include <qobject.h>


class KConfig;
class ConfigDialog;
class IconThemesConfig;

class DeKoratorConfig : public QObject
{
    Q_OBJECT
public:
    DeKoratorConfig( KConfig* config, QWidget* parent );
    ~DeKoratorConfig();

signals:
    void changed();

public slots:
    void load( KConfig* );
    void save( KConfig* );
    void defaults();


protected slots:
    void themeSelected( QListViewItem *item );
    void selectionChanged( int );
    void installNewTheme();
    void removeSelectedTheme();
    void setTheme();
private:
    KConfig *config_;
    IconThemesConfig *themes_;
    ConfigDialog *dialog_;
};

#endif // EXAMPLECONFIG_H
