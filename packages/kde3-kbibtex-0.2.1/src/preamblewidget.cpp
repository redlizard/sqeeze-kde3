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
#include <qlabel.h>
#include <qlayout.h>

#include <kdialogbase.h>
#include <klocale.h>

#include "preamblewidget.h"

namespace KBibTeX
{

    QDialog::DialogCode PreambleWidget::execute( BibTeX::Preamble *preamble, bool isReadOnly, QWidget *parent, const char *name )
    {
        KDialogBase * dlg = new KDialogBase( parent, name, TRUE, i18n( "Edit BibTeX Preamble" ), KDialogBase::Ok | KDialogBase::Cancel );
        PreambleWidget* preambleWidget = new PreambleWidget( preamble, isReadOnly, dlg, "PreambleWidget" );

        dlg->setMainWidget( preambleWidget );
        connect( dlg, SIGNAL( okClicked() ), preambleWidget, SLOT( apply() ) );

        QDialog::DialogCode result = ( QDialog::DialogCode ) dlg->exec();

        delete( preambleWidget );
        delete( dlg );

        return result;
    }

    PreambleWidget::PreambleWidget( BibTeX::Preamble *bibtexpreamble, bool isReadOnly, QWidget *parent, const char *name ) : QWidget( parent, name ), m_isReadOnly( isReadOnly ), m_bibtexpreamble( bibtexpreamble )
    {
        setupGUI();
        reset();
    }


    PreambleWidget::~PreambleWidget()
    {
// nothing
    }

    void PreambleWidget::apply()
    {
        BibTeX::Value *value = m_fieldLineEditPreambleValue->value();
        m_bibtexpreamble->setValue( value );
    }

    void PreambleWidget::reset()
    {
        m_fieldLineEditPreambleValue->setValue( m_bibtexpreamble->value() );
    }

    void PreambleWidget::setupGUI()
    {
        setMinimumWidth( 384 );

        QVBoxLayout * layout = new QVBoxLayout( this, 0, KDialog::spacingHint() );

        QLabel *label = new QLabel( i18n( "&Preamble:" ), this );
        layout->addWidget( label );
        m_fieldLineEditPreambleValue = new KBibTeX::FieldLineEdit( i18n( "Preamble" ), KBibTeX::FieldLineEdit::itMultiLine, m_isReadOnly, this, "m_fieldLineEditPreambleValue" );
        layout->addWidget( m_fieldLineEditPreambleValue );
        label->setBuddy( m_fieldLineEditPreambleValue );
    }

}
