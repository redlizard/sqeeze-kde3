/* 	
 *	This file contains the B2 configuration widget
 *
 *	Copyright (c) 2001
 *		Karol Szwed <gallium@kde.org>
 *		http://gallium.n3.net/
 */

#ifndef _KDE_B2CONFIG_H
#define _KDE_B2CONFIG_H

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qhgroupbox.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <kconfig.h>

class B2Config: public QObject
{
	Q_OBJECT

	public:
		B2Config( KConfig* conf, QWidget* parent );
		~B2Config();

	// These public signals/slots work similar to KCM modules
	signals:
		void changed();

	public slots:
		void load( KConfig* conf );	
		void save( KConfig* conf );
		void defaults();

	protected slots:
		void slotSelectionChanged();	// Internal use

	private:
		KConfig*   b2Config;
		QCheckBox* cbColorBorder;
		QCheckBox*  showGrabHandleCb;
		QHGroupBox* actionsGB;
		QComboBox*  menuDblClickOp;
		QWidget* gb;
};

#endif

// vim: ts=4
