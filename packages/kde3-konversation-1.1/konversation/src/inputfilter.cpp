/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

/*
  Copyright (C) 2002 Dario Abatianni <eisfuchs@tigress.com>
  Copyright (C) 2004 Peter Simonsson <psn@linux.se>
  Copyright (C) 2006-2008 Eike Hein <hein@kde.org>
*/

#include "inputfilter.h"
#include "server.h"
#include "replycodes.h"
#include "konversationapplication.h"
#include "commit.h"
#include "version.h"
#include "query.h"
#include "channel.h"
#include "statuspanel.h"
#include "common.h"
#include "notificationhandler.h"

#include <qdatastream.h>
#include <qstringlist.h>
#include <qdatetime.h>
#include <qregexp.h>

#include <klocale.h>
#include <kdeversion.h>
#include <kstringhandler.h>
#include <config.h>
#include <kdebug.h>
#include <kresolver.h>

InputFilter::InputFilter()
{
    m_connecting = false;
}

InputFilter::~InputFilter()
{
}

void InputFilter::setServer(Server* newServer)
{
    server=newServer;
}

void InputFilter::parseLine(const QString& a_newLine)
{
    QString trailing;
    QString newLine(a_newLine);

    // Remove white spaces at the end and beginning
    newLine = newLine.stripWhiteSpace();
    // Find end of middle parameter list
    int pos = newLine.find(" :");
    // Was there a trailing parameter?
    if(pos != -1)
    {
        // Copy trailing parameter
        trailing = newLine.mid(pos + 2);
        // Cut trailing parameter from string
        newLine = newLine.left(pos);
    }
    // Remove all unnecessary white spaces to make parsing easier
    newLine = newLine.simplifyWhiteSpace();

    QString prefix;

    // Do we have a prefix?
    if(newLine[0] == ':')
    {
        // Find end of prefix
        pos = newLine.find(' ');
        // Copy prefix
        prefix = newLine.mid(1, pos - 1);
        // Remove prefix from line
        newLine = newLine.mid(pos + 1);
    }

    // Find end of command
    pos = newLine.find(' ');
    // Copy command (all lowercase to make parsing easier)
    QString command = newLine.left(pos).lower();
    // Are there parameters left in the string?
    QStringList parameterList;

    if(pos != -1)
    {
        // Cut out the command
        newLine = newLine.mid(pos + 1);
        // The rest of the string will be the parameter list
        parameterList = QStringList::split(" ", newLine);
    }

    Q_ASSERT(server);

    // Server command, if no "!" was found in prefix
    if((prefix.find('!') == -1) && (prefix != server->getNickname()))
    {

        parseServerCommand(prefix, command, parameterList, trailing);
    }
    else
    {
        parseClientCommand(prefix, command, parameterList, trailing);
    }
}

void InputFilter::parseClientCommand(const QString &prefix, const QString &command, const QStringList &parameterList, const QString &_trailing)
{
    KonversationApplication* konv_app = static_cast<KonversationApplication *>(KApplication::kApplication());
    Q_ASSERT(konv_app);
    Q_ASSERT(server);
    // Extract nickname from prefix
    int pos = prefix.find("!");
    QString sourceNick = prefix.left(pos);
    QString sourceHostmask = prefix.mid(pos + 1);
    // remember hostmask for this nick, it could have changed
    server->addHostmaskToNick(sourceNick,sourceHostmask);
    QString trailing = _trailing;

    if(command=="privmsg")
    {
        bool isChan = isAChannel(parameterList[0]);
        // CTCP message?
        if(server->identifyMsg() && (trailing.at(0) == '+' || trailing.at(0) == '-')) {
            trailing = trailing.mid(1);
        }

        if(trailing.at(0)==QChar(0x01))
        {
            // cut out the CTCP command
            QString ctcp = trailing.mid(1,trailing.find(1,1)-1);

            QString ctcpCommand=ctcp.left(ctcp.find(" ")).lower();
            QString ctcpArgument=ctcp.mid(ctcp.find(" ")+1);
            ctcpArgument=static_cast<KonversationApplication*>(kapp)->doAutoreplace(ctcpArgument,false);

            // If it was a ctcp action, build an action string
            if(ctcpCommand=="action" && isChan)
            {
                if(!isIgnore(prefix,Ignore::Channel))
                {
                    Channel* channel = server->getChannelByName( parameterList[0] );

                    if(!channel) {
                        kdError() << "Didn't find the channel " << parameterList[0] << endl;
                        return;
                    }

                    channel->appendAction(sourceNick,ctcpArgument);

                    if(sourceNick != server->getNickname())
                    {
                        if(ctcpArgument.lower().find(QRegExp("(^|[^\\d\\w])"
                            + QRegExp::escape(server->loweredNickname())
                            + "([^\\d\\w]|$)")) !=-1 )
                        {
                            konv_app->notificationHandler()->nick(channel, sourceNick, ctcpArgument);
                        }
                        else
                        {
                            konv_app->notificationHandler()->message(channel, sourceNick, ctcpArgument);
                        }
                    }
                }
            }
            // If it was a ctcp action, build an action string
            else if(ctcpCommand=="action" && !isChan)
            {
                // Check if we ignore queries from this nick
                if(!isIgnore(prefix,Ignore::Query))
                {
                    NickInfoPtr nickinfo = server->obtainNickInfo(sourceNick);
                    nickinfo->setHostmask(sourceHostmask);

                    // create new query (server will check for dupes)
                    query = server->addQuery(nickinfo, false /* we didn't initiate this*/ );

                    // send action to query
                    query->appendAction(sourceNick,ctcpArgument);

                    if(sourceNick != server->getNickname() && query)
                    {
                        konv_app->notificationHandler()->queryMessage(query, sourceNick, ctcpArgument);
                    }
                }
            }

            // Answer ping requests
            else if(ctcpCommand=="ping")
            {
                if(!isIgnore(prefix,Ignore::CTCP))
                {
                    if(isChan)
                    {
                        server->appendMessageToFrontmost(i18n("CTCP"),
                            i18n("Received CTCP-PING request from %1 to channel %2, sending answer.")
                            .arg(sourceNick).arg(parameterList[0])
                            );
                    }
                    else
                    {
                        server->appendMessageToFrontmost(i18n("CTCP"),
                            i18n("Received CTCP-%1 request from %2, sending answer.")
                            .arg("PING").arg(sourceNick)
                            );
                    }
                    server->ctcpReply(sourceNick,QString("PING %1").arg(ctcpArgument));
                }
            }

            // Maybe it was a version request, so act appropriately
            else if(ctcpCommand=="version")
            {
                if(!isIgnore(prefix,Ignore::CTCP))
                {
                    if (isChan)
                    {
                        server->appendMessageToFrontmost(i18n("CTCP"),
                            i18n("Received Version request from %1 to channel %2.")
                            .arg(sourceNick).arg(parameterList[0])
                            );
                    }
                    else
                    {
                        server->appendMessageToFrontmost(i18n("CTCP"),
                            i18n("Received Version request from %1.")
                            .arg(sourceNick)
                            );
                    }

                    QString reply;
                    if(Preferences::customVersionReplyEnabled())
                    {
                        reply = Preferences::customVersionReply().stripWhiteSpace();
                    }
                    else
                    {
                        // Do not internationalize the below version string
                        reply = QString("Konversation %1 (C) 2002-2008 by the Konversation team")
                            .arg(QString(KONVI_VERSION));
                    }
                    server->ctcpReply(sourceNick,"VERSION "+reply);
                }
            }
            // DCC request?
            else if(ctcpCommand=="dcc" && !isChan)
            {
                if(!isIgnore(prefix,Ignore::DCC))
                {
                    // Extract DCC type and argument list
                    QString dccType=ctcpArgument.lower().section(' ',0,0);

                    // Support file names with spaces
                    QString dccArguments = ctcpArgument.mid(ctcpArgument.find(" ")+1);
                    QStringList dccArgumentList;

                    if ((dccArguments.contains('\"') >= 2) && (dccArguments.startsWith("\""))) {
                        int lastQuotePos = dccArguments.findRev("\"");
                        if (dccArguments[lastQuotePos+1] == ' ') {
                            QString fileName = dccArguments.mid(1, lastQuotePos-1);
                            dccArguments = dccArguments.mid(lastQuotePos+2);

                            dccArgumentList.append(fileName);
                        }
                    }
                    dccArgumentList += QStringList::split(' ', dccArguments);

                    if(dccType=="send")
                    {
                        if(dccArgumentList.count()==4)
                        {
                            // incoming file
                            konv_app->notificationHandler()->dccIncoming(server->getStatusView(), sourceNick);
                            emit addDccGet(sourceNick,dccArgumentList);
                        }
                        else if(dccArgumentList.count()==5)
                        {
                            if(dccArgumentList[2]=="0")
                            {
                                // incoming file (Reverse DCC)
                                konv_app->notificationHandler()->dccIncoming(server->getStatusView(), sourceNick);
                                emit addDccGet(sourceNick,dccArgumentList);
                            }
                            else
                            {
                                // the receiver accepted the offer for Reverse DCC
                                emit startReverseDccSendTransfer(sourceNick,dccArgumentList);
                            }
                        }
                        else
                        {
                            server->appendMessageToFrontmost(i18n("DCC"),
                                i18n("Received invalid DCC SEND request from %1.")
                                .arg(sourceNick)
                                );
                        }
                    }
                    else if(dccType=="accept")
                    {
                        // resume request was accepted
                        if(dccArgumentList.count()==3)
                        {
                            emit resumeDccGetTransfer(sourceNick,dccArgumentList);
                        }
                        else
                        {
                            server->appendMessageToFrontmost(i18n("DCC"),
                                i18n("Received invalid DCC ACCEPT request from %1.")
                                .arg(sourceNick)
                                );
                        }
                    }
                    // Remote client wants our sent file resumed
                    else if(dccType=="resume")
                    {
                        if(dccArgumentList.count()==3)
                        {
                            emit resumeDccSendTransfer(sourceNick,dccArgumentList);
                        }
                        else
                        {
                            server->appendMessageToFrontmost(i18n("DCC"),
                                i18n("Received invalid DCC RESUME request from %1.")
                                .arg(sourceNick)
                                );
                        }
                    }
                    else if(dccType=="chat")
                    {

                        if(dccArgumentList.count()==3)
                        {
                            // will be connected via Server to KonversationMainWindow::addDccChat()
                            emit addDccChat(server->getNickname(),sourceNick,dccArgumentList,false);
                        }
                        else
                        {
                            server->appendMessageToFrontmost(i18n("DCC"),
                                i18n("Received invalid DCC CHAT request from %1.")
                                .arg(sourceNick)
                                );
                        }
                    }
                    else
                    {
                        server->appendMessageToFrontmost(i18n("DCC"),
                            i18n("Unknown DCC command %1 received from %2.")
                            .arg(ctcpArgument).arg(sourceNick)
                            );
                    }
                }
            }
            else if (ctcpCommand=="clientinfo" && !isChan)
            {
                server->appendMessageToFrontmost(i18n("CTCP"),
                    i18n("Received CTCP-%1 request from %2, sending answer.")
                    .arg("CLIENTINFO").arg(sourceNick)
                    );
                server->ctcpReply(sourceNick,QString("CLIENTINFO ACTION CLIENTINFO DCC PING TIME VERSION"));
            }
            else if(ctcpCommand=="time" && !isChan)
            {
                server->appendMessageToFrontmost(i18n("CTCP"),
                    i18n("Received CTCP-%1 request from %2, sending answer.")
                    .arg("TIME").arg(sourceNick)
                    );
                server->ctcpReply(sourceNick,QString("TIME ")+QDateTime::currentDateTime().toString());
            }

            // No known CTCP request, give a general message
            else
            {
                if(!isIgnore(prefix,Ignore::CTCP))
                {
                    if (isChan)
                        server->appendServerMessageToChannel(
                            parameterList[0],
                            "CTCP",
                            i18n("Received unknown CTCP-%1 request from %2 to Channel %3.")
                            .arg(ctcp).arg(sourceNick).arg(parameterList[0])
                            );
                    else
                        server->appendMessageToFrontmost(i18n("CTCP"),
                            i18n("Received unknown CTCP-%1 request from %2.")
                            .arg(ctcp).arg(sourceNick)
                            );
                }
            }
        }
        // No CTCP, so it's an ordinary channel or query message
        else
        {
            parsePrivMsg (prefix, parameterList, trailing);
        }
    }
    else if(command=="notice")
    {
        if(!isIgnore(prefix,Ignore::Notice))
        {
            // Channel notice?
            if(isAChannel(parameterList[0]))
            {
                if(server->identifyMsg())
                {
                    trailing = trailing.mid(1);
                }

                server->appendServerMessageToChannel(parameterList[0], i18n("Notice"),
                    i18n("-%1 to %2- %3")
                    .arg(sourceNick).arg(parameterList[0]).arg(trailing)
                    );
            }
            // Private notice
            else
            {
                // Was this a CTCP reply?
                if(trailing.at(0)==QChar(0x01))
                {
                    // cut 0x01 bytes from trailing string
                    QString ctcp(trailing.mid(1,trailing.length()-2));
                    QString replyReason(ctcp.section(' ',0,0));
                    QString reply(ctcp.section(' ',1));

                    // pong reply, calculate turnaround time
                    if(replyReason.lower()=="ping")
                    {
                        int dateArrived=QDateTime::currentDateTime().toTime_t();
                        int dateSent=reply.toInt();
                        int time = dateArrived-dateSent;
                        QString unit = "seconds";

                        if (time==1)
                            unit = "second";

                        server->appendMessageToFrontmost(i18n("CTCP"),
                            i18n("Received CTCP-PING reply from %1: %2 %3.")
                            .arg(sourceNick)
                            .arg(time)
                            .arg(unit)
                            );
                    }
                    // all other ctcp replies get a general message
                    else
                    {
                        server->appendMessageToFrontmost(i18n("CTCP"),
                            i18n("Received CTCP-%1 reply from %2: %3.")
                            .arg(replyReason).arg(sourceNick).arg(reply)
                            );
                    }
                }
                // No, so it was a normal notice
                else
                {
                    // Nickserv
                    if (trailing.startsWith("If this is your nick"))
                    {
                        // Identify command if specified
                        server->registerWithServices();
                    }
                    else if (server->identifyMsg())
                        trailing = trailing.mid(1);

                    if(trailing.lower() == "password accepted - you are now recognized"
                        || trailing.lower() == "you have already identified")
                    {
                        NickInfoPtr nickInfo = server->getNickInfo(server->getNickname());
                        if(nickInfo)
                            nickInfo->setIdentified(true);
                    }
                    server->appendMessageToFrontmost(i18n("Notice"), i18n("-%1- %2").arg(sourceNick).arg(trailing));
                }
            }
        }
    }
    else if(command=="join")
    {
        QString channelName(trailing);
        // Sometimes JOIN comes without ":" in front of the channel name
        if(channelName.isEmpty())
            channelName=parameterList[parameterList.count()-1];

        // Did we join the channel, or was it someone else?
        if(server->isNickname(sourceNick))
        {
            /*
                QString key;
                // TODO: Try to remember channel keys for autojoins and manual joins, so
                //       we can get %k to work

                if(channelName.find(' ')!=-1)
                {
                    key=channelName.section(' ',1,1);
                    channelName=channelName.section(' ',0,0);
                }
            */

            // Join the channel
            server->joinChannel(channelName, sourceHostmask);

            server->resetNickList(channelName);

            // Upon JOIN we're going to receive some NAMES input from the server which
            // we need to be able to tell apart from manual invocations of /names
            setAutomaticRequest("NAMES",channelName,true);

            server->getChannelByName(channelName)->clearModeList();

            // Request modes for the channel
            server->queue("MODE "+channelName, Server::LowPriority);

            // Initiate channel ban list
            server->getChannelByName(channelName)->clearBanList();
            setAutomaticRequest("BANLIST",channelName,true);
            server->queue("MODE "+channelName+" +b", Server::LowPriority);
        }
        else
        {
            Channel* channel = server->nickJoinsChannel(channelName,sourceNick,sourceHostmask);
            konv_app->notificationHandler()->join(channel, sourceNick);
        }
    }
    else if(command=="kick")
    {
        server->nickWasKickedFromChannel(parameterList[0],parameterList[1],sourceNick,trailing);
    }
    else if(command=="part")
    {
        /* FIXME: Ugly workaround for a version of the PART line encountered on ircu:
         *   :Nick!user@host PART :#channel
         * Quote: "The final colon is specified as a "last argument" designator, and
         * is always valid before the final argument."
         */

        QString channel;
        QString reason;

        if (parameterList[0].isEmpty())
        {
            channel = trailing;
        }
        else
        {
            channel = parameterList[0];
            reason = trailing;
        }

        Channel* channelPtr = server->removeNickFromChannel(channel,sourceNick,reason);

        if(sourceNick != server->getNickname())
        {
            konv_app->notificationHandler()->part(channelPtr, sourceNick);
        }
    }
    else if(command=="quit")
    {
        server->removeNickFromServer(sourceNick,trailing);
        if(sourceNick != server->getNickname())
        {
            konv_app->notificationHandler()->quit(server->getStatusView(), sourceNick);
        }
    }
    else if(command=="nick")
    {
        QString newNick(trailing);

        // Message may not include ":" in front of the new nickname
        if (newNick.isEmpty())
            newNick=parameterList[parameterList.count()-1];

        server->renameNick(sourceNick,newNick);

        if (sourceNick != server->getNickname())
        {
            konv_app->notificationHandler()->nickChange(server->getStatusView(), sourceNick, newNick);
        }
    }
    else if(command=="topic")
    {
        server->setChannelTopic(sourceNick,parameterList[0],trailing);
    }
    else if(command=="mode") // mode #channel -/+ mmm params
    {
        QStringList params=parameterList;
        if (!trailing.isEmpty())
            params << trailing;
        parseModes(sourceNick, params);
        Channel* channel = server->getChannelByName(parameterList[0]);
        if(sourceNick != server->getNickname())
        {
            konv_app->notificationHandler()->mode(channel, sourceNick);
        }
    }
    else if(command=="invite")
    {
        QString channel;

        if (parameterList.count() > 1)
            channel = parameterList[1];
        else
            channel = trailing;

        server->appendMessageToFrontmost(i18n("Invite"),
            i18n("%1 invited you to channel %2.")
            .arg(sourceNick).arg(channel)
            );
        emit invitation(sourceNick,channel);
    }
    else
    {
        server->appendMessageToFrontmost(command,parameterList.join(" ")+' '+trailing);
    }
}

void InputFilter::parseServerCommand(const QString &prefix, const QString &command, const QStringList &parameterList, const QString &trailing)
{
    bool isNumeric;
    int numeric = command.toInt(&isNumeric);

    Q_ASSERT(server); if(!server) return;

    if(!isNumeric)
    {
        if(command=="ping")
        {
            QString text;
            text = (!trailing.isEmpty()) ? trailing : parameterList.join(" ");

            if(!trailing.isEmpty())
            {
                text = prefix + " :" + text;
            }

            if(!text.startsWith(" "))
            {
                text.prepend(' ');
            }

            // queue the reply to send it as soon as possible
            server->queue("PONG"+text, Server::HighPriority);

        }
        else if(command=="error :closing link:")
        {
            kdDebug() << "link closed" << endl;
        }
        else if(command=="pong")
        {
            // double check if we are in lag measuring mode since some servers fail to send
            // the LAG cookie back in PONG
            if(trailing.startsWith("LAG") || getLagMeasuring())
            {
                server->pongReceived();
            }
        }
        else if(command=="mode")
        {
            QStringList params=parameterList;
            if (!trailing.isEmpty())
                params << trailing;
            parseModes(prefix, params);
        }
        else if(command=="notice")
        {
            server->appendStatusMessage(i18n("Notice"),i18n("-%1- %2").arg(prefix).arg(trailing));
        }
        else if(command=="kick")
        {
            server->nickWasKickedFromChannel(parameterList[0],parameterList[1],prefix,trailing);
        }
        else if(command == "privmsg")
        {
            parsePrivMsg(prefix, parameterList, trailing);
        }
        // All yet unknown messages go into the frontmost window unaltered
        else
        {
            server->appendMessageToFrontmost(command,parameterList.join(" ")+' '+trailing);
        }
    }
    else
    {
        switch (numeric)
        {
            case RPL_WELCOME:
            case RPL_YOURHOST:
            case RPL_CREATED:
            {
                if(numeric==RPL_WELCOME)
                {
                    QString host;

                    if(trailing.contains("@"))
                        host = trailing.section("@",1);

                    // re-set nickname, since the server may have truncated it
                    if(parameterList[0]!=server->getNickname())
                        server->renameNick(server->getNickname(), parameterList[0]);

                    // Send the welcome signal, so the server class knows we are connected properly
                    emit welcome(host);
                    m_connecting = true;
                }
                server->appendStatusMessage(i18n("Welcome"),trailing);
                break;
            }
            case RPL_MYINFO:
            {
                server->appendStatusMessage(i18n("Welcome"),
                    i18n("Server %1 (Version %2), User modes: %3, Channel modes: %4")
                    .arg(parameterList[1])
                    .arg(parameterList[2])
                    .arg(parameterList[3])
                    .arg(parameterList[4])
                    );
                server->setAllowedChannelModes(parameterList[4]);
                break;
            }
            //case RPL_BOUNCE:   // RFC 1459 name, now seems to be obsoleted by ...
            case RPL_ISUPPORT:                    // ... DALnet RPL_ISUPPORT
            {
                server->appendStatusMessage(i18n("Support"),parameterList.join(" "));

                // The following behavoiur is neither documented in RFC 1459 nor in 2810-2813
                // Nowadays, most ircds send server capabilities out via 005 (BOUNCE).
                // refer to http://www.irc.org/tech_docs/005.html for a kind of documentation.
                // More on http://www.irc.org/tech_docs/draft-brocklesby-irc-isupport-03.txt

                QStringList::const_iterator it = parameterList.begin();
                // don't want the user name
                ++it;
                for (; it != parameterList.end(); ++it )
                {
                    QString property, value;
                    int pos;
                    if ((pos=(*it).find( '=' )) !=-1)
                    {
                        property = (*it).left(pos);
                        value = (*it).mid(pos+1);
                    }
                    else
                    {
                        property = *it;
                    }
                    if (property=="PREFIX")
                    {
                        pos = value.find(')',1);
                        if(pos==-1)
                        {
                            server->setPrefixes(QString(), value);
                            // XXX if ) isn't in the string, NOTHING should be there. anyone got a server
                            if (value.length() || property.length())
                                server->appendStatusMessage("","XXX Server sent bad PREFIX in RPL_ISUPPORT, please report.");
                        }
                        else
                        {
                            server->setPrefixes (value.mid(1, pos-1), value.mid(pos+1));
                        }
                    }
                    else if (property=="CHANTYPES")
                    {
                        server->setChannelTypes(value);
                    }
                    else if (property == "CAPAB")
                    {
                        // Disable as we don't use this for anything yet
                        //server->queue("CAPAB IDENTIFY-MSG");
                    }
                    else
                    {
                        //kdDebug() << "Ignored server-capability: " << property << " with value '" << value << "'" << endl;
                    }
                }                                 // endfor
                break;
            }
            case RPL_UMODEIS:
            {
                QString message=QString("%1 %2").arg(i18n("Your personal modes are:")).arg(parameterList.join(" ").section(' ',1) + ' '+trailing);
                server->appendMessageToFrontmost("Info", message);
                break;
            }
            case RPL_CHANNELMODEIS:
            {
                const QString modeString=parameterList[2];
                // This is the string the user will see
                QString modesAre;
                QString message = i18n("Channel modes: ") + modeString;

                for(unsigned int index=0;index<modeString.length();index++)
                {
                    QString parameter;
                    int parameterCount=3;
                    char mode=modeString[index];
                    if(mode!='+')
                    {
                        if(!modesAre.isEmpty())
                            modesAre+=", ";
                        if(mode=='t')
                            modesAre+=i18n("topic protection");
                        else if(mode=='n')
                            modesAre+=i18n("no messages from outside");
                        else if(mode=='s')
                            modesAre+=i18n("secret");
                        else if(mode=='i')
                            modesAre+=i18n("invite only");
                        else if(mode=='p')
                            modesAre+=i18n("private");
                        else if(mode=='m')
                            modesAre+=i18n("moderated");
                        else if(mode=='k')
                        {
                            parameter=parameterList[parameterCount++];
                            message += ' ' + parameter;
                            modesAre+=i18n("password protected");
                        }
                        else if(mode=='a')
                            modesAre+=i18n("anonymous");
                        else if(mode=='r')
                            modesAre+=i18n("server reop");
                        else if(mode=='c')
                            modesAre+=i18n("no colors allowed");
                        else if(mode=='l')
                        {
                            parameter=parameterList[parameterCount++];
                            message += ' ' + parameter;
                            modesAre+=i18n("limited to %n user", "limited to %n users", parameter.toInt());
                        }
                        else
                        {
                            modesAre+=mode;
                        }
                        server->updateChannelModeWidgets(parameterList[1],mode,parameter);
                    }
                } // endfor
                if(!modesAre.isEmpty() && Preferences::useLiteralModes())
                {
                    server->appendCommandMessageToChannel(parameterList[1],i18n("Mode"),message);
                }
                else
                {
                    server->appendCommandMessageToChannel(parameterList[1],i18n("Mode"),
                        i18n("Channel modes: ") + modesAre
                        );
                }
                break;
            }
            case RPL_CHANNELURLIS:
            {// :niven.freenode.net 328 argonel #channel :http://www.buggeroff.com/
                server->appendCommandMessageToChannel(parameterList[1], i18n("URL"),
                    i18n("Channel URL: %1").arg(trailing));
                break;
            }
            case RPL_CHANNELCREATED:
            {
                QDateTime when;
                when.setTime_t(parameterList[2].toUInt());
                server->appendCommandMessageToChannel(parameterList[1],i18n("Created"),
                    i18n("This channel was created on %1.")
                    .arg(when.toString(Qt::LocalDate))
                    );

                if(Preferences::autoWhoContinuousEnabled())
                {
                    emit endOfWho(parameterList[1]);
                }

                break;
            }
            case RPL_WHOISACCOUNT:
            {
                // Display message only if this was not an automatic request.
                if(getAutomaticRequest("WHOIS",parameterList[1])==0)
                {
                    server->appendMessageToFrontmost(i18n("Whois"),i18n("%1 is logged in as %2.").arg(parameterList[1]).arg(parameterList[2]));
                }
                break;
            }
            case RPL_NAMREPLY:
            {
                QStringList nickList;

                if(!trailing.isEmpty())
                {
                    nickList = QStringList::split(" ", trailing);
                }
                else if(parameterList.count() > 3)
                {
                    for(uint i = 3; i < parameterList.count(); i++) {
                        nickList.append(parameterList[i]);
                    }
                }
                else
                {
                    kdDebug() << "Hmm seems something is broken... can't get to the names!" << endl;
                }

                // send list to channel
                server->addPendingNickList(parameterList[2], nickList);

                // Display message only if this was not an automatic request.
                if(!getAutomaticRequest("NAMES",parameterList[2])==1)
                {
                    server->appendMessageToFrontmost(i18n("Names"),trailing);
                }
                break;
            }
            case RPL_ENDOFNAMES:
            {
                if(getAutomaticRequest("NAMES",parameterList[1])==1)
                {
                    // This code path was taken for the automatic NAMES input on JOIN, upcoming
                    // NAMES input for this channel will be manual invocations of /names
                    setAutomaticRequest("NAMES",parameterList[1],false);
                }
                else
                {
                    server->appendMessageToFrontmost(i18n("Names"),i18n("End of NAMES list."));
                }
                break;
            }
            // Topic set messages
            case RPL_NOTOPIC:
            {
                server->appendMessageToFrontmost(i18n("TOPIC"),i18n("The channel %1 has no topic set.").arg(parameterList[1]) /*.arg(parameterList[2])*/); //FIXME ok, whats the second parameter supposed to be?

                break;
            }
            case RPL_TOPIC:
            {
                QString topic = Konversation::removeIrcMarkup(trailing);

                // FIXME: This is an abuse of the automaticRequest system: We're
                // using it in an inverted manner, i.e. the automaticRequest is
                // set to true by a manual invocation of /topic. Bad bad bad -
                // needs rethinking of automaticRequest.
                if(getAutomaticRequest("TOPIC",parameterList[1])==0)
                {
                    // Update channel window
                    server->setChannelTopic(parameterList[1],topic);
                }
                else
                {
                    server->appendMessageToFrontmost(i18n("Topic"),i18n("The channel topic for %1 is: \"%2\"").arg(parameterList[1]).arg(topic));
                }

                break;
            }
            case RPL_TOPICSETBY:
            {
                // Inform user who set the topic and when
                QDateTime when;
                when.setTime_t(parameterList[3].toUInt());

                // See FIXME in RPL_TOPIC
                if(getAutomaticRequest("TOPIC",parameterList[1])==0)
                {
                    server->appendCommandMessageToChannel(parameterList[1],i18n("Topic"),
                        i18n("The topic was set by %1 on %2.")
                        .arg(parameterList[2]).arg(when.toString(Qt::LocalDate)),
                        false);
                }
                else
                {
                    server->appendMessageToFrontmost(i18n("Topic"),i18n("The topic for %1 was set by %2 on %3.")
                        .arg(parameterList[1])
                        .arg(parameterList[2])
                        .arg(when.toString(Qt::LocalDate))
                        );
                    setAutomaticRequest("TOPIC",parameterList[1],false);
                }
                emit topicAuthor(parameterList[1], parameterList[2], when);

                break;
            }
            case RPL_WHOISACTUALLY:
            {
                // Display message only if this was not an automatic request.
                if(getAutomaticRequest("WHOIS",parameterList[1])==0)
                {
                    server->appendMessageToFrontmost(i18n("Whois"),i18n("%1 is actually using the host %2.").arg(parameterList[1]).arg(parameterList[2]));
                }
                break;
            }
            case ERR_NOSUCHNICK:
            {
                // Display slightly different error message in case we performed a WHOIS for
                // IP resolve purposes, and clear it from the automaticRequest list
                if(getAutomaticRequest("DNS",parameterList[1])==0)
                {
                    server->appendMessageToFrontmost(i18n("Error"),i18n("%1: No such nick/channel.").arg(parameterList[1]));
                }
                else if(getAutomaticRequest("WHOIS",parameterList[1])==0) //Display message only if this was not an automatic request.
                {
                    server->appendMessageToFrontmost(i18n("Error"),i18n("No such nick: %1.").arg(parameterList[1]));
                    setAutomaticRequest("DNS", parameterList[1], false);
                }

                break;
            }
            case ERR_NOSUCHCHANNEL:
            {
                // Display message only if this was not an automatic request.
                if(getAutomaticRequest("WHOIS",parameterList[1])==0)
                {
                    server->appendMessageToFrontmost(i18n("Error"),i18n("%1: No such channel.").arg(parameterList[1]));
                }
                break;
            }
            // Nick already on the server, so try another one
            case ERR_NICKNAMEINUSE:
            {
                // if we are already connected, don't try tro find another nick ourselves
                if(server->isConnected())
                {                                 // Show message
                    server->appendMessageToFrontmost(i18n("Nick"),i18n("Nickname already in use, try a different one."));
                }
                else                              // not connected yet, so try to find a nick that's not in use
                {
                    // Get the next nick from the list or ask for a new one
                    QString newNick = server->getNextNickname();

                    // The user chose to disconnect
                    if (newNick.isNull())
                    {
                        server->disconnect();
                    }
                    else
                    {
                        // Update Server window
                        server->obtainNickInfo(server->getNickname()) ;
                        server->renameNick(server->getNickname(), newNick);
                        // Show message
                        server->appendMessageToFrontmost(i18n("Nick"), i18n("Nickname already in use. Trying %1.").arg(newNick));
                        // Send nickchange request to the server
                        server->queue("NICK "+newNick);
                    }
                }
                break;
            }
            case ERR_ERRONEUSNICKNAME:
            {
                if(server->isConnected())
                {                                 // We are already connected. Just print the error message
                    server->appendMessageToFrontmost(i18n("Nick"), trailing);
                }
                else                              // Find a new nick as in ERR_NICKNAMEINUSE
                {
                    QString newNick = server->getNextNickname();

                    // The user chose to disconnect
                    if (newNick.isNull())
                    {
                        server->disconnect();
                    }
                    else
                    {
                        server->obtainNickInfo(server->getNickname()) ;
                        server->renameNick(server->getNickname(), newNick);
                        server->appendMessageToFrontmost(i18n("Nick"), i18n("Erroneus nickname. Changing nick to %1." ).arg(newNick)) ;
                        server->queue("NICK "+newNick);
                    }
                }
                break;
            }
            case ERR_NOTONCHANNEL:
            {
                server->appendMessageToFrontmost(i18n("Error"),i18n("You are not on %1.").arg(parameterList[1]));

                break;
            }
            case RPL_MOTDSTART:
            {
                if(!m_connecting || !Preferences::skipMOTD())
                  server->appendStatusMessage(i18n("MOTD"),i18n("Message of the day:"));
                break;
            }
            case RPL_MOTD:
            {
                if(!m_connecting || !Preferences::skipMOTD())
                    server->appendStatusMessage(i18n("MOTD"),trailing);
                break;
            }
            case RPL_ENDOFMOTD:
            {
                if(!m_connecting || !Preferences::skipMOTD())
                    server->appendStatusMessage(i18n("MOTD"),i18n("End of message of the day"));

                if(m_connecting)
                    server->autoCommandsAndChannels();

                m_connecting = false;
                break;
            }
            case ERR_NOMOTD:
            {
                if(m_connecting)
                    server->autoCommandsAndChannels();

                m_connecting = false;
                break;
            }
            case RPL_YOUREOPER:
            {
                server->appendMessageToFrontmost(i18n("Notice"),i18n("You are now an IRC operator on this server."));

                break;
            }
            case RPL_GLOBALUSERS:                 // Current global users: 589 Max: 845
            {
                QString current(trailing.section(' ',3));
                //QString max(trailing.section(' ',5,5));
                server->appendStatusMessage(i18n("Users"),i18n("Current users on the network: %1").arg(current));
                break;
            }
            case RPL_LOCALUSERS:                  // Current local users: 589 Max: 845
            {
                QString current(trailing.section(' ',3));
                //QString max(trailing.section(' ',5,5));
                server->appendStatusMessage(i18n("Users"),i18n("Current users on %1: %2.").arg(prefix).arg(current));
                break;
            }
            case RPL_ISON:
            {
                // Tell server to start the next notify timer round
                emit notifyResponse(trailing);
                break;
            }
            case RPL_AWAY:
            {
                NickInfo* nickInfo = server->getNickInfo(parameterList[1]);
                if(nickInfo)
                {
                    nickInfo->setAway(true);
                    if( nickInfo->getAwayMessage() == trailing )
                        break;
                    nickInfo->setAwayMessage(trailing);
                }

                if(getAutomaticRequest("WHOIS",parameterList[1])==0)
                {
                    server->appendMessageToFrontmost(i18n("Away"),i18n("%1 is away: %2")
                        .arg(parameterList[1]).arg(trailing)
                        );
                }

                break;
            }
            case RPL_INVITING:
            {
                server->appendMessageToFrontmost(i18n("Invite"),
                    i18n("You invited %1 to channel %2.")
                    .arg(parameterList[1]).arg(parameterList[2])
                    );
                break;
            }
            //Sample WHOIS response
            //"/WHOIS psn"
            //[19:11] :zahn.freenode.net 311 PhantomsDad psn ~psn h106n2fls23o1068.bredband.comhem.se * :Peter Simonsson
            //[19:11] :zahn.freenode.net 319 PhantomsDad psn :#kde-devel #koffice
            //[19:11] :zahn.freenode.net 312 PhantomsDad psn irc.freenode.net :http://freenode.net/
            //[19:11] :zahn.freenode.net 301 PhantomsDad psn :away
            //[19:11] :zahn.freenode.net 320 PhantomsDad psn :is an identified user
            //[19:11] :zahn.freenode.net 317 PhantomsDad psn 4921 1074973024 :seconds idle, signon time
            //[19:11] :zahn.freenode.net 318 PhantomsDad psn :End of /WHOIS list.
            case RPL_WHOISUSER:
            {
                NickInfo* nickInfo = server->getNickInfo(parameterList[1]);
                if(nickInfo)
                {
                    nickInfo->setHostmask(i18n("%1@%2").arg(parameterList[2]).arg(parameterList[3]));
                    nickInfo->setRealName(trailing);
                }
                // Display message only if this was not an automatic request.
                if(getAutomaticRequest("WHOIS",parameterList[1])==0)
                {
                    // escape html tags
                    QString escapedRealName(trailing);
                    escapedRealName.replace("<","&lt;").replace(">","&gt;");
                    server->appendMessageToFrontmost(i18n("Whois"),
                        i18n("%1 is %2@%3 (%4)")
                        .arg(parameterList[1])
                        .arg(parameterList[2])
                        .arg(parameterList[3])
                        .arg(escapedRealName), false);   // Don't parse any urls
                }
                else
                {
                    // This WHOIS was requested by Server for DNS resolve purposes; try to resolve the host
                    if(getAutomaticRequest("DNS",parameterList[1])==1)
                    {
                        KNetwork::KResolverResults resolved = KNetwork::KResolver::resolve(parameterList[3],"");
                        if(resolved.error() == KResolver::NoError && resolved.size() > 0)
                        {
                            QString ip = resolved.first().address().nodeName();
                            server->appendMessageToFrontmost(i18n("DNS"),
                                i18n("Resolved %1 (%2) to address: %3")
                                .arg(parameterList[1])
                                .arg(parameterList[3])
                                .arg(ip)
                                );
                        }
                        else
                        {
                            server->appendMessageToFrontmost(i18n("Error"),
                                i18n("Unable to resolve address for %1 (%2)")
                                .arg(parameterList[1])
                                .arg(parameterList[3])
                                );
                        }

                        // Clear this from the automaticRequest list so it works repeatedly
                        setAutomaticRequest("DNS", parameterList[1], false);
                    }
                }
                break;
            }
            // From a WHOIS.
            //[19:11] :zahn.freenode.net 320 PhantomsDad psn :is an identified user
            case RPL_WHOISIDENTIFY:
            case RPL_IDENTIFIED:
            {
                NickInfo* nickInfo = server->getNickInfo(parameterList[1]);
                if(nickInfo)
                {
                    nickInfo->setIdentified(true);
                }
                if(getAutomaticRequest("WHOIS",parameterList[1])==0)
                {
                    // Prints "psn is an identified user"
                    //server->appendStatusMessage(i18n("Whois"),parameterList.join(" ").section(' ',1)+' '+trailing);
                    // The above line works fine, but can't be i18n'ised. So use the below instead.. I hope this is okay.
                    server->appendMessageToFrontmost(i18n("Whois"), i18n("%1 is an identified user.").arg(parameterList[1]));
                }
                break;
            }
            // Sample WHO response
            //"/WHO #lounge"
            //[21:39] [352] #lounge jasmine bots.worldforge.org irc.worldforge.org jasmine H 0 jasmine
            //[21:39] [352] #lounge ~Nottingha worldforge.org irc.worldforge.org SherwoodSpirit H 0 Arboreal Entity
            case RPL_WHOREPLY:
            {
                NickInfo* nickInfo = server->getNickInfo(parameterList[5]);
                                                  // G=away G@=away,op G+=away,voice
                bool bAway = parameterList[6].upper().startsWith("G");
                if(nickInfo)
                {
                    nickInfo->setHostmask(i18n("%1@%2").arg(parameterList[2]).arg(parameterList[3]));
                                                  //Strip off the "0 "
                    nickInfo->setRealName(trailing.section(" ", 1));
                    nickInfo->setAway(bAway);
                    if(!bAway)
                    {
                        nickInfo->setAwayMessage(QString());
                    }
                }
                // Display message only if this was not an automatic request.
                if(!whoRequestList.isEmpty())     // for safe
                {
                    if(getAutomaticRequest("WHO",whoRequestList.front())==0)
                    {
                        server->appendMessageToFrontmost(i18n("Who"),
                            i18n("%1 is %2@%3 (%4)%5").arg(parameterList[5])
                            .arg(parameterList[2])
                            .arg(parameterList[3])
                            .arg(trailing.section(" ", 1))
                            .arg(bAway?i18n(" (Away)"):QString())
                            , false); // Don't parse as url
                    }
                }
                break;
            }
            case RPL_ENDOFWHO:
            {
                if(!whoRequestList.isEmpty())
                {                                 // for safety
                    QStringList::iterator it = whoRequestList.find(parameterList[1].lower());

                    if(it != whoRequestList.end())
                    {
                        if(getAutomaticRequest("WHO", *it) == 0)
                        {
                            server->appendMessageToFrontmost(i18n("Who"),
                                i18n("End of /WHO list for %1")
                                .arg(parameterList[1]));
                        }
                        else
                        {
                            setAutomaticRequest("WHO", *it, false);
                        }

                        whoRequestList.remove(it);
                    }
                    else
                    {
                        // whoReauestList seems to be broken.
                        kdDebug()   << "InputFilter::parseServerCommand(): RPL_ENDOFWHO: malformed ENDOFWHO. retrieved: "
                            << parameterList[1] << " expected: " << whoRequestList.front()
                            << endl;
                        whoRequestList.clear();
                    }
                }
                else
                {
                    kdDebug()   << "InputFilter::parseServerCommand(): RPL_ENDOFWHO: unexpected ENDOFWHO. retrieved: "
                        << parameterList[1]
                        << endl;
                }

                emit endOfWho(parameterList[1]);
                break;
            }
            case RPL_WHOISCHANNELS:
            {
                QStringList userChannels,voiceChannels,opChannels,halfopChannels,ownerChannels,adminChannels;

                // get a list of all channels the user is in
                QStringList channelList=QStringList::split(' ',trailing);
                channelList.sort();

                // split up the list in channels where they are operator / user / voice
                for(unsigned int index=0; index < channelList.count(); index++)
                {
                    QString lookChannel=channelList[index];
                    if(lookChannel.startsWith("*") || lookChannel.startsWith("&"))
                    {
                        adminChannels.append(lookChannel.mid(1));
                        server->setChannelNick(lookChannel.mid(1), parameterList[1], 16);
                    }
                                                  // See bug #97354 part 2
                    else if((lookChannel.startsWith("!") || lookChannel.startsWith("~")) && server->isAChannel(lookChannel.mid(1)))
                    {
                        ownerChannels.append(lookChannel.mid(1));
                        server->setChannelNick(lookChannel.mid(1), parameterList[1], 8);
                    }
                                                  // See bug #97354 part 1
                    else if(lookChannel.startsWith("@+"))
                    {
                        opChannels.append(lookChannel.mid(2));
                        server->setChannelNick(lookChannel.mid(2), parameterList[1], 4);
                    }
                    else if(lookChannel.startsWith("@"))
                    {
                        opChannels.append(lookChannel.mid(1));
                        server->setChannelNick(lookChannel.mid(1), parameterList[1], 4);
                    }
                    else if(lookChannel.startsWith("%"))
                    {
                        halfopChannels.append(lookChannel.mid(1));
                        server->setChannelNick(lookChannel.mid(1), parameterList[1], 2);
                    }
                    else if(lookChannel.startsWith("+"))
                    {
                        voiceChannels.append(lookChannel.mid(1));
                        server->setChannelNick(lookChannel.mid(1), parameterList[1], 1);
                    }
                    else
                    {
                        userChannels.append(lookChannel);
                        server->setChannelNick(lookChannel, parameterList[1], 0);
                    }
                }                                 // endfor
                // Display message only if this was not an automatic request.
                if(getAutomaticRequest("WHOIS",parameterList[1])==0)
                {
                    if(userChannels.count())
                    {
                        server->appendMessageToFrontmost(i18n("Whois"),
                            i18n("%1 is a user on channels: %2")
                            .arg(parameterList[1])
                            .arg(userChannels.join(" "))
                            );
                    }
                    if(voiceChannels.count())
                    {
                        server->appendMessageToFrontmost(i18n("Whois"),
                            i18n("%1 has voice on channels: %2")
                            .arg(parameterList[1]).arg(voiceChannels.join(" "))
                            );
                    }
                    if(halfopChannels.count())
                    {
                        server->appendMessageToFrontmost(i18n("Whois"),
                            i18n("%1 is a halfop on channels: %2")
                            .arg(parameterList[1]).arg(halfopChannels.join(" "))
                            );
                    }
                    if(opChannels.count())
                    {
                        server->appendMessageToFrontmost(i18n("Whois"),
                            i18n("%1 is an operator on channels: %2")
                            .arg(parameterList[1]).arg(opChannels.join(" "))
                            );
                    }
                    if(ownerChannels.count())
                    {
                        server->appendMessageToFrontmost(i18n("Whois"),
                            i18n("%1 is owner of channels: %2")
                            .arg(parameterList[1]).arg(ownerChannels.join(" "))
                            );
                    }
                    if(adminChannels.count())
                    {
                        server->appendMessageToFrontmost(i18n("Whois"),
                            i18n("%1 is admin on channels: %2")
                            .arg(parameterList[1]).arg(adminChannels.join(" "))
                            );
                    }
                }
                break;
            }
            case RPL_WHOISSERVER:
            {
                NickInfo* nickInfo = server->getNickInfo(parameterList[1]);
                if(nickInfo)
                {
                    nickInfo->setNetServer(parameterList[2]);
                    nickInfo->setNetServerInfo(trailing);
                    // Clear the away state on assumption that if nick is away, this message will be followed
                    // by a 301 RPL_AWAY message.  Not necessary a invalid assumption, but what can we do?
                    nickInfo->setAway(false);
                    nickInfo->setAwayMessage(QString());
                }
                // Display message only if this was not an automatic request.
                if(getAutomaticRequest("WHOIS",parameterList[1])==0)
                {
                    server->appendMessageToFrontmost(i18n("Whois"),
                        i18n("%1 is online via %2 (%3).").arg(parameterList[1])
                        .arg(parameterList[2]).arg(trailing)
                        );
                }
                break;
            }
            case RPL_WHOISHELPER:
            {
                // Display message only if this was not an automatic request.
                if(getAutomaticRequest("WHOIS",parameterList[1])==0)
                {
                    server->appendMessageToFrontmost(i18n("Whois"),
                        i18n("%1 is available for help.")
                        .arg(parameterList[1])
                        );
                }
                break;
            }
            case RPL_WHOISOPERATOR:
            {
                // Display message only if this was not an automatic request.
                if(getAutomaticRequest("WHOIS",parameterList[1])==0)
                {
                    if (trailing.lower().simplifyWhiteSpace().startsWith("is an irc operator"))
                        server->appendMessageToFrontmost(i18n("Whois"),i18n("%1 is an IRC Operator.").arg(parameterList[1]));
                    else
                        server->appendMessageToFrontmost(i18n("Whois"),QString("%1 %2").arg(parameterList[1]).arg(trailing));
                }
                break;
            }
            case RPL_WHOISIDLE:
            {
                // get idle time in seconds
                long seconds=parameterList[2].toLong();
                long minutes=seconds/60;
                long hours  =minutes/60;
                long days   =hours/24;

                // if idle time is longer than a day
                // Display message only if this was not an automatic request.
                if(getAutomaticRequest("WHOIS",parameterList[1])==0)
                {
                    if(days)
                    {
                        const QString daysString = i18n("1 day", "%n days", days);
                        const QString hoursString = i18n("1 hour", "%n hours", (hours % 24));
                        const QString minutesString = i18n("1 minute", "%n minutes", (minutes % 60));
                        const QString secondsString = i18n("1 second", "%n seconds", (seconds % 60));

                        server->appendMessageToFrontmost(i18n("Whois"),
                            i18n("%1 = name of person, %2 = (x days), %3 = (x hours), %4 = (x minutes), %5 = (x seconds)",
                            "%1 has been idle for %2, %3, %4, and %5.")
                            .arg(parameterList[1])
                            .arg(daysString).arg(hoursString).arg(minutesString).arg(secondsString)
                            );
                        // or longer than an hour
                    }
                    else if(hours)
                    {
                        const QString hoursString = i18n("1 hour", "%n hours", hours);
                        const QString minutesString = i18n("1 minute", "%n minutes", (minutes % 60));
                        const QString secondsString = i18n("1 second", "%n seconds", (seconds % 60));
                        server->appendMessageToFrontmost(i18n("Whois"),
                            i18n("%1 = name of person, %2 = (x hours), %3 = (x minutes), %4 = (x seconds)",
                            "%1 has been idle for %2, %3, and %4.")
                            .arg(parameterList[1])
                            .arg(hoursString).arg(minutesString).arg(secondsString)
                            );
                        // or longer than a minute
                    }
                    else if(minutes)
                    {
                        const QString minutesString = i18n("1 minute", "%n minutes", minutes);
                        const QString secondsString = i18n("1 second", "%n seconds", (seconds % 60));
                        server->appendMessageToFrontmost(i18n("Whois"),
                            i18n("%1 = name of person, %2 = (x minutes), %3 = (x seconds)",
                            "%1 has been idle for %2 and %3.")
                            .arg(parameterList[1])
                            .arg(minutesString).arg(secondsString)
                            );
                        // or just some seconds
                    }
                    else
                    {
                        server->appendMessageToFrontmost(i18n("Whois"),
                            i18n("%1 has been idle for 1 second.", "%1 has been idle for %n seconds.", seconds)
                            .arg(parameterList[1])
                            );
                    }
                }

                if(parameterList.count()==4)
                {
                    QDateTime when;
                    when.setTime_t(parameterList[3].toUInt());
                    NickInfo* nickInfo = server->getNickInfo(parameterList[1]);
                    if(nickInfo)
                    {
                        nickInfo->setOnlineSince(when);
                    }
                    // Display message only if this was not an automatic request.
                    if(getAutomaticRequest("WHOIS",parameterList[1])==0)
                    {
                        server->appendMessageToFrontmost(i18n("Whois"),
                            i18n("%1 has been online since %2.")
                            .arg(parameterList[1]).arg(when.toString(Qt::LocalDate))
                            );
                    }
                }
                break;
            }
            case RPL_ENDOFWHOIS:
            {
                //NickInfo* nickInfo = server->getNickInfo(parameterList[1]);
                // Display message only if this was not an automatic request.
                if(getAutomaticRequest("WHOIS",parameterList[1])==0)
                {
                    server->appendMessageToFrontmost(i18n("Whois"),i18n("End of WHOIS list."));
                }
                // was this an automatic request?
                if(getAutomaticRequest("WHOIS",parameterList[1])!=0)
                {
                    setAutomaticRequest("WHOIS",parameterList[1],false);
                }
                break;
            }
            case RPL_USERHOST:
            {
                // iterate over all nick/masks in reply
                QStringList uhosts=QStringList::split(" ",trailing);

                for(unsigned int index=0;index<uhosts.count();index++)
                {
                    // extract nickname and hostmask from reply
                    QString nick(uhosts[index].section('=',0,0));
                    QString mask(uhosts[index].section('=',1));

                    // get away and IRC operator flags
                    bool away=(mask[0]=='-');
                    bool ircOp=(nick[nick.length()-1]=='*');

                    // cut flags from nick/hostmask
                    mask=mask.mid(1);
                    if(ircOp)
                    {
                        nick=nick.left(nick.length()-1);
                    }

                    // inform server of this user's data
                    emit userhost(nick,mask,away,ircOp);

                    // display message only if this was no automatic request
                    if(getAutomaticRequest("USERHOST",nick)==0)
                    {
                        server->appendMessageToFrontmost(i18n("Userhost"),
                            i18n("%1 = nick, %2 = shows if nick is op, %3 = hostmask, %4 = shows away", "%1%2 is %3%4.")
                            .arg(nick)
                            .arg((ircOp) ? i18n(" (IRC Operator)") : QString())
                            .arg(mask)
                            .arg((away) ? i18n(" (away)") : QString()));
                    }

                    // was this an automatic request?
                    if(getAutomaticRequest("USERHOST",nick)!=0)
                    {
                        setAutomaticRequest("USERHOST",nick,false);
                    }
                }                                 // for
                break;
            }
            case RPL_LISTSTART:                   //FIXME This reply is obsolete!!!
            {
                if(getAutomaticRequest("LIST",QString())==0)
                {
                    server->appendMessageToFrontmost(i18n("List"),i18n("List of channels:"));
                }
                break;
            }
            case RPL_LIST:
            {
                if(getAutomaticRequest("LIST",QString())==0)
                {
                    QString message;
                    message=i18n("%1 (%n user): %2", "%1 (%n users): %2", parameterList[2].toInt());
                    server->appendMessageToFrontmost(i18n("List"),message.arg(parameterList[1]).arg(trailing));
                }
                else                              // send them to /LIST window
                {
                    emit addToChannelList(parameterList[1],parameterList[2].toInt(),trailing);
                }

                break;
            }
            case RPL_LISTEND:
            {
                // was this an automatic request?
                if(getAutomaticRequest("LIST",QString())==0)
                {
                    server->appendMessageToFrontmost(i18n("List"),i18n("End of channel list."));
                }
                else
                {
                    setAutomaticRequest("LIST",QString(),false);
                }
                break;
            }
            case RPL_NOWAWAY:
            {
                NickInfo* nickInfo = server->getNickInfo(parameterList[0]);
                if (nickInfo) nickInfo->setAway(true);

                server->setAway(true);

                break;
            }
            case RPL_UNAWAY:
            {
                NickInfo* nickInfo = server->getNickInfo(parameterList[0]);

                if (nickInfo)
                {
                    nickInfo->setAway(false);
                    nickInfo->setAwayMessage(QString());
                }

                server->setAway(false);

                break;
            }
            case RPL_BANLIST:
            {
                if (getAutomaticRequest("BANLIST", parameterList[1]))
                {
                    server->addBan(parameterList[1], parameterList.join(" ").section(' ', 2, 4));
                } else {
                    QDateTime when;
                    when.setTime_t(parameterList[4].toUInt());

                    server->appendMessageToFrontmost(i18n("BanList:%1").arg(parameterList[1]), i18n("BanList message: e.g. *!*@aol.com set by MrGrim on <date>", "%1 set by %2 on %3").arg(parameterList[2]).arg(parameterList[3].section('!', 0, 0)).arg(when.toString(Qt::LocalDate)));
                }
                break;
            }
            case RPL_ENDOFBANLIST:
            {
                if (getAutomaticRequest("BANLIST", parameterList[1]))
                {
                    setAutomaticRequest("BANLIST", parameterList[1], false);
                } else {
                    server->appendMessageToFrontmost(i18n("BanList:%1").arg(parameterList[1]), i18n("End of Ban List."));
                }
                break;
            }
            case ERR_NOCHANMODES:
            {
                ChatWindow *chatwindow = server->getChannelByName(parameterList[1]);
                if(chatwindow)
                {
                    chatwindow->appendServerMessage(i18n("Channel"), trailing);
                }
                else                              // We couldn't join the channel , so print the error. with [#channel] : <Error Message>
                {
                    server->appendMessageToFrontmost(i18n("Channel"), trailing);
                }
                break;
            }
            case ERR_NOSUCHSERVER:
            {
                //Some servers don't know their name, so they return an error instead of the PING data
                if (getLagMeasuring() && trailing.startsWith(prefix))
                {
                    server->pongReceived();
                }
                break;
            }
            case ERR_UNAVAILRESOURCE:
            {
                server->appendMessageToFrontmost(i18n("Error"),i18n("%1 is currently unavailable.").arg(parameterList[1]));

                break;
            }
            case RPL_HIGHCONNECTCOUNT:
            case RPL_LUSERCLIENT:
            case RPL_LUSEROP:
            case RPL_LUSERUNKNOWN:
            case RPL_LUSERCHANNELS:
            case RPL_LUSERME:
            {
                server->appendStatusMessage(i18n("Users"), parameterList.join(" ").section(' ',1) + ' '+trailing);
                break;
            }
            case ERR_UNKNOWNCOMMAND:
            {
                server->appendMessageToFrontmost(i18n("Error"),i18n("%1: Unknown command.").arg(parameterList[1]));

                break;
            }
            case ERR_NOTREGISTERED:
            {
                server->appendMessageToFrontmost(i18n("Error"),i18n("Not registered."));

                break;
            }
            case ERR_NEEDMOREPARAMS:
            {
                server->appendMessageToFrontmost(i18n("Error"),i18n("%1: This command requires more parameters.").arg(parameterList[1]));

                break;
            }
            case RPL_CAPAB: // Special freenode reply afaik
            {
                // Disable as we don't use this for anything yet
                if(trailing.contains("IDENTIFY-MSG"))
                {
                    server->enableIdentifyMsg(true);
                    break;
                }

            /* don't break; - this is also used as RPL_DATASTR on ircu and some others */
            }
            // FALLTHROUGH to default to let the error display otherwise
            default:
            {
                // All yet unknown messages go into the frontmost window without the
                // preceding nickname
                server->appendMessageToFrontmost(command, parameterList.join(" ").section(' ',1) + ' '+trailing);
            }
        }                                         // end of numeric switch
    }
}

void InputFilter::parseModes(const QString &sourceNick, const QStringList &parameterList)
{
    const QString modestring=parameterList[1];

    if (!isAChannel(parameterList[0]))
    {
        QString message;
        if (parameterList[0] == server->getNickname())
        {
            if (sourceNick == server->getNickname())
            {
                //XXX someone might care about the potentially unnecessary plural here
                message = i18n("You have set personal modes: ") + modestring;
            }
            else
            {   //XXX someone might care about the potentially unnecessary plural here
                message = QString("%1 %2 %3").arg(sourceNick).arg(i18n("has changed your personal modes:")).arg(modestring);
            }
        }
        if (!message.isEmpty())
            server->appendStatusMessage(i18n("Mode"), message);
        return;
    }

    bool plus=false;
    int parameterIndex=0;
    // List of modes that need a parameter (note exception with -k and -l)
    // Mode q is quiet on freenode and acts like b... if this is a channel mode on other
    //  networks then more logic is needed here. --MrGrim
    QString parameterModes="aAoOvhkbleIq";
    QString message = sourceNick + i18n(" sets mode: ") + modestring;

    for(unsigned int index=0;index<modestring.length();index++)
    {
        unsigned char mode=modestring[index];
        QString parameter;

        // Check if this is a mode or a +/- qualifier
        if(mode=='+' || mode=='-')
        {
            plus=(mode=='+');
        }
        else
        {
            // Check if this was a parameter mode
            if(parameterModes.find(mode)!=-1)
            {
                // Check if the mode actually wants a parameter. -k and -l do not!
                if(plus || (!plus && (mode!='k') && (mode!='l')))
                {
                    // Remember the mode parameter
                    parameter=parameterList[2+parameterIndex];
                    message += ' ' + parameter;
                    // Switch to next parameter
                    ++parameterIndex;
                }
            }
            // Let the channel update its modes
            if(parameter.isEmpty())               // XXX Check this to ensure the braces are in the correct place
            {
                kdDebug()   << "in updateChannelMode.  sourceNick: '" << sourceNick << "'  parameterlist: '"
                    << parameterList.join(", ") << "'"
                    << endl;
            }
            server->updateChannelMode(sourceNick,parameterList[0],mode,plus,parameter);
        }
    }                                             // endfor

    if (Preferences::useLiteralModes())
    {
        server->appendCommandMessageToChannel(parameterList[0],i18n("Mode"),message);
    }
}

// # & + and ! are *often*, but not necessarily, Channel identifiers. + and ! are non-RFC,
// so if a server doesn't offer 005 and supports + and ! channels, I think thats broken behaviour
// on their part - not ours. --Argonel
bool InputFilter::isAChannel(const QString &check)
{
    Q_ASSERT(server);
    // if we ever see the assert, we need the ternary
    return server? server->isAChannel(check) : QString("#&").contains(check.at(0));
}

bool InputFilter::isIgnore(const QString &sender, Ignore::Type type)
{
    bool doIgnore = false;

    QPtrList<Ignore> list = Preferences::ignoreList();

    for(unsigned int index =0; index<list.count(); index++)
    {
        Ignore* item = list.at(index);
        QRegExp ignoreItem(QRegExp::escape(item->getName()).replace("\\*", "(.*)"),false);
        if (ignoreItem.exactMatch(sender) && (item->getFlags() & type))
            doIgnore = true;
        if (ignoreItem.exactMatch(sender) && (item->getFlags() & Ignore::Exception))
            return false;
    }

    return doIgnore;
}

void InputFilter::reset()
{
    automaticRequest.clear();
    whoRequestList.clear();
}

void InputFilter::setAutomaticRequest(const QString& command, const QString& name, bool yes)
{
    automaticRequest[command][name.lower()] += (yes) ? 1 : -1;
    if(automaticRequest[command][name.lower()]<0)
    {
        kdDebug()   << "InputFilter::automaticRequest( " << command << ", " << name
            << " ) was negative! Resetting!"
            << endl;
        automaticRequest[command][name.lower()]=0;
    }
}

int InputFilter::getAutomaticRequest(const QString& command, const QString& name)
{
    return automaticRequest[command][name.lower()];
}

void InputFilter::addWhoRequest(const QString& name) { whoRequestList << name.lower(); }

bool InputFilter::isWhoRequestUnderProcess(const QString& name) { return (whoRequestList.contains(name.lower())>0); }

void InputFilter::setLagMeasuring(bool state) { lagMeasuring=state; }

bool InputFilter::getLagMeasuring()           { return lagMeasuring; }

void InputFilter::parsePrivMsg(const QString& prefix,
                               const QStringList& parameterList,
                               const QString& trailing)
{
    int pos = prefix.find("!");
    QString source;
    QString sourceHostmask;
    QString message(trailing);

    if(pos > 0)
    {
        source = prefix.left(pos);
        sourceHostmask = prefix.mid(pos + 1);
    }
    else
    {
        source = prefix;
    }

    KonversationApplication* konv_app = static_cast<KonversationApplication*>(kapp);
    message = konv_app->doAutoreplace(message, false);

    if(isAChannel(parameterList[0]))
    {
        if(!isIgnore(prefix, Ignore::Channel))
        {
            Channel* channel = server->getChannelByName(parameterList[0]);
            if(channel)
            {
                channel->append(source, message);

                if(source != server->getNickname())
                {
                    QRegExp regexp("(^|[^\\d\\w])" +
                            QRegExp::escape(server->loweredNickname()) +
                            "([^\\d\\w]|$)");
                    regexp.setCaseSensitive(false);
                    if(message.find(regexp) !=-1 )
                    {
                        konv_app->notificationHandler()->nick(channel,
                                source, message);
                    }
                    else
                    {
                        konv_app->notificationHandler()->message(channel,
                                source, message);
                    }
                }
            }
        }
    }
    else
    {
        if(!isIgnore(prefix,Ignore::Query))
        {
            NickInfoPtr nickinfo = server->obtainNickInfo(source);
            nickinfo->setHostmask(sourceHostmask);

            // Create a new query (server will check for dupes)
            query = server->addQuery(nickinfo, false /*we didn't initiate this*/ );

            // send action to query
            query->appendQuery(source, message);

            if(source != server->getNickname() && query)
            {
                QRegExp regexp("(^|[^\\d\\w])" +
                        QRegExp::escape(server->loweredNickname()) +
                        "([^\\d\\w]|$)");
                regexp.setCaseSensitive(false);
                if(message.find(regexp) !=-1 )
                {
                    konv_app->notificationHandler()->nick(query,
                            source, message);
                }
                else
                {
                    konv_app->notificationHandler()->queryMessage(query,
                            source, message);
                }
            }
        }
    }
}

#include "inputfilter.moc"

// kate: space-indent on; tab-width 4; indent-width 4; mixed-indent off; replace-tabs on;
// vim: set et sw=4 ts=4 cino=l1,cs,U1:
