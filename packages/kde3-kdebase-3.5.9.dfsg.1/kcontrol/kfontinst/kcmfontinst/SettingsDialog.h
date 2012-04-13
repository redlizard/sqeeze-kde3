#ifndef __SETTINGS_DIALOG_H__
#define __SETTINGS_DIALOG_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : KFI::CSettingsDialog
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 10/05/2005
// Version       : $Revision$ $Date$
//
////////////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2005
////////////////////////////////////////////////////////////////////////////////

#include <kdialogbase.h>

class QCheckBox;

namespace KFI
{

class CSettingsDialog : public KDialogBase
{
    public:

    CSettingsDialog(QWidget *parent);

    private slots:

    void slotOk();

    private:

    QCheckBox *itsDoX,
              *itsDoGs;
};

}

#endif
