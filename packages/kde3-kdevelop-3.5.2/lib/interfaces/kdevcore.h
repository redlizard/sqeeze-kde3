/* This file is part of the KDE project
   Copyright (C) 2001-2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2001-2002 Bernd Gehrmann <bernd@kdevelop.org>
   Copyright (C) 2001 Sandy Meier <smeier@kdevelop.org>
   Copyright (C) 2002 Daniel Engelschalt <daniel.engelschalt@gmx.net>
   Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2002-2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2003 Mario Scalas <mario.scalas@libero.it>
   Copyright (C) 2003 Harald Fernengel <harry@kdevelop.org>
   Copyright (C) 2003 Hamish Rodda <rodda@kde.org>
   Copyright (C) 2004 Alexander Dymo <adymo@kdevelop.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef KDEVCORE_H
#define KDEVCORE_H

/**
@file kdevcore.h
The interface to the application core and context menu classes.
*/

#include <qstringlist.h>
#include <qdict.h>
#include <qobject.h>

#include <kurl.h>

class KDialogBase;
class KDevPlugin;
class CodeModelItem;
class ProjectModelItem;

namespace KParts
{
    class Part;
}

class QStatusBar;
class QPopupMenu;

/**
Base class for every context.
Think of a Context-based class as "useful
info associated to a context menu". Several context menu can be defined,
each defining different information: because of these context menus being
used in many modules, they are defined here.

When context menu with a certain "context" associated appears, KDevelop core 
sends a notification signal and all plugins which receive this signal have
the ability to add own items into the menu. For example, VCS plugin could
add "commit" and "update" menu items to the context menu of a file.

<b>How to use context from a plugin:</b>
-# Create a popup menu in context menu event handler: @code KPopupMenu menu(this); @endcode
-# Create a context: @code MyContext context(param). @endcode
-# Fill a context menu: @code core()->fillContextMenu(&menu, &context); @endcode
-# Show the popup menu: @code menu.exec(event->globalPos()); @endcode
.
In this example @em event is an object of QContextMenuEvent class which you have access
to if you reimplement QWidget::contextMenuEvent method.

<b>How to fill context menu from a plugin:</b>
-# Create a @code contextMenu(QPopupMenu *, const Context *) @endcode slot in your plugin class.
-# Connect KDevCore::contextMenu(QPopupMenu *, const Context *) signal to that slot in
the constructor of your plugin:\n 
@code
connect(core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
    this, SLOT(contextMenu(QPopupMenu *, const Context *)));
@endcode
-# Fill the menu in the slot you created, for example:\n
@code
if (context->hasType(Context::EditorContext))
{
    const EditorContext *econtext = static_cast<const EditorContext*>(context);
    int id = popup->insertItem(i18n("My Menu Item 1"), this, SLOT(myMenuAction1()));
    popup->setWhatsThis(id, i18n("What's this for my menu item 1"));
}
else if context->hasType(MyContext))
{
    int id = popup->insertItem(...
    ...
}
...
@endcode
*/
class Context
{
public:
    /**Pre-defined context types. More may be added so it is possible to add custom
    contexts. <strong>We reserve enum values until 1000 (yeah, it is one thousand )
    for kdevelop official context types.</strong>*/
    enum Type
    { 
        EditorContext,            /**<Editor context menu.*/
        DocumentationContext,     /**<Documentation browser context menu.*/
        FileContext,              /**<File context menu.*/
        ProjectModelItemContext,  /**<Project tree context menu.*/
        CodeModelItemContext      /**<Class tree context menu.*/
    };

    /**Implement this in the context so we can provide rtti.*/
    virtual int type() const = 0;

    /**@return The type of this Context, so clients can discriminate
    between different file contexts.*/
    virtual bool hasType(int type) const;

protected:
    /**Constructor.*/
    Context();

    /**Destructor.*/
    virtual ~Context();
};

/**A context for the popup menu in the editor.*/
class EditorContext: public Context
{
public:
    /**Builds a context for an editor part.
    @param url The url of a file in the editor.
    @param line The line number where the cursor is.
    @param col The column number where the cursor is.
    @param linestr The content of the line where the cursor is.
    @param wordstr The current word under the cursor.*/
    EditorContext(const KURL &url, int line, int col,
        const QString &linestr, const QString &wordstr);
    
    /**Destructor.*/
    virtual ~EditorContext();

    virtual int type() const;

    /**@return The url for the file which this context was invoked for.*/
    const KURL &url() const;

    /**@return The line number for the cursor position.*/
    int line() const;

    /**@return The column number for the cursor position.*/
    int col() const;

    /**@return A QString with the content of the line which this context was
    invoked for.*/
    QString currentLine() const;

    /**@return A QString containing the word near to the cursor when this
    context object was created.*/
    QString currentWord() const;

private:
    class Private;
    Private *d;

    EditorContext( const EditorContext &);
    EditorContext &operator=( const EditorContext &);
};


/**
A context for the popup menu in the documentation browser widget.
*/
class DocumentationContext: public Context
{
public:

    /**Builds a DocumentationContext.
    @param url The URL that the context will be for.
    @param selection Selected text.*/
    DocumentationContext(const QString &url, const QString &selection );

    /**Copy constructor.*/
    DocumentationContext(const DocumentationContext &);
    DocumentationContext &operator=(const DocumentationContext &);

    /**Destructor.*/
    virtual ~DocumentationContext();

    virtual int type() const;

    /**@return The url of the document this context was invoked for.*/
    QString url() const;

    /**@return The selected text in the document.*/
    QString selection() const;

private:
    class Private;
    Private *d;
};

/**
A context for the popup menu in file views and other parts that show files.
Context allows multiple selections of files.
*/
class FileContext : public Context
{
public:
    /**Builds the file context using a @ref KURL::List
    @param someURLs The list of selected files URLs.*/
    FileContext(const KURL::List &someURLs);
    
    /**Destructor.*/
    virtual ~FileContext();

    virtual int type() const;

    /**@return A reference to the selected of URLs.*/
    const KURL::List &urls() const;

private:
    class Private;
    Private *d;

    FileContext( const FileContext &);
    FileContext &operator=( const FileContext &);
};

/**
A context for the popup menu in class views.
*/
class CodeModelItemContext: public Context
{
public:
    /**Builds the context.
    @param item Selected code model item representation. Usually a symbol from the code
    like class, function, etc.*/
    CodeModelItemContext(const CodeModelItem* item);

    /**Destructor.*/
    virtual ~CodeModelItemContext();

    virtual int type() const;

    /**@return The code model item for the selected item.*/
    const CodeModelItem* item() const;

private:
    class Private;
    Private *d;

    CodeModelItemContext( const CodeModelItemContext &);
    CodeModelItemContext &operator=( const CodeModelItemContext &);
};

/**
A context for the popup menu in project views.
*/
class ProjectModelItemContext : public Context
{
public:
    /**Builds the context.
    @param item The item to build the context from.*/
    ProjectModelItemContext(const ProjectModelItem* item);

    /**Destructor.*/
    virtual ~ProjectModelItemContext();

    virtual int type() const;

    /**@return The code model item for the selected item.*/
    const ProjectModelItem* item() const;

private:
    class Private;
    Private *d;

    ProjectModelItemContext( const ProjectModelItemContext &);
    ProjectModelItemContext &operator=( const ProjectModelItemContext &);
};




/**
A KDevCore class defines an object which takes care about the cooperation
between the various plug-in which compose KDevelop.
It defines:
- signals that can be captured for menu customization;
- notifications about opening / closing projects;
- methods to access functionality of KDevelop core;
- requests to fill project and global settings widgets;
- etc.
.
*/
class KDevCore: public QObject
{
    Q_OBJECT
public:
    /**Constructor
    @param parent The QObject that's the parent of this class.
    @param name The name of the class.*/
    KDevCore(QObject *parent=0, const char *name=0);

    /**Destructor.*/
    virtual ~KDevCore();

    /**Fills the context menu.
    This method should be called by a part that wants to show a
    context menu. The parameter @p context should be filled with
    information about the context in which this happens (see
    EditorContext, DocumentationContext, ClassContext, ...).
    Essentially, this method emits the signal contextMenu()
    which other parts can use to hook in.
    @sa Context for a detailed explanation of context menu initializations and usage.
    @param popup The popup menu to fill.
    @param context The pointer to a Context object of this popup menu.*/
    virtual void fillContextMenu(QPopupMenu *popup, const Context *context) = 0;

    /**Closes the current project and open the new one. You cannot use the @ref KDevPlugin::project() 
    * method right after opening a new project, as it will return a null pointer.
    *You must wait for the eventloop to be reentered, so use a signleshot timer
    *to do the job needed after the project is opened or connect a slot to the
    *@ref projectOpened signal.
    * @param projectFileName The file name of the project to open.*/
    virtual void openProject(const QString& projectFileName) = 0;

    /**Marks the component as running (or not running). As long as at least one
    component is running, the stop button is enabled. When it is pressed,
    component get a stopButtonClicked(). This is usable for plugins which 
    run certain commands and want KDevelop core to be notified of that.
    If core is notified, it can allow the user to stop(interrupt) the command
    manually by means of stop button.
    @param which The plugin to mark.
    @param runs true if plugin is running something, false if it is not.*/
    virtual void running(KDevPlugin *which, bool runs) = 0;

signals:
    /**Emitted after the core has done all initializations and
    the main window has been shown.*/
    void coreInitialized();

    /**A project has been opened.*/
    void projectOpened();

    /**The project is about to be closed.*/
    void projectClosed();

    /**The language support part has been changed.*/
    void languageChanged();

    /**The user has clicked the stop button.
    If all actions should be cancelled, pass 0 to @p which
    @param which The KDevPlugin object to stop.*/
    void stopButtonClicked(KDevPlugin *which);

    /**A context menu has been requested somewhere. Components
    may hook some entries into it. More information on the
    context can be obtained by looking for the type of
    @p context and casting it accordingly.
    @sa Context for a detailed explanation of context menu initializations and usage.
    @param popupMenu The popup menu to fill.
    @param context The Context of this popup menu.*/
    void contextMenu(QPopupMenu *popupMenu, const Context *context);

    /**Expects that a configuration page for use in the
    KDevelop settings dialog is created by the component.
    The configuration page is not demand-loading, it will be created before
    global settings dialog is shown. Use @ref ConfigWidgetProxy in your plugin
    to create demand-loading configuration pages.
    @param dlg The dialog which the configuration widget should be added to.*/
    void configWidget(KDialogBase *dlg);

    /**Expects that a configuration page for use in the
    Project settings dialog is created by the component.
    The configuration page is not demand-loading, it will be created before
    project settings dialog is shown. Use @ref ConfigWidgetProxy in your plugin
    to create demand-loading configuration pages.
    @param dlg The dialog which the configuration widget should be added to.*/
    void projectConfigWidget(KDialogBase *dlg);
};

#endif
