//
// C++ Interface: %{MODULE}
//
// Description: 
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CONTEXTMENUHANDLER_H
#define CONTEXTMENUHANDLER_H

// Qt includes
#include <qobject.h>
#include <qstring.h>
#include <qxml.h>

// KDE includes
#include <kaction.h>
#include <kpopupmenu.h>

//forward declarations
class ContextMenuHandler;
class ListViewInterface;

/**
@author Robert Vogl
*/
class ContextActionHandler : public QXmlDefaultHandler 
{
public:
    ContextActionHandler(ContextMenuHandler *menuhandler = 0);
    ~ContextActionHandler();
    
    // Reimplementations from base class
    bool startElement( const QString &namespaceURI,
            const QString &localName,
            const QString &qName,
            const QXmlAttributes &atts );    
    
    // Reimplementations from base class
    bool endElement( const QString &namespaceURI,
            const QString &localName,
            const QString &qName );
            
    // Reimplementations from base class
    bool characters( const QString &ch );

    void setSearchID( const QString xmlID );
    
private:
    ContextMenuHandler *m_menuhandler;
    QString m_subName;
    QString m_actionName;
    QString m_qty;
    KPopupMenu *m_popup;
    bool m_hit;
    QString m_searchID;
};


/**
@author Robert Vogl
*/
class ContextMenuHandler : public QObject
{
Q_OBJECT
public:
    ContextMenuHandler(QObject *parent = 0, const char *name = 0);

    ~ContextMenuHandler();

    /**
     * \param item The selected TreeView item.
     * \returns A pointer to the popup-menu.
     */
    KPopupMenu* getPopupMenu(ListViewInterface *item);
    
    /**
     * \returns A pointer to the Submenu as a child of the parent
     * \p parent.
     */
    KPopupMenu* SubMenuFactory(KPopupMenu *parent);
    
    /**
     * Creates an action sufficiant to the given name.
     * It checks if the action is allowed or not and returns
     * a disabled or enabled action.
     * \param actionName The name of the action as defined in the
     * namespace \p ContextMenu.xml.
     * \param qty The maximum allowed quantity of the same action for the
     * given element.
     * \returns A Pointer to the action. 
     */
    KAction* ActionFactory( const QString &actionName, const QString &qty );
    
    /**
     * Creates a Popup context menu for the given item and stores a
     * pointer to it in \p m_MenuList.
     */
    void registerPopupMenu(ListViewInterface *item);
    
    /**
     * Sets wether the content of the item is editable or not.
     */
    void setItemEditable( bool editable );
    
    /**
     * Sets the number of lines a editable item can hold.
     * -1 = unlimited.
     */
    void setItemMaxlines( const QString &maxlines ); 

private: // Methods
    void initActions();
    bool parseXmlFile(const QString &xmlID);

public: // Attributes
    KPopupMenu* m_popupmenu;

private:
    // Basic actions
    KAction* renameItem;
    KAction* deleteItem;
    // Actions to create new items
    KAction* newBookInfo;
    KAction* newChapter;
    KAction* newKeywordSet;
    KAction* newKeyword;
    KAction* newAbstract;
    KAction* newAuthorGroup;
    KAction* newAuthor;
    KAction* newDate;
    KAction* newReleaseInfo;
    KAction* newTitle;
    KAction* newParagraph;
    KAction* newSection_1;
    KAction* newSection_2;
    KAction* newSection_3;
    KAction* newSection_4;
    KAction* newSection_5;

    // mixed stuff
    QString m_XmlFilePath;
    ListViewInterface* m_item;
    QObject* m_DocTreeView;

};

#endif
