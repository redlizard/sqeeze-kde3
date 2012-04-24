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
#include <qlayout.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qobjectlist.h>
#include <qscrollview.h>

#include <kglobalsettings.h>
#include <klineedit.h>
#include <kpopupmenu.h>
#include <kcombobox.h>
#include <kpushbutton.h>
#include <kdialogbase.h>
#include <kiconloader.h>
#include <klocale.h>

#include <idsuggestions.h>
#include <fileimporter.h>
#include <fileimporterbibtex.h>
#include "idsuggestionswidget.h"

#define min(a,b)  ((a)>(b)?(b):(a))
#define max(a,b)  ((a)<(b)?(b):(a))

namespace KBibTeX
{
    QString IdSuggestionsWidget::exampleBibTeXEntry = "@Article{ dijkstra1983terminationdetect,\nauthor = {Edsger W. Dijkstra and W. H. J. Feijen and A. J. M. {van Gasteren}},\ntitle = {{Derivation of a Termination Detection Algorithm for Distributed Computations}},\njournal = {Information Processing Letters},\nvolume = 16,\nnumber = 5,\npages = {217--219},\nmonth = jun,\nyear = 1983\n}";

    IdSuggestionComponent::IdSuggestionComponent( const QString& title, QWidget *parent ): QFrame( parent ), m_title( title ), m_parent( parent )
    {
        setFrameShape( QFrame::Panel );
        setFrameShadow( QFrame::Sunken );
        setLineWidth( 1 );
    }

    QWidget *IdSuggestionComponent::moveWidgets( QWidget *parent )
    {
        QWidget *container = new QWidget( parent );
        QVBoxLayout *layout = new QVBoxLayout( container, 0, KDialog::spacingHint() );
        m_pushButtonUp = new KPushButton( QIconSet( SmallIcon( "up" ) ), i18n( "Up" ), container );
        m_pushButtonDown = new KPushButton( QIconSet( SmallIcon( "down" ) ), i18n( "Down" ), container );
        m_pushButtonDel = new KPushButton( QIconSet( SmallIcon( "remove" ) ), i18n( "Delete" ), container );
        layout->addWidget( m_pushButtonUp );
        layout->addWidget( m_pushButtonDown );
        layout->addWidget( m_pushButtonDel );
        layout->addStretch( 10 );
        connect( m_pushButtonUp, SIGNAL( clicked() ), this, SLOT( slotUp() ) );
        connect( m_pushButtonDown, SIGNAL( clicked() ), this, SLOT( slotDown() ) );
        connect( m_pushButtonDel, SIGNAL( clicked() ), this, SLOT( slotDelete() ) );
        return container;
    }

    void IdSuggestionComponent::setEnableUpDown( bool enableUp, bool enableDown )
    {
        m_pushButtonUp->setEnabled( enableUp );
        m_pushButtonDown->setEnabled( enableDown );
    }

    void IdSuggestionComponent::slotUp()
    {
        QVBoxLayout *layout = dynamic_cast<QVBoxLayout*>( m_parent->layout() );
        if ( layout == NULL ) return;
        int oldPos = layout->findWidget( this );
        if ( oldPos > 0 )
        {
            layout->remove( this );
            layout->insertWidget( oldPos - 1, this );
            emit moved();
        }
    }

    void IdSuggestionComponent::slotDown()
    {
        QVBoxLayout *layout = dynamic_cast<QVBoxLayout*>( m_parent->layout() );
        if ( layout == NULL ) return;
        int oldPos = layout->findWidget( this );
        int componentCount = dynamic_cast<IdSuggestionsWidget*>( m_parent->parent()->parent()->parent() )->numComponents();
        if ( oldPos < componentCount - 1 )
        {
            layout->remove( this );
            layout->insertWidget( oldPos + 1, this );
            emit moved();
        }
    }

    void IdSuggestionComponent::slotDelete()
    {
        emit deleted();
        delete this;
    }

    IdSuggestionComponentAuthor::IdSuggestionComponentAuthor( const QString &text, QWidget *parent ): IdSuggestionComponent( i18n( "Author" ), parent )
    {
        QGridLayout *layout = new QGridLayout( this, 6, 4, KDialog::marginHint(), KDialog::spacingHint() );

        QLabel *label = new QLabel( m_title, this );
        QFont labelFont( label->font() );
        labelFont.setBold( TRUE );
        label->setFont( labelFont );
        label->setBackgroundColor( KGlobalSettings::highlightColor() );
        label->setPaletteForegroundColor( KGlobalSettings::highlightedTextColor() );
        label->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
        layout->addMultiCellWidget( label, 0, 0, 0, 2 );

        m_checkBoxFirstAuthorOnly = new QCheckBox( i18n( "Use first author only" ), this );
        layout->addMultiCellWidget( m_checkBoxFirstAuthorOnly, 1, 1, 0, 1 );
        m_checkBoxFirstAuthorOnly->setChecked( text[0] == 'a' );
        connect( m_checkBoxFirstAuthorOnly, SIGNAL( toggled( bool ) ), SIGNAL( modified() ) );

        struct IdSuggestionTokenInfo info = IdSuggestions::evalToken( text.mid( 1 ) );

        label = new QLabel( i18n( "Casing:" ), this );
        layout->addWidget( label, 2, 0 );
        m_comboBoxCasing = new KComboBox( FALSE, this );
        label->setBuddy( m_comboBoxCasing );
        layout->addWidget( m_comboBoxCasing, 2, 1 );
        m_comboBoxCasing->insertItem( i18n( "No change" ) );
        m_comboBoxCasing->insertItem( i18n( "Lower case" ) );
        m_comboBoxCasing->insertItem( i18n( "Upper case" ) );
        if ( info.toLower )
            m_comboBoxCasing->setCurrentItem( 1 );
        else if ( info.toUpper )
            m_comboBoxCasing->setCurrentItem( 2 );
        else
            m_comboBoxCasing->setCurrentItem( 0 );
        connect( m_comboBoxCasing, SIGNAL( activated( const QString& ) ), SIGNAL( modified() ) );

        label = new QLabel( i18n( "Only first letters:" ), this );
        layout->addWidget( label, 3, 0 );
        m_spinBoxLen = new QSpinBox( this );
        label->setBuddy( m_spinBoxLen );
        layout->addWidget( m_spinBoxLen, 3, 1 );
        m_spinBoxLen->setMinValue( 0 );
        m_spinBoxLen->setMaxValue( 9 );
        m_spinBoxLen->setSpecialValueText( i18n( "Complete name" ) );
        m_spinBoxLen->setValue( info.len > 9 ? 0 : info.len );
        m_spinBoxLen->setMinimumWidth( m_spinBoxLen->fontMetrics().width( i18n( "Complete name" ) ) + 32 );
        connect( m_spinBoxLen, SIGNAL( valueChanged( int ) ), SIGNAL( modified() ) );

        label = new QLabel( i18n( "Text between authors:" ), this );
        layout->addWidget( label, 4, 0 );
        m_lineEditInBetween = new KLineEdit( this );
        label->setBuddy( m_lineEditInBetween );
        layout->addWidget( m_lineEditInBetween, 4, 1 );
        m_lineEditInBetween->setText( info.inBetween );
        connect( m_lineEditInBetween, SIGNAL( textChanged( const QString& ) ), SIGNAL( modified() ) );

        layout->setRowStretch( 5, 1 );
        layout->setColStretch( 1, 1 );
        layout->setColSpacing( 2, KDialog::spacingHint() * 3 );
        layout->addMultiCellWidget( moveWidgets( this ), 0, 5, 3, 3 );
    }

    QString IdSuggestionComponentAuthor::text() const
    {
        QString result = m_checkBoxFirstAuthorOnly->isChecked() ? "a" : "A";
        if ( m_spinBoxLen->value() > 0 && m_spinBoxLen->value() <= 9 ) result.append( QString::number( m_spinBoxLen->value() ) );
        if ( m_comboBoxCasing->currentItem() == 1 ) result.append( "l" );
        else if ( m_comboBoxCasing->currentItem() == 2 ) result.append( "u" );
        if ( !m_lineEditInBetween->text().isEmpty() ) result.append( '"' ).append( m_lineEditInBetween->text() );

        return result;
    }

    IdSuggestionComponentTitle::IdSuggestionComponentTitle( const QString &text, QWidget *parent ): IdSuggestionComponent( i18n( "Title" ), parent )
    {
        QGridLayout *layout = new QGridLayout( this, 6, 4, KDialog::marginHint(), KDialog::spacingHint() );

        QLabel *label = new QLabel( m_title, this );
        QFont labelFont( label->font() );
        labelFont.setBold( TRUE );
        label->setFont( labelFont );
        label->setBackgroundColor( KGlobalSettings::highlightColor() );
        label->setPaletteForegroundColor( KGlobalSettings::highlightedTextColor() );
        label->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
        layout->addMultiCellWidget( label, 0, 0, 0, 2 );

        m_checkBoxRemoveSmallWords = new QCheckBox( i18n( "Remove small words" ), this );
        layout->addMultiCellWidget( m_checkBoxRemoveSmallWords, 1, 1, 0, 1 );
        m_checkBoxRemoveSmallWords->setChecked( text[0] == 'T' );
        connect( m_checkBoxRemoveSmallWords, SIGNAL( toggled( bool ) ), SIGNAL( modified() ) );

        struct IdSuggestionTokenInfo info = IdSuggestions::evalToken( text.mid( 1 ) );

        label = new QLabel( i18n( "Casing:" ), this );
        layout->addWidget( label, 2, 0 );
        m_comboBoxCasing = new KComboBox( FALSE, this );
        label->setBuddy( m_comboBoxCasing );
        layout->addWidget( m_comboBoxCasing, 2, 1 );
        m_comboBoxCasing->insertItem( i18n( "No change" ) );
        m_comboBoxCasing->insertItem( i18n( "Lower case" ) );
        m_comboBoxCasing->insertItem( i18n( "Upper case" ) );
        if ( info.toLower )
            m_comboBoxCasing->setCurrentItem( 1 );
        else if ( info.toUpper )
            m_comboBoxCasing->setCurrentItem( 2 );
        else
            m_comboBoxCasing->setCurrentItem( 0 );
        connect( m_comboBoxCasing, SIGNAL( textChanged( const QString& ) ), SIGNAL( modified() ) );

        label = new QLabel( i18n( "Only first letters:" ), this );
        layout->addWidget( label, 3, 0 );
        m_spinBoxLen = new QSpinBox( this );
        label->setBuddy( m_spinBoxLen );
        layout->addWidget( m_spinBoxLen, 3, 1 );
        m_spinBoxLen->setMinValue( 0 );
        m_spinBoxLen->setMaxValue( 9 );
        m_spinBoxLen->setSpecialValueText( i18n( "Complete title" ) );
        m_spinBoxLen->setValue( info.len > 9 ? 0 : info.len );
        m_spinBoxLen->setMinimumWidth( m_spinBoxLen->fontMetrics().width( i18n( "Complete title" ) ) + 32 );
        connect( m_spinBoxLen, SIGNAL( valueChanged( int ) ), SIGNAL( modified() ) );

        label = new QLabel( i18n( "Text between words:" ), this );
        layout->addWidget( label, 4, 0 );
        m_lineEditInBetween = new KLineEdit( this );
        label->setBuddy( m_lineEditInBetween );
        layout->addWidget( m_lineEditInBetween, 4, 1 );
        m_lineEditInBetween->setText( info.inBetween );
        connect( m_lineEditInBetween, SIGNAL( textChanged( const QString& ) ), SIGNAL( modified() ) );

        layout->setRowStretch( 5, 1 );
        layout->setColStretch( 1, 1 );
        layout->setColSpacing( 2, KDialog::spacingHint() *2 );
        layout->addMultiCellWidget( moveWidgets( this ), 0, 5, 3, 3 );
    }

    QString IdSuggestionComponentTitle::text() const
    {
        QString result = m_checkBoxRemoveSmallWords->isChecked() ? "T" : "t";
        if ( m_spinBoxLen->value() > 0 && m_spinBoxLen->value() <= 9 ) result.append( QString::number( m_spinBoxLen->value() ) );
        if ( m_comboBoxCasing->currentItem() == 1 ) result.append( "l" );
        else if ( m_comboBoxCasing->currentItem() == 2 ) result.append( "u" );
        if ( !m_lineEditInBetween->text().isEmpty() ) result.append( '"' ).append( m_lineEditInBetween->text() );

        return result;
    }

    IdSuggestionComponentYear::IdSuggestionComponentYear( const QString &text, QWidget *parent ): IdSuggestionComponent( i18n( "Year" ), parent )
    {
        QGridLayout *layout = new QGridLayout( this, 3, 4, KDialog::marginHint(), KDialog::spacingHint() );

        QLabel *label = new QLabel( m_title, this );
        QFont labelFont( label->font() );
        labelFont.setBold( TRUE );
        label->setFont( labelFont );
        label->setBackgroundColor( KGlobalSettings::highlightColor() );
        label->setPaletteForegroundColor( KGlobalSettings::highlightedTextColor() );
        label->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
        layout->addMultiCellWidget( label, 0, 0, 0, 2 );

        label = new QLabel( i18n( "Year:" ), this );
        layout->addWidget( label, 1, 0 );
        m_comboBoxDigits = new KComboBox( this );
        label->setBuddy( m_comboBoxDigits );
        layout->addWidget( m_comboBoxDigits, 1, 1 );
        m_comboBoxDigits->insertItem( i18n( "2 digits" ) );
        m_comboBoxDigits->insertItem( i18n( "4 digits" ) );
        m_comboBoxDigits->setCurrentItem( text[0] == 'y' ? 0 : 1 );
        connect( m_comboBoxDigits, SIGNAL( textChanged( const QString& ) ), SIGNAL( modified() ) );

        layout->setColStretch( 1, 1 );
        layout->setRowStretch( 2, 1 );
        layout->setColSpacing( 2, KDialog::spacingHint() *2 );
        layout->addMultiCellWidget( moveWidgets( this ), 0, 2, 3, 3 );
    }

    QString IdSuggestionComponentYear::text() const
    {
        return m_comboBoxDigits->currentItem() == 0 ? "y" : "Y";
    }

    IdSuggestionComponentText::IdSuggestionComponentText( const QString &text, QWidget *parent ): IdSuggestionComponent( i18n( "Text" ), parent )
    {
        QGridLayout *layout = new QGridLayout( this, 3, 4, KDialog::marginHint(), KDialog::spacingHint() );

        QLabel *label = new QLabel( m_title, this );
        QFont labelFont( label->font() );
        labelFont.setBold( TRUE );
        label->setFont( labelFont );
        label->setBackgroundColor( KGlobalSettings::highlightColor() );
        label->setPaletteForegroundColor( KGlobalSettings::highlightedTextColor() );
        label->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
        layout->addMultiCellWidget( label, 0, 0, 0, 2 );

        label = new QLabel( i18n( "Text in between:" ), this );
        layout->addWidget( label, 1, 0 );
        m_lineEditInBetween = new KLineEdit( this );
        label->setBuddy( m_lineEditInBetween );
        layout->addWidget( m_lineEditInBetween, 1, 1 );
        m_lineEditInBetween->setText( text.mid( 1 ) );

        layout->setColStretch( 1, 1 );
        layout->setRowStretch( 2, 1 );
        layout->setColSpacing( 2, KDialog::spacingHint() * 2 );
        layout->addMultiCellWidget( moveWidgets( this ), 0, 2, 3, 3 );
    }

    QString IdSuggestionComponentText::text() const
    {
        return m_lineEditInBetween->text().isEmpty() ? QString::null : QString( "\"" ).append( m_lineEditInBetween->text() );
    }

    IdSuggestionsScrollView::IdSuggestionsScrollView( QWidget *parent, const char*name ): QScrollView( parent, name ), m_widget( NULL )
    {
        setMinimumHeight( 256 );
        setHScrollBarMode( QScrollView::AlwaysOff );
        setVScrollBarMode( QScrollView::AlwaysOn );
        setLineWidth( 0 );
    }

    IdSuggestionsScrollView::~IdSuggestionsScrollView()
    {
        // nothing
    }

    void IdSuggestionsScrollView::viewportResizeEvent( QResizeEvent * )
    {
        if ( m_widget != NULL )
            m_widget->setFixedWidth( viewport()->width() );
    }

    IdSuggestionsWidget::IdSuggestionsWidget( const QString &formatStr, KDialogBase *parent, const char *name )
            : QWidget( parent, name ), m_originalFormatStr( formatStr ), m_parent( parent )
    {
        BibTeX::FileImporter *importer = new BibTeX::FileImporterBibTeX( false );
        BibTeX::File *file = importer->load( exampleBibTeXEntry );
        m_example = new BibTeX::Entry( dynamic_cast<BibTeX::Entry*>( *( file->begin() ) ) );
        delete file;
        delete importer;

        setupGUI();
        reset( formatStr );
        updateExample();
    }

    IdSuggestionsWidget::~IdSuggestionsWidget()
    {
        // nothing
    }

    int IdSuggestionsWidget::numComponents()
    {
        return m_componentCount;
    }

    QDialog::DialogCode IdSuggestionsWidget::execute( QString &formatStr, QWidget *parent, const char *name )
    {
        KDialogBase * dlg = new KDialogBase( parent, name, true, i18n( "Edit Id Suggestions" ), KDialogBase::Ok | KDialogBase::Cancel );
        IdSuggestionsWidget* ui = new IdSuggestionsWidget( formatStr, dlg, "IdSuggestionsWidget" );
        dlg->setMainWidget( ui );

        QDialog::DialogCode result = ( QDialog::DialogCode ) dlg->exec();
        if ( result == QDialog::Accepted )
            ui->apply( formatStr );

        delete( ui );
        delete( dlg );

        return result;
    }

    void IdSuggestionsWidget::reset( const QString& formatStr )
    {
        QLayoutIterator it = m_listOfComponents->layout()->iterator();
        QLayoutItem *child;
        while (( child = it.current() ) != 0 )
        {
            IdSuggestionComponent *component = dynamic_cast<IdSuggestionComponent*>( child->widget() );
            ++it;
            if ( component != NULL )
                delete component;
        }

        m_componentCount = 0;
        QStringList lines = QStringList::split( '|', formatStr );
        for ( QStringList::Iterator it = lines.begin(); it != lines.end();++it )
        {
            IdSuggestionComponent *component = NULL;
            if (( *it )[0] == 'a' || ( *it )[0] == 'A' )
                component = new IdSuggestionComponentAuthor( *it, m_listOfComponents );
            else if (( *it )[0] == 't' || ( *it )[0] == 'T' )
                component = new IdSuggestionComponentTitle( *it, m_listOfComponents );
            else if (( *it )[0] == 'y' || ( *it )[0] == 'Y' )
                component = new IdSuggestionComponentYear( *it, m_listOfComponents );
            else if (( *it )[0] == '"' )
                component = new IdSuggestionComponentText( *it, m_listOfComponents );

            if ( component != NULL )
            {
                ++m_componentCount;
                connect( component, SIGNAL( moved() ), this, SLOT( componentsMoved() ) );
                connect( component, SIGNAL( deleted() ), this, SLOT( componentDeleted() ) );
                connect( component, SIGNAL( moved() ), this, SLOT( updateExample() ) );
                connect( component, SIGNAL( deleted() ), this, SLOT( updateExample() ) );
                connect( component, SIGNAL( modified() ), this, SLOT( updateExample() ) );
            }
        }

        m_listOfComponents->adjustSize();
        m_scrollViewComponents->verticalScrollBar()->adjustSize();
        componentsMoved();
        m_scrollViewComponents->resize( m_scrollViewComponents->width(), min( 384, m_listOfComponents->height() + 2 ) );
    }

    void IdSuggestionsWidget::apply( QString& formatStr )
    {
        bool first = TRUE;
        formatStr = "";
        QLayoutIterator it = m_listOfComponents->layout()->iterator();
        QLayoutItem *child;
        while (( child = it.current() ) != 0 )
        {
            IdSuggestionComponent *component = dynamic_cast<IdSuggestionComponent*>( child->widget() );
            QString text = QString::null;
            if ( component != NULL && ( text = component->text() ) != QString::null )
            {
                if ( first ) first = FALSE; else formatStr.append( "|" );
                formatStr.append( text );
            }
            ++it;
        }
    }

    void IdSuggestionsWidget::setupGUI()
    {
        QGridLayout *gridLayout = new QGridLayout( this, 3, 2, 0, KDialog::spacingHint() );
        gridLayout->setRowStretch( 2, 1 );
        gridLayout->setColStretch( 0, 1 );

        m_labelExample = new QLabel( this );
        gridLayout->addMultiCellWidget( m_labelExample, 0, 1, 0, 0 );

        m_pushButtonAdd = new KPushButton( i18n( "Add" ), this );
        gridLayout->addWidget( m_pushButtonAdd, 1, 1 );

        m_scrollViewComponents = new IdSuggestionsScrollView( this );
        m_listOfComponents = new QWidget( m_scrollViewComponents->viewport() );
        m_scrollViewComponents->setMainWidget( m_listOfComponents );
        m_scrollViewComponents->addChild( m_listOfComponents );
        gridLayout->addMultiCellWidget( m_scrollViewComponents, 2, 2, 0, 1 );
        QVBoxLayout *listLayout = new QVBoxLayout( m_listOfComponents, 0, KDialog::spacingHint() );
        listLayout->setAutoAdd( TRUE );

        KPopupMenu *addMenu = new KPopupMenu( m_pushButtonAdd );
        addMenu->insertItem( i18n( "Author" ), 1 );
        addMenu->insertItem( i18n( "Year" ), 2 );
        addMenu->insertItem( i18n( "Title" ), 3 );
        addMenu->insertItem( i18n( "Text" ), 4 );
        connect( addMenu, SIGNAL( activated( int ) ), this, SLOT( addMenuActivated( int ) ) );
        m_pushButtonAdd->setPopup( addMenu );
    }

    void IdSuggestionsWidget::addMenuActivated( int id )
    {
        IdSuggestionComponent *comp = NULL;
        if ( id == 1 )
            comp = new IdSuggestionComponentAuthor( "a", m_listOfComponents );
        else if ( id == 2 )
            comp = new IdSuggestionComponentYear( "y", m_listOfComponents );
        else if ( id == 3 )
            comp = new IdSuggestionComponentTitle( "t", m_listOfComponents );
        else if ( id == 4 )
            comp = new IdSuggestionComponentText( "", m_listOfComponents );

        if ( comp != NULL )
        {
            ++m_componentCount;

            comp->show();

            connect( comp, SIGNAL( moved() ), this, SLOT( componentsMoved() ) );
            connect( comp, SIGNAL( deleted() ), this, SLOT( componentDeleted() ) );
            connect( comp, SIGNAL( moved() ), this, SLOT( updateExample() ) );
            connect( comp, SIGNAL( deleted() ), this, SLOT( updateExample() ) );
            connect( comp, SIGNAL( modified() ), this, SLOT( updateExample() ) );
            m_listOfComponents->adjustSize();
            m_scrollViewComponents->ensureVisible( 10, m_listOfComponents->height() - 2 );
            componentsMoved();
        }
    }

    void IdSuggestionsWidget::componentsMoved()
    {
        QLayoutIterator it = m_listOfComponents->layout()->iterator();
        QLayoutItem *child = NULL;
        IdSuggestionComponent *lastComponent = NULL;
        bool first = TRUE;
        int i = 0;
        while (( child = it.current() ) != 0 )
        {
            IdSuggestionComponent *component = dynamic_cast<IdSuggestionComponent*>( child->widget() );
            ++it;
            if ( component != NULL )
            {
                if ( first )
                {
                    first = FALSE;
                    component->setEnableUpDown( FALSE, m_componentCount > 1 );
                }
                else
                    component->setEnableUpDown( TRUE, i < m_componentCount - 1 );

                ++i;
                lastComponent = component;
            }
        }

        if ( lastComponent != NULL )
        {
            lastComponent->setEnableUpDown( m_componentCount > 1, FALSE );
        }

        m_scrollViewComponents->updateGeometry();
        m_parent->enableButtonOK( m_componentCount > 0 );
    }

    void IdSuggestionsWidget::componentDeleted()
    {
        --m_componentCount;
        componentsMoved();
    }

    void IdSuggestionsWidget::updateExample()
    {
        QString formatStr;
        apply( formatStr );
        QString formatted = IdSuggestions::formatId( m_example, formatStr );
        m_labelExample->setText( QString( i18n( "<qt>Example:<br/><b>%1</b></qt>" ) ).arg( formatted ) );
    }

}
#include "idsuggestionswidget.moc"
