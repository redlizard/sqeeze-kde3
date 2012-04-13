//===========================================================================
//
// This file is part of the KDE project
//
// Copyright (c) 1999 Martin R. Jones <mjones@kde.org>
// Copyright (c) 2003 Chris Howells <howells@kde.org>
// Copyright (c) 2003 Oswald Buddenhagen <ossi@kde.org>

#include <config.h>

#include "lockprocess.h"
#include "lockdlg.h"

#include <kcheckpass.h>
#include <dmctl.h>

#include <kapplication.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kseparator.h>
#include <kstandarddirs.h>
#include <kglobalsettings.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kdesu/defaults.h>
#include <kpassdlg.h>
#include <kdebug.h>
#include <kuser.h>
#include <dcopref.h>
#include <kmessagebox.h>

#include <qlayout.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qsimplerichtext.h>
#include <qlabel.h>
#include <qstringlist.h>
#include <qfontmetrics.h>
#include <qstyle.h>
#include <qapplication.h>
#include <qlistview.h>
#include <qheader.h>
#include <qcheckbox.h>

#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <fixx11h.h>

#ifndef AF_LOCAL
# define AF_LOCAL	AF_UNIX
#endif

#define PASSDLG_HIDE_TIMEOUT 10000

//===========================================================================
//
// Simple dialog for entering a password.
//
PasswordDlg::PasswordDlg(LockProcess *parent, GreeterPluginHandle *plugin)
    : QDialog(parent, "password dialog", true, WX11BypassWM),
      mPlugin( plugin ),
      mCapsLocked(-1),
      mUnlockingFailed(false)
{
    frame = new QFrame( this );
    frame->setFrameStyle( QFrame::Panel | QFrame::Raised );
    frame->setLineWidth( 2 );

    QLabel *pixLabel = new QLabel( frame, "pixlabel" );
    pixLabel->setPixmap(DesktopIcon("lock"));

    KUser user;
    QLabel *greetLabel = new QLabel( user.fullName().isEmpty() ?
            i18n("<nobr><b>The session is locked</b><br>") :
            i18n("<nobr><b>The session was locked by %1</b><br>").arg( user.fullName() ), frame );

    mStatusLabel = new QLabel( "<b> </b>", frame );
    mStatusLabel->setAlignment( QLabel::AlignCenter );

    mLayoutButton = new QPushButton( frame );
    mLayoutButton->setFlat( true );

    KSeparator *sep = new KSeparator( KSeparator::HLine, frame );

    mNewSessButton = new KPushButton( KGuiItem(i18n("Sw&itch User..."), "fork"), frame );
    ok = new KPushButton( i18n("Unl&ock"), frame );
    cancel = new KPushButton( KStdGuiItem::cancel(), frame );

    greet = plugin->info->create( this, 0, this, mLayoutButton, QString::null,
              KGreeterPlugin::Authenticate, KGreeterPlugin::ExUnlock );


    QVBoxLayout *unlockDialogLayout = new QVBoxLayout( this );
    unlockDialogLayout->addWidget( frame );

    QHBoxLayout *layStatus = new QHBoxLayout( 0, 0, KDialog::spacingHint());
    layStatus->addWidget( mStatusLabel );
    layStatus->addWidget( mLayoutButton );

    QHBoxLayout *layButtons = new QHBoxLayout( 0, 0, KDialog::spacingHint());
    layButtons->addWidget( mNewSessButton );
    layButtons->addStretch();
    layButtons->addWidget( ok );
    layButtons->addWidget( cancel );

    frameLayout = new QGridLayout( frame, 1, 1, KDialog::marginHint(), KDialog::spacingHint() );
    frameLayout->addMultiCellWidget( pixLabel, 0, 2, 0, 0, AlignTop );
    frameLayout->addWidget( greetLabel, 0, 1 );
    frameLayout->addItem( greet->getLayoutItem(), 1, 1 );
    frameLayout->addLayout( layStatus, 2, 1 );
    frameLayout->addMultiCellWidget( sep, 3, 3, 0, 1 );
    frameLayout->addMultiCellLayout( layButtons, 4, 4, 0, 1 );

    setTabOrder( ok, cancel );
    setTabOrder( cancel, mNewSessButton );
    setTabOrder( mNewSessButton, mLayoutButton );

    connect(mLayoutButton, SIGNAL(clicked()), this, SLOT(layoutClicked()));
    connect(cancel, SIGNAL(clicked()), SLOT(reject()));
    connect(ok, SIGNAL(clicked()), SLOT(slotOK()));
    connect(mNewSessButton, SIGNAL(clicked()), SLOT(slotSwitchUser()));

    if (!DM().isSwitchable() || !kapp->authorize("switch_user"))
        mNewSessButton->hide();

    installEventFilter(this);

    mFailedTimerId = 0;
    mTimeoutTimerId = startTimer(PASSDLG_HIDE_TIMEOUT);
    connect(qApp, SIGNAL(activity()), SLOT(slotActivity()) );

    greet->start();

    DCOPRef kxkb("kxkb", "kxkb");
    if( !kxkb.isNull() ) {
        layoutsList = kxkb.call("getLayoutsList");
        QString currentLayout = kxkb.call("getCurrentLayout");
        if( !currentLayout.isEmpty() && layoutsList.count() > 1 ) {
            currLayout = layoutsList.find(currentLayout);
            if (currLayout == layoutsList.end())
                setLayoutText("err");
            else
                setLayoutText(*currLayout);
        } else
            mLayoutButton->hide();
    } else {
        mLayoutButton->hide(); // no kxkb running
    }
    capsLocked();
}

PasswordDlg::~PasswordDlg()
{
    hide();
    frameLayout->removeItem( greet->getLayoutItem() );
    delete greet;
}

void PasswordDlg::layoutClicked()
{

    if( ++currLayout == layoutsList.end() )
        currLayout = layoutsList.begin();

    DCOPRef kxkb("kxkb", "kxkb");
    setLayoutText( kxkb.call("setLayout", *currLayout) ? *currLayout : "err" );

}

void PasswordDlg::setLayoutText( const QString &txt )
{
    mLayoutButton->setText( txt );
    QSize sz = mLayoutButton->fontMetrics().size( 0, txt );
    int mrg = mLayoutButton->style().pixelMetric( QStyle::PM_ButtonMargin ) * 2;
    mLayoutButton->setFixedSize( sz.width() + mrg, sz.height() + mrg );
}

void PasswordDlg::updateLabel()
{
    if (mUnlockingFailed)
    {
        mStatusLabel->setPaletteForegroundColor(Qt::black);
        mStatusLabel->setText(i18n("<b>Unlocking failed</b>"));
    }
    else
    if (mCapsLocked)
    {
        mStatusLabel->setPaletteForegroundColor(Qt::red);
        mStatusLabel->setText(i18n("<b>Warning: Caps Lock on</b>"));
    }
    else
    {
        mStatusLabel->setText("<b> </b>");
    }
}

//---------------------------------------------------------------------------
//
// Handle timer events.
//
void PasswordDlg::timerEvent(QTimerEvent *ev)
{
    if (ev->timerId() == mTimeoutTimerId)
    {
        reject();
    }
    else if (ev->timerId() == mFailedTimerId)
    {
        killTimer(mFailedTimerId);
        mFailedTimerId = 0;
        // Show the normal password prompt.
        mUnlockingFailed = false;
        updateLabel();
        ok->setEnabled(true);
        cancel->setEnabled(true);
        mNewSessButton->setEnabled( true );
        greet->revive();
        greet->start();
    }
}

bool PasswordDlg::eventFilter(QObject *, QEvent *ev)
{
    if (ev->type() == QEvent::KeyPress || ev->type() == QEvent::KeyRelease)
        capsLocked();
    return false;
}

void PasswordDlg::slotActivity()
{
    if (mTimeoutTimerId) {
        killTimer(mTimeoutTimerId);
        mTimeoutTimerId = startTimer(PASSDLG_HIDE_TIMEOUT);
    }
}

////// kckeckpass interface code

int PasswordDlg::Reader (void *buf, int count)
{
    int ret, rlen;

    for (rlen = 0; rlen < count; ) {
      dord:
        ret = ::read (sFd, (void *)((char *)buf + rlen), count - rlen);
        if (ret < 0) {
            if (errno == EINTR)
                goto dord;
            if (errno == EAGAIN)
                break;
            return -1;
        }
        if (!ret)
            break;
        rlen += ret;
    }
    return rlen;
}

bool PasswordDlg::GRead (void *buf, int count)
{
    return Reader (buf, count) == count;
}

bool PasswordDlg::GWrite (const void *buf, int count)
{
    return ::write (sFd, buf, count) == count;
}

bool PasswordDlg::GSendInt (int val)
{
    return GWrite (&val, sizeof(val));
}

bool PasswordDlg::GSendStr (const char *buf)
{
    int len = buf ? ::strlen (buf) + 1 : 0;
    return GWrite (&len, sizeof(len)) && GWrite (buf, len);
}

bool PasswordDlg::GSendArr (int len, const char *buf)
{
    return GWrite (&len, sizeof(len)) && GWrite (buf, len);
}

bool PasswordDlg::GRecvInt (int *val)
{
    return GRead (val, sizeof(*val));
}

bool PasswordDlg::GRecvArr (char **ret)
{
    int len;
    char *buf;

    if (!GRecvInt(&len))
        return false;
    if (!len) {
        *ret = 0;
        return true;
    }
    if (!(buf = (char *)::malloc (len)))
        return false;
    *ret = buf;
    return GRead (buf, len);
}

void PasswordDlg::reapVerify()
{
    ::close( sFd );
    int status;
    ::waitpid( sPid, &status, 0 );
    if (WIFEXITED(status))
        switch (WEXITSTATUS(status)) {
        case AuthOk:
            greet->succeeded();
            accept();
            return;
        case AuthBad:
            greet->failed();
            mUnlockingFailed = true;
            updateLabel();
            mFailedTimerId = startTimer(1500);
            ok->setEnabled(false);
            cancel->setEnabled(false);
            mNewSessButton->setEnabled( false );
            return;
        case AuthAbort:
            return;
        }
    cantCheck();
}

void PasswordDlg::handleVerify()
{
    int ret;
    char *arr;

    while (GRecvInt( &ret )) {
        switch (ret) {
        case ConvGetBinary:
            if (!GRecvArr( &arr ))
                break;
            greet->binaryPrompt( arr, false );
            if (arr)
                ::free( arr );
            return;
        case ConvGetNormal:
            if (!GRecvArr( &arr ))
                break;
            greet->textPrompt( arr, true, false );
            if (arr)
                ::free( arr );
            return;
        case ConvGetHidden:
            if (!GRecvArr( &arr ))
                break;
            greet->textPrompt( arr, false, false );
            if (arr)
                ::free( arr );
            return;
        case ConvPutInfo:
            if (!GRecvArr( &arr ))
                break;
            if (!greet->textMessage( arr, false ))
                static_cast< LockProcess* >(parent())->msgBox( QMessageBox::Information, QString::fromLocal8Bit( arr ) );
            ::free( arr );
            continue;
        case ConvPutError:
            if (!GRecvArr( &arr ))
                break;
            if (!greet->textMessage( arr, true ))
                static_cast< LockProcess* >(parent())->msgBox( QMessageBox::Warning, QString::fromLocal8Bit( arr ) );
            ::free( arr );
            continue;
        }
        break;
    }
    reapVerify();
}

////// greeter plugin callbacks

void PasswordDlg::gplugReturnText( const char *text, int tag )
{
    GSendStr( text );
    if (text)
        GSendInt( tag );
    handleVerify();
}

void PasswordDlg::gplugReturnBinary( const char *data )
{
    if (data) {
        unsigned const char *up = (unsigned const char *)data;
        int len = up[3] | (up[2] << 8) | (up[1] << 16) | (up[0] << 24);
        if (!len)
            GSendArr( 4, data );
        else
            GSendArr( len, data );
    } else
        GSendArr( 0, 0 );
    handleVerify();
}

void PasswordDlg::gplugSetUser( const QString & )
{
    // ignore ...
}

void PasswordDlg::cantCheck()
{
    greet->failed();
    static_cast< LockProcess* >(parent())->msgBox( QMessageBox::Critical,
        i18n("Cannot unlock the session because the authentication system failed to work;\n"
             "you must kill kdesktop_lock (pid %1) manually.").arg(getpid()) );
    greet->revive();
}

//---------------------------------------------------------------------------
//
// Starts the kcheckpass process to check the user's password.
//
void PasswordDlg::gplugStart()
{
    int sfd[2];
    char fdbuf[16];

    if (::socketpair(AF_LOCAL, SOCK_STREAM, 0, sfd)) {
        cantCheck();
        return;
    }
    if ((sPid = ::fork()) < 0) {
        ::close(sfd[0]);
        ::close(sfd[1]);
        cantCheck();
        return;
    }
    if (!sPid) {
        ::close(sfd[0]);
        sprintf(fdbuf, "%d", sfd[1]);
        execlp("kcheckpass", "kcheckpass",
#ifdef HAVE_PAM
               "-c", KSCREENSAVER_PAM_SERVICE,
#endif
               "-m", mPlugin->info->method,
               "-S", fdbuf,
               (char *)0);
        exit(20);
    }
    ::close(sfd[1]);
    sFd = sfd[0];
    handleVerify();
}

void PasswordDlg::gplugActivity()
{
    slotActivity();
}

void PasswordDlg::gplugMsgBox( QMessageBox::Icon type, const QString &text )
{
    QDialog dialog( this, 0, true, WX11BypassWM );
    QFrame *winFrame = new QFrame( &dialog );
    winFrame->setFrameStyle( QFrame::WinPanel | QFrame::Raised );
    winFrame->setLineWidth( 2 );
    QVBoxLayout *vbox = new QVBoxLayout( &dialog );
    vbox->addWidget( winFrame );

    QLabel *label1 = new QLabel( winFrame );
    label1->setPixmap( QMessageBox::standardIcon( type ) );
    QLabel *label2 = new QLabel( text, winFrame );
    KPushButton *button = new KPushButton( KStdGuiItem::ok(), winFrame );
    button->setDefault( true );
    button->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred ) );
    connect( button, SIGNAL( clicked() ), SLOT( accept() ) );

    QGridLayout *grid = new QGridLayout( winFrame, 2, 2, 10 );
    grid->addWidget( label1, 0, 0, Qt::AlignCenter );
    grid->addWidget( label2, 0, 1, Qt::AlignCenter );
    grid->addMultiCellWidget( button, 1,1, 0,1, Qt::AlignCenter );

    static_cast< LockProcess* >(parent())->execDialog( &dialog );
}

void PasswordDlg::slotOK()
{
    greet->next();
}


void PasswordDlg::show()
{
    QDialog::show();
    QApplication::flushX();
}

void PasswordDlg::slotStartNewSession()
{
    if (!KMessageBox::shouldBeShownContinue( ":confirmNewSession" )) {
        DM().startReserve();
        return;
    }

    killTimer(mTimeoutTimerId);
    mTimeoutTimerId = 0;

    QDialog *dialog = new QDialog( this, "warnbox", true, WX11BypassWM );
    QFrame *winFrame = new QFrame( dialog );
    winFrame->setFrameStyle( QFrame::WinPanel | QFrame::Raised );
    winFrame->setLineWidth( 2 );
    QVBoxLayout *vbox = new QVBoxLayout( dialog );
    vbox->addWidget( winFrame );

    QLabel *label1 = new QLabel( winFrame );
    label1->setPixmap( QMessageBox::standardIcon( QMessageBox::Warning ) );
    QString qt_text =
          i18n("You have chosen to open another desktop session "
               "instead of resuming the current one.<br>"
               "The current session will be hidden "
               "and a new login screen will be displayed.<br>"
               "An F-key is assigned to each session; "
               "F%1 is usually assigned to the first session, "
               "F%2 to the second session and so on. "
               "You can switch between sessions by pressing "
               "Ctrl, Alt and the appropriate F-key at the same time. "
               "Additionally, the KDE Panel and Desktop menus have "
               "actions for switching between sessions.")
            .arg(7).arg(8);
    QLabel *label2 = new QLabel( qt_text, winFrame );
    KPushButton *okbutton = new KPushButton( KGuiItem(i18n("&Start New Session"), "fork"), winFrame );
    okbutton->setDefault( true );
    connect( okbutton, SIGNAL( clicked() ), dialog, SLOT( accept() ) );
    KPushButton *cbutton = new KPushButton( KStdGuiItem::cancel(), winFrame );
    connect( cbutton, SIGNAL( clicked() ), dialog, SLOT( reject() ) );

    QBoxLayout *mbox = new QVBoxLayout( winFrame, KDialog::marginHint(), KDialog::spacingHint() );

    QGridLayout *grid = new QGridLayout( mbox, 2, 2, 2 * KDialog::spacingHint() );
    grid->setMargin( KDialog::marginHint() );
    grid->addWidget( label1, 0, 0, Qt::AlignCenter );
    grid->addWidget( label2, 0, 1, Qt::AlignCenter );
    QCheckBox *cb = new QCheckBox( i18n("&Do not ask again"), winFrame );
    grid->addMultiCellWidget( cb, 1,1, 0,1 );

    QBoxLayout *hbox = new QHBoxLayout( mbox, KDialog::spacingHint() );
    hbox->addStretch( 1 );
    hbox->addWidget( okbutton );
    hbox->addStretch( 1 );
    hbox->addWidget( cbutton );
    hbox->addStretch( 1 );

    // stolen from kmessagebox
    int pref_width = 0;
    int pref_height = 0;
    // Calculate a proper size for the text.
    {
       QSimpleRichText rt(qt_text, dialog->font());
       QRect rect = KGlobalSettings::desktopGeometry(dialog);

       pref_width = rect.width() / 3;
       rt.setWidth(pref_width);
       int used_width = rt.widthUsed();
       pref_height = rt.height();
       if (used_width <= pref_width)
       {
          while(true)
          {
             int new_width = (used_width * 9) / 10;
             rt.setWidth(new_width);
             int new_height = rt.height();
             if (new_height > pref_height)
                break;
             used_width = rt.widthUsed();
             if (used_width > new_width)
                break;
          }
          pref_width = used_width;
       }
       else
       {
          if (used_width > (pref_width *2))
             pref_width = pref_width *2;
          else
             pref_width = used_width;
       }
    }
    label2->setFixedSize(QSize(pref_width+10, pref_height));

    int ret = static_cast< LockProcess* >( parent())->execDialog( dialog );

    delete dialog;

    if (ret == QDialog::Accepted) {
        if (cb->isChecked())
            KMessageBox::saveDontShowAgainContinue( ":confirmNewSession" );
        DM().startReserve();
    }

    mTimeoutTimerId = startTimer(PASSDLG_HIDE_TIMEOUT);
}

class LockListViewItem : public QListViewItem {
public:
    LockListViewItem( QListView *parent,
		      const QString &sess, const QString &loc, int _vt )
	: QListViewItem( parent )
	, vt( _vt )
    {
	setText( 0, sess );
	setText( 1, loc );
    }

    int vt;
};

void PasswordDlg::slotSwitchUser()
{
    int p = 0;
    DM dm;

    QDialog dialog( this, "sessbox", true, WX11BypassWM );
    QFrame *winFrame = new QFrame( &dialog );
    winFrame->setFrameStyle( QFrame::WinPanel | QFrame::Raised );
    winFrame->setLineWidth( 2 );
    QBoxLayout *vbox = new QVBoxLayout( &dialog );
    vbox->addWidget( winFrame );

    QBoxLayout *hbox = new QHBoxLayout( winFrame, KDialog::marginHint(), KDialog::spacingHint() );

    QBoxLayout *vbox1 = new QVBoxLayout( hbox, KDialog::spacingHint() );
    QBoxLayout *vbox2 = new QVBoxLayout( hbox, KDialog::spacingHint() );

    KPushButton *btn;

    SessList sess;
    if (dm.localSessions( sess )) {

        lv = new QListView( winFrame );
        connect( lv, SIGNAL(doubleClicked(QListViewItem *, const QPoint&, int)), SLOT(slotSessionActivated()) );
        connect( lv, SIGNAL(doubleClicked(QListViewItem *, const QPoint&, int)), &dialog, SLOT(reject()) );
        lv->setAllColumnsShowFocus( true );
        lv->addColumn( i18n("Session") );
        lv->addColumn( i18n("Location") );
        lv->setColumnWidthMode( 0, QListView::Maximum );
        lv->setColumnWidthMode( 1, QListView::Maximum );
        QListViewItem *itm = 0;
        QString user, loc;
        int ns = 0;
        for (SessList::ConstIterator it = sess.begin(); it != sess.end(); ++it) {
            DM::sess2Str2( *it, user, loc );
            itm = new LockListViewItem( lv, user, loc, (*it).vt );
            if (!(*it).vt)
                itm->setEnabled( false );
            if ((*it).self) {
                lv->setCurrentItem( itm );
                itm->setSelected( true );
            }
            ns++;
        }
        int fw = lv->frameWidth() * 2;
        QSize hds( lv->header()->sizeHint() );
        lv->setMinimumWidth( fw + hds.width() +
            (ns > 10 ? style().pixelMetric(QStyle::PM_ScrollBarExtent) : 0 ) );
        lv->setFixedHeight( fw + hds.height() +
            itm->height() * (ns < 6 ? 6 : ns > 10 ? 10 : ns) );
        lv->header()->adjustHeaderSize();
        vbox1->addWidget( lv );

        btn = new KPushButton( KGuiItem(i18n("session", "&Activate"), "fork"), winFrame );
        connect( btn, SIGNAL(clicked()), SLOT(slotSessionActivated()) );
        connect( btn, SIGNAL(clicked()), &dialog, SLOT(reject()) );
        vbox2->addWidget( btn );
        vbox2->addStretch( 2 );
    }

    if (kapp->authorize("start_new_session") && (p = dm.numReserve()) >= 0)
    {
        btn = new KPushButton( KGuiItem(i18n("Start &New Session"), "fork"), winFrame );
        connect( btn, SIGNAL(clicked()), SLOT(slotStartNewSession()) );
        connect( btn, SIGNAL(clicked()), &dialog, SLOT(reject()) );
        if (!p)
            btn->setEnabled( false );
        vbox2->addWidget( btn );
        vbox2->addStretch( 1 );
    }

    btn = new KPushButton( KStdGuiItem::cancel(), winFrame );
    connect( btn, SIGNAL(clicked()), &dialog, SLOT(reject()) );
    vbox2->addWidget( btn );

    static_cast< LockProcess* >(parent())->execDialog( &dialog );
}

void PasswordDlg::slotSessionActivated()
{
    LockListViewItem *itm = (LockListViewItem *)lv->currentItem();
    if (itm && itm->vt > 0)
        DM().switchVT( itm->vt );
}

void PasswordDlg::capsLocked()
{
    unsigned int lmask;
    Window dummy1, dummy2;
    int dummy3, dummy4, dummy5, dummy6;
    XQueryPointer(qt_xdisplay(), DefaultRootWindow( qt_xdisplay() ), &dummy1, &dummy2, &dummy3, &dummy4, &dummy5, &dummy6, &lmask);
    mCapsLocked = lmask & LockMask;
    updateLabel();
}

#include "lockdlg.moc"
