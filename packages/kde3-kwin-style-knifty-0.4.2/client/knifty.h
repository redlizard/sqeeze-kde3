/***************************************************************************
*   Copyright (C) 2003 by Sandro Giessl                                   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/

#ifndef KNIFTY_H
#define KNIFTY_H

#include <kdecoration.h>
#include <kdecorationfactory.h>
#include <qfont.h>

namespace KNifty {

enum ButtonType {
    ButtonHelp=0,
    ButtonMax,
    ButtonMin,
    ButtonClose,
    ButtonMenu,
    ButtonOnAllDesktops,
    ButtonTypeCount
};

class KniftyHandler: public KDecorationFactory
{
public:
    KniftyHandler();
    ~KniftyHandler();
    virtual bool reset(unsigned long changed);
    virtual KDecoration *createDecoration(KDecorationBridge *b);

    static bool initialized() { return m_initialized; }

    static int  titleHeight() { return m_titleHeight; }
    static int  titleHeightTool() { return m_titleHeightTool; }
    static QFont titleFont() { return m_titleFont; }
    static QFont titleFontTool() { return m_titleFontTool; }
    static bool titleShadow() { return m_titleShadow; }
    static bool shrinkBorders() { return m_shrinkBorders; }
    static int  borderSize() { return m_borderSize; }
    static bool useTitleColor() { return m_useTitleColor; }
    static bool useHighContrastHoveredButtons() { return m_useHighContrastHoveredButtons; }
    static bool bordersBlendColor() { return m_bordersBlendColor; }
    static Qt::AlignmentFlags titleAlign() { return m_titleAlign; }
//     signals:
//         void softReset();
private:
    bool readConfig();

    static bool m_titleShadow;
    static bool m_shrinkBorders;
    static bool m_useTitleColor;
    static bool m_useHighContrastHoveredButtons;
    static bool m_bordersBlendColor;
    static int  m_borderSize;
    static int  m_titleHeight;
    static int  m_titleHeightTool;
    static QFont m_titleFont;
    static QFont m_titleFontTool;
    static Qt::AlignmentFlags m_titleAlign;

    static bool m_initialized;
};

// KniftyHandler* clientHandler();

} // namespace KNifty

#endif // KNIFT_H
