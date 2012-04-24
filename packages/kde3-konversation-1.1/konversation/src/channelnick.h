/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

/*
  channelnick.h - There is an instance of this for each nick in each channel.  So for a person in multiple channels, they will have one NickInfo, and multiple ChannelNicks.
  begin:     Wed Aug 04 2004
  copyright: (C) 2002,2003,2004 by Dario Abatianni
  email:     eisfuchs@tigress.com
*/

#ifndef CHANNEL_NICK_H
#define CHANNEL_NICK_H

#include "nickinfo.h"

#include <ksharedptr.h>


class ChannelNick :  public QObject, public KShared
{
    Q_OBJECT

        public:
        ChannelNick(const NickInfoPtr& nickInfo, const bool& isop, const bool& isadmin,
            const bool& isowner, const bool& ishalfop, const bool& hasvoice);
        ~ChannelNick();
        bool isOp() const;
        bool isAdmin() const;
        bool isOwner() const;
        bool isHalfOp() const;

        /** Return true if the may have any privillages at all
         * @return true if isOp() || isAdmin() || isOwner() || isHalfOp()
         */
        bool isAnyTypeOfOp() const;
        bool hasVoice() const;
        uint timeStamp() const;
        uint recentActivity() const;
        void moreActive();

        bool setVoice(bool state);
        bool setOp(bool state);
        bool setHalfOp(bool state);
        bool setAdmin(bool state);
        bool setOwner(bool state);
        bool setMode(char mode, bool plus);
        bool setMode(int mode);
        bool setMode(bool admin,bool owner,bool op,bool halfop,bool voice);
        void setTimeStamp(uint stamp);

        NickInfoPtr getNickInfo() const;
        //Purely provided for convenience because they are used so often.
        //Just calls nickInfo->getNickname() etc
        QString getNickname() const;
        QString loweredNickname() const;
        QString getHostmask() const;
        QString tooltip();
    private:
        NickInfoPtr nickInfo;
        bool isop;
        bool isadmin;
        bool isowner;
        bool ishalfop;
        bool hasvoice;
        uint m_timeStamp;
        uint m_recentActivity;

    signals:
        void channelNickChanged();

    public slots:
        void lessActive();
};

/** A ChannelNickPtr is a pointer to a ChannelNick.  Since it is a KSharedPtr,
 *  the ChannelNick object is automatically destroyed when all references are destroyed.
 */
typedef KSharedPtr<ChannelNick> ChannelNickPtr;

/** A ChannelNickMap is a list of ChannelNick pointers, indexed and sorted by
 *  lowercase nickname.
 */
typedef QMap<QString,ChannelNickPtr> ChannelNickMap;

typedef QValueList<ChannelNickPtr> ChannelNickList;

/** A ChannelMembershipMap is a list of ChannelNickMap pointers, indexed and
 *  sorted by lowercase channel name.
 */
typedef QMap<QString,ChannelNickMap *> ChannelMembershipMap;
#endif                                            /* CHANNEL_NICK_H */
