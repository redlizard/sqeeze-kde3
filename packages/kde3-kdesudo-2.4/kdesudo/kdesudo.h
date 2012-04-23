/***************************************************************************
                          kdesudo.cpp  -  description
                             -------------------
    begin                : Sam Feb 15 15:42:12 CET 2003
    copyright            : (C) 2003 by Robert Gruber <rgruber@users.sourceforge.net>
                           (C) 2007 by Martin Böhm <martin.bohm@kubuntu.org>
                                       Anthony Mercatante <tonio@kubuntu.org>
                                       Canonical Ltd (Jonathan Riddell <jriddell@ubuntu.com>)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDESUDO_H
#define KDESUDO_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kapp.h>
#include <qwidget.h>
#include <kpassdlg.h>
#include <qstring.h>
#include <qtextcodec.h>
#include <kprocess.h>
class KCookie;

/*
* KdeSudo is the base class of the project
*
* It inherits KPasswordDialog and overrrides the
* slot slotOK()
*
* @author Robert Gruber <rgruber@users.sourceforge.net>
* @version 2.4
*/

class KdeSudo : public KPasswordDialog
{
	Q_OBJECT 
	public:
	/*
	* Default construtor
	*/
	KdeSudo(QWidget* parent=0, const char *name=0, const QString& icon=QString(), const QString& generic=QString(), bool withIgnoreButton=false);

	/*
	* Default destructor
	*/
	~KdeSudo();
	enum ResultCodes { AsUser = 10 };

public slots: // Public slots
	/*
	* This slot gets exectuted if sudo creates some output
	*/
	void receivedOut(KProcess*proc, char*buffer, int buflen);
	/*
	* This slot gets exectuted when sudo exits
	*/
	void procExited(KProcess *proc);
	/*
	* This slot overrides the slot from KPasswordDialog
	* @see KPasswordDialog
	*/
	void slotOk();
	void slotUser1();
	QString validArg(QString arg);

private: // Private attributes
	KProcess * p;
	bool bError;
	bool newDcop;
	bool keepPwd;
	bool emptyPwd;
	bool useTerm;
	bool noExec;
	bool unCleaned;
	QString m_tmpname;
	QString iceauthorityFile;
	QString dcopServer;
	KCookie* m_pCookie;
	void blockSigChild();
	void unblockSigChild();
};

#endif
