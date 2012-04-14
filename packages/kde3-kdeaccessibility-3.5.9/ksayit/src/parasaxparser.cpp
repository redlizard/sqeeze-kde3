//
// C++ Implementation: parasaxparser
//
// Description: SAX2-Parser for 'para' elements of a DocBook file.
//
//
// Author: Robert Vogl <voglrobe@web.de>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
// #include <iostream> // cout
// using namespace std;

// Qt includes

// KDE includes
#include <kdebug.h>

// App specific includes
#include "parasaxparser.h"

ParaSaxParser::ParaSaxParser()
{
    m_reader = new QXmlSimpleReader();

    m_handler = new SaxHandler();
    m_reader->setContentHandler(m_handler);
    m_reader->setErrorHandler(m_handler);
    m_reader->setEntityResolver(m_handler);
    m_reader->setDeclHandler(m_handler);

    m_XmlInputHeader = QString::null;
    m_XmlInputBody   = QString::null;    
}

ParaSaxParser::~ParaSaxParser()
{
    delete m_handler;
    delete m_reader;
}

ParaSaxParser* ParaSaxParser::_instance = 0;

ParaSaxParser* ParaSaxParser::Instance()
{
    if ( _instance == 0 ){
        _instance = new ParaSaxParser();
    }
    return _instance;
}


void ParaSaxParser::setProcessingInstruction(const QString &data)
{
    m_XmlInputHeader = data;
}


void ParaSaxParser::setData(const QString &data)
{
    m_XmlInputBody = data;
}


void ParaSaxParser::getText(QString &data)
{
    QString XmlPart = m_XmlInputHeader + m_XmlInputBody;
    
    QXmlInputSource input;
    input.setData(XmlPart);

    m_handler->reset();
    m_handler->setRTF(false);
    m_reader->parse( input );
    m_handler->getData(data);

}


void ParaSaxParser::getRTFText(QString &data)
{    
    QString XmlPart = m_XmlInputHeader + m_XmlInputBody;    
    kdDebug(100200) << "ParaSaxParser::getRTFText(): " << XmlPart << endl;
    
    QXmlInputSource input;
    input.setData(XmlPart);

    m_handler->reset();
    m_handler->setRTF(true);
    m_reader->parse( input );
    m_handler->getData(data);
}



