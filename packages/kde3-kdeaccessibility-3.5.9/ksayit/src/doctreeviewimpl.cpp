//
// C++ Implementation: doctreeviewimpl
//
// Description:
//
//
// Author: Robert Vogl <voglrobe@lapislazuli>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <stdlib.h> // getenv

// Qt includes
#include <qdom.h>
#include <qfile.h>
#include <qwidget.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <qdatetime.h>
#include <qcolor.h>
#include <qlistview.h>

//KDE includes
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kinputdialog.h>

// App specific includes
#include "doctreeviewimpl.h"
#include "contextmenuhandler.h"
#include "parasaxparser.h"
#include "docbookclasses.h"
#include "docbookgenerator.h"
#include "docbookparser.h"


//////////////////////////////////////
// TreeView Implementation
//////////////////////////////////////
DocTreeViewImpl::DocTreeViewImpl(QWidget* parent, const char* name, WFlags fl)
 : DocTreeView(parent, name, fl)
{
    m_rootItem = NULL;
    m_url = KURL();
    // the hidden formated id to set a sort order
    listView->setSorting(3, true);
    listView->setColumnText(0, i18n("Chapter"));
    listView->setColumnText(1, i18n("Info"));
    listView->setColumnText(2, i18n("Page"));
    listView->setColumnText(3, "");
    listView->setResizeMode( KListView::NoColumn );
    listView->setColumnWidthMode(0, KListView::Maximum );
    listView->setColumnWidthMode(1, KListView::Maximum );
    listView->setColumnWidthMode(2, KListView::Maximum );
    listView->setColumnWidthMode(3, KListView::Manual );
    listView->setColumnWidth(3, 0);
    listView->setAlternateBackground( QColor(230, 230, 240) );
    listView->setSelectionModeExt( KListView::Single );
    
    m_idCounter = KSayItGlobal::item_initial_id;
    m_stopped = false;
    m_currentItem = m_rootItem;
    m_parasaxparser = ParaSaxParser::Instance();
    m_editMode = false;
    m_changedContent = QString::null;
    
    m_contextmenuhandler = NULL;
    m_contextmenu = NULL;
}


DocTreeViewImpl::~DocTreeViewImpl()
{
    if ( m_rootItem )
        delete m_rootItem;
    delete m_parasaxparser;
    if ( m_contextmenuhandler )
        delete m_contextmenuhandler;
}


void DocTreeViewImpl::enableContextMenus( bool enabled )
{
    if ( enabled ){
        m_contextmenuhandler = new ContextMenuHandler(this, "contextmenuhandler");
    } else {
        if ( m_contextmenuhandler )
            delete m_contextmenuhandler;
        m_contextmenuhandler = NULL;
    }    
}


void DocTreeViewImpl::clear()
{
    kdDebug(100200) << "DocTreeViewImpl::clear()" << endl;
    m_url = KURL();
    
    if ( m_rootItem )
        delete m_rootItem;
    
    // inform BookmarkHandler
    // 
    emit signalSetBookmarkFilename( QString::null );
    
    listView->clear();
    m_rootItem = new RobDocument( listView, i18n("unnamed") );
    m_idCounter = KSayItGlobal::item_initial_id;
    m_stopped = false;
    m_currentItem = static_cast<ListViewInterface*>(m_rootItem);
}


void DocTreeViewImpl::createEmptyDocument()
{
    kdDebug(100200) << "DocTreeViewImpl::createEmptyDocument()" << endl;
    // create empty document
    QByteArray data;
    QTextStream wrapped(data, IO_ReadWrite);
    wrapped.setEncoding(QTextStream::UnicodeUTF8);

    wrapped << "<?xml version=\"1.0\" ?>" << endl;
    wrapped << "<!--" << endl;
    wrapped << "!DOCTYPE book PUBLIC \"-//KDE//DTD DocBook XML V4.2-Based Variant V1.1//EN\" \"dtd/kdex.dtd\"" << endl;
    wrapped << "-->" << endl;
    
    QString header(data);
    m_parasaxparser->setProcessingInstruction( header );
    
    wrapped << "<book>" << endl;
    wrapped << "<bookinfo>" << endl;
    wrapped << "<title><![CDATA[" << i18n("Empty Document") << "]]></title>" << endl;  
    wrapped << "<authorgroup>" << endl;
    wrapped << "<author>" << endl;
    wrapped << "<firstname>KSayIt</firstname>" << endl;
    wrapped << "<surname>Wrapper</surname>" << endl;
    wrapped << "</author>" << endl;
    wrapped << "</authorgroup>" << endl;
    wrapped << "<copyright>" << endl;
    wrapped << "<year>2005</year>" << endl;
    wrapped << "<holder>Robert Vogl</holder>" << endl;        
    wrapped << "</copyright>" << endl;
    wrapped << "</bookinfo>" << endl;       
    wrapped << "<chapter>" << endl;
    wrapped << "<title><![CDATA[" << i18n("Untitled") << "]]></title>" << endl;
    wrapped << "<para>" << endl;
    wrapped << "";
    wrapped << "</para>" << endl;
    wrapped << "</chapter>" << endl;
    wrapped << "</book>" << endl;
    
    QDomDocument domTree;
    if ( ! domTree.setContent( data ) ){
        kdDebug(100200) << "Kein gueltiges Dokument!!" << endl;
    };
    // inform BookmarkHandler
    emit signalSetBookmarkFilename( i18n("Untitled") );

    QDomElement root = domTree.documentElement();
    QDomNode node;  
    if( root.tagName().lower() == "book" ){ // DocBook
        DocbookParser docbookparser(m_contextmenuhandler);
        docbookparser.parseBook(root , m_rootItem);
        m_idCounter = docbookparser.getIdCounter();  
    }
    
    // Initial view
    m_currentItem = static_cast<ListViewInterface*>( listView->lastItem() );
    listView->setSelected( m_currentItem, true );
    listView->ensureItemVisible( m_currentItem );
    slotItemClicked( m_currentItem );
}


void DocTreeViewImpl::openFile(const KURL &url)
{
    kdDebug(100200) << "DocTreeViewImpl::openFile(" << url.path() << ")" << endl;

    m_url = url;
    // open file
    QString err;
    QFile file( url.path() );
    if( !file.open(IO_ReadOnly) ){
        err = i18n("Unable to open File.");
        throw(err);
    }
    QDomDocument domTree;
    // check document
    if( domTree.setContent(&file) ){
        // extract Header
        file.reset();
        QString header = QString::null;
        QString line;
        int offset;
        file.readLine( line, file.size() );
        while( !file.atEnd() && (offset = line.find("<book", 0, false)) < 0 ){
            header += line;    
            file.readLine( line, file.size() );  
        }
        file.close();
        header += line.left( offset );
        kdDebug(100200) << "### Header: " << endl << header << endl;       
        m_parasaxparser->setProcessingInstruction( header );
        // inform BookmarkHandler about the opened file
        emit signalSetBookmarkFilename( url.fileName() );
    } else {
        // File is not a valid XML-File. Wrap it
        file.reset();
        QByteArray data;
        QTextStream wrapped(data, IO_ReadWrite);
        wrapped.setEncoding(QTextStream::UnicodeUTF8);

        wrapped << "<?xml version=\"1.0\" ?>" << endl;
        wrapped << "<!--" << endl;
        wrapped << "!DOCTYPE book PUBLIC \"-//KDE//DTD DocBook XML V4.2-Based Variant V1.1//EN\" \"dtd/kdex.dtd\"" << endl;
        wrapped << "-->" << endl;
        
        QString header(data);
        kdDebug(100200) << "Header (Wrapper): " << endl << header << endl;
        m_parasaxparser->setProcessingInstruction( header );
        
        wrapped << "<book>" << endl;
        wrapped << "<bookinfo>" << endl;
        wrapped << "<title><![CDATA[" << url.path() << "]]></title>" << endl;
        wrapped << "<authorgroup>" << endl;
        wrapped << "<author>" << endl;
        wrapped << "<firstname>KSayIt</firstname>" << endl;
        wrapped << "<surname>Wrapper</surname>" << endl;
        wrapped << "</author>" << endl;
        wrapped << "</authorgroup>" << endl;
        wrapped << "<copyright>" << endl;
        wrapped << "<year>2005</year>" << endl;
        wrapped << "<holder>Robert Vogl</holder>" << endl;        
        wrapped << "</copyright>" << endl;
        wrapped << "</bookinfo>" << endl;       
        wrapped << "<chapter>" << endl;
        wrapped << "<title><![CDATA[" << i18n("Plain File") << "]]></title>" << endl;
        wrapped << "<para><![CDATA[";
        wrapped << QString( file.readAll() );
        wrapped << "]]></para>" << endl;
        wrapped << "</chapter>" << endl;
        wrapped << "</book>" << endl;
        file.close();
        
        // Try again
        if (! domTree.setContent(data)){
            err = i18n("Failed wrapping the file into XML.");
            throw(err);
        }
        emit signalSetBookmarkFilename( i18n("Plain File") );
    }

    // check if correct document type
    QDomElement root = domTree.documentElement();
    if( root.tagName().lower() == "book" ){ // DocBook
            DocbookParser docbookparser(m_contextmenuhandler);
            docbookparser.parseBook(root , m_rootItem);
            m_idCounter = docbookparser.getIdCounter();  
    } else {
        err = i18n("The file is of type %1, 'book' expected.").arg(root.tagName() );
        throw(err);
    }
    
    // Initial view
    listView->setSelected( m_rootItem, true );
    slotItemClicked( m_rootItem );
}


void DocTreeViewImpl::saveFile()
{
    kdDebug(100200) << "DocTreeViewImpl::saveFile()" << endl;
    
    makeCurrentNodePersistent();
    
    QString err;

    if ( m_url.isEmpty() ){ // file has no name, ask user
        QString usershome( getenv("HOME") );
        m_url = KFileDialog::getSaveURL(usershome, "*.docbook", this, i18n("Save File"));
    }
    if ( m_url.isEmpty() ) // dialog cancelled
        return;
    if ( !m_url.isValid() ){
        err = i18n("The given URL is invalid. Try 'File save as...' instead.");
        throw( err );
    }
    if ( m_url.isLocalFile() ){
        QFile file( m_url.path() );
        if ( !file.open(IO_WriteOnly) ){
            err = i18n("Unable open file to write.");
            throw( err );
        }
        // write file
        QTextStream doc(&file);
        doc.setEncoding(QTextStream::UnicodeUTF8);
        
        doc << "<?xml version=\"1.0\" ?>" << endl;
        doc << "<!--" << endl;
        doc << "!DOCTYPE book PUBLIC \"-//KDE//DTD DocBook XML V4.2-Based Variant V1.1//EN\" \"dtd/kdex.dtd\"" << endl;
        doc << "-->" << endl;
        
        DocbookGenerator generator;
        generator.writeBook( doc, m_rootItem );
        
        // update root item
        m_rootItem->setText(0, m_url.fileName() );
        m_rootItem->setValue(KSayItGlobal::RTFHEADER,     m_url.fileName());
        m_rootItem->setValue(KSayItGlobal::SPEAKERHEADER, m_url.fileName());        
        file.close();
    } else {
        err = i18n("Save operation currently works on local files only.");
        throw( err );
    }
    // inform BookmarkHandler
    emit signalChangeBookmarkFilename( m_url.fileName() );
    slotItemClicked( m_currentItem );
}


void DocTreeViewImpl::saveFileAs()
{
    kdDebug(100200) << "DocTreeViewImpl::saveFileAs()" << endl;
    QString err;

    makeCurrentNodePersistent();
    
    m_url = KFileDialog::getSaveURL(QString::null, "*.docbook", this, i18n("Save File As"));
    if ( m_url.isEmpty() ) // dialog cancelled
        return;
    if ( !m_url.isValid() ){
        err = i18n("The given URL is invalid.");
        throw( err );
    }
    if ( m_url.isLocalFile() ){
        QFile file( m_url.path() );
        if ( !file.open(IO_WriteOnly) ){
            err = i18n("Unable open file to write.");
            throw( err );
        }
        
        // write file
        QTextStream doc(&file);
        doc.setEncoding(QTextStream::UnicodeUTF8);

        doc << "<?xml version=\"1.0\" ?>" << endl;
        doc << "<!--" << endl;
        doc << "!DOCTYPE book PUBLIC \"-//KDE//DTD DocBook XML V4.2-Based Variant V1.1//EN\" \"dtd/kdex.dtd\"" << endl;
        doc << "-->" << endl;

        DocbookGenerator generator;
        generator.writeBook( doc, m_rootItem );

        // update root item
        m_rootItem->setText(0, m_url.fileName() );
        m_rootItem->setValue(KSayItGlobal::RTFHEADER,     m_url.fileName());
        m_rootItem->setValue(KSayItGlobal::SPEAKERHEADER, m_url.fileName());        
        
        file.close();
    } else {
        err = i18n("Save operation currently works on local files only.");
        throw( err );
    }
    // inform BookmarkHandler
    emit signalChangeBookmarkFilename( m_url.fileName() );
    slotItemClicked( m_currentItem );
}


void DocTreeViewImpl::setEditMode(bool mode)
{
    m_editMode = mode;
    bool editable = ( m_currentItem->getValue(KSayItGlobal::ISEDITABLE) ).toBool();   
    if ( editable && mode ){
        emit signalEnableTextedit( true );
        slotItemClicked( m_currentItem );
        return;
    }
    
    makeCurrentNodePersistent();
    slotItemClicked( m_currentItem );
 
    emit signalEnableTextedit( false );
}


void DocTreeViewImpl::makeToSingleLine( QString &content )
{
    // canonify string
    content.replace( QRegExp("\n"), "" );          // remove Newlines
    content.replace( QRegExp(" {2,}"), " " );      // remove multiple spaces
    content.replace( QRegExp("[\t|\r]{1,}"), "");  // remove Tabs
}    


QString DocTreeViewImpl::selectItemByID(const QString &ID, const QString title)
{
    kdDebug(100200) << "DocTreeViewImpl::selectItemByID(" << ID << ")" << endl;

    if ( ID.isNull() )
        return QString::null;
        
    QString sTitle = title.lower();
    QString err = QString::null;
    
    /**
     * Search on structure (URL)
     * e.g. 1=RobDokument|1=BookInfo|1=KeywordSet|2=Keyword
     */
    QStringList partList;
    partList = QStringList::split("|", ID);
    QStringList::Iterator it = partList.begin();
    it++; // skip first element (allways RobDocument)
    int childNum = 0;
    QString childType = QString::null;
    ListViewInterface *item = m_rootItem;
    bool hit = false;
    
    for (; it!=partList.end(); ++it){
        kdDebug(100200) << "Part: " << (*it) << endl;
        if ( !item )
            break; // expected item does not exist
        item = static_cast<ListViewInterface*>( item->firstChild() );
        if ( item ){
            childNum  = ((*it).section('=', 0, 0)).toInt();
            childType =  (*it).section('=', 1, 1);
            childType = childType.lower();
            for (int i=1; i<childNum; i++){
                item = static_cast<ListViewInterface*>( item->nextSibling() );
                if ( !item )
                    break; // expected item does not exist
            }
            if ( !item )
                break; // expected item does not exist
            QString type = ( item->getValue(KSayItGlobal::XMLCONTEXTNAME) ).toString();
            type = type.lower();
            if ( childType != type )
                break; // structure has been changed
        }
    }
    
    if ( item ){
        QString itemTitle = getItemTitle( item );
        itemTitle = itemTitle.lower();
        if ( sTitle.left(itemTitle.length()) == itemTitle )
            // Title made unique by KDE by appending "(2)", "(3)"...
            hit = true;
    }
    
    if ( !hit ){
        /**
        * Not found, search on title
        */
        // walk trough the entire tree and try to find an item
        // with the given title.
        item = NULL;
        QListViewItemIterator sit( m_rootItem );
        QString itemTitle = QString::null;
        while ( sit.current() ) {
            item = static_cast<ListViewInterface*>(sit.current());
            if ( !item )
                break; // expected item does not exist
            itemTitle = getItemTitle( item );
            itemTitle = itemTitle.lower();
            if ( sTitle.left(itemTitle.length()) == itemTitle ){
                hit = true;
                err = i18n("Maybe the structure of the document has been changed. Please check if this bookmark is still valid.");
                break;
            }
            ++sit;
        }
    }
    
    // still not found => Failure
    if ( !hit ){
        err = i18n("Bookmark not found. Maybe the file content or the bookmark URL has been changed.");
        return err;
    }
    
    // show
    bool editable = ( item->getValue(KSayItGlobal::ISEDITABLE) ).toBool();
    if ( editable )
        makeCurrentNodePersistent();

    listView->setSelected( item, true );
    listView->ensureItemVisible( item );
    slotItemClicked( item );
    return err;
}


void DocTreeViewImpl::slotItemClicked(QListViewItem *item)
{
    kdDebug(100200) << "DocTreeViewImpl::slotItemClicked()" << endl;
    
    if ( item != m_currentItem ){
        makeCurrentNodePersistent();
    }
    
    if ( item ){
        m_currentItem = static_cast<ListViewInterface*>(item);
        QString str = QString::null;
        QTextStream msg(&str, IO_ReadWrite);
        msg.setEncoding(QTextStream::UnicodeUTF8);

        // check if item has a child and check state
        // of Edit Mode.
        bool editable = ( m_currentItem->getValue(KSayItGlobal::ISEDITABLE) ).toBool() && m_editMode;        
        
        emit signalEnableTextedit( editable );
        recursiveTextCollector( m_currentItem, msg, !editable );
        emit signalContentChanged( str );
        
        // Create Bookmark-ID depending on the location of the item
        // within the tree
        QString idstring = QString::null;
        QTextStream id(&idstring, IO_ReadWrite);
        id.setEncoding(QTextStream::UnicodeUTF8);
        id << "1=RobDocument";
        recursiveBuildItemIdentifier( m_currentItem, id );
        
        // inform the Bookmark Manager about the new item.
        QString title = getItemTitle( m_currentItem );
        emit signalNotifyBookmarkManager(idstring, title);
    }
}


void DocTreeViewImpl::slotRightButtonPressed(QListViewItem *item, const QPoint &pos, int)
{
    kdDebug(100200) << "DocTreeViewImpl::slotRightButtonPressed()" << endl;
    slotItemClicked( item );
    
    if ( !m_currentItem )
        return;

    m_contextmenu = m_contextmenuhandler->getPopupMenu( m_currentItem );
    if ( !m_contextmenu )
        return;

    m_contextmenu->exec( pos );    
}


void DocTreeViewImpl::recursiveBuildItemIdentifier(ListViewInterface* item, QTextStream &idstring)
{
    kdDebug(100200) << "DocTreeViewImpl::recursiveBuildItemIdentifier()" << endl;
    
    if ( !item )
        return;
    
    // 1. Do we have a parent (=parentItem)?
    // If no -> return, if yes -> recursive call
    ListViewInterface *parentItem = NULL;
    parentItem = static_cast<ListViewInterface*>( item->parent() );
    if ( parentItem ){
        recursiveBuildItemIdentifier( parentItem, idstring );
    } else {
        // break recursion
        return;
    }
    
    // 2. Which child of the parentItem are we? Append no. to idstring.
    int childno = 0;
    QString itemID = item->text(3);
    ListViewInterface *i = static_cast<ListViewInterface*>( parentItem->firstChild() );
    while( i ){
        childno++;
        if ( i->text(3) == itemID )
            break;
        i = static_cast<ListViewInterface*>( i->nextSibling() );
    }
    idstring << "|" << childno;
    
    // 3. Who are we? Append ID to idstring.
    QString itemType = ( item->getValue(KSayItGlobal::XMLCONTEXTNAME) ).toString();    
    idstring << "=" << itemType; 
}


QString DocTreeViewImpl::getItemTitle( ListViewInterface *item )
{
    if ( !item )
        return QString::null;
        
    QString col0 = item->text(0);
    QString title = QString::null;
    // if ( col0 == i18n("Paragraph") ){ // better to use XmlContextName?
    if( (item->getValue(KSayItGlobal::XMLCONTEXTNAME)).toString() == i18n("Paragraph") ){
        title = ( item->getValue(KSayItGlobal::SPEAKERDATA) ).toString().left(32);        
        
        // canonify string
        title.replace( QRegExp("^( |\t|\n)+"), "");
        title.replace( QRegExp("( |\t|\n)$+"), "");            
    } else {
        title = col0.left(32);
    }
    return title;
}
    
  
void DocTreeViewImpl::recursiveTextCollector(ListViewInterface* item, QTextStream &msg, bool header)
{
    kdDebug(100200) << "DocTreeViewImpl::recursiveTextCollector()" << endl;

    QString text;
    if ( header ){
        // if header==true get Headers
        msg << ( item->getValue(KSayItGlobal::RTFHEADER) ).toString();
        text = ( item->getValue(KSayItGlobal::RTFDATA  ) ).toString();                
    } else {
        // get raw data (Edit Mode)
        text = ( item->getValue(KSayItGlobal::RAWDATA) ).toString();        
    }
    if ( !text.isNull() )
        msg << text;
  
    // check if current item has a child
    ListViewInterface *i = static_cast<ListViewInterface*>(item->firstChild());

    while( i ){
        recursiveTextCollector( i, msg, header );
        i = static_cast<ListViewInterface*>(i->nextSibling());
    }
}


void DocTreeViewImpl::sayActiveNodeAndChilds()
{
    kdDebug(100200) << "DocTreeViewImpl::sayActiveNodeAndChilds()" << endl;

    m_stopped = false;
    if ( !(m_currentItem->firstChild()) ){
        // current item has no childs. It may be changed.
        makeCurrentNodePersistent();
    }
    
    recursiveSayNodes( m_currentItem );
    emit signalAllNodesProcessed();
}


void DocTreeViewImpl::recursiveSayNodes(ListViewInterface* item)
{
    kdDebug(100200) << "DocTreeViewImpl::recursiveSayNodes()" << endl;

    QString str;
    QTextStream msg(&str, IO_ReadWrite);
    msg.setEncoding(QTextStream::UnicodeUTF8);

    msg << ( item->getValue(KSayItGlobal::SPEAKERHEADER) ).toString();    
    msg << ( item->getValue(KSayItGlobal::SPEAKERDATA) ).toString() << "\n" << endl;        
    
    // request say task
    if ( !m_stopped ){
        emit signalSetText( str );
    } else {
        return;
    }

    // check if current item has a child
    ListViewInterface *i = static_cast<ListViewInterface*>(item->firstChild());
    while( i ){
        recursiveSayNodes( i );
        i = static_cast<ListViewInterface*>(i->nextSibling());
    }
}


void DocTreeViewImpl::stop()
{
    kdDebug(100200) << "DocTreeViewImpl::stop()" << endl;
    m_stopped = true;
}


void DocTreeViewImpl::setNodeContent(QString &text)
{
    kdDebug(100200) << "DocTreeViewImpl::setNodeContent()" << endl;
    m_changedContent = text;
}


void DocTreeViewImpl::makeCurrentNodePersistent()
{
    kdDebug(100200) << "DocTreeViewImpl::slotMakeCurrentNodePersistent()" << endl;
    if ( m_changedContent.isNull() )
        return; // no changes were happen
    
    int maxlines = ( m_currentItem->getValue(KSayItGlobal::MAXLINES) ).toInt();
    if ( maxlines == 1 ){ // Item with max. 1 line.
        makeToSingleLine( m_changedContent );
    }    
    
    m_currentItem->setValue( KSayItGlobal::RAWDATA,     m_changedContent );
    m_currentItem->setValue( KSayItGlobal::RTFDATA,     m_changedContent );
    m_currentItem->setValue( KSayItGlobal::SPEAKERDATA, m_changedContent );

    m_changedContent = QString::null;
}    

/** replaced by slotRightButtonPressed()
 */
// void DocTreeViewImpl::contextMenuEvent(QContextMenuEvent *e)
// {
//     kdDebug(100200) << "DocTreeViewImpl::contextMenuEvent()" << endl;
// 
//     if ( !m_currentItem )
//         return;
// 
//     m_contextmenu = m_contextmenuhandler->getPopupMenu( m_currentItem );
//     if ( !m_contextmenu )
//         return;
// 
//     m_contextmenu->exec( e->globalPos() );    
// }


/******************************************
 * Context-Menu functions
 ******************************************/

void DocTreeViewImpl::slotRenameItem()
{
    kdDebug(100200) << "DocTreeViewImpl::slotRenameItem()" << endl;
    
    if (!m_currentItem)
        return;
        
    bool responseOK;
    QString newname;
    newname = KInputDialog::getText(
        i18n("Rename Item"),
        i18n("Please enter the new name of the item:"),
        m_currentItem->text(0),
        &responseOK,
        this,
        "queryNewItemName"    
    );
    if ( !responseOK )
        return;
             
    // update TreeView
    m_currentItem->setText(0, newname);
    m_currentItem->setValue( KSayItGlobal::RTFHEADER, newname );
    m_currentItem->setValue( KSayItGlobal::SPEAKERHEADER, newname );    
}


void DocTreeViewImpl::slotDeleteItem()
{
    kdDebug(100200) << "DocTreeViewImpl::slotDeleteItem()" << endl;
    
    ListViewInterface *itemToDelete = m_currentItem;
    ListViewInterface *parentItem = NULL;
    parentItem = static_cast<ListViewInterface*>(itemToDelete->parent());
    if ( !parentItem )
        return; // delete only items with parent
    
    // try to delete bookmark
    QString url = QString::null;
    QTextStream id(&url, IO_ReadWrite);
    id.setEncoding(QTextStream::UnicodeUTF8);
    id << "ksayit://1=RobDocument";
    recursiveBuildItemIdentifier( itemToDelete, id );
    QString title = getItemTitle( itemToDelete );
    emit signalDeleteBookmark( url, title );
    
    // remove TreeView item
    delete itemToDelete;
    
    // reindex entire remaining tree
    ListViewInterface *item = NULL;
    m_idCounter = KSayItGlobal::item_initial_id;
    QListViewItemIterator itr( m_rootItem );
    while ( itr.current() ) {
        item = static_cast<ListViewInterface*>(itr.current());
        item->setText(3, QString("%1").arg(++m_idCounter).rightJustify(8,'0') );    
        ++itr;
    }
    
    // show
    m_rootItem->sort();
    listView->setSelected( parentItem, true );
    listView->ensureItemVisible( parentItem );
    slotItemClicked( parentItem );
}


void DocTreeViewImpl::slotNewBookInfo()
{    
    QString whoAmI = ( m_currentItem->getValue(KSayItGlobal::XMLCONTEXTNAME) ).toString();
    if ( whoAmI == "RobDocument" ){
        int newIndex = newIndexFirstChild();
        Overview *overview = new Overview( m_currentItem, NULL, i18n("Overview") );
        overview->setText(3, QString("%1").arg(newIndex).rightJustify(8,'0') );
        
        m_contextmenuhandler->registerPopupMenu( overview );
    }
    m_rootItem->sort();
}

void DocTreeViewImpl::slotNewChapter()
{
    QString whoAmI = ( m_currentItem->getValue(KSayItGlobal::XMLCONTEXTNAME) ).toString();
    if ( whoAmI == "RobDocument" ){
        int newIndex = newIndexLastChild();
        Chapter *chapter = new Chapter( m_currentItem, NULL, i18n("Chapter") );
        chapter->setText(3, QString("%1").arg(newIndex).rightJustify(8,'0') );
        
        QString newTitle = i18n("New Chapter Title");
        chapter->setText(0, newTitle );
        chapter->setValue(KSayItGlobal::RTFHEADER,     newTitle );
        chapter->setValue(KSayItGlobal::SPEAKERHEADER, newTitle );
        
        m_contextmenuhandler->registerPopupMenu( chapter );
    }
    m_rootItem->sort();
}

void DocTreeViewImpl::slotNewKeywordSet()
{
    QString whoAmI = ( m_currentItem->getValue(KSayItGlobal::XMLCONTEXTNAME) ).toString();
    if ( whoAmI == "BookInfo" ){
        // New TreeView item
        int newIndex = newIndexFirstChild();
        KeywordSet *keywordset = new KeywordSet(m_currentItem, NULL, i18n("Keywords"));
        keywordset->setText(3, QString("%1").arg(newIndex).rightJustify(8,'0') );

        m_contextmenuhandler->registerPopupMenu( keywordset );
    }
    m_rootItem->sort();
}

void DocTreeViewImpl::slotNewKeyword()
{
    QString whoAmI = ( m_currentItem->getValue(KSayItGlobal::XMLCONTEXTNAME) ).toString();
    if ( whoAmI == "KeywordSet" ){
        // New TreeView item
        int newIndex = newIndexLastChild();
        Keyword *keyword = new Keyword(m_currentItem, NULL, i18n("Keyword"));
        keyword->setText(3, QString("%1").arg(newIndex).rightJustify(8,'0') );
        
        QString newKeyword = i18n("New Keyword");
        keyword->setText(0, newKeyword );
        keyword->setValue(KSayItGlobal::RAWDATA,     newKeyword);
        keyword->setValue(KSayItGlobal::RTFDATA,     newKeyword);
        keyword->setValue(KSayItGlobal::SPEAKERDATA, newKeyword);

        m_contextmenuhandler->registerPopupMenu( keyword );
    }
    m_rootItem->sort();
}

void DocTreeViewImpl::slotNewAbstract()
{
    QString whoAmI = ( m_currentItem->getValue(KSayItGlobal::XMLCONTEXTNAME) ).toString();
    if ( whoAmI == "BookInfo" ){
        // New TreeView item
        int newIndex = newIndexLastChild();
        Abstract *abstract = new Abstract(m_currentItem, NULL, i18n("Abstract"));
        abstract->setText(3, QString("%1").arg(newIndex).rightJustify(8,'0') );

        m_contextmenuhandler->registerPopupMenu( abstract );
        
        // add a new Para-Element
        ListViewInterface *backup = m_currentItem;
        m_currentItem = abstract;
        slotNewParagraph();
        m_currentItem = backup;
    }
    m_rootItem->sort();
}

void DocTreeViewImpl::slotNewAuthorGroup()
{
    QString whoAmI = ( m_currentItem->getValue(KSayItGlobal::XMLCONTEXTNAME) ).toString();
    if ( whoAmI == "BookInfo" ){
        // New TreeView item
        int newIndex = newIndexFirstChild();
        AuthorGroup *authorgroup = new AuthorGroup(m_currentItem, NULL, i18n("Author(s)"));
        authorgroup->setText(3, QString("%1").arg(newIndex).rightJustify(8,'0') );

        m_contextmenuhandler->registerPopupMenu( authorgroup );
        
        // add a new Author-Element
        ListViewInterface *backup = m_currentItem;
        m_currentItem = authorgroup;
        slotNewAuthor();
        m_currentItem = backup;
    }
    m_rootItem->sort();
}

void DocTreeViewImpl::slotNewAuthor()
{
    QString whoAmI = ( m_currentItem->getValue(KSayItGlobal::XMLCONTEXTNAME) ).toString();
    if ( whoAmI == "AuthorGroup" ){
        // New TreeView item
        int newIndex = newIndexLastChild();
        Author *author = new Author(m_currentItem);
        author->setText(3, QString("%1").arg(newIndex).rightJustify(8,'0') );
    
        author->setText(0, i18n("Author") );
        QString newAuthor = i18n("Firstname Surname"); 
        author->setText(1, newAuthor );
        author->setValue( KSayItGlobal::RAWDATA,     newAuthor );
        author->setValue( KSayItGlobal::RTFDATA,     newAuthor );
        author->setValue( KSayItGlobal::SPEAKERDATA, newAuthor );

        m_contextmenuhandler->registerPopupMenu( author );
    }
    m_rootItem->sort();
}

void DocTreeViewImpl::slotNewDate()
{
    QString whoAmI = ( m_currentItem->getValue(KSayItGlobal::XMLCONTEXTNAME) ).toString();
    if ( whoAmI == "BookInfo" ){
        // New TreeView item
        int newIndex = newIndexLastChild();
        Date *date = new Date(m_currentItem, NULL, i18n("Date"));
        date->setText(3, QString("%1").arg(newIndex).rightJustify(8,'0') );
        // get current date
        QString today;
        today = KGlobal::locale()->formatDate(QDate::currentDate(Qt::LocalTime), true);
        date->setText( 1, today);
        date->setValue( KSayItGlobal::RAWDATA,     today );
        date->setValue( KSayItGlobal::RTFDATA,     today );
        date->setValue( KSayItGlobal::SPEAKERDATA, today );
        
        m_contextmenuhandler->registerPopupMenu( date );
    }
    m_rootItem->sort();
}

void DocTreeViewImpl::slotNewReleaseInfo()
{
    QString whoAmI = ( m_currentItem->getValue(KSayItGlobal::XMLCONTEXTNAME) ).toString();
    if ( whoAmI == "BookInfo" ){
        // New TreeView item
        int newIndex = newIndexLastChild();
        ReleaseInfo *relinfo = new ReleaseInfo(m_currentItem, NULL, i18n("Release"));
        relinfo->setText(3, QString("%1").arg(newIndex).rightJustify(8,'0') );
        
        QString newRelease = "0.0.0";
        relinfo->setText( 1, newRelease );
        relinfo->setValue( KSayItGlobal::RAWDATA,     newRelease );
        relinfo->setValue( KSayItGlobal::RTFDATA,     newRelease );
        relinfo->setValue( KSayItGlobal::SPEAKERDATA, newRelease );
        
        m_contextmenuhandler->registerPopupMenu( relinfo );
    }
    m_rootItem->sort();
}

void DocTreeViewImpl::slotNewTitle()
{

}

void DocTreeViewImpl::slotNewParagraph()
{
/*    // Create Null element
    QDomElement parentElement = QDomElement();

    NodeList_type list;
    list.clear();
    m_currentItem->getNodes( list );
    parentElement = (list.front()).toElement();

    if ( parentElement.isNull() )
        return;
*/
    // New TreeView item
    int newIndex = newIndexLastChild();
    Para *para = new Para(m_currentItem, NULL, i18n("Paragraph"));
    para->setText(3, QString("%1").arg(newIndex).rightJustify(8,'0') );

    m_contextmenuhandler->registerPopupMenu( para );
    
    m_rootItem->sort();
}

void DocTreeViewImpl::slotNewSection_1()
{
    QString whoAmI = ( m_currentItem->getValue(KSayItGlobal::XMLCONTEXTNAME) ).toString();
    if ( whoAmI == "Chapter" ){
        int newIndex = newIndexLastChild();
        Sect1 *sect1 = new Sect1(m_currentItem, NULL, i18n("Section Level 1"));
        sect1->setText(3, QString("%1").arg(newIndex).rightJustify(8,'0') );
        
        QString newTitle = i18n("New Section Title");
        sect1->setValue( KSayItGlobal::RTFHEADER,     newTitle );
        sect1->setValue( KSayItGlobal::SPEAKERHEADER, newTitle );
        
        m_contextmenuhandler->registerPopupMenu( sect1 );
    }
    m_rootItem->sort();
}

void DocTreeViewImpl::slotNewSection_2()
{
    QString whoAmI = ( m_currentItem->getValue(KSayItGlobal::XMLCONTEXTNAME) ).toString();
    if ( whoAmI == "Sect1" ){
        int newIndex = newIndexLastChild();
        Sect2 *sect2 = new Sect2(m_currentItem, NULL, i18n("Section Level 2"));
        sect2->setText(3, QString("%1").arg(newIndex).rightJustify(8,'0') );
        
        QString newTitle = i18n("New Section Title");
        sect2->setValue( KSayItGlobal::RTFHEADER,     newTitle );
        sect2->setValue( KSayItGlobal::SPEAKERHEADER, newTitle );
        
        m_contextmenuhandler->registerPopupMenu( sect2 );
    }
    m_rootItem->sort();
}

void DocTreeViewImpl::slotNewSection_3()
{
    QString whoAmI = ( m_currentItem->getValue(KSayItGlobal::XMLCONTEXTNAME) ).toString();
    if ( whoAmI == "Sect2" ){
        int newIndex = newIndexLastChild();
        Sect3 *sect3 = new Sect3(m_currentItem, NULL, i18n("Section Level 3"));
        sect3->setText(3, QString("%1").arg(newIndex).rightJustify(8,'0') );
        
        QString newTitle = i18n("New Section Title");
        sect3->setValue( KSayItGlobal::RTFHEADER,     newTitle );
        sect3->setValue( KSayItGlobal::SPEAKERHEADER, newTitle );
        
        m_contextmenuhandler->registerPopupMenu( sect3 );
    }
    m_rootItem->sort();
}

void DocTreeViewImpl::slotNewSection_4()
{
    QString whoAmI = ( m_currentItem->getValue(KSayItGlobal::XMLCONTEXTNAME) ).toString();
    if ( whoAmI == "Sect3" ){
        int newIndex = newIndexLastChild();
        Sect4 *sect4 = new Sect4(m_currentItem, NULL, i18n("Section Level 4"));
        sect4->setText(3, QString("%1").arg(newIndex).rightJustify(8,'0') );
        
        QString newTitle = i18n("New Section Title");
        sect4->setValue( KSayItGlobal::RTFHEADER,     newTitle );
        sect4->setValue( KSayItGlobal::SPEAKERHEADER, newTitle );
        
        m_contextmenuhandler->registerPopupMenu( sect4 );
    }
    m_rootItem->sort();
}

void DocTreeViewImpl::slotNewSection_5()
{
    QString whoAmI = ( m_currentItem->getValue(KSayItGlobal::XMLCONTEXTNAME) ).toString();
    if ( whoAmI == "Sect4" ){
        int newIndex = newIndexLastChild();
        Sect5 *sect5 = new Sect5(m_currentItem, NULL, i18n("Section Level 5"));
        sect5->setText(3, QString("%1").arg(newIndex).rightJustify(8,'0') );
        
        QString newTitle = i18n("New Section Title");
        sect5->setValue( KSayItGlobal::RTFHEADER,     newTitle );
        sect5->setValue( KSayItGlobal::SPEAKERHEADER, newTitle );
        
        m_contextmenuhandler->registerPopupMenu( sect5 );
    }
    m_rootItem->sort();
}
  
int DocTreeViewImpl::newIndexFirstChild()
{
    kdDebug(100200) << "DocTreeViewImpl::newIndexFirstChild()" << endl;
    
    int currentIndex = (m_currentItem->text(3)).toInt();
    
    // walk trough the entire tree and increment all
    // indices greater than currentIndex
    ListViewInterface *item = NULL;
    int itemIndex;
    QListViewItemIterator it( m_rootItem );
    while ( it.current() ) {
        item = static_cast<ListViewInterface*>(it.current());
        itemIndex = (item->text(3)).toInt();
        if ( itemIndex > currentIndex ){
            item->setText(3, QString("%1").arg(itemIndex+1).rightJustify(8,'0') );;    
        }
        ++it;
    }
       
    m_idCounter++;
    currentIndex += 1;
    return currentIndex;    
}    


int DocTreeViewImpl::newIndexLastChild()
{
    kdDebug(100200) << "DocTreeViewImpl::newIndexLastChild()" << endl;
       
    // find highest index of the subtree with
    // m_currentItem as root
    int lastIndex = 0;
    findHighestIndex( m_currentItem, lastIndex );
    kdDebug(100200) << "Letzter Index: " << lastIndex << endl;
    
    // walk trough the entire tree and increment all
    // indices greater than lastIndex
    int itemIndex;
    ListViewInterface *item;
    QListViewItemIterator it( m_rootItem );
    while ( it.current() ) {
        item = static_cast<ListViewInterface*>(it.current());
        itemIndex = (item->text(3)).toInt();
        if ( itemIndex > lastIndex ){
            item->setText(3, QString("%1").arg(itemIndex+1).rightJustify(8,'0') );    
        }
        ++it;
    }

    m_idCounter++;
    lastIndex += 1;
    return lastIndex;    
}    


void DocTreeViewImpl::findHighestIndex(ListViewInterface* item, int &index)
{
    int currentIndex = (item->text(3)).toInt();
    index = currentIndex > index ? currentIndex : index; 
    
    ListViewInterface *i = static_cast<ListViewInterface*>(item->firstChild());
    while( i ){
        findHighestIndex(i, index);
        i = static_cast<ListViewInterface*>(i->nextSibling());
    }
}    

