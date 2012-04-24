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
#ifndef KBIBTEXIDSUGGESTIONSWIDGET_H
#define KBIBTEXIDSUGGESTIONSWIDGET_H

#include <qwidget.h>
#include <qdialog.h>

class QCheckBox;
class QScrollView;
class QSpinBox;
class KComboBox;
class KLineEdit;
class KPushButton;

namespace KBibTeX
{
    class IdSuggestionComponent: public QFrame
    {
        Q_OBJECT
    public:
        IdSuggestionComponent( const QString& title, QWidget *parent );
        virtual QString text() const = 0;
        void setEnableUpDown( bool enableUp, bool enableDown );

    signals:
        void modified();
        void moved();
        void deleted();

    protected:
        QWidget *moveWidgets( QWidget *parent );

        QString m_title;
        QWidget *m_parent;
        KPushButton *m_pushButtonDel;
        KPushButton *m_pushButtonUp;
        KPushButton *m_pushButtonDown;

    protected slots:
        void slotUp();
        void slotDown();
        void slotDelete();
    };

    class IdSuggestionComponentAuthor: public IdSuggestionComponent
    {
    public:
        IdSuggestionComponentAuthor( const QString &text, QWidget *parent );
        QString text() const;
    protected:
        QCheckBox *m_checkBoxFirstAuthorOnly;
        KComboBox *m_comboBoxCasing;
        KLineEdit *m_lineEditInBetween;
        QSpinBox *m_spinBoxLen;
    };

    class IdSuggestionComponentTitle: public IdSuggestionComponent
    {
    public:
        IdSuggestionComponentTitle( const QString &text, QWidget *parent );
        QString text() const;
    protected:
        QCheckBox *m_checkBoxRemoveSmallWords;
        KComboBox *m_comboBoxCasing;
        KLineEdit *m_lineEditInBetween;
        QSpinBox *m_spinBoxLen;
    };

    class IdSuggestionComponentYear: public IdSuggestionComponent
    {
    public:
        IdSuggestionComponentYear( const QString &text, QWidget *parent );
        QString text() const;
    protected:
        KComboBox *m_comboBoxDigits;
    };

    class IdSuggestionComponentText: public IdSuggestionComponent
    {
    public:
        IdSuggestionComponentText( const QString &text, QWidget *parent );
        QString text() const;
    protected:
        KLineEdit *m_lineEditInBetween;
    };

    class IdSuggestionsScrollView : public QScrollView
    {
        Q_OBJECT
    public:
        IdSuggestionsScrollView( QWidget *parent, const char*name = NULL );
        ~IdSuggestionsScrollView();

        void setMainWidget( QWidget *widget ) {m_widget = widget;}

    protected:
        void viewportResizeEvent( QResizeEvent * );

    private:
        QWidget *m_widget;
    };

    /**
     @author Thomas Fischer <fischer@unix-ag.uni-kl.de>
    */
    class IdSuggestionsWidget : public QWidget
    {
        Q_OBJECT
    public:
        ~IdSuggestionsWidget();
        int numComponents();

        static QDialog::DialogCode execute( QString &formatStr, QWidget *parent = 0, const char *name = 0 );

    protected:
        IdSuggestionsWidget( const QString &formatStr, KDialogBase *parent, const char *name = 0 );

        QString m_originalFormatStr;
        int m_componentCount;

        void reset( const QString& formatStr );
        void apply( QString& formatStr );

    private:
        static QString exampleBibTeXEntry;
        BibTeX::Entry *m_example;
        IdSuggestionsScrollView *m_scrollViewComponents;
        QLabel *m_labelExample;
        QWidget *m_listOfComponents;
        KPushButton *m_pushButtonAdd;
        KDialogBase *m_parent;

        void setupGUI();

    private slots:
        void addMenuActivated( int id );
        void componentsMoved();
        void componentDeleted();
        void updateExample();
    };

}

#endif
