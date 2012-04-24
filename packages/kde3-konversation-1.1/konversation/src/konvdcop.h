/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

/*
  The konversation DCOP interface class
  begin:     Mar 7 2003
  copyright: (C) 2003 by Alex Zepeda
  email:     zipzippy@sonic.net
*/

#ifndef KONV_DCOP_H
#define KONV_DCOP_H

#include "konviface.h"
#include "common.h"

#include <qobject.h>
#include <dcopobject.h>


class KonvDCOP : public QObject, virtual public KonvIface
{
    Q_OBJECT

    public:
        KonvDCOP();

        QString getNickname (const QString &server);
        QString getAnyNickname ();
        QString getChannelEncoding(const QString& server, const QString& channel);

    signals:
        void dcopSay(const QString& server,const QString& target,const QString& command);
        void dcopInfo(const QString& string);
        void dcopInsertMarkerLine();
        void dcopRaw(const QString& server, const QString& command);
        void dcopMultiServerRaw(const QString& command);

        void connectTo(Konversation::ConnectionFlag flag,
                       const QString& hostName,
                       const QString& port = "",
                       const QString& password = "",
                       const QString& nick = "",
                       const QString& channel = "",
                       bool useSSL = false);

    public slots:
        void setAway(const QString &awaymessage);
        void setBack();
        void sayToAll(const QString &message);
        void actionToAll(const QString &message);
        void raw(const QString& server,const QString& command);
        void say(const QString& server,const QString& target,const QString& command);
        void info(const QString& string);
        void debug(const QString& string);
        void error(const QString& string);
        void insertMarkerLine();
        void connectToServer(const QString& adress, int port, const QString& channel, const QString& password);
        QStringList listServers();
        QStringList listConnectedServers();

        void setScreenSaverStarted();
        void setScreenSaverStopped();
};

class KonvIdentDCOP : public QObject, virtual public KonvIdentityIface
{
    Q_OBJECT

        public:
        KonvIdentDCOP();

        void setrealName(const QString &identity, const QString& name);
        QString getrealName(const QString &identity);
        void setIdent(const QString &identity, const QString& ident);
        QString getIdent(const QString &identity);

        void setNickname(const QString &identity, int index,const QString& nick);
        QString getNickname(const QString &identity, int index);

        void setBot(const QString &identity, const QString& bot);
        QString getBot(const QString &identity);
        void setPassword(const QString &identity, const QString& password);
        QString getPassword(const QString &identity);

        void setNicknameList(const QString &identity, const QStringList& newList);
        QStringList getNicknameList(const QString &identity);

        void setQuitReason(const QString &identity, const QString& reason);
        QString getQuitReason(const QString &identity);
        void setPartReason(const QString &identity, const QString& reason);
        QString getPartReason(const QString &identity);
        void setKickReason(const QString &identity, const QString& reason);
        QString getKickReason(const QString &identity);

        void setShowAwayMessage(const QString &identity, bool state);
        bool getShowAwayMessage(const QString &identity);

        void setAwayMessage(const QString &identity, const QString& message);
        QString getAwayMessage(const QString &identity);
        void setReturnMessage(const QString &identity, const QString& message);
        QString getReturnMessage(const QString &identity);

        QStringList listIdentities();
};
#endif
