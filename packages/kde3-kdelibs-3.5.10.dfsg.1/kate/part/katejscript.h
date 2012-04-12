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

#ifndef __kate_jscript_h__
#define __kate_jscript_h__

#include "../interfaces/document.h"
#include "kateindentscriptabstracts.h"
#include <qdict.h>
#include <kdebug.h>
/**
 * Some common stuff
 */
class KateDocument;
class KateView;
class QString;
class KateJSDocument;
class KateJSView;
class KateJSIndenter;
class KateDocCursor;

/**
 * Cool, this is all we need here
 */
namespace KJS {
  class Object;
  class ObjectImp;
  class Interpreter;
  class ExecState;
}

/**
 * Whole Kate Part scripting in one classs
 * Allow subclassing to allow specialized scripting engine for indenters
 */
class KateJScript
{
  public:
    /**
     * generate new global interpreter for part scripting
     */
    KateJScript ();

    /**
     * be destructive
     */
    virtual ~KateJScript ();

    /**
     * creates a JS wrapper object for given KateDocument
     * @param exec execution state, to find out interpreter to use
     * @param doc document object to wrap
     * @return new js wrapper object
     */
    KJS::ObjectImp *wrapDocument (KJS::ExecState *exec, KateDocument *doc);

    /**
     * creates a JS wrapper object for given KateView
     * @param exec execution state, to find out interpreter to use
     * @param view view object to wrap
     * @return new js wrapper object
     */
    KJS::ObjectImp *wrapView (KJS::ExecState *exec, KateView *view);

    /**
     * execute given script
     * the script will get the doc and view exposed via document and view object
     * in global scope
     * @param view view to expose
     * @param script source code of script to execute
     * @param errorMsg error to return if no success
     * @return success or not?
     */
    bool execute (KateView *view, const QString &script, QString &errorMsg);

  protected:
    /**
     * global object of interpreter
     */
    KJS::Object *m_global;

    /**
     * js interpreter
     */
    KJS::Interpreter *m_interpreter;

    /**
     * object for document
     */
    KJS::Object *m_document;

    /**
     * object for view
     */
    KJS::Object *m_view;
};

class KateJScriptManager : public Kate::Command
{
  private:
    /**
     * Internal used Script Representation
     */
    class Script
    {
      public:
        /**
         * get desktop filename
         * @return desktop filename
         */
        inline QString desktopFilename () { return filename.left(filename.length()-2).append ("desktop"); }

      public:
        /**
         * command name, as used for command line and more
         */
        QString name;

        /**
         * filename of the script
         */
        QString filename;

        /**
         * has it a desktop file?
         */
        bool desktopFileExists;
    };

  public:
    KateJScriptManager ();
    ~KateJScriptManager ();

  private:
    /**
     * go, search our scripts
     * @param force force cache updating?
     */
    void collectScripts (bool force = false);

  //
  // Here we deal with the Kate::Command stuff
  //
  public:
    /**
     * execute command
     * @param view view to use for execution
     * @param cmd cmd string
     * @param errorMsg error to return if no success
     * @return success
     */
    bool exec( class Kate::View *view, const QString &cmd, QString &errorMsg );

    /**
     * get help for a command
     * @param view view to use
     * @param cmd cmd name
     * @param msg help message
     * @return help available or not
     */
    bool help( class Kate::View *view, const QString &cmd, QString &msg );

    /**
     * supported commands as prefixes
     * @return prefix list
     */
    QStringList cmds();

  private:
    /**
     * we need to know somewhere which scripts are around
     */
    QDict<KateJScriptManager::Script> m_scripts;
};

class KateIndentJScriptImpl: public KateIndentScriptImplAbstract {
  public:
    KateIndentJScriptImpl(const QString& internalName,
        const QString  &filePath, const QString &niceName,
        const QString &copyright, double version);
    ~KateIndentJScriptImpl();
    
    virtual bool processChar( class Kate::View *view, QChar c, QString &errorMsg );
    virtual bool processLine( class Kate::View *view, const KateDocCursor &line, QString &errorMsg );
    virtual bool processNewline( class Kate::View *view, const KateDocCursor &begin, bool needcontinue, QString &errorMsg );
  protected:
    virtual void decRef();
  private:
    KateJSView *m_viewWrapper;
    KateJSDocument *m_docWrapper;
    KJS::Object *m_indenter;
    KJS::Interpreter *m_interpreter;
    bool setupInterpreter(QString &errorMsg);
    void deleteInterpreter();
};

class KateIndentJScriptManager: public KateIndentScriptManagerAbstract
{

  public:
    KateIndentJScriptManager ();
    virtual ~KateIndentJScriptManager ();
    virtual KateIndentScript script(const QString &scriptname);
  private:
    /**
     * go, search our scripts
     * @param force force cache updating?
     */
    void collectScripts (bool force = false);
    void parseScriptHeader(const QString &filePath,
        QString *niceName,QString *copyright,double *version);
    QDict<KateIndentJScriptImpl> m_scripts;
};

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
