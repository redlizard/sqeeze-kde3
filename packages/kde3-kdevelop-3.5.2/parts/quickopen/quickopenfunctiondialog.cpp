/*
 *  Copyright (C) 2004 Ahn, Duk J.(adjj22@kornet.net) (adjj1@hanmail.net)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

#include <klocale.h>
#include <qlabel.h>
#include <qvaluelist.h>

#include <kcompletion.h>
#include <kdebug.h>
#include <klistbox.h>
#include <kmessagebox.h>
#include <klineedit.h>

#include <kdevplugin.h>
#include <codemodel_utils.h>
#include <kdevpartcontroller.h>
#include <kdevproject.h>
#include <kdevlanguagesupport.h>
#include <qregexp.h>


#include "quickopenfunctionchooseform.h"
#include "quickopenfunctiondialog.h"

QuickOpenFunctionDialog::QuickOpenFunctionDialog( QuickOpenPart *part, QWidget* parent, const char* name, bool modal, WFlags fl)
: QuickOpenDialog(part, parent, name, modal, fl)
{
        nameLabel->setText( i18n("Function &name:") );
        itemListLabel->setText( i18n("Function &list:") );

        fillItemList();
        
        itemList->insertStringList( wildCardCompletion( "" ) );

        nameEdit->setFocus();

        itemList->setCurrentItem( 0 );
}

void QuickOpenFunctionDialog::fillItemList() {
    m_items.clear();
    m_functionDefList.clear();
    FileList fileList = m_part->codeModel()->fileList();

    // for each one file, get all functions
    FileDom fileDom;
    for( FileList::Iterator it = fileList.begin() ; it!=fileList.end() ; ++it ){
            fileDom = *it;
            FunctionDefinitionList defs = CodeModelUtils::allFunctionDefinitionsDetailed( fileDom ).functionList;
            if( defs.isEmpty() ) {
              m_functionDefList += CodeModelUtils::allFunctionsDetailed( fileDom ).functionList;
            } else {
              for( FunctionDefinitionList::iterator it = defs.begin(); it != defs.end(); ++it )
                (m_functionDefList).append( (*it).data() );
            }
    }

    for( FunctionList::const_iterator it = m_functionDefList.begin(); it != m_functionDefList.end(); ++it )
      m_items << (*it)->name();
    QStringList_unique( m_items );
}

QuickOpenFunctionDialog::~QuickOpenFunctionDialog()
{
}

void QuickOpenFunctionDialog::gotoFile( QString name )
{
        FunctionModel *fmodel;
        FunctionList funcList;
        FunctionDom fdom;

        for( FunctionList::ConstIterator it = m_functionDefList.begin() ; it!=m_functionDefList.end() ; ++it ){
                fdom = *it;
                fmodel = fdom.data();
                if( fmodel->name() == name ){
                        funcList.append( fdom );
                }
        }
        if( funcList.count() == 1 ){
                fdom = funcList.first();
                fmodel = fdom.data();
                QString fileNameStr = fmodel->fileName();
                int startline, startcol;
                fmodel->getStartPosition( &startline, &startcol );
                m_part->partController()->editDocument( KURL( fileNameStr), startline, startcol );
                selectClassViewItem( ItemDom(&(*fmodel)) );

        }else if( funcList.count() > 1 ){
                QString fileStr;

                QuickOpenFunctionChooseForm fdlg( this, name.ascii() );

                for( FunctionList::Iterator it = funcList.begin() ; it!=funcList.end() ; ++it ){
                        fmodel = (*it).data();

                        fdlg.argBox->insertItem( m_part->languageSupport()->formatModelItem(fmodel) +
                                (fmodel->scope().isEmpty() ? "" : "   (in " + fmodel->scope().join("::") + ")"));
                        fileStr = KURL( fmodel->fileName() ).fileName();
                        KURL full_url( fmodel->fileName() );
                        KURL base_url( part()->project()->projectDirectory()+"/" );
                        fdlg.setRelativePath(fdlg.fileBox->count(),
                            KURL::relativeURL( base_url, full_url ));
                        fdlg.fileBox->insertItem(fileStr);
                }
                if( fdlg.exec() ){
                        int id = fdlg.argBox->currentItem();
                        if( id>-1 && id < (int) funcList.count() ){
                                FunctionModel *model = funcList[id].data();
                                int line, col;
                                model->getStartPosition( &line, &col );
                                selectClassViewItem( ItemDom(&(*model)) );
                                QString fileNameStr = model->fileName();
                                m_part->partController()->editDocument( KURL(fileNameStr), line );
                        }
                }
        }
        else{
                KMessageBox::error( this, i18n("Error: cannot find matching name function.") );
        }

        accept();
}

void QuickOpenFunctionDialog::slotExecuted(QListBoxItem* item)
{
        if( item ){
                gotoFile( item->text() );
        }
}

void QuickOpenFunctionDialog::executed(QListBoxItem*)
{
}

void QuickOpenFunctionDialog::itemSelectionChanged() {
      QString text = nameEdit->text();
      QString txt = text;
      QStringList parts = QStringList::split("::", text);
      if( !text.endsWith( "::" ) && !parts.isEmpty() )
        parts.pop_back();
      parts << itemList->currentText();
      nameEdit->setText(parts.join("::"));
}

void QuickOpenFunctionDialog::slotTextChangedDelayed() {
    QString text = nameEdit->text();
    QString txt = text;
    QStringList parts = QStringList::split("::", text);
    if(text.endsWith("::") || parts.isEmpty()) {
        txt = "";
    }else{
        txt = parts.back();
        parts.pop_back();
    }
    QValueList<QRegExp> regExpParts;
    for( QStringList::const_iterator it = parts.begin(); it != parts.end(); ++it ) {
      regExpParts << QRegExp( *it, false, true );
    }

    QString scope = parts.join("::");

    if( m_scope != scope ) {
        if( !scope.startsWith(m_scope) ) { ///Not a specialization, so reload all function-definitions
          fillItemList();
        }

        if(!parts.isEmpty()) {
            FunctionList accepted;
            QStringList acceptedItems;
            FunctionList::iterator it = m_functionDefList.begin();
            while(it != m_functionDefList.end()) {
                QStringList scope = (*it)->scope();
                QValueList<QRegExp>::iterator mit = regExpParts.begin();
                QStringList::iterator sit = scope.begin();
                bool fail = false;
                while(mit != regExpParts.end()) {
                    while(sit != scope.end() && !(*mit).exactMatch( *sit ) ) ++sit;
                    if(sit == scope.end()) {
                        fail = true;
                        break;
                    }
                    ++mit;
                }
                if(!fail) {
                  accepted.append(*it);
                  acceptedItems << (*it)->name();
                }
                ++it;
            }
            m_functionDefList = accepted;
            m_items = acceptedItems;
            QStringList_unique( m_items );
        }

        m_scope = scope;
    }
    
    itemList->clear();
    itemList->insertStringList( wildCardCompletion( txt ) );
    itemList->setCurrentItem(0);
}

void QuickOpenFunctionDialog::slotReturnPressed()
{
        maybeUpdateSelection();
        QListBoxItem *listboxItem = itemList->selectedItem();
        slotExecuted( listboxItem );
}

#include "quickopenfunctiondialog.moc"

