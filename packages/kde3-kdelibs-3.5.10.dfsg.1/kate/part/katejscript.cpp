/* This file is part of the KDE libraries
   Copyright (C) 2005 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2005 Joseph Wenninger <jowenn@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "katejscript.h"

#include "katedocument.h"
#include "kateview.h"
#include "katefactory.h"
#include "kateconfig.h"
#include "kateautoindent.h"
#include "katehighlight.h"
#include "katetextline.h"

#include "kateindentscriptabstracts.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <kdebug.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kconfig.h>

#include <kjs/function_object.h>
#include <kjs/interpreter.h>
#include <kjs/lookup.h>

#include <qfile.h>
#include <qfileinfo.h>
#include <qpopupmenu.h>
#include <qregexp.h>
#include <qtextstream.h>


namespace KJS {

// taken from khtml
// therefor thx to:
// Copyright (C) 1999-2003 Harri Porten (porten@kde.org)
// Copyright (C) 2001-2003 David Faure (faure@kde.org)
// Copyright (C) 2003 Apple Computer, Inc.

UString::UString(const QString &d)
{
  unsigned int len = d.length();
  UChar *dat = new UChar[len];
  memcpy(dat, d.unicode(), len * sizeof(UChar));
  rep = UString::Rep::create(dat, len);
}

QString UString::qstring() const
{
  return QString((QChar*) data(), size());
}

QConstString UString::qconststring() const
{
  return QConstString((QChar*) data(), size());
}

//BEGIN global methods
class KateJSGlobalFunctions : public ObjectImp
{
  public:
    KateJSGlobalFunctions(int i, int length);
    virtual bool implementsCall() const { return true; }
    virtual Value call(ExecState *exec, Object &thisObj, const List &args);

    enum {
      Debug
    };

  private:
    int id;
};
KateJSGlobalFunctions::KateJSGlobalFunctions(int i, int length) : ObjectImp(), id(i)
{
  putDirect(lengthPropertyName,length,DontDelete|ReadOnly|DontEnum);
}
Value KateJSGlobalFunctions::call(ExecState *exec, Object &/*thisObj*/, const List &args)
{
  switch (id) {
    case Debug:
      qDebug("Kate (KJS Scripting): %s", args[0].toString(exec).ascii());
      return Undefined();
    default:
      break;
  }

  return Undefined();
}
//END global methods

} // namespace KJS

//BEGIN JS API STUFF

class KateJSGlobal : public KJS::ObjectImp {
public:
  virtual KJS::UString className() const { return "global"; }
};

class KateJSDocument : public KJS::ObjectImp
{
  public:
    KateJSDocument (KJS::ExecState *exec, KateDocument *_doc);

    KJS::Value get( KJS::ExecState *exec, const  KJS::Identifier &propertyName) const;

    KJS::Value getValueProperty(KJS::ExecState *exec, int token) const;

    void put(KJS::ExecState *exec, const KJS::Identifier &propertyName, const KJS::Value& value, int attr = KJS::None);

    void putValueProperty(KJS::ExecState *exec, int token, const KJS::Value& value, int attr);

    const KJS::ClassInfo* classInfo() const { return &info; }

    enum { FullText,
          Text,
          TextLine,
          Lines,
          Length,
          LineLength,
          SetText,
          Clear,
          InsertText,
          RemoveText,
          InsertLine,
          RemoveLine,
          EditBegin,
          EditEnd,
          IndentWidth,
          IndentMode,
          SpaceIndent,
          MixedIndent,
          HighlightMode,
          IsInWord,
          CanBreakAt,
          CanComment,
          CommentMarker,
          CommentStart,
          CommentEnd,
          Attribute
    };

  public:
    KateDocument *doc;

    static const KJS::ClassInfo info;
};

class KateJSView : public KJS::ObjectImp
{
  public:
    KateJSView (KJS::ExecState *exec, KateView *_view);

    KJS::Value get( KJS::ExecState *exec, const  KJS::Identifier &propertyName) const;

    KJS::Value getValueProperty(KJS::ExecState *exec, int token) const;

    void put(KJS::ExecState *exec, const KJS::Identifier &propertyName, const KJS::Value& value, int attr = KJS::None);

    void putValueProperty(KJS::ExecState *exec, int token, const KJS::Value& value, int attr);

    const KJS::ClassInfo* classInfo() const { return &info; }

    enum { CursorLine,
          CursorColumn,
          CursorColumnReal,
          SetCursorPosition,
          SetCursorPositionReal,
          Selection,
          HasSelection,
          SetSelection,
          RemoveSelectedText,
          SelectAll,
          ClearSelection,
          SelStartLine,
          SelStartCol,
          SelEndLine,
          SelEndCol
    };

  public:
    KateView *view;

    static const KJS::ClassInfo info;
};

class KateJSIndenter : public KJS::ObjectImp
{
  public:
    KateJSIndenter (KJS::ExecState *exec);
    /*
    KJS::Value get( KJS::ExecState *exec, const  KJS::Identifier &propertyName) const;

    KJS::Value getValueProperty(KJS::ExecState *exec, int token) const;

    void put(KJS::ExecState *exec, const KJS::Identifier &propertyName, const KJS::Value& value, int attr = KJS::None);

    void putValueProperty(KJS::ExecState *exec, int token, const KJS::Value& value, int attr);
    */
    const KJS::ClassInfo* classInfo() const { return &info; }

    enum { OnChar,
          OnLine,
          OnNewline,
          Dummy
    };

  public:

    static const KJS::ClassInfo info;
};

#include "katejscript.lut.h"

//END

KateJScript::KateJScript ()
 : m_global (new KJS::Object (new KateJSGlobal ()))
 , m_interpreter (new KJS::Interpreter (*m_global))
 , m_document (new KJS::Object(wrapDocument(m_interpreter->globalExec(), 0)))
 , m_view (new KJS::Object (wrapView(m_interpreter->globalExec(), 0)))
{
  // put some stuff into env., this should stay for all executions, as we keep external
  // references to the inserted KJS::Objects, this should avoid any garbage collection
  m_interpreter->globalObject().put(m_interpreter->globalExec(), "document", *m_document);
  m_interpreter->globalObject().put(m_interpreter->globalExec(), "view", *m_view);
  m_interpreter->globalObject().put(m_interpreter->globalExec(), "debug",
        KJS::Object(new KateJSGlobalFunctions(KateJSGlobalFunctions::Debug,1)));
}

KateJScript::~KateJScript ()
{
  delete m_view;
  delete m_document;
  delete m_interpreter;
  delete m_global;
}

KJS::ObjectImp *KateJScript::wrapDocument (KJS::ExecState *exec, KateDocument *doc)
{
  return new KateJSDocument(exec, doc);
}

KJS::ObjectImp *KateJScript::wrapView (KJS::ExecState *exec, KateView *view)
{
  return new KateJSView(exec, view);
}

bool KateJScript::execute (KateView *view, const QString &script, QString &errorMsg)
{
  // no view, no fun
  if (!view)
  {
    errorMsg = i18n("Could not access view");
    return false;
  }

  // init doc & view with new pointers!
  static_cast<KateJSDocument *>( m_document->imp() )->doc = view->doc();
  static_cast<KateJSView *>( m_view->imp() )->view = view;

  // run the script for real
  KJS::Completion comp (m_interpreter->evaluate(script));

  if (comp.complType() == KJS::Throw)
  {
    KJS::ExecState *exec = m_interpreter->globalExec();

    KJS::Value exVal = comp.value();

    char *msg = exVal.toString(exec).ascii();

    int lineno = -1;

    if (exVal.type() == KJS::ObjectType)
    {
      KJS::Value lineVal = KJS::Object::dynamicCast(exVal).get(exec,"line");

      if (lineVal.type() == KJS::NumberType)
        lineno = int(lineVal.toNumber(exec));
    }

    errorMsg = i18n("Exception, line %1: %2").arg(lineno).arg(msg);
    return false;
  }

  return true;
}

//BEGIN KateJSDocument

// -------------------------------------------------------------------------
/* Source for KateJSDocumentProtoTable.
@begin KateJSDocumentProtoTable 21
#
# edit interface stuff + editBegin/End, this is nice start
#
  textFull       KateJSDocument::FullText      DontDelete|Function 0
  textRange      KateJSDocument::Text          DontDelete|Function 4
  textLine       KateJSDocument::TextLine      DontDelete|Function 1
  lines          KateJSDocument::Lines         DontDelete|Function 0
  length         KateJSDocument::Length        DontDelete|Function 0
  lineLength     KateJSDocument::LineLength    DontDelete|Function 1
  setText        KateJSDocument::SetText       DontDelete|Function 1
  clear          KateJSDocument::Clear         DontDelete|Function 0
  insertText     KateJSDocument::InsertText    DontDelete|Function 3
  removeText     KateJSDocument::RemoveText    DontDelete|Function 4
  insertLine     KateJSDocument::InsertLine    DontDelete|Function 2
  removeLine     KateJSDocument::RemoveLine    DontDelete|Function 1
  editBegin      KateJSDocument::EditBegin     DontDelete|Function 0
  editEnd        KateJSDocument::EditEnd       DontDelete|Function 0
#
# methods from highlight (and around)
#
  isInWord       KateJSDocument::IsInWord         DontDelete|Function 2
  canBreakAt     KateJSDocument::CanBreakAt       DontDelete|Function 2
  canComment     KateJSDocument::CanComment       DontDelete|Function 2
  commentMarker  KateJSDocument::CommentMarker    DontDelete|Function 1
  commentStart   KateJSDocument::CommentStart     DontDelete|Function 1
  commentEnd     KateJSDocument::CommentEnd       DontDelete|Function 1
  attribute      KateJSDocument::Attribute        DontDelete|Function 2
@end

@begin KateJSDocumentTable 6
#
# Configuration properties
#
  indentWidth     KateJSDocument::IndentWidth   DontDelete|ReadOnly
  indentMode      KateJSDocument::IndentMode    DontDelete|ReadOnly
  spaceIndent     KateJSDocument::SpaceIndent   DontDelete|ReadOnly
  mixedIndent     KateJSDocument::MixedIndent   DontDelete|ReadOnly
  highlightMode   KateJSDocument::HighlightMode DontDelete|ReadOnly
@end
*/

DEFINE_PROTOTYPE("KateJSDocument",KateJSDocumentProto)
IMPLEMENT_PROTOFUNC(KateJSDocumentProtoFunc)
IMPLEMENT_PROTOTYPE(KateJSDocumentProto,KateJSDocumentProtoFunc)

const KJS::ClassInfo KateJSDocument::info = { "KateJSDocument", 0, 0, 0 };

KJS::Value KJS::KateJSDocumentProtoFunc::call(KJS::ExecState *exec, KJS::Object &thisObj, const KJS::List &args)
{
  KJS_CHECK_THIS( KateJSDocument, thisObj );

  KateDocument *doc = static_cast<KateJSDocument *>( thisObj.imp() )->doc;

  if (!doc)
    return KJS::Undefined();

  switch (id)
  {
    case KateJSDocument::FullText:
      return KJS::String (doc->text());

    case KateJSDocument::Text:
      return KJS::String (doc->text(args[0].toUInt32(exec), args[1].toUInt32(exec), args[2].toUInt32(exec), args[3].toUInt32(exec)));

    case KateJSDocument::TextLine:
      return KJS::String (doc->textLine (args[0].toUInt32(exec)));

    case KateJSDocument::Lines:
      return KJS::Number (doc->numLines());

    case KateJSDocument::Length:
      return KJS::Number (doc->length());

    case KateJSDocument::LineLength:
      return KJS::Number (doc->lineLength(args[0].toUInt32(exec)));

    case KateJSDocument::SetText:
      return KJS::Boolean (doc->setText(args[0].toString(exec).qstring()));

    case KateJSDocument::Clear:
      return KJS::Boolean (doc->clear());

    case KateJSDocument::InsertText:
      return KJS::Boolean (doc->insertText (args[0].toUInt32(exec), args[1].toUInt32(exec), args[2].toString(exec).qstring()));

    case KateJSDocument::RemoveText:
      return KJS::Boolean (doc->removeText(args[0].toUInt32(exec), args[1].toUInt32(exec), args[2].toUInt32(exec), args[3].toUInt32(exec)));

    case KateJSDocument::InsertLine:
      return KJS::Boolean (doc->insertLine (args[0].toUInt32(exec), args[1].toString(exec).qstring()));

    case KateJSDocument::RemoveLine:
      return KJS::Boolean (doc->removeLine (args[0].toUInt32(exec)));

    case KateJSDocument::EditBegin:
      doc->editBegin();
      return KJS::Null ();

    case KateJSDocument::EditEnd:
      doc->editEnd ();
      return KJS::Null ();

    case KateJSDocument::IsInWord:
      return KJS::Boolean( doc->highlight()->isInWord( args[0].toString(exec).qstring().at(0), args[1].toUInt32(exec) ) );

    case KateJSDocument::CanBreakAt:
      return KJS::Boolean( doc->highlight()->canBreakAt( args[0].toString(exec).qstring().at(0), args[1].toUInt32(exec) ) );

    case KateJSDocument::CanComment:
      return KJS::Boolean( doc->highlight()->canComment( args[0].toUInt32(exec), args[1].toUInt32(exec) ) );

    case KateJSDocument::CommentMarker:
      return KJS::String( doc->highlight()->getCommentSingleLineStart( args[0].toUInt32(exec) ) );

    case KateJSDocument::CommentStart:
      return KJS::String( doc->highlight()->getCommentStart( args[0].toUInt32(exec) ) );

    case KateJSDocument::CommentEnd:
      return KJS::String( doc->highlight()->getCommentEnd(  args[0].toUInt32(exec) ) );

    case KateJSDocument::Attribute:
      return KJS::Number( doc->kateTextLine(args[0].toUInt32(exec))->attribute(args[1].toUInt32(exec)) );
  }

  return KJS::Undefined();
}

KJS::Value KateJSDocument::get( KJS::ExecState *exec, const  KJS::Identifier &propertyName) const
{
  return KJS::lookupGetValue<KateJSDocument,KJS::ObjectImp>(exec, propertyName, &KateJSDocumentTable, this );
}

KJS::Value KateJSDocument::getValueProperty(KJS::ExecState *exec, int token) const
{
  if (!doc)
    return KJS::Undefined ();

  switch (token) {
    case KateJSDocument::IndentWidth:
      return KJS::Number( doc->config()->indentationWidth() );

    case KateJSDocument::IndentMode:
      return KJS::String( KateAutoIndent::modeName( doc->config()->indentationMode() ) );

    case KateJSDocument::SpaceIndent:
      return KJS::Boolean( doc->config()->configFlags() & KateDocumentConfig::cfSpaceIndent );

    case KateJSDocument::MixedIndent:
      return KJS::Boolean( doc->config()->configFlags() & KateDocumentConfig::cfMixedIndent );

    case KateJSDocument::HighlightMode:
      return KJS::String( doc->hlModeName( doc->hlMode() ) );
  }

  return KJS::Undefined ();
}

void KateJSDocument::put(KJS::ExecState *exec, const KJS::Identifier &propertyName, const KJS::Value& value, int attr)
{
  KJS::lookupPut<KateJSDocument,KJS::ObjectImp>(exec, propertyName, value, attr, &KateJSDocumentTable, this );
}

void KateJSDocument::putValueProperty(KJS::ExecState *exec, int token, const KJS::Value& value, int attr)
{
  if (!doc)
    return;
}

KateJSDocument::KateJSDocument (KJS::ExecState *exec, KateDocument *_doc)
    : KJS::ObjectImp (KateJSDocumentProto::self(exec))
    , doc (_doc)
{
}

//END

//BEGIN KateJSView

// -------------------------------------------------------------------------
/* Source for KateJSViewProtoTable.
@begin KateJSViewProtoTable 14
  cursorLine          KateJSView::CursorLine            DontDelete|Function 0
  cursorColumn        KateJSView::CursorColumn          DontDelete|Function 0
  cursorColumnReal    KateJSView::CursorColumnReal      DontDelete|Function 0
  setCursorPosition   KateJSView::SetCursorPosition     DontDelete|Function 2
  setCursorPositionReal KateJSView::SetCursorPositionReal DontDelete|Function 2
  selection           KateJSView::Selection             DontDelete|Function 0
  hasSelection        KateJSView::HasSelection          DontDelete|Function 0
  setSelection        KateJSView::SetSelection          DontDelete|Function 4
  removeSelectedText  KateJSView::RemoveSelectedText    DontDelete|Function 0
  selectAll           KateJSView::SelectAll             DontDelete|Function 0
  clearSelection      KateJSView::ClearSelection        DontDelete|Function 0
@end
*/

/* Source for KateJSViewTable.
@begin KateJSViewTable 5
  selectionStartLine    KateJSView::SelStartLine        DontDelete|ReadOnly
  selectionStartColumn  KateJSView::SelStartCol         DontDelete|ReadOnly
  selectionEndLine      KateJSView::SelEndLine          DontDelete|ReadOnly
  selectionEndColumn    KateJSView::SelEndCol           DontDelete|ReadOnly
@end
*/

DEFINE_PROTOTYPE("KateJSView",KateJSViewProto)
IMPLEMENT_PROTOFUNC(KateJSViewProtoFunc)
IMPLEMENT_PROTOTYPE(KateJSViewProto,KateJSViewProtoFunc)

const KJS::ClassInfo KateJSView::info = { "KateJSView", 0, &KateJSViewTable, 0 };

KJS::Value KJS::KateJSViewProtoFunc::call(KJS::ExecState *exec, KJS::Object &thisObj, const KJS::List &args)
{
  KJS_CHECK_THIS( KateJSView, thisObj );

  KateView *view = static_cast<KateJSView *>( thisObj.imp() )->view;

  if (!view)
    return KJS::Undefined();

  switch (id)
  {
    case KateJSView::CursorLine:
      return KJS::Number (view->cursorLine());

    case KateJSView::CursorColumn:
      return KJS::Number (view->cursorColumn());

    case KateJSView::CursorColumnReal:
      return KJS::Number (view->cursorColumnReal());

    case KateJSView::SetCursorPosition:
      return KJS::Boolean( view->setCursorPosition( args[0].toUInt32(exec), args[1].toUInt32(exec) ) );

    case KateJSView::SetCursorPositionReal:
      return KJS::Boolean( view->setCursorPositionReal( args[0].toUInt32(exec), args[1].toUInt32(exec) ) );

    // SelectionInterface goes in the view, in anticipation of the future
    case KateJSView::Selection:
      return KJS::String( view->selection() );

    case KateJSView::HasSelection:
      return KJS::Boolean( view->hasSelection() );

    case KateJSView::SetSelection:
      return KJS::Boolean( view->setSelection(args[0].toUInt32(exec),
                                              args[1].toUInt32(exec),
                                              args[2].toUInt32(exec),
                                              args[3].toUInt32(exec)) );

    case KateJSView::RemoveSelectedText:
      return KJS::Boolean( view->removeSelectedText() );

    case KateJSView::SelectAll:
      return KJS::Boolean( view->selectAll() );

    case KateJSView::ClearSelection:
      return KJS::Boolean( view->clearSelection() );
  }

  return KJS::Undefined();
}

KateJSView::KateJSView (KJS::ExecState *exec, KateView *_view)
    : KJS::ObjectImp (KateJSViewProto::self(exec))
    , view (_view)
{
}

KJS::Value KateJSView::get( KJS::ExecState *exec, const  KJS::Identifier &propertyName) const
{
  return KJS::lookupGetValue<KateJSView,KJS::ObjectImp>(exec, propertyName, &KateJSViewTable, this );
}

KJS::Value KateJSView::getValueProperty(KJS::ExecState *exec, int token) const
{
  if (!view)
    return KJS::Undefined ();

  switch (token) {
    case KateJSView::SelStartLine:
      return KJS::Number( view->selStartLine() );

    case KateJSView::SelStartCol:
      return KJS::Number( view->selStartCol() );

    case KateJSView::SelEndLine:
      return KJS::Number( view->selEndLine() );

    case KateJSView::SelEndCol:
      return KJS::Number( view->selEndCol() );
    }

  return KJS::Undefined ();
}

void KateJSView::put(KJS::ExecState *exec, const KJS::Identifier &propertyName, const KJS::Value& value, int attr)
{
   KJS::lookupPut<KateJSView,KJS::ObjectImp>(exec, propertyName, value, attr, &KateJSViewTable, this );
}

void KateJSView::putValueProperty(KJS::ExecState *exec, int token, const KJS::Value& value, int attr)
{
  if (!view)
    return;


}

//END

//BEGIN KateJScriptManager

KateJScriptManager::KateJScriptManager ()
{
  m_scripts.setAutoDelete (true);
  collectScripts ();
}

KateJScriptManager::~KateJScriptManager ()
{
}

void KateJScriptManager::collectScripts (bool force)
{
// If there's something in myModeList the Mode List was already built so, don't do it again
  if (!m_scripts.isEmpty())
    return;

  // We'll store the scripts list in this config
  KConfig config("katepartjscriptrc", false, false);

  // figure out if the kate install is too new
  config.setGroup ("General");
  if (config.readNumEntry ("Version") > config.readNumEntry ("CachedVersion"))
  {
    config.writeEntry ("CachedVersion", config.readNumEntry ("Version"));
    force = true;
  }

  // Let's get a list of all the .js files
  QStringList list = KGlobal::dirs()->findAllResources("data","katepart/scripts/*.js",false,true);

  // Let's iterate through the list and build the Mode List
  for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
  {
    // Each file has a group called:
    QString Group="Cache "+ *it;

    // Let's go to this group
    config.setGroup(Group);

    // stat the file
    struct stat sbuf;
    memset (&sbuf, 0, sizeof(sbuf));
    stat(QFile::encodeName(*it), &sbuf);

    // If the group exist and we're not forced to read the .js file, let's build myModeList for katepartjscriptrc
    if (!force && config.hasGroup(Group) && (sbuf.st_mtime == config.readNumEntry("lastModified")))
    {
    }
    else
    {
      kdDebug (13050) << "add script: " << *it << endl;

      QString desktopFile =  (*it).left((*it).length()-2).append ("desktop");

      kdDebug (13050) << "add script (desktop file): " << desktopFile << endl;

      QFileInfo dfi (desktopFile);

      if (dfi.exists())
      {
        KConfig df (desktopFile, true, false);
        df.setDesktopGroup ();

        // get cmdname, fallback to baseName, if it is empty, therefor not use the kconfig fallback
        QString cmdname = df.readEntry ("X-Kate-Command");
        if (cmdname.isEmpty())
        {
          QFileInfo fi (*it);
          cmdname = fi.baseName();
        }

        if (m_scripts[cmdname])
          continue;

        KateJScriptManager::Script *s = new KateJScriptManager::Script ();

        s->name = cmdname;
        s->filename = *it;
        s->desktopFileExists = true;

        m_scripts.insert (s->name, s);
      }
      else // no desktop file around, fall back to scriptfilename == commandname
      {
        kdDebug (13050) << "add script: fallback, no desktop file around!" << endl;

        QFileInfo fi (*it);

        if (m_scripts[fi.baseName()])
          continue;

        KateJScriptManager::Script *s = new KateJScriptManager::Script ();

        s->name = fi.baseName();
        s->filename = *it;
        s->desktopFileExists = false;

        m_scripts.insert (s->name, s);
      }
    }
  }

  // Syncronize with the file katepartjscriptrc
  config.sync();
}

bool KateJScriptManager::exec( Kate::View *view, const QString &_cmd, QString &errorMsg )
{
  // cast it hardcore, we know that it is really a kateview :)
  KateView *v = (KateView*) view;

  if ( !v )
  {
    errorMsg = i18n("Could not access view");
    return false;
  }

   //create a list of args
  QStringList args( QStringList::split( QRegExp("\\s+"), _cmd ) );
  QString cmd ( args.first() );
  args.remove( args.first() );

  kdDebug(13050) << "try to exec: " << cmd << endl;

  if (!m_scripts[cmd])
  {
    errorMsg = i18n("Command not found");
    return false;
  }

  QFile file (m_scripts[cmd]->filename);

  if ( !file.open( IO_ReadOnly ) )
    {
    errorMsg = i18n("JavaScript file not found");
    return false;
  }

  QTextStream stream( &file );
  stream.setEncoding (QTextStream::UnicodeUTF8);

  QString source = stream.read ();

  file.close();

  return KateFactory::self()->jscript()->execute(v, source, errorMsg);
}

bool KateJScriptManager::help( Kate::View *, const QString &cmd, QString &msg )
{
  if (!m_scripts[cmd] || !m_scripts[cmd]->desktopFileExists)
    return false;

  KConfig df (m_scripts[cmd]->desktopFilename(), true, false);
  df.setDesktopGroup ();

  msg = df.readEntry ("X-Kate-Help");

  if (msg.isEmpty())
    return false;

  return true;
}

QStringList KateJScriptManager::cmds()
{
   QStringList l;

   QDictIterator<KateJScriptManager::Script> it( m_scripts );
   for( ; it.current(); ++it )
     l << it.current()->name;

   return l;
}

//END




//BEGIN KateJSIndenter

// -------------------------------------------------------------------------
/* Source for KateJSIndenterProtoTable.
@begin KateJSIndenterProtoTable 1
  Dummy                 KateJSIndenter::Dummy             DontDelete
@end
*/

/* Source for KateJSIndenterTable.
@begin KateJSIndenterTable 3
  onchar                KateJSIndenter::OnChar            DontDelete
  onnewline             KateJSIndenter::OnNewline         DontDelete
  online                KateJSIndenter::OnLine            DontDelete

@end
*/

KateJSIndenter::KateJSIndenter (KJS::ExecState *exec)
    : KJS::ObjectImp (KateJSViewProto::self(exec))
{
}

DEFINE_PROTOTYPE("KateJSIndenter",KateJSIndenterProto)
IMPLEMENT_PROTOFUNC(KateJSIndenterProtoFunc)
IMPLEMENT_PROTOTYPE(KateJSIndenterProto,KateJSIndenterProtoFunc)

const KJS::ClassInfo KateJSIndenter::info = { "KateJSIndenter", 0, &KateJSIndenterTable, 0 };

KJS::Value KJS::KateJSIndenterProtoFunc::call(KJS::ExecState *exec, KJS::Object &thisObj, const KJS::List &args)
{
  KJS_CHECK_THIS( KateJSIndenter, thisObj );

  return KJS::Undefined();
}

//END

//BEGIN KateIndentJScriptImpl
KateIndentJScriptImpl::KateIndentJScriptImpl(const QString& internalName,
        const QString  &filePath, const QString &niceName,
        const QString &copyright, double version):
          KateIndentScriptImplAbstract(internalName,filePath,niceName,copyright,version),m_interpreter(0),m_indenter(0)
{
}


KateIndentJScriptImpl::~KateIndentJScriptImpl()
{
  deleteInterpreter();
}

void KateIndentJScriptImpl::decRef()
{
  KateIndentScriptImplAbstract::decRef();
  if (refCount()==0)
  {
    deleteInterpreter();
  }
}

void KateIndentJScriptImpl::deleteInterpreter()
{
    m_docWrapper=0;
    m_viewWrapper=0;
    delete m_indenter;
    m_indenter=0;
    delete m_interpreter;
    m_interpreter=0;
}

bool KateIndentJScriptImpl::setupInterpreter(QString &errorMsg)
{
  if (!m_interpreter)
  {
    kdDebug(13050)<<"Setting up interpreter"<<endl;
    m_interpreter=new KJS::Interpreter(KJS::Object(new KateJSGlobal()));
    m_docWrapper=new KateJSDocument(m_interpreter->globalExec(),0);
    m_viewWrapper=new KateJSView(m_interpreter->globalExec(),0);
    m_indenter=new KJS::Object(new KateJSIndenter(m_interpreter->globalExec()));
    m_interpreter->globalObject().put(m_interpreter->globalExec(),"document",KJS::Object(m_docWrapper),KJS::DontDelete | KJS::ReadOnly);
    m_interpreter->globalObject().put(m_interpreter->globalExec(),"view",KJS::Object(m_viewWrapper),KJS::DontDelete | KJS::ReadOnly);
    m_interpreter->globalObject().put(m_interpreter->globalExec(),"debug", KJS::Object(new 
              KateJSGlobalFunctions(KateJSGlobalFunctions::Debug,1)));
    m_interpreter->globalObject().put(m_interpreter->globalExec(),"indenter",*m_indenter,KJS::DontDelete | KJS::ReadOnly);
    QFile file (filePath());

    if ( !file.open( IO_ReadOnly ) )
      {
      errorMsg = i18n("JavaScript file not found");
      deleteInterpreter();
      return false;
    }

    QTextStream stream( &file );
    stream.setEncoding (QTextStream::UnicodeUTF8);

    QString source = stream.read ();

    file.close();

    KJS::Completion comp (m_interpreter->evaluate(source));
    if (comp.complType() == KJS::Throw)
    {
      KJS::ExecState *exec = m_interpreter->globalExec();

      KJS::Value exVal = comp.value();

      char *msg = exVal.toString(exec).ascii();

      int lineno = -1;

      if (exVal.type() == KJS::ObjectType)
      {
        KJS::Value lineVal = KJS::Object::dynamicCast(exVal).get(exec,"line");

        if (lineVal.type() == KJS::NumberType)
          lineno = int(lineVal.toNumber(exec));
      }

      errorMsg = i18n("Exception, line %1: %2").arg(lineno).arg(msg);
      deleteInterpreter();
      return false;
    } else {
      return true;
    }
  } else return true;
}


inline static bool KateIndentJScriptCall(Kate::View *view, QString &errorMsg, KateJSDocument *docWrapper, KateJSView *viewWrapper,
        KJS::Interpreter *interpreter, KJS::Object lookupobj,const KJS::Identifier& func,KJS::List params)
{
 // no view, no fun
  if (!view)
  {
    errorMsg = i18n("Could not access view");
    return false;
  }

  KateView *v=(KateView*)view;

  KJS::Object o=lookupobj.get(interpreter->globalExec(),func).toObject(interpreter->globalExec());
  if (interpreter->globalExec()->hadException())
  {
    errorMsg=interpreter->globalExec()->exception().toString(interpreter->globalExec()).qstring();
    kdDebug(13050)<<"Exception(1):"<<errorMsg<<endl;
    interpreter->globalExec()->clearException();
    return false;
  }

  // init doc & view with new pointers!
  docWrapper->doc = v->doc();
  viewWrapper->view = v;

  /*kdDebug(13050)<<"Call Object:"<<o.toString(interpreter->globalExec()).ascii()<<endl;*/
  o.call(interpreter->globalExec(),interpreter->globalObject(),params);
  if (interpreter->globalExec()->hadException())
  {
    errorMsg=interpreter->globalExec()->exception().toString(interpreter->globalExec()).ascii();
    kdDebug(13050)<<"Exception(2):"<<errorMsg<<endl;
    interpreter->globalExec()->clearException();
    return false;
  }
  return true;
}

bool KateIndentJScriptImpl::processChar(Kate::View *view, QChar c, QString &errorMsg )
{

  kdDebug(13050)<<"KateIndentJScriptImpl::processChar"<<endl;
  if (!setupInterpreter(errorMsg)) return false;
  KJS::List params;
  params.append(KJS::String(QString(c)));
  return KateIndentJScriptCall(view,errorMsg,m_docWrapper,m_viewWrapper,m_interpreter,*m_indenter,KJS::Identifier("onchar"),params);
}

bool KateIndentJScriptImpl::processLine(Kate::View *view, const KateDocCursor &line, QString &errorMsg )
{
  kdDebug(13050)<<"KateIndentJScriptImpl::processLine"<<endl;
  if (!setupInterpreter(errorMsg)) return false;
  return KateIndentJScriptCall(view,errorMsg,m_docWrapper,m_viewWrapper,m_interpreter,*m_indenter,KJS::Identifier("online"),KJS::List());
}

bool KateIndentJScriptImpl::processNewline( class Kate::View *view, const KateDocCursor &begin, bool needcontinue, QString &errorMsg )
{
  kdDebug(13050)<<"KateIndentJScriptImpl::processNewline"<<endl;
  if (!setupInterpreter(errorMsg)) return false;
  return KateIndentJScriptCall(view,errorMsg,m_docWrapper,m_viewWrapper,m_interpreter,*m_indenter,KJS::Identifier("onnewline"),KJS::List());
}
//END

//BEGIN KateIndentJScriptManager
KateIndentJScriptManager::KateIndentJScriptManager():KateIndentScriptManagerAbstract()
{
  m_scripts.setAutoDelete (true);
  collectScripts ();
}

KateIndentJScriptManager::~KateIndentJScriptManager ()
{
}

void KateIndentJScriptManager::collectScripts (bool force)
{
// If there's something in myModeList the Mode List was already built so, don't do it again
  if (!m_scripts.isEmpty())
    return;


  // We'll store the scripts list in this config
  KConfig config("katepartindentjscriptrc", false, false);
#if 0
  // figure out if the kate install is too new
  config.setGroup ("General");
  if (config.readNumEntry ("Version") > config.readNumEntry ("CachedVersion"))
  {
    config.writeEntry ("CachedVersion", config.readNumEntry ("Version"));
    force = true;
  }
#endif

  // Let's get a list of all the .js files
  QStringList list = KGlobal::dirs()->findAllResources("data","katepart/scripts/indent/*.js",false,true);

  // Let's iterate through the list and build the Mode List
  for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
  {
    // Each file has a group ed:
    QString Group="Cache "+ *it;

    // Let's go to this group
    config.setGroup(Group);

    // stat the file
    struct stat sbuf;
    memset (&sbuf, 0, sizeof(sbuf));
    stat(QFile::encodeName(*it), &sbuf);

    // If the group exist and we're not forced to read the .js file, let's build myModeList for katepartjscriptrc
    bool readnew=false;
    if (!force && config.hasGroup(Group) && (sbuf.st_mtime == config.readNumEntry("lastModified")))
    {
        config.setGroup(Group);
        QString filePath=*it;
        QString internalName=config.readEntry("internlName","KATE-ERROR");
        if (internalName=="KATE-ERROR") readnew=true;
        else
        {
          QString niceName=config.readEntry("niceName",internalName);
          QString copyright=config.readEntry("copyright",i18n("(Unknown)"));
          double  version=config.readDoubleNumEntry("version",0.0);
          KateIndentJScriptImpl *s=new KateIndentJScriptImpl(
            internalName,filePath,niceName,copyright,version);
          m_scripts.insert (internalName, s);
        }
    }
    else readnew=true;
    if (readnew)
    {
        QFileInfo fi (*it);

        if (m_scripts[fi.baseName()])
          continue;

        QString internalName=fi.baseName();
        QString filePath=*it;
        QString niceName=internalName;
        QString copyright=i18n("(Unknown)");
        double   version=0.0;
        parseScriptHeader(filePath,&niceName,&copyright,&version);
        /*save the information for retrieval*/
        config.setGroup(Group);
        config.writeEntry("lastModified",sbuf.st_mtime);
        config.writeEntry("internalName",internalName);
        config.writeEntry("niceName",niceName);
        config.writeEntry("copyright",copyright);
        config.writeEntry("version",version);
        KateIndentJScriptImpl *s=new KateIndentJScriptImpl(
          internalName,filePath,niceName,copyright,version);
        m_scripts.insert (internalName, s);
    }
  }

  // Syncronize with the file katepartjscriptrc
  config.sync();
}

KateIndentScript KateIndentJScriptManager::script(const QString &scriptname) {
  KateIndentJScriptImpl *s=m_scripts[scriptname];
  kdDebug(13050)<<scriptname<<"=="<<s<<endl;
  return KateIndentScript(s);
}

void KateIndentJScriptManager::parseScriptHeader(const QString &filePath,
        QString *niceName,QString *copyright,double *version)
{
  QFile f(QFile::encodeName(filePath));
  if (!f.open(IO_ReadOnly) ) {
    kdDebug(13050)<<"Header could not be parsed, because file could not be opened"<<endl;
    return;
  }
  QTextStream st(&f);
  st.setEncoding (QTextStream::UnicodeUTF8);
  if (!st.readLine().upper().startsWith("/**KATE")) {
    kdDebug(13050)<<"No header found"<<endl;
    f.close();
    return;
  }
  // here the real parsing begins
  kdDebug(13050)<<"Parsing indent script header"<<endl;
  enum {NOTHING=0,COPYRIGHT=1} currentState=NOTHING;
  QString line;
  QString tmpblockdata="";
  QRegExp endExpr("[\\s\\t]*\\*\\*\\/[\\s\\t]*$");
  QRegExp keyValue("[\\s\\t]*\\*\\s*(.+):(.*)$");
  QRegExp blockContent("[\\s\\t]*\\*(.*)$");
  while ((line=st.readLine())!=QString::null) {
    if (endExpr.exactMatch(line)) {
      kdDebug(13050)<<"end of config block"<<endl;
      if (currentState==NOTHING) break;
      if (currentState==COPYRIGHT) {
        *copyright=tmpblockdata;
        break;
      }
      Q_ASSERT(0);
    }
    if (currentState==NOTHING)
    {
      if (keyValue.exactMatch(line)) {
        QStringList sl=keyValue.capturedTexts();
        kdDebug(13050)<<"key:"<<sl[1]<<endl<<"value:"<<sl[2]<<endl;
        kdDebug(13050)<<"key-length:"<<sl[1].length()<<endl<<"value-length:"<<sl[2].length()<<endl;
        QString key=sl[1];
        QString value=sl[2];
        if (key=="NAME") (*niceName)=value.stripWhiteSpace();
        else if (key=="VERSION") (*version)=value.stripWhiteSpace().toDouble(0);
        else if (key=="COPYRIGHT")
        {
          tmpblockdata="";
          if (value.stripWhiteSpace().length()>0)  tmpblockdata=value;
          currentState=COPYRIGHT;
        } else kdDebug(13050)<<"ignoring key"<<endl;
      }
    } else {
      if (blockContent.exactMatch(line))
      {
        QString  bl=blockContent.capturedTexts()[1];
        //kdDebug(13050)<<"block content line:"<<bl<<endl<<bl.length()<<" "<<bl.isEmpty()<<endl;
        if (bl.isEmpty())
        {
          (*copyright)=tmpblockdata;
          kdDebug(13050)<<"Copyright block:"<<endl<<(*copyright)<<endl;
          currentState=NOTHING;
        } else tmpblockdata=tmpblockdata+"\n"+bl;
      }
    }
  }
  f.close();
}
//END
// kate: space-indent on; indent-width 2; replace-tabs on;
