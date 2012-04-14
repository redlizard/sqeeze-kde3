//
// C++ Interface: docbookparser
//
// Description:
//
//
// Author: Robert Vogl <voglrobe@lapislazuli>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef DOCBOOKPARSER_H
#define DOCBOOKPARSER_H

// Qt includes
#include <qdom.h>

// KDE includes

// App specific includes
#include "Types.h"
#include "contextmenuhandler.h"


class DocbookParser
{
public:
    // Constructor
    DocbookParser(ContextMenuHandler *cmHandler=0);

    // Destructor
    ~DocbookParser();
    
    void parseBook(const QDomElement &element, ListViewInterface *item);
    
    int getIdCounter(){ return m_idCounter; };

private: // Methods
    // a selection of DocBook elements
    void parseBookInfo(const QDomElement &element, ListViewInterface *item);
    void parseAuthorGroup(const QDomElement &element, ListViewInterface *item);
    void parseAuthor(const QDomElement &element, ListViewInterface *item);
    void parseKeywordSet(const QDomElement &element, ListViewInterface *item);
    void parseAbstract(const QDomElement &element, ListViewInterface *item);
    void parseChapter(const QDomElement &element, ListViewInterface *item);
    void parseSect1(const QDomElement &element, ListViewInterface *item);
    void parseSect2(const QDomElement &element, ListViewInterface *item);
    void parseSect3(const QDomElement &element, ListViewInterface *item);
    void parseSect4(const QDomElement &element, ListViewInterface *item);
    void parseSect5(const QDomElement &element, ListViewInterface *item);
    void parsePara(const QDomElement &element, ListViewInterface *item);

    // Helpers
    QString node2raw(QDomNode node) const;
    
private: // Attributes
    ContextMenuHandler *m_contextmenuhandler;
    int m_idCounter;

};

#endif

 
