/*****************************************************************
ksmserver - the KDE session management server

Copyright (C) 2000 Matthias Ettrich <ettrich@kde.org>
******************************************************************/

#ifndef CLIENT_H
#define CLIENT_H

// needed to avoid clash with INT8 defined in X11/Xmd.h on solaris
#define QT_CLEAN_NAMESPACE 1
#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qsocketnotifier.h>
#include <qptrlist.h>
#include <qvaluelist.h>
#include <qcstring.h>
#include <qdict.h>
#include <qptrqueue.h>
#include <qptrdict.h>
#include <kapplication.h>
#include <qtimer.h>
#include <dcopobject.h>

#include "server2.h"

class KSMListener;
class KSMConnection;
class KSMClient
{
public:
    KSMClient( SmsConn );
    ~KSMClient();

    void registerClient( const char* previousId  = 0 );
    SmsConn connection() const { return smsConn; }

    void resetState();
    uint saveYourselfDone : 1;
    uint pendingInteraction : 1;
    uint waitForPhase2 : 1;
    uint wasPhase2 : 1;

    QPtrList<SmProp> properties;
    SmProp* property( const char* name ) const;

    QString program() const;
    QStringList restartCommand() const;
    QStringList discardCommand() const;
    int restartStyleHint() const;
    QString userId() const;
    const char* clientId() { return id ? id : ""; }

private:
    const char* id;
    SmsConn smsConn;
};

#endif
