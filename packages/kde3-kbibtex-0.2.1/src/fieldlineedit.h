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
#ifndef KBIBTEXFIELDLINEEDIT_H
#define KBIBTEXFIELDLINEEDIT_H

#include <qwidget.h>

#include <entryfield.h>
#include <value.h>

class KLineEdit;
class QTextEdit;
class QPushButton;
class KCompletion;

namespace KBibTeX
{
    class FieldLineEdit : public QWidget
    {
        Q_OBJECT
    public:
        enum InputType {itSingleLine, itMultiLine};
        enum ErrorType {etNoError, etInvalidStringKey};

        FieldLineEdit( const QString& caption, InputType inputType = itSingleLine, bool readOnly = FALSE, QWidget *parent = 0, const char *name = 0 );
        ~FieldLineEdit();

        /**
         * Set the widget to use this value. The widget will make
         * a deep copy internally and not modify the parameter.
         * If NULL is passed to this function, the widget will be
         * initialized with an empty BibTeXValue internally.
         */
        void setValue( const BibTeX::Value *value );

        /**
         * Return the BibTeXValue of this widget. The returned
         * variable is a deep copy of the widget's internal state.
         * The returned variable must be delete later by the
         * function caller.
         * The function will return NULL if no data is set by the
         * user for this widget.
         */
        BibTeX::Value *value();

        void setEnabled( bool enabled );

        void setFieldType( BibTeX::EntryField::FieldType fieldType );

        QString caption();

        bool isEmpty();

        bool isModified();

        ErrorType error();

    signals:
        void textChanged( );

    protected slots:
        void updateGUI();

    private slots:
        void slotTextChanged();
        void slotStringToggled();
        void slotComplexClicked();

    private:
        BibTeX::Value *m_value;
        QString m_caption;
        KLineEdit *m_lineEdit;
        QTextEdit *m_textEdit;
        QPushButton *m_pushButtonString;
        QPushButton *m_pushButtonComplex;
        bool m_isReadOnly;
        bool m_enabled;
        InputType m_inputType;
        bool m_isModified;
        BibTeX::EntryField::FieldType m_fieldType;
        KCompletion *m_completion;

        void setupGUI( const char *name );
        void enableSignals( bool enabled );
    };

}

#endif
