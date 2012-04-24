/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

/*
  Copyright (C) 2006 Eike Hein <hein@kde.org>
*/

#ifndef TABS_PREFERENCES_H
#define TABS_PREFERENCES_H

#include "tabs_preferencesui.h"


class Tabs_Config : public Tabs_PreferencesUI
{
    Q_OBJECT

    public:
        explicit Tabs_Config(QWidget *parent = 0, const char *name = 0);
        ~Tabs_Config();

    public slots:
        virtual void show();

    protected slots:
        void toggleCheckBoxes(int activated);
};

#endif
