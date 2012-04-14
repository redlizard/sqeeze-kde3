//
// C++ Implementation: docbookgenerator
//
// Description:
//
//
// Author: Robert Vogl <voglrobe@lapislazuli>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//

// Qt includes
#include <qstring.h>
#include <qvariant.h>

//KDE includes
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

// App specific includes
#include "docbookgenerator.h"


DocbookGenerator::DocbookGenerator()
{
}


DocbookGenerator::~DocbookGenerator()
{
}


void DocbookGenerator::writeBook(QTextStream &doc, ListViewInterface *item)
{
    kdDebug(100200) << "DocTreeViewImpl::writeBook()" << endl;
    // read item's content
    QString whoAmI = (item->getValue(KSayItGlobal::XMLCONTEXTNAME)).toString();
    if ( whoAmI != "RobDocument" )
        return;

    // add node to document
    doc << "<Book>" << endl;

    // process childs
    ListViewInterface *i = static_cast<ListViewInterface*>(item->firstChild());
    QString itemType;
    while( i ){
        itemType = (i->getValue(KSayItGlobal::XMLCONTEXTNAME)).toString();
        if ( itemType == "BookInfo" ){
            writeBookInfo( doc, i );
        } else if ( itemType == "Chapter" ) {
            writeChapter( doc, i );
        }
        // next child
        i = static_cast<ListViewInterface*>(i->nextSibling());
    }
    doc << "</Book>" << endl;
}


void DocbookGenerator::writeBookInfo(QTextStream &doc, ListViewInterface *item)
{
    kdDebug(100200) << "DocTreeViewImpl::writeBookInfo()" << endl;
    // read item's content
    QString whoAmI = (item->getValue(KSayItGlobal::XMLCONTEXTNAME)).toString();
    if ( whoAmI != "BookInfo" )
        return;

    // add node to document
    doc << "<BookInfo>" << endl;
    writeTitleOfBook( doc, item );

    // process childs
    ListViewInterface *i = static_cast<ListViewInterface*>(item->firstChild());
    QString itemType;
    while( i ){
        itemType = (i->getValue(KSayItGlobal::XMLCONTEXTNAME)).toString();
        if ( itemType == "KeywordSet" ){
            writeKeywordSet( doc, i );
        } else if ( itemType == "Abstract" ) {
            writeAbstract( doc, i );
        } else if ( itemType == "AuthorGroup" ) {
            writeAuthorGroup( doc, i );
        } else if ( itemType == "Date" ) {
            writeDate( doc, i );
        } else if ( itemType == "ReleaseInfo" ) {
            writeReleaseInfo( doc, i );
        }
        // next child
        i = static_cast<ListViewInterface*>(i->nextSibling());
    }

    doc << "</BookInfo>" << endl;
}


void DocbookGenerator::writeTitleOfBook(QTextStream &doc, ListViewInterface *item)
{
    kdDebug(100200) << "DocTreeViewImpl::writeTitle()" << endl;
    // read item's content
    QString whoAmI = (item->getValue(KSayItGlobal::XMLCONTEXTNAME)).toString();
    if ( whoAmI != "BookInfo" )
        return;

    // Documents title is stored in root element (parent of BookInfo)
    ListViewInterface *parent = static_cast<ListViewInterface*>(item->parent());
    if ( parent ){
        QString title = parent->text(0);

        // add node to document
        doc << "<Title>" << title << "</Title>" << endl;
    }
}


void DocbookGenerator::writeAuthorGroup(QTextStream &doc, ListViewInterface *item)
{
    kdDebug(100200) << "DocTreeViewImpl::writeAuthorGroup()" << endl;
    // read item's content
    QString whoAmI = (item->getValue(KSayItGlobal::XMLCONTEXTNAME)).toString();
    if ( whoAmI != "AuthorGroup" )
        return;

    // add node to document
    doc << "<AuthorGroup>" << endl;

    // process childs
    ListViewInterface *i = static_cast<ListViewInterface*>(item->firstChild());
    QString itemType;
    while( i ){
        itemType = (i->getValue(KSayItGlobal::XMLCONTEXTNAME)).toString();
        if ( itemType == "Author" ){
            writeAuthor( doc, i );
        }
        // next child
        i = static_cast<ListViewInterface*>(i->nextSibling());
    }

    doc << "</AuthorGroup>" << endl;
}


void DocbookGenerator::writeAuthor(QTextStream &doc, ListViewInterface *item)
{
    kdDebug(100200) << "DocTreeViewImpl::writeAuthor()" << endl;
    // read item's content
    QString whoAmI = (item->getValue(KSayItGlobal::XMLCONTEXTNAME)).toString();
    if ( whoAmI != "Author" )
        return;

    QString author = QString::null;
    author = ( item->getValue(KSayItGlobal::RAWDATA) ).toString();

    // add node to document
    doc << "<Author>" << endl;

    // process childs
    doc << "<Firstname>" << author.section(' ', 0, 0) << "</Firstname>" << endl;
    doc << "<Surname>" << author.section(' ', 1, 1) << "</Surname>" << endl;

    doc << "</Author>" << endl;
}


void DocbookGenerator::writeDate(QTextStream &doc, ListViewInterface *item)
{
    kdDebug(100200) << "DocTreeViewImpl::writeDate()" << endl;
    // read item's content
    QString whoAmI = (item->getValue(KSayItGlobal::XMLCONTEXTNAME)).toString();
    if ( whoAmI != "Date" )
        return;

    QString date = item->text(1);

    // add node to document
    doc << "<Date>" << date << "</Date>" << endl;
}


void DocbookGenerator::writeReleaseInfo(QTextStream &doc, ListViewInterface *item)
{
    kdDebug(100200) << "DocTreeViewImpl::writeReleaseInfo()" << endl;
    // read item's content
    QString whoAmI = (item->getValue(KSayItGlobal::XMLCONTEXTNAME)).toString();
    if ( whoAmI != "ReleaseInfo" )
        return;

    QString releaseinfo = item->text(1);

    // add node to document
    doc << "<ReleaseInfo>" << releaseinfo << "</ReleaseInfo>" << endl;
}


void DocbookGenerator::writeKeywordSet(QTextStream &doc, ListViewInterface *item)
{
    kdDebug(100200) << "DocTreeViewImpl::writeKeywordSet()" << endl;
    // read item's content
    QString whoAmI = (item->getValue(KSayItGlobal::XMLCONTEXTNAME)).toString();
    if ( whoAmI != "KeywordSet" )
        return;

    // add node to document
    doc << "<KeywordSet>" << endl;

    // process childs
    ListViewInterface *i = static_cast<ListViewInterface*>(item->firstChild());
    QString itemType;
    while( i ){
        itemType = ( i->getValue(KSayItGlobal::XMLCONTEXTNAME) ).toString();
        if ( itemType == "Keyword" ){
            writeKeyword( doc, i );
        }
        // next child
        i = static_cast<ListViewInterface*>(i->nextSibling());
    }

    doc << "</KeywordSet>" << endl;
}


void DocbookGenerator::writeKeyword(QTextStream &doc, ListViewInterface *item)
{
    kdDebug(100200) << "DocTreeViewImpl::writeKeyword()" << endl;
    // read item's content
    QString whoAmI = (item->getValue(KSayItGlobal::XMLCONTEXTNAME)).toString();
    if ( whoAmI != "Keyword" )
        return;

    // QString keyword = item->text( 0 );
    QString keyword = QString::null;    
    keyword = (item->getValue(3)).toString();

    // add node to document
    doc << "<Keyword>" << keyword << "</Keyword>" << endl;

    // process childs
    // no childs
}


void DocbookGenerator::writeAbstract(QTextStream &doc, ListViewInterface *item)
{
    kdDebug(100200) << "DocTreeViewImpl::writeAbstract()" << endl;
    // read item's content
    QString whoAmI = (item->getValue(KSayItGlobal::XMLCONTEXTNAME)).toString();
    if ( whoAmI != "Abstract" )
        return;

    // add node to document
    doc << "<Abstract>" << endl;

    // process childs
    ListViewInterface *i = static_cast<ListViewInterface*>(item->firstChild());
    QString itemType;
    while( i ){
        itemType = (i->getValue(KSayItGlobal::XMLCONTEXTNAME)).toString();
        if ( itemType == "Para" ){
            writePara( doc, i );
        }
        // next child
        i = static_cast<ListViewInterface*>(i->nextSibling());
    }

    doc << "</Abstract>" << endl;
}


void DocbookGenerator::writeChapter( QTextStream &doc, ListViewInterface *item)
{
    kdDebug(100200) << "DocTreeViewImpl::writeChapter()" << endl;
    // read item's content
    QString whoAmI = (item->getValue(KSayItGlobal::XMLCONTEXTNAME)).toString();
    if ( whoAmI != "Chapter" )
        return;

    QString title = item->text(0);

    // add node to document
    doc << "<Chapter>" << endl;
        doc << "<Title>" << title << "</Title>" << endl;

    // process childs
    ListViewInterface *i = static_cast<ListViewInterface*>(item->firstChild());
    QString itemType;
    while( i ){
        itemType = (i->getValue(KSayItGlobal::XMLCONTEXTNAME)).toString();
        if ( itemType == "Para" ){
            writePara( doc, i );
        } else if ( itemType == "Sect1" ) {
            writeSect1( doc, i );
        }
        // next child
        i = static_cast<ListViewInterface*>(i->nextSibling());
    }

    doc << "</Chapter>" << endl;
}


void DocbookGenerator::writeSect1(QTextStream &doc, ListViewInterface *item)
{
    kdDebug(100200) << "DocTreeViewImpl::writeSect1()" << endl;
    // read item's content
    QString whoAmI = (item->getValue(KSayItGlobal::XMLCONTEXTNAME)).toString();
    if ( whoAmI != "Sect1" )
        return;

    QString title = item->text(0);

    // add node to document
    doc << "<Sect1>" << endl;
        doc << "<Title>" << title << "</Title>" << endl;

    // process childs
    ListViewInterface *i = static_cast<ListViewInterface*>(item->firstChild());
    QString itemType;
    while( i ){
        itemType = (i->getValue(KSayItGlobal::XMLCONTEXTNAME)).toString();
        if ( itemType == "Para" ){
            writePara( doc, i );
        } else if ( itemType == "Sect2" ) {
            writeSect2( doc, i );
        }
        // next child
        i = static_cast<ListViewInterface*>(i->nextSibling());
    }

    doc << "</Sect1>" << endl;
}


void DocbookGenerator::writeSect2(QTextStream &doc, ListViewInterface *item)
{
    kdDebug(100200) << "DocTreeViewImpl::writeSect1()" << endl;
    // read item's content
    QString whoAmI = (item->getValue(KSayItGlobal::XMLCONTEXTNAME)).toString();
    if ( whoAmI != "Sect2" )
        return;

    QString title = item->text(0);

    // add node to document
    doc << "<Sect2>" << endl;
        doc << "<Title>" << title << "</Title>" << endl;

    // process childs
    ListViewInterface *i = static_cast<ListViewInterface*>(item->firstChild());
    QString itemType;
    while( i ){
        itemType = ( i->getValue(KSayItGlobal::XMLCONTEXTNAME) ).toString();
        if ( itemType == "Para" ){
            writePara( doc, i );
        } else if ( itemType == "Sect3" ) {
            writeSect3( doc, i );
        }
        // next child
        i = static_cast<ListViewInterface*>(i->nextSibling());
    }

    doc << "</Sect2>" << endl;
}


void DocbookGenerator::writeSect3(QTextStream &doc, ListViewInterface *item)
{
    kdDebug(100200) << "DocTreeViewImpl::writeSect3()" << endl;
    // read item's content
    QString whoAmI = (item->getValue(KSayItGlobal::XMLCONTEXTNAME)).toString();
    if ( whoAmI != "Sect3" )
        return;

    QString title = item->text(0);

    // add node to document
    doc << "<Sect3>" << endl;
        doc << "<Title>" << title << "</Title>" << endl;

    // process childs
    ListViewInterface *i = static_cast<ListViewInterface*>(item->firstChild());
    QString itemType;
    while( i ){
        itemType = (i->getValue(KSayItGlobal::XMLCONTEXTNAME)).toString();
        if ( itemType == "Para" ){
            writePara( doc, i );
        } else if ( itemType == "Sect4" ) {
            writeSect4( doc, i );
        }
        // next child
        i = static_cast<ListViewInterface*>(i->nextSibling());
    }

    doc << "</Sect3>" << endl;
}


void DocbookGenerator::writeSect4(QTextStream &doc, ListViewInterface *item)
{
    kdDebug(100200) << "DocTreeViewImpl::writeSect4()" << endl;
    // read item's content
    QString whoAmI = (item->getValue(KSayItGlobal::XMLCONTEXTNAME)).toString();
    if ( whoAmI != "Sect4" )
        return;

    QString title = item->text(0);

    // add node to document
    doc << "<Sect4>" << endl;
        doc << "<Title>" << title << "</Title>" << endl;

    // process childs
    ListViewInterface *i = static_cast<ListViewInterface*>(item->firstChild());
    QString itemType;
    while( i ){
        itemType = (i->getValue(KSayItGlobal::XMLCONTEXTNAME)).toString();
        if ( itemType == "Para" ){
            writePara( doc, i );
        } else if ( itemType == "Sect5" ) {
            writeSect5( doc, i );
        }
        // next child
        i = static_cast<ListViewInterface*>(i->nextSibling());
    }

    doc << "</Sect4>" << endl;
}


void DocbookGenerator::writeSect5(QTextStream &doc, ListViewInterface *item)
{
    kdDebug(100200) << "DocTreeViewImpl::writeSect5()" << endl;
    // read item's content
    QString whoAmI = (item->getValue(KSayItGlobal::XMLCONTEXTNAME)).toString();
    if ( whoAmI != "Sect5" )
        return;

    QString title = item->text(0);

    // add node to document
    doc << "<Sect5>" << endl;
        doc << "<Title>" << title << "</Title>" << endl;

    // process childs
    ListViewInterface *i = static_cast<ListViewInterface*>(item->firstChild());
    QString itemType;
    while( i ){
        itemType = (i->getValue(KSayItGlobal::XMLCONTEXTNAME)).toString();
        if ( itemType == "Para" ){
            writePara( doc, i );
        }
        // next child
        i = static_cast<ListViewInterface*>(i->nextSibling());
    }

    doc << "</Sect5>" << endl;
}


void DocbookGenerator::writePara(QTextStream &doc, ListViewInterface *item)
{
    kdDebug(100200) << "DocTreeViewImpl::writePara()" << endl;
    // read item's content
    QString whoAmI = (item->getValue(KSayItGlobal::XMLCONTEXTNAME)).toString();
    if ( whoAmI != "Para" )
        return;

    QString data = ( item->getValue(KSayItGlobal::RAWDATA) ).toString();

    // add node to document
    doc << "<Para>" << endl;
    doc << data << endl;
    doc << "</Para>" << endl;

    // process childs
    // childs are embedded in data.
}




