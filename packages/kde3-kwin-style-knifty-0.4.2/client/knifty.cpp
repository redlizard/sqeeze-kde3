/***************************************************************************
*   Copyright (C) 2003 by Sandro Giessl                                   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/

#include <kconfig.h>

#include "knifty.h"
#include "kniftyclient.h"
#include "misc.h"

using namespace KNifty;

// static bool pixmaps_created = false;

bool KniftyHandler::m_initialized    = false;
bool KniftyHandler::m_useTitleColor  = false;
bool KniftyHandler::m_useHighContrastHoveredButtons = false;
bool KniftyHandler::m_bordersBlendColor = false;
bool KniftyHandler::m_titleShadow    = true;
bool KniftyHandler::m_shrinkBorders  = true;
int  KniftyHandler::m_borderSize     = 4;
int  KniftyHandler::m_titleHeight    = 19;
int  KniftyHandler::m_titleHeightTool= 12;
QFont KniftyHandler::m_titleFont = QFont();
QFont KniftyHandler::m_titleFontTool = QFont();
Qt::AlignmentFlags KniftyHandler::m_titleAlign = Qt::AlignHCenter;

KniftyHandler::KniftyHandler()
{
    reset(1);
}

KniftyHandler::~KniftyHandler()
{
    m_initialized = false;
}

KDecoration* KniftyHandler::createDecoration (KDecorationBridge *b)
{
    m_titleFont = options()->font(true, false);
    return new KniftyClient(b, this);
}

bool KniftyHandler::reset(unsigned long changed)
{
    m_titleFont = options()->font(true, false);
    m_titleFontTool = m_titleFont;
    m_titleFontTool.setPointSize(m_titleFontTool.pointSize() - 3); // Shrink font by 3pt
    m_titleFontTool.setWeight( QFont::Normal ); // and disable bold

    // read in the configuration
    m_initialized = false;
    readConfig();

    // reset all clients
    resetDecorations (changed);

    m_initialized = true;

    return true;
}

bool KniftyHandler::readConfig()
{
    // create a config object
    KConfig config("kwinkniftyrc");
    config.setGroup("General");

    // grab settings
    m_titleShadow    = config.readBoolEntry("TitleShadow", true);
    m_shrinkBorders  = config.readBoolEntry("ShrinkBorders", true);
    switch(KDecoration::options()->preferredBorderSize( this )) {
        case BorderTiny:
            m_borderSize = 2;
            break;
        case BorderLarge:
            m_borderSize = 6;
            break;
        case BorderVeryLarge:
            m_borderSize = 10;
            break;
        case BorderHuge:
            m_borderSize = 18;
            break;
        case BorderVeryHuge:
            m_borderSize = 27;
            break;
        case BorderOversized:
            m_borderSize = 40;
            break;
        case BorderNormal:
        default:
            m_borderSize = 3;
    }

    QFontMetrics fm(m_titleFont);  // active font = inactive font
    int titleHeightMin = 16;
    // The title should strech with bigger font sizes!
    m_titleHeight = QMAX(titleHeightMin, fm.height() + 4); // 4 px for the shadow etc.

    fm = QFontMetrics(m_titleFontTool);  // active font = inactive font
    int titleHeightToolMin = 13;
    // The title should strech with bigger font sizes!
    m_titleHeightTool = QMAX(titleHeightToolMin, fm.height() ); // don't care about the shadow etc.

    QString value = config.readEntry("TitleAlignment", "AlignHCenter");
    if (value == "AlignLeft")         m_titleAlign = Qt::AlignLeft;
    else if (value == "AlignHCenter") m_titleAlign = Qt::AlignHCenter;
    else if (value == "AlignRight")   m_titleAlign = Qt::AlignRight;

    m_useHighContrastHoveredButtons = config.readBoolEntry("UseHighContrastHoveredButtons", false);
    m_useTitleColor = config.readBoolEntry("UseTitleBarColor", false);
    m_bordersBlendColor = config.readBoolEntry("BordersBlendColor", false);

    return true;
}

//////////////////////////////////////////////////////////////////////////////
// Plugin Stuff                                                             //
//////////////////////////////////////////////////////////////////////////////

static KniftyHandler *handler = 0;

extern "C"
{
    KDecorationFactory* create_factory()
    {
        return new KniftyHandler();
    }
}

// KniftyHandler* clientHandler()
// {
//     return handler;
// }
