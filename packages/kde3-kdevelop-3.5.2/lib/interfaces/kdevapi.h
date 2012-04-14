/* This file is part of the KDE project
   Copyright (C) 2000-2001 Bernd Gehrmann <bernd@kdevelop.org>
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
#ifndef KDEVAPI_H
#define KDEVAPI_H

#include <qobject.h>

class QStringList;
class QDomDocument;
class KDevCore;
class KDevProject;
class KDevLanguageSupport;
class CodeModel;
class KDevPartController;
class KDevMainWindow;
class KDevCodeRepository;
class KDevPlugin;
class KDevPluginController;

/**
@file kdevapi.h
KDevelop API interface.
*/

/**
The interface to KDevelop's core components.
Needs to be implemented in a shell. Developers do not need to use this
class because @ref KDevPlugin already provides API convenience methods.
*/
class KDevApi: public QObject
{
    Q_OBJECT
public:
    /**Constructor.*/
    KDevApi();

    /**Destructor.*/
    virtual ~KDevApi();

    /**@return A reference to the toplevel widget.*/
    virtual KDevMainWindow *mainWindow() const = 0;

    /**@return A reference to the part controller which is used to manipulate loaded KParts.*/
    virtual KDevPartController *partController() const = 0;

    /**@return A reference to the plugin controller which is used to manipulate loaded plugin.*/
    virtual KDevPluginController *pluginController() const = 0;

    /**@return A reference to the application core - an object which provides
    basic functionalities for inter-parts communications / cooperation.*/
    virtual KDevCore *core() const = 0;

    /**@return A reference to the memory symbol store.*/
    virtual CodeModel *codeModel() const = 0;

    /**@return A reference to the DOM tree that represents the project file or 0 if no project is loaded.*/
    QDomDocument *projectDom() const;

    /**Sets the Document Object Model for the current project.
    @param dom The project DOM.*/
    void setProjectDom(QDomDocument *dom);

    /**@return A reference to the current project component or 0 if no project is loaded.*/
    KDevProject *project() const;

    /**Sets the current project.
    @param project The project plugin which becames the current project.*/
    void setProject(KDevProject *project);

    /**@return A reference to the language support component or 0 if no support available.*/
    KDevLanguageSupport *languageSupport() const;

    /**Sets the object charged of providing handling for the source files written in particular
    language (languages support component).
    @param languageSupport The language support plugin.*/
    void setLanguageSupport(KDevLanguageSupport *languageSupport);

    /**@return A reference to the code repository (accessor to persistent symbol stores).*/
    KDevCodeRepository *codeRepository() const;

private:
    class Private;
    Private *d;
};

#endif
