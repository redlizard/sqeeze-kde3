//
// C++ Implementation: docbookclasses
//
// Description: 
//
//
// Author: Robert Vogl <voglrobe@lapislazuli>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
// Qt includes
#include <qwidget.h>
#include <qtextstream.h>
#include <qpixmap.h>
#include <qregexp.h>
#include <qstringlist.h>

//KDE includes
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>

// App specific includes
#include "parasaxparser.h"
#include "docbookclasses.h"


//////////////////////////////////////
// Interface
//////////////////////////////////////
ListViewInterface::ListViewInterface(ListViewInterface *parent, QString label)
    : KListViewItem( parent, label )
{
}

ListViewInterface::ListViewInterface(ListViewInterface *parent, ListViewInterface *after, QString label) : KListViewItem( parent, after, label )
{
}

ListViewInterface::ListViewInterface(KListView *lv, QString label)
    : KListViewItem( lv, label )
{
}


//////////////////////////////////////
// Classes for Roberts elements
//////////////////////////////////////
/**
 * RobDocument
 */
RobDocument::RobDocument(KListView *lv, QString label)
    : ListViewInterface( lv, label )
{
    QPixmap pixmap = KGlobal::iconLoader()->loadIcon("contents", KIcon::Small);
    this->setPixmap(0, pixmap);
    
    // new Concept (begin)
    m_valueList.clear();
    m_valueList.append( QString("RobDocument") );                            // XmlContextName
    m_valueList.append( (bool)false );                                       // isEditable
    m_valueList.append( (int)0 );                                            // MaxLines
    m_valueList.append(QString::null);                                       // Summary
    m_valueList.append(QString::null);                                       // RawData
    m_valueList.append("<H1><center>" + i18n("unnamed") + "</center></H1>"); // RTFHeader
    m_valueList.append(QString::null);                                       // RTFData
    m_valueList.append(i18n("unnamed") + "\n");                              // SpeakerHeader
    m_valueList.append(QString::null);                                       // SpeakerData
    // new Concept (end)      
}

RobDocument::~RobDocument()
{
}

bool RobDocument::setValue( unsigned int index, QVariant data )
{
    if ( index<1 || index > m_valueList.count() )
        return false;

    QValueList<QVariant>::Iterator it = m_valueList.at(index);
    switch (index){
        case KSayItGlobal::ISEDITABLE:
            *it = data;
            break;
        case KSayItGlobal::RTFHEADER:
            *it = "<H1><center>" + data.toString() + "</center></H1>";
            break;
        case KSayItGlobal::SPEAKERHEADER:
            *it = data.toString() + "\n";
            break;
        default:
            return false;
    }
    return true;
}

QVariant RobDocument::getValue( unsigned int index ) const
{
    if (  index > m_valueList.count() )
        return QVariant(); // a invalid QVariant

    QVariant data;
    data = m_valueList[index];

    return data;
}


//////////////////////////////////////
// Classes for Docbook elements
//////////////////////////////////////
/**
 * Overview
 */
Overview::Overview(ListViewInterface *parent, ListViewInterface *after, QString label)
    : ListViewInterface(parent, after, label)
{
    QPixmap pixmap = KGlobal::iconLoader()->loadIcon("filenew", KIcon::Small);
    this->setPixmap(0, pixmap);
    this->setText(3, "0"); // upmost element

    // new Concept (begin)
    m_valueList.clear();
    m_valueList.append( QString("BookInfo") );               // XmlContextName
    m_valueList.append( (bool)false );                       // isEditable
    m_valueList.append( (int)0 );                            // MaxLines 
    m_valueList.append(QString::null);                       // Summary
    m_valueList.append(QString::null);                       // RawData
    m_valueList.append("<H1>" + i18n("Overview") + "</H1>"); // RTFHeader
    m_valueList.append(QString::null);                       // RTFData
    m_valueList.append(i18n("Overview") + "\n");             // SpeakerHeader
    m_valueList.append(QString::null);                       // SpeakerData
    // new Concept (end)      
}

Overview::~Overview()
{
}

bool Overview::setValue( unsigned int index, QVariant data )
{
    if ( index<1 || index > m_valueList.count() )
        return false;

    QValueList<QVariant>::Iterator it = m_valueList.at(index);
    switch (index){
        case KSayItGlobal::ISEDITABLE:
            *it = data;
            break;
        default:
            return false;
    }
    return true;
}

QVariant Overview::getValue( unsigned int index ) const
{
    if (  index > m_valueList.count() )
        return QVariant(); // a invalid QVariant

    QVariant data;
    data = m_valueList[index];

    return data;
}

/**
 * Date
 */
Date::Date(ListViewInterface *parent, ListViewInterface *after, QString label)
    : ListViewInterface(parent, after, label)
{
    QPixmap pixmap = KGlobal::iconLoader()->loadIcon("filenew", KIcon::Small);
    this->setPixmap(0, pixmap);
    this->setText(3, "00000001");
    
    // new Concept (begin)
    m_valueList.clear();
    m_valueList.append( QString("Date") );              // XmlContextName
    m_valueList.append( (bool)false );                  // isEditable
    m_valueList.append( (int)1 );                       // MaxLines
    m_valueList.append(QString::null);                  // Summary
    m_valueList.append(QString::null);                  // RawData
    m_valueList.append("<B>" + i18n("Date") +": </B>"); // RTFHeader
    m_valueList.append(QString::null);                  // RTFData
    m_valueList.append(i18n("Date") + ": ");            // SpeakerHeader
    m_valueList.append(QString::null);                  // SpeakerData
    // new Concept (end)
}

Date::~Date()
{
}
    
bool Date::setValue( unsigned int index, QVariant data )
{
    if ( index<1 || index > m_valueList.count() )
        return false;

    QValueList<QVariant>::Iterator it = m_valueList.at(index);
    switch (index){
        case KSayItGlobal::ISEDITABLE:
            *it = data;
            break;
        case KSayItGlobal::MAXLINES:
            *it = data;
            break;
        case KSayItGlobal::RTFDATA:
            *it = data.toString() + "<br>";
            break;
        case KSayItGlobal::SPEAKERDATA:
            *it = data;
            break;
        case KSayItGlobal::RAWDATA:
            *it = data;
            break;
        default:
            return false;
    }
    return true;
}

QVariant Date::getValue( unsigned int index ) const
{
    if (  index > m_valueList.count() )
        return QVariant(); // a invalid QVariant

    QVariant data;
    data = m_valueList[index];

    return data;
}

/**
 * ReleaseInfo
 */
ReleaseInfo::ReleaseInfo(ListViewInterface *parent, ListViewInterface *after, QString label)
    : ListViewInterface(parent, after, label)
{
    QPixmap pixmap = KGlobal::iconLoader()->loadIcon("filenew", KIcon::Small);
    this->setPixmap(0, pixmap);
    this->setText(3, "00000002");

    // new Concept (begin)
    m_valueList.clear();
    m_valueList.append( QString("ReleaseInfo") );           // XmlContextName
    m_valueList.append( (bool)false );                      // isEditable
    m_valueList.append( (int)1 );                           // MaxLines
    m_valueList.append(QString::null);                      // Summary
    m_valueList.append(QString::null);                      // RawData
    m_valueList.append("<B>" + i18n("Release") + ": </B>"); // RTFHeader
    m_valueList.append(QString::null);                      // RTFData
    m_valueList.append(i18n("Release") + ": ");             // SpeakerHeader
    m_valueList.append(QString::null);                      // SpeakerData
    // new Concept (end)
}

ReleaseInfo::~ReleaseInfo()
{
}

bool ReleaseInfo::setValue( unsigned int index, QVariant data )
{
    if ( index<1 || index > m_valueList.count() )
        return false;

    QValueList<QVariant>::Iterator it = m_valueList.at(index);
    switch (index){
        case KSayItGlobal::ISEDITABLE:
            *it = data;
            break;
        case KSayItGlobal::MAXLINES:
            *it = data;
            break;
        case KSayItGlobal::RTFDATA:
            *it = data.toString() + "<br>";
            break;
        case KSayItGlobal::SPEAKERDATA:
            *it = data;
            break;
        case KSayItGlobal::RAWDATA:
            *it = data;
            break;
        default:
            return false;
    }
    return true;
}

QVariant ReleaseInfo::getValue( unsigned int index ) const
{
    if (  index > m_valueList.count() )
        return QVariant(); // a invalid QVariant

    QVariant data;
    data = m_valueList[index];

    return data;
}

/**
 * Authorgroup
 */
AuthorGroup::AuthorGroup(ListViewInterface *parent, ListViewInterface *after, QString label)
    : ListViewInterface(parent, after, label)
{
    QPixmap pixmap = KGlobal::iconLoader()->loadIcon("kdmconfig", KIcon::Small);
    this->setPixmap(0, pixmap);
    this->setText(3, "00000003");

    // new Concept (begin)
    m_valueList.clear();
    m_valueList.append( QString("AuthorGroup") );            // XmlContextName
    m_valueList.append( (bool)false );                       // isEditable
    m_valueList.append( (int)0 );                            // MaxLines
    m_valueList.append(QString::null);                       // Summary
    m_valueList.append(QString::null);                       // RawData
    m_valueList.append("<H2>" + i18n("Author(s)")+ "</H2>"); // RTFHeader
    m_valueList.append(QString::null);                       // RTFData
    m_valueList.append(i18n("Author(s)") + "\n");            // SpeakerHeader
    m_valueList.append(QString::null);                       // SpeakerData
    // new Concept (end)
}

AuthorGroup::~AuthorGroup()
{
}
    
bool AuthorGroup::setValue( unsigned int index, QVariant data )
{
    if ( index<1 || index > m_valueList.count() )
        return false;

    QValueList<QVariant>::Iterator it = m_valueList.at(index);
    switch (index){
        case KSayItGlobal::ISEDITABLE:
            *it = data;
            break;
        default:
            return false;
    }
    return true;
}

QVariant AuthorGroup::getValue( unsigned int index ) const
{
    if (  index > m_valueList.count() )
        return QVariant(); // a invalid QVariant

    QVariant data;
    data = m_valueList[index];

    return data;
}


/**
 * Author
 */
Author::Author(ListViewInterface *parent, ListViewInterface *after, QString label)
    : ListViewInterface(parent, after, label)
{
    QPixmap pixmap = KGlobal::iconLoader()->loadIcon("personal", KIcon::Small);
    this->setPixmap(0, pixmap);

    // new Concept (begin)
    m_valueList.clear();
    m_valueList.append( QString("Author") );               // XmlContextName
    m_valueList.append( (bool)false );                     // isEditable
    m_valueList.append( (int)1 );                          // MaxLines
    m_valueList.append(QString::null);                     // Summary
    m_valueList.append(QString::null);                     // RawData
    m_valueList.append("<B>" + i18n("Author") + ": </B>"); // RTFHeader
    m_valueList.append(QString::null);                     // RTFData
    m_valueList.append(i18n("Author") + ": ");             // SpeakerHeader
    m_valueList.append(QString::null);                     // SpeakerData
    // new Concept (end)
}

Author::~Author()
{
}
    
// void Author::setData(const QString &data, QDomDocument &)
// {
//     // canonify string
//     QString m_data = data;
//     m_data.replace( QRegExp("\n"), "" ); // remove Newlines
//     m_data.replace( QRegExp(" {2,}"), " " ); // remove multiple spaces
//     m_data.replace( QRegExp("[\t|\r]{1,}"), ""); // remove Tabs
//     // split string "firstname surname"
//     QString firstname = m_data.section(' ', 0, 0);
//     QString surname = m_data.section(' ', 1, 1);
//     // update node elements
//     m_node1.setNodeValue(firstname);
//     m_node2.setNodeValue(surname);
//     // update Listview
//     this->setText(1, firstname + " " + surname);
// }   

bool Author::setValue( unsigned int index, QVariant data )
{
    if ( index<1 || index > m_valueList.count() )
        return false;

    QValueList<QVariant>::Iterator it = m_valueList.at(index);
    switch (index){
        case KSayItGlobal::ISEDITABLE:
            *it = data;
            break;
        case KSayItGlobal::MAXLINES:
            *it = data;
            break;
        case KSayItGlobal::RTFDATA:
            *it = data.toString() + "<br>";
            break;
        case KSayItGlobal::SPEAKERDATA:
            *it = data;
            this->setText( 1, data.toString() );
            break;
        case KSayItGlobal::RAWDATA:
            *it = data;
            break;
        default:
            return false;
    }
    return true;
}

QVariant Author::getValue( unsigned int index ) const
{
    if (  index > m_valueList.count() )
        return QVariant(); // a invalid QVariant

    QVariant data;
    data = m_valueList[index];

    return data;
}

/**
 * KeywordSet
 */
KeywordSet::KeywordSet(ListViewInterface *parent, ListViewInterface *after, QString label)
    : ListViewInterface(parent, after, label)
{
    QPixmap pixmap = KGlobal::iconLoader()->loadIcon("txt", KIcon::Small);
    this->setPixmap(0, pixmap);
    this->setText(3, "00000004");

    // new Concept (begin)
    m_valueList.clear();
    m_valueList.append( QString("KeywordSet") );             // XmlContextName
    m_valueList.append( (bool)false );                       // isEditable
    m_valueList.append( (int)0 );                            // MaxLines
    m_valueList.append(QString::null);                       // Summary
    m_valueList.append(QString::null);                       // RawData
    m_valueList.append("<H2>" + i18n("Keywords") + "</H2>"); // RTFHeader
    m_valueList.append(QString::null);                       // RTFData
    m_valueList.append(i18n("Keywords") + "\n");             // SpeakerHeader
    m_valueList.append(QString::null);                       // SpeakerData
    // new Concept (end)
}

KeywordSet::~KeywordSet()
{
}

bool KeywordSet::setValue( unsigned int index, QVariant data )
{
    if ( index<1 || index > m_valueList.count() )
        return false;

    QValueList<QVariant>::Iterator it = m_valueList.at(index);
    switch (index){
        case KSayItGlobal::ISEDITABLE:
            *it = data;
            break;
        default:
            return false;
    }
    return true;
}

QVariant KeywordSet::getValue( unsigned int index ) const
{
    if (  index > m_valueList.count() )
        return QVariant(); // a invalid QVariant

    QVariant data;
    data = m_valueList[index];

    return data;
}

/**
 * Keyword
 */
Keyword::Keyword(ListViewInterface *parent, ListViewInterface *after, QString label)
    : ListViewInterface(parent, after, label)
{
    // new Concept (begin)
    m_valueList.clear();
    m_valueList.append( QString("Keyword") ); // XmlContextName
    m_valueList.append( (bool)false );        // isEditable
    m_valueList.append( (bool)1 );            // MaxLines
    m_valueList.append(QString::null);        // Summary
    m_valueList.append(QString::null);        // RawData
    m_valueList.append(QString::null);        // RTFHeader
    m_valueList.append(QString::null);        // RTFData
    m_valueList.append(QString::null);        // SpeakerHeader
    m_valueList.append(QString::null);        // SpeakerData
    // new Concept (end)
}

Keyword::~Keyword()
{
}

bool Keyword::setValue( unsigned int index, QVariant data )
{
    if ( index<1 || index > m_valueList.count() )
        return false;

    QValueList<QVariant>::Iterator it = m_valueList.at(index);
    switch (index){
        case KSayItGlobal::ISEDITABLE:
            *it = data;
            break;
        case KSayItGlobal::MAXLINES:
            *it = data;
            break;
        case KSayItGlobal::RAWDATA:
            *it = data;
            this->setText( 0, data.toString() );
            break;
        case KSayItGlobal::RTFDATA:
            *it = data.toString() + "<br>";
            break;
        case KSayItGlobal::SPEAKERDATA:
            *it = data;
            break;
        default:
            return false;
    }
    return true;
}

QVariant Keyword::getValue( unsigned int index ) const
{
    if (  index > m_valueList.count() )
        return QVariant(); // a invalid QVariant

    QVariant data;
    data = m_valueList[index];

    return data;
}



/**
 * Abstract
 */
Abstract::Abstract(ListViewInterface *parent, ListViewInterface *after, QString label)
    : ListViewInterface(parent, after, label)
{
    QPixmap pixmap = KGlobal::iconLoader()->loadIcon("filenew", KIcon::Small);
    this->setPixmap(0, pixmap);
    this->setText(3, "00000005");

    // new Concept (begin)
    m_valueList.clear();
    m_valueList.append( QString("Abstract") );               // XmlContextName
    m_valueList.append( (bool)false );                       // isEditable
    m_valueList.append( (int)0 );                            // MaxLines
    m_valueList.append(QString::null);                       // Summary
    m_valueList.append(QString::null);                       // RawData
    m_valueList.append("<H2>" + i18n("Abstract") + "</H2>"); // RTFHeader
    m_valueList.append(QString::null);                       // RTFData
    m_valueList.append(i18n("Abstract") + "\n");             // SpeakerHeader
    m_valueList.append(QString::null);                       // SpeakerData
    // new Concept (end)
}

Abstract::~Abstract()
{
}
    
bool Abstract::setValue( unsigned int index, QVariant data )
{
    if ( index<1 || index > m_valueList.count() )
        return false;

    QValueList<QVariant>::Iterator it = m_valueList.at(index);
    switch (index){
        case KSayItGlobal::ISEDITABLE:
            *it = data;
            break;
        default:
            return false;
    }
    return true;
}

QVariant Abstract::getValue( unsigned int index ) const
{
    if (  index > m_valueList.count() )
        return QVariant(); // a invalid QVariant

    QVariant data;
    data = m_valueList[index];

    return data;
}


/**
 * Para
 */
Para::Para(ListViewInterface *parent, ListViewInterface *after, QString label)
    : ListViewInterface(parent, after, label)
{
    QPixmap pixmap = KGlobal::iconLoader()->loadIcon("leftjust", KIcon::Small);
    this->setPixmap(0, pixmap);

    // new Concept (begin)
    m_valueList.clear();
    m_valueList.append( QString("Para") );        // XmlContextName
    m_valueList.append( (bool)false );            // isEditable
    m_valueList.append( (int)-1 );                // MaxLines
    m_valueList.append(QString::null);            // Summary
    m_valueList.append(QString::null);            // RawData
    m_valueList.append(QString::null);            // RTFHeader
    m_valueList.append(QString::null);            // RTFData
    m_valueList.append(i18n("Paragraph") + "\n"); // SpeakerHeader
    m_valueList.append(QString::null);            // SpeakerData
    // new Concept (end)
}

Para::~Para()
{
}   

bool Para::setValue( unsigned int index, QVariant data )
{
    if ( index<1 || index > m_valueList.count() )
        return false;

    QValueList<QVariant>::Iterator it = m_valueList.at(index);
    QString sData = QString::null;
    ParaSaxParser *p = ParaSaxParser::Instance();
    switch (index){
        case KSayItGlobal::ISEDITABLE:
            *it = data;
            break;
        case KSayItGlobal::MAXLINES:
            *it = data;
            break;
        case KSayItGlobal::RTFDATA:
            p->setData( "<para>" + data.toString() + "</para>" );
            p->getRTFText( sData );
            *it = "<p>" + sData + "</p>";
            break;
        case KSayItGlobal::SPEAKERDATA:
            p->setData( "<para>" + data.toString() + "</para>" );
            p->getText( sData );
            *it = sData;
            break;
        case KSayItGlobal::RAWDATA:
            sData = data.toString();
            *it = sData;
            break;
        default:
            return false;
    }
    return true;
}

QVariant Para::getValue( unsigned int index ) const
{
    if (  index > m_valueList.count() )
        return QVariant(); // a invalid QVariant

    QVariant data;
    data = m_valueList[index];

    return data;
}


/**
 * Chapter
 */
Chapter::Chapter(ListViewInterface *parent, ListViewInterface *after, QString label)
    : ListViewInterface(parent, after, label)
{
    QPixmap pixmap = KGlobal::iconLoader()->loadIcon("leftjust", KIcon::Small);
    this->setPixmap(0, pixmap);
    
    // new Concept (begin)
    m_valueList.clear();
    m_valueList.append( QString("Chapter") ); // XmlContextName
    m_valueList.append( (bool)false );        // isEditable
    m_valueList.append( (int)0 );             // MaxLines
    m_valueList.append(QString::null);        // Summary
    m_valueList.append(QString::null);        // RawData
    m_valueList.append(QString::null);        // RTFHeader
    m_valueList.append(QString::null);        // RTFData
    m_valueList.append(QString::null);        // SpeakerHeader
    m_valueList.append(QString::null);        // SpeakerData
    // new Concept (end)
}

Chapter::~Chapter()
{
}
    
bool Chapter::setValue( unsigned int index, QVariant data )
{
    if ( index<1 || index > m_valueList.count() )
        return false;

    QValueList<QVariant>::Iterator it = m_valueList.at(index);
    switch (index){
        case KSayItGlobal::ISEDITABLE:
            *it = data;
            break;
        case KSayItGlobal::RTFHEADER:
            *it = "<H1>" + data.toString() + "</H1>";
            break;
        case KSayItGlobal::SPEAKERHEADER:
            *it = data.toString() + "\n";
            break;
         default:
            return false;
    }
    return true;
}

QVariant Chapter::getValue( unsigned int index ) const
{
    if (  index > m_valueList.count() )
        return QVariant(); // a invalid QVariant

    QVariant data;
    data = m_valueList[index];

    return data;
}


/**
 * Sect1
 */
Sect1::Sect1(ListViewInterface *parent, ListViewInterface *after, QString label)
    : ListViewInterface(parent, after, label)
{
    QPixmap pixmap = KGlobal::iconLoader()->loadIcon("leftjust", KIcon::Small);
    this->setPixmap(0, pixmap);
    
    // new Concept (begin)
    m_valueList.clear();
    m_valueList.append( QString("Sect1") );   // XmlContextName
    m_valueList.append( (bool)false );        // isEditable
    m_valueList.append( (int)0 );             // MaxLines
    m_valueList.append(QString::null);        // Summary
    m_valueList.append(QString::null);        // RawData
    m_valueList.append(QString::null);        // RTFHeader
    m_valueList.append(QString::null);        // RTFData
    m_valueList.append(QString::null);        // SpeakerHeader
    m_valueList.append(QString::null);        // SpeakerData
    // new Concept (end)
}

Sect1::~Sect1()
{
}
    
bool Sect1::setValue( unsigned int index, QVariant data )
{
    if ( index<1 || index > m_valueList.count() )
        return false;

    QValueList<QVariant>::Iterator it = m_valueList.at(index);
    switch (index){
        case KSayItGlobal::ISEDITABLE:
            *it = data;
            break;
        case KSayItGlobal::RTFHEADER:
            *it = "<H2>" + data.toString() + "</H2>";
            break;
        case KSayItGlobal::SPEAKERHEADER:
            *it = data.toString() + "\n";
            break;
         default:
            return false;
    }
    return true;
}

QVariant Sect1::getValue( unsigned int index ) const
{
    if (  index > m_valueList.count() )
        return QVariant(); // a invalid QVariant

    QVariant data;
    data = m_valueList[index];

    return data;
}

/**
 * Sect2
 */
Sect2::Sect2(ListViewInterface *parent, ListViewInterface *after, QString label)
    : ListViewInterface(parent, after, label)
{
    QPixmap pixmap = KGlobal::iconLoader()->loadIcon("leftjust", KIcon::Small);
    this->setPixmap(0, pixmap);
    
    // new Concept (begin)
    m_valueList.clear();
    m_valueList.append( QString("Sect2") );   // XmlContextName
    m_valueList.append( (bool)false );        // isEditable
    m_valueList.append( (int)0 );             // MaxLines
    m_valueList.append(QString::null);        // Summary
    m_valueList.append(QString::null);        // RawData
    m_valueList.append(QString::null);        // RTFHeader
    m_valueList.append(QString::null);        // RTFData
    m_valueList.append(QString::null);        // SpeakerHeader
    m_valueList.append(QString::null);        // SpeakerData
    // new Concept (end)
}

Sect2::~Sect2()
{
}
    
bool Sect2::setValue( unsigned int index, QVariant data )
{
    if ( index<1 || index > m_valueList.count() )
        return false;

    QValueList<QVariant>::Iterator it = m_valueList.at(index);
    switch (index){
        case KSayItGlobal::ISEDITABLE:
            *it = data;
            break;
        case KSayItGlobal::RTFHEADER:
            *it = "<H3>" + data.toString() + "</H3>";
            break;
        case KSayItGlobal::SPEAKERHEADER:
            *it = data.toString() + "\n";
            break;
         default:
            return false;
    }
    return true;
}

QVariant Sect2::getValue( unsigned int index ) const
{
    if (  index > m_valueList.count() )
        return QVariant(); // a invalid QVariant

    QVariant data;
    data = m_valueList[index];

    return data;
}

/**
 * Sect3
 */
Sect3::Sect3(ListViewInterface *parent, ListViewInterface *after, QString label)
    : ListViewInterface(parent, after, label)
{
    QPixmap pixmap = KGlobal::iconLoader()->loadIcon("leftjust", KIcon::Small);
    this->setPixmap(0, pixmap);

    // new Concept (begin)
    m_valueList.clear();
    m_valueList.append( QString("Sect3") );   // XmlContextName
    m_valueList.append( (bool)false );        // isEditable
    m_valueList.append( (int)0 );             // MaxLines
    m_valueList.append(QString::null);        // Summary
    m_valueList.append(QString::null);        // RawData
    m_valueList.append(QString::null);        // RTFHeader
    m_valueList.append(QString::null);        // RTFData
    m_valueList.append(QString::null);        // SpeakerHeader
    m_valueList.append(QString::null);        // SpeakerData
    // new Concept (end)
}

Sect3::~Sect3()
{
}
    
bool Sect3::setValue( unsigned int index, QVariant data )
{
    if ( index<1 || index > m_valueList.count() )
        return false;

    QValueList<QVariant>::Iterator it = m_valueList.at(index);
    switch (index){
        case KSayItGlobal::ISEDITABLE:
            *it = data;
            break;
        case KSayItGlobal::RTFHEADER:
            *it = "<B>" + data.toString() + "</B><br>";
            break;
        case KSayItGlobal::SPEAKERHEADER:
            *it = data.toString() + "\n";
            break;
         default:
            return false;
    }
    return true;
}

QVariant Sect3::getValue( unsigned int index ) const
{
    if (  index > m_valueList.count() )
        return QVariant(); // a invalid QVariant

    QVariant data;
    data = m_valueList[index];

    return data;
}

/**
 * Sect4
 */
Sect4::Sect4(ListViewInterface *parent, ListViewInterface *after, QString label)
    : ListViewInterface(parent, after, label)
{
    QPixmap pixmap = KGlobal::iconLoader()->loadIcon("leftjust", KIcon::Small);
    this->setPixmap(0, pixmap);
    
    // new Concept (begin)
    m_valueList.clear();
    m_valueList.append( QString("Sect4") );   // XmlContextName
    m_valueList.append( (bool)false );        // isEditable
    m_valueList.append( (int)0 );             // MaxLines
    m_valueList.append(QString::null);        // Summary
    m_valueList.append(QString::null);        // RawData
    m_valueList.append(QString::null);        // RTFHeader
    m_valueList.append(QString::null);        // RTFData
    m_valueList.append(QString::null);        // SpeakerHeader
    m_valueList.append(QString::null);        // SpeakerData
    // new Concept (end)
}

Sect4::~Sect4()
{
}
    
bool Sect4::setValue( unsigned int index, QVariant data )
{
    if ( index<1 || index > m_valueList.count() )
        return false;

    QValueList<QVariant>::Iterator it = m_valueList.at(index);
    switch (index){
        case KSayItGlobal::ISEDITABLE:
            *it = data;
            break;
        case KSayItGlobal::RTFHEADER:
            *it = "<B>" + data.toString() + "</B><br>";
            break;
        case KSayItGlobal::SPEAKERHEADER:
            *it = data.toString() + "\n";
            break;
         default:
            return false;
    }
    return true;
}

QVariant Sect4::getValue( unsigned int index ) const
{
    if (  index > m_valueList.count() )
        return QVariant(); // a invalid QVariant

    QVariant data;
    data = m_valueList[index];

    return data;
}

/**
 * Sect5
 */
Sect5::Sect5(ListViewInterface *parent, ListViewInterface *after, QString label)
    : ListViewInterface(parent, after, label)
{
    QPixmap pixmap = KGlobal::iconLoader()->loadIcon("leftjust", KIcon::Small);
    this->setPixmap(0, pixmap);

    // new Concept (begin)
    m_valueList.clear();
    m_valueList.append( QString("Sect5") );   // XmlContextName
    m_valueList.append( (bool)false );        // isEditable
    m_valueList.append( (int)0 );             // MaxLines
    m_valueList.append(QString::null);        // Summary
    m_valueList.append(QString::null);        // RawData
    m_valueList.append(QString::null);        // RTFHeader
    m_valueList.append(QString::null);        // RTFData
    m_valueList.append(QString::null);        // SpeakerHeader
    m_valueList.append(QString::null);        // SpeakerData
    // new Concept (end)
}

Sect5::~Sect5()
{
}
    
bool Sect5::setValue( unsigned int index, QVariant data )
{
    if ( index<1 || index > m_valueList.count() )
        return false;

    QValueList<QVariant>::Iterator it = m_valueList.at(index);
    switch (index){
        case KSayItGlobal::ISEDITABLE:
            *it = data;
            break;
        case KSayItGlobal::RTFHEADER:
            *it = "<B>" + data.toString() + "</B><br>";
            break;
        case KSayItGlobal::SPEAKERHEADER:
            *it = data.toString() + "\n";
            break;
         default:
            return false;
    }
    return true;
}

QVariant Sect5::getValue( unsigned int index ) const
{
    if (  index > m_valueList.count() )
        return QVariant(); // a invalid QVariant

    QVariant data;
    data = m_valueList[index];

    return data;
}

