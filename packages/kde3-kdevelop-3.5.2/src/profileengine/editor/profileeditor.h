/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo <adymo@kdevelop.org>             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef PROFILEEDITOR_H
#define PROFILEEDITOR_H

#include <profileengine.h>

#include "profileeditorbase.h"

class QListBoxItem;
class QListViewItem;
class KListViewItem;

class ProfileEditor : public ProfileEditorBase {
    Q_OBJECT
public:
    ProfileEditor(QWidget *parent = 0, const char *name = 0);
    
public slots:
    virtual void removeProperty();
    virtual void addProperty();
    virtual void removeProfile();
    virtual void addProfile();
    virtual void propertyExecuted(QListBoxItem *item);
    virtual void profileExecuted(QListViewItem *item);
    
    virtual void delDisabled();
    virtual void addDisabled();
    virtual void delEnabled();
    virtual void addEnabled();

    virtual void accept();

protected:
    void refresh();
    void refreshPropertyCombo();
    void refreshAvailableList();
    
    void fillPropertyList(Profile *profile);
    void fillEDLists(Profile *profile);
    void fillPluginsList(Profile *profile);
    
    Profile *currentProfile();
    
private:
    ProfileEngine engine;

    KListViewItem *allCore;
    KListViewItem *allGlobal;
    KListViewItem *allProject;    
};

#endif
