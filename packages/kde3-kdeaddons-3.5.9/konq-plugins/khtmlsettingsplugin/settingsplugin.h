/* This file is part of the KDE project
   Copyright (C) 2001 Carsten Pfeiffer <pfeiffer@kde.org>

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

#ifndef SETTINGS_PLUGIN
#define SETTINGS_PLUGIN

#include <kparts/plugin.h>
#include <klibloader.h>

class KConfig;

class SettingsPlugin : public KParts::Plugin
{
    Q_OBJECT
public:
    SettingsPlugin( QObject* parent, const char* name,
	            const QStringList & );
    virtual ~SettingsPlugin();

private:
    bool cookiesEnabled( const QString& url );
    void updateIOSlaves();
    
private slots:
    void toggleJavascript();
    void toggleJava();
    void toggleCookies();
    void togglePlugins();
    void toggleImageLoading();
    void toggleProxy();
    void toggleCache();
    void cachePolicyChanged( int p );

    void showPopup();
    
private:
    KConfig* mConfig;
};

#endif // SETTINGS_PLUGIN
