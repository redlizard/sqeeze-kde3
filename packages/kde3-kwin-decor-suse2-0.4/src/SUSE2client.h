/* SuSE KWin window decoration
  Copyright (C) 2005 Gerd Fleischer <gerdfleischer@web.de>
  Copyright (C) 2003 Sandro Giessl <ceebx@users.sourceforge.net>

  based on the window decoration "Web":
  Copyright (C) 2001 Rik Hemsley (rikkus) <rik@kde.org>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; see the file COPYING.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
  Boston, MA  02110-1301  USA.
*/

#ifndef SUSE2CLIENT_H
#define SUSE2CLIENT_H

#include <kcommondecoration.h>

#include "SUSE2.h"

namespace KWinSUSE2 {

// global constants
static const int TOPMARGIN       = 4;
static const int DECOHEIGHT      = 2;
static const int SIDETITLEMARGIN = 6;

class SUSE2Button;

class SUSE2Client : public KCommonDecoration
{
public:
    SUSE2Client(KDecorationBridge *bridge, KDecorationFactory *factory);
    ~SUSE2Client();

    // pure virtual methods from KCommonDecoration
    virtual QString visibleName() const;
    virtual QString defaultButtonsLeft() const;
    virtual QString defaultButtonsRight() const;
    virtual KCommonDecorationButton *createButton(ButtonType type);

    virtual bool decorationBehaviour(DecorationBehaviour behaviour) const;
    virtual int layoutMetric(LayoutMetric lm, bool respectWindowState = true, const KCommonDecorationButton * = 0) const;
    virtual QRegion cornerShape(WindowCorner corner);
    virtual void updateCaption();

    virtual void init();

    const QPixmap &getTitleBarTile(bool active) const
    {
        return active ? *aTitleBarTile : *iTitleBarTile;
    }

protected:
    virtual void paintEvent(QPaintEvent *e);
    virtual void reset( unsigned long changed );

private:
    QPixmap *aCaptionBuffer, *iCaptionBuffer;
    void update_captionBuffer();

    QPixmap *aTitleBarTile, *iTitleBarTile, *aTitleBarTopTile, *iTitleBarTopTile;
    QColor aGradientBottom, iGradientBottom, aAntialiasBase, iAntialiasBase;
    bool pixmaps_created;
    void create_pixmaps();
    void delete_pixmaps();

    bool captionBufferDirty;

    // settings...
    QFont s_titleFont;
};

} // KWinSUSE2

#endif
