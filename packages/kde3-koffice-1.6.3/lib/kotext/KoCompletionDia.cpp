/* This file is part of the KDE project
   Copyright (C) 2005 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoCompletionDia.h"
#include "KoAutoFormat.h"

#include <klocale.h>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include <kcompletion.h>
#include <kconfig.h>
#include <kdebug.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qwhatsthis.h>

KoCompletionDia::KoCompletionDia( QWidget *parent, const char *name, KoAutoFormat * autoFormat )
    : KDialogBase( parent, name , true, i18n( "Completion" ), Ok|Cancel|User1,
      Ok, true, KGuiItem( i18n( "&Reset" ), "undo" ) )
{
    QVBox *page = makeVBoxMainWidget();
    m_widget = new KoCompletion(page, autoFormat);
    m_widget->layout()->setMargin(0);
    connect( this, SIGNAL( user1Clicked() ), m_widget, SLOT(slotResetConf()));
    setButtonWhatsThis(Ok,i18n("This will save your options."));
    setButtonWhatsThis(Cancel,i18n("This will abort all changes."));
    setButtonWhatsThis(User1,i18n("This will reset to the state after you clicked on the Make Default button."));
}

void KoCompletionDia::slotOk()
{
    m_widget->saveSettings();
    KDialogBase::slotOk();
}

KoCompletion::KoCompletion(QWidget *parent, KoAutoFormat *autoFormat) : KoCompletionBase(parent),
      m_autoFormat( *autoFormat ),
      m_docAutoFormat( autoFormat )
{
    connect(cbAllowCompletion, SIGNAL(toggled ( bool )), this, SLOT( changeButtonStatus()));
    QStringList lst;
    lst << i18n( "Enter" );
    lst << i18n( "Tab" );
    lst << i18n( "Space" );
    lst << i18n( "End" );
    lst << i18n( "Right" );
    m_completionKeyAction->insertStringList( lst );

    connect( m_lbListCompletion, SIGNAL( selected ( const QString & ) ), this, SLOT( slotCompletionWordSelected( const QString & )));
    connect( m_lbListCompletion, SIGNAL( highlighted ( const QString & ) ), this, SLOT( slotCompletionWordSelected( const QString & )));

    connect( pbAddCompletionEntry, SIGNAL( clicked() ), this, SLOT( slotAddCompletionEntry()));
    connect( pbRemoveCompletionEntry, SIGNAL( clicked() ), this, SLOT( slotRemoveCompletionEntry()));
    connect( pbSaveCompletionEntry, SIGNAL( clicked() ), this, SLOT( slotSaveCompletionEntry()));

    slotResetConf(); // aka load config
    changeButtonStatus();
}

void KoCompletion::changeButtonStatus() {
    bool state = cbAllowCompletion->isChecked();

    completionBox->setEnabled( state);
    cbAddCompletionWord->setEnabled( state );
    pbAddCompletionEntry->setEnabled( state );
    m_lbListCompletion->setEnabled( state );
    state = state && (m_lbListCompletion->count()!=0 && !m_lbListCompletion->currentText().isEmpty());
    pbRemoveCompletionEntry->setEnabled( state );
}

void KoCompletion::slotResetConf() {
    cbAllowCompletion->setChecked( m_autoFormat.getConfigCompletion());
    cbShowToolTip->setChecked( m_autoFormat.getConfigToolTipCompletion());
    cbAddCompletionWord->setChecked( m_autoFormat.getConfigAddCompletionWord());
    m_lbListCompletion->clear();
    m_listCompletion = m_docAutoFormat->listCompletion();
    m_lbListCompletion->insertStringList( m_listCompletion );
    m_lbListCompletion->sort();
    if( m_listCompletion.isEmpty() || m_lbListCompletion->currentText().isEmpty())
        pbRemoveCompletionEntry->setEnabled( false );
    m_minWordLength->setValue ( m_docAutoFormat->getConfigMinWordLength() );
    m_maxNbWordCompletion->setValue ( m_docAutoFormat->getConfigNbMaxCompletionWord() );
    cbAppendSpace->setChecked( m_autoFormat.getConfigAppendSpace() );

    switch( m_docAutoFormat->getConfigKeyAction() )
    {
    case KoAutoFormat::Enter:
        m_completionKeyAction->setCurrentItem( 0 );
        break;
    case KoAutoFormat::Tab:
        m_completionKeyAction->setCurrentItem( 1 );
        break;
    case KoAutoFormat::Space:
        m_completionKeyAction->setCurrentItem( 2 );
        break;
    case KoAutoFormat::End:
        m_completionKeyAction->setCurrentItem( 3 );
        break;
    case KoAutoFormat::Right:
        m_completionKeyAction->setCurrentItem( 4 );
        break;
    default:
        m_completionKeyAction->setCurrentItem( 0 );
    }
    changeButtonStatus();
}

void KoCompletion::slotAddCompletionEntry() {
    bool ok;
    QString const newWord = KInputDialog::getText( i18n("Add Completion Entry"), i18n("Enter entry:"), QString::null, &ok, this ).lower();
    if ( ok )
    {
        if ( !m_listCompletion.contains( newWord ))
        {
            m_listCompletion.append( newWord );
            m_lbListCompletion->insertItem( newWord );
            pbRemoveCompletionEntry->setEnabled( !m_lbListCompletion->currentText().isEmpty() );
            m_lbListCompletion->sort();
        }

    }
}

void KoCompletion::slotRemoveCompletionEntry() {
    QString text = m_lbListCompletion->currentText();
    if( !text.isEmpty() )
    {
        m_listCompletion.remove( text );
        m_lbListCompletion->removeItem( m_lbListCompletion->currentItem () );
        if( m_lbListCompletion->count()==0 )
            pbRemoveCompletionEntry->setEnabled( false );
    }
}

void KoCompletion::slotCompletionWordSelected( const QString & word) {
    pbRemoveCompletionEntry->setEnabled( !word.isEmpty() );
}

void KoCompletion::saveSettings() {
    m_docAutoFormat->configCompletion( cbAllowCompletion->isChecked());
    m_docAutoFormat->configToolTipCompletion( cbShowToolTip->isChecked());
    m_docAutoFormat->configAppendSpace( cbAppendSpace->isChecked() );
    m_docAutoFormat->configMinWordLength( m_minWordLength->value() );
    m_docAutoFormat->configNbMaxCompletionWord( m_maxNbWordCompletion->value () );
    m_docAutoFormat->configAddCompletionWord( cbAddCompletionWord->isChecked());

    m_docAutoFormat->getCompletion()->setItems( m_listCompletion );
    m_docAutoFormat->updateMaxWords();
    switch( m_completionKeyAction->currentItem() ) {
        case 1:
            m_docAutoFormat->configKeyCompletionAction( KoAutoFormat::Tab );
            break;
        case 2:
            m_docAutoFormat->configKeyCompletionAction( KoAutoFormat::Space );
            break;
        case 3:
            m_docAutoFormat->configKeyCompletionAction( KoAutoFormat::End );
            break;
        case 4:
            m_docAutoFormat->configKeyCompletionAction( KoAutoFormat::Right );
            break;
        case 0:
        default:
            m_docAutoFormat->configKeyCompletionAction( KoAutoFormat::Enter );
    }

    // Save to config file
    m_docAutoFormat->saveConfig();
}

void KoCompletion::slotSaveCompletionEntry() {
    KConfig config("kofficerc");
    KConfigGroupSaver cgs( &config, "Completion Word" );
    config.writeEntry( "list", m_listCompletion );
    config.sync();
    KMessageBox::information( this, i18n(
            "Completion list saved.\nIt will be used for all documents "
            "from now on."), i18n("Completion List Saved") );
}

#include "KoCompletionDia.moc"
