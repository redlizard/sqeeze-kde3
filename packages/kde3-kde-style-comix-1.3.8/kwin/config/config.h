/***************************************************************************
 *   Copyright (C) 2004 by Jens Luetkens                                   *
 *   j.luetkens@limitland.de                                               *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef KWIN_COMIX_CONFIG_H
#define KWIN_COMIX_CONFIG_H

#include <qobject.h>

class KConfig;
class ConfigDialog;

class ComixConfig : public QObject
{
    Q_OBJECT
public:
    ComixConfig(KConfig* config, QWidget* parent);
    ~ComixConfig();

signals:
    void changed();

public slots:
    void load(KConfig *config);
    void save(KConfig *config);
    void defaults();

private:
    KConfig *m_config;
    ConfigDialog *m_dialog;
};

#endif
