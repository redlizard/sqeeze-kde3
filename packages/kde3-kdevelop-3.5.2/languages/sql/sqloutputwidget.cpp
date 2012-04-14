/***************************************************************************
 *   Copyright (C) 2003 by Harald Fernengel                                *
 *   harry@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qsqldatabase.h>
#include <qsqlerror.h>
#include <qsqlcursor.h>
#include <qsqldriver.h>
#include <qsqlrecord.h>
#include <qwidgetstack.h>
#include <qdatatable.h>
#include <qtextedit.h>
#include <qlayout.h>
#include <qstylesheet.h>

#include <klocale.h>

#include "sqloutputwidget.h"

class QCustomSqlCursor: public QSqlCursor
{
public:
    QCustomSqlCursor( const QString & query = QString::null, bool autopopulate = TRUE, QSqlDatabase* db = 0 ) :
                QSqlCursor( QString::null, autopopulate, db )
    {
        exec( query );
        if ( isSelect() && autopopulate ) {
            QSqlRecordInfo inf = ((QSqlQuery*)this)->driver()->recordInfo( *(QSqlQuery*)this );
            for ( QSqlRecordInfo::iterator it = inf.begin(); it != inf.end(); ++it ) {
                append( *it );
            }
        }
        setMode( QSqlCursor::ReadOnly );
    }
    QCustomSqlCursor( const QCustomSqlCursor & other ): QSqlCursor( other ) {}
    bool select( const QString & /*filter*/, const QSqlIndex & /*sort*/ = QSqlIndex() )
        { return exec( lastQuery() ); }
    QSqlIndex primaryIndex( bool /*prime*/ = TRUE ) const
        { return QSqlIndex(); }
    int insert( bool /*invalidate*/ = TRUE )
        { return FALSE; }
    int update( bool /*invalidate*/ = TRUE )
        { return FALSE; }
    int del( bool /*invalidate*/ = TRUE )
        { return FALSE; }
    void setName( const QString& /*name*/, bool /*autopopulate*/ = TRUE ) {}
};


SqlOutputWidget::SqlOutputWidget ( QWidget* parent, const char* name ) :
    QWidget( parent, name )
{
    m_stack = new QWidgetStack( this );
    m_table = new QDataTable( this );
    m_textEdit = new QTextEdit( this );

    m_textEdit->setTextFormat( QTextEdit::RichText );
    m_textEdit->setReadOnly( true );
    
    m_stack->addWidget( m_textEdit );
    m_stack->addWidget( m_table );

    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->addWidget( m_stack );
}

SqlOutputWidget::~SqlOutputWidget()
{}

void SqlOutputWidget::showQuery( const QString& connectionName, const QString& query )
{
    QSqlDatabase* db = QSqlDatabase::database( connectionName, true );
    if ( !db ) {
        showError( i18n("No such connection: %1").arg( connectionName ) );
        return;
    }
    if ( !db->isOpen() ) {
        showError( db->lastError() );
        return;
    }

    QSqlCursor* cur = new QCustomSqlCursor( query, true, db );
    if ( !cur->isActive() ) {
        showError( cur->lastError() );
    } else if ( cur->isSelect() ) {
        m_table->setSqlCursor( cur, true, true );
        m_table->refresh( QDataTable::RefreshAll );
        m_stack->raiseWidget( m_table );
    } else {
        showSuccess( cur->numRowsAffected() );
    }
}

void SqlOutputWidget::showSuccess( int rowsAffected )
{
    m_textEdit->clear();
    m_textEdit->setText( i18n("Query successful, number of rows affected: %1").arg( rowsAffected ) );
    m_stack->raiseWidget( m_textEdit );
}

void SqlOutputWidget::showError( const QString& message )
{
    m_textEdit->clear();
    m_textEdit->setText( "<p><b>" + i18n("An error occurred:") + "</b></p>\n" + message );
    m_stack->raiseWidget( m_textEdit );
}

void SqlOutputWidget::showError( const QSqlError& message )
{
    m_textEdit->clear();
    m_textEdit->setText( "<p><b>" + i18n("An error occurred:") + 
                         "</b></p>\n<p><i>" + i18n("Driver") + "</i>: " + 
                         QStyleSheet::escape( message.driverText() ) + 
                         "<br><i>" + i18n("Database") + ":</i>: " +
                         QStyleSheet::escape( message.databaseText() ) );
    m_stack->raiseWidget( m_textEdit );
}

#include "sqloutputwidget.moc"

