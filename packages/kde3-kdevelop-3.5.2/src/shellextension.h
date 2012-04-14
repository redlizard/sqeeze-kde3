/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
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
#ifndef SHELLEXTENSION_H
#define SHELLEXTENSION_H

#include <qstring.h>

class KDialogBase;

/**
Shell extension.
Provides application-dependent and shell-independent functionality.
Shell uses extensions to perform application dependent actions.
*/
class ShellExtension {
public:
    /**Returns an instance of a shell. Subclasses must create an instance of a shell
    by themselves. For example they could provide static init() method like:
    @code
    static void init()
    {
        s_instance = new MyExtension();
    }
    @endcode*/
    static ShellExtension *getInstance() { return s_instance; }
    
    /**Reimplement to create global settings page in the dialog @p dlg.*/
    virtual void createGlobalSettingsPage(KDialogBase *dlg) = 0;
    /**Reimplement to perform actions when the global settings page in the
    dialog @p dlg is accepted (user clicked "Ok").*/
    virtual void acceptGlobalSettingsPage(KDialogBase *dlg) = 0;
    
    /**Reimplement to return the name of KXMLGUI resource file for an application.*/
    virtual QString xmlFile() = 0;
    
    /**Reimplement to set a default profile for the shell. Default profile
    will be used by a shell if no --profile argument is specified.*/
    virtual QString defaultProfile() = 0;

protected:
    ShellExtension();
    static ShellExtension *s_instance;
};

#endif
