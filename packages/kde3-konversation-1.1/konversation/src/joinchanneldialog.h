/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

/*
  Copyright (C) 2004 Peter Simonsson <psn@linux.se>
*/

#ifndef KONVERSATIONJOINCHANNELDIALOG_H
#define KONVERSATIONJOINCHANNELDIALOG_H

#include <kdialogbase.h>


class Server;

namespace Konversation
{

    class JoinChannelUI;

    class JoinChannelDialog : public KDialogBase
    {
        Q_OBJECT
            public:
            explicit JoinChannelDialog(Server* server, QWidget *parent = 0, const char *name = 0);
            ~JoinChannelDialog();

            QString channel() const;
            QString password() const;

        protected slots:
            virtual void slotOk();

        private:
            JoinChannelUI* m_widget;
            Server* m_server;
    };

}
#endif
