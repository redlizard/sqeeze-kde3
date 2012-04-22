/**************************************************************************
*   Copyright (C) 2006 by Remi Villatel <maxilys@tele2.fr>                *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
**************************************************************************/

#ifndef SERENITYCONFIG_H
#define SERENITYCONFIG_H

#include <qobject.h>


class KConfig;
class ConfigDialog;

class SerenityConfig : public QObject
{
	Q_OBJECT

	public:
		SerenityConfig(KConfig* config, QWidget* parent); ///< Constructor
		~SerenityConfig(); ///< Destructor
	
	signals:
		void changed(); ///< Widget state has changed
	
	public slots:
		void load(KConfig *config); ///< Load configuration data
		void save(KConfig *config); ///< Save configuration data
		void defaults(); ///< Set configuration defaults

	protected slots:
		void linkedChange();

	protected:
		int limitedTo(int lowest, int highest, int variable);
	
	private:
		KConfig *m_config;
		ConfigDialog *m_dialog;
};

#endif
