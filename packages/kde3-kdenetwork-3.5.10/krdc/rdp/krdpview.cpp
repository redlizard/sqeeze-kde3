/*
     krdpview.h, implementation of the KRdpView class
     Copyright (C) 2002 Arend van Beelen jr.

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or (at
     your option) any later version.

     This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
     or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
     for more details.

     You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

     For any questions, comments or whatever, you may mail me at: arend@auton.nl
*/

#include <kdialogbase.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kwallet.h>
#include <kpassdlg.h>

#include <qvbox.h>
#include <qxembed.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>

#undef Bool

#include "krdpview.h"
#include "rdphostpref.h"
#include "rdpprefs.h"

bool rdpAppDataConfigured = false;
extern KWallet::Wallet *wallet;

static KRdpView *krdpview;

RdpContainer::RdpContainer(QWidget *parent, const char *name, WFlags f) :
  QXEmbed(parent, name, f),
  m_viewOnly(false)
{
}

RdpContainer::~RdpContainer()
{
}

void RdpContainer::windowChanged(WId window)
{
	if(window == 0)
	{
		emit embeddedWindowDestroyed();
	}
	else
	{
		emit newEmbeddedWindow(window);
	}
}

bool RdpContainer::x11Event(XEvent *e)
{
	// FIXME: mouse events still get through in view-only
	if(m_viewOnly && (e->type == KeyPress || e->type == KeyRelease || e->type == ButtonPress ||
	                  e->type == ButtonRelease || e->type == MotionNotify || e->type == FocusIn ||
	                  e->type == FocusOut || e->type == EnterNotify || e->type == LeaveNotify))
	{
		return true;
	}

	return QXEmbed::x11Event(e);
}


// constructor
KRdpView::KRdpView(QWidget *parent, const char *name,
                   const QString &host, int port,
                   const QString &user, const QString &password,
                   int flags, const QString &domain,
                   const QString &shell, const QString &directory) :
  KRemoteView(parent, name, Qt::WResizeNoErase | Qt::WRepaintNoErase | Qt::WStaticContents),
  m_name(name),
  m_host(host),
  m_port(port),
  m_user(user),
  m_password(password),
  m_flags(flags),
  m_domain(domain),
  m_shell(shell),
  m_directory(directory),
  m_quitFlag(false),
  m_process(NULL)
{
	krdpview = this;
	setFixedSize(16, 16);
	if(m_port == 0)
	{
		m_port = TCP_PORT_RDP;
	}

	m_container = new RdpContainer(this);
}

// destructor
KRdpView::~KRdpView()
{
	startQuitting();
	delete m_container;
}

// returns the size of the framebuffer
QSize KRdpView::framebufferSize()
{
	return m_container->sizeHint();
}

// returns the suggested size
QSize KRdpView::sizeHint()
{
	return maximumSize();
}

// start closing the connection
void KRdpView::startQuitting()
{
	m_quitFlag = true;
	if(m_process != NULL)
	{
		m_container->sendDelete();
	}
}

// are we currently closing the connection?
bool KRdpView::isQuitting()
{
	return m_quitFlag;
}

// return the host we're connected to
QString KRdpView::host()
{
	return m_host;
}

// return the port number we're connected on
int KRdpView::port()
{
	return m_port;
}

bool KRdpView::editPreferences( HostPrefPtr host )
{
	SmartPtr<RdpHostPref> hp( host );

	int wv = hp->width();
	int hv = hp->height();
	int cd = hp->colorDepth();
	QString kl = hp->layout();
	bool kwallet = hp->useKWallet();

	// show preferences dialog
	KDialogBase *dlg = new KDialogBase( 0L, "dlg", true,
		i18n( "RDP Host Preferences for %1" ).arg( host->host() ),
		KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true );

	QVBox *vbox = dlg->makeVBoxMainWidget();
	RdpPrefs *prefs = new RdpPrefs( vbox );
	QWidget *spacer = new QWidget( vbox );
	vbox->setStretchFactor( spacer, 10 );

	prefs->setRdpWidth( wv );
	prefs->setRdpHeight( hv );
	prefs->setResolution();
	prefs->setColorDepth(cd);
	prefs->setKbLayout( keymap2int( kl ) );
	prefs->setShowPrefs( true );
	prefs->setUseKWallet(kwallet);

	if ( dlg->exec() == QDialog::Rejected )
		return false;

	wv = prefs->rdpWidth();
	hv = prefs->rdpHeight();
	kl = int2keymap( prefs->kbLayout() );
	hp->setAskOnConnect( prefs->showPrefs() );
	hp->setWidth(wv);
	hp->setHeight(hv);
	hp->setColorDepth( prefs->colorDepth() );
	hp->setLayout(kl);
	hp->setUseKWallet(prefs->useKWallet());

	delete dlg;
	return true;
}

// open a connection
bool KRdpView::start()
{
	SmartPtr<RdpHostPref> hp, rdpDefaults;
	bool useKWallet = false;

	if(!rdpAppDataConfigured)
	{
		HostPreferences *hps = HostPreferences::instance();
		hp = SmartPtr<RdpHostPref>(hps->createHostPref(m_host,
		                                              RdpHostPref::RdpType));
		if(hp->askOnConnect())
		{
			if( !editPreferences( hp ))
				return false;
			hps->sync();
		}

		useKWallet = hp->useKWallet();
	}

	m_container->show();

	m_process = new KProcess(m_container);
	*m_process << "rdesktop";
	*m_process << "-g" << (QString::number(hp->width()) + "x" + QString::number(hp->height()));
	*m_process << "-k" << hp->layout();
	if(!m_user.isEmpty())     { *m_process << "-u" << m_user; }

	if(m_password.isEmpty() && useKWallet ) {
		QString krdc_folder = "KRDC-RDP";

		// Bugfix: Check if wallet has been closed by an outside source
		if ( wallet && !wallet->isOpen() ) {
			delete wallet; wallet=0;
		}

		// Do we need to open the wallet?
		if ( !wallet ) {
			QString walletName = KWallet::Wallet::NetworkWallet();
			wallet = KWallet::Wallet::openWallet(walletName);
		}

		if (wallet && wallet->isOpen()) {
			bool walletOK = wallet->hasFolder(krdc_folder);
			if (walletOK == false) {
				walletOK = wallet->createFolder(krdc_folder);
			}

			if (walletOK == true) {
				wallet->setFolder(krdc_folder);
				if ( wallet->hasEntry(m_host) ) {
					wallet->readPassword(m_host, m_password);
				}
			}

			if ( m_password.isEmpty() ) {
				//There must not be an existing entry. Let's make one.
				QCString newPassword;
				if (KPasswordDialog::getPassword(newPassword, i18n("Please enter the password.")) == KPasswordDialog::Accepted) {
					m_password = newPassword;
					wallet->writePassword(m_host, m_password);
				}
			}
		}
	}

	if(!m_password.isEmpty()) {
		*m_process << "-p" << m_password;
	}

	*m_process << "-X" << ("0x" + QString::number(m_container->winId(), 16));
	*m_process << "-a" << QString::number(hp->colorDepth());
	*m_process << (m_host + ":" + QString::number(m_port));
	connect(m_process, SIGNAL(processExited(KProcess *)), SLOT(processDied(KProcess *)));
	connect(m_process, SIGNAL(receivedStderr(KProcess *, char *, int)), SLOT(receivedStderr(KProcess *, char *, int)));
	connect(m_container, SIGNAL(embeddedWindowDestroyed()), SLOT(connectionClosed()));
	connect(m_container, SIGNAL(newEmbeddedWindow(WId)), SLOT(connectionOpened(WId)));
  qDebug("Color depth: %d", hp->colorDepth());
	if(!m_process->start(KProcess::NotifyOnExit, KProcess::Stderr))
	{
		KMessageBox::error(0, i18n("Could not start rdesktop; make sure rdesktop is properly installed."),
		                      i18n("rdesktop Failure"));
		return false;
	}

	setStatus(REMOTE_VIEW_CONNECTING);

	return true;
}

void KRdpView::switchFullscreen(bool on)
{
	if(on == true)
	{
		m_container->grabKeyboard();
	}
}

// captures pressed keys
void KRdpView::pressKey(XEvent *e)
{
	m_container->x11Event(e);
	m_container->grabKeyboard();
}

bool KRdpView::viewOnly()
{
	return m_container->m_viewOnly;
}

void KRdpView::setViewOnly(bool s)
{
	m_container->m_viewOnly = s;
}

void KRdpView::connectionOpened(WId /*window*/)
{
	QSize size = m_container->sizeHint();

	setStatus(REMOTE_VIEW_CONNECTED);
	setFixedSize(size);
	m_container->setFixedSize(size);
	emit changeSize(size.width(), size.height());
	emit connected();
	setFocus();
}

void KRdpView::connectionClosed()
{
	emit disconnected();
	setStatus(REMOTE_VIEW_DISCONNECTED);
	m_quitFlag = true;
}

void KRdpView::processDied(KProcess */*proc*/)
{
	if(m_status == REMOTE_VIEW_CONNECTING)
	{
		setStatus(REMOTE_VIEW_DISCONNECTED);
    if(m_clientVersion.isEmpty())
    {
      KMessageBox::error(0, i18n("Connection attempt to host failed."),
			                      i18n("Connection Failure"));
    }
		emit disconnectedError();
	}
}

void KRdpView::receivedStderr(KProcess */*proc*/, char *buffer, int /*buflen*/)
{
	QString output(buffer);
	QString line;
	int i = 0;
	while(!(line = output.section('\n', i, i)).isEmpty())
	{
		if(line.startsWith("Version "))
		{
			m_clientVersion = line.section(' ', 1, 1);
			m_clientVersion = m_clientVersion.left(m_clientVersion.length() - 1);
			return;
		}
		i++;
	}
}

#include "krdpview.moc"
