/*
    kopetewalletmanager.cpp - Kopete Wallet Manager

    Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>
    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopetewalletmanager.h"

#include "kopeteuiglobal.h"

#include <kdebug.h>
#include <kstaticdeleter.h>
#include <kwallet.h>

#include <qtimer.h>
#include <qwidget.h>
#include <qapplication.h>

static WId mainWindowID()
{
	if ( QWidget *w = Kopete::UI::Global::mainWidget() )
		return w->winId();
	return 0;
}

class Kopete::WalletManager::Private
{
public:
	Private() : wallet(0), signal(0) {}
	~Private() { delete wallet; delete signal; }

	KWallet::Wallet *wallet;

	// we can't just connect every slot that wants the wallet to the
	// walletOpened signal - since we disconnect all the slots immediately
	// after emitting the signal, this would result in everyone who asked
	// for the wallet again in response to a walletOpened signal to fail
	// to receive it.
	// instead, we store a KopeteWalletSignal which we connect to, and create
	// a new one for each set of requests.
	KopeteWalletSignal *signal;
};

Kopete::WalletManager::WalletManager()
 : d( new Private )
{
}

Kopete::WalletManager::~WalletManager()
{
	closeWallet();
	delete d;
}

Kopete::WalletManager *Kopete::WalletManager::self()
{
	static KStaticDeleter<Kopete::WalletManager> s_deleter;
	static Kopete::WalletManager *s_self = 0;

	if ( !s_self )
		s_deleter.setObject( s_self, new Kopete::WalletManager() );
	return s_self;
}

void Kopete::WalletManager::openWallet( QObject *object, const char *slot )
{
	if ( !d->signal )
		d->signal = new KopeteWalletSignal;
	// allow connecting to protected slots  by calling object->connect
	connect( d->signal, SIGNAL( walletOpened( KWallet::Wallet* ) ), object, slot );
	//object->connect( d->signal, SIGNAL( walletOpened( KWallet::Wallet* ) ), slot );
	openWalletInner();
}

void Kopete::WalletManager::openWalletInner()
{
	// do we already have a wallet?
	if ( d->wallet )
	{
		// if the wallet isn't open yet, we're pending a slotWalletChangedStatus
		// anyway, so we don't set up a single shot.
		if ( d->wallet->isOpen() )
		{
			kdDebug(14010) << k_funcinfo << " wallet already open" << endl;
			QTimer::singleShot( 0, this, SLOT( slotGiveExistingWallet() ) );
		}
		else
		{
			kdDebug(14010) << k_funcinfo << " still waiting for earlier request" << endl;
		}
		return;
	}
	
	kdDebug(14010) << k_funcinfo << " about to open wallet async" << endl;

	// we have no wallet: ask for one.
	d->wallet = KWallet::Wallet::openWallet( KWallet::Wallet::NetworkWallet(),
	            mainWindowID(), KWallet::Wallet::Asynchronous );

	connect( d->wallet, SIGNAL( walletOpened(bool) ), SLOT( slotWalletChangedStatus() ) );
}

void Kopete::WalletManager::slotWalletChangedStatus()
{
	kdDebug(14010) << k_funcinfo << " isOpen: " << d->wallet->isOpen() << endl;

	if( d->wallet->isOpen() )
	{
		if ( !d->wallet->hasFolder( QString::fromLatin1( "Kopete" ) ) )
			d->wallet->createFolder( QString::fromLatin1( "Kopete" ) );

		if ( d->wallet->setFolder( QString::fromLatin1( "Kopete" ) ) )
		{
			// success!
			QObject::connect( d->wallet, SIGNAL( walletClosed() ), this, SLOT( closeWallet() ) );
		}
		else
		{
			// opened OK, but we can't use it
			delete d->wallet;
			d->wallet = 0;
		}
	}
	else
	{
		// failed to open
		delete d->wallet;
		d->wallet = 0;
	}

	emitWalletOpened( d->wallet );
}

void Kopete::WalletManager::slotGiveExistingWallet()
{
	kdDebug(14010) << k_funcinfo << " with d->wallet " << d->wallet << endl;

	if ( d->wallet )
	{
		// the wallet was already open
		if ( d->wallet->isOpen() )
			emitWalletOpened( d->wallet );
		// if the wallet was not open, but d->wallet is not 0,
		// then we're waiting for it to open, and will be told
		// when it's done: do nothing.
		else
			kdDebug(14010) << k_funcinfo << " wallet gone, waiting for another wallet" << endl;
	}
	else
	{
		// the wallet was lost between us trying to open it and
		// getting called back. try to reopen it.
		openWalletInner();
	}
}

void Kopete::WalletManager::closeWallet()
{
	if ( !d->wallet ) return;

	delete d->wallet;
	d->wallet = 0L;

	emit walletLost();
}

void Kopete::WalletManager::emitWalletOpened( KWallet::Wallet *wallet )
{
	KopeteWalletSignal *signal = d->signal;
	d->signal = 0;
	if ( signal )
		emit signal->walletOpened( wallet );
	delete signal;
}


#include "kopetewalletmanager.moc"

// vim: set noet ts=4 sts=4 sw=4:

