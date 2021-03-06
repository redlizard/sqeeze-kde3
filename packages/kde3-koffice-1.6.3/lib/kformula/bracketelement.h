/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef BRACKETELEMENT_H
#define BRACKETELEMENT_H

#include <qpoint.h>
#include <qsize.h>

#include "basicelement.h"

KFORMULA_NAMESPACE_BEGIN

class Artwork;
class SequenceElement;


/**
 * The base of (all) elements that contain just one content element.
 * (No indexes.)
 */
class SingleContentElement : public BasicElement {
    SingleContentElement& operator=( const SingleContentElement& ) { return *this; }
public:

    SingleContentElement(BasicElement* parent = 0);
    ~SingleContentElement();

    SingleContentElement( const SingleContentElement& );

    /**
     * @returns the character that represents this element. Used for
     * parsing a sequence.
     * This is guaranteed to be QChar::null for all non-text elements.
     */
    virtual QChar getCharacter() const;

    /**
     * Sets the cursor and returns the element the point is in.
     * The handled flag shows whether the cursor has been set.
     * This is needed because only the innermost matching element
     * is allowed to set the cursor.
     */
    virtual BasicElement* goToPos( FormulaCursor*, bool& handled,
                                   const LuPixelPoint& point, const LuPixelPoint& parentOrigin );

    /**
     * Dispatch this FontCommand to all our TextElement children.
     */
    virtual void dispatchFontCommand( FontCommand* cmd );

    /**
     * Enters this element while moving to the left starting inside
     * the element `from'. Searches for a cursor position inside
     * this element or to the left of it.
     */
    virtual void moveLeft(FormulaCursor* cursor, BasicElement* from);

    /**
     * Enters this element while moving to the right starting inside
     * the element `from'. Searches for a cursor position inside
     * this element or to the right of it.
     */
    virtual void moveRight(FormulaCursor* cursor, BasicElement* from);

    /**
     * Enters this element while moving up starting inside
     * the element `from'. Searches for a cursor position inside
     * this element or above it.
     */
    virtual void moveUp(FormulaCursor* cursor, BasicElement* from);

    /**
     * Enters this element while moving down starting inside
     * the element `from'. Searches for a cursor position inside
     * this element or below it.
     */
    virtual void moveDown(FormulaCursor* cursor, BasicElement* from);

    /**
     * Removes all selected children and returns them. Places the
     * cursor to where the children have been.
     *
     * We remove ourselve if we are requested to remove our content.
     */
    virtual void remove(FormulaCursor*, QPtrList<BasicElement>&, Direction);

    /**
     * Moves the cursor to a normal place where new elements
     * might be inserted.
     */
    virtual void normalize(FormulaCursor*, Direction);

    // main child
    //
    // If an element has children one has to become the main one.

    virtual SequenceElement* getMainChild();

    /**
     * Sets the cursor to select the child. The mark is placed before,
     * the position behind it.
     */
    virtual void selectChild(FormulaCursor* cursor, BasicElement* child);

protected:

    /**
     * Appends our attributes to the dom element.
     */
    virtual void writeDom(QDomElement element);

    virtual void writeMathMLContent( QDomDocument& doc, 
                                     QDomElement& element,
                                     bool oasisFormat ) const;
    /**
     * Reads our content from the node. Sets the node to the next node
     * that needs to be read.
     * Returns false if it failed.
     */
    virtual bool readContentFromDom(QDomNode& node);

    /**
     * Reads our content from the MathML node. Sets the node to the next node
     * that needs to be read.
     * Returns false if it failed.
     */
	virtual int readContentFromMathMLDom(QDomNode& node);

    SequenceElement* getContent() { return content; }

private:

    /**
     * The one below the graph.
     */
    SequenceElement* content;
};


/**
 * A left and/or right bracket around one child.
 */
class BracketElement : public SingleContentElement {
    typedef SingleContentElement inherited;
    BracketElement& operator=( const BracketElement& ) { return *this; }
public:

    enum { contentPos };

    BracketElement(SymbolType left = EmptyBracket, SymbolType right = EmptyBracket,
                   BasicElement* parent = 0);
    ~BracketElement();

    BracketElement( const BracketElement& );

    virtual BracketElement* clone() {
        return new BracketElement( *this );
    }

    virtual bool accept( ElementVisitor* visitor );

    /**
     * @returns the type of this element. Used for
     * parsing a sequence.
     */
    virtual TokenType getTokenType() const { return BRACKET; }

    /**
     * The cursor has entered one of our child sequences.
     * This is a good point to tell the user where he is.
     */
    virtual void entered( SequenceElement* child );

    /**
     * Sets the cursor and returns the element the point is in.
     * The handled flag shows whether the cursor has been set.
     * This is needed because only the innermost matching element
     * is allowed to set the cursor.
     */
    virtual BasicElement* goToPos( FormulaCursor*, bool& handled,
                                   const LuPixelPoint& point, const LuPixelPoint& parentOrigin );

    /**
     * Calculates our width and height and
     * our children's parentPosition.
     */
    virtual void calcSizes( const ContextStyle& context,
							ContextStyle::TextStyle tstyle,
							ContextStyle::IndexStyle istyle,
							StyleAttributes& style );

    /**
     * Draws the whole element including its children.
     * The `parentOrigin' is the point this element's parent starts.
     * We can use our parentPosition to get our own origin then.
     */
    virtual void draw( QPainter& painter, const LuPixelRect& r,
                       const ContextStyle& context,
                       ContextStyle::TextStyle tstyle,
                       ContextStyle::IndexStyle istyle,
					   StyleAttributes& style,
					   const LuPixelPoint& parentOrigin );

protected:

    //Save/load support

    /**
     * Returns the tag name of this element type.
     */
    virtual QString getTagName() const { return "BRACKET"; }

    /**
     * Reads our attributes from the element.
     * Returns false if it failed.
     */
    virtual bool readAttributesFromDom(QDomElement element);

    virtual void writeDom(QDomElement element);

    virtual QString getElementName() const { return "mfenced"; }
    virtual void writeMathMLAttributes( QDomElement& element ) const;
    /**
     * Reads our attributes from the MathML element.
     * Returns false if it failed.
     */
	virtual bool readAttributesFromMathMLDom(const QDomElement& element);

    /**
     * Reads our content from the MathML node. Sets the node to the next node
     * that needs to be read. It is sometimes needed to read more than one node
     * (e. g. for fence operators).
     * Returns the number of nodes processed or -1 if it failed.
     */
	virtual int readContentFromMathMLDom(QDomNode& node);

    /**
     * @returns the latex representation of the element and
     * of the element's children
     */
    virtual QString toLatex();

    virtual QString formulaString();

private:

    /**
     * @return a LaTex string for the given symbol
     */
    QString latexString(char);
    
    /**
     * Set left and right types in operator fences
     * @param open if true set SymbolType for open (left) bracket, 
     * otherwise set for close (right) bracket.
     */
    int operatorType( QDomNode& node, bool open );

    /**
     * Search through the nodes to find the close operator to match current
     * open bracket.
     */
    int searchOperator( const QDomNode& node );

    /**
     * The brackets we are showing.
     */
    Artwork* left;
    Artwork* right;

    SymbolType leftType;
    SymbolType rightType;

    QString m_separators;
    bool m_operator;
    bool m_customLeft;
    bool m_customRight;
};


/**
 * A line above the content.
 */
class OverlineElement : public SingleContentElement {
    OverlineElement& operator=( const OverlineElement& ) { return *this; }
public:

    OverlineElement(BasicElement* parent = 0);
    ~OverlineElement();

    OverlineElement( const OverlineElement& );

    virtual OverlineElement* clone() {
        return new OverlineElement( *this );
    }

    virtual bool accept( ElementVisitor* visitor );

    /**
     * The cursor has entered one of our child sequences.
     * This is a good point to tell the user where he is.
     */
    virtual void entered( SequenceElement* child );

    /**
     * Calculates our width and height and
     * our children's parentPosition.
     */
    virtual void calcSizes( const ContextStyle& context,
						    ContextStyle::TextStyle tstyle,
						    ContextStyle::IndexStyle istyle,
							StyleAttributes& style );

    /**
     * Draws the whole element including its children.
     * The `parentOrigin' is the point this element's parent starts.
     * We can use our parentPosition to get our own origin then.
     */
    virtual void draw( QPainter& painter, const LuPixelRect& r,
                       const ContextStyle& context,
                       ContextStyle::TextStyle tstyle,
                       ContextStyle::IndexStyle istyle,
					   StyleAttributes& style,
                       const LuPixelPoint& parentOrigin );

    /**
     * @returns the latex representation of the element and
     * of the element's children
     */
    virtual QString toLatex();

    virtual QString formulaString();

    virtual void writeMathML( QDomDocument& doc, QDomNode& parent, bool oasisFormat = false ) const;

protected:

    //Save/load support

    /**
     * Returns the tag name of this element type.
     */
    virtual QString getTagName() const { return "OVERLINE"; }

private:
};


/**
 * A line below the content.
 */
class UnderlineElement : public SingleContentElement {
    UnderlineElement& operator=( const UnderlineElement& ) { return *this; }
public:
    UnderlineElement(BasicElement* parent = 0);
    ~UnderlineElement();

    UnderlineElement( const UnderlineElement& );

    virtual UnderlineElement* clone() {
        return new UnderlineElement( *this );
    }

    virtual bool accept( ElementVisitor* visitor );

    /**
     * The cursor has entered one of our child sequences.
     * This is a good point to tell the user where he is.
     */
    virtual void entered( SequenceElement* child );

    /**
     * Calculates our width and height and
     * our children's parentPosition.
     */
    virtual void calcSizes( const ContextStyle& context,
						    ContextStyle::TextStyle tstyle,
						    ContextStyle::IndexStyle istyle,
							StyleAttributes& style );

    /**
     * Draws the whole element including its children.
     * The `parentOrigin' is the point this element's parent starts.
     * We can use our parentPosition to get our own origin then.
     */
    virtual void draw( QPainter& painter, const LuPixelRect& r,
                       const ContextStyle& context,
                       ContextStyle::TextStyle tstyle,
                       ContextStyle::IndexStyle istyle,
					   StyleAttributes& style,
                       const LuPixelPoint& parentOrigin );

    /**
     * @returns the latex representation of the element and
     * of the element's children
     */
    virtual QString toLatex();

    virtual QString formulaString();

    virtual void writeMathML( QDomDocument& doc, QDomNode& parent, bool oasisFormat = false ) const;

protected:

    //Save/load support

    /**
     * Returns the tag name of this element type.
     */
    virtual QString getTagName() const { return "UNDERLINE"; }

private:
};


KFORMULA_NAMESPACE_END

#endif // BRACKETELEMENT_H
