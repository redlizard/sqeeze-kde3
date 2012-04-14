//
// C++ Interface: parasaxparser
//
// Description: SAX2-Parser for 'para' elements of a DocBook file.
//
//
// Author: Robert Vogl <voglrobe@web.de>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PARASAXPARSER_H
#define PARASAXPARSER_H

// Qt includes
#include <qxml.h>
#include <qstring.h>

// App specific includes
#include "saxhandler.h"

/**
This class is a singleton.
@author Robert Vogl
*/
class ParaSaxParser{
public:
   ~ParaSaxParser();

    /**
     * Creates the single instance of this parser (if not
     * allready exists) and returns a pointer to it.
     */
    static ParaSaxParser* Instance();
    
    /**
     * Because this parser sees nothing but the 'para'
     * parts of the document, it requires the documents header
     * info, e.g. to resolve entities defined in the internal DTD.
     * \param data The header (preamble) of the DocBook document. 
     */
    void setProcessingInstruction(const QString &data);
    
    /**
     * This is the 'para' element to parse. It has to be encapsulated
     * in <para> and </para>. Nested 'para' elements are not supported.
     * \param data See description.
     */
    void setData(const QString &data);
    
    /**
     * Returns the pure text content of the paragraph without tags.
     * \param data See description.
     */
    void getText(QString &data);
    
    /**
     * Returns the text in RTF format, i.e. replaces DocBook tags
     * with RTF tags.
     * \param data See description. 
     */
    void getRTFText(QString &data);

protected:
    ParaSaxParser();

private:
    static ParaSaxParser* _instance;
    SaxHandler* m_handler;
    QXmlSimpleReader* m_reader;
    QString m_XmlInputHeader;
    QString m_XmlInputBody;


};

#endif
