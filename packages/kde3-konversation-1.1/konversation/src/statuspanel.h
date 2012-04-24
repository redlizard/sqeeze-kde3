/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

/*
  Copyright (C) 2003 Dario Abatianni <eisfuchs@tigress.com>
  Copyright (C) 2006-2008 Eike Hein <hein@kde.org>
*/

#ifndef STATUSPANEL_H
#define STATUSPANEL_H

#include "chatwindow.h"

#include <qstring.h>


class QPushButton;
class QCheckBox;
class QLabel;
class QComboBox;

class IRCInput;
class NickChangeDialog;

class StatusPanel : public ChatWindow
{
    Q_OBJECT

    public:
        explicit StatusPanel(QWidget* parent);
        ~StatusPanel();

        virtual void setName(const QString& newName);

        virtual QString getTextInLine();
        virtual bool closeYourself(bool askForConfirmation=true);
        virtual bool canBeFrontView();
        virtual bool searchView();

        virtual void setChannelEncoding(const QString& encoding);
        virtual QString getChannelEncoding();
        virtual QString getChannelEncodingDefaultDesc();
        virtual void emitUpdateInfo();

        void setIdentity(const IdentityPtr identity);

        virtual bool isInsertSupported() { return true; }

        virtual void setNotificationsEnabled(bool enable);

    signals:
        void sendFile();

    public slots:
        void setNickname(const QString& newNickname);
        virtual void indicateAway(bool show);
        void showNicknameBox(bool show);
        void updateAppearance();
        virtual void appendInputText(const QString&, bool fromCursor);
        void updateName();
        void serverSaysClose();

    protected slots:
        void sendFileMenu();
        void statusTextEntered();
        void sendStatusText(const QString& line);
        // connected to IRCInput::textPasted() - used for large/multiline pastes
        void textPasted(const QString& text);
        void changeNickname(const QString& newNickname);
        void nicknameComboboxChanged();
        //Used to disable functions when not connected
        virtual void serverOnline(bool online);

        void popupCommand(int command);

    protected:

        /** Called from ChatWindow adjustFocus */
        virtual void childAdjustFocus();

        bool awayChanged;
        bool awayState;

        void showEvent(QShowEvent* event);

        QComboBox* nicknameCombobox;
        QLabel* awayLabel;
        IRCInput* statusInput;
        QCheckBox* logCheckBox;
        QString oldNick;
};
#endif
