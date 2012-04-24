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
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qtextedit.h>

#include <kdialog.h>
#include <klocale.h>
#include <kiconloader.h>
#include <klineedit.h>

#include <element.h>
#include <entry.h>
#include <settings.h>
#include <value.h>
#include <valuewidget.h>
#include "fieldlineedit.h"

namespace KBibTeX
{
    FieldLineEdit::FieldLineEdit( const QString& caption, InputType inputType, bool isReadOnly, QWidget *parent, const char *name )
            : QWidget( parent, name ), m_value( new BibTeX::Value() ), m_caption( caption ), m_lineEdit( NULL ), m_textEdit( NULL ), m_isReadOnly( isReadOnly ), m_enabled( TRUE ), m_inputType( inputType ), m_isModified( FALSE ), m_fieldType( BibTeX::EntryField::ftUnknown ), m_completion( NULL )
    {
        setupGUI( name );
    }


    FieldLineEdit::~FieldLineEdit()
    {
        delete m_value;
    }

    void FieldLineEdit::setValue( const BibTeX::Value *value )
    {
        delete m_value;

        if ( value != NULL )
            m_value = new BibTeX::Value( value );
        else
            m_value = new BibTeX::Value();

        updateGUI();
        m_isModified = FALSE;
    }

    BibTeX::Value *FieldLineEdit::value()
    {
        if ( m_value->items.isEmpty() )
            return NULL;
        else
            return new BibTeX::Value( m_value );
    }

    void FieldLineEdit::setEnabled( bool enabled )
    {
        m_enabled = enabled;
        updateGUI();
    }

    void FieldLineEdit::setFieldType( BibTeX::EntryField::FieldType fieldType )
    {
        m_fieldType = fieldType;
        Settings * settings = Settings::self();
        m_completion = settings->completion( m_fieldType );

        if ( m_lineEdit != NULL )
        {
            if ( !m_pushButtonString->isOn() )
                m_lineEdit->setCompletionObject( m_completion );
            else
                m_lineEdit->setCompletionObject( settings->completionMacro );
            QToolTip::add( m_lineEdit, QString( i18n( "BibTeX field '%1'" ) ).arg( BibTeX::EntryField::fieldTypeToString( fieldType ) ) );
            QWhatsThis::add( m_lineEdit, QString( i18n( "BibTeX field '%1'" ) ).arg( BibTeX::EntryField::fieldTypeToString( fieldType ) ) );
        }
        else if ( m_textEdit != NULL )
        {
            QToolTip::add( m_textEdit, QString( i18n( "BibTeX field '%1'" ) ).arg( BibTeX::EntryField::fieldTypeToString( fieldType ) ) );
            QWhatsThis::add( m_textEdit, QString( i18n( "BibTeX field '%1'" ) ).arg( BibTeX::EntryField::fieldTypeToString( fieldType ) ) );
        }

    }

    QString FieldLineEdit::caption()
    {
        return m_caption;
    }

    bool FieldLineEdit::isEmpty()
    {
        return m_value->items.isEmpty();
    }

    bool FieldLineEdit::isModified()
    {
        return m_isModified;
    }

    FieldLineEdit::ErrorType FieldLineEdit::error()
    {
        ErrorType result = etNoError;
        for ( QValueList<BibTeX::ValueItem*>::ConstIterator it = m_value->items.begin(); result == etNoError && it != m_value->items.end(); ++it )
        {
            BibTeX::MacroKey *macroKey = dynamic_cast<BibTeX::MacroKey*>( *it );
            result = ( macroKey != NULL && !macroKey->isValid() ) ? etInvalidStringKey : etNoError;
        }

        return result;
    }

    void FieldLineEdit::slotTextChanged( )
    {
        QString text;
        switch ( m_inputType )
        {
        case itSingleLine:
            text = m_lineEdit->text();
            break;
        case itMultiLine:
            text = m_textEdit->text();
            break;
        }

        if ( m_value->items.count() <= 1 )
        {
            m_value->items.clear();
            if ( !text.isEmpty() )
            {
                if ( m_pushButtonString->isOn() )
                    m_value->items.append( new BibTeX::MacroKey( text ) );
                else
                    m_value->items.append( new BibTeX::PlainText( text ) );
            }
            m_isModified = TRUE;
        }

        updateGUI();
    }

    void FieldLineEdit::slotStringToggled()
    {
        if ( m_value->items.count() == 1 )
        {
            QString text = m_value->items.first()->text();
            if ( m_pushButtonString->isOn() )
            {
                BibTeX::PlainText *plainText = dynamic_cast<BibTeX::PlainText*>( m_value->items.first() );
                if ( plainText != NULL )
                {
                    m_value->items.clear();
                    m_value->items.append( new BibTeX::MacroKey( text ) );
                }
            }
            else
            {
                BibTeX::MacroKey *macroKey = dynamic_cast<BibTeX::MacroKey*>( m_value->items.first() );
                if ( macroKey != NULL )
                {
                    m_value->items.clear();
                    m_value->items.append( new BibTeX::PlainText( text ) );
                }
            }
        }

        if ( m_lineEdit != NULL )
        {
            if ( m_pushButtonString->isOn() )
            {
                Settings * settings = Settings::self();
                m_lineEdit->setCompletionObject( settings->completionMacro );
            }
            else
                m_lineEdit->setCompletionObject( m_completion );
        }

        updateGUI();
        emit textChanged();
        m_isModified = TRUE;
    }

    void FieldLineEdit::slotComplexClicked()
    {
        if ( ValueWidget::execute( m_caption, m_fieldType, m_value, m_isReadOnly, this ) == QDialog::Accepted )
        {
            updateGUI();
            emit textChanged();
            m_isModified = TRUE;
        }
    }

    void FieldLineEdit::setupGUI( const char *name )
    {
        Settings * settings = Settings::self();

        char * subname = NULL;
        if ( name != NULL )
            subname = new char[ strlen( name ) + 20 ];

        if ( subname != NULL )
        {
            strcpy( subname, name );
            strcat( subname, "_pbstring" );
        }
        m_pushButtonString = new QPushButton( this, subname );
        m_pushButtonString->setIconSet( QIconSet( SmallIcon( "flag" ) ) );
        m_pushButtonString->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
        m_pushButtonString->setToggleButton( TRUE );
        QToolTip::add( m_pushButtonString, QString( i18n( "Set '%1' to be a string key" ) ).arg( m_caption ) );
        connect( m_pushButtonString, SIGNAL( clicked() ), this, SLOT( slotStringToggled() ) );

        if ( subname != NULL )
        {
            strcpy( subname, name );
            strcat( subname, "_pbcomplex" );
        }
        m_pushButtonComplex = new QPushButton( this, subname );
        m_pushButtonComplex->setIconSet( QIconSet( SmallIcon( "leftjust" ) ) );
        m_pushButtonComplex->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
        QToolTip::add( m_pushButtonComplex, QString( i18n( "Edit '%1' as a concatenated value" ) ).arg( m_caption ) );
        connect( m_pushButtonComplex, SIGNAL( clicked() ), this, SLOT( slotComplexClicked() ) );

        QGridLayout * layout = NULL;
        switch ( m_inputType )
        {
        case itSingleLine:
            {
                layout = new QGridLayout( this, 2, 3, 0, KDialog::spacingHint() );
                if ( subname != NULL )
                {
                    strcpy( subname, name );
                    strcat( subname, "_lineedit" );
                }
                m_lineEdit = new KLineEdit( this, subname );
                m_lineEdit->setReadOnly( m_isReadOnly );
                if ( settings->editing_UseSpecialFont )
                    m_lineEdit->setFont( settings->editing_SpecialFont );
                layout->addWidget( m_lineEdit, 0, 0 );
                setFocusProxy( m_lineEdit );
                layout->addWidget( m_pushButtonString, 0, 1 );
                layout->addWidget( m_pushButtonComplex, 0, 2 );
                QWidget::setTabOrder( m_lineEdit, m_pushButtonString );
            }
            break;
        case itMultiLine:
            {
                layout = new QGridLayout( this, 3, 2, 0, KDialog::spacingHint() );
                layout->setRowStretch( 2, 1 );
                if ( subname != NULL )
                {
                    strcpy( subname, name );
                    strcat( subname, "_textedit" );
                }
                m_textEdit = new QTextEdit( this, subname );
                m_textEdit->setReadOnly( m_isReadOnly );
                if ( settings->editing_UseSpecialFont )
                    m_textEdit->setFont( settings->editing_SpecialFont );
                layout->addMultiCellWidget( m_textEdit, 0, 2, 0, 0 );
                setFocusProxy( m_textEdit );
                layout->addWidget( m_pushButtonString, 0, 1 );
                layout->addWidget( m_pushButtonComplex, 1, 1 );
                QWidget::setTabOrder( m_textEdit, m_pushButtonString );
            }
            break;
        }

        QWidget::setTabOrder( m_pushButtonString, m_pushButtonComplex );
        layout->setRowStretch( layout->numRows() - 1, 1 );

        if ( subname != NULL )
            delete subname;

        enableSignals( TRUE );
    }

    void FieldLineEdit::updateGUI()
    {
        enableSignals( FALSE );

        bool inputEnable = ( m_value->items.count() <= 1 ) && m_enabled;
        m_pushButtonString->setEnabled( inputEnable && !m_isReadOnly );
        bool isComplex = !inputEnable && m_value->items.count() > 1;

        if ( m_value->items.count() == 1 )
        {
            BibTeX::ValueItem * item = m_value->items.first();
            m_pushButtonString->setOn( dynamic_cast<BibTeX::MacroKey*>( m_value->items.first() ) != NULL );
            switch ( m_inputType )
            {
            case itSingleLine:
                if ( QString::compare( m_lineEdit->text(), item->text() ) != 0 )
                    m_lineEdit->setText( item->text() );
                break;
            case itMultiLine:
                if ( QString::compare( m_textEdit->text(), item->text() ) != 0 )
                    m_textEdit->setText( item->text() );
                break;
            }
        }
        else
        {
            switch ( m_inputType )
            {
            case itSingleLine:
                m_lineEdit->setText( isComplex ? i18n( "Concatenated value" ) : "" );
                break;
            case itMultiLine:
                m_textEdit->setText( isComplex ?  i18n( "Concatenated value" ) : "" );
                break;
            }
        }

        m_pushButtonComplex->setEnabled( m_enabled );

        switch ( m_inputType )
        {
        case itSingleLine:
            m_lineEdit->setEnabled( inputEnable );
            m_lineEdit->setReadOnly( m_isReadOnly );
            break;
        case itMultiLine:
            m_textEdit->setEnabled( inputEnable );
            m_textEdit->setReadOnly( m_isReadOnly );
            break;
        }

        enableSignals( TRUE );
    }

    void FieldLineEdit::enableSignals( bool enabled )
    {
        switch ( m_inputType )
        {
        case itSingleLine:
            if ( enabled )
            {
                connect( m_lineEdit, SIGNAL( textChanged( const QString& ) ), this, SLOT( slotTextChanged( ) ) );
                connect( m_lineEdit, SIGNAL( textChanged( const QString& ) ), this, SIGNAL( textChanged( ) ) );
            }
            else
            {
                disconnect( m_lineEdit, SIGNAL( textChanged( const QString& ) ), this, SLOT( slotTextChanged( ) ) );
                disconnect( m_lineEdit, SIGNAL( textChanged( const QString& ) ), this, SIGNAL( textChanged( ) ) );
            }
            break;
        case itMultiLine:
            if ( enabled )
            {
                connect( m_textEdit, SIGNAL( textChanged( ) ), this, SLOT( slotTextChanged( ) ) );
                connect( m_textEdit, SIGNAL( textChanged( ) ), this, SIGNAL( textChanged( ) ) );
            }
            else
            {
                disconnect( m_textEdit, SIGNAL( textChanged( ) ), this, SLOT( slotTextChanged( ) ) );
                disconnect( m_textEdit, SIGNAL( textChanged( ) ), this, SIGNAL( textChanged( ) ) );
            }
            break;
        }
    }
}
#include "fieldlineedit.moc"
