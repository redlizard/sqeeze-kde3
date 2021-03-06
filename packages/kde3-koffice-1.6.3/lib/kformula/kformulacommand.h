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

#ifndef KFORMULACOMMAND_H
#define KFORMULACOMMAND_H

#include <qmap.h>
#include <qptrlist.h>
#include <qptrdict.h>
#include <qvaluevector.h>

#include <kcommand.h>

#include "fontstyle.h"
#include "kformulacontainer.h"
#include "formulacursor.h"

KFORMULA_NAMESPACE_BEGIN


/**
 * Base for all kformula commands.
 *
 * Each command works in the same way. The constructor sets up
 * everything. After the command is created you can execute
 * it. To create a command doesn't mean to execute it. These are
 * two different things.
 *
 * If the command execution fails or has nothing to do in the first place
 * you must not put it in the command history. isSenseless()
 * will return true then.
 *
 * If you don't like what you've done feel free to unexecute() .
 */
class PlainCommand : public KNamedCommand
{
public:

    /**
     * Sets up the command. Be careful not to change the cursor in
     * the constructor of any command. Each command must use the selection
     * it finds when it is executed for the first time. This way
     * you can use the @ref KMacroCommand .
     *
     * @param name a description to be used as menu entry.
     */
    PlainCommand(const QString& name);
    virtual ~PlainCommand();

    /**
     * debug only.
     */
    static int getEvilDestructionCount() { return evilDestructionCount; }

private:

    // debug
    static int evilDestructionCount;
};


class Command : public PlainCommand
{
public:

    /**
     * Sets up the command. Be careful not to change the cursor in
     * the constructor of any command. Each command must use the selection
     * it finds when it is executed for the first time. This way
     * you can use the @ref KMacroCommand .
     *
     * @param name a description to be used as menu entry.
     * @param document the container we are working for.
     */
    Command(const QString& name, Container* document);
    virtual ~Command();

protected:

    /**
     * @returns the cursor that is to be used to @ref execute the
     * command.
     */
    FormulaCursor* getExecuteCursor();

    /**
     * @returns the cursor that is to be used to @ref unexecute the
     * command.
     */
    FormulaCursor* getUnexecuteCursor();

    /**
     * Saves the cursor that is used to execute the command.
     */
    void setExecuteCursor(FormulaCursor* cursor);

    /**
     * Sets the cursor that is to be used to @ref unexecute the
     * command. This has to be called by @ref execute after the
     * formula has been changed but before the cursor has been
     * normalized.
     */
    void setUnexecuteCursor(FormulaCursor* cursor);

    /**
     * @returns the cursor that is active. It will be used to @ref execute
     * the command.
     */
    FormulaCursor* getActiveCursor() { return doc->activeCursor(); }

    /**
     * Tells the document to check if the formula changed.
     * Needs to be called by each @ref execute and @ref unexecute .
     */
    void testDirty() { doc->testDirty(); }

    /**
     * @returns our document.
     */
    Container* getDocument() const { return doc; }

private:

    void destroyUndoCursor() { delete undocursor; undocursor = 0; }

    /**
     * Cursor position before the command execution.
     */
    FormulaCursor::CursorData* cursordata;

    /**
     * Cursor position after the command execution.
     */
    FormulaCursor::CursorData* undocursor;

    /**
     * The container we belong to.
     */
    Container* doc;
};


/**
 * Base for all commands that want to add a simple element.
 */
class KFCAdd : public Command
{
public:

    KFCAdd(const QString &name, Container* document);

    virtual void execute();
    virtual void unexecute();

    /**
     * Collects all elements that are to be added.
     */
    void addElement(BasicElement* element) { addList.append(element); }

private:

    /**
     * the list where all elements are stored that are removed
     * from the tree.
     */
    QPtrList<BasicElement> addList;
};


/**
 * Command that is used to remove the current selection
 * if we want to replace it with another element.
 */
class KFCRemoveSelection : public Command
{
public:

    /**
     * generic add command, default implementation do nothing
     */
    KFCRemoveSelection(Container* document,
                       Direction dir = beforeCursor);

    virtual void execute();
    virtual void unexecute();

private:

    /**
     * the list where all elements are stored that are removed
     * from the tree.
     */
    QPtrList<BasicElement> removedList;

    Direction dir;
};


/**
 * Removes the current selection and adds any new elements
 * afterwards.
 */
class KFCReplace : public KFCAdd
{
public:

    KFCReplace(const QString &name, Container* document);
    ~KFCReplace();

    virtual void execute();
    virtual void unexecute();

private:

    /**
     * The command that needs to be executed first if there is a selection.
     */
    KFCRemoveSelection* removeSelection;
};

/**
 * Command to add a token element, with content.
 */
class KFCAddToken : public Command
{
public:
    KFCAddToken( const QString &name, Container *document);
    ~KFCAddToken();

    virtual void execute();
    virtual void unexecute();

    /**
     * Collect all tokens that are to be added
     */
    void addToken( BasicElement* element );

    /**
     * Collect content for each token
     */
    void addContent( BasicElement* element, BasicElement* content ) { 
        contentList.find( element )->append( content ); 
    }


private:

    /**
     * List where all token elements are stored.
     */
    QPtrList< BasicElement > tokenList;

    /**
     * Dictionary where all content elements are stored
     */
    QPtrDict< QPtrList< BasicElement > > contentList;
};

/**
 * Removes the current selection and adds any new tokens
 * afterwards.
 */
class KFCReplaceToken : public KFCAddToken
{
public:

    KFCReplaceToken(const QString &name, Container* document);
    ~KFCReplaceToken();

    virtual void execute();
    virtual void unexecute();

private:

    /**
     * The command that needs to be executed first if there is a selection.
     */
    KFCRemoveSelection* removeSelection;

};


/**
 * Add and remove arbitrary token elements and their contents. This command
 * add needed flexibility in order to remove and insert new tokens in the
 * middle of a token, that is, splitting the token and creating new ones.
 */
class KFCSplitToken : public KFCAddToken
{
public:

    KFCSplitToken(const QString &name, Container* document);
    ~KFCSplitToken();

    virtual void execute();
    virtual void unexecute();

    /**
     * Collect cursor data that will be needed to do the stuff properly
     */
    void addCursor( FormulaCursor* cursor ) { setExecuteCursor( cursor ); }

private:

    /**
     * The command that needs to be executed first if there is a selection.
     */
    KFCRemoveSelection* removeSelection;

    /**
     * The list of elements that are splited into another token
     */
    QPtrList< BasicElement > splitList;

    /**
     * Information about cursor position where the split is made.
     * Needed to restore splited elements.
     */
    FormulaCursor::CursorData* splitCursor;
};


/**
 * Command that is used to remove the currently
 * selected element.
 */
class KFCRemove : public Command
{
public:

    /**
     * generic add command, default implementation do nothing
     */
    KFCRemove(Container* document, Direction dir);
    ~KFCRemove();

    virtual void execute();
    virtual void unexecute();

    /**
     * A command might have no effect.
     * @returns true if nothing happened.
     */
    //virtual bool isSenseless() { return removedList.isEmpty(); }

private:

    /**
     * the list where all elements are stored that are removed
     * from the tree.
     */
    QPtrList<BasicElement> removedList;

    /**
     * The element we might have extracted.
     */
    BasicElement* element;

    /**
     * If this is a complex remove command we need to remember two
     * cursor positions. The one after the first removal (this one)
     * and another at the end.
     */
    FormulaCursor::CursorData* simpleRemoveCursor;

    Direction dir;
};


/**
 * Command to remove the parent element.
 */
class KFCRemoveEnclosing : public Command
{
public:
    KFCRemoveEnclosing(Container* document, Direction dir);
    ~KFCRemoveEnclosing();

    virtual void execute();
    virtual void unexecute();

private:
    BasicElement* element;

    Direction direction;
};


/**
 * The command that takes the main child out of the selected
 * element and replaces the element with it.
 */
class KFCAddReplacing : public Command
{
public:
    KFCAddReplacing(const QString &name, Container* document);
    ~KFCAddReplacing();

    virtual void execute();
    virtual void unexecute();

    void setElement(BasicElement* e) { element = e; }

private:

    /**
     * The element that is to be inserted.
     */
    BasicElement* element;
};


/**
 * Add an index. The element that gets the index needs to be there
 * already.
 */
class KFCAddGenericIndex : public KFCAdd
{
public:

    KFCAddGenericIndex(Container* document, ElementIndexPtr index);

    virtual void execute();

private:
    ElementIndexPtr index;
};


class IndexElement;

/**
 * Add an IndexElement.
 */
class KFCAddIndex : public KFCAddReplacing
{
public:

    KFCAddIndex(Container* document, IndexElement* element, ElementIndexPtr index);
    ~KFCAddIndex();

    virtual void execute();
    virtual void unexecute();

private:
    KFCAddGenericIndex addIndex;
};


class FormulaElement;

class KFCChangeBaseSize : public PlainCommand {
public:
    KFCChangeBaseSize( const QString& name, Container* document, FormulaElement* formula, int size );

    void execute();
    void unexecute();

private:
    Container* m_document;
    FormulaElement* m_formula;
    int m_size;
    int m_oldSize;
};


/**
 * Base for all font commands that act on the current selection.
 * Implements the visitor pattern. (Really?)
 */
class FontCommand : public Command {
public:
    FontCommand( const QString& name, Container* document );

    /**
     * Collects all elements that are to be modified.
     */
    void addTextElement( TextElement* element ) { list.append(element); }

    /**
     * Collects all parent elements those children are to be changend.
     */
    void addElement( BasicElement* element ) { elementList.append( element ); }

protected:

    QPtrList<TextElement>& childrenList() { return list; }

    void collectChildren();

    void parseSequences( const QMap<SequenceElement*, int>& parents );

private:

    /**
     * the list where all elements are stored that are removed
     * from the tree.
     */
    QPtrList<TextElement> list;

    QPtrList<BasicElement> elementList;
};


/**
 * Changes the char style of a number of elements an their children.
 */
class CharStyleCommand : public FontCommand {
public:
    CharStyleCommand( CharStyle cs, const QString& name, Container* document );

    virtual void execute();
    virtual void unexecute();

private:

    typedef QValueVector<CharStyle> StyleList;

    StyleList styleList;
    CharStyle charStyle;
};


/**
 * Changes the char family of a number of elements an their children.
 */
class CharFamilyCommand : public FontCommand {
public:
    CharFamilyCommand( CharFamily cf, const QString& name, Container* document );

    virtual void execute();
    virtual void unexecute();

private:

    typedef QValueVector<CharFamily> FamilyList;

    FamilyList familyList;
    CharFamily charFamily;
};


KFORMULA_NAMESPACE_END

#endif // KFORMULACOMMAND_H
