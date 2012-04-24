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
#ifndef COMMENTWIDGET_H
#define COMMENTWIDGET_H

#include <qwidget.h>
#include <qdialog.h>

#include <comment.h>

class QMultiLineEdit;
class QCheckBox;

namespace KBibTeX
{
    class CommentWidget : public QWidget
    {
        Q_OBJECT
    public:
        CommentWidget( BibTeX::Comment *comment, bool isReadOnly, QWidget *parent = 0, const char *name = 0 );
        ~CommentWidget();

        static QDialog::DialogCode execute( BibTeX::Comment *comment, bool isReadOnly, QWidget *parent = 0, const char *name = 0 );

    private:
        BibTeX::Comment* m_comment;
        QCheckBox *m_checkboxUseCommand;
        QMultiLineEdit *m_multiLineEdit;
        bool m_isReadOnly;

        void setupGUI();
        void setCommentData();
        void getCommentData();
    };

}

#endif
