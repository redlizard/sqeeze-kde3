/***************************************************************************
 *   Copyright (C) 2004-2005 by Thomas Fischer                             *
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
#ifndef KBIBTEXMERGEENTRIES_H
#define KBIBTEXMERGEENTRIES_H

#include <qobject.h>

#include <file.h>
#include <entry.h>

class KDialog;
class KPushButton;
class KTextEdit;

namespace KBibTeX
{

    /**
     @author Thomas Fischer <fischer@unix-ag.uni-kl.de>
    */
    class MergeEntries : public QObject
    {
        Q_OBJECT
    public:
        static void mergeBibTeXFiles( BibTeX::File *original, BibTeX::File *inserted );

    protected:
        static bool mergeBibTeXEntries( BibTeX::File *bibTeXFile, MergeEntries *mergeEntries, BibTeX::Entry *original, BibTeX::Entry *inserted );
        static bool mergeBibTeXMacros( BibTeX::File *bibTeXFile, MergeEntries *mergeEntries, BibTeX::Macro *original, BibTeX::Macro *inserted );

    protected:
        enum MergeChoice{ mcKeepOriginal, mcUseInserted, mcMerge, mcKeepBoth, mcCancel };

        KDialog *m_mergeWindow;
        KTextEdit *m_originalText;
        KTextEdit *m_insertedText;
        KPushButton *m_buttonMerge;
        MergeChoice m_status;

        MergeEntries();
        ~MergeEntries();
        MergeChoice show( BibTeX::Element *original, BibTeX::Element *inserted );

    private:
        void createMergeWindow();
        QString elementText( BibTeX::Element *element );

    private slots:
        void slotKeepOriginal();
        void slotUseInserted();
        void slotKeepBoth();
        void slotMerge();
    };

}

#endif
