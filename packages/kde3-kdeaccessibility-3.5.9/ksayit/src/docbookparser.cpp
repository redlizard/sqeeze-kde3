//
// C++ Implementation: docbookparser
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
#include <qregexp.h>
#include <qstring.h>
#include <qtextstream.h>

//KDE includes
#include <kdebug.h>
#include <klocale.h>

// App specific includes
#include "docbookparser.h"
#include "docbookclasses.h"


DocbookParser::DocbookParser(ContextMenuHandler *cmHandler)
{
    m_contextmenuhandler = cmHandler;
    m_idCounter = KSayItGlobal::item_initial_id;
}


DocbookParser::~DocbookParser()
{
}


QString DocbookParser::node2raw(QDomNode node) const
{
    QString ret;
    
    // get content of element
    QTextStream in(ret, IO_WriteOnly);
    in.setEncoding(QTextStream::UnicodeUTF8);
    in << node;
    
    return ret;
}


/******************************************
 * XML-Parser functions
 ******************************************/
//
// DocBook parser
//
void DocbookParser::parseBook(const QDomElement &element, ListViewInterface *item)
{
    kdDebug(100200) << "+++ entering parseBook()" << endl;
    item->setText(3, QString("%1").arg(++m_idCounter).rightJustify(8,'0') );
    // register Popup menu
    m_contextmenuhandler->registerPopupMenu( item );
          
    // TODO: Attributes Id, Lang

    // Elements (parse BookInfo before Chapter)
    QDomNode node = element.firstChild();
    while( !node.isNull() ){
        if( node.isElement() && node.nodeName().lower()=="bookinfo" ){
            parseBookInfo( node.toElement(), item );
            break;
        }
        node = node.nextSibling();
    }
    node = element.firstChild();
    while( !node.isNull() ){    
        if ( node.isElement() && node.nodeName().lower()=="chapter" ){
            parseChapter( node.toElement(), item );
        }
        node = node.nextSibling();
    }
    kdDebug(100200) << "--- leaving parseBook()" << endl;
}


void DocbookParser::parseBookInfo(const QDomElement &element, ListViewInterface *item)
{
    kdDebug(100200) << "+++ entering parseBookInfo()" << endl;

    Overview *overview = new Overview(item, NULL, i18n("Overview"));
    overview->setText(3, QString("%1").arg(++m_idCounter).rightJustify(8,'0') );
    // register Popup menu
    m_contextmenuhandler->registerPopupMenu( overview );
    
    QDomText data;
    QDomNode node = element.firstChild();
    // fixed order
    while( !node.isNull() ){
        if( node.isElement() && node.nodeName().lower()=="title" ){
            data = node.firstChild().toText();
            if( !data.isNull() ){
                item->setText( 0, data.nodeValue() ); // assign title to RootItem                
                item->setValue(KSayItGlobal::RTFHEADER, data.nodeValue() );
                item->setValue(KSayItGlobal::SPEAKERHEADER, data.nodeValue() );                
                
                break;
            }
        }
        node = node.nextSibling();
    }
    node = element.firstChild();
    while( !node.isNull() ){
        if ( node.isElement() && node.nodeName().lower()=="authorgroup" ){
            parseAuthorGroup( node.toElement(), overview );
        }
        node = node.nextSibling();
    }
    node = element.firstChild();
    while( !node.isNull() ){
        if ( node.isElement() && node.nodeName().lower()=="date" ){
            data = node.firstChild().toText();
            if( !data.isNull() ){
                Date *date = new Date(overview, NULL, i18n("Date"));
                date->setText(3, QString("%1").arg(++m_idCounter).rightJustify(8,'0') );
                date->setText( 1, data.nodeValue() );
                date->setValue(KSayItGlobal::RTFDATA, data.nodeValue());
                date->setValue(KSayItGlobal::SPEAKERDATA, data.nodeValue());
                date->setValue(KSayItGlobal::RAWDATA, node2raw(data));
            
                // register Popup menu
                m_contextmenuhandler->registerPopupMenu( date );
            }
        }
        node = node.nextSibling();
    }
    node = element.firstChild();
    while( !node.isNull() ){
        if ( node.isElement() && node.nodeName().lower()=="releaseinfo" ){
            data = node.firstChild().toText();
            if( !data.isNull() ){
                ReleaseInfo *relinfo = new ReleaseInfo(overview, NULL, i18n("Release"));
                relinfo->setText(3, QString("%1").arg(++m_idCounter).rightJustify(8,'0') );
                relinfo->setText( 1, data.nodeValue() );
                relinfo->setValue(KSayItGlobal::RTFDATA, data.nodeValue());
                relinfo->setValue(KSayItGlobal::SPEAKERDATA, data.nodeValue());
                relinfo->setValue(KSayItGlobal::RAWDATA, node2raw(data));
                
                // register Popup menu
                m_contextmenuhandler->registerPopupMenu( relinfo );
            }
        }
        node = node.nextSibling();
    }    
    node = element.firstChild();
    while( !node.isNull() ){
         if ( node.isElement() && node.nodeName().lower()=="keywordset" ){
            parseKeywordSet( node.toElement(), overview );
        }
        node = node.nextSibling();
    }        
    node = element.firstChild();
    while( !node.isNull() ){        
        if ( node.isElement() && node.nodeName().lower()=="abstract" ){
            parseAbstract( node.toElement(), overview );
        }
        node = node.nextSibling();
    }
    kdDebug(100200) << "--- leaving parseBookInfo()" << endl;
}


void DocbookParser::parseAuthorGroup(const QDomElement &element, ListViewInterface *item)
{
    kdDebug(100200) << "+++ entering parseAuthorGroup()" << endl;

    // item = overview
    AuthorGroup *authorgroup = new AuthorGroup(item, NULL, i18n("Author(s)"));
    authorgroup->setText(3, QString("%1").arg(++m_idCounter).rightJustify(8,'0') );
    // register Popup menu
    m_contextmenuhandler->registerPopupMenu( authorgroup );

    QDomNode node = element.firstChild();
    while( !node.isNull() ){
        if( node.isElement() && node.nodeName().lower()=="author" ){
            parseAuthor( node.toElement(), authorgroup );
        }
        node = node.nextSibling();
    }
    kdDebug(100200) << "--- leaving parseAuthorGroup()" << endl;
}


void DocbookParser::parseAuthor(const QDomElement &element, ListViewInterface *item)
{
    kdDebug(100200) << "+++ entering parseAuthor()" << endl;

    // item = authorgroup
    QDomText data;
    QDomText firstname = QDomText();
    QDomText surname = QDomText();
    QString s_firstname = QString::null;
    QString s_surname = QString::null;

    Author *author = new Author(item);
    author->setText(3, QString("%1").arg(++m_idCounter).rightJustify(8,'0') );

    QDomNode node = element.firstChild();
    while( !node.isNull() ){
        if( node.isElement() && node.nodeName().lower()=="firstname" ){
            data = node.firstChild().toText();
            if( !data.isNull() ){
                s_firstname = data.nodeValue();
                firstname = data;
            }
        } else if ( node.isElement() && node.nodeName().lower()=="surname" ){
            data = node.firstChild().toText();
            if( !data.isNull() ){
                s_surname = data.nodeValue();
                surname = data;
            }
        }
        node = node.nextSibling();
    }
    // register Popup menu
    m_contextmenuhandler->registerPopupMenu( author );

    author->setText(0, i18n("Author") );
    author->setText(1, s_firstname + " " + s_surname);
    author->setValue(KSayItGlobal::RTFDATA, s_firstname + " " + s_surname);
    author->setValue(KSayItGlobal::SPEAKERDATA, s_firstname + " " + s_surname);
    author->setValue(KSayItGlobal::RAWDATA, s_firstname + " " + s_surname);
    
    kdDebug(100200) << "--- leaving parseAuthor()" << endl;
}


void DocbookParser::parseKeywordSet(const QDomElement &element, ListViewInterface *item)
{
    kdDebug(100200) << "+++ entering parseKeywordSet()" << endl;

    // item = overview
    KeywordSet *keywordset = new KeywordSet(item, NULL, i18n("Keywords"));
    keywordset->setText(3, QString("%1").arg(++m_idCounter).rightJustify(8,'0') );
    // register Popup menu
    m_contextmenuhandler->registerPopupMenu( keywordset );
    
    QDomText data;
    QDomNode node = element.firstChild();
    while( !node.isNull() ){
        if( node.isElement() && node.nodeName().lower()=="keyword" ){
            data = node.firstChild().toText();
            if( !data.isNull() ){
                Keyword *keyword = new Keyword(keywordset);
                keyword->setText(3, QString("%1").arg(++m_idCounter).rightJustify(8,'0') );
                keyword->setText(0, data.nodeValue() );
                keyword->setValue(KSayItGlobal::RTFDATA,     data.nodeValue() );
                keyword->setValue(KSayItGlobal::SPEAKERDATA, data.nodeValue() );
                keyword->setValue(KSayItGlobal::RAWDATA, node2raw(data) );

                // register Popup menu
                m_contextmenuhandler->registerPopupMenu( keyword );
            }
        }
        node = node.nextSibling();
    }
    kdDebug(100200) << "--- leaving parseKeywordSet()" << endl;
}


void DocbookParser::parseAbstract(const QDomElement &element, ListViewInterface *item)
{
    kdDebug(100200) << "+++ entering parseAbstract()" << endl;

    // item = overview
    QDomText data;
    Abstract *abstract = new Abstract(item, NULL, i18n("Abstract"));
    abstract->setText(3, QString("%1").arg(++m_idCounter).rightJustify(8,'0') );
    // register Popup menu
    m_contextmenuhandler->registerPopupMenu( abstract );

    QDomNode node = element.firstChild();
    while( !node.isNull() ){
        if( node.isElement() && node.nodeName().lower()=="para" ){
            parsePara( node.toElement(), abstract );
        }
        node = node.nextSibling();
    }
    kdDebug(100200) << "--- leaving parseAbstract()" << endl;
}


void DocbookParser::parseChapter(const QDomElement &element, ListViewInterface *item)
{
    kdDebug(100200) << "+++ entering parseChapter()" << endl;
    
    QDomText data;
    Chapter *chapter = new Chapter(item, NULL, i18n("Chapter"));
    chapter->setText(3, QString("%1").arg(++m_idCounter).rightJustify(8,'0') );
    // register Popup menu
    m_contextmenuhandler->registerPopupMenu( chapter );
    
    QDomNode node = element.firstChild();
    while( !node.isNull() ){
        if( node.isElement() && node.nodeName().lower()=="title" ){
            data = node.firstChild().toText();
            if( !data.isNull() ){
                chapter->setText( 0, data.nodeValue() );
                chapter->setValue(KSayItGlobal::RTFHEADER, data.nodeValue());
                chapter->setValue(KSayItGlobal::SPEAKERHEADER, data.nodeValue());                
            }
        } else if ( node.isElement() && node.nodeName().lower()=="para" ){
            parsePara( node.toElement(), chapter );
        } else if ( node.isElement() && node.nodeName().lower()=="sect1" ){
            parseSect1( node.toElement(), chapter );
        }
        node = node.nextSibling();
    }
    kdDebug(100200) << "--- leaving parseChapter()" << endl;
}


void DocbookParser::parseSect1(const QDomElement &element, ListViewInterface *item)
{
    kdDebug(100200) << "+++ entering parseSect1()" << endl;

    QDomText data;
    Sect1 *sect1 = new Sect1(item, NULL, i18n("Section Level 1"));
    sect1->setText(3, QString("%1").arg(++m_idCounter).rightJustify(8,'0') );
    // register Popup menu
    m_contextmenuhandler->registerPopupMenu( sect1 );

    QDomNode node = element.firstChild();
    while( !node.isNull() ){
        if( node.isElement() && node.nodeName().lower()=="title" ){
            data = node.firstChild().toText();
            if( !data.isNull() ){
                sect1->setText( 0, data.nodeValue() );
                sect1->setValue(KSayItGlobal::RTFHEADER, data.nodeValue());
                sect1->setValue(KSayItGlobal::SPEAKERHEADER, data.nodeValue());                
            }
        } else if ( node.isElement() && node.nodeName().lower()=="para" ){
            parsePara( node.toElement(), sect1 );
        } else if ( node.isElement() && node.nodeName().lower()=="sect2" ){
            parseSect2( node.toElement(), sect1 );
        }
        node = node.nextSibling();
    }
    kdDebug(100200) << "--- leaving parseSect1()" << endl;
}


void DocbookParser::parseSect2(const QDomElement &element, ListViewInterface *item)
{
    kdDebug(100200) << "+++ entering parseSect2()" << endl;

    QDomText data;
    Sect2 *sect2 = new Sect2(item, NULL, i18n("Section Level 2"));
    sect2->setText(3, QString("%1").arg(++m_idCounter).rightJustify(8,'0') );
    // register Popup menu
    m_contextmenuhandler->registerPopupMenu( sect2 );

    QDomNode node = element.firstChild();
    while( !node.isNull() ){
        if( node.isElement() && node.nodeName().lower()=="title" ){
            data = node.firstChild().toText();
            if( !data.isNull() ){
                sect2->setText( 0, data.nodeValue() );
                sect2->setValue(KSayItGlobal::RTFHEADER, data.nodeValue());
                sect2->setValue(KSayItGlobal::SPEAKERHEADER, data.nodeValue());                
            }
        } else if ( node.isElement() && node.nodeName().lower()=="para" ){
            parsePara( node.toElement(), sect2 );
        } else if ( node.isElement() && node.nodeName().lower()=="sect3" ){
            parseSect3( node.toElement(), sect2 );
        }
        node = node.nextSibling();
    }
    kdDebug(100200) << "--- leaving parseSect2()" << endl;
}


void DocbookParser::parseSect3(const QDomElement &element, ListViewInterface *item)
{
    kdDebug(100200) << "+++ entering parseSect3()" << endl;

    QDomText data;
    Sect3 *sect3 = new Sect3(item, NULL, i18n("Section Level 3"));
    sect3->setText(3, QString("%1").arg(++m_idCounter).rightJustify(8,'0') );
    // register Popup menu
    m_contextmenuhandler->registerPopupMenu( sect3 );

    QDomNode node = element.firstChild();
    while( !node.isNull() ){
        if( node.isElement() && node.nodeName().lower()=="title" ){
            data = node.firstChild().toText();
            if( !data.isNull() ){
                sect3->setText( 0, data.nodeValue() );
                sect3->setValue(KSayItGlobal::RTFHEADER, data.nodeValue());
                sect3->setValue(KSayItGlobal::SPEAKERHEADER, data.nodeValue());                
            }
        } else if ( node.isElement() && node.nodeName().lower()=="para" ){
            parsePara( node.toElement(), sect3 );
        } else if ( node.isElement() && node.nodeName().lower()=="sect4" ){
            parseSect4( node.toElement(), sect3 );
        }
        node = node.nextSibling();
    }
    kdDebug(100200) << "--- leaving parseSect3()" << endl;
}


void DocbookParser::parseSect4(const QDomElement &element, ListViewInterface *item)
{
    kdDebug(100200) << "+++ entering parseSect4()" << endl;
    
    QDomText data;
    Sect4 *sect4 = new Sect4(item, NULL, i18n("Section Level 4"));
    sect4->setText(3, QString("%1").arg(++m_idCounter).rightJustify(8,'0') );
    // register Popup menu
    m_contextmenuhandler->registerPopupMenu( sect4 );

    QDomNode node = element.firstChild();
    while( !node.isNull() ){
        if( node.isElement() && node.nodeName().lower()=="title" ){
            data = node.firstChild().toText();
            if( !data.isNull() ){
                sect4->setText( 0, data.nodeValue() );
                sect4->setValue(KSayItGlobal::RTFHEADER, data.nodeValue());
                sect4->setValue(KSayItGlobal::SPEAKERHEADER, data.nodeValue());                
            }
        } else if ( node.isElement() && node.nodeName().lower()=="para" ){
            parsePara( node.toElement(), sect4 );
        } else if ( node.isElement() && node.nodeName().lower()=="sect5" ){
            parseSect5( node.toElement(), sect4 );
        }
        node = node.nextSibling();
    }
    kdDebug(100200) << "--- leaving parseSect4()" << endl;
}


void DocbookParser::parseSect5(const QDomElement &element, ListViewInterface *item)
{
    kdDebug(100200) << "+++ entering parseSect5()" << endl;
    
    QDomText data;
    Sect5 *sect5 = new Sect5(item, NULL, i18n("Section Level 4"));
    sect5->setText(3, QString("%1").arg(++m_idCounter).rightJustify(8,'0') );
    // register Popup menu
    m_contextmenuhandler->registerPopupMenu( sect5 );

    QDomNode node = element.firstChild();
    while( !node.isNull() ){
        if( node.isElement() && node.nodeName().lower()=="title" ){
            data = node.firstChild().toText();
            if( !data.isNull() ){
                sect5->setText( 0, data.nodeValue() );
                sect5->setValue(KSayItGlobal::RTFHEADER, data.nodeValue());
                sect5->setValue(KSayItGlobal::SPEAKERHEADER, data.nodeValue());                
            }
        } else if ( node.isElement() && node.nodeName().lower()=="para" ){
            parsePara( node.toElement(), sect5 );
        }
        node = node.nextSibling();
    }
    kdDebug(100200) << "--- leaving parseSect5()" << endl;
}


void DocbookParser::parsePara(const QDomElement &element, ListViewInterface *item)
{
    kdDebug(100200) << "+++ entering parsePara()" << endl;

    Para *para = new Para(item, NULL, i18n("Paragraph"));
    para->setText(3, QString("%1").arg(++m_idCounter).rightJustify(8,'0') );
    // register Popup menu
    m_contextmenuhandler->registerPopupMenu( para );
    
    QString raw = node2raw(element);
    // remove <para> tags
    raw.replace( QRegExp("</?(para|Para|PARA)/?>"),"");
    raw.replace( QRegExp("^ "),"" );
    raw.replace( QRegExp("^\n"), "" );
    
    para->setValue(KSayItGlobal::RAWDATA,     raw);
    para->setValue(KSayItGlobal::RTFDATA,     raw);
    para->setValue(KSayItGlobal::SPEAKERDATA, raw);    
    
    kdDebug(100200) << "--- leaving parsePara()" << endl;
}

 
