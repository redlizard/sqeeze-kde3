//
// C++ Implementation: saxhandler
//
// Description:
//
//
// Author: Robert Vogl <voglrobe@web.de>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
// #include <iostream> // cout
// using namespace std;

// KDE includes
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

// App specific includes
#include "saxhandler.h"

SaxHandler::SaxHandler()
 : QXmlDefaultHandler()
{
    m_output = QString::null;
    m_rtf = true;
    m_tagmap["action"]        = "B";
    m_tagmap["application"]   = "B";
    m_tagmap["function"]      = "B";
    m_tagmap["guibutton"]     = "B";
    m_tagmap["guiicon"]       = "B";
    m_tagmap["guilabel"]      = "B";
    m_tagmap["guimenu"]       = "B";
    m_tagmap["guimenuitem"]   = "B";
    m_tagmap["guisubmenu"]    = "B";
    m_tagmap["menuchoice"]    = "B";
    m_tagmap["mousebutton"]   = "B";
    m_tagmap["option"]        = "B";
    m_tagmap["author"]        = "B";
    m_tagmap["corpauthor"]    = "B";
    m_tagmap["warning"]       = "FONT color=\"red\"";
    m_tagmap["command"]       = "TT";
    m_tagmap["email"]         = "TT";
    m_tagmap["filename"]      = "TT";
    m_tagmap["keycap"]        = "TT";
    m_tagmap["keycode"]       = "TT";
    m_tagmap["keycombo"]      = "TT";
    m_tagmap["keysym"]        = "TT";
    m_tagmap["link"]          = "TT";
    m_tagmap["literal"]       = "TT";
    m_tagmap["userinput"]     = "TT";
    m_tagmap["citation"]      = "EM";
    m_tagmap["emphasis"]      = "EM";
    m_tagmap["foreignphrase"] = "EM";
    m_tagmap["phrase"]        = "EM";
    m_tagmap["comment"]       = "EM";
    m_tagmap["note"]          = "EM";
    m_tagmap["tip"]           = "EM";
    m_tagmap["subscript"]     = "small";
    m_tagmap["superscript"]   = "small";
    m_tagmap["itemizedlist"]  = "UL";
    m_tagmap["listitem"]      = "LI";
}

SaxHandler::~SaxHandler()
{
}


void SaxHandler::setRTF(bool rtf)
{
    m_rtf = rtf;
}


bool SaxHandler::startElement(const QString &,
                    const QString &,
                    const QString & qName,
                    const QXmlAttributes & atts )
{
    if ( !m_rtf )
        return true;
        
    QString tag = qName.lower();
    
    TagMapT::iterator it;
    it = m_tagmap.find(tag);
    if ( it != m_tagmap.end() ){
        // tag found in hash table
        QString rtftag = (*it).second;
        m_output += "<" + rtftag + ">";
    }
    
    return true;
}


bool SaxHandler::endElement(const QString &,
                    const QString &,
                    const QString & qName)
{
    if ( !m_rtf )
        return true;

    QString tag = qName.lower();

    TagMapT::iterator it;
    it = m_tagmap.find(tag);
    if ( it != m_tagmap.end() ){
        // tag found in hash table
        QString rtftag = (*it).second;
        m_output += "</" + rtftag.section(" ", 0, 0) + ">";
    }

    return true;
}


bool SaxHandler::characters(const QString & ch)
{
    m_output += ch;
    return true;
}


bool SaxHandler::fatalError(const QXmlParseException &exc)
{
    QString err = i18n("Fatal error while parsing XML-Paragraph:\n");
    err += i18n("%1, Line: %2").arg(exc.message()).arg(exc.lineNumber());
    KMessageBox::error(0, err, i18n("Fatal error") );
    return false;
}


bool SaxHandler::resolveEntity(const QString &publicId,
                    const QString &systemId,
                    QXmlInputSource* &ret)
{
    return true;
}


bool SaxHandler::externalEntityDecl(const QString & name,
                    const QString & publicId,
                    const QString & systemId)
{
    kdDebug(100200) << "externalEntityDecl(): " << name << ", " << publicId << ", " << systemId << endl;
    return true;
}


bool SaxHandler::internalEntityDecl(const QString & name,
                    const QString & value)
{
    kdDebug() << "internalEntityDecl(): " << name << ", " << value << endl;
    return true;
}


bool SaxHandler::skippedEntity(const QString &name)
{
    QString warn = i18n("Unresolved entity found: %1.\n").arg(name);
    warn += i18n("KSayIt does not support DocBook files with external entities. ");
    warn += i18n("Parsing can continue, but the resulting text will contain gaps.");
    
    int res;   
    // TODO: "Option: don't show again this warning."
    res = KMessageBox::warningContinueCancel(0, warn, i18n("Parser problem") );
    if ( res == KMessageBox::Cancel )
        return false;
    return true;
}


void SaxHandler::getData( QString &data ) const
{
    data = m_output;
}


void SaxHandler::reset()
{
    m_output = QString::null;
}


