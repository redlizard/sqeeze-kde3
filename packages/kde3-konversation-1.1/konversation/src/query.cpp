/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

/*
  Copyright (C) 2002 Dario Abatianni <eisfuchs@tigress.com>
  Copyright (C) 2005-2008 Eike Hein <hein@kde.org>
*/

#include "query.h"
#include "channel.h"
#include "server.h"
#include "konversationapplication.h"
#include "konversationmainwindow.h"
#include "viewcontainer.h"
#include "ircinput.h"
#include "ircview.h"
#include "ircviewbox.h"
#include "common.h"
#include "topiclabel.h"

#include <qhbox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qtextcodec.h>
#include <qtooltip.h>
#include <qtextstream.h>
#include <qwhatsthis.h>
#include <qsplitter.h>

#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kstringhandler.h>
#include <kpopupmenu.h>


Query::Query(QWidget* parent, QString _name) : ChatWindow(parent)
{
    name=_name; // need the name a little bit earlier for setServer
    // don't setName here! It will break logfiles!
    //   setName("QueryWidget");
    setType(ChatWindow::Query);

    setChannelEncodingSupported(true);

    m_headerSplitter = new QSplitter(Qt::Vertical, this);

    m_initialShow = true;
    awayChanged=false;
    awayState=false;
    QHBox *box = new QHBox(m_headerSplitter);
    addresseeimage = new QLabel(box, "query_image");
    addresseeimage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    addresseeimage->hide();
    addresseelogoimage = new QLabel(box, "query_logo_image");
    addresseelogoimage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    addresseelogoimage->hide();

    queryHostmask=new Konversation::TopicLabel(box, "query_hostmask");

    QString whatsthis = i18n("<qt>Some details of the person you are talking to in this query is shown in this bar.  The full name and hostmask is shown, along with any image or logo this person has associated with them in the KDE Addressbook.<p>See the <i>Konversation Handbook</i> for information on associating a nick with a contact in the Addressbook, and for an explanation of what the hostmask is.</qt>");
    QWhatsThis::add(addresseeimage, whatsthis);
    QWhatsThis::add(addresseelogoimage, whatsthis);
    QWhatsThis::add(queryHostmask, whatsthis);

    IRCViewBox* ircBox = new IRCViewBox(m_headerSplitter,0);
    setTextView(ircBox->ircView());               // Server will be set later in setServer();
    textView->setAcceptDrops(true);
    connect(textView,SIGNAL(filesDropped(const QStrList&)),this,SLOT(filesDropped(const QStrList&)));
    connect(textView,SIGNAL(popupCommand(int)),this,SLOT(popup(int)));

    // link "Whois", "Ignore" ... menu items into ircview popup
    QPopupMenu* popup=textView->getPopup();
    popup->insertSeparator();
    popup->insertItem(i18n("&Whois"),Konversation::Whois);
    popup->insertItem(i18n("&Version"),Konversation::Version);
    popup->insertItem(i18n("&Ping"),Konversation::Ping);
    popup->insertSeparator();

    popup->insertItem(i18n("Ignore"), Konversation::IgnoreNick);
    popup->insertItem(i18n("Unignore"), Konversation::UnignoreNick);
    popup->setItemVisible(Konversation::IgnoreNick, false);
    popup->setItemVisible(Konversation::UnignoreNick, false);

    if (kapp->authorize("allow_downloading"))
        popup->insertItem(SmallIcon("2rightarrow"),i18n("Send &File..."),Konversation::DccSend);

    popup->insertItem(i18n("Add to Watched Nicks"), Konversation::AddNotify);

    // This box holds the input line
    QHBox* inputBox=new QHBox(this, "input_log_box");
    inputBox->setSpacing(spacing());

    awayLabel=new QLabel(i18n("(away)"),inputBox);
    awayLabel->hide();
    blowfishLabel = new QLabel(inputBox);
    blowfishLabel->hide();
    blowfishLabel->setPixmap(KGlobal::iconLoader()->loadIcon("encrypted", KIcon::Toolbar));
    queryInput=new IRCInput(inputBox);

    getTextView()->installEventFilter(queryInput);
    queryInput->installEventFilter(this);

    // connect the signals and slots
    connect(queryInput,SIGNAL (submit()),this,SLOT (queryTextEntered()) );
    connect(queryInput,SIGNAL (envelopeCommand()),this,SLOT (queryPassthroughCommand()) );
    connect(queryInput,SIGNAL (textPasted(const QString&)),this,SLOT (textPasted(const QString&)) );
    connect(getTextView(), SIGNAL(textPasted(bool)), queryInput, SLOT(paste(bool)));
    connect(getTextView(),SIGNAL (gotFocus()),queryInput,SLOT (setFocus()) );

    connect(textView,SIGNAL (sendFile()),this,SLOT (sendFileMenu()) );
    connect(textView,SIGNAL (extendedPopup(int)),this,SLOT (popup(int)) );
    connect(textView,SIGNAL (autoText(const QString&)),this,SLOT (sendQueryText(const QString&)) );

    updateAppearance();

    setLog(Preferences::log());
}

Query::~Query()
{
}

void Query::setServer(Server* newServer)
{
    if (m_server != newServer)
        connect(newServer, SIGNAL(connectionStateChanged(Server*, Konversation::ConnectionState)),
                SLOT(connectionStateChanged(Server*, Konversation::ConnectionState)));

    ChatWindow::setServer(newServer);

    if (newServer->getKeyForRecipient(getName()))
        blowfishLabel->show();
}

void Query::connectionStateChanged(Server* server, Konversation::ConnectionState state)
{
    if (server == m_server)
    {
        if (state ==  Konversation::SSConnected)
        {
            //HACK the way the notification priorities work sucks, this forces the tab text color to ungray right now.
            if (m_currentTabNotify == Konversation::tnfNone || !Preferences::tabNotificationsEvents())
                KonversationApplication::instance()->getMainWindow()->getViewContainer()->unsetViewNotification(this);
        }
        else
        {
            //HACK the way the notification priorities work sucks, this forces the tab text color to gray right now.
            if (m_currentTabNotify == Konversation::tnfNone || (!Preferences::tabNotificationsEvents() && m_currentTabNotify == Konversation::tnfControl))
                KonversationApplication::instance()->getMainWindow()->getViewContainer()->unsetViewNotification(this);
        }
    }
}

void Query::setName(const QString& newName)
{
    //if(ChatWindow::getName() == newName) return;  // no change, so return

    ChatWindow::setName(newName);

    // don't change logfile name if query name changes
    // This will prevent Nick-Changers to create more than one log file,
    if (logName.isEmpty())
    {
        QString logName =  (Preferences::lowerLog()) ? getName().lower() : getName() ;

        if(Preferences::addHostnameToLog())
        {
            if(m_nickInfo)
                logName += m_nickInfo->getHostmask();
        }

        setLogfileName(logName);
    }
}

void Query::setEncryptedOutput(bool e)
{
    if (e)
        blowfishLabel->show();
    else
        blowfishLabel->hide();
}

void Query::queryTextEntered()
{
    QString line=queryInput->text();
    queryInput->setText("");
    if(line.lower()==Preferences::commandChar()+"clear")
    {
        textView->clear();
    }
    else if(line.lower()==Preferences::commandChar()+"part")
    {
        m_server->closeQuery(getName());
    }
    else if(line.length())
    {
         sendQueryText(line);
    }
}

void Query::queryPassthroughCommand()
{
    QString commandChar = Preferences::commandChar();
    QString line = queryInput->text();

    queryInput->setText("");

    if(!line.isEmpty())
    {
        // Prepend commandChar on Ctrl+Enter to bypass outputfilter command recognition
        if (line.startsWith(commandChar))
        {
            line = commandChar + line;
        }
        sendQueryText(line);
    }
}

void Query::sendQueryText(const QString& sendLine)
{
    // create a work copy
    QString outputAll(sendLine);
    // replace aliases and wildcards
    if(m_server->getOutputFilter()->replaceAliases(outputAll))
    {
        outputAll = m_server->parseWildcards(outputAll, m_server->getNickname(), getName(), QString(), QString(), QString());
    }

    // Send all strings, one after another
    QStringList outList=QStringList::split('\n',outputAll);
    for(unsigned int index=0;index<outList.count();index++)
    {
        QString output(outList[index]);

        // encoding stuff is done in Server()
        Konversation::OutputFilterResult result = m_server->getOutputFilter()->parse(m_server->getNickname(), output, getName());

        if(!result.output.isEmpty())
        {
            if(result.type == Konversation::Action) appendAction(m_server->getNickname(), result.output);
            else if(result.type == Konversation::Command) appendCommandMessage(result.typeString, result.output);
            else if(result.type == Konversation::Program) appendServerMessage(result.typeString, result.output);
            else if(!result.typeString.isEmpty()) appendQuery(result.typeString, result.output);
            else appendQuery(m_server->getNickname(), result.output);
        }
        else if (result.outputList.count())
        {
            Q_ASSERT(result.type==Konversation::Message);
            for ( QStringList::Iterator it = result.outputList.begin(); it != result.outputList.end(); ++it )
            {
                append(m_server->getNickname(), *it);
            }
        }

        if (!result.toServerList.empty())
        {
            m_server->queueList(result.toServerList);
        }
        else
        {
            m_server->queue(result.toServer);
        }
    } // for
}

void Query::updateAppearance()
{
    QColor fg;
    QColor bg;

    if(Preferences::inputFieldsBackgroundColor())
    {
        fg=Preferences::color(Preferences::ChannelMessage);
        bg=Preferences::color(Preferences::TextViewBackground);
    }
    else
    {
        fg=colorGroup().foreground();
        bg=colorGroup().base();
    }

    queryInput->unsetPalette();
    queryInput->setPaletteForegroundColor(fg);
    queryInput->setPaletteBackgroundColor(bg);

    getTextView()->unsetPalette();

    if (Preferences::showBackgroundImage())
    {
        getTextView()->setViewBackground(Preferences::color(Preferences::TextViewBackground),
            Preferences::backgroundImage());
    }
    else
    {
        getTextView()->setViewBackground(Preferences::color(Preferences::TextViewBackground),
            QString());
    }

    if (Preferences::customTextFont())
    {
        getTextView()->setFont(Preferences::textFont());
        queryInput->setFont(Preferences::textFont());
    }
    else
    {
        getTextView()->setFont(KGlobalSettings::generalFont());
        queryInput->setFont(KGlobalSettings::generalFont());
    }

    ChatWindow::updateAppearance();
}

void Query::textPasted(const QString& text)
{
    if(m_server)
    {
        QStringList multiline=QStringList::split('\n',text);
        for(unsigned int index=0;index<multiline.count();index++)
        {
            QString line=multiline[index];
            QString cChar(Preferences::commandChar());
            // make sure that lines starting with command char get escaped
            if(line.startsWith(cChar)) line=cChar+line;
            sendQueryText(line);
        }
    }
}

void Query::indicateAway(bool show)
{
    // QT does not redraw the label properly when they are not on screen
    // while getting hidden, so we remember the "soon to be" state here.
    if(isHidden())
    {
        awayChanged=true;
        awayState=show;
    }
    else
    {
        if(show)
            awayLabel->show();
        else
            awayLabel->hide();
    }
}

// fix QTs broken behavior on hidden QListView pages
void Query::showEvent(QShowEvent*)
{
    if(awayChanged)
    {
        awayChanged=false;
        indicateAway(awayState);
    }

    if(m_initialShow) {
        m_initialShow = false;
        QValueList<int> sizes;
        sizes << queryHostmask->sizeHint().height() << (height() - queryHostmask->sizeHint().height());
        m_headerSplitter->setSizes(sizes);
    }
}

void Query::popup(int id)
{
    // get the nickname to the context menu popup
    QString name = textView->getContextNick();
    // if there was none (right click into the text view) assume query partner
    if (name.isEmpty()) name = getName();

    switch (id)
    {
        case Konversation::Whois:
            sendQueryText(Preferences::commandChar()+"WHOIS "+name+' '+name);
            break;

        case Konversation::IgnoreNick:
        {
            if (KMessageBox::warningContinueCancel(this, i18n("Do you want to ignore %1?").arg(name),
                i18n("Ignore"), i18n("Ignore"), "IgnoreNick") == KMessageBox::Continue)
            {
                sendQueryText(Preferences::commandChar()+"IGNORE -ALL "+name);

                int rc = KMessageBox::questionYesNo(this,
                i18n("Do you want to close this query after ignoring this nickname?"),
                i18n("Close This Query"),
                i18n("Close"),
                i18n("Keep Open"),
                "CloseQueryAfterIgnore");

                if (rc == KMessageBox::Yes && m_server)
                    QTimer::singleShot(0, this, SLOT(closeWithoutAsking()));
            }

            break;
        }
        case Konversation::UnignoreNick:
        {
            QString question = i18n("Do you want to stop ignoring %1?").arg(name);

            if (KMessageBox::warningContinueCancel(this, question, i18n("Unignore"), i18n("Unignore"), "UnignoreNick") ==
                KMessageBox::Continue)
            {
                sendQueryText(Preferences::commandChar()+"UNIGNORE "+name);
            }

            break;
        }
        case Konversation::AddNotify:
        {
            if (m_server->getServerGroup())
            {
                if (!Preferences::isNotify(m_server->getServerGroup()->id(), name))
                    Preferences::addNotify(m_server->getServerGroup()->id(),name);
            }
            break;
        }
        case Konversation::DccSend:
            sendQueryText(Preferences::commandChar()+"DCC SEND "+name);
            break;

        case Konversation::Version:
            sendQueryText(Preferences::commandChar()+"CTCP "+name+" VERSION");
            break;

        case Konversation::Ping:
            sendQueryText(Preferences::commandChar()+"CTCP "+name+" PING");
            break;

        case Konversation::Topic:
            m_server->requestTopic(getTextView()->currentChannel());
            break;
        case Konversation::Names:
            m_server->queue("NAMES " + getTextView()->currentChannel(), Server::LowPriority);
            break;
        case Konversation::Join:
            m_server->queue("JOIN " + getTextView()->currentChannel());
            break;

        default:
            kdDebug() << "Query::popup(): Popup id " << id << " does not belong to me!" << endl;
            break;
    }

    // delete context menu nickname
    textView->clearContextNick();
}

void Query::sendFileMenu()
{
    emit sendFile(getName());
}

void Query::childAdjustFocus()
{
    queryInput->setFocus();
}

void Query::setNickInfo(const NickInfoPtr & nickInfo)
{
    if(m_nickInfo)
        disconnect(m_nickInfo, SIGNAL(nickInfoChanged()), this, SLOT(nickInfoChanged()));

    m_nickInfo = nickInfo;
    Q_ASSERT(m_nickInfo); if(!m_nickInfo) return;
    setName(m_nickInfo->getNickname());
    connect(m_nickInfo, SIGNAL(nickInfoChanged()), this, SLOT(nickInfoChanged()));
    nickInfoChanged();
}

void Query::nickInfoChanged()
{
    if(m_nickInfo)
    {
        setName(m_nickInfo->getNickname());
        QString text = m_nickInfo->getBestAddresseeName();
        if(!m_nickInfo->getHostmask().isEmpty() && !text.isEmpty())
            text += " - ";
        text += m_nickInfo->getHostmask();
        if(m_nickInfo->isAway() && !m_nickInfo->getAwayMessage().isEmpty())
            text += " (" + KStringHandler::rsqueeze(m_nickInfo->getAwayMessage(),100) + ") ";
        queryHostmask->setText(Konversation::removeIrcMarkup(text));

        KABC::Picture pic = m_nickInfo->getAddressee().photo();
        if(pic.isIntern())
        {
            QPixmap qpixmap(pic.data().scaleHeight(queryHostmask->height()));
            if(!qpixmap.isNull())
            {
                addresseeimage->setPixmap(qpixmap);
                addresseeimage->show();
            }
            else
            {
                addresseeimage->hide();
            }
        }
        else
        {
            addresseeimage->hide();
        }
        KABC::Picture logo = m_nickInfo->getAddressee().logo();
        if(logo.isIntern())
        {
            QPixmap qpixmap(logo.data().scaleHeight(queryHostmask->height()));
            if(!qpixmap.isNull())
            {
                addresseelogoimage->setPixmap(qpixmap);
                addresseelogoimage->show();
            }
            else
            {
                addresseelogoimage->hide();
            }
        }
        else
        {
            addresseelogoimage->hide();
        }

        QString strTooltip;
        QTextStream tooltip( &strTooltip, IO_WriteOnly );

        tooltip << "<qt>";

        tooltip << "<table cellspacing=\"0\" cellpadding=\"0\">";

        m_nickInfo->tooltipTableData(tooltip);

        tooltip << "</table></qt>";
        QToolTip::add(queryHostmask, strTooltip);
        QToolTip::add(addresseeimage, strTooltip);
        QToolTip::add(addresseelogoimage, strTooltip);

    }
    else
    {
        addresseeimage->hide();
        addresseelogoimage->hide();
    }

    emit updateQueryChrome(this,getName());
    emitUpdateInfo();
}

NickInfoPtr Query::getNickInfo()
{
    return m_nickInfo;
}

QString Query::getTextInLine() { return queryInput->text(); }

bool Query::canBeFrontView()        { return true; }
bool Query::searchView()       { return true; }

void Query::appendInputText(const QString& s, bool fromCursor)
{
    if(!fromCursor)
    {
        queryInput->append(s);
    }
    else
    {
        int para = 0, index = 0;
        queryInput->getCursorPosition(&para, &index);
        queryInput->insertAt(s, para, index);
        queryInput->setCursorPosition(para, index + s.length());
    }
}

                                                  // virtual
void Query::setChannelEncoding(const QString& encoding)
{
    Preferences::setChannelEncoding(m_server->getDisplayName(), getName(), encoding);
}

QString Query::getChannelEncoding()               // virtual
{
    return Preferences::channelEncoding(m_server->getDisplayName(), getName());
}

QString Query::getChannelEncodingDefaultDesc()    // virtual
{
    return i18n("Identity Default ( %1 )").arg(getServer()->getIdentity()->getCodecName());
}

bool Query::closeYourself(bool confirm)
{
    int result = KMessageBox::Continue;
    if (confirm)
        result=KMessageBox::warningContinueCancel(this, i18n("Do you want to close your query with %1?").arg(getName()), i18n("Close Query"), i18n("Close"), "QuitQueryTab");

    if (result == KMessageBox::Continue)
    {
        m_server->removeQuery(this);
        return true;
    }

    return false;
}

void Query::closeWithoutAsking()
{
    m_server->removeQuery(this);
}

void Query::filesDropped(const QStrList& files)
{
    m_server->sendURIs(files,getName());
}

void Query::serverOnline(bool online)
{
    //queryInput->setEnabled(online);
    getTextView()->setNickAndChannelContextMenusEnabled(online);

    QPopupMenu* popup = getTextView()->getPopup();

    if (popup)
    {
        popup->setItemEnabled(Konversation::Whois, online);
        popup->setItemEnabled(Konversation::Version, online);
        popup->setItemEnabled(Konversation::Ping, online);
        popup->setItemEnabled(Konversation::IgnoreNick, online);
        popup->setItemEnabled(Konversation::UnignoreNick, online);
        popup->setItemEnabled(Konversation::AddNotify, online);

        if (kapp->authorize("allow_downloading"))
            popup->setItemEnabled(Konversation::DccSend, online);
    }
}

void Query::emitUpdateInfo()
{
    QString info;
    if(m_nickInfo->loweredNickname() == m_server->loweredNickname())
        info = i18n("Talking to yourself");
    else if(m_nickInfo)
        info = m_nickInfo->getBestAddresseeName();
    else
        info = getName();

    emit updateInfo(info);
}

// show quit message of nick if we see it
void Query::quitNick(const QString& reason)
{
    QString displayReason = reason;

    if (displayReason.isEmpty())
    {
        appendCommandMessage(i18n("Quit"),i18n("%1 has left this server.").arg(getName()),false);
    }
    else
    {
        if (displayReason.find(QRegExp("[\\0000-\\0037]"))!=-1)
            displayReason+="\017";

        appendCommandMessage(i18n("Quit"),i18n("%1 has left this server (%2).").arg(getName()).arg(displayReason),false);
    }
}

#include "query.moc"
