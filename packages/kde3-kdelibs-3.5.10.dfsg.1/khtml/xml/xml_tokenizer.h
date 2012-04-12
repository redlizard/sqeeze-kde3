/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 2000 Peter Kelly (pmk@post.com)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef _XML_Tokenizer_h_
#define _XML_Tokenizer_h_

#include <qxml.h>
#include <qptrlist.h>
#include <qptrstack.h>
#include <qvaluestack.h>
#include <qobject.h>
#include "misc/loader_client.h"
#include "misc/stringit.h"

class KHTMLView;

namespace khtml {
    class CachedObject;
    class CachedScript;
}

namespace DOM {
    class DocumentImpl;
    class NodeImpl;
    class HTMLScriptElementImpl;
    class DocumentImpl;
    class HTMLScriptElementImpl;
}

namespace khtml {

class XMLHandler : public QXmlDefaultHandler
{
public:
    XMLHandler(DOM::DocumentImpl *_doc, KHTMLView *_view);
    virtual ~XMLHandler();

    // return the error protocol if parsing failed
    QString errorProtocol();

    // overloaded handler functions
    bool startDocument();
    bool startElement(const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& atts);
    bool endElement(const QString& namespaceURI, const QString& localName, const QString& qName);
    bool startCDATA();
    bool endCDATA();
    bool characters(const QString& ch);
    bool comment(const QString & ch);
    bool processingInstruction(const QString &target, const QString &data);
    
    // namespace handling, to workaround problem in QXML where some attributes
    // do not get the namespace resolved properly
    bool startPrefixMapping(const QString& prefix, const QString& uri);
    bool endPrefixMapping(const QString& prefix);
    void fixUpNSURI(QString& uri, const QString& qname);
    QMap<QString, QValueStack<QString> > namespaceInfo;


    // from QXmlDeclHandler
    bool attributeDecl(const QString &eName, const QString &aName, const QString &type, const QString &valueDefault, const QString &value);
    bool externalEntityDecl(const QString &name, const QString &publicId, const QString &systemId);
    bool internalEntityDecl(const QString &name, const QString &value);

    // from QXmlDTDHandler
    bool notationDecl(const QString &name, const QString &publicId, const QString &systemId);
    bool unparsedEntityDecl(const QString &name, const QString &publicId, const QString &systemId, const QString &notationName);

    bool enterText();
    void exitText();

    QString errorString();

    bool fatalError( const QXmlParseException& exception );

    unsigned long errorLine;
    unsigned long errorCol;

private:
    void pushNode( DOM::NodeImpl *node );
    DOM::NodeImpl *popNode();
    DOM::NodeImpl *currentNode() const;
private:
    QString errorProt;
    DOM::DocumentImpl *m_doc;
    KHTMLView *m_view;
    QPtrStack<DOM::NodeImpl> m_nodes;
    DOM::NodeImpl *m_rootNode;

    enum State {
        StateInit,
        StateDocument,
        StateQuote,
        StateLine,
        StateHeading,
        StateP
    };
    State state;
};

class Tokenizer : public QObject
{
    Q_OBJECT
public:
    virtual void begin() = 0;
    // script output must be prepended, while new data
    // received during executing a script must be appended, hence the
    // extra bool to be able to distinguish between both cases. document.write()
    // always uses false, while khtmlpart uses true
    virtual void write( const TokenizerString &str, bool appendData) = 0;
    virtual void end() = 0;
    virtual void finish() = 0;
    virtual void setOnHold(bool /*_onHold*/) {}
    virtual bool isWaitingForScripts() const = 0;
    virtual bool isExecutingScript() const = 0;
    virtual void abort() {}
    virtual void setAutoClose(bool b=true) = 0;

signals:
    void finishedParsing();

};

class XMLIncrementalSource : public QXmlInputSource
{
public:
    XMLIncrementalSource();
    virtual void fetchData();
    virtual QChar next();
    virtual void setData( const QString& str );
    virtual void setData( const QByteArray& data );
    virtual QString data();

    void appendXML( const QString& str );
    void setFinished( bool );

private:
    QString      m_data;
    uint         m_pos;
    const QChar *m_unicode;
    bool         m_finished;
};

class XMLTokenizer : public Tokenizer, public khtml::CachedObjectClient
{
public:
    XMLTokenizer(DOM::DocumentImpl *, KHTMLView * = 0);
    virtual ~XMLTokenizer();
    virtual void begin();
    virtual void write( const TokenizerString &str, bool );
    virtual void end();
    virtual void finish();
    virtual void setAutoClose(bool b=true) { qWarning("XMLTokenizer::setAutoClose: stub."); (void)b; }

    // from CachedObjectClient
    void notifyFinished(khtml::CachedObject *finishedObj);

    virtual bool isWaitingForScripts() const;
    virtual bool isExecutingScript() const { return false; }

protected:
    DOM::DocumentImpl *m_doc;
    KHTMLView *m_view;

    void executeScripts();
    void addScripts(DOM::NodeImpl *n);

    QPtrList<DOM::HTMLScriptElementImpl> m_scripts;
    QPtrListIterator<DOM::HTMLScriptElementImpl> *m_scriptsIt;
    khtml::CachedScript *m_cachedScript;

    XMLHandler m_handler;
    QXmlSimpleReader m_reader;
    XMLIncrementalSource m_source;
    bool m_noErrors;
};

} // end namespace

#endif
