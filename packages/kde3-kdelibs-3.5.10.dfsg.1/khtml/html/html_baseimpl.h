/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 *           (C) 2000-2003 Dirk Mueller (mueller@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Simon Hausmann <hausmann@kde.org>
 * Copyright (C) 2002 Apple Computer, Inc.
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

#ifndef HTML_BASEIMPL_H
#define HTML_BASEIMPL_H

#include "html/dtd.h"
#include "html/html_elementimpl.h"
#include "misc/khtmllayout.h"

#include <qscrollview.h>

class KHTMLView;
class KHTMLPart;

namespace khtml {
    class RenderFrameSet;
    class RenderFrame;
    class RenderPartObject;
}

namespace DOM {

class DOMString;
class CSSStyleSheetImpl;
class HTMLFrameElement;

// -------------------------------------------------------------------------

class HTMLBodyElementImpl : public HTMLElementImpl
{
public:
    HTMLBodyElementImpl(DocumentImpl *doc);
    ~HTMLBodyElementImpl();

    virtual Id id() const;

    virtual void parseAttribute(AttributeImpl *);
    virtual void attach();

    virtual void insertedIntoDocument();
    virtual void removedFromDocument();

    CSSStyleSheetImpl *sheet() const { return m_styleSheet; }

protected:
    CSSStyleSheetImpl *m_styleSheet;
    bool m_bgSet;
    bool m_fgSet;
};

// -------------------------------------------------------------------------

class HTMLFrameElementImpl : public HTMLElementImpl
{
    friend class khtml::RenderFrame;
    friend class khtml::RenderPartObject;

public:
    HTMLFrameElementImpl(DocumentImpl *doc);

    ~HTMLFrameElementImpl();

    virtual Id id() const;

    virtual void parseAttribute(AttributeImpl *);
    virtual void attach();

    bool noResize() { return noresize; }
    void setLocation( const DOMString& str );

    virtual bool isFocusable() const;
    virtual void setFocus(bool);

    DocumentImpl* contentDocument() const;
    KHTMLPart*    contentPart() const;

    DOMString url;
    DOMString name;

    int marginWidth;
    int marginHeight;
    QScrollView::ScrollBarMode scrolling;

    bool frameBorder : 1;
    bool frameBorderSet : 1;
    bool noresize : 1;
};

// -------------------------------------------------------------------------

class HTMLFrameSetElementImpl : public HTMLElementImpl
{
    friend class khtml::RenderFrameSet;
public:
    HTMLFrameSetElementImpl(DocumentImpl *doc);

    ~HTMLFrameSetElementImpl();

    virtual Id id() const;

    virtual void parseAttribute(AttributeImpl *);
    virtual void attach();

    virtual void defaultEventHandler(EventImpl *evt);

    bool frameBorder() { return frameborder; }
    bool noResize() { return noresize; }

    int totalRows() const { return m_totalRows; }
    int totalCols() const { return m_totalCols; }
    int border() const { return m_border; }
    virtual void detach();

    virtual void recalcStyle( StyleChange ch );

protected:
    khtml::Length* m_rows;
    khtml::Length* m_cols;

    int m_totalRows;
    int m_totalCols;
    int m_border;

    bool frameborder : 1;
    bool frameBorderSet : 1;
    bool noresize : 1;
    bool m_resizing : 1;  // is the user resizing currently
    
    EventListener* m_onLoad;
    EventListener* m_onUnLoad;
};

// -------------------------------------------------------------------------

class HTMLHeadElementImpl : public HTMLElementImpl
{
public:
    HTMLHeadElementImpl(DocumentImpl *doc)
        : HTMLElementImpl(doc) {}

    virtual Id id() const;
};

// -------------------------------------------------------------------------

class HTMLHtmlElementImpl : public HTMLElementImpl
{
public:
    HTMLHtmlElementImpl(DocumentImpl *doc)
        : HTMLElementImpl(doc) {}

    virtual Id id() const;
};


// -------------------------------------------------------------------------

class HTMLIFrameElementImpl : public HTMLFrameElementImpl
{
public:
    HTMLIFrameElementImpl(DocumentImpl *doc);

    ~HTMLIFrameElementImpl();

    virtual Id id() const;

    virtual void parseAttribute(AttributeImpl *attr);
    virtual void attach();
    virtual void recalcStyle( StyleChange ch );

protected:

    void updateFrame();

    bool needWidgetUpdate;
    bool m_frame;
};


} //namespace

#endif

