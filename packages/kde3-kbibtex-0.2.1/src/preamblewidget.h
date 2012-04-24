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
#ifndef KBIBTEXPREAMBLEWIDGET_H
#define KBIBTEXPREAMBLEWIDGET_H

#include <qwidget.h>
#include <qdialog.h>

#include <preamble.h>
#include <fieldlineedit.h>

namespace KBibTeX
{

    /**
     @author Thomas Fischer <fischer@unix-ag.uni-kl.de>
    */
    class PreambleWidget : public QWidget
    {
        Q_OBJECT
    public:
        static QDialog::DialogCode execute( BibTeX::Preamble *preamble, bool isReadOnly, QWidget *parent = 0, const char *name = 0 );

    private slots:
        void apply();
        void reset();

    private:
        bool m_isReadOnly;
        BibTeX::Preamble* m_bibtexpreamble;
        KBibTeX::FieldLineEdit *m_fieldLineEditPreambleValue;

        PreambleWidget( BibTeX::Preamble *bibtexpreamble, bool isReadOnly, QWidget *parent = 0, const char *name = 0 );
        ~PreambleWidget();

        void setupGUI();

    };

}

#endif
