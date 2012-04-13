/*

Conversation widget for kdm greeter

Copyright (C) 1997, 1998 Steffen Hansen <hansen@kde.org>
Copyright (C) 2000-2003 Oswald Buddenhagen <ossi@kde.org>


This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/


#ifndef KGREET_WINBIND_H
#define KGREET_WINBIND_H

#include "kgreeterplugin.h"

#include <qobject.h>
#include <qtimer.h>

class KComboBox;
class KLineEdit;
class KPasswordEdit;
class KSimpleConfig;
class QGridLayout;
class QLabel;
class KdmThemer;
class KProcIO;

class KWinbindGreeter : public QObject, public KGreeterPlugin {
	Q_OBJECT

  public:
	KWinbindGreeter( KGreeterPluginHandler *handler,
	                 KdmThemer *themer,
	                 QWidget *parent, QWidget *predecessor,
	                 const QString &fixedEntitiy,
	                 Function func, Context ctx );
	~KWinbindGreeter();
	virtual void loadUsers( const QStringList &users );
	virtual void presetEntity( const QString &entity, int field );
	virtual QString getEntity() const;
	virtual void setUser( const QString &user );
	virtual void setEnabled( bool on );
	virtual bool textMessage( const char *message, bool error );
	virtual void textPrompt( const char *prompt, bool echo, bool nonBlocking );
	virtual bool binaryPrompt( const char *prompt, bool nonBlocking );
	virtual void start();
	virtual void suspend();
	virtual void resume();
	virtual void next();
	virtual void abort();
	virtual void succeeded();
	virtual void failed();
	virtual void revive();
	virtual void clear();

  public slots:
	void slotLoginLostFocus();
	void slotChangedDomain( const QString &dom );
	void slotActivity();
        void slotStartDomainList();
        void slotReadDomainList();
        void slotEndDomainList();

  private:
	void setActive( bool enable );
	void setActive2( bool enable );
	void returnData();

	QLabel *domainLabel, *loginLabel, *passwdLabel, *passwd1Label, *passwd2Label;
	KComboBox *domainCombo;
	KLineEdit *loginEdit;
	KPasswordEdit *passwdEdit, *passwd1Edit, *passwd2Edit;
	KSimpleConfig *stsFile;
	QString fixedDomain, fixedUser, curUser;
	QStringList allUsers, mDomainListing;
	KProcIO* m_domainLister;
        QTimer mDomainListTimer;

	Function func;
	Context ctx;
	int exp, pExp, has;
	bool running, authTok;
};

#endif /* KGREET_WINBIND_H */
