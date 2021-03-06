/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoCustomVariablesDia.h"
#include "KoCustomVariablesDia.moc"

#include <klocale.h>
#include <kbuttonbox.h>

#include <qcombobox.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qheader.h>
#include <klineedit.h>
#include <kdebug.h>

/******************************************************************
 *
 * Class: KoVariableNameDia
 *
 ******************************************************************/

KoVariableNameDia::KoVariableNameDia( QWidget *parent )
    : KDialogBase( parent, "", TRUE,i18n( "Entry Name" ),Ok|Cancel )
{
    init();
}


KoVariableNameDia::KoVariableNameDia( QWidget *parent, const QPtrList<KoVariable>& vars )
    : KDialogBase( parent, "", TRUE, i18n( "Variable Name" ), Ok|Cancel )
{

    init();
    enableButtonOK(false);
    QPtrListIterator<KoVariable> it( vars );
     for ( ; it.current() ; ++it ) {
        KoVariable *var = it.current();
        if ( var->type() == VT_CUSTOM )
            names->insertItem( ( (KoCustomVariable*) var )->name(), -1 );
    }

}

void KoVariableNameDia::init()
{
    back = makeVBoxMainWidget();

    QHBox *row1 = new QHBox( back );
    row1->setSpacing( KDialog::spacingHint() );

    QLabel *l = new QLabel( i18n( "Name:" ), row1 );
    l->setFixedSize( l->sizeHint() );
    names = new QComboBox( TRUE, row1 );
    names->setFocus();

    connect( names, SIGNAL( textChanged ( const QString & )),
             this, SLOT( textChanged ( const QString & )));
    connect( this, SIGNAL( okClicked() ),
             this, SLOT( accept() ) );
    connect( this, SIGNAL( cancelClicked() ),
             this, SLOT( reject() ) );
    enableButtonOK( !names->currentText().isEmpty() );
    resize( 350, 100 );
}

QString KoVariableNameDia::getName() const
{
    return names->currentText();
}

void KoVariableNameDia::textChanged ( const QString &_text )
{
    enableButtonOK(!_text.isEmpty());
}

/******************************************************************
 *
 * Class: KoCustomVariablesListItem
 *
 ******************************************************************/

KoCustomVariablesListItem::KoCustomVariablesListItem( QListView *parent )
    : QListViewItem( parent )
{
    editWidget = new KLineEdit( listView()->viewport() );
    listView()->addChild( editWidget );
}

void KoCustomVariablesListItem::setup()
{
    QListViewItem::setup();
    setHeight( QMAX( listView()->fontMetrics().height(),
                     editWidget->sizeHint().height() ) );
    //if ( listView()->columnWidth( 1 ) < editWidget->sizeHint().width() )
    //    listView()->setColumnWidth( 1, editWidget->sizeHint().width() );
}

void KoCustomVariablesListItem::update()
{
    editWidget->resize( listView()->header()->cellSize( 1 ), height() );
    listView()->moveChild( editWidget, listView()->header()->cellPos( 1 ),
                           listView()->itemPos( this ) + listView()->contentsY() );
    editWidget->show();
}

void KoCustomVariablesListItem::setVariable( KoCustomVariable *v )
{
    var = v;
    editWidget->setText( var->value() );
    setText( 0, v->name() );
}

KoCustomVariable *KoCustomVariablesListItem::getVariable() const
{
    return var;
}

void KoCustomVariablesListItem::applyValue()
{
    QString newVal=editWidget->text();
    if(var->value()!=newVal)
        var->setValue( newVal );
}

int KoCustomVariablesListItem::width( const QFontMetrics & fm, const QListView *lv, int c ) const
{
    // The text of the 2nd column isn't known to QListViewItem, only we know it
    // (it's in our lineedit)
    if ( c == 1 ) {
        QString val = editWidget->text();
        int w = fm.width( val );
        return w;
    } else
        return QListViewItem::width( fm, lv, c );
}

/******************************************************************
 *
 * Class: KoCustomVariablesList
 *
 ******************************************************************/

KoCustomVariablesList::KoCustomVariablesList( QWidget *parent )
    : KListView( parent )
{
    header()->setMovingEnabled( FALSE );
    addColumn( i18n( "Variable" ) );
    addColumn( i18n( "Value" ) );
    connect( header(), SIGNAL( sizeChange( int, int, int ) ),
             this, SLOT( columnSizeChange( int, int, int ) ) );
    connect( header(), SIGNAL( sectionClicked( int ) ),
             this, SLOT( sectionClicked( int ) ) );

    setResizeMode(QListView::LastColumn);
    setSorting( -1 );
}

void KoCustomVariablesList::setValues()
{
    QListViewItemIterator it( this );
    for ( ; it.current(); ++it )
        ( (KoCustomVariablesListItem *)it.current() )->applyValue();
}

void KoCustomVariablesList::columnSizeChange( int c, int, int )
{
    if ( c == 0 || c == 1 )
        updateItems();
}

void KoCustomVariablesList::sectionClicked( int )
{
    updateItems();
}

void KoCustomVariablesList::updateItems()
{
    QListViewItemIterator it( this );
    for ( ; it.current(); ++it )
        ( (KoCustomVariablesListItem*)it.current() )->update();
}

/******************************************************************
 *
 * Class: KoCustomVariablesDia
 *
 ******************************************************************/

KoCustomVariablesDia::KoCustomVariablesDia( QWidget *parent, const QPtrList<KoVariable> &variables )
    : KDialogBase( parent, "", TRUE,i18n( "Variable Value Editor" ), Ok|Cancel )
{

    back = makeVBoxMainWidget();

    list = new KoCustomVariablesList( back );

    QStringList lst;
    QPtrListIterator<KoVariable> it( variables );
    for ( ; it.current() ; ++it ) {
        KoVariable *var = it.current();
        if ( var->type() == VT_CUSTOM ) {
            KoCustomVariable *v = (KoCustomVariable*)var;
            if ( !lst.contains( v->name() ) ) {
                lst.append( v->name() );
                KoCustomVariablesListItem *item = new KoCustomVariablesListItem( list );
                item->setVariable( v );
            }
        }
    }


    connect( this, SIGNAL( okClicked() ),
             this, SLOT( slotOk() ) );
    connect( this, SIGNAL( cancelClicked() ),
             this, SLOT( reject() ) );
    showButtonOK(lst.count()>0);

    resize( 600, 400 );
}

void KoCustomVariablesDia::slotOk()
{
    list->setValues();
    accept();
}

/******************************************************************
 *
 * Class: KoCustomVarDialog
 *
 ******************************************************************/

KoCustomVarDialog::KoCustomVarDialog( QWidget *parent )
    : KDialogBase( parent, "", TRUE,i18n( "Add Variable" ), Ok|Cancel )
{
    init();
    m_name->setFocus();


    connect( this, SIGNAL( okClicked() ),
             this, SLOT( slotAddOk() ) );
    connect( this, SIGNAL( cancelClicked() ),
             this, SLOT( reject() ) );

    connect( m_name, SIGNAL( textChanged(const QString&) ),
             this, SLOT( slotTextChanged(const QString&) ) );

    enableButtonOK( false );
    resize( 350, 100 );

}
// edit existing variable
KoCustomVarDialog::KoCustomVarDialog( QWidget *parent, KoCustomVariable *var )
    : KDialogBase( parent, "", TRUE,i18n( "Edit Variable" ), Ok|Cancel )
{
    m_var = var;
    init();
    m_name->setText( m_var->name() );
    m_value->setText( m_var->value() );
    m_name->setReadOnly(true);
    m_value->setFocus();


    connect( this, SIGNAL( okClicked() ),
             this, SLOT( slotEditOk() ) );
    connect( this, SIGNAL( cancelClicked() ),
             this, SLOT( reject() ) );

    connect( m_value, SIGNAL( textChanged(const QString&) ),
             this, SLOT( slotTextChanged(const QString&) ) );

    enableButtonOK( true );
    resize( 350, 100 );
}

void KoCustomVarDialog::init()
{
    back = makeVBoxMainWidget();
    QHBox *row1 = new QHBox( back );
    row1->setSpacing( KDialog::spacingHint() );
    QLabel *ln = new QLabel( i18n( "Name:" ), row1 );
    ln->setFixedSize( ln->sizeHint() );
    m_name = new KLineEdit( row1 );

    QHBox *row2 = new QHBox( back );
    row2->setSpacing( KDialog::spacingHint() );
    QLabel *lv = new QLabel( i18n( "Value:" ), row2 );
    lv->setFixedSize( lv->sizeHint() );
    m_value = new KLineEdit( row2 );
}

void KoCustomVarDialog::slotAddOk()
{
    accept();
}
void KoCustomVarDialog::slotEditOk()
{
    m_var->setValue( m_value->text() );
    accept();
}

void KoCustomVarDialog::slotTextChanged(const QString&text)
{
    enableButtonOK( !text.isEmpty() );
}
QString KoCustomVarDialog::name()
{
    if ( m_name->text().isEmpty() )
        return QString( "No name" );
    return m_name->text();
}

QString KoCustomVarDialog::value()
{
    if ( m_value->text().isEmpty() )
        return QString( "No value" );
    return m_value->text();
}
