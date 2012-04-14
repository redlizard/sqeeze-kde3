/***************************************************************************
 *   Copyright (C) 2003 by Ajay Guleria                                    *
 *   ajay_guleria at yahoo dot com                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _CLEARCASEPART_H_
#define _CLEARCASEPART_H_

#include <kdevversioncontrol.h>

class Context;
class QPopupMenu;

class ClearcasePart : public KDevVersionControl {
    Q_OBJECT

public:
    ClearcasePart( QObject *parent, const char *name, const QStringList & );
    ~ClearcasePart();

    const QString default_checkin;
    const QString default_checkout;
    const QString default_uncheckout;
    const QString default_create;
    const QString default_remove;
    const QString default_lshistory;
    const QString default_lscheckout;
    const QString default_diff;

    virtual void createNewProject(const QString& dir) {}
    virtual bool fetchFromRepository() { return true; }
    virtual KDevVCSFileInfoProvider *fileInfoProvider() const { return fileInfoProvider_; }
    virtual bool isValidDirectory(const QString &dirPath) const;

private slots:
    void contextMenu(QPopupMenu *popup, const Context *context);

    void slotCheckin();
    void slotCheckout();
    void slotUncheckout();

    void slotCreate();
    void slotRemove();
    void slotDiff();
    void slotDiffFinished( const QString& diff, const QString& err );
    void slotListHistory();
    void slotListCheckouts();


private:

    bool isValidCCDirectory_;
    QString popupfile_;
    QString viewname;

    KDevVCSFileInfoProvider *fileInfoProvider_;
};

#endif
