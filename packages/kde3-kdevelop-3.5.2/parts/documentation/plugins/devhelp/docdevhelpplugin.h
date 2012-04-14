/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef DOCDEVHELPPLUGIN_H
#define DOCDEVHELPPLUGIN_H

#include <qvaluestack.h>
#include <qdom.h>

#include <kdevdocumentationplugin.h>

class DocDevHelpPlugin: public DocumentationPlugin
{
    Q_OBJECT
public:
    DocDevHelpPlugin(QObject* parent, const char* name, const QStringList args = QStringList());
    ~DocDevHelpPlugin();

    virtual QString pluginName() const;

    virtual QString catalogTitle(const QString& url);
    virtual void setCatalogURL(DocumentationCatalogItem* item);
    
    virtual void createTOC(DocumentationCatalogItem* item);
    
    virtual DocumentationCatalogItem* createCatalog(KListView* contents, const QString& title, const QString& url);

    virtual bool needRefreshIndex(DocumentationCatalogItem* item);
    virtual void createIndex(IndexBox* index, DocumentationCatalogItem* item);
        
    virtual QStringList fullTextSearchLocations();
    virtual void autoSetupPlugin();
    
    virtual QPair<KFile::Mode, QString> catalogLocatorProps();

    virtual ProjectDocumentationPlugin *projectDocumentationPlugin(ProjectDocType type);
        
protected:
    void pushToScanStack(QValueStack<QString> &stack, const QString &value);
    void scanDevHelpDir(const QString &path);
    void addTocSect(DocumentationItem *parent, QDomElement childEl, QString baseUrl, bool book=false);
};

#endif
