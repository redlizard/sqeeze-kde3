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

#ifndef SUSE2_H
#define SUSE2_H

#include <qfont.h>

#include <kdecoration.h>
#include <kdecorationfactory.h>

namespace KWinSUSE2 {

enum ButtonStatus {
    ActiveUp = 0,
    ActiveDown,
    InactiveUp,
    InactiveDown,
    Shadow,
    NumButtonStatus
};

enum ButtonIcon {
    CloseIcon = 0,
    MaxIcon,
    MaxRestoreIcon,
    MinIcon,
    HelpIcon,
    OnAllDesktopsIcon,
    NotOnAllDesktopsIcon,
    KeepAboveIcon,
    NoKeepAboveIcon,
    KeepBelowIcon,
    NoKeepBelowIcon,
    ShadeIcon,
    UnShadeIcon,
    NumButtonIcons
};

enum ColorType {
    WindowContour = 0,
    TitleGradientFrom,
    TitleGradientTo,
    Border,
    TitleFont,
    BtnBg
};

class SUSE2Handler: public QObject, public KDecorationFactory
{
    Q_OBJECT
public:
    SUSE2Handler();
    ~SUSE2Handler();
    virtual bool reset( unsigned long changed );

    virtual KDecoration *createDecoration( KDecorationBridge * );
    virtual bool supports( Ability ability ) const;

    const KPixmap &buttonPixmap(ButtonIcon type, int size, ButtonStatus status);

    int  titleHeight() const { return m_titleHeight; }
    int  titleHeightTool() const { return m_titleHeightTool; }
    QFont titleFont() const { return m_titleFont; }
    QFont titleFontTool() const { return m_titleFontTool; }
    bool titleLogo() const { return m_titleLogo; }
    int titleLogoOffset() const { return m_titleLogoOffset; }
    QString titleLogoURL() const { return m_titleLogoURL; }
    bool titleShadow() const { return m_titleShadow; }
    int  borderSize() const { return m_borderSize; }
    int titlebarStyle() const { return m_titlebarStyle; }
    int buttonType() const { return m_buttonType; }
    bool customColors() const { return m_customColors; }
    bool useTitleProps() const { return m_useTitleProps; }
    bool animateButtons() const { return m_animateButtons; }
    bool redCloseButton() const { return m_redCloseButton; }
    float iconSize() const { return m_iconSize; }
    bool iconShadow() const { return m_iconShadow; }
    bool menuClose() const { return m_menuClose; }
    Qt::AlignmentFlags titleAlign() const { return m_titleAlign; }
    int roundCorners() const { return m_roundCorners; }
    bool reverseLayout() const { return m_reverse; }
    QColor getColor(KWinSUSE2::ColorType type, const bool active = true) const;
    QValueList< SUSE2Handler::BorderSize >  borderSizes() const;

private:
    void readConfig();

    bool m_titleLogo;
    bool m_titleShadow;
    bool m_shrinkBorders;
    int  m_buttonType;
    bool m_customColors;
    float m_iconSize;
    int m_titlebarStyle;
    bool m_customIconColors;
    QColor m_aFgColor;
    QColor m_aBgColor;
    QColor m_iFgColor;
    QColor m_iBgColor;
    bool m_iconShadow;
    bool m_animateButtons;
    bool m_redCloseButton;
    bool m_menuClose;
    bool m_reverse;
    int  m_borderSize;
    int  m_titleHeight;
    int  m_titleHeightTool;
    QFont m_titleFont;
    QFont m_titleFontTool;
    Qt::AlignmentFlags m_titleAlign;
    int m_roundCorners;
    int m_titleLogoOffset;
    QString m_titleLogoURL;
    bool m_useTitleProps;

    KPixmap *m_pixmaps[NumButtonStatus][NumButtonIcons];
};

SUSE2Handler* Handler();

} // KWinSUSE2

#endif

// kate: space-indent on; indent-width 4; replace-tabs on;
