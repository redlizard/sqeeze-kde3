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

#ifndef KWIN_COMIX_CLIENT_H
#define KWIN_COMIX_CLIENT_H

#include <qlayout.h>

#include "comix.h"
#include "comixCircle.h"

struct PixelMarks {
	int x;
	int y;
	int alpha;
};

class QBoxLayout;
class QSpacerItem;

namespace COMIX {

enum ButtonType
{
	ButtonHelp=0,
	ButtonOnAllDesktops,
	ButtonMenu,
	ButtonSeparator,
	ButtonIconify,
	ButtonMaximize,
	ButtonClose,
	ButtonShade,
	ButtonAbove,
	ButtonBelow,
	ButtonResize,
	ButtonCount
};

class ComixButton;

class ComixClient : public KDecoration
{

    Q_OBJECT

public:

	ComixClient(KDecorationBridge *b, KDecorationFactory *f);
	~ComixClient();

    virtual void	init(void);

	void			drawButton(QRect rect, QPainter * p, bool sunken);

protected:

    virtual void resizeEvent(QResizeEvent *e);
    virtual void paintEvent(QPaintEvent *e);
    virtual void showEvent(QShowEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *e);

    virtual void activeChange();
    virtual void desktopChange();
    virtual void maximizeChange ();
    virtual void iconChange();
    virtual void shadeChange ();
    virtual void captionChange ();
    virtual void resize(const QSize& size);
    virtual void borders (int &l, int &r, int &t, int &b) const;

    virtual bool isTool() const;
    virtual QSize minimumSize() const;
    virtual Position mousePosition(const QPoint &point) const;

private slots:

    void maxButtonPressed();
    void slotShade();
    void slotKeepAbove();
    void slotKeepBelow();
    void slotResize();

private:

    bool			readConfig();

    bool			eventFilter (QObject *obj, QEvent *e);

	void			createButtons(QBoxLayout *layout, const QString& buttons);
	void			createLayout();
	void			createPixmaps();
	void			deletePixmaps();
	void			repaintButtons();
	void			someGeometry();
	void			titleGeometry( bool forceUpdate );
	void			frameWindowRect(QRect rect, QPainter * p);
	void			updateCaptionBuffer( int boxWidth, int boxHeight );
	
	void			insetRect(QRect *rect, int inset);


	/* Configuration */
	Qt::AlignmentFlags			m_titleAlign;
	bool			m_titleExpand;
	bool			m_windowSeparator;
	int				m_contrast;
	QColor			aWindowBackgroundColor, iWindowBackgroundColor;
	QColor			aWindowContourColor, iWindowContourColor;
	
	/* window layout */
    QVBoxLayout *mainLayout_;
    QSpacerItem *titleSpacer_;

	/* Buttons */
	ComixButton		*m_buttons[ButtonCount];

	/* drawings */
	QPixmap			menuIcon;
	QPixmap			*activeCaption, *inactiveCaption;
	
	/* some geometry */
	int			titleWidth;
	int			textMargin;
	int			boxHeight;
	int			iconHeight;
	QColor		aTitleBarColor, iTitleBarColor;
	QColor		aTitleBlendColor, iTitleBlendColor;
	int			boxRadius;
	int			countButtons, countSpacer;
	int			borderSize, penWidth;
	QRect		iconRect;
	QRect		titleRect;

	bool		hasMenu;
	/*QPoint		resizeMouse, diffResizeMouse;*/
	
	ComixHandler	*clientHandler;
	
};

} // namespace COMIX

#endif
