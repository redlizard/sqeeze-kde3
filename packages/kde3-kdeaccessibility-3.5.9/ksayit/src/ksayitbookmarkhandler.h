//
// C++ Interface: ksayitbookmarkhandler
//
// Description:
//
//
// Author: Robert Vogl <voglrobe@web.de>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef KSAYITBOOKMARKHANDLER_H
#define KSAYITBOOKMARKHANDLER_H

// Qt includes
#include <qstring.h>

// KDE includes
#include <kbookmarkmanager.h>


class KSayItApp;

/**
 Each of the objects in the TreeView that can be bookmarked has an unique ID.
 The "URL" is to be created as follows: \p ksayit://filename/ID.\n
 The "Title" is either the content of column 0 of the TreeView or the 32nd leading
 charcters of the text (e.g. if the item references to a paragraph).
@author Robert Vogl
*/
class KSayItBookmarkHandler : public KBookmarkOwner
{
public:
    KSayItBookmarkHandler(KBookmarkManager *bkManager=0, KSayItApp* parent=0);
    virtual ~KSayItBookmarkHandler();

public:
    /**
     * Reimplemented from base class.\n
     * Called when a bookmark in the menu was clicked.
     * \param url The URL of the selected bookmark.
     */
    void openBookmarkURL(const QString &url);

    /**
     * Reimplemented from base class.\n
     * Returns the Title when a new bookmark is about to be created.
     * \returns The title of the bookmark.
     */
    QString currentTitle() const;


    /**
     * Reimplemented from base class.\n
     * Returns the URL when a new bookmark is about to be created.
     * \returns The URL of the bookmark.
     */
    QString currentURL() const;

    /**
     * Sets ID and title of the current TreeView item.
     * \param ID The unique ID of the item.
     * \param title The bookmark title of the item.
     */
    void notifyBookmarkHandler(const QString &ID, const QString &title);
    
    /**
     * Deletes the bookmark designated by the given url.
     * \param url The url of the bookmark.
     */
    void deleteBookmark(const QString &url, const QString &title);

    /**
     * Iterates recursively through all bookmarks below the
     * given group.
     * \param bkGroup The KBookmarkGroup to start.
     */
    void traverseBookmarks(KBookmarkGroup bkGroup);

private:    
    /**
     * Searches the Bookmark designted by its URL.
     * \param bookmark A reference that will contain the search result (bookmark).
     * \param group A reference that will contain the search result
     * (the group the found bookmark belongs to).
     * \param bkGroup The root-bookmark from which the search will start
     * (usually the topmost bookmark in the tree).
     * \param url The URL of the bookmark to search for (including <tt>ksayit://</tt>).
     * \returns \em true, if the bookmark was found, \em false, if not.
     */
    bool recursiveGetBkByURL(
            KBookmark &bookmark,
            KBookmarkGroup &group,
            const KBookmarkGroup &bkGroup,
            const QString &url);
    
    /**
     * Searches the Bookmark designted by its URL.
     * \param bookmark A reference that will contain the search result (bookmark).
     * \param group A reference that will contain the search result
     * (the group the found bookmark belongs to).
     * \param bkGroup The root-bookmark from which the search will start
     * (usually the topmost bookmark in the tree).
     * \param title The title of the bookmark to search for.
     * \returns Number of bookmarks with the given title.
     */
    int recursiveGetBkByTitle(
            KBookmark &bookmark,
            KBookmarkGroup &group,
            const KBookmarkGroup &bkGroup,
            const QString &title);
private:
    QString m_ID;
    QString m_title;
    KBookmarkManager *m_bkManager;
    KSayItApp *m_parent;

};

#endif
