/*
 * main.h
 *
 * Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __keys_main_h
#define __keys_main_h

#include <qtabwidget.h>
#include <kcmodule.h>

class CommandShortcutsModule;
class ShortcutsModule;
class ModifiersModule;

class KeyModule : public KCModule
{
	Q_OBJECT
 public:
	KeyModule( QWidget *parent, const char *name );
        ~KeyModule();

	void load();
	void load( bool useDefaults );
	void save();
	void defaults();

 protected:
	void initGUI();
	void resizeEvent( QResizeEvent* );

 private:
	QTabWidget* m_pTab;
	CommandShortcutsModule* m_pCommandShortcuts;
	ShortcutsModule* m_pShortcuts;
	ModifiersModule* m_pModifiers;
};

#endif
