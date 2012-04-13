#ifndef __FONT_VIEW_PART_H__
#define __FONT_VIEW_PART_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : KFI::CFontViewPart
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 03/08/2002
// Version       : $Revision: 465343 $ $Date: 2005-09-29 15:24:25 +0200 (Do, 29 Sep 2005) $
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
// (C) Craig Drummond, 2002, 2003, 2004
////////////////////////////////////////////////////////////////////////////////

#include <kparts/part.h>

class QPushButton;
class QFrame;
class QLabel;
class KIntNumInput;
class KAction;
class KURL;

namespace KFI
{

class CFontPreview;

class CFontViewPart : public KParts::ReadOnlyPart
{
    Q_OBJECT

    public:

    CFontViewPart(QWidget *parent=0, const char *name=0);
    virtual ~CFontViewPart() {}

    bool openURL(const KURL &url);

    protected:

    bool openFile();

    private slots:

    void previewStatus(bool st);
    void timeout();
    void install();
    void changeText();
    void print();

    private:

    CFontPreview  *itsPreview;
    QPushButton   *itsInstallButton;
    QFrame        *itsFrame,
                  *itsToolsFrame;
    QLabel        *itsFaceLabel;
    KIntNumInput  *itsFaceSelector;
    KAction       *itsChangeTextAction,
                  *itsPrintAction;
    bool          itsShowInstallButton;
    int           itsFace;
};

}

#endif
