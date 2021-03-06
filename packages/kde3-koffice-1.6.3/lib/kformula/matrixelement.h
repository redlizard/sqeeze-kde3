/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
   Copyright (C) 2006 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>

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

#ifndef MATRIXELEMENT_H
#define MATRIXELEMENT_H

#include <qptrlist.h>

#include "basicelement.h"

KFORMULA_NAMESPACE_BEGIN


class MatrixSequenceElement;


/**
 * A matrix.
 */
class MatrixElement : public BasicElement {
    friend class KFCRemoveColumn;
    friend class KFCRemoveRow;
    friend class MatrixSequenceElement;

    enum VerticalAlign { NoAlign, TopAlign, BottomAlign, CenterAlign, BaselineAlign, AxisAlign };
    enum LineType { NoLine, NoneLine, SolidLine, DashedLine };
    enum SideType { NoSide, LeftSide, RightSide, LeftOverlapSide, RightOverlapSide };
    MatrixElement& operator=( const MatrixElement& ) { return *this; }
public:
    MatrixElement(uint rows = 1, uint columns = 1, BasicElement* parent = 0);
    ~MatrixElement();

    MatrixElement( const MatrixElement& );

    virtual MatrixElement* clone() {
        return new MatrixElement( *this );
    }

    virtual bool accept( ElementVisitor* visitor );

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

    // drawing
    //
    // Drawing depends on a context which knows the required properties like
    // fonts, spaces and such.
    // It is essential to calculate elements size with the same context
    // before you draw.

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
                       const ContextStyle& context,
                       ContextStyle::TextStyle tstyle,
                       ContextStyle::IndexStyle istyle,
					   StyleAttributes& style,
                       const LuPixelPoint& parentOrigin );

    /**
     * Dispatch this FontCommand to all our TextElement children.
     */
    virtual void dispatchFontCommand( FontCommand* cmd );

    // navigation
    //
    // The elements are responsible to handle cursor movement themselves.
    // To do this they need to know the direction the cursor moves and
    // the element it comes from.
    //
    // The cursor might be in normal or in selection mode.

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
    virtual void moveUp(FormulaCursor*, BasicElement*);

    /**
     * Enters this element while moving down starting inside
     * the element `from'. Searches for a cursor position inside
     * this element or below it.
     */
    virtual void moveDown(FormulaCursor*, BasicElement*);

    /**
     * Sets the cursor inside this element to its start position.
     * For most elements that is the main child.
     */
    virtual void goInside(FormulaCursor* cursor);

    /**
     * We define the Main Child of a matrix to be the first row/column.
     **/
    // If there is a main child we must provide the insert/remove semantics.
    virtual SequenceElement* getMainChild();

    /**
     * Inserts all new children at the cursor position. Places the
     * cursor according to the direction.
     */
    //virtual void insert(FormulaCursor*, QPtrList<BasicElement>&, Direction);

    /**
     * Removes all selected children and returns them. Places the
     * cursor to where the children have been.
     */
    //virtual void remove(FormulaCursor*, QPtrList<BasicElement>&, Direction);

    /**
     * Moves the cursor to a normal place where new elements
     * might be inserted.
     */
    //virtual void normalize(FormulaCursor*, Direction);

    /**
     * Sets the cursor to select the child. The mark is placed before,
     * the position behind it.
     */
    virtual void selectChild(FormulaCursor*, BasicElement*);

    /**
     * Moves the cursor away from the given child. The cursor is
     * guaranteed to be inside this element.
     */
    //virtual void childWillVanish(FormulaCursor* cursor, BasicElement* child) = 0;

    /**
     * Returns wether the element has no more useful
     * children (except its main child) and should therefore
     * be replaced by its main child's content.
     */
    //virtual bool isSenseless();

    /**
     * @returns the latex representation of the element and
     * of the element's children
     */
    virtual QString toLatex();

    virtual QString formulaString();

    uint getRows() const { return content.count(); }
    uint getColumns() const { return content.getFirst()->count(); }

    SequenceElement* elementAt(uint row, uint column);

protected:

    //Save/load support

    /**
     * Returns the tag name of this element type.
     */
    virtual QString getTagName() const { return "MATRIX"; }

    /**
     * Appends our attributes to the dom element.
     */
    virtual void writeDom(QDomElement element);

    virtual QString getElementName() const { return "mtable"; }
    virtual void writeMathMLAttributes( QDomElement& element ) const;
    virtual void writeMathMLContent( QDomDocument& doc, 
                                     QDomElement& element,
                                     bool oasisFormat ) const;
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

    virtual bool readAttributesFromMathMLDom( const QDomElement& element );

    /**
     * Reads our content from the MathML node. Sets the node to the next node
     * that needs to be read. It is sometimes needed to read more than one node
     * (e. g. for fence operators).
     * Returns the number of nodes processed or -1 if it failed.
     */
	virtual int readContentFromMathMLDom(QDomNode& node);

private:
    void writeMathMLAttributes( QDomElement& element );

    MatrixSequenceElement* getElement(uint row, uint column)
        { return content.at(row)->at(column); }

    const MatrixSequenceElement* getElement( uint row, uint column ) const;

    /**
     * Searches through the matrix for the element. Sets the
     * row and column if found.
     * Returns true if the element was found. false otherwise.
     */
    bool searchElement(BasicElement* element, uint& row, uint& column);

    /**
     * The elements we contain.
     */
    QPtrList< QPtrList< MatrixSequenceElement > > content;

    /**
     * MathML Attributes. See Section 3.5.1.2
     */
    int m_rowNumber;
    VerticalAlign m_align;
    QValueList< VerticalAlign > m_rowAlign;
    QValueList< HorizontalAlign > m_columnAlign;
    QValueList< bool > m_alignmentScope;
    QValueList< SizeType > m_columnWidthType;
    QValueList< double > m_columnWidth;
    SizeType m_widthType;
    double m_width;
    QValueList< SizeType > m_rowSpacingType;
    QValueList< double > m_rowSpacing;
    QValueList< SizeType > m_columnSpacingType;
    QValueList< double > m_columnSpacing;
    QValueList< LineType > m_rowLines;
    QValueList< LineType > m_columnLines;
    LineType m_frame;
    SizeType m_frameHSpacingType;
    double m_frameHSpacing;
    SizeType m_frameVSpacingType;
    double m_frameVSpacing;
    SideType m_side;
    SizeType m_minLabelSpacingType;
    double m_minLabelSpacing;
    bool m_equalRows;
    bool m_customEqualRows;
    bool m_equalColumns;
    bool m_customEqualColumns;
    bool m_displayStyle;
    bool m_customDisplayStyle;
};



class MultilineSequenceElement;


/**
 * Any number of lines.
 */
class MultilineElement : public BasicElement {
    friend class KFCNewLine;

    typedef BasicElement inherited;
public:

    /**
     * The container this FormulaElement belongs to must not be 0,
     * except you really know what you are doing.
     */
    MultilineElement( BasicElement* parent = 0 );
    ~MultilineElement();

    MultilineElement( const MultilineElement& );

    virtual MultilineElement* clone() {
        return new MultilineElement( *this );
    }

    virtual bool accept( ElementVisitor* visitor );

    /**
     * The cursor has entered one of our child sequences.
     * This is a good point to tell the user where he is.
     */
    virtual void entered( SequenceElement* child );

    /**
     * Returns the element the point is in.
     */
    BasicElement* goToPos( FormulaCursor* cursor, bool& handled,
                           const LuPixelPoint& point, const LuPixelPoint& parentOrigin );

    /**
     * Sets the cursor inside this element to its start position.
     * For most elements that is the main child.
     */
    virtual void goInside(FormulaCursor* cursor);

    /**
     * Enters this element while moving to the left starting inside
     * the element `from'. Searches for a cursor position inside
     * this element or to the left of it.
     */
    virtual void moveLeft( FormulaCursor* cursor, BasicElement* from );

    /**
     * Enters this element while moving to the right starting inside
     * the element `from'. Searches for a cursor position inside
     * this element or to the right of it.
     */
    virtual void moveRight( FormulaCursor* cursor, BasicElement* from );

    /**
     * Enters this element while moving up starting inside
     * the element `from'. Searches for a cursor position inside
     * this element or above it.
     */
    virtual void moveUp( FormulaCursor* cursor, BasicElement* from );

    /**
     * Enters this element while moving down starting inside
     * the element `from'. Searches for a cursor position inside
     * this element or below it.
     */
    virtual void moveDown( FormulaCursor* cursor, BasicElement* from );

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
                       const ContextStyle& context,
                       ContextStyle::TextStyle tstyle,
                       ContextStyle::IndexStyle istyle,
					   StyleAttributes& style,
                       const LuPixelPoint& parentOrigin );

    /**
     * Dispatch this FontCommand to all our TextElement children.
     */
    virtual void dispatchFontCommand( FontCommand* cmd );

    virtual void insert(FormulaCursor*, QPtrList<BasicElement>&, Direction);
    virtual void remove(FormulaCursor*, QPtrList<BasicElement>&, Direction);

    virtual void normalize(FormulaCursor*, Direction);

    virtual SequenceElement* getMainChild();

    /**
     * Sets the cursor to select the child. The mark is placed before,
     * the position behind it.
     */
    virtual void selectChild(FormulaCursor* cursor, BasicElement* child);

    /**
     * @returns the latex representation of the element and
     * of the element's children
     */
    virtual QString toLatex();

    virtual QString formulaString();

    virtual void writeMathML( QDomDocument& doc, QDomNode& parent, bool oasisFormat = false ) const ;

protected:

    //Save/load support

    /**
     * Returns the tag name of this element type.
     */
    virtual QString getTagName() const { return "MULTILINE"; }

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


private:

    /**
     * The list of sequences. Each one is a line.
     */
    QPtrList< MultilineSequenceElement > content;
};


KFORMULA_NAMESPACE_END

#endif // MATRIXELEMENT_H
