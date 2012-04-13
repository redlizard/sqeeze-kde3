#ifndef __FONT_PREVIEW_H__
#define __FONT_PREVIEW_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : KFI::CFontPreview
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 04/11/2001
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
// (C) Craig Drummond, 2001, 2002, 2003, 2004
////////////////////////////////////////////////////////////////////////////////

#include <qstring.h>
#include <qpixmap.h>
#include <qsize.h>
#include <qwidget.h>
#include <qcolor.h>
#include <kurl.h>
#include "FcEngine.h"

namespace KFI
{

class CFontPreview : public QWidget
{
    Q_OBJECT

    public:

    CFontPreview(QWidget *parent, const char *name=NULL);
    virtual ~CFontPreview() {}

    void        paintEvent(QPaintEvent *);
    QSize       sizeHint() const;
    QSize       minimumSizeHint() const;

    void        showFont(const KURL &url);
    void        showFont();

    CFcEngine & engine() { return itsEngine; }

    public slots:

    void        showFace(int face);

    signals:

    void        status(bool);

    private:

    CFcEngine itsEngine;
    QPixmap   itsPixmap;
    KURL      itsCurrentUrl;
    int       itsCurrentFace,
              itsLastWidth,
              itsLastHeight;
    QColor    itsBgndCol;
    QString   itsFontName;
};

}

#endif
