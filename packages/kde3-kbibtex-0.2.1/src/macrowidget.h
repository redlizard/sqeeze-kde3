/**************************************************************************
* Copyright ( C ) 2004 - 2005 by Thomas Fischer *
* fischer@unix - ag.uni - kl.de *
* *
* This program is free software; you can redistribute it and / or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or *
* ( at your option ) any later version. *
* *
* This program is distributed in the hope that it will be useful, *
* but WITHOUT ANY WARRANTY; without even the implied warranty of *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the *
* GNU General Public License for more details. *
* *
* You should have received a copy of the GNU General Public License *
* along with this program; if not, write to the *
* Free Software Foundation, Inc., *
* 59 Temple Place - Suite 330, Boston, MA 02111 - 1307, USA. *
**************************************************************************/
#ifndef MACROWIDGET_H
#define MACROWIDGET_H

#include <qwidget.h>

#include <macro.h>
#include <fieldlineedit.h>

class KLineEdit;

namespace KBibTeX
{
    class MacroWidget : public QWidget
    {
        Q_OBJECT
    public:
        static QDialog::DialogCode execute( BibTeX::Macro *macro, bool isReadOnly, QWidget *parent = 0, const char *name = 0 );

    private slots:
        void apply();
        void reset();

    private:
        bool m_isReadOnly;
        BibTeX::Macro* m_bibtexmacro;
        KLineEdit *m_lineEditMacroId;
        KBibTeX::FieldLineEdit *m_fieldLineEditMacroValue;

        MacroWidget( BibTeX::Macro *bibtexmacro, bool isReadOnly, QWidget *parent = 0, const char *name = 0 );
        ~MacroWidget();

        void setupGUI();

    };
}

#endif
