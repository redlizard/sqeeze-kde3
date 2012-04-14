/* This file is part of the KDE project
   Copyright (C) 2004 Jens Dagerbo <jens.dagerbo@swipnet.se>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef _CONFIGWIDGETPROXY_H
#define _CONFIGWIDGETPROXY_H

#include <qobject.h>
#include <qstring.h>
#include <qmap.h>

class KDevCore;
class KDialogBase;

/**
@file configwidgetproxy.h
Configuration widget proxy class.
*/

/**
This class can be used to implement demand-loading of config pages.
In order to avoid the potentially heavy and unneccessary creation
of a config page that might not be needed, this class can be used
to delay the config page creation until the user explicitly asks
for it.
 
A typical case looks like this:
@code
#define GLOBALDOC_OPTIONS 1
#define PROJECTDOC_OPTIONS 2

_configProxy = new ConfigWidgetProxy( core() );
_configProxy->createGlobalConfigPage( i18n("My Part"), GLOBALDOC_OPTIONS, info()->icon() );
_configProxy->createProjectConfigPage( i18n("My Part"), PROJECTDOC_OPTIONS, info()->icon() );
connect( _configProxy, SIGNAL(insertConfigWidget(const QObject*, QWidget*, unsigned int )), 
    this, SLOT(insertConfigWidget(const QObject*, QWidget*, unsigned int )) );

...
...

void MyPart::insertConfigWidget( QObject const * dlg, QWidget * page, unsigned int pagenumber )
{
    if ( pagenumber == PROJECTDOC_OPTIONS ) {
        MyPartGlobalSettings * w = new MyPartGlobalSettings( this, page );
        connect( dlg, SIGNAL(okClicked()), w, SLOT(slotAccept()) );
    } else if ( pagenumber == PROJECTDOC_OPTIONS ) {
        MyPartProjectSettings * w = new MyPartProjectSettings( this, page );
        connect( dlg, SIGNAL(okClicked()), w, SLOT(slotAccept()) );
    }
}
@endcode

Note that this replaces the functionality of typical KDevCore::configWidget() and
KDevCore::projectConfigWidget() slots.
*/
class ConfigWidgetProxy : public QObject
{
Q_OBJECT

public:
	/**Constructor.
	@param core An instance of KDevelop Core.*/
	ConfigWidgetProxy( KDevCore * core );
	virtual ~ConfigWidgetProxy();
    
	/**
	* Tells the proxy you want a page in the Global Settings. 
	* @param title The title of the config page, shown in the settings dialog.
	* @param pagenumber A per-proxy unique identifier, used when responding to insertConfigWidget() signal.
	* @param icon The name of the icon to use.
	*/
	void createGlobalConfigPage( QString const & title, unsigned int pagenumber, QString const & icon = "kdevelop" );
	
	/**
	* Tells the proxy you want a page in the Project Settings. 
	* @param title The title of the config page, shown in the settings dialog.
	* @param pagenumber A per-proxy unique identifier, used when responding to insertConfigWidget() signal.
	* @param icon The name of the icon to use.
	*/
	void createProjectConfigPage( QString const & title, unsigned int pagenumber, QString const & icon = "kdevelop" );
	
	/**
	* Removes a config page from the proxy. Next time the settings dialog opens, this page will not be available.
	* @param pagenumber The identifier set in createGlobalConfigPage() or createProjectConfigPage().
	*/
	void removeConfigPage( int pagenumber );

signals:
	/**
	* The proxy emits this signal to notify the client that a specific config page has been requested.
	* @param dlg The settings dialog. The client should connect to its okClicked() signal.
	* @param page The setting page. The client should use this as parent to the config widget.
	* @param pagenumber The identifier set in createGlobalConfigPage() or createProjectConfigPage(). Identifies the requested config page.
	*/
	void insertConfigWidget( const KDialogBase * dlg, QWidget * page, unsigned int pagenumber );

private slots:
	void slotConfigWidget( KDialogBase * );
	void slotProjectConfigWidget( KDialogBase * );
	void slotConfigWidgetDestroyed();	
	void slotAboutToShowPage( QWidget * page );

private:
	typedef QMap<unsigned int, QPair<QString,QString> > TitleMap;
	typedef QMap<QWidget*, int> PageMap;
	
	TitleMap _globalTitleMap;
	TitleMap _projectTitleMap;
	PageMap _pageMap;
};

#endif

// kate: space-indent off; indent-width 4; replace-tabs off; tab-width 4;
