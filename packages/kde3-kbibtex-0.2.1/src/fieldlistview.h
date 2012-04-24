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
#ifndef KBIBTEXFIELDLISTVIEW_H
#define KBIBTEXFIELDLISTVIEW_H

#include <qwidget.h>

#include <value.h>
#include <entryfield.h>

class QString;
class KListView;
class QListViewItem;
class QPushButton;
class QCheckBox;

namespace KBibTeX
{

    /**
     @author Thomas Fischer <fischer@unix-ag.uni-kl.de>
    */
    class FieldListView : public QWidget
    {
        Q_OBJECT
    public:
        FieldListView( const QString& caption, const QString& prefixNew, bool isReadOnly, QWidget *parent = 0, const char *name = 0 );
        ~FieldListView();

        void setValue( const BibTeX::Value *value );
        BibTeX::Value *value();

        void setEnabled( bool enabled );

        void setFieldType( BibTeX::EntryField::FieldType fieldType );

        QString caption();

        bool isEmpty();

        bool isModified();

        virtual bool eventFilter( QObject *o, QEvent * e );

    private slots:
        void updateGUI();
        void slotAdd();
        void slotEdit();
        void slotDelete();
        void slotUp();
        void slotDown();
        void slotComplex();
        void slotListViewDoubleClicked( QListViewItem * );
        void slotItemRenamed( QListViewItem * item, int col, const QString & text );
        void apply();

    private:
        BibTeX::Value *m_value;
        QString m_caption;
        QString m_prefixNew;
        bool m_isReadOnly;
        bool m_enabled;
        bool m_isComplex;
        bool m_isModified;
        int m_newValueCounter;
        BibTeX::EntryField::FieldType m_fieldType;
        KListView *m_listViewElements;
        QPushButton *m_pushButtonAdd;
        QPushButton *m_pushButtonEdit;
        QPushButton *m_pushButtonDelete;
        QPushButton *m_pushButtonUp;
        QPushButton *m_pushButtonDown;
        QPushButton *m_pushButtonComplexEdit;
        QCheckBox *m_checkBoxEtAl;

        void setupGUI();
        void reset();
        void itemRenameDone();
        bool isSimple();
    };

}

#endif
