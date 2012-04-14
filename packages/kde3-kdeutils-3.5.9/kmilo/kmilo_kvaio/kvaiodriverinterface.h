/* -*- C++ -*-

   This file declares the KVaioDrierInterface class.
   It provides an event-oriented wrapper for the kernel sonypi driver. 

   $ Author: Mirko Boehm $
   $ Copyright: (C) 1996-2003, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Troll Tech, Norway. $

   $Id: kvaiodriverinterface.h 277751 2004-01-08 01:48:18Z mirko $
*/

#ifndef KVAIO_DRIVERINTERFACE_H
#define KVAIO_DRIVERINTERFACE_H

#include <qobject.h>

class QTimer;

extern "C"
{
#include <fcntl.h>

#include "./sonypi.h"
}

class QSocketNotifier;

class KVaioDriverInterface : public QObject
{
    Q_OBJECT
public:
    KVaioDriverInterface(QObject *parent=0);
    bool connectToDriver(bool listen = true);
    void disconnectFromDriver();
    /** Return the current display brightness, a value between 0 and
        255. Returns -1 if the setting cannot be retrieved. */
    int brightness();
    /** Get the battery status. */
    bool getBatteryStatus(bool& bat1Avail, int& bat1Remaining, int& bat1Max,
			  bool& bat2Avail, int& bat2Remaining, int& bat2Max,
			  bool& acConnected);
public slots:
    /** Set the display brightness. 0<= value <=255. */
    void setBrightness(int);
protected:
    int mFd;
    fd_set mRfds;
    QSocketNotifier *mNotifier;
signals:
    void vaioEvent(int);
    // void brightnessChanged(int);
protected slots:
    void socketActivated(int);
};

#endif // KVAIO_DRIVERINTERFACE_H
