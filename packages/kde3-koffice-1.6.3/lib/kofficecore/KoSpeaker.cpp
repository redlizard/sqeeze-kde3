/* 
*   This file is part of the KDE/KOffice project.
*   Copyright (C) 2005, Gary Cramblitt <garycramblitt@comcast.net>
*
*   @author Gary Cramblitt <garycramblitt@comcast.net>
*   @since KOffice 1.5
*
*   This library is free software; you can redistribute it and/or
*   modify it under the terms of the GNU Library General Public
*   License as published by the Free Software Foundation; either
*   version 2 of the License, or (at your option) any later version.
*
*   This library is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*   Library General Public License for more details.
*
*   You should have received a copy of the GNU Library General Public License
*   along with this library; see the file COPYING.LIB.  If not, write to
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
*   Boston, MA 02110-1301, USA.
*/

// Qt includes.
#include <qtimer.h>
#include <qcursor.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qmenubar.h>
#include <qlabel.h>
#include <qbutton.h>
#include <qcombobox.h>
#include <qtabbar.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qlistview.h>
#include <qlistbox.h>
#include <qiconview.h>
#include <qtable.h>
#include <qgridview.h>
#include <qregexp.h>
#include <qstylesheet.h>

// KDE includes.
#include <kapplication.h>
#include <klocale.h>
#include <kglobal.h>
#include <dcopclient.h>
#include <kconfig.h>
#include <ktrader.h>
#include <kdebug.h>

// KoSpeaker includes.
#include "KoSpeaker.h"
#include "KoSpeaker.moc"

// ------------------ KoSpeakerPrivate ------------------------

class KoSpeakerPrivate
{
public:
    KoSpeakerPrivate() :
        m_versionChecked(false),
        m_enabled(false),
        m_speakFlags(0),
        m_timeout(600),
        m_timer(0),
        m_prevPointerWidget(0),
        m_prevPointerId(-1),
        m_prevFocusWidget(0),
        m_prevFocusId(-1),
        m_prevWidget(0),
        m_prevId(-1),
        m_cancelSpeakWidget(false)
        {}

    // List of text jobs.
    QValueList<uint> m_jobNums;
    // Whether the version of KTTSD has been requested from the daemon.
    bool m_versionChecked;
    // KTTSD version string.
    QString m_kttsdVersion;
    // Language code of last spoken text.
    QString m_langCode;
    // Word used before speaking an accelerator letter.
    QString m_acceleratorPrefix;
    // Whether TTS service is available or not.
    bool m_enabled;
    // TTS options.
    uint m_speakFlags;
    // Timer which implements the polling interval.
    int m_timeout;
    QTimer* m_timer;
    // Widget and part of widget for 1) last widget under mouse pointer, 2) last widget with focus, and
    // last widget spoken.
    QWidget* m_prevPointerWidget;
    int m_prevPointerId;
    QWidget* m_prevFocusWidget;
    int m_prevFocusId;
    QWidget* m_prevWidget;
    int m_prevId;
    // True when cancelSpeakWidget has been called in response to customSpeakWidget signal.
    bool m_cancelSpeakWidget;
};

// ------------------ KoSpeaker -------------------------------

KoSpeaker* KoSpeaker::KSpkr = 0L;

KoSpeaker::KoSpeaker()
{
    Q_ASSERT(!KSpkr);
    KSpkr = this;
    d = new KoSpeakerPrivate();
    readConfig(KGlobal::config());
}

KoSpeaker::~KoSpeaker()
{
    if (d->m_jobNums.count() > 0) {
        for (int i = d->m_jobNums.count() - 1; i >= 0; i--)
            removeText(d->m_jobNums[i]);
        d->m_jobNums.clear();
    }
    delete d;
    KSpkr = 0;
}

bool KoSpeaker::isEnabled() const { return d->m_enabled; }

void KoSpeaker::probe()
{
    d->m_timer->stop();
    QWidget* w;
    QPoint pos;
    bool spoke = false;
    if ( d->m_speakFlags & SpeakFocusWidget ) {
        w = kapp->focusWidget();
        if (w) {
            spoke = maybeSayWidget(w);
            if (!spoke)
                emit customSpeakWidget(w, pos, d->m_speakFlags);
        }
    }
    if ( !spoke && d->m_speakFlags & SpeakPointerWidget ) {
        pos = QCursor::pos();
        w = kapp->widgetAt(pos, true);
        if (w) {
            if (!maybeSayWidget(w, pos))
                emit customSpeakWidget(w, pos, d->m_speakFlags);
        }
    }
    d->m_timer->start(d->m_timeout);
}

void KoSpeaker::queueSpeech(const QString& msg, const QString& langCode /*= QString()*/, bool first /*= true*/)
{
    if (!startKttsd()) return;
    int jobCount = d->m_jobNums.count();
    if (first && jobCount > 0) {
        for (int i = jobCount - 1; i >= 0; i--)
            removeText(d->m_jobNums[i]);
        d->m_jobNums.clear();
        jobCount = 0;
    }
    QString s = msg.stripWhiteSpace();
    if (s.isEmpty()) return;
    // kdDebug() << "KoSpeaker::queueSpeech: s = [" << s << "]" << endl;
    // If no language code given, assume desktop setting.
    QString languageCode = langCode;
    if (langCode.isEmpty())
        languageCode = KGlobal::locale()->language();
    // kdDebug() << "KoSpeaker::queueSpeech:languageCode = " << languageCode << endl;
    // If KTTSD version is 0.3.5 or later, we can use the appendText method to submit a
    // single, multi-part text job.  Otherwise, must submit separate text jobs.
    // If language code changes, then must also start a new text job so that it will
    // be spoken in correct talker.
    if (getKttsdVersion().isEmpty())
        d->m_jobNums.append(setText(s, languageCode));
    else {
        if ((jobCount == 0) || (languageCode != d->m_langCode))
            d->m_jobNums.append(setText(s, languageCode));
        else
            appendText(s, d->m_jobNums[jobCount-1]);
    }
    d->m_langCode = languageCode;
}

void KoSpeaker::startSpeech()
{
    for (uint i = 0; i < d->m_jobNums.count(); i++)
        startText(d->m_jobNums[i]);
}

void KoSpeaker::readConfig(KConfig* config)
{
    delete d->m_timer;
    d->m_timer = 0;
    config->setGroup("TTS");
    d->m_speakFlags = 0;
    if (config->readBoolEntry("SpeakPointerWidget", false)) d->m_speakFlags |= SpeakPointerWidget;
    if (config->readBoolEntry("SpeakFocusWidget", false)) d->m_speakFlags |= SpeakFocusWidget;
    if (config->readBoolEntry("SpeakTooltips", true)) d->m_speakFlags |= SpeakTooltip;
    if (config->readBoolEntry("SpeakWhatsThis", false)) d->m_speakFlags |= SpeakWhatsThis;
    if (config->readBoolEntry("SpeakDisabled", true)) d->m_speakFlags |= SpeakDisabled;
    if (config->readBoolEntry("SpeakAccelerators", true)) d->m_speakFlags |= SpeakAccelerator;
    d->m_timeout = config->readNumEntry("PollingInterval", 600);
    d->m_acceleratorPrefix = config->readEntry("AcceleratorPrefixWord", i18n("Accelerator"));
    if (d->m_speakFlags & (SpeakPointerWidget | SpeakFocusWidget)) {
        if (startKttsd()) {
            d->m_timer = new QTimer( this );
            connect( d->m_timer, SIGNAL(timeout()), this, SLOT(probe()) );
            d->m_timer->start( d->m_timeout );
        }
    }
}

bool KoSpeaker::maybeSayWidget(QWidget* w, const QPoint& pos /*=QPoint()*/)
{
    if (!w) return false;

    int id = -1;
    QString text;

    if (w->inherits("QViewportWidget")) {
        w = w->parentWidget();
        if (!w) return false;
    }

    // Handle widgets that have multiple parts.

    if ( w->inherits("QMenuBar") ) {
        QMenuBar* menuBar = dynamic_cast<QMenuBar *>(w);
        if (pos == QPoint()) {
            for (uint i = 0; i < menuBar->count(); ++i)
                if (menuBar->isItemActive(menuBar->idAt(i))) {
                    id = menuBar->idAt(i);
                    break;
                }
        }
            // TODO: This doesn't work.  Need way to figure out the QMenuItem underneath mouse pointer.
            // id = menuBarItemAt(menuBar, pos);
        if ( id != -1 )
            text = menuBar->text(id);
    }
    else
    if (w->inherits("QPopupMenu")) {
        QPopupMenu* popupMenu = dynamic_cast<QPopupMenu *>(w);
        if (pos == QPoint()) {
            for (uint i = 0; i < popupMenu->count(); ++i)
                if (popupMenu->isItemActive(popupMenu->idAt(i))) {
                    id = popupMenu->idAt(i);
                    break;
                }
        } else
            id = popupMenu->idAt(popupMenu->mapFromGlobal(pos));
        if ( id != -1 )
            text = popupMenu->text(id);
    }
    else
    if (w->inherits("QTabBar")) {
        QTabBar* tabBar = dynamic_cast<QTabBar *>(w);
        QTab* tab = 0;
        if (pos == QPoint())
            tab = tabBar->tabAt(tabBar->currentTab());
        else
            tab = tabBar->selectTab(tabBar->mapFromGlobal(pos));
        if (tab) {
            id = tab->identifier();
            text = tab->text();
        }
    }
    else
    if (w->inherits("QListView")) {
        QListView* lv = dynamic_cast<QListView *>(w);
        QListViewItem* item = 0;
        if (pos == QPoint())
            item = lv->currentItem();
        else
            item = lv->itemAt(lv->viewport()->mapFromGlobal(pos));
        if (item) {
            id = lv->itemPos(item);
            text = item->text(0);
            for (int col = 1; col < lv->columns(); ++col)
                if (!item->text(col).isEmpty()) text += ". " + item->text(col);
        }
    }
    else
    if (w->inherits("QListBox")) {
        QListBox* lb = dynamic_cast<QListBox *>(w);
        // qt docs say coordinates are in "on-screen" coordinates.  What does that mean?
        QListBoxItem* item = 0;
        if (pos == QPoint())
            item = lb->item(lb->currentItem());
        else
            item = lb->itemAt(lb->mapFromGlobal(pos));
        if (item) {
            id = lb->index(item);
            text = item->text();
        }
    }
    else
    if (w->inherits("QIconView")) {
        QIconView* iv = dynamic_cast<QIconView *>(w);
        QIconViewItem* item = 0;
        if (pos == QPoint())
            item = iv->currentItem();
        else
            item = iv->findItem(iv->viewportToContents(iv->viewport()->mapFromGlobal(pos)));
        if (item) {
            id = item->index();
            text = item->text();
        }
    }
    else
    if (w->inherits("QTable")) {
        QTable* tbl = dynamic_cast<QTable *>(w);
        int row = -1;
        int col = -1;
        if (pos == QPoint()) {
            row = tbl->currentRow();
            col = tbl->currentColumn();
        } else {
            QPoint p = tbl->viewportToContents(tbl->viewport()->mapFromGlobal(pos));
            row = tbl->rowAt(p.y());
            col = tbl->columnAt(p.x());
        }
        if (row >= 0 && col >= 0) {
            id = (row * tbl->numCols()) + col;
            text = tbl->text(row, col);
        }
    }
    else
    if (w->inherits("QGridView")) {
        QGridView* gv = dynamic_cast<QGridView *>(w);
        // TODO: QGridView does not have a "current" row or column.  Don't think they can even get focus?
        int row = -1;
        int col = -1;
        if (pos != QPoint()) {
            QPoint p = gv->viewportToContents(gv->viewport()->mapFromGlobal(pos));
            row = gv->rowAt(p.y());
            col = gv->columnAt(p.x());
        }
        if (row >= 0 && col >= 0)
            id = (row * gv->numCols()) + col;
    }

    if (pos == QPoint()) {
        if ( w == d->m_prevFocusWidget && id == d->m_prevFocusId) return false;
        d->m_prevFocusWidget = w;
        d->m_prevFocusId = id;
    } else {
        if ( w == d->m_prevPointerWidget && id == d->m_prevPointerId) return false;
        d->m_prevPointerWidget = w;
        d->m_prevPointerId = id;
    }
    if (w == d->m_prevWidget && id == d->m_prevId) return false;
    d->m_prevWidget = w;
    d->m_prevId = id;

    // kdDebug() << " w = " << w << endl;

    d->m_cancelSpeakWidget = false;
    emit customSpeakNewWidget(w, pos, d->m_speakFlags);
    if (d->m_cancelSpeakWidget) return true;

    // Handle simple, single-part widgets.
    if ( w->inherits("QButton") )
        text = dynamic_cast<QButton *>(w)->text();
    else
    if (w->inherits("QComboBox"))
        text = dynamic_cast<QComboBox *>(w)->currentText();
    else 
    if (w->inherits("QLineEdit"))
        text = dynamic_cast<QLineEdit *>(w)->text();
    else 
    if (w->inherits("QTextEdit"))
        text = dynamic_cast<QTextEdit *>(w)->text();
    else
    if (w->inherits("QLabel"))
        text = dynamic_cast<QLabel *>(w)->text();
    else
    if (w->inherits("QGroupBox")) {
        // TODO: Should calculate this number from font size?
        if (w->mapFromGlobal(pos).y() < 30)
            text = dynamic_cast<QGroupBox *>(w)->title();
    }
//    else
//     if (w->inherits("QWhatsThat")) {
//         text = dynamic_cast<QWhatsThat *>(w)->text();
//     }

    text = text.stripWhiteSpace();
    if (!text.isEmpty()) {
        if (text.right(1) == ".")
            text += " ";
        else
            text += ". ";
    }
    if (d->m_speakFlags & SpeakTooltip || text.isEmpty()) {
        // kdDebug() << "pos = " << pos << endl;
        // QPoint p = w->mapFromGlobal(pos);
        // kdDebug() << "p = " << p << endl;
        QString t = QToolTip::textFor(w, pos);
        t = t.stripWhiteSpace();
        if (!t.isEmpty()) {
            if (t.right(1) != ".") t += ".";
            text += t + " ";
        }
    }

    if (d->m_speakFlags & SpeakWhatsThis || text.isEmpty()) {
        QString t = QWhatsThis::textFor(w, pos);
        t = t.stripWhiteSpace();
        if (!t.isEmpty()) {
            if (t.right(1) != ".") t += ".";
            text += t + " ";
        }
    }

    if (d->m_speakFlags & SpeakDisabled) {
        if (!w->isEnabled())
            text += i18n("A grayed widget", "Disabled. ");
    }

    return sayWidget(text);
}

bool KoSpeaker::sayWidget(const QString& msg)
{
    QString s = msg;
    if (d->m_speakFlags & SpeakAccelerator) {
        int amp = s.find("&");
        if (amp >= 0) {
            QString acc = s.mid(++amp,1);
            acc = acc.stripWhiteSpace();
            if (!acc.isEmpty())
                s += ". " + d->m_acceleratorPrefix + " " + acc + ".";
        }
    }
    s.remove("&");
    if (QStyleSheet::mightBeRichText(s)) {
        // kdDebug() << "richtext" << endl;
        s.replace(QRegExp("</?[pbius]>"), "");
        s.replace(QRegExp("</?h\\d>"), "");
        s.replace(QRegExp("<(br|hr)>"), " ");
        s.replace(QRegExp(
            "</?(qt|center|li|pre|div|span|em|strong|big|small|sub|sup|code|tt|font|nobr|ul|ol|dl|dt)>"), "");
        s.replace(QRegExp("</?(table|tr|th|td).*>"), "");
        s.replace(QRegExp("</?a\\s.+>"), "");
        // Replace <img source="small|frame_text"> with "small frame_text image. "
        s.replace(QRegExp("<img\\s.*(?:source=|src=)\"([^|\"]+)[|]?([^|\"]*)\">"), "\\1 \\2 image. ");
    }
    if (s.isEmpty()) return false;
    s.replace("Ctrl+", i18n("control plus "));
    s.replace("Alt+", i18n("alt plus "));
    s.replace("+", i18n(" plus "));
    sayScreenReaderOutput(s, "");
    return true;
}

// This doesn't work.  Anybody know how to find the menu item underneath mouse pointer
// in a QMenuBar?
// int KoSpeaker::menuBarItemAt(QMenuBar* m, const QPoint& p)
// {
//     for (uint i = 0; i < m->count(); i++) {
//         int id = m->idAt(i);
//         QMenuItem* mi = m->findItem(id);
//         QWidget* w = mi->widget();
//         if (w->rect().contains(w->mapFromGlobal(p))) return id;
//     }
//     return -1;
// }

/*static*/ bool KoSpeaker::isKttsdInstalled()
{
     KTrader::OfferList offers = KTrader::self()->query("DCOP/Text-to-Speech", "Name == 'KTTSD'");
     return (offers.count() > 0);
}

bool KoSpeaker::startKttsd()
{
    DCOPClient *client = kapp->dcopClient();
    // If KTTSD not running, start it.
    if (!client->isApplicationRegistered("kttsd"))
    {
        QString error;
        if (kapp->startServiceByDesktopName("kttsd", QStringList(), &error)) {
            kdDebug() << "KoSpeaker::startKttsd: error starting KTTSD service: " << error << endl;
            d->m_enabled = false;
        } else
            d->m_enabled = true;
    } else
        d->m_enabled = true;
    return d->m_enabled;
}

QString KoSpeaker::getKttsdVersion()
{
    // Determine which version of KTTSD is running.  Note that earlier versions of KSpeech interface
    // did not support version() method, so we must manually marshall this call ourselves.
    if (d->m_enabled) {
        if (!d->m_versionChecked) {
            DCOPClient *client = kapp->dcopClient();
            QByteArray  data;
            QCString    replyType;
            QByteArray  replyData;
            if ( client->call("kttsd", "KSpeech", "version()", data, replyType, replyData, true) ) {
                QDataStream arg(replyData, IO_ReadOnly);
                arg >> d->m_kttsdVersion;
                kdDebug() << "KoSpeaker::startKttsd: KTTSD version = " << d->m_kttsdVersion << endl;
            }
            d->m_versionChecked = true;
        }
    }
    return d->m_kttsdVersion;
}

void KoSpeaker::sayScreenReaderOutput(const QString &msg, const QString &talker)
{
    if (msg.isEmpty()) return;
    DCOPClient *client = kapp->dcopClient();
    QByteArray  data;
    QCString    replyType;
    QByteArray  replyData;
    QDataStream arg(data, IO_WriteOnly);
    arg << msg << talker;
    if ( !client->call("kttsd", "KSpeech", "sayScreenReaderOutput(QString,QString)",
        data, replyType, replyData, true) ) {
        kdDebug() << "KoSpeaker::sayScreenReaderOutput: failed" << endl;
    }
}

uint KoSpeaker::setText(const QString &text, const QString &talker)
{
    if (text.isEmpty()) return 0;
    DCOPClient *client = kapp->dcopClient();
    QByteArray  data;
    QCString    replyType;
    QByteArray  replyData;
    QDataStream arg(data, IO_WriteOnly);
    arg << text << talker;
    uint jobNum = 0;
    if ( !client->call("kttsd", "KSpeech", "setText(QString,QString)",
        data, replyType, replyData, true) ) {
        kdDebug() << "KoSpeaker::sayText: failed" << endl;
    } else {
        QDataStream arg2(replyData, IO_ReadOnly);
        arg2 >> jobNum;
    }
    return jobNum;
}

int KoSpeaker::appendText(const QString &text, uint jobNum /*=0*/)
{
    if (text.isEmpty()) return 0;
    DCOPClient *client = kapp->dcopClient();
    QByteArray  data;
    QCString    replyType;
    QByteArray  replyData;
    QDataStream arg(data, IO_WriteOnly);
    arg << text << jobNum;
    int partNum = 0;
    if ( !client->call("kttsd", "KSpeech", "appendText(QString,uint)",
        data, replyType, replyData, true) ) {
        kdDebug() << "KoSpeaker::appendText: failed" << endl;
    } else {
        QDataStream arg2(replyData, IO_ReadOnly);
        arg2 >> partNum;
    }
    return partNum;
}

void KoSpeaker::startText(uint jobNum /*=0*/)
{
    DCOPClient *client = kapp->dcopClient();
    QByteArray  data;
    QCString    replyType;
    QByteArray  replyData;
    QDataStream arg(data, IO_WriteOnly);
    arg << jobNum;
    if ( !client->call("kttsd", "KSpeech", "startText(uint)",
        data, replyType, replyData, true) ) {
        kdDebug() << "KoSpeaker::startText: failed" << endl;
    }
}

void KoSpeaker::removeText(uint jobNum /*=0*/)
{
    DCOPClient *client = kapp->dcopClient();
    QByteArray  data;
    QCString    replyType;
    QByteArray  replyData;
    QDataStream arg(data, IO_WriteOnly);
    arg << jobNum;
    if ( !client->call("kttsd", "KSpeech", "removeText(uint)",
        data, replyType, replyData, true) ) {
        kdDebug() << "KoSpeaker::removeText: failed" << endl;
    }
}

