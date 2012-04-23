/***************************************************************************
*   Copyright (C) 2003 by Sandro Giessl                                   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/

#ifndef KNIFTYCONFIG_H
#define KNIFTYCONFIG_H

#include <qobject.h>

class QButtonGroup;
class QGroupBox;
class KConfig;
class ConfigDialog;

class KniftyConfig : public QObject
{
    Q_OBJECT
public:
    KniftyConfig(KConfig* config, QWidget* parent);
    ~KniftyConfig();

signals:
    void changed();

public slots:
    void load(KConfig *config);
    void save(KConfig *config);
    void defaults();

private:
    KConfig *m_config;
    ConfigDialog *m_dialog;
};

#endif // KNIFTYCONFIG_H
