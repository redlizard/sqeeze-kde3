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

#ifndef FRACTIONELEMENT_H
#define FRACTIONELEMENT_H

#include "basicelement.h"

KFORMULA_NAMESPACE_BEGIN
class SequenceElement;


/**
 * A fraction.
 */
class FractionElement : public BasicElement {
    FractionElement& operator=( const FractionElement& ) { return *this; }
public:

    enum { numeratorPos, denominatorPos };

    FractionElement(BasicElement* parent = 0);
    ~FractionElement();

    FractionElement( const FractionElement& );

    virtual FractionElement* clone() {
        return new FractionElement( *this );
    }

    virtual bool accept( ElementVisitor* visitor );

    /**
     * @returns the type of this element. Used for
     * parsing a sequence.
     */
    virtual TokenType getTokenType() const { return INNER; }

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
    virtual void calcSizes( const ContextStyle& cstyle,
						    ContextStyle::TextStyle tstyle,
						    ContextStyle::IndexStyle istyle,
							StyleAttributes& style );

    /**
     * Draws the whole element including its children.
     * The `parentOrigin' is the point this element's parent starts.
     * We can use our parentPosition to get our own origin then.
     */
    virtual void draw( QPainter& painter, const LuPixelRect& r,
                       const ContextStyle& cstyle,
                       ContextStyle::TextStyle tstyle,
                       ContextStyle::IndexStyle istyle,
					   StyleAttributes& style,
                       const LuPixelPoint& parentOrigin );

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
     * Reinserts the denominator if it has been removed.
     */
    virtual void insert(FormulaCursor*, QPtrList<BasicElement>&, Direction);

    /**
     * Removes all selected children and returns them. Places the
     * cursor to where the children have been.
     *
     * We remove ourselve if we are requested to remove our numerator.
     *
     * It is possible to remove the denominator. But after this we
     * are senseless and the caller is required to replace us.
     */
    virtual void remove(FormulaCursor*, QPtrList<BasicElement>&, Direction);


    // main child
    //
    // If an element has children one has to become the main one.

    virtual SequenceElement* getMainChild();

    SequenceElement* getNumerator() { return numerator; }
    SequenceElement* getDenominator() { return denominator; }

    /**
     * Returns wether the element has no more useful
     * children (except its main child) and should therefore
     * be replaced by its main child's content.
     */
    virtual bool isSenseless();

    /**
     * Sets the cursor to select the child. The mark is placed before,
     * the position behind it.
     */
    virtual void selectChild(FormulaCursor* cursor, BasicElement* child);

    /**
     * Moves the cursor away from the given child. The cursor is
     * guaranteed to be inside this element.
     */
    //virtual void childWillVanish(FormulaCursor* cursor, BasicElement* child) = 0;

    /**
     * Tells whether the fraction should be drawn with a line.
     */
    void showLine(bool line) { 
        m_lineThicknessType = RelativeSize;
        if ( line ) {
            m_lineThickness = 1.0;
        }
        else {
            m_lineThickness = 0.0;
        }
    }

    /**
     * @returns the latex representation of the element and
     * of the element's children
     */
    virtual QString toLatex();

    virtual QString formulaString();

protected:

    //Save/load support

    /**
     * Returns the tag name of this element type.
     */
    virtual QString getTagName() const { return "FRACTION"; }

    /**
     * Appends our attributes to the dom element.
     */
    virtual void writeDom(QDomElement element);

    /**
     * Reads our attributes from the element.
     * Returns false if it failed.
     */
    virtual bool readAttributesFromDom(QDomElement element);

    /**
     * Reads our content from the node. Sets the node to the next node
     * that needs to be read.
     * Returns false if it failed.
     */
    virtual bool readContentFromDom(QDomNode& node);

    /**
     * Reads our attributes from the MathML element.
     * Returns false if it failed.
     */
	virtual bool readAttributesFromMathMLDom(const QDomElement& element);

    /**
     * Reads our content from the MathML node. Sets the node to the next node
     * that needs to be read.
     * Returns false if it failed.
     */
	virtual int readContentFromMathMLDom(QDomNode& node);

private:
    virtual QString getElementName() const { return "mfrac"; }
    virtual void writeMathMLAttributes( QDomElement& element ) const ;
    virtual void writeMathMLContent( QDomDocument& doc, 
                                     QDomElement& element,
                                     bool oasisFormat ) const ;

    double lineThickness( const ContextStyle& context, double factor );

    bool withLine() { return m_lineThicknessType == NoSize || m_lineThickness != 0.0; }

    SequenceElement* numerator;
    SequenceElement* denominator;

    SizeType m_lineThicknessType;
    double m_lineThickness;
    HorizontalAlign m_numAlign;
    HorizontalAlign m_denomAlign;
    bool m_customBevelled;
    bool m_bevelled;
};

KFORMULA_NAMESPACE_END

#endif // FRACTIONELEMENT_H
