/*
	Copyright (C) 2006 Michael Lentner <michaell@gmx.net>
	
	based on KDE2 Default configuration widget:
	Copyright (c) 2001
		Karol Szwed <gallium@kde.org>
		http://gallium.n3.net/
	
	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public
	License version 2 as published by the Free Software Foundation.
	
	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
	Library General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef DOMINOCONFIG_H
#define DOMINOCONFIG_H

#include <qcheckbox.h>
#include <kconfig.h>
#include <qvbox.h>
#include <kcolorbutton.h>

class DominoConfig: public QObject
{
	Q_OBJECT

	public:
		DominoConfig( KConfig* config, QWidget* parent );
		~DominoConfig();

	// These public signals/slots work similar to KCM modules
	signals:
		void changed();

	public slots:
		void load(KConfig* config);
		void save(KConfig* config);
		void defaults();

	protected slots:
		void slotSelectionChanged();	// Internal use
		void loadButtonContourColors();
	private:
		KConfig* conf;
		QVBox* vBox;
		QCheckBox* customBorderColor;
		KColorButton* borderColor;
		QCheckBox* customButtonColor;
		KColorButton* buttonColor;
		QCheckBox* customButtonIconColor;
		KColorButton* buttonIconColor;
		
		QCheckBox* showInactiveButtons;
		QCheckBox* showButtonIcons;
		QCheckBox* customGradientColors;
		KColorButton* topGradientColor;
		KColorButton* bottomGradientColor;
		QCheckBox* darkFrame;
		
		QCheckBox* useDominoStyleContourColors;
		KColorButton* buttonContourColor;
		KColorButton* buttonMouseOverContourColor;
		KColorButton* buttonPressedContourColor;
		
};

#endif
