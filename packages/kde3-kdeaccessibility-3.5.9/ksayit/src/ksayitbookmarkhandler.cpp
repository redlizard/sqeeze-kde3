//
// C++ Implementation: ksayitbookmarkhandler
//
// Description:
//
//
// Author: Robert Vogl <voglrobe@web.de>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//

// Qt include
#include <qregexp.h>

// KDE includes
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kurl.h>

// App specific includes
#include "ksayitbookmarkhandler.h"
#include "ksayit.h"

KSayItBookmarkHandler::KSayItBookmarkHandler(KBookmarkManager *bkManager, KSayItApp* parent)
 : KBookmarkOwner(), m_bkManager(bkManager), m_parent(parent)
{
    m_ID       = QString::null;
    m_title    = QString::null;
}

KSayItBookmarkHandler::~KSayItBookmarkHandler()
{
}


void KSayItBookmarkHandler::notifyBookmarkHandler(const QString &ID, const QString &title)
{
    kdDebug(100200) << "KSayItBookmarkHandler::notifyBookmarkManager()" << endl;
    
    m_ID = ID;
    m_title = title;
}


void KSayItBookmarkHandler::openBookmarkURL(const QString &url)
{
    kdDebug(100200) << "KSayItBookmarkHandler::openBookmarkURL(" << url << ")" << endl;
    
    QString l_url = url;
    QString title = QString::null;
    QString type = l_url.section( "://", 0, 0 );
    QString ID   = l_url.section( QRegExp("/+"), 1, 1 );
    QString err  = QString::null;
    
    // Some checks
    if ( type != "ksayit" ){
        err += i18n("This is not a KSayIt bookmark.\n"); 
    }
    
    // get title
    KBookmarkGroup bkRoot = m_bkManager->root();
    if ( bkRoot.isNull() )
        return;
        
    KBookmark bookmark;
    KBookmarkGroup group;
    bool found = recursiveGetBkByURL( bookmark, group, bkRoot, url );
    if ( found ){
        title = bookmark.text();
    }
        
    QString result = QString::null;
    result = m_parent->setItemByBookmark( ID, title );
    if ( !result.isNull() ){
        KMessageBox::sorry( 0, result, i18n("Bookmark not found") );           
    }    
}


QString KSayItBookmarkHandler::currentTitle() const
{
    kdDebug(100200) << "KSayItBookmarkHandler::currentTitle()" << endl;
    
    QString result;
    if ( m_title.isEmpty()){
        result = i18n("untitled");
    } else {
        result = m_title;
    }   
    
    return result;
}


QString KSayItBookmarkHandler::currentURL() const
{
    kdDebug(100200) << "KSayItBookmarkHandler::currentURL()" << endl;
    
    QString url;
    url = "ksayit://" + m_ID;
    
    return url;
}


void KSayItBookmarkHandler::deleteBookmark(const QString &url, const QString &title)
{
    kdDebug(100200) << "KSayItBookmarkHandler::deleteBookmark()" << endl;
    
    KBookmarkGroup bkRoot = m_bkManager->root();
    if ( bkRoot.isNull() )
        return;
        
    // search bookmark by URL
    KBookmark bookmark;
    KBookmarkGroup group;
    bool found = false;
    found = recursiveGetBkByURL( bookmark, group, bkRoot, url );
    if ( found ){
        group.deleteBookmark( bookmark );
        m_bkManager->emitChanged( group );
        m_bkManager->save(); // make persistent    
        return;   
    }
    
    // if not found, search bookmark by title
    int qty = 0;
    qty = recursiveGetBkByTitle( bookmark, group, bkRoot, title );
    if ( qty == 1 ){
        QString url = bookmark.url().url();
        QString title = bookmark.text();
        group.deleteBookmark( bookmark );
        m_bkManager->emitChanged( group );   
        m_bkManager->save(); // make persistent    
    }    
}


bool KSayItBookmarkHandler::recursiveGetBkByURL(
        KBookmark &bookmark,
        KBookmarkGroup &group,
        const KBookmarkGroup &bkGroup,
        const QString &url)
{
    KBookmark bkNext;
    bool found = false;
    
    KBookmark bk = bkGroup.first();           
    while ( !bk.isNull() && !bk.isSeparator() ){
        if ( bk.isGroup() ){
            // recursive call
            found = recursiveGetBkByURL( bookmark, group, bk.toGroup(), url );
            if ( found )
                return true;
            bkNext = bkGroup.next( bk );
        } else {
            QString l_url = bk.url().url();
            if ( l_url == url ){
                bookmark = bk;
                group = bkGroup;
                return true;        
            }
            bkNext = bkGroup.next( bk );            
        }
        bk = bkNext;    
    }
    return false;
}


int KSayItBookmarkHandler::recursiveGetBkByTitle(
        KBookmark &bookmark,
        KBookmarkGroup &group,
        const KBookmarkGroup &bkGroup,
        const QString &title)
{
    KBookmark bkNext;
    int qty = 0;
    
    KBookmark bk = bkGroup.first();           
    while ( !bk.isNull() && !bk.isSeparator() ){
        if ( bk.isGroup() ){
            // recursive call
            qty += recursiveGetBkByTitle( bookmark, group, bk.toGroup(), title );
            bkNext = bkGroup.next( bk );
        } else {
            QString l_title = bk.text();
            if ( l_title == title ){
                bookmark = bk;
                group = bkGroup;
                qty++;
            }
            bkNext = bkGroup.next( bk );            
        }
        bk = bkNext;    
    }
    return qty;
}


void KSayItBookmarkHandler::traverseBookmarks(KBookmarkGroup bkGroup)
{
    kdDebug(100200) << "### KSayItBookmarkHandler::traverseBookmarks()" << endl;
    
    if( bkGroup.isNull() )
        return;
        
    KURL url;
    QString title;
    KBookmark bkNext, bkPrev, bkNew;
    
    KBookmark bk = bkGroup.first();           
    while ( !bk.isNull() && !bk.isSeparator() ){
        if ( bk.isGroup() ){
            traverseBookmarks( bk.toGroup() ); // recursive call
            bkNext = bkGroup.next( bk );
        } else {
            url = bk.url();
            title = bk.text();
            bkNext = bkGroup.next( bk );
            bkPrev = bkGroup.previous( bk );
            if ( bkPrev.isNull() )  // no predecessor
                bkPrev = bk;
            
            // Modifications on URL/Title BEGIN
            //
            // Modifications on URL/Title END           
                          
            bkNew = bkGroup.addBookmark( m_bkManager, title, url, QString::null, false );
            bkGroup.moveItem( bkNew, bkPrev );
            bkGroup.deleteBookmark( bk );            
        }
        bk = bkNext;    
    }
    m_bkManager->save(); // make persistent
}
