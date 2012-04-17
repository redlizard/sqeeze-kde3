/***************************************************************************
 *   Copyright (C) 2004 by Jens Luetkens                                   *
 *   j.luetkens@limitland.de                                               *
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

#include <qsettings.h>
#include <kconfig.h>

#include "comix.h"
#include "comixClient.h"

using namespace COMIX;

//
// Constructor
//
ComixHandler::ComixHandler()
{

	m_borderSize 	= 5;
	m_penWidth		= 2;
	
	activeBorderColor = KDecoration::options()->color( ColorFrame, true );
	inactiveBorderColor = KDecoration::options()->color( ColorFrame, false );
	
	textMargin = 0;
	fontHeight = 0;
	boxHeight = 0;
	contrast = 0;
	
	borderCircle = NULL;
	borderCircle = new ComixCircle(m_borderSize,m_penWidth);
	
	for( int i = 0; i < NumPixmaps; i++ ) {
		pixmapBuffer[i] = NULL;
	}

	pixmapsDirty = true;
	createPixmaps();
	
	reset(1);
}

//
// Destructor
//
ComixHandler::~ComixHandler()
{
	delete borderCircle;
	
	destroyPixmaps();
	
	m_initialized = false;
}

//
// createDecoration( KDecorationBridge *b )
//
KDecoration* ComixHandler::createDecoration( KDecorationBridge *b )
{
    return new ComixClient(b, this);
}

/*
//
// supports( Ability ability )
//
bool ComixHandler::supports( Ability ability )
{
    switch( ability )
    {
        case AbilityAnnounceButtons:
        case AbilityButtonMenu:
        case AbilityButtonOnAllDesktops:
        case AbilityButtonSpacer:
        case AbilityButtonHelp:
        case AbilityButtonMinimize:
        case AbilityButtonMaximize:
        case AbilityButtonClose:
        case AbilityButtonAboveOthers:
        case AbilityButtonBelowOthers:
        case AbilityButtonShade:
        case AbilityButtonResize:
            return true;
        default:
            return false;
    };

}
*/

//
// reset(unsigned long changed)
//
bool ComixHandler::reset(unsigned long changed)
{

    m_initialized = false;
	
	// border sizese
    switch(KDecoration::options()->preferredBorderSize( this )) {
        case BorderTiny:
            m_borderSize = 3;
			m_penWidth = 2;
            break;
        case BorderLarge:
            m_borderSize = 8;
			m_penWidth = 3;
            break;
        case BorderVeryLarge:
            m_borderSize = 12;
			m_penWidth = 4;
            break;
        case BorderHuge:
            m_borderSize = 18;
			m_penWidth = 5;
            break;
        case BorderVeryHuge:
            m_borderSize = 26;
			m_penWidth = 6;
            break;
        case BorderOversized:
            m_borderSize = 40;
			m_penWidth = 7;
            break;
        case BorderNormal:
        default:
            m_borderSize = 5;
			m_penWidth = 2;
    }
	
	if( pixmapsDirty ) {
		destroyPixmaps();
		createPixmaps();
	}
	
    // reset all clients
    resetDecorations( changed );

    m_initialized = true;

    return true;
}

//
// readConfig()
//
void ComixHandler::readConfig()
{

	if( KDecoration::options()->color( ColorFrame, true ) != activeBorderColor ) {
		activeBorderColor = KDecoration::options()->color( ColorFrame, true );
		pixmapsDirty = true;
	}
	if( KDecoration::options()->color( ColorFrame, false ) != inactiveBorderColor ) {
		inactiveBorderColor = KDecoration::options()->color( ColorFrame, false );
		pixmapsDirty = true;
	}
	if( KDecoration::options()->color( ColorHandle, true ) != activeBackgroundColor ) {
		activeBackgroundColor = KDecoration::options()->color( ColorHandle, true );
		pixmapsDirty = true;
	}
	if( KDecoration::options()->color( ColorHandle, false ) != inactiveBackgroundColor ) {
		inactiveBackgroundColor = KDecoration::options()->color( ColorHandle, false );
		pixmapsDirty = true;
	}
	if( KDecoration::options()->color( ColorTitleBar, true ) != activeTitleColor ) {
		activeTitleColor = KDecoration::options()->color( ColorTitleBar, true );
		pixmapsDirty = true;
	}
	if( KDecoration::options()->color( ColorTitleBar, false ) != inactiveTitleColor ) {
		inactiveTitleColor = KDecoration::options()->color( ColorTitleBar, false );
		pixmapsDirty = true;
	}
	
	QSettings settings;
	int newcontrast = settings.readNumEntry("/Qt/KDE/contrast", 4);
	if( contrast != newcontrast ) {
		contrast = newcontrast;
		pixmapsDirty = true;
	}
	
	// geometry
	QFontMetrics fm(KDecoration::options()->font(true, false));
	textMargin = QMAX(3,fm.height() / 8);
	fontHeight = QMAX(8, fm.height());
	boxHeight = fontHeight + textMargin + textMargin;
	// must be a multiple of 2
	if (! ( boxHeight%2 == 0 ) )
		boxHeight += 1;
	
}


//
// createPixmaps()
//
void ComixHandler::createPixmaps()
{

	// config changed?
	readConfig();
	
	pixmapBuffer[ActiveBorder] = borderCircle->circlePixmap( activeBorderColor );
	pixmapBuffer[InactiveBorder] = borderCircle->circlePixmap( inactiveBorderColor );
	
	pixmapBuffer[ActiveButton] = new QPixmap(boxHeight,boxHeight);
	pixmapBuffer[InactiveButton] = new QPixmap(boxHeight,boxHeight);
	
	createButton(pixmapBuffer[ActiveButton], true);
	createButton(pixmapBuffer[InactiveButton], false);
	
	QWMatrix m;
	m.rotate(180);
	
	pixmapBuffer[SunkenButton] = new QPixmap(pixmapBuffer[ActiveButton]->xForm( m ));
	
	pixmapBuffer[InactiveSunkenButton] = new QPixmap(pixmapBuffer[InactiveButton]->xForm( m ));
	

}


//
// createButton(QPixmap *button, bool active)
//
void ComixHandler::createButton(QPixmap *button, bool active)
{

	QRect rect = button->rect();
	QColor titleColor = QColor();
	
	QPainter p( button );
	
	if( active )	p.setPen(activeBackgroundColor);
	else			p.setPen(inactiveBackgroundColor);

	p.drawRect(rect);
	rect.setRect(rect.x()+1,rect.y()+1,rect.width()-2,rect.height()-2);
	p.drawRect(rect);
	rect.setRect(rect.x()+1,rect.y()+1,rect.width()-2,rect.height()-2);
	
	
	if( active ) 	titleColor = activeTitleColor;
	else			titleColor = inactiveTitleColor;
	
	p.fillRect(rect, titleColor);

	int grad = 0;
	grad = (3 * contrast) + 100;
	p.setPen(titleColor.light(grad));
	p.drawLine(rect.left(),rect.top(),rect.right(),rect.top());
	p.setPen(titleColor.dark(grad));
	p.drawLine(rect.left(),rect.bottom(),rect.right(),rect.bottom());

	grad = (2 * contrast) + 100;
	p.setPen(titleColor.light(grad));
	p.drawLine(rect.left(),rect.top()+1,rect.right(),rect.top()+1);
	p.setPen(titleColor.dark(grad));
	p.drawLine(rect.left(),rect.bottom()-1,rect.right(),rect.bottom()-1);

	grad = (contrast) + 100;
	p.setPen(titleColor.light(grad));
	p.drawLine(rect.left(),rect.top()+2,rect.right(),rect.top()+2);
	p.setPen(titleColor.dark(grad));
	p.drawLine(rect.left(),rect.bottom()-2,rect.right(),rect.bottom()-2);
	
	if( active )	p.setPen(activeBorderColor);
	else			p.setPen(inactiveBorderColor);
	
	rect = button->rect();
	
	p.drawLine(rect.left()+5,rect.top(),rect.right()-5,rect.top());
	p.drawLine(rect.left()+5,rect.top()+1,rect.right()-5,rect.top()+1);
	p.drawLine(rect.left()+5,rect.bottom(),rect.right()-5,rect.bottom());
	p.drawLine(rect.left()+5,rect.bottom()-1,rect.right()-5,rect.bottom()-1);
	p.drawLine(rect.left(),rect.top()+5,rect.left(),rect.bottom()-5);
	p.drawLine(rect.left()+1,rect.top()+5,rect.left()+1,rect.bottom()-5);
	p.drawLine(rect.right(),rect.top()+5,rect.right(),rect.bottom()-5);
	p.drawLine(rect.right()-1,rect.top()+5,rect.right()-1,rect.bottom()-5);

	if( active ) {
		p.drawPixmap(rect.left(), rect.top(), *pixmapBuffer[ActiveBorder], 0, 0, 5, 5);
		p.drawPixmap(rect.right() - 4, rect.top(), *pixmapBuffer[ActiveBorder], 5, 0, 5, 5);
		p.drawPixmap(rect.left(), rect.bottom() - 4, *pixmapBuffer[ActiveBorder], 0, 5, 5, 5);
		p.drawPixmap(rect.right() - 5 + 1, rect.bottom() - 4, *pixmapBuffer[ActiveBorder], 5, 5, 5, 5);
	}
	else {
		p.drawPixmap(rect.left(), rect.top(), *pixmapBuffer[InactiveBorder], 0, 0, 5, 5);
		p.drawPixmap(rect.right() - 4, rect.top(), *pixmapBuffer[InactiveBorder], 5, 0, 5, 5);
		p.drawPixmap(rect.left(), rect.bottom() - 4, *pixmapBuffer[InactiveBorder], 0, 5, 5, 5);
		p.drawPixmap(rect.right() - 5 + 1, rect.bottom() - 4, *pixmapBuffer[InactiveBorder], 5, 5, 5, 5);
	}
	
	
	
}

//
// destroyPixmaps()
//
void ComixHandler::destroyPixmaps()
{
	for( int i = 0; i < NumPixmaps; i++ ) {
		if( pixmapBuffer[i] )
			delete pixmapBuffer[i];
	}
}


//
// borderSizes()
//
QValueList< ComixHandler::BorderSize >
ComixHandler::borderSizes() const
{
    // the list must be sorted
    return QValueList< BorderSize >() << BorderTiny << BorderNormal <<
		BorderLarge << BorderVeryLarge <<  BorderHuge <<
		BorderVeryHuge << BorderOversized;

}

//
// Plugin Stuff
//
extern "C"
{
    KDecorationFactory* create_factory()
    {
        return new ComixHandler();
    }
}

