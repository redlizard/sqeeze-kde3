//
// C++ Interface: saxhandler
//
// Description: Qt SAX2-Handler
//
//
// Author: Robert Vogl <voglrobe@web.de>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SAXHANDLER_H
#define SAXHANDLER_H

// STL includes
#include <map>
using namespace std;

// Qt includes
#include <qxml.h>
#include <qstring.h>

typedef map<QString, QString> TagMapT;


/**
@author Robert Vogl
*/
class SaxHandler : public QXmlDefaultHandler
{
public:
    SaxHandler();
    ~SaxHandler();

    // Reimplementations from base class
    bool startElement(const QString &namespaceURI,
                    const QString &localName,
                    const QString &qName,
                    const QXmlAttributes &atts );

    bool endElement(const QString &namespaceURI,
                    const QString &localName,
                    const QString &qName);

    bool characters(const QString &ch);

    bool fatalError(const QXmlParseException &exc);

    bool resolveEntity(const QString &publicId,
                    const QString &systemId,
                    QXmlInputSource* &ret);

    bool externalEntityDecl(const QString &name,
                    const QString &publicId,
                    const QString &systemId);

    bool internalEntityDecl(const QString &name,
                    const QString &value);

    bool skippedEntity(const QString &name);

    /**
     * Returns the parser result.
     */
    void getData( QString &data ) const;

    /**
     * Reset
     */
    void reset();

    /**
     * Set Mode:
     * \param rtf 'true' = Replace DocBook tags with RTF-tags.\n
     * 'false' = Ignore tags, return text content only. 
     */
    void setRTF(bool rtf);


private:
    QString m_output;
    bool m_rtf;
    TagMapT m_tagmap;

};

#endif
