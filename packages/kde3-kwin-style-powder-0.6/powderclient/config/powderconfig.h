/***************************************************************************
*   Copyright (C) 2003 by Sandro Giessl                                   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/

#ifndef POWDERCONFIG_H
#define POWDERCONFIG_H

#include <qobject.h>

class QButtonGroup;
class QGroupBox;
class QComboBox;
class KConfig;
class ConfigDialog;

class PowderConfig : public QObject
{
    Q_OBJECT
public:
    PowderConfig(KConfig* config, QWidget* parent); ///< Constructor
    ~PowderConfig(); ///< Destructor

signals:
    void changed(); ///< Widget state has changed

public slots:
    void load(KConfig *config); ///< Load configuration data
    void save(KConfig *config); ///< Save configuration data
    void defaults(); ///< Set configuration defaults

private:
    KConfig *m_config;
    ConfigDialog *m_dialog;
};

#endif
