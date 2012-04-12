/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 *           (C) 2004 Apple Computer, Inc.
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

//#define CSS_STYLESHEET_DEBUG

#include "dom/dom_string.h"
#include "dom/dom_exception.h"
#include "dom/css_stylesheet.h"
#include "dom/css_rule.h"

#include "css/css_ruleimpl.h"
#include "css/css_valueimpl.h"
#include "css/cssparser.h"
#include "css/css_stylesheetimpl.h"

#include "xml/dom_nodeimpl.h"
#include "html/html_documentimpl.h"
#include "misc/loader.h"

#include <kdebug.h>

using namespace DOM;
using namespace khtml;
// --------------------------------------------------------------------------------

StyleSheetImpl::StyleSheetImpl(StyleSheetImpl *parentSheet, DOMString href)
    : StyleListImpl(parentSheet)
{
    m_disabled = false;
    m_media = 0;
    m_parentNode = 0;
    m_strHref = href;
}


StyleSheetImpl::StyleSheetImpl(DOM::NodeImpl *parentNode, DOMString href)
    : StyleListImpl()
{
    m_parentNode = parentNode;
    m_disabled = false;
    m_media = 0;
    m_strHref = href;
}

StyleSheetImpl::StyleSheetImpl(StyleBaseImpl *owner, DOMString href)
    : StyleListImpl(owner)
{
    m_disabled = false;
    m_media = 0;
    m_parentNode = 0;
    m_strHref = href;
}

StyleSheetImpl::~StyleSheetImpl()
{
    if(m_media) {
	m_media->setParent( 0 );
	m_media->deref();
    }
}

StyleSheetImpl *StyleSheetImpl::parentStyleSheet() const
{
    if( !m_parent ) return 0;
    if( m_parent->isStyleSheet() ) return static_cast<StyleSheetImpl *>(m_parent);
    if( m_parent->isRule() ) return m_parent->stylesheet();
    return 0;
}

void StyleSheetImpl::setMedia( MediaListImpl *media )
{
    if( media )
	media->ref();
    if( m_media )
	m_media->deref();
    m_media = media;
}

void StyleSheetImpl::setDisabled( bool disabled )
{
    bool updateStyle = isCSSStyleSheet() && m_parentNode && disabled != m_disabled;
    m_disabled = disabled;
    if (updateStyle)
        m_parentNode->getDocument()->updateStyleSelector();
}

// -----------------------------------------------------------------------


CSSStyleSheetImpl::CSSStyleSheetImpl(CSSStyleSheetImpl *parentSheet, DOMString href)
    : StyleSheetImpl(parentSheet, href)
{
    m_lstChildren = new QPtrList<StyleBaseImpl>;
    m_doc = parentSheet ? parentSheet->doc() : 0;
    m_implicit = false;
    m_namespaces = 0;
    m_defaultNamespace = anyNamespace;
}

CSSStyleSheetImpl::CSSStyleSheetImpl(DOM::NodeImpl *parentNode, DOMString href, bool _implicit)
    : StyleSheetImpl(parentNode, href)
{
    m_lstChildren = new QPtrList<StyleBaseImpl>;
    m_doc = parentNode->getDocument();
    m_implicit = _implicit;
    m_namespaces = 0;
    m_defaultNamespace = anyNamespace;
}

CSSStyleSheetImpl::CSSStyleSheetImpl(CSSRuleImpl *ownerRule, DOMString href)
    : StyleSheetImpl(ownerRule, href)
{
    m_lstChildren = new QPtrList<StyleBaseImpl>;
    m_doc = static_cast<CSSStyleSheetImpl*>(ownerRule->stylesheet())->doc();
    m_implicit = false;
    m_namespaces = 0;
    m_defaultNamespace = anyNamespace;
}

CSSStyleSheetImpl::CSSStyleSheetImpl(DOM::NodeImpl *parentNode, CSSStyleSheetImpl *orig)
    : StyleSheetImpl(parentNode, orig->m_strHref)
{
    m_lstChildren = new QPtrList<StyleBaseImpl>;
    StyleBaseImpl *rule;
    for ( rule = orig->m_lstChildren->first(); rule != 0; rule = orig->m_lstChildren->next() )
    {
        m_lstChildren->append(rule);
        rule->setParent(this);
    }
    m_doc = parentNode->getDocument();
    m_implicit = false;
    m_namespaces = 0;
    m_defaultNamespace = anyNamespace;
}

CSSStyleSheetImpl::CSSStyleSheetImpl(CSSRuleImpl *ownerRule, CSSStyleSheetImpl *orig)
    : StyleSheetImpl(ownerRule, orig->m_strHref)
{
    // m_lstChildren is deleted in StyleListImpl
    m_lstChildren = new QPtrList<StyleBaseImpl>;
    StyleBaseImpl *rule;
    for ( rule = orig->m_lstChildren->first(); rule != 0; rule = orig->m_lstChildren->next() )
    {
        m_lstChildren->append(rule);
        rule->setParent(this);
    }
    m_doc = static_cast<CSSStyleSheetImpl*>(ownerRule->stylesheet())->doc();
    m_implicit = false;
    m_namespaces = 0;
    m_defaultNamespace = anyNamespace;
}

CSSRuleImpl *CSSStyleSheetImpl::ownerRule() const
{
    if( !m_parent ) return 0;
    if( m_parent->isRule() ) return static_cast<CSSRuleImpl *>(m_parent);
    return 0;
}

unsigned long CSSStyleSheetImpl::insertRule( const DOMString &rule, unsigned long index, int &exceptioncode )
{
    exceptioncode = 0;
    if(index > m_lstChildren->count()) {
        exceptioncode = DOMException::INDEX_SIZE_ERR;
        return 0;
    }
    CSSParser p( strictParsing );
    CSSRuleImpl *r = p.parseRule( this, rule );

    if(!r) {
        exceptioncode = CSSException::SYNTAX_ERR + CSSException::_EXCEPTION_OFFSET;
        return 0;
    }
    // ###
    // HIERARCHY_REQUEST_ERR: Raised if the rule cannot be inserted at the specified index e.g. if an
    //@import rule is inserted after a standard rule set or other at-rule.
    m_lstChildren->insert(index, r);
    if (m_doc)
        m_doc->updateStyleSelector(true /*shallow*/);
    return index;
}

CSSRuleList CSSStyleSheetImpl::cssRules()
{
    return this;
}

void CSSStyleSheetImpl::deleteRule( unsigned long index, int &exceptioncode )
{
    exceptioncode = 0;
    StyleBaseImpl *b = m_lstChildren->take(index);
    if(!b) {
        exceptioncode = DOMException::INDEX_SIZE_ERR;
        return;
    }
    // TreeShared requires delete not deref when removed from tree
    b->setParent(0);
    if( !b->refCount() ) delete b;
    if (m_doc)
        m_doc->updateStyleSelector(true /*shallow*/);
}

void CSSStyleSheetImpl::addNamespace(CSSParser* p, const DOM::DOMString& prefix, const DOM::DOMString& uri)
{
    int exceptioncode = 0;
    if (uri.isEmpty())
        return;

    m_namespaces = new CSSNamespace(prefix, uri, m_namespaces);

    if (prefix.isEmpty()) {
        Q_ASSERT(m_doc != 0);

        m_defaultNamespace = m_doc->getId(NodeImpl::NamespaceId, uri.implementation(), false, false, &exceptioncode);
    }
}

void CSSStyleSheetImpl::determineNamespace(Q_UINT32& id, const DOM::DOMString& prefix)
{
    // If the stylesheet has no namespaces we can just return.  There won't be any need to ever check
    // namespace values in selectors.
    if (!m_namespaces)
        return;

    if (prefix.isEmpty())
         id = makeId(emptyNamespace, localNamePart(id)); // No namespace. If an element/attribute has a namespace, we won't match it.
    else if (prefix == "*")
        id = makeId(anyNamespace, localNamePart(id)); // We'll match any namespace.
    else {
        int exceptioncode = 0;
        CSSNamespace* ns = m_namespaces->namespaceForPrefix(prefix);
        if (ns) {
            Q_ASSERT(m_doc != 0);

            // Look up the id for this namespace URI.
            Q_UINT16 nsid = m_doc->getId(NodeImpl::NamespaceId, 0, 0, ns->uri().implementation(), false, false, &exceptioncode);
            id = makeId(nsid, localNamePart(id));
        }
    }
}

bool CSSStyleSheetImpl::parseString(const DOMString &string, bool strict)
{
#ifdef CSS_STYLESHEET_DEBUG
    kdDebug( 6080 ) << "parsing sheet, len=" << string.length() << ", sheet is " << string.string() << endl;
#endif

    strictParsing = strict;
    CSSParser p( strict );
    p.parseSheet( this, string );
    return true;
}

bool CSSStyleSheetImpl::isLoading() const
{
    StyleBaseImpl *rule;
    for ( rule = m_lstChildren->first(); rule != 0; rule = m_lstChildren->next() )
    {
        if(rule->isImportRule())
        {
            CSSImportRuleImpl *import = static_cast<CSSImportRuleImpl *>(rule);
#ifdef CSS_STYLESHEET_DEBUG
            kdDebug( 6080 ) << "found import" << endl;
#endif
            if(import->isLoading())
            {
#ifdef CSS_STYLESHEET_DEBUG
                kdDebug( 6080 ) << "--> not loaded" << endl;
#endif
                return true;
            }
        }
    }
    return false;
}

void CSSStyleSheetImpl::checkLoaded() const
{
    if(isLoading()) return;
    if(m_parent) m_parent->checkLoaded();
    if(m_parentNode) m_parentNode->sheetLoaded();
}

void CSSStyleSheetImpl::setNonCSSHints()
{
    StyleBaseImpl *rule = m_lstChildren->first();
    while(rule) {
        if(rule->isStyleRule()) {
            static_cast<CSSStyleRuleImpl *>(rule)->setNonCSSHints();
        }
        rule = m_lstChildren->next();
    }
}


// ---------------------------------------------------------------------------


StyleSheetListImpl::~StyleSheetListImpl()
{
    for ( QPtrListIterator<StyleSheetImpl> it ( styleSheets ); it.current(); ++it )
        it.current()->deref();
}

void StyleSheetListImpl::add( StyleSheetImpl* s )
{
    if ( !styleSheets.containsRef( s ) ) {
        s->ref();
        styleSheets.append( s );
    }
}

void StyleSheetListImpl::remove( StyleSheetImpl* s )
{
    if ( styleSheets.removeRef( s ) )
        s->deref();
}

unsigned long StyleSheetListImpl::length() const
{
    // hack so implicit BODY stylesheets don't get counted here
    unsigned long l = 0;
    QPtrListIterator<StyleSheetImpl> it(styleSheets);
    for (; it.current(); ++it) {
        if (!it.current()->isCSSStyleSheet() || !static_cast<CSSStyleSheetImpl*>(it.current())->implicit())
            ++l;
    }
    return l;
}

StyleSheetImpl *StyleSheetListImpl::item ( unsigned long index )
{
    unsigned long l = 0;
    QPtrListIterator<StyleSheetImpl> it(styleSheets);
    for (; it.current(); ++it) {
        if (!it.current()->isCSSStyleSheet() || !static_cast<CSSStyleSheetImpl*>(it.current())->implicit()) {
            if (l == index)
                return it.current();
            ++l;
        }
    }
    return 0;
}

// --------------------------------------------------------------------------------------------

MediaListImpl::MediaListImpl( CSSStyleSheetImpl *parentSheet,
                              const DOMString &media )
    : StyleBaseImpl( parentSheet )
{
    setMediaText( media );
}

MediaListImpl::MediaListImpl( CSSRuleImpl *parentRule, const DOMString &media )
    : StyleBaseImpl(parentRule)
{
    setMediaText( media );
}

bool MediaListImpl::contains( const DOMString &medium ) const
{
    return m_lstMedia.empty() || m_lstMedia.contains( medium ) ||
            m_lstMedia.contains( "all" );
}

CSSStyleSheetImpl *MediaListImpl::parentStyleSheet() const
{
    if( m_parent->isCSSStyleSheet() ) return static_cast<CSSStyleSheetImpl *>(m_parent);
    return 0;
}

CSSRuleImpl *MediaListImpl::parentRule() const
{
    if( m_parent->isRule() ) return static_cast<CSSRuleImpl *>(m_parent);
    return 0;
}

void MediaListImpl::deleteMedium( const DOMString &oldMedium )
{
    const QValueList<DOMString>::Iterator itEnd = m_lstMedia.end();

    for ( QValueList<DOMString>::Iterator it = m_lstMedia.begin(); it != itEnd; ++it ) {
        if( (*it) == oldMedium ) {
            m_lstMedia.remove( it );
            return;
        }
    }
}

DOM::DOMString MediaListImpl::mediaText() const
{
    DOMString text;
    const QValueList<DOMString>::ConstIterator itEnd = m_lstMedia.end();

    for ( QValueList<DOMString>::ConstIterator it = m_lstMedia.begin(); it != itEnd; ++it ) {
        text += *it;
        text += ", ";
    }
    return text;
}

void MediaListImpl::setMediaText(const DOM::DOMString &value)
{
    m_lstMedia.clear();
    const QString val = value.string();
    const QStringList list = QStringList::split( ',', val );

    const QStringList::ConstIterator itEnd = list.end();

    for ( QStringList::ConstIterator it = list.begin(); it != itEnd; ++it )
    {
        const DOMString medium = (*it).stripWhiteSpace();
        if( !medium.isEmpty() )
            m_lstMedia.append( medium );
    }
}
