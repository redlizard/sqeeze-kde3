/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

/*
  Copyright (C) 2008 Eike Hein <hein@kde.org>
*/

#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include "server.h"
#include "identity.h"

#include <qobject.h>


class ConnectionSettings;

class ConnectionManager : public QObject
{
    Q_OBJECT

    public:
        explicit ConnectionManager(QObject* parent = 0);
        ~ConnectionManager();

        uint connectionCount() const { return m_connectionList.count(); }

        QPtrList<Server> getServerList();

        Server* getServerByConnectionId(int connectionId);
        Server* getServerByName(const QString& name);
        Server* getAnyServer();

        void quitServers();
        void toggleGlobalAway();


    public slots:
        void connectTo(Konversation::ConnectionFlag flag,
                       const QString& target,
                       const QString& port = "",
                       const QString& password = "",
                       const QString& nick = "",
                       const QString& channel = "",
                       bool useSSL = false);

        void connectTo(Konversation::ConnectionFlag flag, int serverGroupId);
        void connectTo(Konversation::ConnectionFlag flag, ConnectionSettings& settings);


    signals:
        void connectionChangedState(Server* server, Konversation::ConnectionState state);

        void connectionChangedAwayState(bool away);

        void requestReconnect(Server* server);

        void identityOnline(int identityId);
        void identityOffline(int identityId);

        void closeServerList();


    private slots:
        void delistConnection(int connectionId);

        void handleConnectionStateChange(Server* server, Konversation::ConnectionState state);

        void handleReconnect(Server* server);


    private:
        void enlistConnection(int connectionId, Server* server);

        void decodeIrcUrl(const QString& url, ConnectionSettings& settings);

        void decodeAddress(const QString& address,
                           ConnectionSettings& settings,
                           bool checkIfServerGroup = true);

        bool reuseExistingConnection(ConnectionSettings& settings, bool interactive);
        bool validateIdentity(IdentityPtr identity, bool interactive = true);

        QMap<int, Server*> m_connectionList;
        QValueList<uint> m_activeIdentities;

        enum ConnectionDupe { SameServer, SameServerGroup };
};

#endif
