/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) version 2.
*/

/*
  Copyright (C) 2008 Eli J. MacKenzie <argonel at gmail.com>
*/



#include "queuetuner.h"
#include "server.h"
#include "ircqueue.h"
#include "channel.h"
#include "viewcontainer.h"
#include "konversationapplication.h"

#include <qtoolbutton.h>
#include <qspinbox.h>
#include <qpopupmenu.h>
#include <qevent.h>

#include <kdebug.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kstdguiitem.h>
#include <klocale.h>


QueueTuner::QueueTuner(QWidget* parent, ViewContainer *container)
: QueueTunerBase(parent), m_server(0), m_timer(this, "qTuner"),
    m_vis(Preferences::self()->showQueueTunerItem()->value())
{
    m_closeButton->setIconSet(kapp->iconLoader()->loadIconSet("fileclose", KIcon::Toolbar, 16));
    connect(m_closeButton, SIGNAL(clicked()), SLOT(close()));
    connect(container, SIGNAL(frontServerChanging(Server*)), SLOT(setServer(Server*)));
    connect(&m_timer, SIGNAL(timeout()), SLOT(timerFired()));

    connect(m_slowRate, SIGNAL(valueChanged(int)), SLOT(slowRateChanged(int)));
    connect(m_slowType, SIGNAL(activated(int)), SLOT(slowTypeChanged(int)));
    connect(m_slowInterval, SIGNAL(valueChanged(int)), SLOT(slowIntervalChanged(int)));

    connect(m_normalRate, SIGNAL(valueChanged(int)), SLOT(normalRateChanged(int)));
    connect(m_normalType, SIGNAL(activated(int)), SLOT(normalTypeChanged(int)));
    connect(m_normalInterval, SIGNAL(valueChanged(int)), SLOT(normalIntervalChanged(int)));

    connect(m_fastRate, SIGNAL(valueChanged(int)), SLOT(fastRateChanged(int)));
    connect(m_fastType, SIGNAL(activated(int)), SLOT(fastTypeChanged(int)));
    connect(m_fastInterval, SIGNAL(valueChanged(int)), SLOT(fastIntervalChanged(int)));
}

QueueTuner::~QueueTuner()
{
}

//lps, lpm, bps, kbps
static void rateToWidget(IRCQueue::EmptyingRate& rate, QSpinBox *r, QComboBox* t, QSpinBox *i)
{
    r->setValue(rate.m_rate);
    t->setCurrentItem(rate.m_type);
    i->setValue(rate.m_interval/1000);
}

void QueueTuner::serverDestroyed(QObject* ref)
{
    if (ref == m_server)
        setServer(0);
}

void QueueTuner::setServer(Server* newServer)
{
    const char *w=0;
    bool toShow=false;
    if (!m_server && newServer)
    {
        toShow=true;
        w="showing";
    }
    else if (!newServer && m_server)
    {
        hide();
        w="hiding";
    }
    else
        w="unchanged";
    // since this is tied to the new signal, we assume we're only getting called with a change

    m_server = newServer;

    if (toShow)
        show();

    if (m_server)
    {
        connect(m_server, SIGNAL(destroyed(QObject*)), SLOT(serverDestroyed(QObject*)));

        getRates();
    }
}

void QueueTuner::getRates()
{
    if (!m_server) // you can only get the popup if there is a server, but.. paranoid
        return;

    rateToWidget(m_server->m_queues[0]->getRate(), m_slowRate, m_slowType, m_slowInterval);
    rateToWidget(m_server->m_queues[1]->getRate(), m_normalRate, m_normalType, m_normalInterval);
    rateToWidget(m_server->m_queues[2]->getRate(), m_fastRate, m_fastType, m_fastInterval);
}

void QueueTuner::timerFired()
{
    if (m_server)
    {
        IRCQueue *q=0;

        q=m_server->m_queues[0];
        m_slowAge->setNum(q->currentWait()/1000);
        m_slowBytes->setNum(q->bytesSent());
        m_slowCount->setNum(q->pendingMessages());
        m_slowLines->setNum(q->linesSent());

        q=m_server->m_queues[1];
        m_normalAge->setNum(q->currentWait()/1000);
        m_normalBytes->setNum(q->bytesSent());
        m_normalCount->setNum(q->pendingMessages());
        m_normalLines->setNum(q->linesSent());

        q=m_server->m_queues[2];
        m_fastAge->setNum(q->currentWait()/1000);
        m_fastBytes->setNum(q->bytesSent());
        m_fastCount->setNum(q->pendingMessages());
        m_fastLines->setNum(q->linesSent());

        m_srverBytes->setNum(m_server->m_encodedBytesSent);
        m_globalBytes->setNum(m_server->m_bytesSent);
        m_globalLines->setNum(m_server->m_linesSent);
        m_recvBytes->setNum(m_server->m_bytesReceived);
    }
}

void QueueTuner::open()
{
    Preferences::setShowQueueTuner(true);
    show();
}

void QueueTuner::close()
{
    Preferences::setShowQueueTuner(false);
    QueueTunerBase::close();
}

void QueueTuner::show()
{
    if (m_server && Preferences::showQueueTuner())
    {
        QueueTunerBase::show();
        m_timer.start(500);

    }
}

void QueueTuner::hide()
{
    QueueTunerBase::hide();
    m_timer.stop();
}

void QueueTuner::slowRateChanged(int v)
{
    if (!m_server) return;
    int &r=m_server->m_queues[0]->getRate().m_rate;
    r=v;
}

void QueueTuner::slowTypeChanged(int v)
{
    if (!m_server) return;
    IRCQueue::EmptyingRate::RateType &r=m_server->m_queues[0]->getRate().m_type;
    r=IRCQueue::EmptyingRate::RateType(v);
}

void QueueTuner::slowIntervalChanged(int v)
{
    if (!m_server) return;
    int &r=m_server->m_queues[0]->getRate().m_interval;
    r=v*1000;
}

void QueueTuner::normalRateChanged(int v)
{
    if (!m_server) return;
    int &r=m_server->m_queues[1]->getRate().m_rate;
    r=v;
}

void QueueTuner::normalTypeChanged(int v)
{
    if (!m_server) return;
    IRCQueue::EmptyingRate::RateType &r=m_server->m_queues[1]->getRate().m_type;
    r=IRCQueue::EmptyingRate::RateType(v);
}

void QueueTuner::normalIntervalChanged(int v)
{
    if (!m_server) return;
    int &r=m_server->m_queues[1]->getRate().m_interval;
    r=v*1000;
}

void QueueTuner::fastRateChanged(int v)
{
    if (!m_server) return;
    int &r=m_server->m_queues[2]->getRate().m_rate;
    r=v;
}

void QueueTuner::fastTypeChanged(int v)
{
    if (!m_server) return;
    IRCQueue::EmptyingRate::RateType &r=m_server->m_queues[2]->getRate().m_type;
    r=IRCQueue::EmptyingRate::RateType(v);
}

void QueueTuner::fastIntervalChanged(int v)
{
    if (!m_server) return;
    int &r=m_server->m_queues[2]->getRate().m_interval;
    r=v*1000;
}

void QueueTuner::contextMenuEvent(QContextMenuEvent* e)
{
    QPopupMenu p(this);
    p.insertItem("Reset...", 1);
    int id = p.exec(e->globalPos());
    if (id > 0)
    {

        QString question(i18n("This cannot be undone, are you sure you wish to reset to default values?"));
        int x = KMessageBox::warningContinueCancel(this, question, i18n("Reset Values"), KStdGuiItem::reset(), QString::null, KMessageBox::Dangerous);
        if ( x == KMessageBox::Continue)
        {
            Server::_resetRates();
            getRates();
        }
    }
    e->accept();
}

#include "queuetuner.moc"
