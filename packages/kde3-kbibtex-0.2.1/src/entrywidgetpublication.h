/***************************************************************************
*   Copyright (C) 2004-2006 by Thomas Fischer                             *
*   fischer@unix-ag.uni-kl.de                                             *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#ifndef KBIBTEXENTRYWIDGETPUBLICATION_H
#define KBIBTEXENTRYWIDGETPUBLICATION_H

#include <entrywidgettab.h>

class QPushButton;

namespace KBibTeX
{
    class FieldLineEdit;

    class EntryWidgetPublication : public EntryWidgetTab
    {
        Q_OBJECT
    public:
        EntryWidgetPublication( BibTeX::File *bibtexfile, bool isReadOnly, QWidget *parent = 0, const char *name = 0 );
        ~EntryWidgetPublication();

        bool isModified();

    public slots:
        void updateGUI( BibTeX::Entry::EntryType entryType, bool enableAll );
        void apply( BibTeX::Entry *entry );
        void reset( BibTeX::Entry *entry );
        void updateWarnings( BibTeX::Entry::EntryType entryType, QListView *listViewWarnings );

    private slots:
        void slotSetMonth( int month );
        void slotSetCrossRefEntry();
        void slotOpenISBN();

    private:
        KBibTeX::FieldLineEdit *m_fieldLineEditOrganization;
        KBibTeX::FieldLineEdit *m_fieldLineEditPublisher;
        KBibTeX::FieldLineEdit *m_fieldLineEditSchool;
        KBibTeX::FieldLineEdit *m_fieldLineEditInstitution;
        KBibTeX::FieldLineEdit *m_fieldLineEditLocation;
        KBibTeX::FieldLineEdit *m_fieldLineEditAddress;
        KBibTeX::FieldLineEdit *m_fieldLineEditJournal;
        KBibTeX::FieldLineEdit *m_fieldLineEditPages;
        KBibTeX::FieldLineEdit *m_fieldLineEditEdition;
        KBibTeX::FieldLineEdit *m_fieldLineEditChapter;
        KBibTeX::FieldLineEdit *m_fieldLineEditVolume;
        KBibTeX::FieldLineEdit *m_fieldLineEditNumber;
        KBibTeX::FieldLineEdit *m_fieldLineEditMonth;
        QPushButton *m_pushButtonMonths;
        KBibTeX::FieldLineEdit *m_fieldLineEditYear;
        KBibTeX::FieldLineEdit *m_fieldLineEditISBN;
        QPushButton *m_pushButtonISBN;
        KBibTeX::FieldLineEdit *m_fieldLineEditISSN;
        KBibTeX::FieldLineEdit *m_fieldLineEditCrossRef;
        KBibTeX::FieldLineEdit *m_fieldLineEditHowPublished;

        void setupGUI();
        QString isbn() const;

    };

}

#endif
