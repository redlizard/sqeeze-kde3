/* -*- C++ -*-

   This file declares the KVaio module for KMilo.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 1996-2003, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Troll Tech, Norway. $

   $Id: kmilo_kvaio.h 431357 2005-07-04 03:57:52Z staikos $

   * Portions of this code are
   * (C) 2001-2002 Stelian Pop <stelian@popies.net> and
   * (C) 2001-2002 Alcove <www.alcove.com>.
   * Thanks to Stelian for the implementation of the sonypi driver.
*/

#ifndef _KMILO_KVAIO_H_
#define _KMILO_KVAIO_H_

#include <kmainwindow.h>
#include <kglobalaccel.h>
#include <dcopref.h>
#include <kapplication.h>

#include "kmilod.h"
#include "monitor.h"
#include "kmilointerface.h"

class KVaio;

class KMiloKVaio : public KMilo::Monitor {
//	Q_OBJECT

	public:
		KMiloKVaio(QObject *parent, 
			   const char *name, const QStringList&);
		virtual ~KMiloKVaio();

		virtual bool init();
		virtual DisplayType poll();
		virtual int progress() const;

		virtual void reconfigure(KConfig*);


		bool showTextMsg(const QString& msg) {
		    _interface->displayText(msg); 
		    return true;
		}
		bool showProgressMsg(const QString& msg, int progress) {
		    _interface->displayProgress(msg,progress);
		    return true;
		}

	private:
		KVaio *m_kvaio;
		Monitor::DisplayType m_displayType;
};


#endif
