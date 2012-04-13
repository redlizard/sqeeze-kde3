//===========================================================================
//
// This file is part of the KDE project
//
// Copyright (c) 1999 Martin R. Jones <mjones@kde.org>
//

#ifndef __XAUTOLOCK_H__
#define __XAUTOLOCK_H__

#include <qwidget.h>

#include <X11/Xlib.h>

//===========================================================================
//
// Detect user inactivity.
// Named XAutoLock after the program that it is based on.
//
class XAutoLock : public QWidget
{
    Q_OBJECT
public:
    XAutoLock();
    ~XAutoLock();

    //-----------------------------------------------------------------------
    //
    // The time in seconds of continuous inactivity.
    //
    void setTimeout(int t);
    
    void setDPMS(bool s);
    
    //-----------------------------------------------------------------------
    //
    // Start watching Activity
    //
    void start();

    //-----------------------------------------------------------------------
    //
    // Stop watching Activity
    //
    void stop();

    //-----------------------------------------------------------------------
    //
    // Should be called only from a slot connected to the timeout() signal. Will
    // result in the timeout() signal being emitted again with a delay (i.e. postponed).
    //
    void postpone();

    // internal
    void resetTrigger();
    // internal
    void setTrigger( time_t );
    // internal
    bool ignoreWindow( WId );
    
signals:
    void timeout();

protected:
    virtual void timerEvent(QTimerEvent *ev);
    virtual bool x11Event( XEvent* );

protected:
    int     mTimerId;
    int     mTimeout;
    time_t  mTrigger;
    bool    mActive;
    time_t  mLastTimeout;
    bool    mDPMS;
};

#endif
