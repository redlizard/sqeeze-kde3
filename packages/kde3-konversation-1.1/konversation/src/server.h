/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

/*
  Copyright (C) 2002 Dario Abatianni <eisfuchs@tigress.com>
  Copyright (C) 2005 Ismail Donmez <ismail@kde.org>
  Copyright (C) 2005-2006 Peter Simonsson <psn@linux.se>
  Copyright (C) 2006-2008 Eli J. MacKenzie <argonel at gmail.com>
  Copyright (C) 2005-2008 Eike Hein <hein@kde.org>
*/

#ifndef SERVER_H
#define SERVER_H

#include "common.h"
#include "channelnick.h"
#include "inputfilter.h"
#include "outputfilter.h"
#include "nickinfo.h"
#include "sslsocket.h"
#include "serversettings.h"
#include "servergroupsettings.h"
#include "connectionsettings.h"

#include <qtimer.h>
#include <qvaluevector.h>
#include <qguardedptr.h>

#include <ksharedptr.h>
#include <kprocess.h>
#include <ksocketbase.h>
#include <kbufferedsocket.h>
#include <kstreamsocket.h>


class Channel;
class DccTransfer;
class DccTransferPanelItem;
class Query;
class StatusPanel;
class Identity;
class RawLog;
class ChannelListPanel;
class ScriptLauncher;
class ServerISON;
class QStrList;
class ChatWindow;
class ViewContainer;

using namespace KNetwork;

class IRCQueue;

class Server : public QObject
{
    Q_OBJECT
    friend class IRCQueue;
    friend class QueueTuner;
void resetNickSelection();
    public:
        enum QueuePriority
        {
            LowPriority,      ///<slow queue, for automatic info gathering
            StandardPriority, ///<regular queue, for chat and user initiated commands
            HighPriority,     ///<for pongs and quits

            Howmanyqueuesdoweneedanywayquestionmark,

            HiPriority=HighPriority,
            LoPriority=LowPriority,
            NormalPriorty=StandardPriority,
            RegularPriority=StandardPriority
        };

        Server(QObject* parent, ConnectionSettings& settings);
        ~Server();

        int connectionId() { return m_connectionId; }

        ConnectionSettings& getConnectionSettings() { return m_connectionSettings; }
        void setConnectionSettings(ConnectionSettings& settings) { m_connectionSettings = settings; }

        QString getDisplayName() { return m_connectionSettings.name(); }
        QString getServerName() { return m_connectionSettings.server().host(); }

        Konversation::ServerGroupSettingsPtr getServerGroup() { return m_connectionSettings.serverGroup(); }
        IdentityPtr getIdentity() { return m_connectionSettings.identity(); }

        Konversation::ConnectionState getConnectionState() { return m_connectionState; }

        bool isConnected() { return (m_connectionState == Konversation::SSConnected); }
        bool isConnecting() { return (m_connectionState == Konversation::SSConnecting); }

        bool getUseSSL() const;
        QString getSSLInfo() const;
        int getPort();
        int getLag() const;

        void updateAutoJoin(Konversation::ChannelSettings channel = Konversation::ChannelSettings());

        void resetNickList(const QString& channelName);
        void addPendingNickList(const QString& channelName,const QStringList& nickList);
        void addHostmaskToNick(const QString &sourceNick, const QString &sourceHostmask);
        Channel* nickJoinsChannel(const QString &channelName, const QString &nickname, const QString &hostmask);
        void renameNick(const QString &nickname,const QString &newNick);
        Channel* removeNickFromChannel(const QString &channelName, const QString &nickname, const QString &reason, bool quit=false);
        void nickWasKickedFromChannel(const QString &channelName, const QString &nickname, const QString &kicker, const QString &reason);
        void removeNickFromServer(const QString &nickname, const QString &reason);

        void setChannelTypes(const QString &types);
        QString getChannelTypes() const;

        // extended user modes support
        void setPrefixes(const QString &modes, const QString& prefixes);
        void mangleNicknameWithModes(QString &nickname,bool& isAdmin,bool& isOwner,bool &isOp,
            bool& isHalfop,bool &hasVoice);

        bool isAChannel(const QString &channel) const;
        bool isNickname(const QString& compare) const;

        QString getNickname() const;
        QString loweredNickname() const;

        QString getNextNickname();

        InputFilter* getInputFilter() { return &m_inputFilter; }
        Konversation::OutputFilter* getOutputFilter() { return m_outputFilter; };

        void joinChannel(const QString& name, const QString& hostmask);
        void removeChannel(Channel* channel);
        void appendServerMessageToChannel(const QString& channel, const QString& type, const QString& message);
        void appendCommandMessageToChannel(const QString& channel, const QString& command, const QString& message, bool highlight = true);
        void appendStatusMessage(const QString& type,const QString& message);
        void appendMessageToFrontmost(const QString& type,const QString& message, bool parseURL = true);

        int getPreLength(const QString& command, const QString& dest);

        void dcopRaw(const QString& command);
        void dcopSay(const QString& target,const QString& command);
        void dcopInfo(const QString& string);
        void ctcpReply(const QString& receiver, const QString& text);

        void setChannelTopic(const QString& channel, const QString& topic);
                                                  // Overloaded
        void setChannelTopic(const QString& nickname, const QString& channel, const QString& topic);
        void updateChannelMode(const QString& nick, const QString& channel, char mode, bool plus, const QString& parameter);
        void updateChannelModeWidgets(const QString& channel, char mode, const QString& parameter);

        Channel* getChannelByName(const QString& name);
        Query* getQueryByName(const QString& name);
        QString parseWildcards(const QString& toParse, const QString& nickname, const QString& channelName, const QString &channelKey, const QStringList &nickList, const QString& parameter);
        QString parseWildcards(const QString& toParse, const QString& nickname, const QString& channelName, const QString &channelKey, const QString& nick, const QString& parameter);

        void autoCommandsAndChannels();

        void sendURIs(const QStrList& uris, const QString& nick);

        void notifyAction(const QString& nick);
        ChannelListPanel* getChannelListPanel() const;

        StatusPanel* getStatusView() const { return m_statusView; }
        virtual bool closeYourself(bool askForConfirmation=true);

        QString getOwnIpByNetworkInterface();
        QString getOwnIpByServerMessage();

        void requestAway(const QString& reason = "");
        void requestUnaway();

        bool isAway() { return m_away; }
        void setAway(bool away);
        QString awayTime() const;

        void setAwayReason(const QString& reason) { m_awayReason = reason; }

        /**
         * Returns true if the given nickname is known to be online.
         * @param nickname      The nickname.  Case insensitive.
         * @return              True if the nickname is known to be online by the server.
         * Note that a nick that is not in any of the joined channels and is not on the
         * notify list, and has not initiated a query with you, may well be online,
         * but server doesn't know if it is or not, in which case False is returned.
         */
        bool isNickOnline(const QString &nickname);
        /** Given a nickname, returns NickInfo object.
         *  @param nickname    The desired nickname.  Case insensitive.
         *  @return            Pointer to the nickinfo for this nickname if one exists.
         *                     0 if not known to be online.
         *
         *  A NickInfo pointer will only be returned if the nickname is known to the Konvi
         *  Server object.  A nick will be known if:
         *  - It is in one of the server's channels user has joined.
         *  - It is on the notify list and is known to be online.
         *  - The nick initiated a query with the user.
         *  A NickInfo is destroyed when it is offline.
         */
        NickInfoPtr getNickInfo(const QString& nickname);
        /** Given a nickname, returns an existing NickInfo object, or creates a new NickInfo object.
         *  Guaranteed to return a nickinfo.
         *  @param nickname    The desired nickname.  Case sensitive.
         *  @return            Pointer to the found or created NickInfo object.
         */
        NickInfoPtr obtainNickInfo(const QString& nickname);
        /** Returns a list of all the NickInfos that are online and known to the server.
         * Caller should not modify the list.
         * A nick will be known if:
         *  - It is in one of the server's channels user has joined.
         *  - It is on the notify list and is known to be online.
         *  - The nick initiated a query with the user.
         *
         * @return A QMap of KSharedPtrs to NickInfos indexed by lowercase nickname.
         */
        const NickInfoMap* getAllNicks();
        /** Returns the list of members for a channel in the joinedChannels list.
         *  A joinedChannel is one that you are in, as opposed to a channel that you aren't in,
         *  but one of your watched nicks is in.
         *  Code that calls this must not modify the list.
         *  @param channelName Name of desired channel.  Case insensitive.
         *  @return            A map of all the nicks in the channel.
         *                     0 if channel is not in the joinedChannels list.
         */
        const ChannelNickMap *getJoinedChannelMembers(const QString& channelName) const;
        /** Returns the list of members for a channel in the unjoinedChannels list.
         *  An unjoinedChannel is a channel you aren't in.  As such, this is only going to return
         *  nicks that you know are in that channel because a /whois has been done against them.
         *  This could be done automatically if they are on the watch list.
         *  Code that calls this must not modify the list.
         *  @param channelName Name of desired channel.  Case insensitive.
         *  @return            A map of only the nicks that we know that are in the channel.
         *                     0 if channel is not in the unjoinedChannels list.
         */
        const ChannelNickMap *getUnjoinedChannelMembers(const QString& channelName) const;
        /** Searches the Joined and Unjoined lists for the given channel and returns the member list.
         *  Code that calls this must not modify the list.
         *  @param channelName Name of desired channel.  Case insensitive.
         *  @return            A map of nicks in that channel.  0 if channel is not in either list.
         *
         *  @see getJoinedChannelMembers(const QString& channelName)
         *  @see getUnjoinedChannelMembers(const QString& channelName)
         */
        const ChannelNickMap *getChannelMembers(const QString& channelName) const;
        /** Returns a list of all the joined channels that a nick is in.
         *  @param nickname    The desired nickname.  Case insensitive.
         *  @return            A list of joined channels the nick is in.  Empty if none.
         */
        QStringList getNickJoinedChannels(const QString& nickname);
        /** Returns a list of all the channels (joined or unjoined) that a nick is in.
         *  @param nickname    The desired nickname.  Case insensitive.
         *  @return            A list of channels the nick is in.  Empty if none.
         *
         *  A nick will not appear in the Unjoined channels list unless a WHOIS
         *  has been performed on it.
         */
        QStringList getNickChannels(const QString& nickname);
        /** Returns pointer to the ChannelNick (mode and pointer to NickInfo) for a
         *  given channel and nickname.
         *  @param channelName The desired channel name.  Case insensitive.
         *  @param nickname    The desired nickname.  Case insensitive.
         *  @return            Pointer to ChannelNick structure containing a pointer
         *                     to the NickInfo and the mode of the nick in the channel.
         *                     0 if not found.
         */
        ChannelNickPtr getChannelNick(const QString& channelName, const QString& nickname);
        /** Updates a nickname in a channel.  If not on the joined or unjoined lists, and nick
         *  is in the watch list, adds the channel and nick to the unjoinedChannels list.
         *  If mode != 99, sets the mode for the nick in the channel.
         *  Returns the NickInfo object if nick is on any lists, otherwise 0.
         *  @param channelName The channel name.  Case sensitive.
         *  @param nickname    The nickname.  Case sensitive.
         *  @param mode        Bit mask containing the modes the nick has in the channel,
         *                     or 99 if not known.  See channelnick.cpp for bit definitions.
         */
        ChannelNickPtr setChannelNick(const QString& channelName, const QString& nickname, unsigned int mode = 99);
        /**
         * Given the nickname of nick that is offline (or at least not known to be online),
         * returns the addressbook entry (if any) for the nick.
         * @param nickname       Desired nickname.  Case insensitive.
         * @return               Addressbook entry of the nick or empty if not found.
         */
        KABC::Addressee getOfflineNickAddressee(QString& nickname);

        /**
         * Returns a QPtrList of all channels
         */
        const QPtrList<Channel>& getChannelList() const { return m_channelList; }

        void emitChannelNickChanged(const ChannelNickPtr channelNick);
        void emitNickInfoChanged(const NickInfoPtr nickInfo);

        /**
         * Returns a list of all the nicks on the user watch list plus nicks in the addressbook.
         */
        QStringList getWatchList();
        QString getWatchListString();
        /**
         * Return true if the given nickname is on the watch list.
         */
        bool isWatchedNick(const QString& nickname);
        /**
         * Returns a list of all the nicks on the watch list that are not in joined
         * channels.  ISON command is sent for these nicks.
         */
        QStringList getISONList();
        QString getISONListString();

        ViewContainer* getViewContainer() const;

        /** Adds a nickname to the joinedChannels list.
         *  Creates new NickInfo if necessary.
         *  If needed, moves the channel from the unjoined list to the joined list.
         *  If needed, moves the nickname from the Offline to Online lists.
         *  If mode != 99 sets the mode for this nick in this channel.
         *  @param channelName The channel name.  Case sensitive.
         *  @param nickname    The nickname.  Case sensitive.
         *  @return            The NickInfo for the nickname.
         */
        ChannelNickPtr addNickToJoinedChannelsList(const QString& channelName, const QString& nickname);

        void setAllowedChannelModes(const QString& modes) { m_allowedChannelModes = modes; }
        QString allowedChannelModes() const { return m_allowedChannelModes; }

        void registerWithServices();

        // Blowfish stuff
        QCString getKeyForRecipient(const QString& recipient) const;
        void setKeyForRecipient(const QString& recipient, const QCString& key);

        bool identifyMsg() const { return m_identifyMsg; }

        ChannelListPanel* addChannelListPanel();

        // invoked by DccTransferSend
        void dccSendRequest(const QString& recipient,const QString& fileName,const QString& address,const QString& port,unsigned long size);
        void dccPassiveSendRequest(const QString& recipient,const QString& fileName,const QString& address,unsigned long size,const QString& token);
        // invoked by DccTransferRecv
        void dccResumeGetRequest(const QString& sender,const QString& fileName,const QString& port,KIO::filesize_t startAt);
        void dccReverseSendAck(const QString& partnerNick,const QString& fileName,const QString& ownAddress,const QString& ownPort,unsigned long size,const QString& reverseToken);

    // IRCQueueManager
        bool validQueue(QueuePriority priority); ///< is this queue index valid?
        void resetQueues(); ///< Tell all of the queues to reset
        static int _max_queue() { return Howmanyqueuesdoweneedanywayquestionmark-1; }

        /** Forces the queued data to be sent in sequence of age, without pause.

            This could flood you off but since you're quitting, we probably don't care. This is done
            here instead of in the queues themselves so we can interleave the queues without having to
            zip the queues together. If you want to quit the server normally without sending, reset the queues
            first.
         */
        void flushQueues();

        //These are really only here to limit where ircqueue.h is included
        static void _fetchRates(); ///< on server construction
        static void _stashRates(); ///< on application exit
        static void _resetRates(); ///< when QueueTuner says to


    signals:
        void destroyed(int connectionId);
        void nicknameChanged(const QString&);
        void serverLag(Server* server,int msec);  /// will be connected to KonversationMainWindow::updateLag()
        void tooLongLag(Server* server, int msec);/// will be connected to KonversationMainWindow::updateLag()
        void resetLag(); ///< will be emitted when new 303 came in
        void nicksNowOnline(Server* server,const QStringList& list,bool changed);
        void awayState(bool away);                /// will be connected to any user input panel;
        void multiServerCommand(const QString& command, const QString& parameter);

        /**
         * Emitted when the server gains/loses connection.
         * Will be connected to all server dependant tabs.
         */
        void serverOnline(bool state);

        /**
         * Emitted every time something gets sent.
         *
         *  @param  bytes           The count of bytes sent to the server, before re-encoding.
         *  @param  encodedBytes    The count of bytes sent to the server after re-encoding.
         */
        void sentStat(int bytes, int encodedBytes, IRCQueue *whichQueue);
        //FIXME can anyone who can connect to a Server signal not know about an IRCQueue?
        void sentStat(int bytes, int encodedBytes);

        //Note that these signals haven't been implemented yet.
        /// Fires when the information in a NickInfo object changes.
        void nickInfoChanged(Server* server, const NickInfoPtr nickInfo);

        /// Fires when the mode of a nick in a channel changes.
        void channelNickChanged(Server* server, const ChannelNickPtr channelNick);

        /// Fires when a nick leaves or joins a channel.  Based on joined flag, receiver could
        /// call getJoinedChannelMembers or getUnjoinedChannelMembers, or just
        /// getChannelMembers to get a list of all the nicks now in the channel.
        /// parted indicates whether the nick joined or left the channel.
        void channelMembersChanged(Server* server, const QString& channelName, bool joined, bool parted, const QString& nickname);

        /// Fires when a channel is moved to/from the Joinied/Unjoined lists.
        /// joined indicates which list it is now on.  Note that if joined is False, it is
        /// possible the channel does not exist in any list anymore.
        void channelJoinedOrUnjoined(Server* server, const QString& channelName, bool joined);

        /// Fires when a nick on the watch list goes online or offline.
        void watchedNickChanged(Server* server, const QString& nickname, bool online);
        ///Fires when the user switches his state to away and has enabled "Insert Remember Line on away" in his identity.
        void awayInsertRememberLine(Server* server);
        void sslInitFailure();
        void sslConnected(Server* server);

        void connectionStateChanged(Server* server, Konversation::ConnectionState state);

        void showView(ChatWindow* view);
        void addDccPanel();
        void addDccChat(const QString& myNick,const QString& nick,const QStringList& arguments,bool listen);

    public slots:
        void connectToIRCServer();

        /** Adds line to queue if non-empty. */
        bool queue(const QString& line, QueuePriority priority=StandardPriority);
        //TODO this should be an overload, not a separate name. ambiguous cases need QString() around the cstring
        bool queueList(const QStringList& buffer, QueuePriority priority=StandardPriority);

        void setNickname(const QString &newNickname);
        /** This is called when we want to open a new query, or focus an existing one.
         *  @param nickInfo The nickinfo we want to open the query to.  Must exist.
         *  @param weinitiated This is whether we initiated this - did we do /query, or somebody else sending us a message.
         *  @return A pointer to a new or already-existing query.  Guaranteed to be non-null
         */
        Query *addQuery(const NickInfoPtr & nickInfo, bool weinitiated);
        void closeQuery(const QString &name);
        void closeChannel(const QString &name);
        void quitServer();
        void openDccChat(const QString& nickname);
        void requestDccChat(const QString& partnerNick, const QString& numericalOwnIp, const QString& ownPort);
        void requestBan(const QStringList& users,const QString& channel,const QString& option);
        void requestUnban(const QString& mask,const QString& channel);

        void addDccSend(const QString &recipient,KURL fileURL, const QString &altFileName = QString(), uint fileSize = 0);
        void removeQuery(Query *query);
        void startNotifyTimer(int msec=0);
        void sendJoinCommand(const QString& channelName, const QString& password = QString());
        void requestChannelList();
        void requestWhois(const QString& nickname);
        void requestWho(const QString& channel);
        void requestUserhost(const QString& nicks);
        void requestTopic(const QString& channel);
        void resolveUserhost(const QString& nickname);
        void addRawLog(bool show);
        void closeRawLog();
        void addToChannelList(const QString& channel, int users, const QString& topic);
        void closeChannelListPanel();
        void updateChannelQuickButtons();
        void sendMultiServerCommand(const QString& command, const QString& parameter);
        void executeMultiServerCommand(const QString& command, const QString& parameter);
        void reconnect();
        void disconnect(); //FIXME is this overriding a qobject method? do we care?
        void showSSLDialog();
        void sendToAllChannels(const QString& text);
        void notifyTimeout();

        void enableIdentifyMsg(bool enabled);
        bool identifyMsgEnabled();
        void addBan(const QString &channel, const QString &ban);
        void removeBan(const QString &channel, const QString &ban);

        /// Called when we received a PONG from the server
        void pongReceived();

    protected slots:
        void lookupFinished();
        void preShellCommandExited(KProcess*);
        void ircServerConnectionSuccess();
        void startAwayTimer();
        void incoming();
        void processIncomingData();
        /// Sends the QString to the socket. No longer has any internal concept of queueing
        void toServer(QString&, IRCQueue *);
        /// Because KBufferedSocket has no closed(int) signal we use this slot to call broken(0)
        void closed();
        void broken(int state);
        /** This is connected to the SSLSocket failed.
         * @param reason The reason why this failed.  This is already translated, ready to show the user.
         */
        void sslError(const QString& reason);
        void connectionEstablished(const QString& ownHost);
        void notifyResponse(const QString& nicksOnline);
        void slotNewDccTransferItemQueued(DccTransfer* transfer);
        void startReverseDccSendTransfer(const QString& sourceNick,const QStringList& dccArguments);
        void addDccGet(const QString& sourceNick,const QStringList& dccArguments);
        void requestDccSend();                    // -> to outputFilter, dccPanel
                                                  // -> to outputFilter
        void requestDccSend(const QString& recipient);
                                                  // -> to inputFilter
        void resumeDccGetTransfer(const QString& sourceNick,const QStringList& dccArguments);
                                                  // -> to inputFilter
        void resumeDccSendTransfer(const QString& sourceNick,const QStringList& dccArguments);
        void dccGetDone(DccTransfer* item);
        void dccSendDone(DccTransfer* item);
        void dccStatusChanged(DccTransfer* item, int newStatus, int oldStatus);
        void scriptNotFound(const QString& name);
        void scriptExecutionError(const QString& name);
        void userhost(const QString& nick,const QString& hostmask,bool away,bool ircOp);
        void setTopicAuthor(const QString& channel,const QString& author, QDateTime t);
        void endOfWho(const QString& target);
        void invitation(const QString& nick,const QString& channel);
        void sendToAllChannelsAndQueries(const QString& text);
        void gotOwnResolvedHostByWelcome(KResolverResults res);
        void gotOwnResolvedHostByUserhost(KResolverResults res);

        /// Send a PING to the server so we can meassure the lag
        void sendPing();
        /// Updates GUI when the lag gets high
        void updateLongPongLag();

        /// Update the encoding shown in the mainwindow's actions
        void updateEncoding();

    private slots:
        void collectStats(int bytes, int encodedBytes);

        /** Called in the server constructor if the preferences are set to run a command on a new server instance.
         *  This sets up the kprocess, runs it, and connects the signals to call preShellCommandExited when done. */
        void doPreShellCommand();

    protected:
        // constants
        static const int BUFFER_LEN=513;

        /// Initialize the timers
        void initTimers();

        /// Connect to the signals used in this class.
        void connectSignals();

        int _send_internal(QString outputline); ///< Guts of old send, isn't a slot.

        /** Adds a nickname to the unjoinedChannels list.
         *  Creates new NickInfo if necessary.
         *  If needed, moves the channel from the joined list to the unjoined list.
         *  If needed, moves the nickname from the Offline to the Online list.
         *  If mode != 99 sets the mode for this nick in this channel.
         *  @param channelName The channel name.  Case sensitive.
         *  @param nickname    The nickname.  Case sensitive.
         *  @return            The NickInfo for the nickname.
         */
        ChannelNickPtr addNickToUnjoinedChannelsList(const QString& channelName, const QString& nickname);

        /**
         * If not already online, changes a nick to the online state by creating
         * a NickInfo for it and emits various signals and messages for it.
         * This method should only be called for nicks on the watch list.
         * @param nickname           The nickname that is online.
         * @return                   Pointer to NickInfo for nick.
         */
        NickInfoPtr setWatchedNickOnline(const QString& nickname);

        /**
        * Display offline notification for a certain nickname. The function doesn't change NickInfo objects.
        * If NickInfoPtr is given, then also the integration with KAddressBook is engaged (i.e. the
        * nick is marked as away)
        * @param nickname           The nickname that is offline
        * @param nickInfo           Pointer to NickInfo for nick
        */
        void setWatchedNickOffline(const QString& nickname, const NickInfoPtr nickInfo);

        /**
         * If nickname is no longer on any channel list, or the query list, delete it altogether.
         * Call this routine only if the nick is not on the notify list or is on the notify
         * list but is known to be offline.
         * @param nickname           The nickname to be deleted.  Case insensitive.
         * @return                   True if the nickname is deleted.
         */
        bool deleteNickIfUnlisted(const QString &nickname);

        /**
         * If not already offline, changes a nick to the offline state.
         * Removes it from all channels on the joined and unjoined lists.
         * If the nick is in the watch list, and went offline, emits a signal,
         * posts a Notify message, and posts a KNotify.
         * If the nick is in the addressbook, and went offline, informs addressbook of change.
         * If the nick goes offline, the NickInfo is deleted.
         *
         * @param nickname     The nickname.  Case sensitive.
         * @return             True if the nick was online.
         */
        bool setNickOffline(const QString& nickname);

        /** Remove nickname from a channel (on joined or unjoined lists).
         *  @param channelName The channel name.  Case insensitive.
         *  @param nickname    The nickname.  Case insensitive.
         */
        void removeChannelNick(const QString& channelName, const QString& nickname);

        /** Remove channel from the joined list.
         *  Nicknames in the channel are added to the unjoined list if they are in the watch list.
         *  @param channelName The channel name.  Case insensitive.
         */
        void removeJoinedChannel(const QString& channelName);

        /** Renames a nickname in all NickInfo lists.
         *  @param nickInfo    Pointer to existing NickInfo object.
         *  @param newname     New nickname for the nick.  Case sensitive.
         */
        void renameNickInfo(NickInfoPtr nickInfo, const QString& newname);

        bool getAutoJoin() const;
        void setAutoJoin(bool on);

        QStringList getAutoJoinCommands() const { return m_autoJoinCommands; }
        void setAutoJoinCommands(const QStringList& commands) { m_autoJoinCommands = commands; }

        unsigned int m_completeQueryPosition;
        QValueList<int> m_nickIndices;
        QStringList m_referenceNicklist;

        // TODO roll these into a QMap.
        QString m_serverNickPrefixes;               // Prefixes used by the server to indicate a mode
        QString m_serverNickPrefixModes;            // if supplied: modes related to those prefixes
        QString m_channelPrefixes;                  // prefixes that indicate channel names. defaults to RFC1459 "#&"

        bool m_autoJoin;

        QStringList m_autoJoinCommands;

        KNetwork::KStreamSocket* m_socket;

        QTimer m_reconnectTimer;
        QTimer m_incomingTimer;
        QTimer m_notifyTimer;
        QStringList m_notifyCache;                  // List of users found with ISON
        int m_checkTime;                            // Time elapsed while waiting for server 303 response
        int m_currentLag;

        QCString m_inputBufferIncomplete;
        QStringList m_inputBuffer;

        QValueVector<IRCQueue *> m_queues;
        int m_bytesSent, m_encodedBytesSent, m_linesSent, m_bytesReceived;

        QString m_nickname;
        QString m_loweredNickname;
        QString m_ownIpByUserhost;                  // RPL_USERHOST
        QString m_ownIpByWelcome;                   // RPL_WELCOME

        QPtrList<Channel> m_channelList;
        QPtrList<Query> m_queryList;

        InputFilter m_inputFilter;
        Konversation::OutputFilter* m_outputFilter;

        QGuardedPtr<StatusPanel> m_statusView;
        QGuardedPtr<RawLog> m_rawLog;
        QGuardedPtr<ChannelListPanel> m_channelListPanel;

        bool m_away;
        QString m_awayReason;
        QString m_nonAwayNick;
        int m_awayTime;

        Konversation::ConnectionState m_connectionState;
        void updateConnectionState(Konversation::ConnectionState state);
        bool isSocketConnected() const;

        ScriptLauncher* m_scriptLauncher;

        KProcess m_preShellCommand;

    private:
        /// Helper object to construct ISON (notify) list and map offline nicks to
        /// addressbook.
        ServerISON* m_serverISON;
        /// All nicks known to this server.  Note this is NOT a list of all nicks on the server.
        /// Any nick appearing in this list is online, but may not necessarily appear in
        /// any of the joined or unjoined channel lists because a WHOIS has not yet been
        /// performed on the nick.
        NickInfoMap m_allNicks;
        /// List of membership lists for joined channels.  A "joined" channel is a channel
        /// that user has joined, i.e., a tab appears for the channel in the main window.
        ChannelMembershipMap m_joinedChannels;
        /// List of membership lists for unjoined channels.  These come from WHOIS responses.
        /// Note that this is NOT a list of all channels on the server, just those we are
        /// interested in because of nicks in the Nick Watch List.
        ChannelMembershipMap m_unjoinedChannels;
        /// List of nicks in Queries.
        NickInfoMap m_queryNicks;

        QString m_allowedChannelModes;

        // Blowfish key map
        QMap<QString,QCString> m_keyMap;

        bool m_identifyMsg;
        bool m_autoIdentifyLock;

        /// Used to lock incomingTimer while processing message.
        bool m_processingIncoming;

        /// Meassures the lag between PING and PONG
        QTime m_lagTime;
        /// Updates the gui when the lag gets too high
        QTimer m_pingResponseTimer;

        /// Previous ISON reply of the server, needed for comparison with the next reply
        QStringList m_prevISONList;

        ConnectionSettings m_connectionSettings;

        static int m_availableConnectionId;
        int m_connectionId;
};

#endif
