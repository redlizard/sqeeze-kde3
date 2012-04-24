/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

/*
  Copyright (C) 2006 Dario Abatianni <eisfuchs@tigress.com>
  Copyright (C) 2006 John Tapsell <johnflux@gmail.com>
*/

#ifndef NICKLISTBEHAVIOR_Config_H
#define NICKLISTBEHAVIOR_Config_H

#include "konvisettingspage.h"
#include "nicklistbehavior_preferencesui.h"

#include <qobject.h>


class NicklistBehavior_Config : public NicklistBehavior_ConfigUI, public KonviSettingsPage
{
    Q_OBJECT

    public:
        explicit NicklistBehavior_Config(QWidget *parent = 0, const char *name = 0);
        ~NicklistBehavior_Config();

        virtual void saveSettings();
        virtual void loadSettings();
        virtual void restorePageToDefaults();

        virtual bool hasChanged();

    private:
        void setNickList(const QString &sortingOrder);
        QString currentSortingOrder();

        QString m_oldSortingOrder;

    signals:
        void modified();
};

#endif
