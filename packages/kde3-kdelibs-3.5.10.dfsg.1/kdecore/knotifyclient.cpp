/* This file is part of the KDE libraries
   Copyright (C) 2000 Charles Samuels <charles@altair.dhs.org>
                 2000 Malte Starostik <starosti@zedat.fu-berlin.de>
		 2000,2003 Carsten Pfeiffer <pfeiffer@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "knotifyclient.h"

#include <qdatastream.h>
#include <qptrstack.h>

#include <kapplication.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kconfig.h>
#include <dcopclient.h>
#include <kdebug.h>
#include <kstaticdeleter.h>

static const char daemonName[] = "knotify";

static bool canAvoidStartupEvent( const QString& event, const QString& appname, int present )
{
    static bool checkAvoid = true;
    if( !checkAvoid )
        return false;
    if(( appname != "kwin" && appname != "ksmserver" ) || present > 0 ) {
        checkAvoid = false;
        return false;
    }
    // startkde event is in global events file
    static KConfig* configfile = appname != "ksmserver"
        ? new KConfig( appname + ".eventsrc", true, false )
        : new KConfig( "knotify.eventsrc", true, false );
    static KConfig* eventsfile = appname != "ksmserver"
        ? new KConfig( appname + "/eventsrc", true, false, "data" )
        : new KConfig( "knotify/eventsrc", true, false, "data" );
    configfile->setGroup( event );
    eventsfile->setGroup( event );
    int ev1 = configfile->readNumEntry( "presentation", -2 );
    int ev2 = eventsfile->readNumEntry( "default_presentation", -2 );
    if(( ev1 == -2 && ev2 == -2 ) // unknown
        || ev1 > 0 // configured to have presentation
        || ( ev1 == -2 && ev2 > 0 )) { // not configured, has default presentation
        checkAvoid = false;
        return false;
    }
    return true;
}

static int sendNotifyEvent(const QString &message, const QString &text,
                            int present, int level, const QString &sound,
                            const QString &file, int winId )
{
  if (!kapp) return 0;

  DCOPClient *client=kapp->dcopClient();
  if (!client->isAttached())
  {
    client->attach();
    if (!client->isAttached())
      return 0;
  }

  QString appname = KNotifyClient::instance()->instanceName();

  if( canAvoidStartupEvent( message, appname, present ))
      return -1; // done "successfully" - there will be no event presentation

  int uniqueId = kMax( 1, kapp->random() ); // must not be 0 -- means failure!

  // knotify daemon needs toplevel window
  QWidget* widget = QWidget::find( (WId)winId );
  if( widget )
    winId = (int)widget->topLevelWidget()->winId();

  QByteArray data;
  QDataStream ds(data, IO_WriteOnly);
  ds << message << appname << text << sound << file << present << level
     << winId << uniqueId;

  if ( !KNotifyClient::startDaemon() )
      return 0;

  if ( client->send(daemonName, "Notify", "notify(QString,QString,QString,QString,QString,int,int,int,int)", data) )
  {
      return uniqueId;
  }

  return 0;
}

int KNotifyClient::event( StandardEvent type, const QString& text )
{
    return event( 0, type, text );
}

int KNotifyClient::event(const QString &message, const QString &text)
{
    return event(0, message, text);
}

int KNotifyClient::userEvent(const QString &text, int present, int level,
                              const QString &sound, const QString &file)
{
    return userEvent( 0, text, present, level, sound, file );
}


int KNotifyClient::event( int winId, StandardEvent type, const QString& text )
{
    QString message;
    switch ( type ) {
    case cannotOpenFile:
	message = QString::fromLatin1("cannotopenfile");
	break;
    case warning:
	message = QString::fromLatin1("warning");
	break;
    case fatalError:
	message = QString::fromLatin1("fatalerror");
	break;
    case catastrophe:
	message = QString::fromLatin1("catastrophe");
	break;
    case notification: // fall through
    default:
	message = QString::fromLatin1("notification");
	break;
    }

    return sendNotifyEvent( message, text, Default, Default,
			    QString::null, QString::null, winId );
}

int KNotifyClient::event(int winId, const QString &message,
                          const QString &text)
{
  return sendNotifyEvent(message, text, Default, Default, QString::null, QString::null, winId);
}

int KNotifyClient::userEvent(int winId, const QString &text, int present,
                              int level,
                              const QString &sound, const QString &file)
{
  return sendNotifyEvent(QString::null, text, present, level, sound, file, winId);
}

int KNotifyClient::getPresentation(const QString &eventname)
{
	int present;
	if (eventname.isEmpty()) return Default;

	KConfig eventsfile( KNotifyClient::instance()->instanceName()+".eventsrc", true, false);
	eventsfile.setGroup(eventname);

	present=eventsfile.readNumEntry("presentation", -1);

	return present;
}

QString KNotifyClient::getFile(const QString &eventname, int present)
{
	if (eventname.isEmpty()) return QString::null;

	KConfig eventsfile( KNotifyClient::instance()->instanceName()+".eventsrc", true, false);
	eventsfile.setGroup(eventname);

	switch (present)
	{
	case (Sound):
		return eventsfile.readPathEntry("soundfile");
	case (Logfile):
		return eventsfile.readPathEntry("logfile");
	}

	return QString::null;
}

int KNotifyClient::getDefaultPresentation(const QString &eventname)
{
	int present;
	if (eventname.isEmpty()) return Default;

	KConfig eventsfile( KNotifyClient::instance()->instanceName()+"/eventsrc", true, false, "data");
	eventsfile.setGroup(eventname);

	present=eventsfile.readNumEntry("default_presentation", -1);

	return present;
}

QString KNotifyClient::getDefaultFile(const QString &eventname, int present)
{
	if (eventname.isEmpty()) return QString::null;

	KConfig eventsfile( KNotifyClient::instance()->instanceName()+"/eventsrc", true, false, "data");
	eventsfile.setGroup(eventname);

	switch (present)
	{
	case (Sound):
		return eventsfile.readPathEntry("default_sound");
	case (Logfile):
		return eventsfile.readPathEntry("default_logfile");
	}

	return QString::null;
}

bool KNotifyClient::startDaemon()
{
  static bool firstTry = true;
  if (!kapp->dcopClient()->isApplicationRegistered(daemonName)) {
    if( firstTry ) {
      firstTry = false;
      return KApplication::startServiceByDesktopName(daemonName) == 0;
    }
    return false;
  }
  return true;
}


void KNotifyClient::beep(const QString& reason)
{
  if ( !kapp || KNotifyClient::Instance::currentInstance()->useSystemBell() ) {
    QApplication::beep();
    return;
  }

  DCOPClient *client=kapp->dcopClient();
  if (!client->isAttached())
  {
    client->attach();
    if (!client->isAttached() || !client->isApplicationRegistered(daemonName))
    {
      QApplication::beep();
      return;
    }
  }
  // The kaccess daemon handles visual and other audible beeps
  if ( client->isApplicationRegistered( "kaccess" ) )
  {
      QApplication::beep();
      return;
  }

  KNotifyClient::event(KNotifyClient::notification, reason);
}


KInstance * KNotifyClient::instance() {
    return KNotifyClient::Instance::current();
}


class KNotifyClient::InstanceStack
{
public:
	InstanceStack() { m_defaultInstance = 0; }
	virtual ~InstanceStack() { delete m_defaultInstance; }
	void push(Instance *instance) { m_instances.push(instance); }

	void pop(Instance *instance)
	{
		if (m_instances.top() == instance)
			m_instances.pop();
		else if (!m_instances.isEmpty())
		{
			kdWarning(160) << "Tried to remove an Instance that is not the current," << endl;
			kdWarning(160) << "Resetting to the main KApplication." << endl;
			m_instances.clear();
		}
		else
			kdWarning(160) << "Tried to remove an Instance, but the stack was empty." << endl;
    }

	Instance *currentInstance()
	{
		if (m_instances.isEmpty())
		{
			m_defaultInstance = new Instance(kapp);
		}
		return m_instances.top();
	}

private:
	QPtrStack<Instance> m_instances;
	Instance *m_defaultInstance;
};

KNotifyClient::InstanceStack * KNotifyClient::Instance::s_instances = 0L;
static KStaticDeleter<KNotifyClient::InstanceStack > instancesDeleter;

struct KNotifyClient::InstancePrivate
{
    KInstance *instance;
    bool useSystemBell;
};

KNotifyClient::Instance::Instance(KInstance *instance)
{
    d = new InstancePrivate;
    d->instance = instance;
    instances()->push(this);

    KConfig *config = instance->config();
    KConfigGroupSaver cs( config, "General" );
    d->useSystemBell = config->readBoolEntry( "UseSystemBell", false );
}

KNotifyClient::Instance::~Instance()
{
	if (s_instances)
		s_instances->pop(this);
	delete d;
}

KNotifyClient::InstanceStack *KNotifyClient::Instance::instances()
{
	if (!s_instances)
		instancesDeleter.setObject(s_instances, new InstanceStack);
	return s_instances;
}

bool KNotifyClient::Instance::useSystemBell() const
{
    return d->useSystemBell;
}


// static methods

// We always return a valid KNotifyClient::Instance here. If no special one
// is available, we have a default-instance with kapp as KInstance.
// We make sure to always have that default-instance in the stack, because
// the stack might have gotten cleared in the destructor.
// We can't use QStack::setAutoDelete( true ), because no instance besides
// our default instance is owned by us.
KNotifyClient::Instance * KNotifyClient::Instance::currentInstance()
{
	return instances()->currentInstance();
}

KInstance *KNotifyClient::Instance::current()
{
    return currentInstance()->d->instance;
}
