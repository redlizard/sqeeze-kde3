/*  *************************************************************************
    *   copyright: (C) 2003 Richard L�rk�ng <nouseforaname@home.se>         *
    *   copyright: (C) 2003 Gav Wood <gav@kde.org>                          *
    *************************************************************************
*/

/*  *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef SMSPROTOCOL_H
#define SMSPROTOCOL_H

#include <qmap.h>
#include <qmovie.h>
#include <qpixmap.h>
#include <qptrdict.h>
#include <qptrlist.h>
#include <qstringlist.h>

#include "kopeteprotocol.h"
#include "kopeteonlinestatus.h"
#include "kopetecontact.h"

class KAction;
class KActionMenu;

namespace Kopete { class Contact; }
namespace Kopete { class MetaContact; }
namespace Kopete { class Message; }
namespace Kopete { class ChatSession; }
class SMSContact;

class SMSProtocol : public Kopete::Protocol
{
	Q_OBJECT

public:
	SMSProtocol(QObject *parent, const char *name, const QStringList &args);
	~SMSProtocol();

	static SMSProtocol *protocol();

	/**
	 * Deserialize contact data
	 */
	virtual Kopete::Contact *deserializeContact(Kopete::MetaContact *metaContact,
		const QMap<QString, QString> &serializedData, const QMap<QString, QString> &addressBookData );

	virtual AddContactPage *createAddContactWidget(QWidget *parent , Kopete::Account *i);
	virtual KopeteEditAccountWidget *createEditAccountWidget(Kopete::Account *account, QWidget *parent);
	virtual Kopete::Account *createNewAccount(const QString &accountId);

	const Kopete::OnlineStatus SMSOnline;
	const Kopete::OnlineStatus SMSOffline;
	const Kopete::OnlineStatus SMSConnecting;

private:
	static SMSProtocol *s_protocol;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

