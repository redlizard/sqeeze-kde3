/***************************************************************************
*   Copyright (C) 2004-2008 by Thomas Fischer                             *
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
#include <qlabel.h>
#include <qlayout.h>

#include <klineedit.h>
#include <kdialogbase.h>
#include <klocale.h>
#include <kdebug.h>

#include <element.h>
#include <fileexporterbibtex.h>
#include <fileimporterbibtex.h>
#include <fieldlineedit.h>

#include "macrowidget.h"

namespace KBibTeX
{
    QDialog::DialogCode MacroWidget::execute( BibTeX::Macro *macro, bool isReadOnly, QWidget *parent, const char *name )
    {
        KDialogBase * dlg = new KDialogBase( parent, name, TRUE, i18n( "Edit BibTeX Macro" ), KDialogBase::Ok | KDialogBase::Cancel );
        MacroWidget* macroWidget = new MacroWidget( macro, isReadOnly, dlg, "MacroWidget" );

        dlg->setMainWidget( macroWidget );
        connect( dlg, SIGNAL( okClicked() ), macroWidget, SLOT( apply() ) );

        QDialog::DialogCode result = ( QDialog::DialogCode ) dlg->exec();

        delete( macroWidget );
        delete( dlg );

        return result;
    }

    MacroWidget::MacroWidget( BibTeX::Macro *bibtexmacro, bool isReadOnly, QWidget *parent, const char *name )
            : QWidget( parent, name ), m_isReadOnly( isReadOnly ), m_bibtexmacro( bibtexmacro )
    {
        setupGUI();
        reset();
    }

    MacroWidget::~MacroWidget()
    {
        // nothing
    }

    void MacroWidget::apply()
    {
        m_bibtexmacro->setKey( m_lineEditMacroId->text() );
        BibTeX::Value *value = m_fieldLineEditMacroValue->value();
        m_bibtexmacro->setValue( value );
    }

    void MacroWidget::reset()
    {
        m_lineEditMacroId->setText( m_bibtexmacro->key() );
        m_fieldLineEditMacroValue->setValue( m_bibtexmacro->value() );
    }

    void MacroWidget::setupGUI()
    {
        setMinimumWidth( 384 );

        QVBoxLayout * layout = new QVBoxLayout( this, 0, KDialog::spacingHint() );

        QLabel *label = new QLabel( i18n( "Macro &id:" ), this );
        layout->addWidget( label );
        m_lineEditMacroId = new KLineEdit( this, "m_lineEditMacroId" );
        m_lineEditMacroId->setReadOnly( m_isReadOnly );
        layout->addWidget( m_lineEditMacroId );
        label->setBuddy( m_lineEditMacroId );

        label = new QLabel( i18n( "Macro &value:" ), this );
        layout->addWidget( label );
        m_fieldLineEditMacroValue = new KBibTeX::FieldLineEdit( i18n( "Macro" ), KBibTeX::FieldLineEdit::itMultiLine, m_isReadOnly, this, "m_fieldLineEditMacroValue" );
        layout->addWidget( m_fieldLineEditMacroValue );
        label->setBuddy( m_fieldLineEditMacroValue );
    }
}

#include "macrowidget.moc"
