//
// C++ Implementation: %{MODULE}
//
// Description:
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//

// Qt includes
#include <qfile.h>
#include <qvariant.h>

// KDE includes
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>

// App specific includes
#include "Types.h" 
#include "contextmenuhandler.h"


/**
 * XML-ActionHandler
 */
ContextActionHandler::ContextActionHandler(ContextMenuHandler *menuhandler)
 : QXmlDefaultHandler(), m_menuhandler(menuhandler)
{
    m_subName = QString::null;
    m_actionName = QString::null;
    m_qty = QString::null;
    m_popup = menuhandler->m_popupmenu;
    m_hit = false;
    m_searchID = QString::null;
} 

ContextActionHandler::~ContextActionHandler()
{
}


void ContextActionHandler::setSearchID( const QString xmlID )
{
    m_searchID = xmlID;
}
    

bool ContextActionHandler::startElement( const QString &,
            const QString &,
            const QString &qName,
            const QXmlAttributes &atts )
{
    if ( qName == "Item" ){
        if ( atts.value("id") == m_searchID ){
            if ( atts.value("editable") == "yes"){
                m_menuhandler->setItemEditable( true );
            } 
            if ( atts.value("maxlines") != ""){
                m_menuhandler->setItemMaxlines( atts.value("maxlines") );
            }
            m_hit = true;   
        } else {
            m_hit = false;
        }
    }
    if ( !(m_hit && m_menuhandler->m_popupmenu) )
        return true;
        
    if ( qName == "Action" ){
        m_actionName = "";
        m_qty = atts.value("qty");
    } else if ( qName == "Submenu" ){
        if ( atts.value("name") != "" ){
            m_subName = atts.value("name");
            m_popup = m_menuhandler->SubMenuFactory(m_popup);
        }       
    }
    return true;    
}    
    

bool ContextActionHandler::endElement( const QString &,
            const QString &,
            const QString &qName )
{
    if ( !(m_hit && m_menuhandler->m_popupmenu) )
        return true;
        
    if ( qName == "Action" ){
        KAction* newAction = m_menuhandler->ActionFactory(m_actionName, m_qty);
        if ( newAction )
            newAction->plug( m_popup );
    } else if ( qName == "Submenu" ){
        if ( m_subName == "New" ){
            m_menuhandler->m_popupmenu->insertItem( i18n("New"), m_popup);    
        }
        // only one sublevel spported -> back to toplevel menu
        m_popup = m_menuhandler->m_popupmenu;   
    }
    return true;
}
            

bool ContextActionHandler::characters( const QString &ch )
{ 
    m_actionName += ch;
    return true;
}



/**
 * MenuHandler
 */
ContextMenuHandler::ContextMenuHandler(QObject *parent, const char *name)
 : QObject(parent, name), m_DocTreeView(parent)
{
    m_popupmenu = NULL;
    m_item = NULL;  
    initActions();
    m_XmlFilePath = KGlobal::dirs()->findResource("data", "ksayit/ContextMenus.xml");
}


ContextMenuHandler::~ContextMenuHandler()
{
    delete renameItem;
    delete deleteItem;
    delete newBookInfo;
    delete newChapter;
    delete newKeywordSet;
    delete newKeyword;
    delete newAbstract;
    delete newAuthorGroup;
    delete newAuthor;
    delete newDate;
    delete newReleaseInfo;
    delete newTitle;
    delete newParagraph;
    delete newSection_1;
    delete newSection_2;
    delete newSection_3;
    delete newSection_4;
    delete newSection_5;

    if (m_popupmenu)
       delete m_popupmenu;
}


void ContextMenuHandler::initActions()
{
  // User defined actions
  renameItem = new KAction (i18n("Rename..."),
              0,
              m_DocTreeView, SLOT (slotRenameItem()), NULL );
  
  deleteItem = new KAction (i18n("Delete..."),
              0,
              m_DocTreeView, SLOT (slotDeleteItem()), NULL );
              
  newBookInfo = new KAction ( i18n("Overview"),
              0,
              m_DocTreeView, SLOT (slotNewBookInfo()), NULL );

  newChapter = new KAction ( i18n("Chapter"),
              0,
              m_DocTreeView, SLOT (slotNewChapter()), NULL );

  newKeywordSet = new KAction ( i18n("Keywords"),
              0,
              m_DocTreeView, SLOT (slotNewKeywordSet()), NULL );

  newKeyword = new KAction ( i18n("Keyword"),
              0,
              m_DocTreeView, SLOT (slotNewKeyword()), NULL );
  
  newAbstract = new KAction ( i18n("Abstract"),
              0,
              m_DocTreeView, SLOT (slotNewAbstract()), NULL );
  
  newAuthorGroup = new KAction ( i18n("Authors"),
              0,
              m_DocTreeView, SLOT (slotNewAuthorGroup()), NULL );
  
  newAuthor = new KAction ( i18n("Author"),
              0,
              m_DocTreeView, SLOT (slotNewAuthor()), NULL );
  
  newDate = new KAction ( i18n("Date"),
              0,
              m_DocTreeView, SLOT (slotNewDate()), NULL );
  
  newReleaseInfo = new KAction ( i18n("Release Info"),
              0,
              m_DocTreeView, SLOT (slotNewReleaseInfo()), NULL );

  newTitle = new KAction ( i18n("Title"),
              0,
              m_DocTreeView, SLOT (slotNewTitle()), NULL );

  newParagraph = new KAction ( i18n("Paragraph"),
              0,
              m_DocTreeView, SLOT (slotNewParagraph()), NULL );

  newSection_1 = new KAction ( i18n("Section Level 1"),
              0,
              m_DocTreeView, SLOT (slotNewSection_1()), NULL );

  newSection_2 = new KAction ( i18n("Section Level 2"),
              0,
              m_DocTreeView, SLOT (slotNewSection_2()), NULL );

  newSection_3 = new KAction ( i18n("Section Level 3"),
              0,
              m_DocTreeView, SLOT (slotNewSection_3()), NULL );

  newSection_4 = new KAction ( i18n("Section Level 4"),
              0,
              m_DocTreeView, SLOT (slotNewSection_4()), NULL );

  newSection_5 = new KAction ( i18n("Section Level 5"),
              0,
              m_DocTreeView, SLOT (slotNewSection_5()), NULL );
}


KPopupMenu* ContextMenuHandler::getPopupMenu(ListViewInterface *item)
{
    if ( !item )
        return NULL;

    m_item = item;

    QString xmlID = ( item->getValue(KSayItGlobal::XMLCONTEXTNAME) ).toString();
    if ( xmlID.isNull() )
        return NULL;

    // delete old popup menu and create new one
    if ( m_popupmenu ){
        delete m_popupmenu;
        m_popupmenu = NULL;
    }
    m_popupmenu = new KPopupMenu(0);

    bool res;
    res = parseXmlFile(xmlID);

    if ( res ){
        return m_popupmenu;
    } else {
        delete m_popupmenu;
        m_popupmenu = NULL;
        return NULL;
    }
}


void ContextMenuHandler::registerPopupMenu(ListViewInterface *item)
{
    if ( !item )
        return;

    m_item = item;

    QString xmlID = ( item->getValue(KSayItGlobal::XMLCONTEXTNAME) ).toString();
    if ( xmlID.isNull() )
        return;

    // delete old popup menu
    if ( m_popupmenu ){
        delete m_popupmenu;
        m_popupmenu = NULL;
    }

    // if m_popupmenu == NULL, the parser should not create a popupmenu
    parseXmlFile(xmlID);
}


bool ContextMenuHandler::parseXmlFile(const QString &xmlID)
{
    kdDebug(100200) << "ContextMenuHandler::parseXmlFile()" << endl;
    kdDebug(100200) << "XML-File: " << m_XmlFilePath << endl;
     
    QFile xmlFile( m_XmlFilePath );
    QXmlSimpleReader parser;
    ContextActionHandler handler(this);
    handler.setSearchID(xmlID);
    parser.setContentHandler( &handler );
    return parser.parse( &xmlFile );     
}


KAction* ContextMenuHandler::ActionFactory( const QString &actionName, const QString &qty )
{
    bool enabled;
    if ( qty.lower() == "n" ){
        // unlimited don't check return enabled action
        enabled = true;
    } else {
        // check if qty is within allowed limits
        ListViewInterface *i = static_cast<ListViewInterface*>(m_item->firstChild());
        int count = 0;
        QString name = QString::null;
        while (i){
            name = ( i->getValue(KSayItGlobal::XMLCONTEXTNAME) ).toString();
            if ( name.lower() == actionName.lower() ){
                count++;
            }
            i = static_cast<ListViewInterface*>(i->nextSibling());
        }
        enabled = count < qty.toInt() ? true : false;
    }
    
    if ( actionName == "Rename" ){
        renameItem->setEnabled( enabled );
        return renameItem;
    } else if ( actionName == "Delete" ){
        deleteItem->setEnabled( enabled );
        return deleteItem;        
    } else if ( actionName == "BookInfo" ){
        newBookInfo->setEnabled( enabled );
        return newBookInfo;
    } else if ( actionName == "Chapter" ){
        newChapter->setEnabled( enabled );
        return newChapter;
    } else if ( actionName == "KeywordSet" ){
        newKeywordSet->setEnabled( enabled );
        return newKeywordSet;
    } else if ( actionName == "Keyword" ){
        newKeyword->setEnabled( enabled );
        return newKeyword;
    } else if ( actionName == "Abstract" ){
        newAbstract->setEnabled( enabled );
        return newAbstract;
    } else if ( actionName == "AuthorGroup" ){
        newAuthorGroup->setEnabled( enabled );
        return newAuthorGroup;
    } else if ( actionName == "Author" ){
        newAuthor->setEnabled( enabled );
        return newAuthor;
    } else if ( actionName == "Date" ){
        newDate->setEnabled( enabled );
        return newDate;
    } else if ( actionName == "ReleaseInfo" ){
        newReleaseInfo->setEnabled( enabled );
        return newReleaseInfo;
    } else if ( actionName == "Title" ){
        newTitle->setEnabled( enabled );
        return newTitle;
    } else if ( actionName == "Paragraph" ){
        newParagraph->setEnabled( enabled );
        return newParagraph;
    } else if ( actionName == "Section_1" ){
        newSection_1->setEnabled( enabled );
        return newSection_1;
    } else if ( actionName == "Section_2" ){
        newSection_2->setEnabled( enabled );        
        return newSection_2;
    } else if ( actionName == "Section_3" ){
        newSection_3->setEnabled( enabled );
        return newSection_3;
    } else if ( actionName == "Section_4" ){
        newSection_4->setEnabled( enabled );
        return newSection_4;
    } else if ( actionName == "Section_5" ){
        newSection_5->setEnabled( enabled );
        return newSection_5;
    }
    kdDebug(100200) << "ERROR!!! ContextMenuHandler::ActionFactory()" << endl;
    return NULL;
}


KPopupMenu* ContextMenuHandler::SubMenuFactory(KPopupMenu *parent)
{
    return new KPopupMenu(parent);
}


void ContextMenuHandler::setItemEditable( bool editable )
{
    if ( m_item )
        m_item->setValue(KSayItGlobal::ISEDITABLE, editable);        
}


void ContextMenuHandler::setItemMaxlines( const QString &maxlines )
{
    if ( maxlines == "" )
        return;
    
    if ( !m_item )
        return;
        
    int value = maxlines.toInt();
    m_item->setValue( KSayItGlobal::MAXLINES, value );
} 


#include "contextmenuhandler.moc"
