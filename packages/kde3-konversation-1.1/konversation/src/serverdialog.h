/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

/*
  copyright: (C) 2004 by Peter Simonsson
  email:     psn@linux.se
*/
#ifndef KONVERSATIONSERVERDIALOG_H
#define KONVERSATIONSERVERDIALOG_H

#include <kdialogbase.h>

class QLineEdit;
class QSpinBox;
class QCheckBox;

namespace Konversation
{

    class ServerSettings;

    class ServerDialog : public KDialogBase
    {
        Q_OBJECT

        public:
            explicit ServerDialog(const QString& title, QWidget *parent = 0, const char *name = 0);
            ~ServerDialog();

            void setServerSettings(const ServerSettings& server);
            ServerSettings serverSettings();

        protected slots:
            void slotOk();

        private:
            QLineEdit* m_serverEdit;
            QSpinBox* m_portSBox;
            QLineEdit* m_passwordEdit;
            QCheckBox* m_sslChBox;
    };

}
#endif
