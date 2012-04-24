/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

/*
  Copyright (C) 2008 Eike Hein <hein@kde.org>
*/

#ifndef AWAYMANAGER_H
#define AWAYMANAGER_H


#include <qobject.h>
#include <qdatetime.h>


class ConnectionManager;

class QTimer;


struct AwayManagerPrivate;


class AwayManager : public QObject
{
    Q_OBJECT

    public:
        explicit AwayManager(QObject* parent = 0);
        ~AwayManager();


    public slots:
        void identitiesChanged();

        void identityOnline(int identityId);
        void identityOffline(int identityId);

        void requestAllAway(const QString& reason = "");
        void requestAllUnaway();

        void setManagedIdentitiesAway();
        void setManagedIdentitiesUnaway();

        void toggleGlobalAway(bool away);
        void updateGlobalAwayAction(bool away);


    private slots:
        void checkActivity();


    private:
        void toggleTimer();
        bool Xactivity();

        void implementIdleAutoAway(bool activity);

        AwayManagerPrivate* d;

        QTime m_idleTime;
        QTimer* m_activityTimer;

        QValueList<int> m_identitiesOnAutoAway;

        ConnectionManager* m_connectionManager;
};

#endif
