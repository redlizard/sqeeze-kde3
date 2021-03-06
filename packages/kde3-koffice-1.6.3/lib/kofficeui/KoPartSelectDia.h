/* This file is part of the KDE libraries
    Copyright (C) 1998 Torben Weis <weis@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef DlgPartSelect_included
#define DlgPartSelect_included

#include <kdialogbase.h>
#include <KoQueryTrader.h>

class QListView;
class QListViewItem;
class QPushButton;

/**
 *  This dialog presents the user all available
 *  KOffice components ( KSpread, KWord etc ) with name
 *  and mini icon. The user may select one and
 *  the corresponding KoDocumentEntry is returned.
 */
class KOFFICEUI_EXPORT KoPartSelectDia : public KDialogBase
{
    Q_OBJECT

public:

    /**
     *  Constructor.
     */
    KoPartSelectDia( QWidget* parent = 0, const char* name = 0 );

    /**
     *  Retrieves the result of the part selection.
     *
     *  @return A document entry.
     */
    KoDocumentEntry entry();

    /**
     *  Convenience function for using the dialog.
     *
     *  @returns the KoDocumentEntry of the selected KOffice components
     *           or an empty entry.
     */
    static KoDocumentEntry selectPart( QWidget *parent = 0L );

private slots:
    void selectionChanged( QListViewItem * );

private:
    QValueList<KoDocumentEntry> m_lstEntries;
    QListView *listview;
};

#endif
