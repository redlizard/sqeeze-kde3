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

#ifndef KWIN_COMIX_H
#define KWIN_COMIX_H

#include <qpainter.h>
#include <kdecoration.h>
#include <kdecorationfactory.h>

#include "comixCircle.h"

class ComixCircle;

namespace COMIX {

enum pixIDs { ActiveBorder = 0, InactiveBorder,
				ActiveButton, InactiveButton,
				SunkenButton, InactiveSunkenButton, 
				NumPixmaps
			};

class ComixButton;

class ComixHandler : public KDecorationFactory
{

public:

	ComixHandler();
	~ComixHandler();

    virtual					bool reset(unsigned long changed);
    virtual KDecoration*	createDecoration(KDecorationBridge *b);

/*
	// diabled for KDE 3.3 compatibility
    virtual bool			supports( Ability ability );
*/

    QValueList< ComixHandler::BorderSize >  borderSizes() const;
    int  			borderSize() { return m_borderSize; }
    int  			penWidth() { return m_penWidth; }
    int  			gTextMargin() { return textMargin; }
    int  			gFontHeight() { return fontHeight; }
    int  			gBoxHeight() { return boxHeight; }
    const 			QPixmap*	getPixmap(int pixId) const { return pixmapBuffer[pixId]; }

private:

	void			createPixmaps();
	void			destroyPixmaps();
	void			readConfig();
	void			createButton(QPixmap *button, bool active);

    bool			m_initialized;
	int				m_borderSize, m_penWidth;
	
    ComixCircle     *borderCircle;
	QPixmap			*pixmapBuffer[NumPixmaps];
	QColor			activeBorderColor, inactiveBorderColor;
	QColor			activeBackgroundColor, inactiveBackgroundColor;
	QColor			activeTitleColor, inactiveTitleColor;
	int				contrast;
	bool			pixmapsDirty;
	
	/* some geometry */
	int				textMargin;
	int				fontHeight;
	int				boxHeight;

};

} // namespace COMIX

#endif
