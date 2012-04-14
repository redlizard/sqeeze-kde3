//
// C++ Interface: docbookgenerator
//
// Description:
//
//
// Author: Robert Vogl <voglrobe@lapislazuli>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef DOCBOOKGENERATOR_H
#define DOCBOOKGENERATOR_H

// Qt includes
#include <qtextstream.h>

// KDE includes

// App specific includes
#include "Types.h"

class DocbookGenerator
{
public:
    // Constructor
    DocbookGenerator();

    // Destructor
    ~DocbookGenerator();

    /**
     * Walks throug the entire treeview starting with given item and returns
     * returns a DocBook representation of its content.
     * \returns doc The content of the TreeView in DocBook format
     * \param item A pointer to the root item of the tree. Must be of type
     * \p RobDocument.
     */
    void writeBook(QTextStream &doc, ListViewInterface *item);


private:
    void writeBookInfo(QTextStream &doc, ListViewInterface *item);
    void writeTitleOfBook(QTextStream &doc, ListViewInterface *item);
    void writeChapter(QTextStream &doc, ListViewInterface *item);
    void writeKeywordSet(QTextStream &doc, ListViewInterface *item);
    void writeKeyword(QTextStream &doc, ListViewInterface *item);
    void writeAbstract(QTextStream &doc, ListViewInterface *item);
    void writePara(QTextStream &doc, ListViewInterface *item);
    void writeAuthorGroup(QTextStream &doc, ListViewInterface *item);
    void writeAuthor(QTextStream &doc, ListViewInterface *item);
    void writeDate(QTextStream &doc, ListViewInterface *item);
    void writeReleaseInfo(QTextStream &doc, ListViewInterface *item);
    void writeSect1(QTextStream &doc, ListViewInterface *item);
    void writeSect2(QTextStream &doc, ListViewInterface *item);
    void writeSect3(QTextStream &doc, ListViewInterface *item);
    void writeSect4(QTextStream &doc, ListViewInterface *item);
    void writeSect5(QTextStream &doc, ListViewInterface *item);



};







#endif

