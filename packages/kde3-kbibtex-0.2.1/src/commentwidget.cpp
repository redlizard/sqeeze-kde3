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
#include <qmultilineedit.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <kdialogbase.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <comment.h>
#include <element.h>

#include "commentwidget.h"

namespace KBibTeX
{
    CommentWidget::CommentWidget( BibTeX::Comment *comment, bool isReadOnly, QWidget *parent, const char *name )
            : QWidget( parent, name ), m_comment( comment ), m_isReadOnly( isReadOnly )
    {
        setupGUI();
        getCommentData();
        m_multiLineEdit->setReadOnly( isReadOnly );
    }


    CommentWidget::~CommentWidget()
    {
        // nothing
    }

    void CommentWidget::setupGUI()
    {
        setMinimumWidth( 384 );
        QVBoxLayout * layout = new QVBoxLayout( this, 0, KDialog::spacingHint() );

        QLabel *label = new QLabel( i18n( "Co&mment:" ), this );
        layout->addWidget( label );

        m_multiLineEdit = new QMultiLineEdit( this );
        m_multiLineEdit->setFont( KGlobalSettings::fixedFont() );
        layout->addWidget( m_multiLineEdit );
        label->setBuddy( m_multiLineEdit );
        m_multiLineEdit->setReadOnly( m_isReadOnly );

        m_checkboxUseCommand = new QCheckBox( i18n( "&Use @Comment for comment instead of plain text" ), this );
        layout->addWidget( m_checkboxUseCommand );
    }

    void CommentWidget::getCommentData()
    {
        m_multiLineEdit->setText( m_comment->text() );
        m_checkboxUseCommand->setChecked( m_comment->useCommand() );
    }

    void CommentWidget::setCommentData()
    {
        m_comment->setText( m_multiLineEdit->text() );
        m_comment->setUseCommand( m_checkboxUseCommand->isChecked() );
    }

    QDialog::DialogCode CommentWidget::execute( BibTeX::Comment *comment, bool isReadOnly, QWidget *parent, const char *name )
    {
        KDialogBase * dlg = new KDialogBase( parent, name, true, i18n( "Edit BibTeX Comment" ), KDialogBase::Ok | KDialogBase::Cancel );
        CommentWidget* ui = new CommentWidget( comment, isReadOnly, dlg, "kbibtex::commentwidget" );
        dlg->setMainWidget( ui );

        QDialog::DialogCode result = ( QDialog::DialogCode ) dlg->exec();
        if ( !isReadOnly && result == QDialog::Accepted )
            ui->setCommentData();

        delete( ui );
        delete( dlg );

        return result;
    }
}

#include "commentwidget.moc"

