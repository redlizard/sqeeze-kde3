/* Plastik KWin window decoration
  Copyright (C) 2005 Gerd Fleischer <gerdfleischer@web.de>
  Copyright (C) 2003 Sandro Giessl <ceebx@users.sourceforge.net>

  based on the window decoration "Web":
  Copyright (C) 2001 Rik Hemsley (rikkus) <rik@kde.org>

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
  the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
  Boston, MA  02110-1301  USA.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <qobject.h>

class QButtonGroup;
class QGroupBox;
class KConfig;
class ConfigDialog;

class SUSE2Config : public QObject
{
    Q_OBJECT

public:
    SUSE2Config(KConfig *config, QWidget *parent);
    ~SUSE2Config();

signals:
    void changed();

public slots:
    void load(KConfig *config);
    void save(KConfig *config) const;
    void defaults();

private:
    QWidget *m_parent;
    KConfig *m_config;
    ConfigDialog *m_dialog;
    QString titlebarLogoURL;

private slots:
    void toggleIconSettings(bool) const;
    void selectImage();
};

#endif

