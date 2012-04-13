/***************************************************************************
                          componentchooser.h  -  description
                             -------------------
    copyright            : (C) 2002 by Joseph Wenninger
    email                : jowenn@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundationi                            *
 *                                                                         *
 ***************************************************************************/

#ifndef _COMPONENTCHOOSER_H_
#define _COMPONENTCHOOSER_H_

#include "componentchooser_ui.h"
#include "componentconfig_ui.h"
#include "emailclientconfig_ui.h"
#include "terminalemulatorconfig_ui.h"
#include "browserconfig_ui.h"
#include <qdict.h>
#include <qstring.h>

#include <kservice.h>

class QListBoxItem;
class KEMailSettings;
class KConfig;

/* The CfgPlugin  class is an exception. It is LGPL. It will be parted of the plugin interface
	which I plan for KDE 3.2.
*/
class CfgPlugin
{
public:
	CfgPlugin(){};
	virtual ~CfgPlugin(){};
	virtual void load(KConfig *cfg)=0;
	virtual void save(KConfig *cfg)=0;
	virtual void defaults()=0;
};


class CfgComponent: public ComponentConfig_UI,public CfgPlugin
{
Q_OBJECT
public:
	CfgComponent(QWidget *parent);
	virtual ~CfgComponent();
	virtual void load(KConfig *cfg);
	virtual void save(KConfig *cfg);
	virtual void defaults();

protected:
	QDict<QString>  m_lookupDict,m_revLookupDict;

protected slots:
	void slotComponentChanged(const QString&);
signals:
	void changed(bool);
};


class CfgEmailClient: public EmailClientConfig_UI,public CfgPlugin
{
Q_OBJECT
public:
	CfgEmailClient(QWidget *parent);
	virtual ~CfgEmailClient();
	virtual void load(KConfig *cfg);
	virtual void save(KConfig *cfg);
	virtual void defaults();

private:
	KEMailSettings *pSettings;

protected slots:
	void selectEmailClient();
	void configChanged();
signals:
	void changed(bool);
};

class CfgTerminalEmulator: public TerminalEmulatorConfig_UI,public CfgPlugin
{
Q_OBJECT
public:
	CfgTerminalEmulator(QWidget *parent);
	virtual ~CfgTerminalEmulator();
	virtual void load(KConfig *cfg);
	virtual void save(KConfig *cfg);
	virtual void defaults();

protected slots:
	void selectTerminalApp();
	void configChanged();

signals:
	void changed(bool);
};

class CfgBrowser: public BrowserConfig_UI,public CfgPlugin
{
Q_OBJECT
public:
	CfgBrowser(QWidget *parent);
	virtual ~CfgBrowser();
	virtual void load(KConfig *cfg);
	virtual void save(KConfig *cfg);
	virtual void defaults();

protected slots:
	void selectBrowser();
	void configChanged();

signals:
	void changed(bool);
private:
	QString m_browserExec;
	KService::Ptr m_browserService;	
};


class ComponentChooser : public ComponentChooser_UI
{

Q_OBJECT

public:
	ComponentChooser(QWidget *parent=0, const char *name=0);
	virtual ~ComponentChooser();
	void load();
	void save();
	void restoreDefault();

private:
	QString latestEditedService;
	bool somethingChanged;
	QWidget *configWidget;
	QVBoxLayout *myLayout;
protected slots:
	void emitChanged(bool);
	void slotServiceSelected(QListBoxItem *);

signals:
	void changed(bool);

};


#endif
