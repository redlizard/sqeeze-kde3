/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _GREPVIEWPART_H_
#define _GREPVIEWPART_H_

#include <qguardedptr.h>
#include <kdevgenericfactory.h>
#include "kdevplugin.h"

class KDialogBase;
class QPopupMenu;
class Context;
class GrepViewWidget;


class GrepViewPart : public KDevPlugin
{
    Q_OBJECT

public:
    GrepViewPart( QObject *parent, const char *name, const QStringList & );
    ~GrepViewPart();

private slots:
    void stopButtonClicked(KDevPlugin *which);
    void projectOpened();
    void projectClosed();
    void contextMenu(QPopupMenu *popup, const Context *context);

    void slotGrep();
    void slotContextGrep();

private:
    QGuardedPtr<GrepViewWidget> m_widget;
    QString m_popupstr;
    friend class GrepViewWidget;
};

typedef KDevGenericFactory<GrepViewPart> GrepViewFactory;

#endif
