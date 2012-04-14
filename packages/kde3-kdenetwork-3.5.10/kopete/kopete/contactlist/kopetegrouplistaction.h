/*
    kopetegrouplistaction.h

    Copyright (c) 2005   Olivier Goffart <ogoffart@ kde.org>


    Kopete    (c) 2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEGRLISTACT_H
#define KOPETEGRLISTACT_H


#include <kaction.h>


/**
 * Action used for Copy To  and  Move To
 */
class KopeteGroupListAction : public KListAction
{
	Q_OBJECT

public:
	KopeteGroupListAction( const QString &, const QString &, const KShortcut &,
		const QObject *, const char *, QObject *, const char * );
	~KopeteGroupListAction();

protected slots:
	void slotUpdateList();
private:
	QStringList m_groupList;
};

#endif
