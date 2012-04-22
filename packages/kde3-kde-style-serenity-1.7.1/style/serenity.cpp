/*
	Serenity Style for KDE 3+
	
		Copyright (C) 2006 Remi Villatel <maxilys@tele2.fr>
	
	Based on Lipstik engine:
	
		Copyright (C) 2004 Patrice Tremblay <tremblaypatrice@yahoo.fr>
	
	Also based on numerous open source styles that helped me to not 
	re-invent the wheel for every line of code. Thanks to everyone.  ;-)
	
	This library is free software; you can redistribute it and/or 
	modify it under the terms of the GNU Library General Public 
	License version 2 as published by the Free Software Foundation.
	
	This library is distributed in the hope that it will be useful, 
	but WITHOUT ANY WARRANTY; without even the implied warranty of 
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
	Library General Public License for more details.
	
	You should have received a copy of the GNU Library General Public 
	License along with this library. (See the file COPYING.) If not, 
	write to:
	The Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
	Boston, MA 02111-1307, USA.
*/

#include <qapplication.h>
#include <qcheckbox.h>
#include <qcleanuphandler.h>
#include <qcombobox.h>
#include <qcursor.h>
#include <qgroupbox.h>
#include <qheader.h>
#include <qimage.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qmenubar.h>
#include <qobject.h>
#include <qobjectlist.h>
#include <qpainter.h>
#include <qpalette.h>
#include "qpixmapcache.h"
#include <qpopupmenu.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qregion.h>
#include <qscrollbar.h>
#include <qsettings.h>
#include <qsimplerichtext.h>
#include <qslider.h>
#include <qstatusbar.h>
#include <qstylefactory.h>
#include <qstyleplugin.h>
#include <qtabbar.h>
#include <qtabwidget.h>
#include <qtextedit.h>
#include <qtimer.h>
#include "private/qtitlebar_p.h"
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qvariant.h>
#include <qwidget.h>

#include <kanimwidget.h>
#include <kcolorbutton.h>
#include <kcolorcombo.h>
#include <kkeybutton.h>
#include <kruler.h>
#include <ktoolbar.h>

#include "serenity.h"
#include "serenity.moc"

/// Radio button stuff
static QBitmap radiobutton_mask;
static const uchar radiobutton_mask_bits[] = 
{	// 15x15
	0xf0, 0x07, 0xfc, 0x1f, 0xfe, 0x3f, 0xfe, 0x3f, 0xff, 0x7f, 0xff, 0x7f, 
	0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xfe, 0x3f, 
	0xfe, 0x3f, 0xfc, 0x1f, 0xf0, 0x07
};
static QBitmap radio_contour_rough;
static const uchar radio_contour_rough_bits[] = 
{	// 15x15
	0xe0, 0x03, 0x18, 0x0c, 0x04, 0x10, 0x02, 0x20, 0x02, 0x20, 0x01, 0x40, 
	0x01, 0x40, 0x01, 0x40, 0x01, 0x40, 0x01, 0x40, 0x02, 0x20, 0x02, 0x20, 
	0x04, 0x10, 0x18, 0x0c, 0xe0, 0x03
};
static QBitmap radio_outer_alias;
static const uchar radio_outer_alias_bits[] = 
{	// 15x15
	0x10, 0x04, 0x04, 0x10, 0x02, 0x20, 0x00, 0x00, 0x01, 0x40, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x40, 0x00, 0x00, 
	0x02, 0x20, 0x04, 0x10, 0x10, 0x04
};
static QBitmap radio_inner_alias;
static const uchar radio_inner_alias_bits[] = 
{	// 15x15
	0x00, 0x00, 0xe0, 0x03, 0x08, 0x08, 0x04, 0x10, 0x00, 0x00, 0x02, 0x20, 
	0x02, 0x20, 0x02, 0x20, 0x02, 0x20, 0x02, 0x20, 0x00, 0x00, 0x04, 0x10, 
	0x08, 0x08, 0xe0, 0x03, 0x00, 0x00
};

/// Radio mark
static QBitmap radiomark_rough;
static const uchar radiomark_rough_bits[] = 
{	// 7x7 @ (x+4, y+4)
	0x1c, 0x22, 0x41, 0x41, 0x41, 0x22, 0x1c
};
static QBitmap radiomark_alias;
static const uchar radiomark_alias_bits[] = 
{	// 7x7 @ (x+4, y+4)
	0x22, 0x55, 0x22, 0x00, 0x22, 0x55, 0x22
};

/// Check mark
static QBitmap checkmark_rough;
static const uchar checkmark_rough_bits[] = 
{
	0x00, 0x01, 0x80, 0x01, 0x80, 0x00, 0xc0, 0x00, 0x43, 0x00, 0x66, 0x00, 
	0x2c, 0x00, 0x38, 0x00, 0x10, 0x00
};
static QBitmap checkmark_alias;
static const uchar checkmark_alias_bits[] = 
{
	0x80, 0x00, 0x00, 0x00, 0x40, 0x01, 0x01, 0x00, 0xa0, 0x00, 0x01, 0x00, 
	0x52, 0x00, 0x04, 0x00, 0x28, 0x00
};

/// Tristate mark
static QBitmap tristate_rough;
static const uchar tristate_rough_bits[] = 
{
	0x38, 0x00, 0x10, 0x00, 0x00, 0x00, 0x01, 0x01, 0x83, 0x01, 0x01, 0x01,
	0x00, 0x00, 0x10, 0x00, 0x38, 0x00
};
static QBitmap tristate_alias;
static const uchar tristate_alias_bits[] = 
{
	0x44, 0x00, 0x28, 0x00, 0x11, 0x01, 0x82, 0x00, 0x44, 0x00, 0x82, 0x00, 
	0x11, 0x01, 0x28, 0x00, 0x44, 0x00
};

/// The big arrows
// const uint ARROWSIZE = 9;
static QBitmap bigUp;
static const uchar bigUp_bits[] =
{
	0x10, 0x00, 0x38, 0x00, 0x7c, 0x00, 0xfe, 0x00, 0xc7, 0x01, 0x82, 0x00
};
static QBitmap bigUp_alias;
static const uchar bigUp_alias_bits[] =
{
	0x28, 0x00, 0x44, 0x00, 0x82, 0x00, 0x01, 0x01, 0x28, 0x00, 0x45, 0x01
};
static QBitmap bigDown;
static const uchar bigDown_bits[] =
{
	0x82, 0x00, 0xc7, 0x01, 0xfe, 0x00, 0x7c, 0x00, 0x38, 0x00, 0x10, 0x00
};
static QBitmap bigDown_alias;
static const uchar bigDown_alias_bits[] =
{
	0x45, 0x01, 0x28, 0x00, 0x01, 0x01, 0x82, 0x00, 0x44, 0x00, 0x28, 0x00
};
static QBitmap bigLeft;
static const uchar bigLeft_bits[] =
{
	0x10, 0x38, 0x1c, 0x0e, 0x0f, 0x0e, 0x1c, 0x38, 0x10
};
static QBitmap bigLeft_alias;
static const uchar bigLeft_alias_bits[] =
{
	0x28, 0x04, 0x22, 0x11, 0x00, 0x11, 0x22, 0x04, 0x28
};
static QBitmap bigRight;
static const uchar bigRight_bits[] =
{
	0x02, 0x07, 0x0e, 0x1c, 0x3c, 0x1c, 0x0e, 0x07, 0x02
};
static QBitmap bigRight_alias;
static const uchar bigRight_alias_bits[] =
{
	0x05, 0x08, 0x11, 0x22, 0x00, 0x22, 0x11, 0x08, 0x05
};

/// Tree view arrow symbols
static QBitmap collapsed_rough;
static const uchar collapsed_rough_bits[] = 
{	// 7x9
	0x00, 0x06, 0x0c, 0x18, 0x30, 0x18, 0x0c, 0x06, 0x00
};
static QBitmap collapsed_alias;
static const uchar collapsed_alias_bits[] = 
{	// 7x9
	0x06, 0x09,0x12,0x24, 0x48, 0x24, 0x12, 0x09, 0x06
};
static QBitmap expanded_rough;
static const uchar expanded_rough_bits[] = 
{	// 9x7
	0x00, 0x00, 0x82, 0x00, 0xc6, 0x00, 0x6c, 0x00, 0x38, 0x00, 0x10, 0x00, 
	0x00, 0x00
};
static QBitmap expanded_alias;
static const uchar expanded_alias_bits[] = 
{	// 9x7
	0x82, 0x00, 0x45, 0x01, 0x29, 0x01, 0x92, 0x00, 0x44, 0x00, 0x28, 0x00, 
	0x10, 0x00
};
static QBitmap bigPlus;
static const uchar bigPlus_bits[] =
{	// 9x9
	0x00, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0xfe, 0x00,
	0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x00, 0x00
};
static QBitmap bigPlus_alias;
static const uchar bigPlus_alias_bits[] =
{	// 9x9
	0x10, 0x00, 0x28, 0x00, 0x28, 0x00, 0xee, 0x00, 0x01, 0x01,
	0xee, 0x00, 0x28, 0x00, 0x28, 0x00, 0x10, 0x00
};
static QBitmap bigMinus;
static const uchar bigMinus_bits[] =
{	// 9x3
	0x00, 0x00, 0xfe, 0x00, 0x00, 0x00
};
static QBitmap bigMinus_alias;
static const uchar bigMinus_alias_bits[] =
{	// 9x3
	0xfe, 0x00, 0x01, 0x01,	0xfe, 0x00
};

/// Checklist radiobutton stuff
static QBitmap tiny_radiobutton_mask;
static const uchar tiny_radiobutton_mask_bits[] = 
{	// 13x13
	0xf0, 0x01, 0xfc, 0x07, 0xfe, 0x0f, 0xfe, 0x0f, 0xff, 0x1f, 0xff, 0x1f, 
	0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xfe, 0x0f, 0xfe, 0x0f, 0xfc, 0x07, 
	0xf0, 0x01
};
static QBitmap tiny_radio_contour_rough;
static const uchar tiny_radio_contour_rough_bits[] = 
{	// 13x13
	0xe0, 0x00, 0x18, 0x03, 0x04, 0x04, 0x02, 0x08, 0x02, 0x08, 0x01, 0x10, 
	0x01, 0x10, 0x01, 0x10, 0x02, 0x08, 0x02, 0x08, 0x04, 0x04, 0x18, 0x03, 
	0xe0, 0x00
};
static QBitmap tiny_radio_outer_alias;
static const uchar tiny_radio_outer_alias_bits[] = 
{	// 13x13
	0x10, 0x01, 0x04, 0x04, 0x02, 0x08, 0x00, 0x00, 0x01, 0x10, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x01, 0x10, 0x00, 0x00, 0x02, 0x08, 0x04, 0x04, 
	0x10, 0x01
};
/// Checklist radio mark
static QBitmap tiny_radiomark_rough;
static const uchar tiny_radiomark_rough_bits[] = 
{	// 5x5 @ (x+4, y+4)
	0x0e, 0x11, 0x11, 0x11, 0x0e
};
static QBitmap tiny_radiomark_alias;
static const uchar tiny_radiomark_alias_bits[] = 
{	// 5x5 @ (x+4, y+4)
	0x11, 0x0a, 0x00, 0x0a, 0x11
};

/// The hard bitmaps (i.e. without aliasing)
//
static QBitmap littleUp;
static const uchar littleUp_bits[] =
{	// 7x5
	0x08, 0x1c, 0x3e, 0x77, 0x63
};
static QBitmap littleDown;
static const uchar littleDown_bits[] =
{	// 7x5
	0x63, 0x77, 0x3e, 0x1c, 0x08
};
static QBitmap littleLeft;
static const uchar littleLeft_bits[] =
{	// 5x7
	0x18, 0x1c, 0x0e, 0x07, 0x0e, 0x1c, 0x18
};
static QBitmap littleRight;
static const uchar littleRight_bits[] =
{	// 5x7
	0x03, 0x07, 0x0e, 0x1c, 0x0e, 0x07, 0x03
};
static QBitmap miniUp;
static const uchar miniUp_bits[] =
{	// 5x5
	0x04, 0x0e, 0x1b, 0x11, 0x00
};
static QBitmap miniDown;
static const uchar miniDown_bits[] =
{	// 5x5
	0x00, 0x11, 0x1b, 0x0e, 0x04
};
static QBitmap miniLeft;
static const uchar miniLeft_bits[] =
{	// 5x5
	0x0c, 0x06, 0x03, 0x06, 0x0c
};
static QBitmap miniRight;
static const uchar miniRight_bits[] =
{	// 5x5
	0x06, 0x0c, 0x18, 0x0c, 0x06
};
static QBitmap toolPlus;
static const uchar toolPlus_bits[] =
{	// 8x8
	0x18, 0x18, 0x18, 0xff, 0xff, 0x18, 0x18, 0x18
};
static QBitmap appletMenu;
static const uchar appletMenu_bits[] =
{	// 6x6
	0x0c, 0x0c, 0x3f, 0x3f, 0x0c, 0x0c
};
static QBitmap dockClose;
static const uchar dockClose_bits[] =
{	// 8x8
	0xc3, 0xe7, 0x7e, 0x3c, 0x3c, 0x7e, 0xe7, 0xc3
};
static QBitmap dockIn;
static const uchar dockIn_bits[] =
{	// 9x8
	0x02, 0x00, 0x07, 0x00, 0xce, 0x00, 0xdc, 0x00, 
	0xf8, 0x00, 0xf0, 0x00, 0xfc, 0x00, 0xfc, 0x00
};
static QBitmap dockOut;
static const uchar dockOut_bits[] =
{	// 9x8
	0x3f, 0x00, 0x3f, 0x00, 0x0f, 0x00, 0x1f, 0x00, 
	0x3b, 0x00, 0x73, 0x00, 0xe0, 0x00, 0x40, 0x00
};
static QBitmap dockStay;
static const uchar dockStay_bits[] =
{	// 8x8
	0xdb, 0xdb, 0x00, 0xc3, 0xc3, 0x00, 0xdb, 0xdb
};

/// Modern tree view
static QBitmap modLeftDeco;
static const uchar modLeftDeco_bits[] =
{	// 4x11
	0x04, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x04
};
static QBitmap modCollapsed;
static const uchar modCollapsed_bits[] =
{	// 11x11
	0x30, 0x00, 0x70, 0x00, 0xe0, 0x00, 0xdc, 0x01, 0xbe, 0x03, 0x3e, 0x07, 
	0xbe, 0x03, 0xdc, 0x01, 0xe0, 0x00, 0x70, 0x00, 0x30, 0x00
};
static QBitmap modTopDeco;
static const uchar modTopDeco_bits[] =
{	// 11x4
	0x00, 0x00, 0xfe, 0x03, 0xff, 0x07, 0xfe, 0x03
};
static QBitmap modExpanded;
static const uchar modExpanded_bits[] =
{	// 11x11
	0x00, 0x00, 0x70, 0x00, 0xf8, 0x00, 0xf8, 0x00, 0xfb, 0x06, 0x77, 0x07, 
	0x8e, 0x03, 0xdc, 0x01, 0xf8, 0x00, 0x70, 0x00, 0x20, 0x00
};
static QBitmap modDot;
static const uchar modDot_bits[] =
{	// 5x5
	0x0e, 0x1f, 0x1f, 0x1f, 0x0e
};

/// Some KDE "hard-coded" bitmaps
static const char* dock_window_close_xpm[] =
{
	"8 8 2 1",
	"# c #000000",
	". c None",
	"##....##",
	"###..###",
	".######.",
	"..####..",
	"..####..",
	".######.",
	"###..###",
	"##....##"
};
static const char* serene_close_xpm[] =
{
	"12 12 2 1",
	"# c #000000",
	". c None",
	"............",
	".##......##.",
	".###....###.",
	"..###..###..",
	"...######...",
	"....####....",
	"....####....",
	"...######...",
	"..###..###..",
	".###....###.",
	".##......##.",
	"............"
};
static const char* serene_maximize_xpm[]=
{
	"12 12 2 1",
	"# c #000000",
	". c None",
	"............",
	".##########.",
	".##########.",
	".##########.",
	".#........#.",
	".#........#.",
	".#........#.",
	".#........#.",
	".#........#.",
	".#........#.",
	".##########.",
	"............"
};
static const char* serene_minimize_xpm[] = 
{
	"12 12 2 1",
	"# c #000000",
	". c None",
	"............",
	"............",
	"............",
	"............",
	"............",
	"............",
	"............",
	"..########..",
	"..########..",
	"..########..",
	"............",
	"............"
};
static const char* serene_restore_xpm[] =
{
	"12 12 2 1",
	"# c #000000",
	". c None",
	"............",
	"...########.",
	"...########.",
	"...#......#.",
	".#######..#.",
	".#######..#.",
	".#.....#..#.",
	".#.....####.",
	".#.....#....",
	".#.....#....",
	".#######....",
	"............"
};
static const char* serene_shade_xpm[] = 
{
	"12 12 2 1",
	"# c #000000",
	". c None",
	"............",
	"............",
	"............",
	"............",
	".....##.....",
	"....####....",
	"...######...",
	"..########..",
	"............",
	"............",
	"............",
	"............"
};
static const char* serene_unshade_xpm[] = 
{
	"12 12 2 1",
	"# c #000000",
	". c None",
	"............",
	"............",
	"............",
	"............",
	"..########..",
	"...######...",
	"....####....",
	".....##.....",
	"............",
	"............",
	"............",
	"............"
};

// Popupmenu item constants...
static const int itemHMargin = 6;
static const int itemFrame = 2;
static const int arrowHMargin = 6;
static const int rightBorder = 12;

// Some handy constants
const int SRN_SHADOW = 125;	// Tab shadow
const int SRN_DASH = 135;	// Separators in scrollbars and tabbars
const int SRN_DOWN = 144;	// Sunken effect
const int SRN_SPLIT = 160;	// Separators headers
int SRN_OVER = 192;		/// Hover effect. NOT a constant.
const int SRN_MIX = 216;	//
const int SRN_LIGHT = 232;	// Unmarked panels
//
const bool TOP = true;		// Some handy values to avoid
const bool BOTTOM = false;	// criptic true/false parameters.
const bool CONCAVE = true;
const bool RAISED = true;
const bool HORIZONTAL = true;
const bool VERTICAL = false;
const bool THICK = true;
const bool THIN = false;
const bool SUNKEN = true;
const bool HOVERED = true;
const bool ENABLED = true;
const bool ALPHA = true;
//
// ScrollBar Style Tuning Flag (according to nbr of arrows and their location)
int SBSTF = 1;	// ThreeButtonScrollBar

/// MXLS: Serene auto-calculated shape
class SereneShape
{
	int sTop;
	int sTopX1;
	int sTopX2;
	int sLeft;
	int sLeftY1;
	int sLeftY2;
	int sRight;
	int sRightY1;
	int sRightY2;
	int sBottom;
	int sBottomX1;
	int sBottomX2;
	int sCornerPixels;
	QPoint sCorner[4];
	//
	int sAliasedPixels;
	QPoint sAliased[8];
	//
	int sSolidPixels;
	QPoint sSolid[4];
	//
	bool sRoundTopLeft;
	int sCornerTopLeftIndex;
	int sAliasedTopLeftIndex;
	int sSolidTopLeftIndex;
	//
	bool sRoundTopRight;
	int sCornerTopRightIndex;
	int sAliasedTopRightIndex;
	int sSolidTopRightIndex;
	//
	bool sRoundBottomLeft;
	int sCornerBottomLeftIndex;
	int sAliasedBottomLeftIndex;
	int sSolidBottomLeftIndex;
	//
	bool sRoundBottomRight;
	int sCornerBottomRightIndex;
	int sAliasedBottomRightIndex;
	int sSolidBottomRightIndex;
	
	public:
		SereneShape(QRect shapeRect, uint shapeFlags);
		~SereneShape() {}
		//
		// The four border lines
		QPoint topLeft() const { return QPoint(sTopX1, sTop); }
		QPoint topRight() const { return QPoint(sTopX2, sTop); }
		QPoint leftTop() const { return QPoint(sLeft, sLeftY1); }
		QPoint leftBottom() const { return QPoint(sLeft, sLeftY2); }
		QPoint rightTop() const { return QPoint(sRight, sRightY1); }
		QPoint rightBottom() const { return QPoint(sRight, sRightY2); }
		QPoint bottomLeft() const { return QPoint(sBottomX1, sBottom); }
		QPoint bottomRight() const { return QPoint(sBottomX2, sBottom); }
		// The four lines as QRect's to draw gradients
		QRect topBorder() { return QRect(sTopX1, sTop, sTopX2-sTopX1+1, 1); }
		QRect leftBorder() { return QRect(sLeft, sLeftY1, 1, sLeftY2-sLeftY1+1); }
		QRect rightBorder() { return QRect(sRight, sRightY1, 1, sRightY2-sRightY1+1); }
		QRect bottomBorder() { return QRect(sBottomX1, sBottom, sBottomX2-sBottomX1+1, 1); }
		// Corners
		int cornerPixels() const { return sCornerPixels; }
		QPoint cornerPoint(int n) const { return sCorner[n-1]; }
		//
		// Aliased corners
		int aliasedPixels() const { return sAliasedPixels; }
		QPoint aliasedPoint(int n) const { return sAliased[n-1]; }
		//
		// Background pixels in the corners, if not alphablend
		int solidPixels() const { return sSolidPixels; }
		QPoint solidPoint(int n) const { return sSolid[n-1]; }
		//
		// To get each corner
		bool roundTopLeft() const { return sRoundTopLeft; }
		QPoint cornerTopLeftPoint() const { return sCorner[sCornerTopLeftIndex]; }
		QPoint aliasedTopLeftPoint(int n) const { return sAliased[sAliasedTopLeftIndex + n-1]; }
		QPoint solidTopLeftPoint() const { return sSolid[sSolidTopLeftIndex]; }
		//
		bool roundTopRight() const { return sRoundTopRight; }
		QPoint cornerTopRightPoint() const { return sCorner[sCornerTopRightIndex]; }
		QPoint aliasedTopRightPoint(int n) const { return sAliased[sAliasedTopRightIndex + n-1]; }
		QPoint solidTopRightPoint() const { return sSolid[sSolidTopRightIndex]; }
		//
		bool roundBottomLeft() const { return sRoundBottomLeft; }
		QPoint cornerBottomLeftPoint() const { return sCorner[sCornerBottomLeftIndex]; }
		QPoint aliasedBottomLeftPoint(int n) const { return sAliased[sAliasedBottomLeftIndex + n-1]; }
		QPoint solidBottomLeftPoint() const { return sSolid[sSolidBottomLeftIndex]; }
		//
		bool roundBottomRight() const { return sRoundBottomRight; }
		QPoint cornerBottomRightPoint() const { return sCorner[sCornerBottomRightIndex]; }
		QPoint aliasedBottomRightPoint(int n) const { return sAliased[sAliasedBottomRightIndex + n-1]; }
		QPoint solidBottomRightPoint() const { return sSolid[sSolidBottomRightIndex]; }
};
// Initiator
SereneShape::SereneShape(QRect rect, uint flags)
{
	rect.coords(&sLeft, &sTop, &sRight, &sBottom);
	//
	sTopX1 = sLeft+1;
	sTopX2 = sRight-1;
	//
	sLeftY1 = sTop+1;
	sLeftY2 = sBottom-1;
	//
	sRightY1 = sTop+1;
	sRightY2 = sBottom-1;
	//
	sBottomX1 = sLeft+1;
	sBottomX2 = sRight-1;
	//
	sCornerPixels = 0;
	sAliasedPixels = 0;
	sSolidPixels = 0;
	//
	if (flags & Round_UpperLeft)
	{
		sRoundTopLeft = true;
		sCornerTopLeftIndex = sCornerPixels;
		sCorner[sCornerPixels] = QPoint(sTopX1, sLeftY1);
		sCornerPixels++;
		sAliasedTopLeftIndex = sAliasedPixels;
		sAliased[sAliasedPixels] = QPoint(sTopX1, sTop);
		sAliasedPixels++;
		sAliased[sAliasedPixels] = QPoint(sLeft, sLeftY1);
		sSolidTopLeftIndex = sSolidPixels;
		sSolid[sSolidPixels] = QPoint(sLeft, sTop);
		sSolidPixels++;
		sTopX1++;
		sLeftY1++;
	}
	else
	{
		sRoundTopLeft = false;
		sAliasedTopLeftIndex = sAliasedPixels;
		sAliased[sAliasedPixels] = QPoint(sLeft, sTop);
	}
	sAliasedPixels++;
	if (flags & Round_UpperRight)
	{
		sRoundTopRight = true;
		sCornerTopRightIndex = sCornerPixels;
		sCorner[sCornerPixels] = QPoint(sTopX2, sRightY1);
		sCornerPixels++;
		sAliasedTopRightIndex = sAliasedPixels;
		sAliased[sAliasedPixels] = QPoint(sTopX2, sTop);
		sAliasedPixels++;
		sAliased[sAliasedPixels] = QPoint(sRight, sRightY1);
		sSolidTopRightIndex = sSolidPixels;
		sSolid[sSolidPixels] = QPoint(sRight, sTop);
		sSolidPixels++;
		sTopX2--;
		sRightY1++;
	}
	else
	{
		sRoundTopRight = false;
		sAliasedTopRightIndex = sAliasedPixels;
		sAliased[sAliasedPixels] = QPoint(sRight, sTop);
	}
	sAliasedPixels++;
	if (flags & Round_BottomLeft) 
	{
		sRoundBottomLeft = true;
		sCornerBottomLeftIndex = sCornerPixels;
		sCorner[sCornerPixels] = QPoint(sBottomX1, sLeftY2);
		sCornerPixels++;
		sAliasedBottomLeftIndex = sAliasedPixels;
		sAliased[sAliasedPixels] = QPoint(sBottomX1, sBottom);
		sAliasedPixels++;
		sAliased[sAliasedPixels] = QPoint(sLeft, sLeftY2);
		sSolidBottomLeftIndex = sSolidPixels;
		sSolid[sSolidPixels] = QPoint(sLeft, sBottom);
		sSolidPixels++;
		sBottomX1++;
		sLeftY2--;
	}
	else
	{
		sRoundBottomLeft = false;
		sAliasedBottomLeftIndex = sAliasedPixels;
		sAliased[sAliasedPixels] = QPoint(sLeft, sBottom);
	}
	sAliasedPixels++;
	if (flags & Round_BottomRight)
	{
		sRoundBottomRight = true;
		sCornerBottomRightIndex = sCornerPixels;
		sCorner[sCornerPixels] = QPoint(sBottomX2, sRightY2);
		sCornerPixels++;
		sAliasedBottomRightIndex = sAliasedPixels;
		sAliased[sAliasedPixels] = QPoint(sBottomX2, sBottom);
		sAliasedPixels++;
		sAliased[sAliasedPixels] = QPoint(sRight, sRightY2);
		sSolidBottomRightIndex = sSolidPixels;
		sSolid[sSolidPixels] = QPoint(sRight, sBottom);
		sSolidPixels++;
		sBottomX2--;
		sRightY2--;
	}
	else
	{
		sRoundBottomRight = false;
		sAliasedBottomRightIndex = sAliasedPixels;
		sAliased[sAliasedPixels] = QPoint(sRight, sBottom);
	}
	sAliasedPixels++;
	//
	if (flags & Draw_AlphaBlend)
	{
		sSolidPixels = 0;
	}
}
/// ----- -----

/// Serene classes to access protected members
class SereneWidget : public QWidget
{
	friend class SerenityStyle;
	public:
		SereneWidget(QWidget* parent=0, const char* name=0)
		: QWidget(parent, name) {}
};

class SereneFrame : public QFrame
{
	friend class SerenityStyle;
	public:
		SereneFrame(QFrame* parent=0, const char* name=0)
		: QFrame(parent, name) {}
};

// QWhatsThat a.k.a. QWhatsThis
class SereneWhatsThat : public QWidget
{
	friend class SerenityStyle;
	public:
		SereneWhatsThat(): QWidget() {}
		QString text;
};
/// ----- -----

/// ----- Style Plugin Interface -----
class SerenityStylePlugin : public QStylePlugin
{
	public:
	SerenityStylePlugin() {}
	~SerenityStylePlugin() {}

	QStringList keys() const 
	{
		return QStringList() << "Serenity";
	}

	QStyle* create( const QString& key ) 
	{
		if (key.lower() == "serenity")
			return new SerenityStyle;
		return 0;
	}
};

Q_EXPORT_PLUGIN( SerenityStylePlugin )
/// ----- -----

SerenityStyle::SerenityStyle() : KStyle( AllowMenuTransparency, 
					ThreeButtonScrollBar), 
					kickerMode(false), kornMode(false), 
					kateMode(false), kontactMode(false)
{
	hoverWidget = 0;
	hoverTab = 0;
	hoverToolTab = false;
	hoverThis = HOVER_NONE;

	QSettings settings;
	_contrast = settings.readNumEntry("/Qt/KDE/contrast", 5);
	if (_contrast<0 || _contrast>10)
		_contrast = 5;
	LO_FACTOR = _contrast*3;
	MED_FACTOR = _contrast*4;
	HI_FACTOR = _contrast*5;
	ADJUSTOR = 50;

	settings.beginGroup("/serenitystyle/Settings");
	_activeTabStyle = limitedTo(0, 3, settings.readNumEntry("/activeTabStyle", 0));
	_alternateSinking = settings.readBoolEntry("/alternateSunkenEffect", false);
	_animateProgressBar = settings.readBoolEntry("/animateProgressBar", true);
	_centerTabs = settings.readBoolEntry("/centerTabs", false);
	_customGlobalColor = settings.readBoolEntry("/customGlobalColor", false);
	_customOverHighlightColor = settings.readBoolEntry("/customOverHighlightColor", false);
	_drawFocusRect = settings.readBoolEntry("/drawFocusRect", true);
	_drawToolBarHandle = settings.readBoolEntry("/drawToolBarHandle", true);
	_drawToolBarSeparator = settings.readBoolEntry("/drawToolBarSeparator", false);
	_drawTriangularExpander = settings.readBoolEntry("/drawTriangularExpander", true);
	_flatProgressBar = settings.readBoolEntry("/flatProgressBar", false);
	_globalColor.setNamedColor( settings.readEntry("/globalColor", "#800000") );
	_globalStyle = limitedTo(0, 2, settings.readNumEntry("/globalStyle", 1));
	_hiliteRubber = settings.readBoolEntry("/highlightedRubberband", false);
	_menubarHack = settings.readBoolEntry("/menubarHack", false);
	_menuGrooveStyle = limitedTo(0, 5, settings.readNumEntry("/menuGrooveStyle", 0));
	//_mouseOverLabel = settings.readBoolEntry("/mouseOverLabel", true);
	_overHighlightColor.setNamedColor( settings.readEntry("/overHighlightColor", "#00d000") );
	_passiveTabStyle = limitedTo(0, 3, settings.readNumEntry("/passiveTabStyle", 0));
	_progressPattern = limitedTo(0, 3, settings.readNumEntry("/progressBarPattern", 0));
	_purerHover = settings.readBoolEntry("/purerHover", false);
	_resizerStyle = limitedTo(0, 2, settings.readNumEntry("/resizerStyle", 0));
	_scrollBarStyle = limitedTo(0, 4, settings.readNumEntry("/scrollBarStyle", 0));
	_scrollerScheme = limitedTo(0, 2, settings.readNumEntry("/scrollerScheme", 0));
	_splitTabs = settings.readBoolEntry("/splitTabs", true);
	_subArrowFrame = settings.readBoolEntry("/submenuArrowFrame", false);
	_submenuSpeed = limitedTo(-16, 16, settings.readNumEntry("/submenuSpeed", 0));
	_tabWarnType = limitedTo(0, 3, settings.readNumEntry("/tabWarningStyle", 0));
	_textboxHilite = settings.readBoolEntry("/textboxFocusHighlight", true);
	_tipTint = limitedTo(0, 7, settings.readNumEntry("/tipTint", 0));
	_treeViewStyle = limitedTo(0, 3, settings.readNumEntry("/treeViewStyle", 0));
	_wideSlider = settings.readBoolEntry("/wideSlider", false);

	settings.endGroup();

	if (! _customGlobalColor)
	{
		/// MXLS: If we have none, create one. We may need it.
		QPalette pal = QApplication::palette();
		_globalColor = colorMix(
					colorMix(pal.color(QPalette::Active, QColorGroup::Background), 
						pal.color(QPalette::Active, QColorGroup::Highlight)), 
					pal.color(QPalette::Active, QColorGroup::Foreground));
	}

	// SBSTF = ScrollBar Style Tuning Flag.
	if (_scrollBarStyle == 4)
	{
		this->setScrollBarType(KStyle::WindowsStyleScrollBar);
		SBSTF = 2;
	}
	else if (_scrollBarStyle == 3)
	{
		this->setScrollBarType(KStyle::PlatinumStyleScrollBar);
		SBSTF = 0;
	}
	else if (_scrollBarStyle == 2)
	{
		this->setScrollBarType(KStyle::NextStyleScrollBar);
		SBSTF = 0;
	}
	else if (_scrollBarStyle == 1)	// Arrowless
	{
		this->setScrollBarType(KStyle::ThreeButtonScrollBar); // Whatever, OO is still broken!
		SBSTF = -1;
	}
	else
	{
		this->setScrollBarType(KStyle::ThreeButtonScrollBar);
		SBSTF = 1;	// KDEStyleScrollBar
	}

	if ( _animateProgressBar )
	{
		animationTimer = new QTimer( this );
		connect( animationTimer, SIGNAL(timeout()), this, SLOT(updateProgressPos()) );
	}
	
	// Popup sub-menu delay
	_submenuDelay = 100;	// Motif-like
	if (_submenuSpeed > 0)
		_submenuDelay += (_submenuSpeed * 50);
	else if (_submenuSpeed < 0)
		_submenuDelay += (_submenuSpeed * 5);
	//
	// Purer hover effect
	SRN_OVER = _purerHover ? 128 : 192;

	_reverseLayout = QApplication::reverseLayout();
	/// Width of the comboboxes and spinwidgets control area (was: 17)
	_controlWidth = 19;

	// Manage auto-masked bitmaps
	radiobutton_mask = QBitmap(15, 15, radiobutton_mask_bits, true);
	radiobutton_mask.setMask(radiobutton_mask);
	//
	radiomark_rough = QBitmap(7, 7, radiomark_rough_bits, true);
	radiomark_rough.setMask(radiomark_rough);
	radiomark_alias = QBitmap(7, 7, radiomark_alias_bits, true);
	radiomark_alias.setMask(radiomark_alias);
	checkmark_rough = QBitmap(9, 9, checkmark_rough_bits, true);
	checkmark_rough.setMask(checkmark_rough);
	checkmark_alias = QBitmap(9, 9, checkmark_alias_bits, true);
	checkmark_alias.setMask(checkmark_alias);
	tristate_rough = QBitmap(9, 9, tristate_rough_bits, true);
	tristate_rough.setMask(tristate_rough);
	tristate_alias = QBitmap(9, 9, tristate_alias_bits, true);
	tristate_alias.setMask(tristate_alias);
	bigUp = QBitmap(9, 6, bigUp_bits, true);
	bigUp.setMask(bigUp);
	bigUp_alias = QBitmap(9, 6, bigUp_alias_bits, true);
	bigUp_alias.setMask(bigUp_alias);
	bigDown = QBitmap(9, 6, bigDown_bits, true);
	bigDown.setMask(bigDown);
	bigDown_alias = QBitmap(9, 6, bigDown_alias_bits, true);
	bigDown_alias.setMask(bigDown_alias);
	bigLeft = QBitmap(6, 9, bigLeft_bits, true);
	bigLeft.setMask(bigLeft);
	bigLeft_alias = QBitmap(6, 9, bigLeft_alias_bits, true);
	bigLeft_alias.setMask(bigLeft_alias);
	bigRight = QBitmap(6, 9, bigRight_bits, true);
	bigRight.setMask(bigRight);
	bigRight_alias = QBitmap(6, 9, bigRight_alias_bits, true);
	bigRight_alias.setMask(bigRight_alias);
	collapsed_rough = QBitmap(7, 9, collapsed_rough_bits, true);
	collapsed_rough.setMask(collapsed_rough);
	collapsed_alias = QBitmap(7, 9, collapsed_alias_bits, true);
	collapsed_alias.setMask(collapsed_alias);
	expanded_rough = QBitmap(9, 7, expanded_rough_bits, true);
	expanded_rough.setMask(expanded_rough);
	expanded_alias = QBitmap(9, 7, expanded_alias_bits, true);
	expanded_alias.setMask(expanded_alias);
	bigPlus = QBitmap(9, 9, bigPlus_bits, true);
	bigPlus.setMask(bigPlus);
	bigPlus_alias = QBitmap(9, 9, bigPlus_alias_bits, true);
	bigPlus_alias.setMask(bigPlus_alias);
	bigMinus = QBitmap(9, 3, bigMinus_bits, true);
	bigMinus.setMask(bigMinus);
	bigMinus_alias = QBitmap(9, 3, bigMinus_alias_bits, true);
	bigMinus_alias.setMask(bigMinus_alias);
	tiny_radiomark_rough = QBitmap(5, 5, tiny_radiomark_rough_bits, true);
	tiny_radiomark_rough.setMask(tiny_radiomark_rough);
	tiny_radiomark_alias = QBitmap(5, 5, tiny_radiomark_alias_bits, true);
	tiny_radiomark_alias.setMask(tiny_radiomark_alias);
	//
	littleLeft = QBitmap(5, 7, littleLeft_bits, true);
	littleLeft.setMask(littleLeft);
	littleRight = QBitmap(5, 7, littleRight_bits, true);
	littleRight.setMask(littleRight);
	littleUp = QBitmap(7, 5, littleUp_bits, true);
	littleUp.setMask(littleUp);
	littleDown = QBitmap(7, 5, littleDown_bits, true);
	littleDown.setMask(littleDown);
	miniLeft = QBitmap(5, 5, miniLeft_bits, true);
	miniLeft.setMask(miniLeft);
	miniRight = QBitmap(5, 5, miniRight_bits, true);
	miniRight.setMask(miniRight);
	miniUp = QBitmap(5, 5, miniUp_bits, true);
	miniUp.setMask(miniUp);
	miniDown = QBitmap(5, 5, miniDown_bits, true);
	miniDown.setMask(miniDown);
	toolPlus = QBitmap(8, 8, toolPlus_bits, true);
	toolPlus.setMask(toolPlus);
	appletMenu = QBitmap(6, 6, appletMenu_bits, true);
	appletMenu.setMask(appletMenu);
	dockClose = QBitmap(8, 8, dockClose_bits, true);
	dockClose.setMask(dockClose);
	dockIn = QBitmap(9, 8, dockIn_bits, true);
	dockIn.setMask(dockIn);
	dockOut = QBitmap(9, 8, dockOut_bits, true);
	dockOut.setMask(dockOut);
	dockStay = QBitmap(8, 8, dockStay_bits, true);
	dockStay.setMask(dockStay);
	modLeftDeco = QBitmap(4, 11, modLeftDeco_bits, true);
	modLeftDeco.setMask(modLeftDeco);
	modCollapsed = QBitmap(11, 11, modCollapsed_bits, true);
	modCollapsed.setMask(modCollapsed);
	modTopDeco = QBitmap(11, 4, modTopDeco_bits, true);
	modTopDeco.setMask(modTopDeco);
	modExpanded = QBitmap(11, 11, modExpanded_bits, true);
	modExpanded.setMask(modExpanded);
	modDot = QBitmap(5, 5, modDot_bits, true);
	modDot.setMask(modDot);

	// Some non-masked bitmaps
	radio_inner_alias = QBitmap(15, 15, radio_inner_alias_bits, true);
	radio_contour_rough = QBitmap(15, 15, radio_contour_rough_bits, true);
	radio_outer_alias = QBitmap(15, 15, radio_outer_alias_bits, true);
	tiny_radiobutton_mask = QBitmap(13, 13, tiny_radiobutton_mask_bits, true);
	tiny_radio_contour_rough = QBitmap(13, 13, tiny_radio_contour_rough_bits, true);
	tiny_radio_outer_alias = QBitmap(13, 13, tiny_radio_outer_alias_bits, true);
}

void SerenityStyle::updateProgressPos()
{
	QProgressBar* pb;
	// Update the registered progress bars.
	QMap<QWidget*, int>::iterator iter;
	bool visible = false;
	for (iter = progAnimWidgets.begin(); iter != progAnimWidgets.end(); iter++)
	{
		if (! ::qt_cast<QProgressBar*>(iter.key()))
			continue;
		
		pb = dynamic_cast<QProgressBar*>(iter.key());
	
		if ( pb->isEnabled() && pb->progress() != pb->totalSteps() )
		{
			// Update animation offset of current widget
			iter.data() = (iter.data() + 1) % 24;
			pb->update();
		}
	
		if (iter.key()->isVisible())
			visible = true;
	}
	if (!visible)
		animationTimer->stop();
}


SerenityStyle::~SerenityStyle()
{
}

void SerenityStyle::polish( QPalette& pal )
{
	/// MXLS: With this palette the widgets are blended
	/// with the background when disabled and --most of the time--
	/// the painting just acknowledges it without special code.
	QColor groundColor = pal.color(QPalette::Active, QColorGroup::Background);
	QColor baseColor = pal.color(QPalette::Active, QColorGroup::Base);
	QColor textColor = pal.color(QPalette::Active, QColorGroup::Foreground);
	QColor buttonColor = pal.color(QPalette::Active, QColorGroup::Button);
	QColor btnText = pal.color(QPalette::Active, QColorGroup::ButtonText);
	QColor hilite = pal.color(QPalette::Active, QColorGroup::Highlight);
	QColor hiText = pal.color(QPalette::Active, QColorGroup::HighlightedText);
	pal.setColor(QPalette::Disabled, QColorGroup::Background, groundColor);
	pal.setColor(QPalette::Disabled, QColorGroup::Base, 
				colorMix(groundColor, baseColor) );
	pal.setColor(QPalette::Disabled, QColorGroup::Foreground, 
				colorMix(groundColor, textColor) );
	pal.setColor(QPalette::Disabled, QColorGroup::Dark,
				colorMix(groundColor, textColor) ); // Disabled KToolButton label
	pal.setColor(QPalette::Disabled, QColorGroup::Text, 
				colorMix(baseColor, textColor) );
	pal.setColor(QPalette::Disabled, QColorGroup::Button, 
				colorMix(groundColor, buttonColor) );
	pal.setColor(QPalette::Disabled, QColorGroup::ButtonText, 
				colorMix(buttonColor, btnText) );
	pal.setColor(QPalette::Disabled, QColorGroup::Highlight, 
				colorMix(baseColor, hilite) );
	pal.setColor(QPalette::Disabled, QColorGroup::HighlightedText, 
				colorMix(hilite, hiText) );

	globalHiliteColor = hilite;
	globalHoverColor = (_customOverHighlightColor) ? _overHighlightColor : hilite;
}

void SerenityStyle::polish(QApplication* app)
{
	if (! qstrcmp(app->argv()[0], "kicker"))
		kickerMode = true;
	else if (!qstrcmp(app->argv()[0], "korn"))
		kornMode = true;
	else if (!qstrcmp(app->argv()[0], "kate"))
		kateMode = true;
	else if (!qstrcmp(app->argv()[0], "kontact"))
		kontactMode = true;
}

void SerenityStyle::polish(QWidget* widget)
{
	 // Is it a khtml widget?
	if (! strcmp(widget->name(), "__khtml"))
	{
		khtmlWidgets[widget] = true;
		connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(khtmlWidgetDestroyed(QObject*)));
	}
	
	// This removes some flickering.
	/// MXLS: Warning! Setting the background may imply 
	/// the corresponding foreground for some widgets.
	if ( ::qt_cast<QToolBar*>(widget) || ::qt_cast<QToolButton*>(widget) 
	|| ::qt_cast<QScrollBar*>(widget) || ::qt_cast<KKeyButton*>(widget) 
	|| ::qt_cast<KToolBarSeparator*>(widget) || ::qt_cast<QProgressBar*>(widget) 
	|| ::qt_cast<QComboBox*>(widget) || ::qt_cast<QPushButton*>(widget) 
	|| widget->inherits("QSplitterHandle") || widget->inherits("QToolBarSeparator") 
	|| widget->inherits("QDockWindowHandle") || widget->inherits("QDockWindowResizeHandle") 
	|| widget->inherits("KMultiTabBarButton") || widget->inherits("PanelButton") 
	|| widget->isA("AppletHandleButton") ) 
	{
		if (widget->backgroundMode() != NoBackground)
			widget->setBackgroundMode(PaletteBackground);
	}
	
	if ( ::qt_cast<QPushButton*>(widget) || ::qt_cast<QSlider*>(widget) 
	|| ::qt_cast<QToolButton*>(widget) 
	|| widget->inherits("QSplitterHandle") || widget->inherits("QDockWindowHandle") 
	|| widget->inherits("QDockWindowResizeHandle") || widget->isA("KDockButton_Private") 
	|| widget->isA("AppletHandleButton") 
	|| (widget->parent() && widget->parent()->isA("QToolBox")) )
	{
		widget->installEventFilter(this);
	}
	else if (::qt_cast<QMenuBar*>(widget))
	{
		widget->setBackgroundMode(NoBackground);
		if (_menubarHack)
			widget->installEventFilter(this);
	}
	else if (::qt_cast<QLineEdit*>(widget))
	{
		widget->setBackgroundMode(PaletteBase);
		QPalette pal = QApplication::palette();
		pal.setColor(QPalette::Disabled, QColorGroup::Background, 
				pal.color(QPalette::Disabled, QColorGroup::Base) );
		widget->setPalette(pal);
		widget->installEventFilter(this);
	}
	else if ( ::qt_cast<QScrollBar*>(widget) || ::qt_cast<QComboBox*>(widget) 
	|| ::qt_cast<QCheckBox*>(widget) || ::qt_cast<QRadioButton*>(widget) 
	|| ::qt_cast<QSpinWidget*>(widget) || ::qt_cast<QTabBar*>(widget) ) 
	{
		widget->setMouseTracking(true);
		widget->installEventFilter(this);
	}
	else if (::qt_cast<QHeader*>(widget))
	{
		widget->setBackgroundMode(PaletteBase);
		widget->installEventFilter(this);
	}
	else if (::qt_cast<QPopupMenu*>(widget))
	{
		widget->setBackgroundMode(NoBackground);
	}
	else if (! qstrcmp(widget->name(), "kde toolbar widget"))
	{
		widget->setBackgroundMode(PaletteBackground);
		widget->installEventFilter(this);
	}
	else if ( widget->isA("QWhatsThat") || widget->isA("QTipLabel") 
	|| widget->isA("KonqFileTip") || widget->inherits("QToolTip") )
	{
		widget->setBackgroundMode(NoBackground);
		widget->installEventFilter(this);
	}
	else if (::qt_cast<QStatusBar*>(widget))
	{
		/// MXLS: This event will disable all color changes of 
		/// anybody's status bar.
		widget->installEventFilter(this);
	}

	if ( _animateProgressBar && ::qt_cast<QProgressBar*>(widget) )
	{
		widget->installEventFilter(this);
		progAnimWidgets[widget] = 0;
		connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(progressBarDestroyed(QObject*)));
		if (!animationTimer->isActive())
			animationTimer->start(50, false);	// Progress Animation Rate (msec)
	}
	
	if (::qt_cast<QGroupBox*>(widget))
	{
		QGroupBox* gb = dynamic_cast<QGroupBox*>(widget);
		gb->setAlignment( QStyle::AlignHCenter );
	
	}
	
	if (::qt_cast<QLabel*>(widget))
	{
		widget->installEventFilter(this);
	}
	
	/// MXLS: This one must stay at the end of the list!
	if (::qt_cast<QFrame*>(widget))
	{
		widget->installEventFilter(this);
	}
	
	KStyle::polish(widget);
}

void SerenityStyle::unPolish(QWidget* widget)
{
	if (! strcmp(widget->name(), "__khtml"))
	{
		khtmlWidgets.remove(widget);
	}
	
	if ( ::qt_cast<QMenuBar*>(widget) || ::qt_cast<QToolBar*>(widget) 
	|| ::qt_cast<QToolButton*>(widget) || ::qt_cast<KKeyButton*>(widget) 
	|| ::qt_cast<KToolBarSeparator*>(widget) || ::qt_cast<QComboBox*>(widget) 
	|| ::qt_cast<QPushButton*>(widget) 
	|| widget->inherits("QSplitterHandle") || widget->inherits("QToolBarSeparator") 
	|| widget->inherits("QDockWindowHandle") || widget->inherits("QDockWindowResizeHandle") 
	|| widget->inherits("KMultiTabBarButton") || widget->inherits("PanelButton") 
	|| widget->isA("AppletHandleButton") )
	{
		widget->setBackgroundMode(PaletteButton);
	}
		
	if (::qt_cast<QMenuBar*>(widget) && _menubarHack)
	{
			widget->removeEventFilter(this);
	}

	if (::qt_cast<QLineEdit*>(widget) || ::qt_cast<QProgressBar*>(widget))
	{
		QPalette pal = QApplication::palette();
		pal.setColor(QPalette::Disabled, QColorGroup::Background, 
				pal.color(QPalette::Active, QColorGroup::Background) );
		widget->setPalette(pal);
	}
	
	if ( ::qt_cast<QPushButton*>(widget) || ::qt_cast<QComboBox*>(widget) 
	|| ::qt_cast<QToolButton*>(widget) || ::qt_cast<QLineEdit*>(widget) 
	|| ::qt_cast<QStatusBar*>(widget) || ::qt_cast<QSlider*>(widget) 
	|| widget->inherits("QSplitterHandle") || widget->inherits("QDockWindowHandle") 
	|| widget->inherits("QDockWindowResizeHandle") || widget->inherits("QViewportWidget") 
	|| widget->isA("KDockButton_Private") || widget->isA("AppletHandleButton") 
	|| (widget->parent() && widget->parent()->isA("QToolBox")) 
	|| (!qstrcmp(widget->name(), "kde toolbar widget")) )
	{
		widget->removeEventFilter(this);
	}
	else if (::qt_cast<QScrollBar*>(widget))
	{
		widget->setBackgroundMode(PaletteBackground);
		widget->setMouseTracking(false);
		widget->removeEventFilter(this);
	}
	else if (::qt_cast<QCheckBox*>(widget) || ::qt_cast<QRadioButton*>(widget) 
	|| ::qt_cast<QTabBar*>(widget) || ::qt_cast<QSpinWidget*>(widget)) 
	{
		widget->setMouseTracking(false);
		widget->removeEventFilter(this);
	}
	else if (::qt_cast<QHeader*>(widget))
	{
		widget->setBackgroundMode(PaletteButton);
		widget->removeEventFilter(this);
	}
	else if (::qt_cast<QPopupMenu*>(widget))
	{
		widget->setBackgroundMode(PaletteBackground);
	}
	else if ( widget->isA("QWhatsThat") || widget->isA("QTipLabel") 
	|| widget->isA("KonqFileTip") || widget->inherits("QToolTip") )
	{
		widget->setBackgroundMode(PaletteBase);
		widget->removeEventFilter(this);
	}
	
	if (::qt_cast<QProgressBar*>(widget) )
	{
		progAnimWidgets.remove(widget);
	}
	
	if (::qt_cast<QFrame*>(widget))
	{
		widget->removeEventFilter(this);
	}
	
	KStyle::unPolish(widget);
}

void SerenityStyle::khtmlWidgetDestroyed(QObject* obj)
{
	khtmlWidgets.remove(static_cast<QWidget*>(obj));
}

void SerenityStyle::progressBarDestroyed(QObject* obj)
{
	progAnimWidgets.remove(static_cast<QWidget*>(obj));
}

void SerenityStyle::renderGradient(QPainter* p, 
				const QRect &r, 
				const QColor &startColor, 
				const QColor &endColor, 
				const int direction, 
				const bool convex) const
{
	if ( (r.width() <= 0) || (r.height() <= 0) )
		return;
	
	int width = r.width();
	int height = r.height();
	
	QColor first, last;
	if (convex)
	{
		first = startColor;
		last = endColor;
	}
	else	// Exchange colors.
	{
		first = endColor;
		last = startColor;
	}
	
	int fR, fG, fB;		// first RGB components
	first.getRgb(&fR, &fG, &fB);
	int lR, lG, lB;		// last RGB components
	last.getRgb(&lR, &lG, &lB);
	//
	int rr = fR*1000;	// Start color
	int gg = fG*1000;
	int bb = fB*1000;
	//
	// Where to draw our gradient.
	QImage* result = new QImage(width, height, 32);
	//
	int deltaR, deltaG, deltaB;
	register int x, y;
	unsigned int* line;
	if (direction == VerticalGradient)
	{
		deltaR = ((lR-fR)*1000)/height;
		deltaG = ((lG-fG)*1000)/height;
		deltaB = ((lB-fB)*1000)/height;
		for (y = 0; y < height; y++)
		{
			line = (unsigned int*)(result->scanLine(y));
			int color = qRgb(rr/1000, gg/1000, bb/1000);
			for (x = 0; x < width; x++)
			{
				line[x] = color;
			}
			rr += deltaR;
			gg += deltaG;
			bb += deltaB;
		}
	}
	else
	{
		deltaR = ((lR-fR)*1000)/width;
		deltaG = ((lG-fG)*1000)/width;
		deltaB = ((lB-fB)*1000)/width;
		line = (unsigned int*)(result->scanLine(0));
		for (x = 0; x < width; x++)
		{
			line[x] = qRgb(rr/1000, gg/1000, bb/1000);
			rr += deltaR;
			gg += deltaG;
			bb += deltaB;
		}
		unsigned int* src;
		unsigned int* dst;
		for (y = 1; y < height; y++)
		{
			src = line;
			dst = (unsigned int*)(result->scanLine(y));
			for (x = 0; x < width; x++)
			{
				*dst++ = *src++;
			}
		}
	}
	//
	p->drawImage(r.left(), r.top(), *result);
	delete result;
}

void SerenityStyle::renderGradLine(QPainter* p, 
				const QRect &r, 
				const QColor &startColor, 
				const QColor &endColor, 
				const int direction, 
				const bool convex) const
{
	if ( (r.width() <= 0) || (r.height() <= 0) )
		return;
	
	int width = r.width();
	int height = r.height();
	
	QColor first, last;
	if (convex)
	{
		first = startColor;
		last = endColor;
	}
	else	// Exchange colors.
	{
		first = endColor;
		last = startColor;
	}
	
	int fR, fG, fB;		// first RGB components
	first.getRgb(&fR, &fG, &fB);
	int lR, lG, lB;		// last RGB components
	last.getRgb(&lR, &lG, &lB);
	//
	int rr = fR*1000;	// Start color
	int gg = fG*1000;
	int bb = fB*1000;
	//
	// Where to draw our gradient.
	QImage* result = new QImage(width, height, 32);
	//
	int deltaR, deltaG, deltaB;
	register int x, y;
	unsigned int* line;
	if (direction == VerticalGradient)
	{
		deltaR = ((lR-fR)*1000)/height;
		deltaG = ((lG-fG)*1000)/height;
		deltaB = ((lB-fB)*1000)/height;
		for (y = 0; y < height; y++)
		{
			line = (unsigned int*)(result->scanLine(y));
			line[0] = qRgb(rr/1000, gg/1000, bb/1000);
			rr += deltaR;
			gg += deltaG;
			bb += deltaB;
		}
	}
	else	// (direction == HorizontalGradient)
	{
		deltaR = ((lR-fR)*1000)/width;
		deltaG = ((lG-fG)*1000)/width;
		deltaB = ((lB-fB)*1000)/width;
		line = (unsigned int*)(result->scanLine(0));
		for (x = 0; x < width; x++)
		{
			line[x] = qRgb(rr/1000, gg/1000, bb/1000);
			rr += deltaR;
			gg += deltaG;
			bb += deltaB;
		}
	}
	//
	p->drawImage(r.left(), r.top(), *result);
	delete result;
}

void SerenityStyle::renderDiagonalGradient(QPainter* p, 
					const QRect &r, 
					const QColor &startColor, 
					const QColor &middleColor, 
					const QColor &endColor, 
					const bool convex) const
{
	if ( (r.width() < 2) || (r.height() < 2) )
		return;
	
	int width = r.width();
	int height = r.height();
	
	QColor first, last;
	if (convex)
	{
		first = startColor;
		last = endColor;
	}
	else	// Exchange colors.
	{
		first = endColor;
		last = startColor;
	}
	
	int fR, fG, fB;		// first RGB components
	first.getRgb(&fR, &fG, &fB);
	int mR, mG, mB;		// middle RGB components
	middleColor.getRgb(&mR, &mG, &mB);
	int lR, lG, lB;		// last RGB components
	last.getRgb(&lR, &lG, &lB);
	//
	int ldR = ((mR-fR)*1000)/height;	// On the left,
	int ldG = ((mG-fG)*1000)/height;	// from "first" to "middleColor". 
	int ldB = ((mB-fB)*1000)/height;	//
	int rdR = ((lR-mR)*1000)/height;	// On the right,
	int rdG = ((lG-mG)*1000)/height;	// from "middleColor" to "last". 
	int rdB = ((lB-mB)*1000)/height;	//
	//
	int leftR = fR*1000;	// top left
	int leftG = fG*1000;
	int leftB = fB*1000;
	int rightR = mR*1000;	// top right
	int rightG = mG*1000;
	int rightB = mB*1000;
	//
	// Where to draw our gradient.
	QImage* result = new QImage(width, height, 32);
	//
	int rr, gg, bb;
	int deltaR, deltaG, deltaB;
	register int x, y;
	unsigned int* line;
	for (y = 0; y < height; y++)
	{
		rr = leftR;
		gg = leftG;
		bb = leftB;
		deltaR = (rightR-leftR)/width;	// Horizontal delta(s)
		deltaG = (rightG-leftG)/width;
		deltaB = (rightB-leftB)/width;
		line = (unsigned int*)(result->scanLine(y));
		for (x = 0; x < width; x++)
		{
			line[x] = qRgb(rr/1000, gg/1000, bb/1000);
			rr += deltaR;
			gg += deltaG;
			bb += deltaB;
		}
		leftR += ldR;	// Left vertical delta(s)
		leftG += ldG;
		leftB += ldB;
		rightR += rdR;	// Right vertical delta(s)
		rightG += rdG;
		rightB += rdB;
	}
	//
	p->drawImage(r.left(), r.top(), *result);
	delete result;
}

void SerenityStyle::renderZenGradient(QPainter* p, 
					const QRect &r, 
					const QColor &startColor, 
					const QColor &middleColor, 
					const QColor &endColor, 
					const bool convex) const
{
	if ( (r.width() < 4) || (r.height() < 4) )
		return;
	
	QColor first, last;
	if (convex)
	{
		first = startColor;
		last = endColor;
	}
	else	// Exchange colors.
	{
		first = endColor;
		last = startColor;
	}
	
	int left, top, right, bottom;
	r.coords(&left, &top, &right, &bottom);
	int width, height;
	r.rect(&left, &top, &width, &height);
	//
	int hSize, vSize;
	if ( (height > width) && (width < 20) )
	{
		// For the vertical scrollers
		hSize = width/2;
		vSize = (height <= 14) ? 4 : 7;
	}
	else
	{
		hSize = (width <= 14) ? 4 : 7;
		vSize = height/2;
	}
	int hTwice = hSize*2;
	int vTwice = vSize*2;
	//
	int x1, y1, x2, y2;
	x1 = left+hSize;
	y1 = top+vSize;
	x2 = right-hSize+1;
	y2 = bottom-vSize+1;
	//
	if ( (width > hTwice) || (height > vTwice) )
	{
		p->fillRect(x1, y1, width-hTwice, height-vTwice, QBrush(middleColor));
	}
	if (width > hTwice)
	{
		renderGradient(p, QRect(x1, top, width-hTwice, vSize),
				first, middleColor, VerticalGradient);
		renderGradient(p, QRect(x1, y2, width-hTwice, vSize),
				middleColor, last, VerticalGradient);
	}
	if (height > vTwice)
	{
		renderGradient(p, QRect(left, y1, hSize, height-vTwice),
				first, middleColor, HorizontalGradient);
		renderGradient(p, QRect(x2, y1, hSize, height-vTwice),
				middleColor, last, HorizontalGradient);
	}
	renderDiagonalGradient(p, QRect(left, top, hSize, vSize), 
				first, first, middleColor);
	renderDiagonalGradient(p, QRect(x2, top, hSize, vSize), 
				first, middleColor, last);
	renderDiagonalGradient(p, QRect(left, y2, hSize, vSize), 
				first, middleColor, last);
	renderDiagonalGradient(p, QRect(x2, y2, hSize, vSize), 
				middleColor, last, last);
}

void SerenityStyle::renderCircleGradient(QPainter* p, 
					const QRect &r, 
					const QColor &startColor, 
					const QColor &middleColor, 
					const QColor &endColor, 
					const bool convex) const
{
	if ( (r.width() < 4) || (r.height() < 4) )
		return;
	
	int left, top, right, bottom;
	r.coords(&left, &top, &right, &bottom);
	int width, height;
	r.rect(&left, &top, &width, &height);
	
	QColor first, last;
	if (convex)
	{
		first = startColor;
		last = endColor;
	}
	else	// Exchange colors.
	{
		first = endColor;
		last = startColor;
	}
	
	int leftW, topH, rightW, bottomH;
	leftW = width/2;
	rightW = width-leftW;
	topH = height/2;
	bottomH = height-topH;
	//
	int xx = right-rightW+1;
	int yy = bottom-bottomH+1;
	//
	renderDiagonalGradient(p, QRect(left, top, leftW, topH), 
				first, first, middleColor);
	renderDiagonalGradient(p, QRect(xx, top, rightW, topH), 
				first, middleColor, last);
	renderDiagonalGradient(p, QRect(left, yy, leftW, bottomH), 
				first, middleColor, last);
	renderDiagonalGradient(p, QRect(xx, yy, rightW, bottomH), 
				middleColor, last, last);
}

void SerenityStyle::renderBicoloreGradient(QPainter* p, 
					const QRect &r, 
					const QColorGroup &g, 
					const QColor &backgroundColor, 
					const QColor &startColor, 
					const QColor &middleColor, 
					const QColor &endColor,
					const int flags, 
					const bool convex) const
{
	if ( (r.width() < 4) || (r.height() < 4) )
		return;
	
	int left, top, right, bottom;
	r.coords(&left, &top, &right, &bottom);
	int width, height;
	r.rect(&left, &top, &width, &height);
	
	bool sunken = flags & Is_Sunken;
	bool mouseOver = flags & Is_Hovered;

	QColor first, last;
	if (sunken || (!convex && !mouseOver))
	{
		// Exchange colors.
		first = endColor;
		last = startColor;
	}
	else 	// (convex || mouseOver)
	{
		first = startColor;
		last = endColor;
	}
	QColor middle = middleColor;
	QColor hilite;
	if (sunken)
	{
		hilite = g.highlight();
		first = colorMix(first, hilite, SRN_DOWN); 
		middle = colorMix(middle, hilite, SRN_DOWN); 
		last = colorMix(last, hilite, SRN_DOWN); 
	}
	else if (mouseOver)
	{
		hilite = getColor(g, HoverColor);
		first = colorMix(first, hilite, SRN_OVER); 
		middle = colorMix(middle, hilite, SRN_OVER); 
		last = colorMix(last, hilite, SRN_OVER); 
	}
	
	int topH, bottomH;
	topH = height/2;
	bottomH = height-topH;
	//
	renderGradient(p, QRect(left, top, width, topH), 
				first, middle, VerticalGradient);
	renderGradient(p, QRect(left, top+topH, width, bottomH), 
				middle, last, VerticalGradient);
	//
	if (flags & Round_UpperLeft)
	{
		p->setPen(backgroundColor);
		p->drawPoint(left, top);
		p->setPen( colorMix(backgroundColor, first) );
		p->drawLine(left, top+1, left+1, top);
	}
	if (flags & Round_UpperRight)
	{
		p->setPen(backgroundColor);
		p->drawPoint(right, top);
		p->setPen( colorMix(backgroundColor, first) );
		p->drawLine(right-1, top, right, top+1);
	}
	if (flags & Round_BottomLeft)
	{
		p->setPen(backgroundColor);
		p->drawPoint(left, bottom);
		p->setPen( colorMix(backgroundColor, last) );
		p->drawLine(left, bottom-1, left+1, bottom);
	}
	if (flags & Round_BottomRight)
	{
		p->setPen(backgroundColor);
		p->drawPoint(right, bottom);
		p->setPen( colorMix(backgroundColor, last) );
		p->drawLine(right-1, bottom, right, bottom-1);
	}
}

void SerenityStyle::renderMenuGradient(QPainter* p, 
					const QRect &r, 
					const QColor &startColor, 
					const QColor &middleColor, 
					const QColor &endColor,
					const bool convex) const
{
	if ( (r.width() < 4) || (r.height() < 4) )
		return;
	
	int left, top, right, bottom;
	r.coords(&left, &top, &right, &bottom);
	int width, height;
	r.rect(&left, &top, &width, &height);
	//
	int topH, bottomH;
	topH = height/2;
	bottomH = height-topH;
	
	if (convex)
	{
		p->fillRect(left, top, width, topH, QBrush(middleColor));
		p->setPen(startColor);
		p->drawLine(left, top, right, top);
		renderGradient(p, QRect(left, top+topH, width, bottomH), 
					middleColor, endColor, VerticalGradient);
	}
	else
	{
		renderGradient(p, QRect(left, top, width, topH), 
					endColor, middleColor, VerticalGradient);
		p->fillRect(left, top+topH, width, bottomH, QBrush(middleColor));
		p->setPen(startColor);
		p->drawLine(left, bottom, right, bottom);
	}
}

void SerenityStyle::renderPixel(QPainter* p, 
				const QPoint xy, 
				const QColor &color, 
				const int alpha) const
{
	QRgb rgb = color.rgb();
	QImage* tempo = new QImage(1, 1, 32);	// (1x1) x 32 bits
	tempo->setAlphaBuffer(true);
	tempo->setPixel(0, 0, qRgba(qRed(rgb), qGreen(rgb), qBlue(rgb), alpha) );
	p->drawImage(xy, *tempo);
	delete tempo;
	// Qt will take care of any conversion --if required.
}

void SerenityStyle::renderSereneContour(QPainter* p, 
				const QRect &r, 
				const QColor &backgroundColor, 
				const QColor &contourColor, 
				const uint flags) const
{
	if ( (r.width() < 4) || (r.height() < 4) )
		return;

	QPen oldPen = p->pen();

	// Re-calculated base colors
	QColor topLeftColor = borderColor(contourColor, TOP, flags&Is_Sunken);
	QColor bottomRightColor = borderColor(contourColor, BOTTOM, flags&Is_Sunken);
	QColor baseColor = contourColor;
	if (flags&Is_Disabled)
	{
		topLeftColor = colorMix(backgroundColor, topLeftColor);
		bottomRightColor = colorMix(backgroundColor, bottomRightColor);
		baseColor = colorMix(backgroundColor, baseColor);
	}
	//
	SereneShape shape(r, flags);
	//
	if ( (_globalStyle != Zen) || (flags&Serene_Drawing) )
	{
		if ((flags&Square_Box) == Square_Box)
		{
			renderGradLine(p, shape.topBorder(),
					topLeftColor, baseColor, HorizontalGradient);
			renderGradLine(p, shape.leftBorder(), 
					topLeftColor, baseColor, VerticalGradient);
			renderGradLine(p, shape.rightBorder(),
					baseColor, bottomRightColor, VerticalGradient);
			renderGradLine(p, shape.bottomBorder(),
					baseColor, bottomRightColor, HorizontalGradient);
		}
		else
		{
			if (flags&Draw_Top)
			{
				renderGradLine(p, shape.topBorder(),
						topLeftColor, baseColor, HorizontalGradient);
			}
			if (flags&Draw_Left)
			{
				renderGradLine(p, shape.leftBorder(), 
						topLeftColor, baseColor, VerticalGradient);
			}
			if (flags&Draw_Right)
			{
				renderGradLine(p, shape.rightBorder(),
						baseColor, bottomRightColor, VerticalGradient);
			}
			if (flags&Draw_Bottom)
			{
				renderGradLine(p, shape.bottomBorder(),
						baseColor, bottomRightColor, HorizontalGradient);
			}
		}
	}
	else	// (_globalStyle == Zen)
	{
		p->setPen(topLeftColor);
		if ((flags&Square_Box) == Square_Box)
		{
			p->drawLine(shape.topLeft(), shape.topRight());
			p->drawLine(shape.leftTop(), shape.leftBottom());
			p->setPen(bottomRightColor);
			p->drawLine(shape.rightTop(), shape.rightBottom());
			p->drawLine(shape.bottomLeft(), shape.bottomRight());
		}
		else
		{
			if (flags&Draw_Top)
				p->drawLine(shape.topLeft(), shape.topRight());
			if (flags&Draw_Left)
				p->drawLine(shape.leftTop(), shape.leftBottom());
			p->setPen(bottomRightColor);
			if (flags&Draw_Right)
				p->drawLine(shape.rightTop(), shape.rightBottom());
			if (flags&Draw_Bottom)
				p->drawLine(shape.bottomLeft(), shape.bottomRight());
		}
	}
	if (flags&Draw_AlphaBlend)
	{
		if ((flags&Round_All) == Round_All)
		{
			renderPixel(p, shape.aliasedTopLeftPoint(1), topLeftColor);
			renderPixel(p, shape.aliasedTopLeftPoint(2), topLeftColor);
			p->setPen(topLeftColor);
			p->drawPoint(shape.cornerTopLeftPoint());
			//
			renderPixel(p, shape.aliasedTopRightPoint(1), baseColor);
			renderPixel(p, shape.aliasedTopRightPoint(2), baseColor);
			p->setPen(baseColor);
			p->drawPoint(shape.cornerTopRightPoint());
			//
			renderPixel(p, shape.aliasedBottomLeftPoint(1), baseColor);
			renderPixel(p, shape.aliasedBottomLeftPoint(2), baseColor);
			p->setPen(baseColor);
			p->drawPoint(shape.cornerBottomLeftPoint());
			//
			renderPixel(p, shape.aliasedBottomRightPoint(1), bottomRightColor);
			renderPixel(p, shape.aliasedBottomRightPoint(2), bottomRightColor);
			p->setPen(bottomRightColor);
			p->drawPoint(shape.cornerBottomRightPoint());
		}
		else if ((flags&Round_All) == No_Flags)	// Square shape
		{
			renderPixel(p, shape.aliasedTopLeftPoint(1), topLeftColor);
			renderPixel(p, shape.aliasedTopRightPoint(1), baseColor);
			renderPixel(p, shape.aliasedBottomLeftPoint(1), baseColor);
			renderPixel(p, shape.aliasedBottomRightPoint(1), bottomRightColor);
		}
		else	// Partially round, partially square (PITA)
		{
			renderPixel(p, shape.aliasedTopLeftPoint(1), topLeftColor);
			if (shape.roundTopLeft())
			{
				renderPixel(p, shape.aliasedTopLeftPoint(2), topLeftColor);
				p->setPen(topLeftColor);
				p->drawPoint(shape.cornerTopLeftPoint());
			}
			renderPixel(p, shape.aliasedTopRightPoint(1), baseColor);
			if (shape.roundTopRight())
			{
				renderPixel(p, shape.aliasedTopRightPoint(2), baseColor);
				p->setPen(baseColor);
				p->drawPoint(shape.cornerTopRightPoint());
			}
			renderPixel(p, shape.aliasedBottomLeftPoint(1), baseColor);
			if (shape.roundBottomLeft())
			{
				renderPixel(p, shape.aliasedBottomLeftPoint(2), baseColor);
				p->setPen(baseColor);
				p->drawPoint(shape.cornerBottomLeftPoint());
			}
			renderPixel(p, shape.aliasedBottomRightPoint(1), bottomRightColor);
			if (shape.roundBottomRight())
			{
				renderPixel(p, shape.aliasedBottomRightPoint(2), bottomRightColor);
				p->setPen(bottomRightColor);
				p->drawPoint(shape.cornerBottomRightPoint());
			}
		}
	}
	else
	{
		if ((flags&Round_All) == Round_All)
		{
			p->setPen( colorMix(backgroundColor, topLeftColor) );
			p->drawPoint(shape.aliasedTopLeftPoint(1));
			p->drawPoint(shape.aliasedTopLeftPoint(2));
			p->setPen(topLeftColor);
			p->drawPoint(shape.cornerTopLeftPoint());
			p->setPen(backgroundColor);
			p->drawPoint(shape.solidTopLeftPoint());
			//
			p->setPen( colorMix(backgroundColor, baseColor) );
			p->drawPoint(shape.aliasedTopRightPoint(1));
			p->drawPoint(shape.aliasedTopRightPoint(2));
			p->setPen(baseColor);
			p->drawPoint(shape.cornerTopRightPoint());
			p->setPen(backgroundColor);
			p->drawPoint(shape.solidTopRightPoint());
			//
			p->setPen( colorMix(backgroundColor, baseColor) );
			p->drawPoint(shape.aliasedBottomLeftPoint(1));
			p->drawPoint(shape.aliasedBottomLeftPoint(2));
			p->setPen(baseColor);
			p->drawPoint(shape.cornerBottomLeftPoint());
			p->setPen(backgroundColor);
			p->drawPoint(shape.solidBottomLeftPoint());
			//
			p->setPen( colorMix(backgroundColor, bottomRightColor) );
			p->drawPoint(shape.aliasedBottomRightPoint(1));
			p->drawPoint(shape.aliasedBottomRightPoint(2));
			p->setPen(bottomRightColor);
			p->drawPoint(shape.cornerBottomRightPoint());
			p->setPen(backgroundColor);
			p->drawPoint(shape.solidBottomRightPoint());
		}
		else if ((flags&Round_All) == No_Flags)	// Square shape
		{
			p->setPen( colorMix(backgroundColor, topLeftColor) );
			p->drawPoint(shape.aliasedTopLeftPoint(1));
			p->setPen( colorMix(backgroundColor, baseColor));
			p->drawPoint(shape.aliasedTopRightPoint(1));
			p->setPen( colorMix(backgroundColor, baseColor));
			p->drawPoint(shape.aliasedBottomLeftPoint(1));
			p->setPen( colorMix(backgroundColor, bottomRightColor) );
			p->drawPoint(shape.aliasedBottomRightPoint(1));
		}
		else	// Partially round, partially square (PITA)
		{
			p->setPen( colorMix(backgroundColor, topLeftColor) );
			p->drawPoint(shape.aliasedTopLeftPoint(1));
			if (shape.roundTopLeft())
			{
				p->drawPoint(shape.aliasedTopLeftPoint(2));
				p->setPen(topLeftColor);
				p->drawPoint(shape.cornerTopLeftPoint());
				p->setPen(backgroundColor);
				p->drawPoint(shape.solidTopLeftPoint());
			}
			p->setPen( colorMix(backgroundColor, baseColor) );
			p->drawPoint( shape.aliasedTopRightPoint(1) );
			if (shape.roundTopRight())
			{
				p->drawPoint(shape.aliasedTopRightPoint(2));
				p->setPen(baseColor);
				p->drawPoint(shape.cornerTopRightPoint());
				p->setPen(backgroundColor);
				p->drawPoint(shape.solidTopRightPoint());
			}
			p->setPen( colorMix(backgroundColor, baseColor) );
			p->drawPoint(shape.aliasedBottomLeftPoint(1));
			if (shape.roundBottomLeft())
			{
				p->drawPoint(shape.aliasedBottomLeftPoint(2));
				p->setPen(baseColor);
				p->drawPoint(shape.cornerBottomLeftPoint());
				p->setPen(backgroundColor);
				p->drawPoint(shape.solidBottomLeftPoint());
			}
			p->setPen( colorMix(backgroundColor, bottomRightColor) );
			p->drawPoint(shape.aliasedBottomRightPoint(1));
			if (shape.roundBottomRight())
			{
				p->drawPoint(shape.aliasedBottomRightPoint(2));
				p->setPen(bottomRightColor);
				p->drawPoint(shape.cornerBottomRightPoint());
				p->setPen(backgroundColor);
				p->drawPoint(shape.solidBottomRightPoint());
			}
		}
	}
	//
	p->setPen(oldPen);
}

void SerenityStyle::renderContour(QPainter* p, 
				const QRect &r, 
				const QColor &backgroundColor, 
				const QColor &contourColor, 
				const uint flags) const
{
	if ( (r.width() < 4) || (r.height() < 4) )
		return;

	QPen oldPen = p->pen();

	QColor contour = (flags&Is_Disabled)
			? colorMix(backgroundColor, contourColor) 
			: contourColor;
	//
	SereneShape shape(r, flags);
	//
	p->setPen(contour);
	if ((flags&Square_Box) == Square_Box)
	{
		p->drawLine(shape.topLeft(), shape.topRight());
		p->drawLine(shape.leftTop(), shape.leftBottom());
		p->drawLine(shape.rightTop(), shape.rightBottom());
		p->drawLine(shape.bottomLeft(), shape.bottomRight());
	}
	else
	{
		if (flags&Draw_Top)	p->drawLine(shape.topLeft(), shape.topRight());
		if (flags&Draw_Left)	p->drawLine(shape.leftTop(), shape.leftBottom());
		if (flags&Draw_Right)	p->drawLine(shape.rightTop(), shape.rightBottom());
		if (flags&Draw_Bottom)	p->drawLine(shape.bottomLeft(), shape.bottomRight());
	}
	for (int i = 1 ; i <= shape.cornerPixels() ; i++)
	{
		p->drawPoint(shape.cornerPoint(i));
	}
	if (flags & Draw_AlphaBlend)
	{
		for (int i = 1 ; i <= shape.aliasedPixels() ; i++)	// Real anti-aliasing
		{
			renderPixel(p, shape.aliasedPoint(i), contour);
		}
	}
	else
	{
		p->setPen( colorMix(backgroundColor, contour) );	// Quick faked anti-aliasing
		for (int i = 1 ; i <= shape.aliasedPixels() ; i++)
		{
			p->drawPoint( shape.aliasedPoint(i) );
		}
		p->setPen(backgroundColor);				// Fill the corners
		for (int i = 1 ; i <= shape.solidPixels() ; i++)
		{
			p->drawPoint( shape.solidPoint(i) );
		}
	}
	//
	p->setPen(oldPen);
}

void SerenityStyle::renderFlatArea(QPainter* p, 
				 const QRect &r, 
				 const QColor &backgroundColor, 
				 const QColor &surfaceColor, 
				 const uint flags) const
{
	if ( (r.width() < 4) || (r.height() < 4) )
		return;
	//
	SereneShape shape(r, flags);
	//
	QRect work(r);
	work.addCoords(1, 1, -1, -1);
	p->fillRect( work, QBrush(surfaceColor) );
	p->setPen(surfaceColor);
	p->drawLine(shape.topLeft(), shape.topRight());
	p->drawLine(shape.leftTop(), shape.leftBottom());
	p->drawLine(shape.rightTop(), shape.rightBottom());
	p->drawLine(shape.bottomLeft(), shape.bottomRight());
	if (flags & Draw_AlphaBlend)
	{
		for (int i = 1 ; i <= shape.aliasedPixels() ; i++)
		{
			renderPixel(p, shape.aliasedPoint(i), surfaceColor);
		}
	}
	else
	{
		p->setPen( colorMix(backgroundColor, surfaceColor) );	// Quick anti-aliasing
		for (int i = 1 ; i <= shape.aliasedPixels() ; i++)
		{
			p->drawPoint(shape.aliasedPoint(i));
		}
		p->setPen(backgroundColor);				// Fill the corners
		for (int i = 1 ; i <= shape.solidPixels() ; i++)
		{
			p->drawPoint(shape.solidPoint(i));
		}
	}
}

void SerenityStyle::renderSurface(QPainter* p, 
				 const QRect &r, 
				 const QColor &backgroundColor, 
				 const QColor &surfaceColor, 
				 const QColor &highlightColor, 
				 const uint flags) const
{
	if ( (r.width() < 4) || (r.height() < 4) )
		return;

	QPen oldPen = p->pen();
	
	const bool disabled = flags&Is_Disabled;
	bool convex = true;
	
	QColor baseColor = surfaceColor;
	if (flags&Is_Sunken)
	{
		if (!_alternateSinking)
			convex = false;
		if (! (flags&Keep_Color))
			baseColor = colorMix(baseColor, highlightColor, SRN_DOWN);
	}
	else if ( (!disabled) && (flags&Is_Hovered) )
	{
		// Highlight only if not sunken && not disabled.
		baseColor = colorMix(baseColor, highlightColor, SRN_OVER);
	}
	// Re-calculated base colors and surface
	if (disabled)
	{
		baseColor = colorMix(backgroundColor, baseColor);
	}
	// Draw surface
	QRect work(r);
	if (flags&Square_Box)	// If there is any contour
		work.addCoords(1, 1, -1, -1);
	//
	if (_globalStyle == Zen)
	{
		renderZenGradient(p, work,
					gradientColor(baseColor, TOP), 
					baseColor, 
					gradientColor(baseColor, BOTTOM), 
					convex);
	}
	else if (_globalStyle == Flat)
	{
		p->fillRect(work, QBrush(baseColor));
	}
	else	// (_globalStyle == Tao)
	{
		renderDiagonalGradient(p, work,
					gradientColor(baseColor, TOP), 
					baseColor, 
					gradientColor(baseColor, BOTTOM), 
					convex);
	}
	// Draw contour (if any)
	if (flags&Square_Box)
		renderSereneContour(p, r, backgroundColor, baseColor, flags);
	//
	p->setPen(oldPen);
}

void SerenityStyle::renderZenCircle(QPainter* p, 
				 const QRect &r, 
				 const QColor &backgroundColor, 
				 const QColor &surfaceColor, 
				 const QColor &highlightColor, 
				 const uint flags) const
{
	if ( (r.width() < 4) || (r.height() < 4) )
		return;
	
	const bool disabled = flags&Is_Disabled;
	bool convex = true;
	
	// Re-calculated base color
	QColor baseColor = surfaceColor;
	if (flags&Is_Sunken)
	{
		if (!_alternateSinking)
			convex = false;
		if (! flags&Keep_Color)
			baseColor = colorMix(baseColor, highlightColor, SRN_DOWN);
	}
	else if ( (!disabled) && (flags&Is_Hovered) )
	{
		// Highlight only if not sunken && not disabled.
		baseColor = colorMix(baseColor, highlightColor, SRN_OVER);
	}
	if (disabled)
	{
		baseColor = colorMix(backgroundColor, baseColor);
	}
	renderCircleGradient(p, r,
				radioGradientColor(baseColor, TOP), 
				baseColor, 
				radioGradientColor(baseColor, BOTTOM), 
				convex);
}

void SerenityStyle::renderMask(QPainter* p, 
				const QRect &r, 
				const QColor &color, 
				const uint flags) const
{
	if ( (r.width() <= 0) || (r.height() <= 0) )
		return;
	
	const bool roundUpperLeft = flags&Round_UpperLeft;
	const bool roundUpperRight = flags&Round_UpperRight;
	const bool roundBottomLeft = flags&Round_BottomLeft;
	const bool roundBottomRight = flags&Round_BottomRight;
	
	int left, top, width, height;
	r.rect(&left, &top, &width, &height);
	int right, bottom;
	r.coords(&left, &top, &right, &bottom);
	
	p->fillRect(left+1, top+1, width-2, height-2, QBrush(color));
	
	// Sides
	p->setPen(color);
	p->drawLine(roundUpperLeft?left+1:left, top, roundUpperRight?right-1:right, top);
	p->drawLine(roundBottomLeft?left+1:left, bottom, roundBottomRight?right-1:right, bottom);
	p->drawLine(left, roundUpperLeft?top+1:top, left, roundBottomLeft?bottom-1:bottom);
	p->drawLine(right, roundUpperLeft?top+1:top, right, roundBottomLeft?bottom-1:bottom);
}

void SerenityStyle::renderButton(QPainter* p, 
				const QRect &r, 
				const QColorGroup &g, 
				bool sunken, 
				bool mouseOver, 
				bool enabled, 
				bool khtmlMode, 
				const uint extdFlags) const
{
	// Small fix for the taskbar buttons.
	if (kickerMode) enabled = true;
	
	const QPen oldPen(p->pen());
	
	uint surfaceFlags = Button_Alike;
	
	QColor hilite = getColor(g, HoverColor);
	QColor sunkenlite = g.highlight();
	QColor ground = g.background();
	QColor surface;
	if (extdFlags == ToolButton_Type)
	{
		surface = ground;
	}
	else if (extdFlags == Scroller_Type)
	{
		ground = getColor(g, ScrollerGroove);
		surface = getColor(g, ScrollerSurface);
	}
	else if (extdFlags == Slider_Type)
	{
		surface = getColor(g, ScrollerSurface, enabled);
		surfaceFlags |= Draw_AlphaBlend;
	}
	else if (extdFlags == Header_Type)
	{
		surface = getColor(g, HeaderColor);
	}
	else if (extdFlags == Expander_Type)
	{
		surface = getColor(g, ExpanderColor);
		surfaceFlags = Button_Alike|Draw_AlphaBlend;
		hilite = sunkenlite;		// Not the mouseover highlight.
	}
	else if (extdFlags & (TabAction_Type|MultiTab_Type)) 
	{
		surface = getColor(g, TabGroove_Solid, enabled);
	}
	else if (extdFlags  == Flat_Type)
	{
		surface = getColor(g, MixedSurface, enabled);
	}
	else if (extdFlags == FlatButton_Type)
	{
		surface = getColor(g, FlatButtonSurface, enabled);
		surfaceFlags = Square_Box;
	}
	else
	{
		surface = g.button();
	}
	if (khtmlMode) surfaceFlags |= Draw_AlphaBlend;
	//
	if (enabled)
	{
		if (sunken)
		{
			hilite = sunkenlite;
			if (extdFlags & (Scroller_Type|Slider_Type))
			{
				surface = colorMix(surface, hilite, SRN_DOWN);
			}
			else
				surfaceFlags |= Is_Sunken;
		}
		else if (mouseOver)
		{
			surfaceFlags |= Is_Hovered;
		}
		else if (extdFlags == Default_Button)
		{
			surface = colorMix(g.button(), sunkenlite, SRN_DOWN);
		}
	}
	if (extdFlags & (TabAction_Type|MultiTab_Type|Flat_Type|FlatButton_Type))
	{
		QRect work(r);
		if (extdFlags & (MultiTab_Type|FlatButton_Type))
		{
			int left, top, right, bottom;
			work.coords(&left, &top, &right, &bottom);
			p->setPen(ground);
			p->drawLine(left, top, left, bottom);
			p->drawLine(right, top, right, bottom);
			work.addCoords(1, 0, -1, 0);
		}
		if (extdFlags == MultiTab_Type)
		{
			if (sunken)
				surface = colorMix(surface, hilite, SRN_DOWN);
			else if (mouseOver)
				surface = colorMix(surface, hilite, SRN_OVER);
			//
			renderFlatArea(p, work, ground, surface, Button_Alike);
		}
		else if (extdFlags == FlatButton_Type)
		{
			if (qGray(surface.rgb()) < 64)
				surface = brighter(surface, ADJUSTOR);
			//
			QRect inner(work);
			inner.addCoords(1, 1, -1, -1);
			if (mouseOver || sunken)
				renderSurface(p, inner, ground, surface, hilite, surfaceFlags);
			else
			{
				p->fillRect(inner, QBrush(surface));
				renderSereneContour(p, inner, ground, surface, Square_Box|Serene_Drawing);
			}
			renderSereneContour(p, work, ground, ground, Button_Alike|Is_Sunken|Serene_Drawing);
		}
		else // (extdFlags == Flat_Type)
		{
			if (mouseOver || sunken)
				renderSurface(p, work, ground, surface, hilite, surfaceFlags);
			else
				renderFlatArea(p, work, ground, surface, Button_Alike);
		}
	}
	else if (extdFlags == Header_Type)
	{
		if (sunken)
			surface = colorMix(surface, sunkenlite, SRN_DOWN);
		else if (mouseOver)	/// MXLS: Sorting section.
			surface = colorMix(surface, sunkenlite, SRN_OVER);
		//
		p->fillRect( r, QBrush(surface) );
	}
	else
	{
		renderSurface(p, r, ground, surface, hilite, surfaceFlags);
	}
	p->setPen(oldPen);
}

void SerenityStyle::renderSereneEditFrame(QPainter* p, 
					const QRect &r, 
					const QColorGroup &g, 
					const bool enabled, 
					const bool focused, 
					const bool khtml) const
{
	QColor ground;
	uint contourFlags = Button_Alike;
	if (khtml)
	{
		contourFlags |= Draw_AlphaBlend;
		ground = g.background();
	}
	else
	{
		ground = getColor(g, DefaultGround);
	}
	QColor contour = getColor(g, FrameContour, enabled);
	QColor surface = g.base();
	if (_textboxHilite && focused)
	{
		contour = colorMix(contour, g.highlight(), SRN_DOWN);
		surface = colorMix(surface, g.highlight(), SRN_DOWN);
	}
	//
	/// MXLS: Don't fill to avoid huge slow downs!
	p->setPen(surface);
	p->drawRect(r.left()+1, r.top()+1, r.width()-2, r.height()-2);
	renderContour(p, r, ground, contour, contourFlags);
}

void SerenityStyle::renderGrip(QPainter* p, 
				const QRect &r, 
				const QColorGroup &g, 
				const QColor &color, 
				const bool horizontal, 
				const bool sunken, 
				const bool mouseOver, 
				const bool enabled, 
				const bool thick) const
{
	int minimum = (thick ? 3 : 2);
	if ( (r.width() < minimum) || (r.height() < minimum) )
		return;
	
	// Re-calculated colors
	QColor baseColor = color;
	if (enabled)
	{
		if (sunken)
			baseColor = colorMix(baseColor, g.highlight(), SRN_DOWN);
		else if (mouseOver)
			baseColor = colorMix(baseColor, getColor(g, HoverColor), SRN_OVER);
	}
	QColor borderTop = borderColor(baseColor, TOP, CONCAVE);
	QColor borderBottom = borderColor(baseColor, BOTTOM, CONCAVE);
	
	int left, top, right, bottom;
	r.coords(&left, &top, &right, &bottom);
	int width = r.width()-2;
	int height = r.height()-2;
	
	if (horizontal)
	{
		int y = r.center().y();
		bottom = y+1;
		if (thick)
		{
			top = y-1;
			p->setPen(baseColor);
			p->drawLine(left+1, y, right-1, y);
			p->setPen(borderTop);
			p->drawPoint(left, y);
			p->setPen(borderBottom);
			p->drawPoint(right, y);
		}
		else
		{
			top = y;
		}
		renderGradLine(p, QRect(left+1, top, width, 1),
				borderTop, baseColor, 
				HorizontalGradient);
		renderGradLine(p, QRect(left+1, bottom, width, 1),
				baseColor, borderBottom, 
				HorizontalGradient);
	}
	else	// Vertical
	{
		int x = r.center().x();
		if (thick)
		{
			left = x-1;
			right = x+1;
			p->setPen(baseColor);
			p->drawLine(x, top+1, x, bottom-1);
			p->setPen(borderTop);
			p->drawPoint(x, top);
			p->setPen(borderBottom);
			p->drawPoint(x, bottom);
		}
		else
		{
			left = x;
			right = x+1;
		}
		renderGradLine(p, QRect(left, top+1, 1, height), 
				borderTop, baseColor, 
				VerticalGradient);
		renderGradLine(p, QRect(right, top+1, 1, height),
				baseColor, borderBottom, 
				VerticalGradient);
	}
	// Square corners
	p->setPen( colorMix(baseColor, borderTop) );
	p->drawPoint(left, top);
	//
	p->setPen( colorMix(baseColor, borderBottom) );
	p->drawPoint(right, bottom);
}

void SerenityStyle::renderSereneTab(QPainter* p, 
				const QRect &r, 
				const QColorGroup &g, 
				const uint flags, 
				const bool isBottom, 
				const TabPosition pos, 
				const bool triangular/*, 
				const bool cornerWidget*/) const
{
	const bool isFirst = (pos == First) || (pos == Single);
	const bool isLast = (pos == Last);
	const bool isSingle = (pos == Single);
	//
	const bool enabled = flags & Style_Enabled;
	const bool selected = flags & Style_Selected;
	const bool mouseOver = flags & Style_MouseOver;
	//
	uint surfaceFlags = Square_Box|((mouseOver && !selected) ? Is_Hovered : 0);
	if (isSingle)
	{
		surfaceFlags |= (isBottom ? Round_Bottom : Round_Top);
	}
	else if (isFirst)
	{
		surfaceFlags |= (isBottom
				? (_reverseLayout ? Round_BottomRight : Round_BottomLeft) 
				:(_reverseLayout ? Round_UpperRight : Round_UpperLeft) );
	}
	else if (isLast)
	{
		surfaceFlags |= (isBottom
				? (_reverseLayout ? Round_BottomLeft : Round_BottomRight) 
				: (_reverseLayout ? Round_UpperLeft : Round_UpperRight) );
	}
	
	QColor ground = getColor(g, DefaultGround);
	QColor surface = getColor(g, TabGroove_Solid, enabled);
	QColor sepColor = surface;
	QColor contour = getColor(g, PanelContour, enabled);
	
	int left, top, right, bottom;
	r.coords(&left, &top, &right, &bottom);
	int width, height;
	r.rect(&left, &top, &width, &height);
	//
	int diff = (triangular ? 2 : 0);	// Enlarge separators if triangular.
	int push = (_splitTabs ? 3 : 5);	// Enlarge active tab if split groove.
	
	if (!isBottom)
	{
		// TabBar groove
		QRect tab;
		if (_splitTabs)
		{
			tab = QRect(left+2, top+1, width-4, height-1);
			surfaceFlags |= Round_Top;
		}
		else
		{
			tab = QRect(left, top+1, width, height-1);
		}
		if (! (selected && _splitTabs))	/// MXLS: Don't touch this. It works!
		{
			if (_passiveTabStyle <= GradPaTab)
			{
				sepColor = ground;
				renderBicoloreGradient(p, tab, g, 
							ground, 
							getColor(g, TabGroove_GradTop, enabled), 
							ground, 
							getColor(g, TabGroove_GradBottom, enabled), 
							surfaceFlags, 
							(_passiveTabStyle == GradPaTab));
			}
			else
			{
				if (mouseOver && !selected)
				{
					surface = colorMix(surface, getColor(g, HoverColor), SRN_OVER);
				}
				renderFlatArea(p, tab, ground, surface, surfaceFlags);
			}
		}
		p->setPen(contour);
		p->drawLine(left, bottom, right, bottom);
		//
		if ( (!isSingle) && (!_splitTabs) )
		{
			// The separators
			if ( (!_reverseLayout && !isFirst) || (_reverseLayout && !isLast) )
			{
				renderGradLine(p, QRect(left, top+7-diff, 1, bottom-top-11 + diff*2), 
						sepColor, 
						borderColor(sepColor, BOTTOM, CONCAVE), 
						VerticalGradient);
			}
			if ( (!_reverseLayout && !isLast) || (_reverseLayout && !isFirst) )
			{
				renderGradLine(p, QRect(right, top+7-diff, 1, bottom-top-11 + diff*2), 
						borderColor(sepColor, TOP, CONCAVE), 
						sepColor, 
						VerticalGradient);
			}
		}
		if (selected)
		{
			left += push;
			right -= push;
			width -= push*2;
			//
			if (_activeTabStyle <= HilitedTab)
			{
				if (_activeTabStyle == InvGradTab) 
					surface = getColor(g, TabGroove_GradBottom);
				else
					surface = gradientColor(ground, TOP);
				//
				int h1 = (height-1)/2;
				int h2 = (height-1) - h1;
				renderGradient(p, QRect(left, top+1, width, h1), 
						(_activeTabStyle != HilitedTab) 
							? surface 
							: colorMix(surface, g.highlight(), SRN_DOWN), 
						ground, 
						VerticalGradient);
				p->fillRect(left, top+1+h1, width, h2, QBrush(ground));
			}
			else
			{
				p->fillRect(left, top+1, width, height-1, QBrush(ground));
			}
			renderContour(p, QRect(left-1, top, width+2, height),
					ground, contour, TopTab_Alike);
			p->setPen(ground);
			p->drawPoint(left-1, bottom);
			p->drawPoint(right+1, bottom);
			p->setPen(contour);
			p->drawPoint(left-2, bottom-1);
			p->drawPoint(right+2, bottom-1);
			p->setPen( colorMix(ground, contour) );
			p->drawLine(left-2, bottom, left-1, bottom-1);
			p->drawLine(right+2, bottom, right+1, bottom-1);
		}
	}
	else	/// Bottom tabs
	{
		// TabBar groove
		QRect tab;
		if (_splitTabs)
		{
			tab = QRect(left+2, top, width-4, height-1);
			surfaceFlags |= Round_Bottom;
		}
		else
		{
			tab = QRect(left, top, width, height-1);
		}
		if (! (selected && _splitTabs))	/// MXLS: Don't touch this. It works!
		{
			if (_passiveTabStyle <= GradPaTab)
			{
				surface = ground;
				/// Bottom tabs are inverted in comparison to top tabs.
				renderBicoloreGradient(p, tab, g,  
							ground, 
							getColor(g, TabGroove_GradTop), 
							ground, 
							getColor(g, TabGroove_GradBottom), 
							surfaceFlags, 
							(_passiveTabStyle == InvGradPaTab));
			}
			else
			{
				if (mouseOver && !selected)
				{
					surface = colorMix(surface, getColor(g, HoverColor), SRN_OVER);
				}
				renderFlatArea(p, tab, ground, surface, surfaceFlags);
			}
		}
		p->setPen(contour);
		p->drawLine(left, top, right, top);
		//
		if ( (!isSingle) && (!_splitTabs) )
		{
			// The separators
			if ( (!_reverseLayout && !isFirst) || (_reverseLayout && !isLast) )
			{
				renderGradLine(p, QRect(left, top+7-diff, 1, bottom-top-11 + diff*2), 
						sepColor, 
						borderColor(sepColor, BOTTOM, CONCAVE), 
						VerticalGradient);
			}
			if ( (!_reverseLayout && !isLast) || (_reverseLayout && !isFirst) )
			{
				renderGradLine(p, QRect(right, top+7-diff, 1, bottom-top-11 + diff*2), 
						borderColor(sepColor, TOP, CONCAVE), 
						sepColor, 
						VerticalGradient);
			}
		}
		if (selected)
		{
			left += push;
			right -= push;
			width -= push*2;
			//
			if (_activeTabStyle <= HilitedTab)
			{
				if (_activeTabStyle == InvGradTab) 
 					surface = getColor(g, TabGroove_GradBottom);
				else
					surface = gradientColor(ground, TOP);
				//
				int h1 = (height-1)/2;
				int h2 = (height-1) - h1;
				p->fillRect(left, top, width, h1, QBrush(ground));
				renderGradient(p, QRect(left, top+h1, width, h2), 
						ground, 
						(_activeTabStyle != HilitedTab) 
							? surface 
							: colorMix(surface, g.highlight(), SRN_DOWN), 
						VerticalGradient);
			}
			else
			{
				p->fillRect(left, top, width, height-1, QBrush(ground));
			}
			renderContour(p, QRect(left-1, top, width+2, height),
					ground, contour, BottomTab_Alike);
			p->setPen(ground);
			p->drawPoint(left-1, top);
			p->drawPoint(right+1, top);
			p->setPen(contour);
			p->drawPoint(left-2, top+1);
			p->drawPoint(right+2, top+1);
			p->setPen( colorMix(ground, contour) );
			p->drawLine(left-2, top, left-1, top+1);
			p->drawLine(right+2, top, right+1, top+1);
		}
	}
}

void SerenityStyle::renderHardMap(QPainter* p, 
				const QRect &r, 
				const QColor foreColor, 
				const BitmapItem littleMap) const
{
	QBitmap rough;
	int dx, dy;
	switch (littleMap)
	{
		case DOCKCLOSE:
		{
			rough = dockClose;
			dx = 3;
			dy = 3;
			break;
		}
		case DOCKIN:
		{
			rough = dockIn;
			dx = 3;
			dy = 3;
			break;
		}
		case DOCKOUT:
		{
			rough = dockOut;
			dx = 3;
			dy = 3;
			break;
		}
		case DOCKSTAY:
		{
			rough = dockStay;
			dx = 3;
			dy = 3;
			break;
		}
		case TOOLPLUS:
		{
			rough = toolPlus;
			dx = 3;
			dy = 3;
			break;
		}
		case APPLETMENU:
		{
			rough = appletMenu;
			dx = 2;
			dy = 2;
			break;
		}
		case MODLEFTDECO:
		{
			rough = modLeftDeco;
			dx = 7;		// Misaligned on purpose!
			dy = 5;
			break;
		}
		case MODCOLLAPSED:
		{
			rough = modCollapsed;
			dx = 3;		// Misaligned on purpose!
			dy = 5;
			break;
		}
		case MODTOPDECO:
		{
			rough = modTopDeco;
			dx = 5;
			dy = 7;		// Misaligned on purpose!
			break;
		}
		case MODEXPANDED:
		{
			rough = modExpanded;
			dx = 5;
			dy = 3;		// Misaligned on purpose!
			break;
		}
		case MODDOT:
		{
			rough = modDot;
			dx = 2;
			dy = 2;
			break;
		}
		case MINILEFT:
		{
			rough = miniLeft;
			dx = 2;
			dy = 2;
			break;
		}
		case MINIRIGHT:
		{
			rough = miniRight;
			dx = 2;
			dy = 2;
			break;
		}
		case MINIUP:
		{
			rough = miniUp;
			dx = 2;
			dy = 2;
			break;
		}
		case MINIDOWN:
		{
			rough = miniDown;
			dx = 2;
			dy = 2;
			break;
		}
		case ARROWLEFT:
		{
			rough = littleLeft;
			dx = 2;
			dy = 3;
			break;
		}
		case ARROWRIGHT:
		{
			rough = littleRight;
			dx = 2;
			dy = 3;
			break;
		}
		case ARROWUP:
		{
			rough = littleUp;
			dx = 3;
			dy = 2;
			break;
		}
		case ARROWDOWN:
		default:
		{
			rough = littleDown;
			dx = 3;
			dy = 2;
			break;
		}
	}
	QPoint center = r.center();
	p->setPen(foreColor);
	p->drawPixmap(center.x()-dx, center.y()-dy, rough);
}

void SerenityStyle::renderSoftMap(QPainter* p, 
				const QRect &r, 
				const QColor backColor, 
				const QColor foreColor, 
				const BitmapItem bigMap) const
{
	QBitmap rough, alias;
	int dx, dy;
	switch (bigMap)
	{
		case CHECKMARK:
		{
			rough =  checkmark_rough;
			alias =  checkmark_alias;
			dx = 4;
			dy = 4;
			break;
		}
		case TRISTATE:
		{
			rough =  tristate_rough;
			alias =  tristate_alias;
			dx = 4;
			dy = 4;
			break;
		}
		case RADIOMARK:
		{
			rough =  radiomark_rough;
			alias =  radiomark_alias;
			dx = 3;
			dy = 3;
			break;
		}
		case TINY_RADIOMARK:
		{
			rough =  tiny_radiomark_rough;
			alias =  tiny_radiomark_alias;
			dx = 2;
			dy = 2;
			break;
		}
		case COLLAPSED:
		{
			rough =  collapsed_rough;
			alias =  collapsed_alias;
			dx = 2;		// Misaligned on purpose!
			dy = 4;
			break;
		}
		case EXPANDED:
		{
			rough =  expanded_rough;
			alias =  expanded_alias;
			dx = 4;
			dy = 3;
			break;
		}
		case BIGPLUS:
		{
			rough =  bigPlus;
			alias =  bigPlus_alias;
			dx = 4;
			dy = 4;
			break;
		}
		case BIGMINUS:
		{
			rough =  bigMinus;
			alias =  bigMinus_alias;
			dx = 4;
			dy = 1;
			break;
		}
		case ARROWLEFT:
		{
			rough =  bigLeft;
			alias =  bigLeft_alias;
			dx = 3;
			dy = 4;
			break;
		}
		case ARROWRIGHT:
		{
			rough =  bigRight;
			alias =  bigRight_alias;
			dx = 2;
			dy = 4;
			break;
		}
		case ARROWUP:
		{
			rough =  bigUp;
			alias =  bigUp_alias;
			dx = 4;
			dy = 3;
			break;
		}
		case ARROWDOWN:
		default:
		{
			rough =  bigDown;
			alias =  bigDown_alias;
			dx = 4;
			dy = 2;
			break;
		}
	}
	QPoint center = r.center();
	int x = center.x() - dx;
	int y = center.y() - dy;
	p->setPen(foreColor);
	p->drawPixmap(x, y, rough);
	p->setPen( colorMix(backColor, foreColor, 144) );
	p->drawPixmap(x, y, alias);
}

void SerenityStyle::drawKStylePrimitive(KStylePrimitive kpe,
					QPainter* p,
					const QWidget* widget,
					const QRect &r,
					const QColorGroup &cg,
					SFlags flags,
					const QStyleOption& opt) const
{
	const bool enabled = (flags & Style_Enabled);
	
	int left, top, width, height;
	r.rect(&left, &top, &width, &height);
	int right, bottom;
	r.coords(&left, &top, &right, &bottom);
	
	switch (kpe)
	{
		case KPE_ToolBarHandle:
		case KPE_GeneralHandle:
		{
			/// MXLS: Never seen!
			renderSurface(p, r, cg.background(), cg.background(), cg.background(), 
						Square_Box);
		}
		case KPE_SliderGroove:
		{
			const QSlider* slider = (const QSlider*)widget;
			bool horizontal = slider->orientation() == Horizontal;
			QRect work = slider->sliderRect();
			QPoint center = work.center();
			int x = center.x();
			int y = center.y();
			//
			QRect groove(r);
			QRect back, fill;
			int space;
			int size = _wideSlider ? pixelMetric(PM_SliderThickness)-2 : 7 ;
			if (horizontal)
			{
				space = (height-size)/2;
				groove.addCoords(1, space, -1, -space);
				back = fill = groove;
				if (_reverseLayout)
				{
					back.setRight(x);
					fill.setLeft(x);
				}
				else
				{
					back.setLeft(x);
					fill.setRight(x);
				}
			}
			else
			{
				space = (width-size)/2;
				groove.addCoords(space, 1, -space, -1);
				back = fill = groove;
				back.setBottom(y);
				fill.setTop(y);
			}
			back.addCoords(1, 1, -1, -1);
			p->fillRect( back, QBrush(cg.base()) ); 
			fill.addCoords(1, 1, -1, -1);
			p->fillRect( fill, QBrush( colorMix(cg.highlight(), cg.base()) ) ); 
			renderContour(p, groove, getColor(cg, DefaultGround), 
					getColor(cg, FrameContour, enabled), 
					Button_Alike);
			break;
		}
		case KPE_SliderHandle:
		{
			const QSlider* slider = (const QSlider*)widget;
			bool horizontal = slider->orientation() == Horizontal;
			const bool pressed = (flags&Style_Active);
			QColor surface = getColor(cg, ScrollerSurface, enabled);
	
			bool handleHighlighted = ( (!pressed) && (hoverWidget == widget) ) 
						? true : false;
			QRect handle(r);
			if (_wideSlider)
				handle.addCoords(2, 2, -2, -2); // Handle must not overwrite the frame.
			else
				handle.addCoords(1, 1, -1, -1); // Nor the focus rectangle.
			//
			QRect grip(handle);
			QRect arrowR1, arrowR2;
			BitmapItem arrowT1, arrowT2;
			if (horizontal)
			{
				grip.addCoords(0, 2, 0, -2);
				arrowR1 = QRect(handle.left()+1, handle.top(),
						7, handle.height());
				arrowR2 = arrowR1;
				arrowR2.moveLeft(handle.right()-7);
				arrowT1 = MINILEFT;
				arrowT2 = MINIRIGHT;
			}
			else
			{
				grip.addCoords(2, 0, -2, 0);
				arrowR1 = QRect(handle.left(), handle.top()+1,
						handle.width(), 7);
				arrowR2 = arrowR1;
				arrowR2.moveTop(handle.bottom()-7);
				arrowT1 = MINIUP;
				arrowT2 = MINIDOWN;
			}
			renderButton(p, handle, cg, pressed, handleHighlighted, 
					enabled, !ALPHA, Slider_Type);
			renderGrip(p, grip, cg, surface, !horizontal,	// Grip is perpendicular.
					pressed, handleHighlighted, enabled);
			if (pressed)
				surface = colorMix(surface, cg.highlight(), SRN_DOWN);
			else if (handleHighlighted)
				surface = colorMix(surface, getColor(cg, HoverColor), SRN_OVER);
			QColor arrowColor = colorMix(surface, getColor(cg, ScrollerText), 112);
			renderHardMap(p, arrowR1, arrowColor, arrowT1);
			renderHardMap(p, arrowR2, arrowColor, arrowT2);
			break;
		}
		case KPE_ListViewExpander:
		{
			bool collapsed = flags & Style_On;
			/// MXLS: Warning: KDE doesn't set the flag Style_Enabled!
			/// And neither does Serenity...
			if (_treeViewStyle < 3)
			{
				renderButton(p, r, cg, !collapsed, !HOVERED, ENABLED, ALPHA, 
						Expander_Type);
				QColor surface = getColor(cg, ExpanderColor);
				BitmapItem symbol;
				if (collapsed)
				{
					symbol = _drawTriangularExpander 
							? COLLAPSED : BIGPLUS;
				}
				else
				{
					symbol = _drawTriangularExpander 
							? EXPANDED : BIGMINUS;
					surface = colorMix(surface, cg.highlight(), SRN_DOWN);
				}
				renderSoftMap(p, r, surface, 
						colorMix(surface,
							collapsed ? cg.text() : cg.highlightedText(),
							96),
						symbol);
			}
			else	// (_treeViewStyle == "Modern")
			{
				BitmapItem deco, symbol;
				QColor surface;
				if (collapsed)
				{
					deco = MODLEFTDECO;
					symbol = MODCOLLAPSED;
					surface = getColor(cg, ModernCollapsed);
				}
				else
				{
					deco = MODTOPDECO;
					symbol = MODEXPANDED;
					surface = getColor(cg, ModernExpanded);
				}
				renderHardMap(p, r, getColor(cg, TreeBranchColor), deco);
				renderHardMap(p, r, surface, symbol);
			}
			break;
		}
		case KPE_ListViewBranch:
		{
			if (_treeViewStyle == 2)	// No branches at all. (Safety measure.)
				break;
			//
			int left, top, right, bottom;
			r.coords(&left, &top, &right, &bottom);
			if (_treeViewStyle == 1)
			{
				if ( (r.width() > r.height()) && (r.height() == 1) )
				{
					p->setPen( getColor(cg, TreeBranchColor) );
					p->drawLine(left, top-7, left, top+7);	// Simple level indicator
				}
			}
			else if (_treeViewStyle == 3)
			{
				if ( (r.width() > r.height()) && (r.height() == 1) )
				{
					renderHardMap(p, QRect(left-4, top-4, 9, 9), 
							getColor(cg, ModernDotColor), 
							MODDOT);
				}
			}
			else	// (_treeViewStyle == 0)
			{
				p->setPen( getColor(cg, TreeBranchColor) );
				p->drawLine(left, top, right, bottom);	// Draw all branches
			}
			break;
		}
		default:
			KStyle::drawKStylePrimitive(kpe, p, widget, r, cg, flags, opt);
	}
}

void SerenityStyle::drawPrimitive(PrimitiveElement pe,
				QPainter* p,
				const QRect &r,
				const QColorGroup &cg,
				SFlags flags,
				const QStyleOption &opt ) const
{
	bool down = flags & Style_Down;
	bool on	= flags & Style_On;
	bool on_OR_down = (on || down);
	bool horiz = flags & Style_Horizontal;
	bool enabled = flags & Style_Enabled;
	bool mouseOver = (flags & Style_MouseOver) || (hoverWidget == p->device());
	bool sunken = flags & Style_Sunken;
	bool hasFocus = flags & Style_HasFocus;
	bool khtmlMode = opt.isDefault() ? false : khtmlWidgets.contains(opt.widget());
	//
	int left, top, width, height;
	r.rect(&left, &top, &width, &height);
	int right, bottom;
	r.coords(&left, &top, &right, &bottom);
	
	switch (pe)
	{
		case PE_Separator:
		{
			p->setPen( getColor(cg, SeparatorColor) );
			p->drawLine(left, top, right, top);
			break;
		}
		
		case PE_FocusRect:
		{
			if (_drawFocusRect)
			{
				QColor focusColor = colorMix(cg.highlight(), cg.foreground(), 160);
				p->setPen(focusColor);
				p->drawLine(left+1, top, right-1, top);
				p->drawLine(left, top+1, left, bottom-1);
				p->drawLine(right, top+1, right, bottom-1);
				p->drawLine(left+1, bottom, right-1, bottom);
			}
			break;
		}
		case PE_RubberBand:
		{
			p->save();
			QColor rubberColor = cg.highlight();
			QRect work(r);
			p->setPen( QPen(rubberColor, 1, Qt::SolidLine) );
			if ( (_hiliteRubber) && (work.width()>2) && (work.height()>2) )
			{
				work.addCoords(1, 1, -1, -1);
				p->setRasterOp(Qt::CopyROP);
 				p->drawRect(work);
			}
			p->setRasterOp(Qt::NotROP);
			p->drawRect(r);
			p->restore();
			break;
		}
		
		case PE_HeaderSection:
		{
			// The taskbar buttons are painted with PE_HeaderSection
			// and so are the popup titles in different places.
			bool taskMode = kickerMode;
			bool titleStyle = false;
			bool kmenuStyle = false;
			if ( p->device() && dynamic_cast<QWidget*>(p->device()) )
			{
				QWidget* w = dynamic_cast<QWidget*>(p->device());
				if ( w->parentWidget() && w->parentWidget()->inherits("QPopupMenu") )
						titleStyle = true;
				if ( (kickerMode) && (w->backgroundMode() != PaletteBase) 
				&& (w->backgroundMode() != PaletteBackground) )
						kmenuStyle = true;
				if ( (taskMode) && (w->backgroundMode() == PaletteBase) )
						taskMode = false;
			}
			if (titleStyle || kmenuStyle)
			{
				// Popup titles
				// or KMenu titles (which are disabled. Solution: Patch Kicker!)
				p->fillRect( r, QBrush(getColor(cg, PopupContour, enabled)) );
			}
			else if (taskMode)
			{
				// Taskbar buttons
				QRect work(r);
				work.addCoords(1, 1, -1, -1);
				renderButton(p, work, cg, down, HOVERED, ENABLED, ALPHA, 
						ToolButton_Type);
				p->setPen(cg.foreground());
			}
			else
			{
				// Real headers
				// Sunken flag is set on sorted column...
				// of which the header will be highlighted.
				renderButton(p, r, cg, down, sunken, enabled, !ALPHA, 
						Header_Type);
				if (! opt.isDefault())
				{
					QHeader* hdr = dynamic_cast<QHeader*>(opt.widget());
					QColor surface = getColor(cg, HeaderColor);
					QColor splitter = colorMix(surface, cg.text(), 160);
					p->setPen( colorMix(surface, cg.text(), SRN_LIGHT) );
					if (hdr->orientation() == QHeader::Horizontal)
					{
						p->drawLine(left, bottom, right, bottom);
						int idx = hdr->mapToIndex(hdr->sectionAt(left));
						if ( (idx > 0) || ((idx == 0) && (left > 0)) )
						{
							renderGradLine(p, QRect(left, top+4, 1, height-9), 
							surface, gradientColor(splitter, TOP, CONCAVE), 
							VerticalGradient);
						}
						if (idx < hdr->count()) 
						{
							renderGradLine(p, QRect(right, top+4, 1, height-9), 
							gradientColor(splitter, BOTTOM, CONCAVE), surface, 
							VerticalGradient);
						}
					}
					else
					{
						if (_reverseLayout)
							p->drawLine(left, top, left, bottom);
						else
							p->drawLine(right, top, right, bottom);
						int idx = hdr->mapToIndex(hdr->sectionAt(top));
						if ( (idx > 0) || ((idx == 0) && (top > 0)) )
						{
							renderGradLine(p, QRect(left+5, top, width-10, 1), 
							surface, gradientColor(splitter, TOP, CONCAVE), 
							HorizontalGradient);
						}
						if (idx < hdr->count()) 
						{
							renderGradLine(p, QRect(left+5, bottom, width-10, 1), 
							gradientColor(splitter, BOTTOM, CONCAVE), surface, 
							HorizontalGradient);
						}
					}
				}
			}
			break;
		}
		
		case PE_ButtonBevel:
		case PE_ButtonTool:
		case PE_ButtonDropDown:
		case PE_ButtonCommand:
		{
			renderButton(p, r, cg, on_OR_down, mouseOver, enabled, khtmlMode);
			break;
		}
		
		case PE_ButtonDefault:
		{
			renderButton(p, r, cg, on_OR_down, mouseOver, enabled, khtmlMode, 
					Default_Button);
			break;
		}
		
		case PE_ScrollBarAddPage:
		case PE_ScrollBarSubPage:
		{
			QColor groove = getColor(cg, ScrollerGroove, enabled);
			//
			if (on_OR_down)
			{
				p->fillRect(r, QBrush(colorMix(groove, 
							cg.highlight(), SRN_DOWN)) );
			}
			else
			{
				p->fillRect(r, QBrush(groove) );
			}
			if (pe == PE_ScrollBarAddPage)
			{
				if (SBSTF == 2)
				{
					if (horiz)
					{
						renderGradLine(p, QRect(right, top+3, 1, bottom-top-6), 
								borderColor(groove, TOP, CONCAVE), 
								groove, 
								VerticalGradient);
					}
					else
					{
						renderGradLine(p, QRect(left+3, bottom, right-left-6, 1), 
								borderColor(groove, TOP, CONCAVE), 
								groove, 
								HorizontalGradient);
					}
				}
			}
			else if (pe == PE_ScrollBarSubPage)
			{
				if (SBSTF > 0)
				{
					if (horiz)
					{
						renderGradLine(p, QRect(left, top+3, 1, bottom-top-6), 
								groove, 
								borderColor(groove, BOTTOM, CONCAVE), 
								VerticalGradient);
					}
					else
					{
						renderGradLine(p, QRect(left+3, top, right-left-6, 1), 
								groove, 
								borderColor(groove, BOTTOM, CONCAVE), 
								HorizontalGradient);
					}
				}
			}
			if (_drawFocusRect && hasFocus)	// Never seen so far...
			{
				if ( (width>7) && (height>7) )
				{
					QRect fr(r);
					fr.addCoords(3, 3, -3, -3);
					renderContour(p, fr, groove, 
							colorMix(cg.highlight(), cg.foreground(), 160),
							Square_Box);
				}
			}
			break;
		}
		case PE_ScrollBarSlider:
		{
			mouseOver = (flags & Style_MouseOver);
			QColor groove = getColor(cg, ScrollerGroove);
			//
			if (!enabled)
			{
				p->fillRect(r, QBrush(groove) );
				if (SBSTF > 0)
				{
					if (horiz)
					{
						renderGradLine(p, QRect(left, top+3, 1, bottom-top-6), 
								groove, 
								borderColor(groove, BOTTOM, CONCAVE), 
								VerticalGradient);
					}
					else
					{
						renderGradLine(p, QRect(left+3, top, right-left-6, 1), 
								groove, 
								borderColor(groove, BOTTOM, CONCAVE), 
								HorizontalGradient);
					}
				}
			}
			else
			{
				QColor surface = getColor(cg, ScrollerSurface);
				/// MXLS: Drawing is buffered
				/// because the grip disappears sometimes.
				QPixmap buffer;
				buffer.resize(width, height);
				QRect br(buffer.rect());
				QPainter bp(&buffer);
				bp.fillRect(br, QBrush(groove));
				QRect grip, arrowR1, arrowR2;
				BitmapItem arrowT1, arrowT2;
				if (horiz)
				{
					br.addCoords(0, 1, 0, -1);
					grip = br;
					grip.addCoords(0, 2, 0, -2);
					arrowR1 = QRect(br.left()+1, br.top(),
							7, br.height());
					arrowR2 = arrowR1;
					arrowR2.moveLeft(br.right()-7);
					arrowT1 = ARROWLEFT;
					arrowT2 = ARROWRIGHT;
				}
				else
				{
					br.addCoords(1, 0, -1, 0);
					grip = br;
					grip.addCoords(2, 0, -2, 0);
					arrowR1 = QRect(br.left(), br.top()+1,
							br.width(), 7);
					arrowR2 = arrowR1;
					arrowR2.moveTop(br.bottom()-7);
					arrowT1 = ARROWUP;
					arrowT2 = ARROWDOWN;
				}
				renderButton(&bp, br, cg, on_OR_down, mouseOver, 
						ENABLED, ALPHA, Scroller_Type);
				renderGrip(&bp, grip, cg, surface, !horiz,	// Grip is perpendicular.
						on_OR_down, mouseOver);
				if (SBSTF == -1)
				{
					if (on_OR_down)
						surface = colorMix(surface, cg.highlight(), SRN_DOWN);
					else if (mouseOver)
						surface = colorMix(surface, getColor(cg, HoverColor), SRN_OVER);
					QColor arrowColor = colorMix(surface, getColor(cg, ScrollerText), 96);
					renderHardMap(&bp, arrowR1, arrowColor, arrowT1);
					renderHardMap(&bp, arrowR2, arrowColor, arrowT2);
				}
				bp.end();
				p->drawPixmap(r, buffer);
				/// MXLS: TODO: Focus!
			}
			break;
		}
		case PE_ScrollBarSubLine:
		{
			mouseOver = (flags & Style_MouseOver);
			QColor groove = getColor(cg, ScrollerGroove, enabled);
			QColor textColor = getColor(cg, ScrollerText);
			QColor arrowColor = colorMix(groove, textColor, 80);
			//
			p->fillRect(r, QBrush(groove) );
			if (!enabled)
			{
				arrowColor = colorMix(groove, arrowColor);
			}
			else if (on_OR_down)
			{
				renderSurface(p, QRect(left+1, top+1, width-2, height-2), 
							groove, groove, 
							cg.highlight(), 
							Button_Alike|Is_Sunken);
				arrowColor = textColor;
			}
			else if (mouseOver)
			{
				renderSurface(p, QRect(left+1, top+1, width-2, height-2), 
							groove, groove, 
							getColor(cg, HoverColor), 
							Button_Alike|Is_Hovered);
			}
			if (horiz)
			{
				renderSoftMap(p, r, groove, arrowColor, ARROWLEFT);
				// Arrow separator
				renderGradLine(p, QRect(right, top+3, 1, bottom-top-6), 
						borderColor(groove, TOP, CONCAVE), 
						groove, VerticalGradient);
			}
			else
			{
				renderSoftMap(p, r, groove, arrowColor, ARROWUP);
				// Arrow separator
				renderGradLine(p, QRect(left+3, bottom, right-left-6, 1), 
						borderColor(groove, TOP, CONCAVE), 
						groove, HorizontalGradient);
			}
			break;
		}
		case PE_ScrollBarAddLine:
		{
			mouseOver = (flags & Style_MouseOver);
			QColor groove = getColor(cg, ScrollerGroove, enabled);
			QColor textColor = getColor(cg, ScrollerText);
			QColor arrowColor = colorMix(groove, textColor, 80);
			//
			p->fillRect(r, QBrush(groove) );
			if (!enabled)
			{
				arrowColor = colorMix(groove, arrowColor);
			}
			else if (on_OR_down)
			{
				renderSurface(p, QRect(left+1, top+1, width-2, height-2), 
							groove, groove, 
							cg.highlight(), 
							Button_Alike|Is_Sunken);
				arrowColor = textColor;
			}
			else if (mouseOver)
			{
				renderSurface(p, QRect(left+1, top+1, width-2, height-2), 
							groove, groove, 
							getColor(cg, HoverColor), 
							Button_Alike|Is_Hovered);
			}
			if (horiz)
			{
				renderSoftMap(p, r, groove, arrowColor, ARROWRIGHT);
				// Arrow separator
				renderGradLine(p, QRect(left, top+3, 1, bottom-top-6), 
						groove, 
						borderColor(groove, BOTTOM, CONCAVE), 
						VerticalGradient);
			}
			else
			{
				renderSoftMap(p, r, groove, arrowColor, ARROWDOWN);
				// Arrow separator
				renderGradLine(p, QRect(left+3, top, right-left-6, 1), 
						groove, 
						borderColor(groove, BOTTOM, CONCAVE), 
						HorizontalGradient);
			}
			break;
		}
	
		// CHECKBOXES
		// ----------
		case PE_IndicatorMask:
		{
			p->fillRect(r, color1);
			p->setPen(color0);
			p->drawPoint(left, top);
			p->drawPoint(right, top);
			p->drawPoint(left, bottom);
			p->drawPoint(right, bottom);
			break;
		}
		case PE_Indicator:
		{
			mouseOver = (flags & Style_MouseOver);
			QColor ground = cg.background();
			QColor contour = cg.button();
			QColor surface = cg.base();
			QColor hilite = cg.highlight();
			uint surfaceFlags = No_Flags;
			uint contourFlags = Button_Alike|Draw_AlphaBlend;
			if (down || !(flags & Style_Off) )	// (down || on || "tristate_on")
			{
				surfaceFlags |= Is_Sunken;
				contourFlags |= Is_Sunken;
			}
			/*
			/// MXLS: UNDO: No mouseOver effect.
			else if (mouseOver && (hoverThis == HOVER_CHECKRADIO))
			{
				hilite = getColor(cg, HoverColor);
				surfaceFlags |= Is_Hovered;
			}
			*/
			// Surface
			renderSurface(p, QRect(left+1, top+1, width-2, height-2),
						ground, surface, hilite, 
						surfaceFlags);
			// Contour
			renderSereneContour(p, r, ground, contour, contourFlags);
			//
			// And go on with the checkmark...
		}
		case PE_CheckMark:
		{
			if (flags & Style_Off)
			{
				// Nothing
				break;
			} 
			mouseOver = (flags & Style_MouseOver);
			QColor ground = cg.base();
			if (on_OR_down)
			{
				ground = colorMix(ground, cg.highlight(), 
								SRN_DOWN);
			}
			/*
			/// MXLS: UNDO: No mouseOver effect.
			else if (mouseOver && (hoverThis == HOVER_CHECKRADIO))
			{
				ground = colorMix(ground, getColor(cg, HoverColor), 
							SRN_OVER);
			}
			*/
			if (on && !down)
			{
				renderSoftMap(p, r, ground, cg.text(), CHECKMARK);
			} 
			else if (!on && !down)
			{
				renderSoftMap(p, r, ground, cg.text(), TRISTATE);
			}
			break;
		}

		// RADIOBUTTONS
		// ------------
		case PE_ExclusiveIndicatorMask:
		{
			p->fillRect(r, color0);
			p->setPen(color1);
			p->drawPixmap(left, top, radiobutton_mask);
			break;
		}
		case PE_ExclusiveIndicator:
		{
			mouseOver = (flags & Style_MouseOver);
			QColor ground = cg.background();
			QColor contour = cg.button();
			QColor surface = cg.base();
			QColor hilite = cg.highlight();
			bool convex = true;
			uint surfaceFlags = No_Flags;
			if (on_OR_down)
			{
				convex = false;
				surfaceFlags |= Is_Sunken|Keep_Color;
				surface = colorMix(surface, hilite, SRN_DOWN);
			}
			/*
			/// MXLS: UNDO: No mouseOver effect.
			else if (mouseOver && (hoverThis == HOVER_CHECKRADIO))
			{
				hilite = getColor(cg, HoverColor);
				surfaceFlags |= Is_Hovered;
			}
			*/
			// Surface
			QRect inner(r);
			inner.addCoords(1, 1, -1, -1);
			if (_globalStyle != Zen)
			{
				p->setClipRegion(radiobutton_mask);
				renderSurface(p, inner,
						ground, surface, hilite, 
						surfaceFlags);
				p->setClipRegion(radio_inner_alias);
				renderSurface(p, inner, ground, 
							colorMix(surface, contour), 
							hilite, 
							surfaceFlags);
				p->setClipRegion(radio_contour_rough);
				renderDiagonalGradient(p, r, 
							radioBorderColor(contour, TOP), 
							contour, 
							radioBorderColor(contour, BOTTOM), 
							convex);
				QColor color = colorMix(ground, contour, 144);
				p->setClipRegion(radio_outer_alias);
				renderDiagonalGradient(p, r, 
							radioBorderColor(color, TOP), 
							color, 
							radioBorderColor(color, BOTTOM), 
							convex);
			}
			else	// (_globalStyle == Zen)
			{
				p->setClipRegion(radiobutton_mask);
				renderZenCircle(p, inner,
						ground, surface, hilite, 
						surfaceFlags);
				p->setClipRegion(radio_inner_alias);
				renderZenCircle(p, inner, ground, 
							colorMix(surface, contour), 
							hilite, 
							surfaceFlags);
				p->setClipRegion(radio_contour_rough);
				renderCircleGradient(p, r, 
							radioBorderColor(contour, TOP), 
							contour, 
							radioBorderColor(contour, BOTTOM), 
							convex);
				QColor color = colorMix(ground, contour, 144);
				p->setClipRegion(radio_outer_alias);
				renderCircleGradient(p, r, 
							radioBorderColor(color, TOP), 
							color, 
							radioBorderColor(color, BOTTOM), 
							convex);
			}
			p->setClipping(false);
			// Radio mark
			if (flags & Style_Off)
			{
				// Nothing
				break;
			} 
			if (on && !down)
			{
				renderSoftMap(p, r, surface, cg.text(), RADIOMARK);
			}
			break;
		}
		
		// CHECKLIST ITEMS
		// ---------------
		case PE_CheckListController:
		{
			QCheckListItem* item = opt.checkListItem();
			if (!item)
				break;
			QListView* lv = item->listView();
			int marg = lv->itemMargin();
			
			QRect outer;
			if (height > width)	// To be on the safe side.
				outer = QRect(left, top + (height-width)/2, width, width);
			else
				outer = QRect(left, top, height, height);
			QRect inner = outer;
			inner.addCoords(1, 1, -1, -1);
			
			QColor ground = cg.base();
			QColor hilite = cg.highlight();
			QColor surface = colorMix(getColor(cg, ExpanderColor),
							hilite, SRN_OVER);
			QColor markColor = colorMix(cg.text(), hilite, SRN_DOWN);
			if (flags & Style_Selected)
			{
				p->fillRect( 0, 0, left + marg + width + 4, height,
						QBrush(hilite) );
				ground = hilite;
			}
			if (item->childCount() == 0)
			{
				// This is a child or a standalone.
				renderSurface(p, inner, ground, 
							surface, hilite, 
							Button_Alike);
				renderSoftMap(p, outer, surface, 
						markColor, RADIOMARK);
			}
			else	// (_globalStyle == Zen)
			{
				// This is a parent.
				QColor contour = colorMix(getColor(cg, FrameContour), hilite, SRN_OVER);
				/// MXLS: Bufferized drawing otherwise it won't work.
				QPixmap buffer;
				buffer.resize(15, 15);
				QRect work(buffer.rect());
				QPainter bp(&buffer);
				bp.fillRect(work, QBrush(ground));
				if (_globalStyle != Zen)
				{
					bp.setClipRegion(radiobutton_mask);
					renderSurface(&bp, work,
							ground, surface, hilite, 
							No_Flags);
					bp.setClipRegion(radio_inner_alias);
					renderSurface(&bp, work, ground, 
								colorMix(surface, contour), 
								hilite, 
								No_Flags);
					bp.setClipRegion(radio_contour_rough);
					renderDiagonalGradient(&bp, r, 
								radioBorderColor(contour, TOP), 
								contour, 
								radioBorderColor(contour, BOTTOM), 
								RAISED);
					QColor color = colorMix(ground, contour, 144);
					bp.setClipRegion(radio_outer_alias);
					renderDiagonalGradient(&bp, r, 
								radioBorderColor(color, TOP), 
								color, 
								radioBorderColor(color, BOTTOM), 
								RAISED);
				}
				else	// (_globalStyle == Zen)
				{
					bp.setClipRegion(radiobutton_mask);
					renderZenCircle(&bp, work,
							ground, surface, hilite, 
							No_Flags);
					bp.setClipRegion(radio_inner_alias);
					renderZenCircle(&bp, work, ground, 
								colorMix(surface, contour), 
								hilite, 
								No_Flags);
					bp.setClipRegion(radio_contour_rough);
					renderCircleGradient(&bp, r, 
								radioBorderColor(contour, TOP), 
								contour, 
								radioBorderColor(contour, BOTTOM), 
								RAISED);
					QColor color = colorMix(ground, contour, 144);
					bp.setClipRegion(radio_outer_alias);
					renderCircleGradient(&bp, r, 
								radioBorderColor(color, TOP), 
								color, 
								radioBorderColor(color, BOTTOM), 
								RAISED);
				}
				bp.setClipping(false);
				bp.end();
				inner = QRect(left, top + (height-15)/2, 15, 15);
				p->drawPixmap(inner, buffer);
				// Symbol is an arrow down.
				renderSoftMap(p, inner, surface, markColor, ARROWDOWN);
			}
			break;
		}
		case PE_CheckListIndicator:
		{
			QCheckListItem* item = opt.checkListItem();
			if (!item)
				break;
			QListView* lv = item->listView();
			int marg = lv->itemMargin();
			//
			bool noChange = (flags & Style_NoChange);
			QRect outer(left, top, width, width);
			QRect inner = outer;
			inner.addCoords(1, 1, -1, -1);
			//
			QColor ground = cg.base();
			QColor surface = getColor(cg, ExpanderColor);
			QColor hilite = cg.highlight();
			if (flags & Style_Selected)
			{
				p->fillRect( 0, 0, left + marg + width + 4, height,
						QBrush(hilite) );
				ground = hilite;
			}
			// Button is raised unless checked (on).
			QColor markColor = cg.text();
			QColor contour;
			uint surfaceFlags = No_Flags;
			if (on)
			{
				// Highlight the surface but not the border.
				surfaceFlags |= Is_Sunken|Keep_Color;
				surface = colorMix(surface, hilite, SRN_DOWN);
			}
			if (item->childCount() == 0)
			{
				// This is a child or a standalone.
				outer.addCoords(1, 1, -1, -1);
				inner.addCoords(1, 1, -1, -1);
				contour = cg.button();
			}
			else
			{
				// This is a controller/parent.
				contour = getColor(cg, FrameContour);
				if (noChange)
				{
					// Highlight another way.
					contour = colorMix(surface, hilite, SRN_DOWN);
					markColor = colorMix(markColor, hilite, SRN_DOWN);
				}
			}
			// Surface
			renderSurface(p, inner, 
					ground, surface, hilite, 
					surfaceFlags);
			// Contour
			renderSereneContour( p, outer, ground, contour, 
						Button_Alike|
						((on) ? Is_Sunken : 0) );
			if (noChange)
			{
				renderSoftMap(p, inner, surface, markColor, ARROWDOWN);
			}
			else if (on)
			{
				renderSoftMap(p, inner, surface, markColor, CHECKMARK);
			}
			break;
		}
		case PE_CheckListExclusiveIndicator:
		{
			QCheckListItem* item = opt.checkListItem();
			if (!item)
				break;
			QListView* lv = item->listView();
			int marg = lv->itemMargin();
			
			QColor ground = cg.base();
			QColor surface = getColor(cg, ExpanderColor);
			QColor hilite = cg.highlight();
			QColor contour = cg.button();
			if (flags & Style_Selected)
			{
				p->fillRect( 0, 0, left + marg + width + 4, height,
						QBrush(hilite) );
				ground = hilite;
			}
			bool convex = true;
			uint surfaceFlags = No_Flags;
			if (on)
			{
				convex = false;
				surfaceFlags |= Is_Sunken|Keep_Color;
				surface = colorMix(surface, hilite, SRN_DOWN);
			}
			/// MXLS: Bufferized drawing or it won't work.
			QPixmap buffer;
			buffer.resize(13, 13);
			QRect work(buffer.rect());
			QRect inner(work);
			inner.addCoords(1, 1, -1, -1);
			QPainter bp(&buffer);
			bp.fillRect(work, QBrush(ground));
			if (_globalStyle != Zen)
			{
				bp.setClipRegion(tiny_radiobutton_mask);
				renderSurface(&bp, inner,
							ground, surface, hilite, 
							surfaceFlags);
				bp.setClipRegion(tiny_radio_contour_rough);
				renderDiagonalGradient(&bp, work, 
							radioBorderColor(contour, TOP), 
							contour, 
							radioBorderColor(contour, BOTTOM), 
							convex);
				QColor color = colorMix(ground, contour, 144);
				bp.setClipRegion(tiny_radio_outer_alias);
				renderDiagonalGradient(&bp, work, 
							radioBorderColor(color, TOP), 
							color, 
							radioBorderColor(color, BOTTOM), 
							convex);
			}
			else	// (_globalStyle == Zen)
			{
				bp.setClipRegion(tiny_radiobutton_mask);
				renderZenCircle(&bp, inner,
							ground, surface, hilite, 
							surfaceFlags);
				bp.setClipRegion(tiny_radio_contour_rough);
				renderCircleGradient(&bp, work, 
							radioBorderColor(contour, TOP), 
							contour, 
							radioBorderColor(contour, BOTTOM), 
							convex);
				QColor color = colorMix(ground, contour, 144);
				bp.setClipRegion(tiny_radio_outer_alias);
				renderCircleGradient(&bp, work, 
							radioBorderColor(color, TOP), 
							color, 
							radioBorderColor(color, BOTTOM), 
							convex);
			}
			bp.setClipping(false);
			bp.end();
			// To put the radiobutton in a better position.
			work = QRect(right-13, top + (height-13)/2 - 2, 13, 13);
			p->drawPixmap(work, buffer);
			// Radio mark
			if (on)
			{
				renderSoftMap(p, work, surface, cg.text(), TINY_RADIOMARK);
			}
			break;
		}
		
		// GENERAL PANELS
		// --------------
		case PE_PanelGroupBox:
		case PE_GroupBoxFrame:
		{
			// Will be handled by event filter...
			// because serene groupboxes are always activated.
			if ( opt.isDefault() || opt.lineWidth() <= 0 )
				break;
			
			renderContour(p, r,  getColor(cg, DefaultGround), 
					getColor(cg, PanelContour, enabled), 
					Button_Alike);
			break;
		}
		case PE_WindowFrame:
		{
			renderSereneContour(p, r, cg.background(), 
					getColor(cg, PanelContour), 
					Square_Box);
		}
		case PE_Panel:
		{
			QColor ground = cg.background();
			int opt1 = opt.lineWidth();
			int opt2 = opt.midLineWidth();
			if (opt1==2 && opt2==0)
			{
				if ( p->device() && dynamic_cast<QWidget*>(p->device()) )
				{
					// This is a floating toolbar or sub-window.
					renderContour(p, r, ground, 
							getColor(cg, PanelContour), 
							Square_Box);
				}
				else
				{
					// This must be a KArrowButton.
					/// MXLS: No mouseover. Solution: Patch KDE!
					renderButton(p, r, cg, sunken, mouseOver,
							ENABLED, ALPHA, ToolButton_Type);
				}
			}
			else if (opt1==2 && opt2==2)
			{
				// Patch for gtk-qt panels.
				p->setPen(ground);
				p->drawRect(r);
			}
			else if ( p->device() && dynamic_cast<QPixmap*>(p->device()) )
			{
				// This is a frame within khtml.
				if ( opt.isDefault() || opt1 <= 0 )
					break;
				renderContour(p, r, ground, 
						getColor(cg, FrameContour), 
						Square_Box);
			}
			else if ( p->device() && dynamic_cast<QLabel*>(p->device()) )
			{
				/// MXLS: HACK: To restore the frames in the font dialog.
				/// Handled by event filter too.
				renderContour(p, r, ground, 
						getColor(cg, FrameContour, enabled), 
						Button_Alike);
			}
			else
			{
				// Otherwise don't draw anything (visible).
				p->setPen(ground);
				p->drawRect(r);
			}
			break;
		}
	
		case PE_PanelLineEdit:
		{
			bool khtml = false;
			if ( p->device() && dynamic_cast<QPixmap*>(p->device()) )
			{
				khtml = true;
			}
			renderSereneEditFrame(p, r, cg, enabled, hasFocus, khtml);
			break;
		}
	
		case PE_StatusBarSection:
		{
			break;
		}
		
		case PE_TabBarBase:
		{
			break;
		}
		case PE_PanelTabWidget:
		{
			renderContour(p, r, cg.background(), 
					getColor(cg, PanelContour, enabled), 
					Square_Box );
			break;
		}
	
		case PE_PanelPopup:
		{
			QColor contour;
			if (! dynamic_cast<QWidget*>(p->device()))
			{
				// For the color configuration dialog.
				p->setPen(colorMix(cg.background(), cg.foreground(), 184));
				p->drawRect(r);
			}
			else
			{
				renderContour(p, r, cg.button(), 
						getColor(cg, PopupContour), 
						Square_Box|Draw_AlphaBlend);
			}
			break;
		}
	
		case PE_PanelMenuBar:
		{
			/// MXLS: MenuGroove
			QColor ground = getColor(cg, DefaultGround);
			if (! dynamic_cast<QWidget*>(p->device()))
			{
				// For the color configuration dialog.
				p->fillRect( r, QBrush(cg.background()) ); /// MXLS: Not "DefaultGround"!
			}
			else if (_menuGrooveStyle <= MuInvGradient)
			{
				bool convex = (_menuGrooveStyle == MuGradient);
				QColor topSurface = getColor(cg, Menubar_GradTop);
				QColor bottomSurface = getColor(cg, Menubar_GradBottom);
				renderMenuGradient(p, QRect(left, top, 4, bottom-top), 
							topSurface, ground, bottomSurface, 
							convex);
				renderMenuGradient(p, QRect(right-3, top, 4, bottom-top), 
							topSurface, ground, bottomSurface, 
							convex);
				p->setPen(convex ? bottomSurface : ground);
				p->drawLine(left, top, right, top);
				p->setPen(convex ? topSurface : ground);
				p->drawLine(left, bottom, right, bottom);
			}
			else if (_menuGrooveStyle <= MuInvSolid)
			{
				QRect work(r);
				work.addCoords(0, 0, 0, -1);
				p->setPen(ground);
				p->drawLine(left, bottom, right, bottom);
				renderContour(p, work, ground, 
						getColor(cg, Menubar_Solid), 
						Button_Alike);
			}
			else if (_menuGrooveStyle == MuTwoLines)
			{
				p->setPen(ground);
				p->drawRect(left, top+1, width, height-1);
				p->setPen( getColor(cg, Menubar_2lines) );
				p->drawLine(left, top, right, top);	/// MXLS: Only top line!
				p->drawLine(left, bottom-1, right, bottom-1);
			}
			else				// Empty
			{
				p->setPen(ground);
				p->drawRect(r);
			}
			break;
		}
		
		case PE_PanelDockWindow:
		{
			// Fix for toolbar lag (from Mosfet Liquid) 
			QWidget* w = dynamic_cast<QWidget*>(p->device());
			if (w && w->backgroundMode() == PaletteButton)
				w->setBackgroundMode(PaletteBackground);
	
			p->fillRect(r, QBrush(cg.background()) );
			break;
		}
	
		// SPLITTER/HANDLE
		// ---------------
		case PE_Splitter:
		case PE_DockWindowResizeHandle:
		{
			QColor surface = colorMix(cg.button(), cg.buttonText(), SRN_SPLIT);
			if ( (mouseOver) && (_resizerStyle != 2) )
			{	// Do not highlight "invisible" resizers.
				surface = colorMix(surface, getColor(cg, HoverColor), SRN_DOWN);
			}
			QColor alias = colorMix(cg.background(), surface);
			if ( (_resizerStyle == 1) && (!mouseOver) )
			{	// So hovered "discreet" resizers look the same than "obvious" ones.
				surface = alias;
			}
			QRect grip(r);
			int x, y, three_eight;
			if (width > height)
			{
				three_eight = (width*3)/8;
				// 1/4 in the center for the grip.
				grip.addCoords(three_eight, 0, -three_eight, 0);
				y = top + (height-2)/2;
				if ( (_resizerStyle == 2) && (!mouseOver) )
				{
					for (x = grip.left() ; x <= grip.right() ; x+=6)
					{
						p->setPen(cg.background());
						p->drawRect(x-1, y, 4, 2);
					}
				}
				else
				{
					for (x = grip.left() ; x <= grip.right() ; x+=6)
					{
						p->setPen(alias);
						p->drawRect(x-1, y, 4, 2);
						p->setPen(surface);
						p->drawRect(x, y, 2, 2);
					}
				}
			} 
			else
			{
				three_eight = (height*3)/8;
				grip.addCoords(0, three_eight, 0, -three_eight);
				x = left + (width-2)/2;
				if ( (_resizerStyle == 2) && (!mouseOver) )
				{
					for (y = grip.top() ; y <= grip.bottom() ; y+=6)
					{
						p->setPen(cg.background());
						p->drawRect(x, y-1, 2, 4);
					}
				}
				else
				{
					for (y = grip.top() ; y <= grip.bottom() ; y+=6)
					{
						p->setPen(alias);
						p->drawRect(x, y-1, 2, 4);
						p->setPen(surface);
						p->drawRect(x, y, 2, 2);
					}
				}
			}
			break;
		}
		case PE_DockWindowHandle:
		{
			if (_drawToolBarHandle || mouseOver)
			{
				QColor surface = getColor(cg, LightSurface);
				if (qGray(surface.rgb()) < 64)
					surface = brighter(surface, ADJUSTOR);
				//
				QRect grip(r);
				if (horiz)
				{
					grip.addCoords(0, 3, 0, -3);
					renderGrip(p, grip, cg, surface, VERTICAL, 
							!SUNKEN, mouseOver, ENABLED, THICK);
				} 
				else
				{
					grip.addCoords(3, 0, -3, 0);
					renderGrip(p, grip, cg, surface, HORIZONTAL, 
							!SUNKEN, mouseOver, ENABLED, THICK);
				}
			}
			break;
		}
		case PE_DockWindowSeparator:
		{
			if (_drawToolBarSeparator)
			{
				QColor surface = getColor(cg, LightSurface);
				if (qGray(surface.rgb()) < 64)
					surface = brighter(surface, ADJUSTOR);
				//
				QRect grip(r);
				if (horiz)
				{
					grip.addCoords(0, 4, 0, -4);
					renderGrip(p, grip, cg, surface, VERTICAL, 
							!SUNKEN, mouseOver, ENABLED, THIN);
				} 
				else
				{
					grip.addCoords(4, 0, -4, 0);
					renderGrip(p, grip, cg, surface, HORIZONTAL, 
							!SUNKEN, mouseOver, ENABLED, THIN);
				}
			}
			break;
		}
		
		// ARROWS/SPIN WIDGET
		// ------------------
		case PE_HeaderArrow:
		{
			bool headerStyle = false;
			if ( p->device() && dynamic_cast<QWidget*>(p->device()) )
			{
				QWidget* w = dynamic_cast<QWidget*>(p->device());
				if (w->parentWidget() && w->parentWidget()->inherits("QScrollView"))
				{
					headerStyle = true;
				}
			}
			QColor arrowColor;
			if (headerStyle)
			{
				// Real column header
				arrowColor = cg.foreground();
			}
			else
			{
				// (Shouldn't happen.)
				arrowColor = cg.buttonText();
			}
			renderHardMap(p, r, arrowColor,
					(flags & Style_Down) ? ARROWDOWN : ARROWUP);
			break;
		}
		case PE_SpinWidgetUp:
		case PE_SpinWidgetDown:
		{
			BitmapItem arrowType;
			switch (pe)
			{
				case PE_SpinWidgetUp:
				{
					arrowType = ARROWUP;
					break;
				}
				case PE_SpinWidgetDown:
				default:
				{
					arrowType = ARROWDOWN;
					break;
				}
			}
			renderHardMap(p, r, cg.text(), arrowType);
			break;
		}
		case PE_ArrowUp:
		case PE_ArrowDown:
		case PE_ArrowLeft:
		case PE_ArrowRight:
		{
			BitmapItem arrowType;
			switch (pe)
			{
				case PE_ArrowUp:
				{
					arrowType = ARROWUP;
					break;
				}
				case PE_ArrowLeft:
				{
					arrowType = ARROWLEFT;
					break;
				}
				case PE_ArrowRight: 
				{
					arrowType = ARROWRIGHT;
					break;
				}
				case PE_ArrowDown:
				default:
				{
					arrowType = ARROWDOWN;
					break;
				}
			}
			renderHardMap(p, r, cg.foreground(), arrowType);
			break;
		}
		case PE_SpinWidgetPlus:
		case PE_SpinWidgetMinus:
		{
			p->setPen(cg.text());
			int size = QMIN(width-2, height-2)/2;
			QPoint center = r.center();
			int cx = center.x();
			int cy = center.y();
			p->drawLine(cx-size, cy, cx+size, cy);
			if (pe == PE_SpinWidgetPlus)
			{
				p->drawLine(cx, cy-size, cx, cy+size);
			}
			break;
		}
		
		case PE_SizeGrip:
		{
			// Allowed area (according to Qt sources)
          		int size = QMIN(width, height)-1;
			int x = _reverseLayout ? left : right-size;
			int y = bottom-size;
			QRect work(x, y, size, size);
			
			QColor baseColor = getColor(cg, LightSurface);
			if (qGray(baseColor.rgb()) < 64)
				baseColor = brighter(baseColor, ADJUSTOR);
			//
			QColor borderTop = darker(baseColor, HI_FACTOR);
			QColor borderBottom = brighter(baseColor, MED_FACTOR);
			QColor aliasedTop = colorMix(baseColor, borderTop);
			QColor aliasedBottom = colorMix(baseColor, borderBottom);
			
			int left, top, right, bottom;
			work.coords(&left, &top, &right, &bottom);
			
			if (_reverseLayout)
			{
				p->setPen(baseColor);
				p->drawLine(left+1, top+1, left+1, bottom-1);
				p->drawLine(left+1, bottom-1, right-1, bottom-1);
				p->setPen(aliasedTop);
				p->drawPoint(left, top);
				p->setPen(borderTop);
				p->drawPoint(left+1, top);
				p->setPen(borderBottom);
				p->drawPoint(right, bottom-1);
				p->setPen(aliasedBottom);
				p->drawPoint(right, bottom);
				
				renderGradLine(p, QRect(left, top+1, 1, height-3), 
						borderTop, baseColor, 
						VerticalGradient);
				renderGradLine(p, QRect(left+2, top+1, 1, height-5),
						baseColor, borderBottom, 
						VerticalGradient);
				
				renderGradLine(p, QRect(left+3, bottom-2, width-5, 1),
						borderTop, baseColor, 
						HorizontalGradient);
				renderGradLine(p, QRect(left+1, bottom, width-3, 1),
						baseColor, borderBottom, 
						HorizontalGradient);
			}
			else
			{
				p->setPen(baseColor);
				p->drawLine(right-1, top+1, right-1, bottom-1);
				p->drawLine(left+1, bottom-1, right-1, bottom-1);
				p->setPen(borderTop);
				p->drawPoint(right-1, top);
				p->drawPoint(left, bottom-1);
				p->setPen(aliasedTop);
				p->drawPoint(right-2, top);
				p->drawPoint(left, bottom-2);
				p->setPen(aliasedBottom);
				p->drawPoint(right, bottom);
				
				renderGradLine(p, QRect(right-2, top+1, 1, height-3), 
						borderTop, baseColor, 
						VerticalGradient);
				renderGradLine(p, QRect(right, top+1, 1, height-3),
						baseColor, borderBottom, 
						VerticalGradient);
				
				renderGradLine(p, QRect(left+1, bottom-2, width-3, 1),
						borderTop, baseColor, 
						HorizontalGradient);
				renderGradLine(p, QRect(left+1, bottom, width-3, 1),
						baseColor, borderBottom, 
						HorizontalGradient);
			}
			break;
		}

		default:
		{
			return KStyle::drawPrimitive(pe, p, r, cg, flags, opt);
		}
	}
}

void SerenityStyle::drawControl(ControlElement element, 
				QPainter* p, 
				const QWidget* widget, 
				const QRect &r, 
				const QColorGroup &cg, 
				SFlags flags, 
				const QStyleOption& opt) const
{
	const bool enabled = (flags & Style_Enabled);
	
	int left, top, width, height;
	r.rect(&left, &top, &width, &height);
	int right, bottom;
	r.coords(&left, &top, &right, &bottom);
	
	switch (element)
	{
		case CE_ProgressBarGroove:
		{
			QColor surface = getColor(cg, ProgressGroove);
			QRect work(r);
			work.addCoords(1, 1, -1, -1);
			p->fillRect( work, QBrush(surface) );
			renderContour(p, r, getColor(cg, DefaultGround), 
					getColor(cg, ProgressSurface), 
					Button_Alike);
			break;
		}
		case CE_ProgressBarContents:
		{
			const QProgressBar* pb = dynamic_cast<const QProgressBar*>(widget);
			int steps = pb->totalSteps();
			QColor ground = getColor(cg, ProgressGroove);
			QColor surface2 = getColor(cg, ProgressSurface);	// Darkest
			QColor surface3 = colorMix(ground, surface2, 160);	// Brightest
			QColor surface = colorMix(surface3, surface2);		// Medium
			
			// Busy indicator
			if (steps == 0)
			{
				static const int barWidth = height;
				int ww = (width-barWidth)*2;
				int progress = pb->progress() % ww;
				if (progress < 0)
					progress = 0;
				if (progress > width-barWidth)
					progress = ww - progress;
				
				QRect bar(left+progress, top, barWidth, height);
				if (_flatProgressBar)
				{
					renderFlatArea(p, bar, ground, surface, Square_Box);
				}
				else
				{
					renderSurface(p, bar, ground, surface, surface, 
								Square_Box);
				}
			}
			else
			{
				double percent = static_cast<double>(pb->progress()) / static_cast<double>(steps);
				int w = static_cast<int>(width * percent);
				
				// Set a minimal progressbar width...
				if (w < 8) w = 8;

				int shift = 0;
				if (_animateProgressBar)
				{
					// Find the animation Offset for the current Widget
					QWidget* nonConstWidget = const_cast<QWidget*>(widget);
					QMapConstIterator<QWidget*, int> iter = progAnimWidgets.find(nonConstWidget);
					if (iter != progAnimWidgets.end())
						shift = iter.data();
				}
				QRect bar;
				int dx;
				if (_reverseLayout)
				{
					shift = 24-shift; // Reverse animation
					bar = QRect(right-w+1, top, w, height);
					dx = (_progressPattern == 3) ? -16 : 16;
				}
				else
				{
					bar = QRect(left, top, w, height);
					dx = (_progressPattern == 3) ? 16 : -16;
				}
				int left, top, width, height;
				bar.rect(&left, &top, &width, &height);
				QRegion mask = maskPattern(width+72, height, shift);
				mask.translate(left-48, top);
				// The progress bar
				if (_flatProgressBar)
				{
					renderFlatArea(p, bar, ground, surface, Square_Box);
				}
				else
				{
					renderSurface(p, bar, ground, 
								surface, surface, 
								Square_Box);
				}
				// Plus some stripes
				p->setClipRegion(mask, QPainter::CoordPainter);
				if (_flatProgressBar)
				{
					renderFlatArea(p, bar, ground, surface2, Square_Box);
					p->setClipping(false);
					mask.translate(dx, 0);
					p->setClipRegion(mask, QPainter::CoordPainter);
					renderFlatArea(p, bar, ground, surface3, Square_Box);
				}
				else
				{
					renderSurface(p, bar, ground, 
								surface2, surface2, 
								Square_Box);
					p->setClipping(false);
					mask.translate(dx, 0);
					p->setClipRegion(mask, QPainter::CoordPainter);
					renderSurface(p, bar, ground, 
								surface3, surface3, 
								Square_Box);
				}
				p->setClipping(false);
			}
			break;
		}
		case CE_ProgressBarLabel:
		{
			const QProgressBar* pb = (const QProgressBar*)widget;
			QRect cr = subRect(SR_ProgressBarContents, widget);
			if (!cr.isValid())
				return;
			/// MXLS: Progress bar is unicolore. Simplify!
			QFont font = p->font();
			font.setBold(true);
			p->setFont(font);
			p->setPen(cg.foreground());
			p->drawText(cr, AlignCenter, pb->progressString());
			break;
		}
		
		case CE_TabBarTab:
		{
			const QTabBar* tb = (const QTabBar*)widget;
			
			/// MXLS: The presence of a corner widget has
			/// no influence over the drawing in Serenity.
			/*
			bool cornerWidget = false;
			if (::qt_cast<QTabWidget*>(tb->parent()))
			{
				const QTabWidget* tw = (const QTabWidget*)tb->parent();
				// Is there a corner widget in the (top) left edge?
				QWidget* cw = tw->cornerWidget(Qt::TopLeft);
				if (cw)
					cornerWidget = true;
			}
			*/
			TabPosition pos;
			if (tb->count() == 1)
			{
				pos = Single;
			} 
			else if ((tb->indexOf(opt.tab()->identifier()) == 0))
			{
				pos = First;
			} 
			else if (tb->indexOf(opt.tab()->identifier()) == tb->count() - 1)
			{
				pos = Last;
			} 
			else
			{
				pos = Middle;
			}
			QTabBar::Shape tbs = tb->shape();
			switch (tbs)
			{
				case QTabBar::RoundedAbove:
					renderSereneTab(p, r, cg, flags, false, pos, false/*, cornerWidget*/);
					break;
				case QTabBar::RoundedBelow:
					renderSereneTab(p, r, cg, flags, true, pos, false/*, cornerWidget*/);
					break;
				case QTabBar::TriangularAbove:
					renderSereneTab(p, r, cg, flags, false, pos, true/*, cornerWidget*/);
					break;
				case QTabBar::TriangularBelow:
					renderSereneTab(p, r, cg, flags, true, pos, true/*, cornerWidget*/);
					break;
				default:
					// For the vertical tabs of Qt 4.  ;-)
					KStyle::drawControl(element, p, widget, r, cg, flags, opt);
			}
			break;
		}
		case CE_TabBarLabel:
		{
			if (opt.isDefault())
				return ;
			
			QTab* tab = opt.tab();
			bool enabled = flags & Style_Enabled;
			
			// Any icon will be drawn by KDE
			// without respecting a reverse layout. (Solution: Fix KDE!)
			//
			/// MXLS: HACK: This indirect way to get the color prevents 
			/// apps (like Konqueror) from tuning the color of the label 
			/// so we can do whatever we want instead.
			QColor textColor = getColor(cg, DefaultPen);
			if (textColor != cg.foreground())
			{
				if (_tabWarnType == 1)		// Bold text
				{
					QFont font = p->font();
					font.setBold(true);
					p->setFont(font);
				}
				else if (_tabWarnType == 2)	// Italic text
				{
					QFont font = p->font();
					font.setItalic(true);
					p->setFont(font);
				}
				else if (_tabWarnType == 0)	// App. default
				{
					textColor = cg.foreground();
				}
			}
			p->setPen(enabled ? textColor
					: colorMix(getColor(cg, DefaultGround), textColor));
			p->drawText(QRect(left, top, width+4, height), 
					AlignCenter|ShowPrefix, tab->text());
	
			if ( (_drawFocusRect) && (flags & Style_HasFocus) )
				drawPrimitive(PE_FocusRect, p, QRect(left, top, width+4, height), cg);
			break;
		}
		
		case CE_PushButton:
		{
			QPushButton* button = (QPushButton*)widget;
			const bool isDefault = enabled && button->isDefault();
			//
			if (widget == hoverWidget)
				flags |= Style_MouseOver;
			//
			if ( widget->isA("KMultiTabBarTab") 
			|| widget->isA("KMultiTabBarButton") ) 
			{
				renderButton(p, r, cg, (button->isOn() || button->isDown()), 
						(button->hasMouse()), enabled, !ALPHA, 
						MultiTab_Type);
			}
			else if (button->isFlat())
			{
				renderButton(p, r, cg, (button->isOn() || button->isDown()), 
						(button->hasMouse()), enabled, !ALPHA, 
						FlatButton_Type);
			}
			else if (isDefault)	// Default button
			{
				drawPrimitive(PE_ButtonDefault, p, r, cg, flags, QStyleOption(button));
			}
			else		// Regular button
			{
				drawPrimitive(PE_ButtonBevel, p, r, cg, flags, QStyleOption(button));
			}
			break;
		}
		case CE_PushButtonLabel:
		{
			const QPushButton* button = dynamic_cast<const QPushButton*>(widget);
						
			if (button->isOn() || button->isDown())
				flags |= Style_Sunken;
			
			// Draw the icon if there is one
			if (button->iconSet() && ! button->iconSet()->isNull())
			{
				QIconSet::Mode  mode  = QIconSet::Disabled;
				QIconSet::State state = QIconSet::Off;
				
				if (button->isEnabled())
					mode = button->hasFocus() ? QIconSet::Active : QIconSet::Normal;
				if (button->isToggleButton() && button->isOn())
					state = QIconSet::On;
				
				QPixmap pixmap = button->iconSet()->pixmap( QIconSet::Small, mode, state );
				int pixW = pixmap.width();
				int pixH = pixmap.height();
				if (button->text().isEmpty() && !button->pixmap())
					p->drawPixmap(left + (width-pixW)/2, top + (height-pixH)/2,
						pixmap);
				else
				{
					if (_reverseLayout)
					{
						p->drawPixmap(right-pixW-4, top + (height-pixH)/2, pixmap);
					}
					else
					{
						p->drawPixmap(left+4, top + (height-pixH)/2, pixmap);
						left += (pixW+4);
					}
					width -= (pixW+4);
				}
			}
			// The label
			QColor textColor;
			if ( widget->isA("KMultiTabBarTab") ) 
				textColor = cg.foreground();
			else if (!kateMode)
				textColor = cg.buttonText();
			else
			{
				/// MXLS: HACK: This indirect way to get the color 
				/// prevents Kate from using another color (red).
				textColor = getColor(cg, ButtonPen, enabled);
			}
			drawItem(p, QRect(left, top, width, height), AlignCenter|ShowPrefix, 
				button->colorGroup(), button->isEnabled(), button->pixmap(), 
				button->text(), -1, &textColor);
			// The popup menu indicator (eventually) over everything
			if (button->isMenuButton())
			{
				QRect vr((_reverseLayout ? left : right-9), bottom-7, 9, 7);
				if (widget->isA("KMultiTabBarButton"))
				{
					/// MXLS: The arrow is misplaced.
					if (_reverseLayout)
						vr.addCoords(-3, 3, -3, 3);
					else
						vr.addCoords(4, 3, 4, 3);
				}
				renderHardMap(p, vr, textColor, ARROWDOWN);
			}
			// Focus rectangle (eventually)
			if ( (_drawFocusRect) && (flags & Style_HasFocus) )
				drawPrimitive(PE_FocusRect, p,
					visualRect(subRect(SR_PushButtonFocusRect, widget), widget),
					cg);
			break;
		}
		
		case CE_ToolButtonLabel:
		{
			/// MXLS: This is only for QToolButtons.
			/// KDE has --alas!-- its own routines to draw KToolButtons.
			const QToolButton* button = dynamic_cast<const QToolButton*>(widget);
			QColor textColor = cg.foreground();
			p->setPen(textColor); // (Eventually...)
			
			/// MXLS: Let's simplify that tiny button with a long name.
			if ( button->parentWidget() 
			&& button->parentWidget()->inherits("QToolBarExtensionWidget") )
			{
				renderHardMap(p, QRect(left, bottom-10, width, 9), 
						textColor, TOOLPLUS);
				break;
			}
			// Is it a tab arrow button (or an arrow only QToolButton)?
			if ( (!opt.isDefault()) && (button->iconSet().isNull())
			&& (! button->usesTextLabel()) )
			{
				Qt::ArrowType arrow = opt.arrowType();
				if (arrow == Qt::LeftArrow)
				{
					renderHardMap(p, r, textColor, ARROWLEFT);
					break;
				}
				else if (arrow == Qt::RightArrow)
				{
					renderHardMap(p, r, textColor, ARROWRIGHT);
					break;
				}
				else if (arrow == Qt::DownArrow)
				{
					renderHardMap(p, r, textColor, ARROWDOWN);
					break;
				}
				else if (arrow == Qt::UpArrow)
				{
					renderHardMap(p, r, textColor, ARROWUP);
					break;
				}
			}
			
			// Now, the real QToolButtons.
			QRect txtR(r);
			// Draw the icon if there is one.
			if (! button->iconSet().isNull())
			{
				QIconSet::Mode  mode  = QIconSet::Disabled;
				QIconSet::State state = QIconSet::Off;
		
				if (button->isEnabled())
					mode = button->hasFocus() ? QIconSet::Active : QIconSet::Normal;
				if (button->isToggleButton() && button->isOn())
					state = QIconSet::On;
				
				QPixmap pixmap = button->iconSet().pixmap( QIconSet::Small, mode, state );
				int pixW = pixmap.width();
				int pixH = pixmap.height();
				QRect pixR(left, top, pixW, pixH);
				if ( (button->textLabel().isEmpty() && button->text().isEmpty()) 
				|| (!button->usesTextLabel()) ) 
				{
					// No text, just center the pixmap.
					pixR.moveBy((width-pixW)/2, (height-pixH)/2);
				}
				else
				{
					if (button->textPosition() == QToolButton::Under)
					{
						int h = p->fontMetrics().height();
						pixR.moveBy((width-pixW)/2, (height-pixH-2-h)/2);
						top = pixR.top() + pixH + 4;
						height = h;
					}
					else
					{
						if (_reverseLayout)
						{
							pixR.moveTopLeft( QPoint(right-pixW-4, 
									top + (height-pixH)/2) );
						}
						else
						{
							pixR.moveBy(4, (height-pixH)/2);
							left += (pixW+4);
						}
						width -= (pixW+4);
					}
					txtR = QRect(left, top, width, height);
				}
				p->drawPixmap(pixR, pixmap);
			}
			// The label
			if ( (button->usesTextLabel()) && (! button->textLabel().isEmpty()) )
			{
				p->drawText(txtR, AlignCenter|ShowPrefix, button->textLabel());
			}
			else if (! button->text().isEmpty())
			{
				p->drawText(txtR, AlignCenter|ShowPrefix, button->text());
			}
			// Popup menu indicator (eventually) over everything.
			if (button->popup())
			{
				renderHardMap(p, QRect((_reverseLayout ? r.left() : r.right()-7), 
							r.bottom()-7, 7, 7), 
						textColor, ARROWDOWN);
			}
			// The focus rectangle (eventually) is drawn bt Qt
			break;
		}
		
		case CE_CheckBox:
		{
			/*
			/// MXLS: UNDO: Disable mouseover
			if (hoverThis == HOVER_CHECKRADIO)
				flags |= Style_MouseOver;
			else
			*/
			flags &= ~Style_MouseOver;
			//
			drawPrimitive(PE_Indicator, p, r, cg, flags, opt);
			break;
		}
		case CE_RadioButton:
		{
			/*
			/// MXLS: UNDO: Disable mouseover
			if (hoverThis == HOVER_CHECKRADIO)
				flags |= Style_MouseOver;
			else
			*/
			flags &= ~Style_MouseOver;
			//
			drawPrimitive(PE_ExclusiveIndicator, p, r, cg, flags, opt);
			break;
		}
		case CE_CheckBoxLabel:
		case CE_RadioButtonLabel:
		{
			const QButton* button = dynamic_cast<const QButton*>(widget);
			int textFlags = (_reverseLayout ? AlignRight : AlignLeft) | AlignVCenter | ShowPrefix;
			/// MXLS: UNDO: Always hover labels.
			if (enabled && (hoverWidget==button) && (hoverThis==HOVER_CHECKRADIO))
			//&& (_mouseOverLabel) ) 
			{
				renderFlatArea(p, subRect(SR_CheckBoxFocusRect, widget), 
					cg.background(), 
					colorMix( cg.background(), getColor(cg, HoverColor), SRN_OVER), 
					Button_Alike);
			}
			QColor textColor = cg.foreground();
			drawItem(p, r, textFlags, cg, enabled, button->pixmap(), 
				button->text(), -1, &textColor);
			
			// Only draw focus if it has content. Forms on html don't.
			if (_drawFocusRect && (flags & Style_HasFocus))
			{
				if (button->text() || button->pixmap())
				{
					drawPrimitive(PE_FocusRect, p,
							subRect(SR_CheckBoxFocusRect, button), 
							cg);
				}
			}
			break;
		}
		
		case CE_MenuBarItem:
		{
			/// MXLS: MenuGroove
			QColor surface;
			if (_menuGrooveStyle <= MuInvGradient)
			{
				QRect work(r);
				surface = getColor(cg, DefaultGround);
				uint surfaceFlags = No_Flags;
				if ( (flags & Style_Active) && (flags & Style_HasFocus) )
				{
					if (flags & Style_Down)
						surfaceFlags |= Is_Sunken;
					else
						surfaceFlags |= Is_Hovered;
					//
					renderBicoloreGradient(p, work, 
								cg, surface, 
								gradientColor(surface, TOP), 
								surface, 
								gradientColor(surface, BOTTOM), 
								surfaceFlags, 
								(_menuGrooveStyle == MuGradient));
				}
				else
				{
					work.addCoords(0, -2, 0, 2);
					renderMenuGradient(p, work, 
								getColor(cg, Menubar_GradTop), 
								surface, 
								getColor(cg, Menubar_GradBottom), 
								(_menuGrooveStyle == MuGradient));
				}
			}
			else
			{ 
				if (_menuGrooveStyle <= MuInvSolid)
				{
					surface = getColor(cg, Menubar_Solid);
					p->fillRect( r, QBrush(surface) );
				}
				else if (_menuGrooveStyle >= MuTwoLines)
				{
					surface = getColor(cg, DefaultGround);
					p->fillRect( r, QBrush(surface) );
				}
				if ( (flags & Style_Active) && (flags & Style_HasFocus) )
				{
					uint surfaceFlags = Square_Box|Draw_AlphaBlend;
					if (flags & Style_Down)
					{
						renderSurface(p, r, surface, surface, 
								cg.highlight(), 
								surfaceFlags|Round_Top|Is_Sunken);
					}
					else
					{
						renderSurface(p, r, surface, surface, 
								getColor(cg, HoverColor), 
								surfaceFlags|Round_All|Is_Hovered);
					}
				}
			}
			p->setPen(cg.foreground());
			p->drawText(r, AlignVCenter|AlignHCenter|ShowPrefix|DontClip|SingleLine, 
					(opt.menuItem())->text());
			break;
		}
		case CE_MenuBarEmptyArea:
		{
			/// MXLS: MenuGroove
			if (_menuGrooveStyle <= MuInvGradient)
			{
				renderMenuGradient(p, r, 
							getColor(cg, Menubar_GradTop), 
							getColor(cg, DefaultGround), 
							getColor(cg, Menubar_GradBottom), 
							(_menuGrooveStyle == MuGradient));
			}
			else if (_menuGrooveStyle <= MuInvSolid) 
			{
				p->fillRect( r, QBrush(getColor(cg, Menubar_Solid)) );
			}
			else if (_menuGrooveStyle >= MuTwoLines)
			{
				p->fillRect( r, QBrush(getColor(cg, DefaultGround)) );
			}
			if (_menuGrooveStyle == MuTwoLines)	// Bottom line
			{
				p->setPen( getColor(cg, Menubar_2lines) );
				p->drawLine(left, bottom, right, bottom);
			}
			break;
		}
		case CE_PopupMenuItem:
		{
			const QPopupMenu* popupmenu = static_cast<const QPopupMenu*>( widget );
			QMenuItem* mi = opt.menuItem();
			QColor ground;
			QColor thePen;
			bool mouseOver = flags & Style_Active;
			if (! dynamic_cast<QWidget*>(p->device()))
			{
				// For the color configuration dialog.
				ground = cg.background();
				thePen = cg.foreground();
			}
			else
			{
				ground = getColor(cg, ButtonSurface);
				/// MXLS: IMPORTANT: Set the color for everything in the entry
				thePen = mouseOver 
					? cg.highlightedText() 
					: cg.buttonText();
			}
			QColor surface = ground;
			//
			if (!mi)
			{
				// Don't leave blank holes if we set NoBackground for the QPopupMenu.
				// This only happens when the popupMenu spans more than one column.
				if (! (widget->erasePixmap() && !widget->erasePixmap()->isNull()) )
					p->fillRect(r, ground);
				break;
			}
			int tab = opt.tabWidth();
			int checkcol = opt.maxIconWidth();
			bool enabled = flags & Style_Enabled;
			bool checkable = popupmenu->isCheckable();
			//bool down = flags & Style_Down;	// Qt says yes. KDE says no.
			if (checkable)
				checkcol = QMAX( checkcol, 20 );
			
			// Draw the menu item background
			if (widget->erasePixmap() && !widget->erasePixmap()->isNull())
			{
				p->drawPixmap( r.topLeft(), *widget->erasePixmap(), r );
			}
			else
			{
				p->fillRect(r, ground);	
			}
			if (mouseOver)
			{
				if (enabled)	surface = cg.highlight();
				else		surface = colorMix(ground, cg.highlight());
				
				renderSurface(p, QRect(left+1, top+1, width-2, height-2), 
							ground, 
							surface, surface, 
							Button_Alike);
			}
			// Are we a menu separator?
			if (mi->isSeparator())
			{
				p->setPen( getColor(cg, PopupContour) );
				p->drawLine(left, top+2, right, top+2);
			}
			
			QRect cr = visualRect(QRect(left+2, top+2, height-4, height-4), r);
			
			// Do we have an icon?
			if (mi->iconSet())
			{
				QIconSet::Mode mode;
		
				// Select the correct icon from the iconset
				if (mouseOver)
					mode = enabled?QIconSet::Active:QIconSet::Disabled;
				else
					mode = enabled?QIconSet::Normal:QIconSet::Disabled;
			
				// Do we have an icon and are checked at the same time?
				if (checkable && mi->isChecked())
				{
					renderSurface(p, cr, surface, 
								ground, cg.highlight(), 
								Button_Alike|Is_Sunken);
				}
				// Draw the icon
				QPixmap pixmap = mi->iconSet()->pixmap(QIconSet::Small, mode);
				QRect pmr(0, 0, pixmap.width(), pixmap.height());
				pmr.moveCenter(cr.center());
				p->drawPixmap(pmr.topLeft(), pixmap);
			}
			// Are we checked? (This time without an icon.)
			else if (checkable && mi->isChecked())
			{
				renderSurface(p, cr, surface, 
							ground, cg.highlight(), 
							Button_Alike|Is_Sunken);
				renderSoftMap(p, cr, colorMix(ground, cg.highlight(), SRN_DOWN),
						cg.buttonText(), CHECKMARK);
			}
			
			// High time to draw the menu item label...
			int xm = 2 + checkcol + 2;
			int xp = _reverseLayout // X position
				? left + tab + rightBorder + itemHMargin + itemFrame - 1 
				: left + xm;
			// Label width (minus the width of the accelerator portion)
			int tw = width - xm - tab - arrowHMargin - itemHMargin * 3 - itemFrame + 1;
	
			p->setPen(thePen);
			
			// Does the menu item draw it's own label?
			if (mi->custom())
			{
				p->save();
				// Transparent pixmap
				if (widget->erasePixmap() && !widget->erasePixmap()->isNull())
				{
					p->drawPixmap(r.topLeft(), *widget->erasePixmap(), r);
				}
				mi->custom()->paint(p, cg, mouseOver, enabled, xp, top, tw, height);
				p->restore();
			}
			else
			{
				// The menu item doesn't draw it's own label
				QString s = mi->text();
				if (! s.isNull())
				{
					int t = s.find( '\t' );
					int m = 2;
					int textFlags = (_reverseLayout ? AlignRight : AlignLeft)
						| AlignVCenter | ShowPrefix | DontClip | SingleLine;
					// Does the menu item have a tabstop? (for the accelerator text)
					if ( t >= 0 )
					{
						int tabx = _reverseLayout 
							? left + rightBorder + itemHMargin + itemFrame 
							: left + width - tab - rightBorder - itemHMargin - itemFrame;
						// Draw the right part of the label (accelerator text)
						p->drawText( tabx, top+m, tab, height - 2*m, textFlags, s.mid(t+1) );
						s = s.left(t);
					}
					// Draw the left part of the label 
					// (or the whole label if there's no accelerator)
					p->drawText( xp, top+m, tw, height - 2*m, textFlags, s, t );
				}
				// The menu item doesn't have a text label
				// Check if it has a pixmap instead
				else if (mi->pixmap())
				{
					QPixmap* pixmap = mi->pixmap();
					// Draw the pixmap
					if (pixmap->depth() == 1)
						p->setBackgroundMode(OpaqueMode);
					
					int diffw = ( (width - pixmap->width())/2 ) + ( (width - pixmap->width())%2 );
					p->drawPixmap( left+diffw, top+1, *pixmap );
					
					if (pixmap->depth() == 1)
						p->setBackgroundMode(TransparentMode);
				}
			}
			// Does the menu item have a submenu?
			if (mi->popup())
			{
				int dim = pixelMetric(PM_MenuButtonIndicator) + 8;
				QRect vr = visualRect( QRect( right-3-dim, top + (height-dim)/2, 
							dim|1, dim), r );
				//Draw an arrow at the far end of the menu item.
				if (_subArrowFrame)	// Eventually with a framing
				{
					uint surfaceFlags = Button_Alike|Draw_AlphaBlend;
					if (mouseOver)
					{	// Press the arrow.
						surfaceFlags |= Is_Sunken;
					}
					else
					{	// "Un-highlight" the arrow.
						ground = colorMix(ground, cg.buttonText(), SRN_MIX);
					}
					renderSurface(p, vr, surface, 
								ground, surface,
								surfaceFlags);
					renderHardMap(p, vr, thePen, 
							_reverseLayout ? ARROWLEFT : ARROWRIGHT);
				}
				else		// Bigger arrow without framing.
				{
					renderSoftMap(p, vr, surface, colorMix(surface, thePen, 96), 
							_reverseLayout ? ARROWLEFT : ARROWRIGHT);
				}
			}
			break;
		}
		
		case CE_HeaderLabel:
		{
			const QHeader* header = dynamic_cast<const QHeader*>(widget);
			int section = opt.headerSection();
			QString label = header->label(section);
			
			int textFlags = (_reverseLayout ? AlignRight : AlignLeft) 
					| AlignVCenter | ShowPrefix;
			
			bool titleStyle = false;
			bool taskMode = kickerMode;
			if ( (header->parentWidget() && header->parentWidget()->inherits("QPopupMenu")) 
			|| ((kickerMode) && (header->backgroundMode() != PaletteBackground)) )
			{
				titleStyle = true;
			}
			if ( (taskMode) && (header->backgroundMode() == PaletteBase) )
			{
				taskMode = false;
			}
			/// MXLS: HACK: This indirect way to get the color prevents 
			/// the apps (especially KDesktop) from using strange colors.
			QColor textColor;
			if (titleStyle)
			{
				// Popup titles
				/// MXLS: Kicker draws the KMenu titles by itself
				/// and so does KPopupTitle class but --at least--
				/// the wrong color from KDesktop popups is fixed.
				/// Just keep the code until KDE is fixed too.
				textColor = getColor(cg, ButtonPen);
				textFlags = AlignHCenter | AlignVCenter | SingleLine;
			}
			else if (taskMode)
			{
				// Taskbar buttons
				textColor = getColor(cg, ButtonPen, enabled);
			}
			else
			{
				// Real headers
				textColor = cg.text();
			}
			// Draw the icon if there is one.
			QIconSet* icon = header->iconSet(section);
			if (icon)
			{
				QPixmap pixmap = icon->pixmap( QIconSet::Small,
								flags & Style_Enabled ?
								QIconSet::Normal : QIconSet::Disabled );
				int pixW = pixmap.width();
				int pixH = pixmap.height();
				if (label.isEmpty())
				{
					p->drawPixmap(left + (width-pixW)/2, top + (height-pixH)/2,
						pixmap);
				}
				else
				{
					if (_reverseLayout)
					{
						p->drawPixmap(right-pixW, top + (height-pixH)/2, pixmap);
					}
					else
					{
						p->drawPixmap(left, top + (height-pixH)/2, pixmap);
						left += (pixW+4);
					}
					width -= (pixW+4);
				}
			}
			// The label
			p->setPen(textColor);
			p->drawText(QRect(left, top, width, height), textFlags, label);
			break;
		}
		
		case CE_DockWindowEmptyArea:
		{
			p->fillRect(r, getColor(cg, DefaultGround));
			break;
		}
		
		case CE_ToolBoxTab:
		{
			bool selected = flags & Style_Selected;
			//
			QColor ground = cg.background();
			QRect gr(r);				// Global area
			QRect ar(left+4, top, 15, height);	// Arrow zone (Label is at left+22.)
			QRect tr(r);				// Tab area
			//
			if (selected)
			{
				p->setPen( getColor(cg, PanelContour, enabled) );
				p->drawLine(left, bottom, right, bottom);
				tr.addCoords(2, 0, -2, 0);
				renderSereneTab(p, tr, cg, flags, false, Single);
				// Move the arrow to look nicer.
				ar.addCoords(4, 0, 0, 0);
			}
			else
			{
				p->setPen(ground);
				p->drawRect(gr);
				gr.addCoords(0, 1, 0, -1);
				p->drawRect(gr);
				gr.addCoords(0, 1, 0, -1);
				QColor surface = getColor(cg, TabGroove_Solid, enabled);
				if (hoverToolTab)
				{
					surface = colorMix(surface, getColor(cg, HoverColor), SRN_OVER);
				}
				renderFlatArea(p, gr, ground, surface, Button_Alike);
				
			}
			// The arrow indicator.
			renderHardMap(p, ar, cg.foreground(), 
					selected ? ARROWDOWN : ARROWRIGHT);
			break;
		}
		
		default:
			KStyle::drawControl(element, p, widget, r, cg, flags, opt);
	}
}

void SerenityStyle::drawControlMask(ControlElement element, 
				QPainter* p, 
				const QWidget* w, 
				const QRect &r, 
				const QStyleOption &opt) const
{
	switch (element)
	{
		case CE_PushButton:
		{
			p->fillRect(r, color0);
			renderMask(p, r, color1, Round_All);
			break;
		}
		default:
		{
			KStyle::drawControlMask (element, p, w, r, opt);
		}
	}
}

void SerenityStyle::drawComplexControlMask(ComplexControl c,
					 QPainter* p,
					 const QWidget* w,
					 const QRect &r,
					 const QStyleOption &o) const
{
	switch (c)
	{
		case CC_SpinWidget:
		case CC_ListView:
		case CC_ComboBox:
		{
			p->fillRect(r, color0);
			renderMask(p, r, color1, Round_All);
			break;
		}
		default:
		{
			KStyle::drawComplexControlMask (c, p, w, r, o);
		}
	}
}

void SerenityStyle::drawComplexControl(ComplexControl control, 
					 QPainter* p, 
					 const QWidget* widget, 
					 const QRect &r, 
					 const QColorGroup &cg, 
					 SFlags flags, 
					 SCFlags controls, 
					 SCFlags active, 
					 const QStyleOption& opt) const
{
	const bool enabled = (flags & Style_Enabled);
	
	switch (control)
	{
		case CC_ScrollBar:
		{
			const QScrollBar* sb = (const QScrollBar*)widget;
			bool disabled = (sb->minValue() == sb->maxValue());
			bool horizontal = (sb->orientation() == Qt::Horizontal);
			SFlags sflags = ((horizontal ? Style_Horizontal : Style_Default) 
					| (disabled ? Style_Default : Style_Enabled));
			
			QRect addline, subline, subline2, addpage, subpage, slider;
			subline = querySubControlMetrics(control, widget, SC_ScrollBarSubLine, opt);
			addline = querySubControlMetrics(control, widget, SC_ScrollBarAddLine, opt);
			subpage = querySubControlMetrics(control, widget, SC_ScrollBarSubPage, opt);
			addpage = querySubControlMetrics(control, widget, SC_ScrollBarAddPage, opt);
			slider = querySubControlMetrics(control, widget, SC_ScrollBarSlider, opt);
			subline2 = addline;
			if (SBSTF > -1) // Only if we have arrow buttons.
			{
				SFlags more;
				if (SBSTF == 1) // ThreeButtonScrollBar
				{
					if (horizontal)
						subline2.moveBy(-addline.width(), 0);
					else
						subline2.moveBy(0, -addline.height());
				}
				if ((controls & SC_ScrollBarSubLine) && subline.isValid())
				{
					// Only the button under the mouse must react.
					more = Style_Default;
					if (hoverThis == HOVER_SUBLINE)
					{
						if (active == SC_ScrollBarSubLine)
							more = Style_Down;
						else
							more = Style_MouseOver;
					}
					//
					drawPrimitive(PE_ScrollBarSubLine, p, subline, 
							cg, sflags|more);
					//
					if ( (SBSTF == 1) && (subline2.isValid()) )
					{
						more = Style_Default;
						if (hoverThis == HOVER_SUBLINE2)
						{
							if (active == SC_ScrollBarSubLine)
								more = Style_Down;
							else
								more = Style_MouseOver;
						}
						//
						drawPrimitive( PE_ScrollBarSubLine, p, subline2, 
								cg, sflags|more);
					}
				}
				if ((controls & SC_ScrollBarAddLine) && addline.isValid())
				{
					drawPrimitive( PE_ScrollBarAddLine, p, addline, cg,
							sflags 
							| (active == SC_ScrollBarAddLine 
								? Style_Down 
								: (hoverThis == HOVER_ADDLINE 
									? Style_MouseOver 
									: Style_Default)) );
				}
			}
			/// MXLS: Focus. (I don't think it's actually used...)
			if (sb->hasFocus())
			{
				sflags |= Style_HasFocus;
			}
			if ((controls & SC_ScrollBarSubPage) && subpage.isValid())
			{
				drawPrimitive( PE_ScrollBarSubPage, p, subpage, cg,
						sflags 
						| (active == SC_ScrollBarSubPage 
							? Style_Down 
							: Style_Default) );
			}
			if ((controls & SC_ScrollBarAddPage) && addpage.isValid())
			{
				drawPrimitive( PE_ScrollBarAddPage, p, addpage, cg,
						sflags 
						| (active == SC_ScrollBarAddPage 
							? Style_Down 
							: Style_Default) );
			}
			if ((controls & SC_ScrollBarSlider) && slider.isValid())
			{
				drawPrimitive( PE_ScrollBarSlider, p, slider, cg,
						sflags 
						| (active == SC_ScrollBarSlider 
							? Style_Down 
							: (hoverThis == HOVER_SLIDER 
								? Style_MouseOver 
								: Style_Default)) );
			}
			break;
		}
		
		case CC_ComboBox:
		{
			const QComboBox* cb = dynamic_cast<const QComboBox*>(widget);
			bool editable = false;
			bool hasFocus = false;
			if (cb)
			{
				editable = cb->editable();
				hasFocus = cb->hasFocus();
			}
			bool mouseOver = ((widget == hoverWidget) || (flags & Style_MouseOver));
			bool khtml = ( khtmlWidgets.contains(cb) );
			QColor ground = getColor(cg, DefaultGround);
			QColor surface = cg.button();
			QColor bSurface = editable ? getColor(cg, FrameContour, enabled)
						: getColor(cg, ComboMix, enabled);
			QColor textColor = cg.foreground();
			QColor hilite = getColor(cg, HoverColor);
			
			QRect work(r);
			int left, top, width, height;
			work.rect(&left, &top, &width, &height);
			int right, bottom;
			work.coords(&left, &top, &right, &bottom);
			//
			QRect contentArea, buttonArea;
			QRegion buttonRegion;
			int x, gx;
			if (_reverseLayout)
			{
				buttonArea = QRect(left, top, _controlWidth, height);
				contentArea = QRect(left+_controlWidth, top, 
							width-_controlWidth, height);
				x = contentArea.left();
				gx = buttonArea.right()-1;
				buttonRegion = buttonArea;
				buttonRegion += QRect(x, top, 2, 1);		// Some pixels
				buttonRegion += QRect(x, bottom, 2, 1);		// to look nice.
			}
			else
			{
				contentArea = QRect(left, top, 
							width-_controlWidth, height);
				buttonArea = QRect(contentArea.right()+1, top, 
							_controlWidth, height);
				x = contentArea.right();
				gx = x - 1;
				buttonRegion = buttonArea;
				buttonRegion += QRect(x-1, top, 2, 1);		// Some pixels
				buttonRegion += QRect(x-1, bottom, 2, 1);	// to look nice.
			}
			buttonRegion += QRect(x, top+1, 1, 1);		// More pixels
			buttonRegion += QRect(x, bottom-1, 1, 1);	// to look nicer.
			if (mouseOver)
			{
				surface = colorMix(surface, hilite, SRN_OVER);
				bSurface = colorMix(bSurface, hilite, SRN_OVER);
			}
			uint surfaceFlags = Button_Alike 
						| (khtml ? Draw_AlphaBlend : 0);
			if (!editable)
			{
				renderSurface(p, r, ground, 
							surface, hilite,
							surfaceFlags);
				p->setClipRegion(buttonRegion, QPainter::CoordPainter);
				renderSurface(p, r, ground, 
							bSurface, hilite,
							surfaceFlags);
				p->setClipping(false);
				if ( (_drawFocusRect) && (hasFocus) )
				{
					contentArea.addCoords(3, 3, -3, -3);
					drawPrimitive(PE_FocusRect, p, 
							contentArea, cg);
				}
			}
			else
			{
				// Draw the edit frame.
				renderSereneEditFrame(p, contentArea, cg, enabled, hasFocus, khtml);
				// The flat area for the arrow.
				surface = getColor(cg, FrameContour, enabled);
				renderFlatArea(p, buttonArea, ground, 
						surface, surfaceFlags);
				p->setPen(surface);
				p->drawLine(gx, top, gx+3, top);		// Fill the gap.
				p->drawLine(gx+1, top+1, gx+2, top+1);
				p->drawLine(gx+1, bottom-1, gx+2, bottom-1);
				p->drawLine(gx, bottom, gx+3, bottom);
				/// MXLS: Menu button is only raised 
				/// if the combobox has the cursor or is hovered,
				/// otherwise the button is highlighted when hovered.
				bool redraw = false;
				buttonArea.addCoords(1, 1, -1, -1);
				if ( mouseOver && (hoverThis == HOVER_COMBO) )
				{
					surfaceFlags = Button_Alike|Is_Hovered;
					redraw = true;
				}
				else if ( (hasFocus && !mouseOver)
				|| (mouseOver && (hoverThis == HOVER_NONE)) ) 
				{
					surfaceFlags = Button_Alike;
					redraw = true;
				}
				if (redraw)
				{
					renderSurface(p, buttonArea, surface, 
								surface, hilite, 
								surfaceFlags);
				}
			}
			// The arrow for both comboboxes.
			renderSoftMap(p, buttonArea, surface, 
					colorMix(bSurface, cg.buttonText(), 96),   
					ARROWDOWN);
			
			// QComboBox draws the text using cg.text(), 
			// we can override this from here.
			if (!editable)
			{
				p->setPen(cg.buttonText());
				p->setBackgroundColor(cg.button());
			}
			else
			{
				p->setPen(textColor);
				p->setBackgroundColor(cg.base());
			}
			break;
		}
		
		case CC_SpinWidget:
		{
			const QSpinWidget* sw = dynamic_cast<const QSpinWidget*>(widget);
			bool mouseOver = (widget == hoverWidget);
			bool khtml = ( khtmlWidgets.contains(sw) );
			bool hasFocus = false;
			if (sw)
				hasFocus = sw->hasFocus();
			QColor ground = getColor(cg, DefaultGround);
			QColor surface = getColor(cg, FrameContour, enabled);
			QColor upArrowColor = colorMix(surface, cg.text(), 96);
			QColor downArrowColor = upArrowColor;
			QColor hilite = getColor(cg, HoverColor);
			
			QRect work(r);
			int left, top, width, height;
			work.rect(&left, &top, &width, &height);
			int right, bottom;
			work.coords(&left, &top, &right, &bottom);
			//
			QRect contentArea, buttonArea;
			int gx;
			if (_reverseLayout)
			{
				buttonArea = QRect(left, top, _controlWidth, height);
				contentArea = QRect(left+_controlWidth, top, 
							width-_controlWidth, height);
				gx = buttonArea.right()-1;
			}
			else
			{
				contentArea = QRect(left, top, 
							width-_controlWidth, height);
				buttonArea = QRect(contentArea.right()+1, top, 
							_controlWidth, height);
				gx = contentArea.right() - 1;
			}
			uint surfaceFlags = Button_Alike 
						| (khtml ? Draw_AlphaBlend : 0);
			// Draw the edit frame.
			renderSereneEditFrame(p, contentArea, cg, enabled, hasFocus, khtml);
			// The flat area for the arrows.
			renderFlatArea(p, buttonArea, ground, 
					surface, surfaceFlags);
			p->setPen(surface);
			p->drawLine(gx, top, gx+3, top);		// Fill the gap.
			p->drawLine(gx+1, top+1, gx+2, top+1);
			p->drawLine(gx+1, bottom-1, gx+2, bottom-1);
			p->drawLine(gx, bottom, gx+3, bottom);
			//
			buttonArea.addCoords(1, 1, -1, -1);
			// Tiny buttons surface
			buttonArea.rect(&left, &top, &width, &height);
			buttonArea.coords(&left, &top, &right, &bottom);
			height /= 2;
			QRect upRect(left, top, width, height);
			QRect downRect(left, bottom-height+1, width, height);
			int redraw;
			SFlags sflagsUp, sflagsDown;
			// Draw a button behind the up arrow (eventually).
			sflagsUp = Style_Default | (enabled ? Style_Enabled : 0);
			if (active == SC_SpinWidgetUp)
			{
				renderSurface(p, upRect, surface, surface, 
						cg.highlight(), Button_Alike|Is_Sunken);
				sflagsUp |= Style_On|Style_Sunken;
			} 
			else
			{
				/// MXLS: Up and down buttons are raised 
				/// if the spin widget has the cursor or is hovered,
				/// except if one button is hovered and highlighted.
				redraw = false;
				if(! sw->isUpEnabled())
				{
					// Disable up arrow/symbol.
					upArrowColor = colorMix(surface, upArrowColor);
					sflagsUp = Style_Default;
				}
				else if ( mouseOver && (hoverThis == HOVER_SPIN_UP) )
				{
					// Hover the up button.
					sflagsUp |= Style_MouseOver;
					surfaceFlags = Button_Alike|Is_Hovered;
					redraw = true;
				}
				else if ( (hasFocus && !mouseOver)
				|| (mouseOver && (hoverThis == HOVER_NONE)) 
				|| (hasFocus && (mouseOver && (hoverThis != HOVER_SPIN_DOWN))) ) 
				{
					// Raise the up button.
					surfaceFlags = Button_Alike;
					redraw = true;
				}
				if (redraw)
				{
					renderSurface(p, upRect, surface, 
								surface, hilite, 
								surfaceFlags);
				}
				sflagsUp |= Style_Raised;
			}
			// Draw a button behind the down arrow (eventually).
			sflagsDown = Style_Default | (enabled ? Style_Enabled : 0);
			if (active == SC_SpinWidgetDown)
			{
				renderSurface(p, downRect, surface, surface, 
						cg.highlight(), Button_Alike|Is_Sunken);
				sflagsDown |= Style_On|Style_Sunken;
			} 
			else
			{
				redraw = false;
				if(! sw->isDownEnabled())
				{
					// Disable down arrow/symbol.
					downArrowColor = colorMix(surface, downArrowColor);
					sflagsUp = Style_Default;
				}
				else if ( mouseOver && (hoverThis == HOVER_SPIN_DOWN) )
				{
					// Hover the down button.
					sflagsDown |= Style_MouseOver;
					surfaceFlags = Button_Alike|Is_Hovered;
					redraw = true;
				}
				else if ( (hasFocus && !mouseOver)
				|| (mouseOver && (hoverThis == HOVER_NONE)) 
				|| (hasFocus && (mouseOver && (hoverThis != HOVER_SPIN_UP))) ) 
				{
					// Raise the down button.
					surfaceFlags = Button_Alike;
					redraw = true;
				}
				if (redraw)
				{
					renderSurface(p, downRect, surface, 
								surface, hilite, 
								surfaceFlags);
				}
				sflagsDown |= Style_Raised;
			}
			if (sw->buttonSymbols() == QSpinWidget::PlusMinus)
			{
				drawPrimitive(PE_SpinWidgetPlus, p, upRect, cg, sflagsUp);
				drawPrimitive(PE_SpinWidgetMinus, p, downRect, cg, sflagsDown);
			}
			else
			{
				renderHardMap(p, upRect, upArrowColor, ARROWUP);
				renderHardMap(p, downRect, downArrowColor, ARROWDOWN);
			}
			break;
		}
		
		case CC_ListView:
		{
			/// MXLS: Copied and pasted from "kstyles.cpp"...
			/// but simplified for Serenity.
			if (controls & SC_ListView)
			{
				// Wipe the area to make room for a new branch.
				/// MXLS: Two lines copied from "qcommonstyle.cpp".
				QListView *listview = (QListView*)widget;
				p->fillRect(r, listview->viewport()->backgroundBrush());
			}
			// If we have a branch or are expanded...
			if (controls & (SC_ListViewBranch|SC_ListViewExpand))
			{
				if (opt.isDefault()) // No item.
					break;
				
				QListViewItem *item  = opt.listViewItem();
				QListViewItem *child = item->firstChild();
				
				int left, top, right, bottom;
				r.coords(&left, &top, &right, &bottom);
				int x, y, width, height;
				r.rect(&x, &y, &width, &height);
				
				if ( active == SC_All && controls == SC_ListViewExpand )
				{
					// Only a vertical line
					if (_treeViewStyle == 0)
					{
						drawKStylePrimitive(KPE_ListViewBranch, p, NULL, 
								QRect(right, top, 1, bottom-top), 
								cg);
					}
				}
				else
				{
					int rowTop = 0;
					int rowBottom = 0;
					
					// Skip the stuff above the exposed rectangle
					while (child && y + child->height() <= 0)
					{
						y += child->totalHeight();
						child = nextVisibleSibling(child);
					}
					
					x += width/2;
					
					// Paint stuff in the magical area
					QListView* v = item->listView();
					int lh = QMAX(p->fontMetrics().height() + 2 * v->itemMargin(),
							QApplication::globalStrut().height());
					if (lh & 1) lh++;
					// Draw all the expand/close boxes...
					while (child && y < height)
					{
						rowBottom = y + lh/2;
						if ( (child->isExpandable() || child->childCount()) 
						&& (child->height() > 0) )
						{
							// Draw the branch.
							if (_treeViewStyle == 0)
							{
								drawKStylePrimitive(KPE_ListViewBranch, p, NULL, 
										QRect(x, rowTop, 1, rowBottom-rowTop), 
										cg);
								drawKStylePrimitive(KPE_ListViewBranch, p, NULL, 
										QRect(x, rowBottom, width-x, 1), 
										cg);
							}
							// Draw the expand/collapse indicator.
							drawKStylePrimitive(KPE_ListViewExpander, p, NULL, 
									QRect(x-7, rowBottom-7, 15, 15), 
									cg, 
									child->isOpen()
										? QStyle::Style_Off 
										: QStyle::Style_On, 
									opt);
							rowTop = rowBottom+8;
						}
						else if (_treeViewStyle != 2)
						{
							// Draw an horizontal line
							// to a child.
							drawKStylePrimitive(KPE_ListViewBranch, p, NULL, 
									QRect(x, rowBottom, width-x, 1), 
									cg);
						}
						
						y += child->totalHeight();
						child = nextVisibleSibling(child);
					}
					if (child)
					{
						// A child to draw, so move rowBottom to edge of rectangle.
						rowBottom = height;
					}
					if (rowTop < rowBottom)
					{
						// Draw a vertical line.
						drawKStylePrimitive(KPE_ListViewBranch, p, NULL, 
								QRect(x, rowTop, 1, rowBottom-rowTop), 
								cg);
					}
				}
			}
			break;
		}
		
		case CC_ToolButton:
		{
			bool down = flags & (Style_On|Style_Down);
			bool mouseOver = ( (widget == hoverWidget) || (flags & Style_MouseOver) );
			
			const QToolButton* tb = (const QToolButton*)widget;
						
			QRect button, menuarea;
			button = querySubControlMetrics(control, widget, SC_ToolButton, opt);
			menuarea = querySubControlMetrics(control, widget, SC_ToolButtonMenu, opt);
			
			SFlags bflags = flags, 
				mflags = flags;
			
			if (kornMode) 	/// MXLS: What is Korn???
			{
				drawPrimitive(PE_ButtonTool, p, button, cg, bflags, opt);
				break;
			} 
			// For the tab arrow buttons and (eventually) some more...
			Qt::ArrowType arrow = opt.arrowType();
			if ( (arrow == Qt::LeftArrow || arrow == Qt::RightArrow 
			|| arrow == Qt::UpArrow || arrow == Qt::DownArrow) 
			&& (tb->iconSet().isNull()) 
			&& (tb->textLabel().isEmpty() && tb->text().isEmpty()) )
			{
				/// MXLS; This is a mess!
				/// The QStyleOptions() can be set to "arrow-like" 
				/// although no arrow must be drawn.
				uint tabFlags = Square_Box;
				QColor hilite = cg.highlight();
				if (down)
				{
					tabFlags |= Is_Sunken;
				}
				else if (mouseOver)
				{
					hilite = getColor(cg, HoverColor);
					tabFlags |= Is_Hovered;
				}
				renderSurface(p, button, cg.background(), 
							getColor(cg, MixedSurface, enabled), 
							hilite, tabFlags);
				break;
			}
			// For the Add/Remove tab buttons
			// and the so-called QToolBarExtensionWidget...
			if ( tb->parentWidget() && (tb->parentWidget()->inherits("QTabWidget")
			|| tb->parentWidget()->inherits("QToolBarExtensionWidget") ) ) 
			{
				/// MXLS: The QTBEW is never down. FIXME: Fix Qt!
				renderButton(p, button, cg, down, mouseOver, enabled, !ALPHA, 
						TabAction_Type);
				break;
			}
			
			bflags &= ~Style_MouseOver;	// No mouseOver, no highlight.
			
			if (active & SC_ToolButton)
				bflags |= Style_Down;
			
			if (active & SC_ToolButtonMenu)
				mflags |= Style_Down;
			
			if (controls & SC_ToolButton)
			{
				// If we're pressed, on, or raised...
				if (bflags & (Style_Down | Style_On | Style_Raised) || widget==hoverWidget )
				{
					renderButton(p, button, cg, down, mouseOver, enabled, !ALPHA, 
							ToolButton_Type);
				} 
				else if (tb->parentWidget() && tb->parentWidget()->backgroundPixmap() &&
					 !tb->parentWidget()->backgroundPixmap()->isNull()) 
				{
					QPixmap pixmap = *(tb->parentWidget()->backgroundPixmap());
					p->drawTiledPixmap(r, pixmap, tb->pos());
				}
			}
			// Draw a menu indicator if required
			if (controls & SC_ToolButtonMenu)
			{
				mflags |= Style_MouseOver;
				if (mflags & (Style_Down | Style_On | Style_Raised))
				{
					drawPrimitive(PE_ButtonDropDown, p, menuarea, cg, mflags, opt);
				}
				renderHardMap(p, menuarea, cg.foreground(), ARROWDOWN);
			}
			if (_drawFocusRect)
			{
				if (tb->hasFocus() && !tb->focusProxy())
				{
					QRect fr(tb->rect());
					fr.addCoords(2, 2, -2, -2);
					drawPrimitive(PE_FocusRect, p, fr, cg);
				}
			}
			/// MXLS: Any use? I don't think so.
			if (down)
				p->setPen(cg.buttonText());
			else if (flags & Style_Raised)
				p->setPen(cg.foreground());
			else
				p->setPen(cg.buttonText());
			//
			break;
		}
		
		// Dock window / Sub-window
		case CC_TitleBar:
		{
			const QTitleBar* titlebar = (const QTitleBar*) widget;
			
			QRect ir;
			QColor ground = getColor(cg, DefaultGround);
			QColor hilite = cg.highlight();
			QColor surface;
			if (titlebar->usesActiveColor())	// Pseudo-window is active.
				surface = colorMix(ground, getColor(cg, HoverColor), SRN_DOWN);
			else
				surface = ground;
			
			if (controls & SC_TitleBarLabel)
			{
				p->fillRect( titlebar->rect(), QBrush(ground) );
				
				ir = visualRect(querySubControlMetrics(CC_TitleBar, widget, SC_TitleBarLabel), widget);
				renderSurface(p, ir, ground, surface, surface, 
							Button_Alike);
				p->setPen(cg.buttonText());
				p->drawText(ir.x()+2, ir.y(), ir.width()-4, ir.height(),
					AlignHCenter|AlignVCenter|SingleLine, titlebar->visibleText());
			}
			bool down = false;
			QPixmap pm;
			if (controls & SC_TitleBarCloseButton)
			{
				ir = visualRect(querySubControlMetrics(CC_TitleBar, widget, SC_TitleBarCloseButton), widget);
				down = active & SC_TitleBarCloseButton;
				if ( widget->testWFlags( WStyle_Tool )
				|| ::qt_cast<QDockWindow*>(widget) )
					pm = stylePixmap(SP_DockWindowCloseButton, widget);
				else
					pm = stylePixmap(SP_TitleBarCloseButton, widget);
				renderSurface(p, ir, ground, surface, hilite, 
						Button_Alike|(down ? Is_Sunken : 0) );
				drawItem(p, ir, AlignCenter, cg, true, &pm, QString::null);
			}
			if (titlebar->window())
			{
				if (controls & SC_TitleBarMaxButton)
				{
					ir = visualRect(querySubControlMetrics(CC_TitleBar, widget, SC_TitleBarMaxButton), widget);
					down = active & SC_TitleBarMaxButton;
					pm = QPixmap(stylePixmap(SP_TitleBarMaxButton, widget));
					renderSurface(p, ir, ground, surface, hilite, 
							Button_Alike|(down ? Is_Sunken : 0) );
					drawItem(p, ir, AlignCenter, cg, true, &pm, QString::null);
				}
				if (controls & SC_TitleBarNormalButton || controls & SC_TitleBarMinButton)
				{
					ir = visualRect(querySubControlMetrics(CC_TitleBar, widget, SC_TitleBarMinButton), widget);
					QStyle::SubControl ctrl = (controls & SC_TitleBarNormalButton ?
								SC_TitleBarNormalButton :
								SC_TitleBarMinButton);
					down = (controls & SC_TitleBarNormalButton 
						? true : active & ctrl);
					pm = QPixmap(stylePixmap(SP_TitleBarMinButton, widget));
					renderSurface(p, ir, ground, surface, hilite, 
							Button_Alike|(down ? Is_Sunken : 0) );
					drawItem(p, ir, AlignCenter, cg, true, &pm, QString::null);
				}
				if (controls & SC_TitleBarShadeButton)
				{
					ir = visualRect(querySubControlMetrics(CC_TitleBar, widget, SC_TitleBarShadeButton), widget);
					down = active & SC_TitleBarShadeButton;
					pm = QPixmap(stylePixmap(SP_TitleBarShadeButton, widget));
					renderSurface(p, ir, ground, surface, hilite, 
							Button_Alike|(down ? Is_Sunken : 0) );
					drawItem(p, ir, AlignCenter, cg, true, &pm, QString::null);
				}
				if (controls & SC_TitleBarUnshadeButton)
				{
					ir = visualRect(querySubControlMetrics(CC_TitleBar, widget, SC_TitleBarUnshadeButton), widget);
					pm = QPixmap(stylePixmap(SP_TitleBarShadeButton, widget));
					renderSurface(p, ir, ground, surface, hilite, 
							Button_Alike|Is_Sunken);
					drawItem(p, ir, AlignCenter, cg, true, &pm, QString::null);
				}
			}
			if (controls & SC_TitleBarSysMenu)
			{
				ir = visualRect(querySubControlMetrics(CC_TitleBar, widget, SC_TitleBarSysMenu), widget);
				renderSurface(p, ir, ground, surface, surface, 
						Button_Alike);
				if (titlebar->icon())
				{
					drawItem(p, ir, AlignCenter, cg, true, titlebar->icon(), QString::null);
				}
				else
				{
					renderHardMap(p, ir, cg.foreground(), ARROWDOWN);
				}
			}
			break;
		}
		
		default:
			KStyle::drawComplexControl(control, p, widget, r, cg, flags, controls, active, opt);
			break;
	}
}

void SerenityStyle::drawItem( QPainter* p, 
				const QRect &r, 
				int flags, 
				const QColorGroup &cg, 
				bool enabled, 
				const QPixmap* pixmap, 
				const QString &text, 
				int len, 
				const QColor* penColor ) const
{
	p->save();
	int x, y, w, h;
	r.rect(&x, &y, &w, &h);
	
	if (!enabled)
		p->setPen( colorMix(getColor(cg, DefaultGround), getColor(cg, DefaultPen)) );
	else
		p->setPen(penColor ? *penColor : cg.foreground());
	//
	if (pixmap)
	{
		QPixmap  pm(*pixmap);
		bool clip = (flags & Qt::DontClip) == 0;
		int pixW = pm.width();
		int pixH = pm.height();
		if (clip)
		{
			if ( (pixW > w) || (pixH > h) )
			{
				QRegion cr = QRect(x, y, w, h);
				if (p->hasClipping())
					cr &= p->clipRegion(QPainter::CoordPainter);
				p->setClipRegion(cr);
			}
		}
		if ( (flags & Qt::AlignVCenter) == Qt::AlignVCenter )
			y += h/2 - pixH/2;
		else if ( (flags & Qt::AlignBottom) == Qt::AlignBottom )
			y += h - pixH;
		if ( (flags & Qt::AlignRight) == Qt::AlignRight )
			x += w - pixW;
		else if ( (flags & Qt::AlignHCenter) == Qt::AlignHCenter )
			x += w/2 - pixW/2;
		else if ( ((flags & Qt::AlignLeft) != Qt::AlignLeft) 
		&& _reverseLayout ) // AlignAuto && rightToLeft
			x += w - pixW;
		
		if (!enabled)
		{
			if (pm.mask())			// Pixmap with a mask
			{
				if (! pm.selfMask())	// Mask is not a pixmap
				{
				    QPixmap pmm( *pm.mask() );
				    pmm.setMask( *((QBitmap*)&pmm) );
				    pm = pmm;
				}
			}
			else if (pm.depth() == 1) // Monochrome pixmap, no mask
			{
				pm.setMask( *((QBitmap*)&pm) );
			}
#ifndef QT_NO_IMAGE_HEURISTIC_MASK
			else	// Color pixmap, no mask
			{
				QString k;
				k.sprintf("$qt-drawitem-%x", pm.serialNumber());
				QPixmap* mask = QPixmapCache::find(k);
				bool del = false;
				if (!mask)
				{
					mask = new QPixmap(pm.createHeuristicMask());
					mask->setMask( *((QBitmap*)mask) );
					del = !QPixmapCache::insert(k, mask);
				}
				pm = *mask;
				if (del) delete mask;
			}
#endif
		}
		p->drawPixmap(x, y, pm);
	}
	else if (! text.isNull())
	{
		p->drawText( x, y, w, h, flags, text, len );
	}
	p->restore();
}

QRect SerenityStyle::subRect(SubRect r, const QWidget* widget) const
{
	QRect rw(widget->rect());
	//
	switch (r)
	{
		case SR_PushButtonFocusRect:
		{
			rw.addCoords(3, 3, -3, -3);
			return rw;
		}
		case SR_ComboBoxFocusRect:
		{
			return querySubControlMetrics(CC_ComboBox, widget, SC_ComboBoxEditField);
		}
		case SR_ToolBoxTabContents:
		{
			rw.addCoords(18, 0, -7, 0);
			return rw;
		}
		// We don't use KStyle progressbar subrect.
		case SR_ProgressBarGroove:
		{
			return rw;
		}
		case SR_ProgressBarContents:
		case SR_ProgressBarLabel:
		{
			rw.addCoords(2, 2, -2, -2);
			return rw;
		}
		case SR_CheckBoxFocusRect:
		case SR_RadioButtonFocusRect:
		{
			const QButton* button = dynamic_cast<const QButton*>( widget );
			QString bs = button->text();
			int fw = 6;
			if (! bs.isNull())
			{
				QFontMetrics fm = button->fontMetrics();
				fw += fm.width(bs);
				fw -= bs.contains('&') * fm.width('&');
				fw += bs.contains("&&") * fm.width('&');
			}
			else if (button->pixmap())	// The label can be a pixmap.
			{
				const QPixmap* bp = button->pixmap();
				fw += bp->width();
			}
			int x = _reverseLayout
				? rw.right()-pixelMetric(PM_IndicatorWidth)-3-fw 
				: rw.left()+pixelMetric(PM_IndicatorWidth)+3;
			//
			// To handle multiline labels.
			if (x<0)
			{
				fw += x; // fw = fw - abs(x)
				x = 0;
			}
			if ((x+fw) > rw.right())
				fw = rw.right()-x;

			return QRect(x, rw.top(), fw, rw.height()); 
		}
		default:
		{
			return KStyle::subRect(r, widget);
		}
	}
}

QRect SerenityStyle::querySubControlMetrics(ComplexControl control,
					const QWidget* widget,
					SubControl subcontrol,
					const QStyleOption &opt) const
{
	if (!widget)
	{
		return QRect();
	}
	
	QRect r(widget->rect());
	switch (control)
	{
		case CC_ToolButton:
		{
			const QToolButton* toolbutton = (const QToolButton*)widget;
			int mbi = pixelMetric(PM_MenuButtonIndicator, widget);
			
			switch (subcontrol)
			{
				// HACK: For the bigger arrow.
				r.addCoords(1, 1, -1, -1);
				case SC_ToolButton:
				{
					if (toolbutton->popup() && ! toolbutton->popupDelay())
						r.addCoords(0, 0, -mbi, 0);
					return r;
				}
				case SC_ToolButtonMenu:
				{
					if (toolbutton->popup() && ! toolbutton->popupDelay())
						return QRect(r.right()-mbi, r.top(),
								mbi, r.height());
					else
						return r;
				}
				default:
				{
					return KStyle::querySubControlMetrics(control, widget, subcontrol, opt);
				}
			}
			break;
		}
		case CC_ScrollBar:
		{
			/// MXLS: We take care only of the arrowless scrollbars.
			if (SBSTF == -1)
			{
				const QScrollBar* sb = (const QScrollBar*)widget;
				bool horizontal = sb->orientation() == Qt::Horizontal;
				int sliderstart = sb->sliderStart();
				int sbextent = pixelMetric(PM_ScrollBarExtent, widget);
				int maxlen = (horizontal ? sb->width() : sb->height());
				int sliderlen;
		
				// Calculate slider length.
				if (sb->maxValue() != sb->minValue())
				{
					uint range = sb->maxValue() - sb->minValue();
					sliderlen = (sb->pageStep() * maxlen) / (range + sb->pageStep());
					
					int slidermin = pixelMetric(PM_ScrollBarSliderMin, widget);
					if ( (sliderlen < slidermin) || (range > INT_MAX / 2) )
						sliderlen = slidermin;
					if (sliderlen > maxlen)
						sliderlen = maxlen;
				} 
				else
				{ 
					sliderlen = maxlen;
				}
				// Subcontrols
				switch (subcontrol)
				{
					case SC_ScrollBarSubLine:
					case SC_ScrollBarAddLine:
					{
						return QRect();
					}
					case SC_ScrollBarSubPage:
					{
						if (horizontal)
							return QRect(0, 0, sliderstart, sbextent);
						else
							return QRect(0, 0, sbextent, sliderstart);
					}
					case SC_ScrollBarAddPage:
					{
						if (horizontal)
						{
							return QRect(sliderstart + sliderlen, 0,
									maxlen - sliderstart - sliderlen,
									sbextent);
						}
						else
						{
							return QRect(0, sliderstart + sliderlen, 
									sbextent,
									maxlen - sliderstart - sliderlen);
						}
					}
					case SC_ScrollBarGroove:
					{
						return r;
					}
					case SC_ScrollBarSlider:
					{
						if (horizontal)
							return QRect(sliderstart, 0, sliderlen, sbextent);
						else
							return QRect(0, sliderstart, sbextent, sliderlen);
					}
					default:
						break;
				}
			}
			else
			{
				// KDE takes care of the other ones.
				return KStyle::querySubControlMetrics(control, widget, subcontrol, opt);
			}
			break;
		}
		case CC_ComboBox:
		{
			// Qt takes care of the reverse layout.
			QRect contentArea(r.left(), r.top(), 
						r.width()-_controlWidth, r.height());
			QRect buttonArea(contentArea.right()+1, r.top(), 
						_controlWidth, r.height());
			contentArea.addCoords(2, 2, -2, -2);
			//
			switch (subcontrol)
			{
				case SC_ComboBoxFrame:
					return r;
				
				case SC_ComboBoxEditField:
					return contentArea;
				
				case SC_ComboBoxArrow:
					return buttonArea;
				
				default:
				{
					return KStyle::querySubControlMetrics(control, widget, subcontrol, opt);
				}
			}
			break;
		}
		case CC_SpinWidget:
		{
			// Qt takes care of the reverse layout.
			QRect contentArea(r.left(), r.top(), 
						r.width()-_controlWidth, r.height());
			QRect buttonArea(contentArea.right()+1, r.top(), 
						_controlWidth, r.height());
			contentArea.addCoords(2, 2, -2, -2);
			//
			QRect work(buttonArea);
			work.addCoords(1, 1, -1, -1);
			int left, top, width, height;
			work.rect(&left, &top, &width, &height);
			int right, bottom;
			work.coords(&left, &top, &right, &bottom);
			height /= 2;
			QRect upRect(left, top, width, height);
			QRect downRect(left, bottom-height+1, width, height);
			//
			switch (subcontrol)
			{
				case SC_SpinWidgetFrame:
					return r;
				
				case SC_SpinWidgetEditField:
					return contentArea;
				
				case SC_SpinWidgetButtonField:
					return buttonArea;
				
				case SC_SpinWidgetUp:
					return upRect;
				
				case SC_SpinWidgetDown:
					return downRect;
				
				default:
				{
					return KStyle::querySubControlMetrics(control, widget, subcontrol, opt);
				}
			}
			break;
		}
		case CC_TitleBar:
		{
			const QTitleBar* titlebar = (const QTitleBar*)widget;
			//
			r.addCoords(0, 1, 0, -1);
			int left, top, width, size;
			r.rect(&left, &top, &width, &size);
			int right, bottom;
			r.coords(&left, &top, &right, &bottom);
			//
			switch (subcontrol)
			{
				case SC_TitleBarLabel:
				{
					if (titlebar->testWFlags(WStyle_Tool))
					{
						if (titlebar->testWFlags(WStyle_SysMenu))
							r.addCoords(0, 0, -size-2, 0);
						if (titlebar->testWFlags(WStyle_MinMax))
							r.addCoords(0, 0, -size-1, 0);
					}
					else
					{
						if (titlebar->testWFlags(WStyle_SysMenu))
							r.addCoords(size+2, 0, -size-2, 0);
						if (titlebar->testWFlags(WStyle_Minimize))
							r.addCoords(0, 0, -size-1, 0);
						if (titlebar->testWFlags(WStyle_Maximize))
							r.addCoords(0, 0, -size-1, 0);
					}
					return r;
				}
				case SC_TitleBarCloseButton:
				{
					return QRect(right-size, top, size, size);
				}
				case SC_TitleBarMaxButton:
				case SC_TitleBarShadeButton:
				case SC_TitleBarUnshadeButton:
				{
					return QRect(right-((size+1)*2)+1, top, size, size);
				}
				case SC_TitleBarMinButton:
				case SC_TitleBarNormalButton:
				{
					int offset = size+1;
					if (! titlebar->testWFlags(WStyle_Maximize))
						offset *= 2;
					else
						offset *= 3;
					return QRect(right-offset+1, top, size, size);
				}
				case SC_TitleBarSysMenu:
					return QRect(1, top, size, size);
				
				default:
					break;
			}
			break;
		}
		default:
			break;
	}
	return KStyle::querySubControlMetrics(control, widget, subcontrol, opt);
}

int SerenityStyle::pixelMetric(PixelMetric m, const QWidget* widget) const
{
	switch (m)
	{
		case PM_TabBarTabVSpace:
		{
			const QTabBar* tb = (const QTabBar*)widget;
			if (tb->shape() == QTabBar::RoundedAbove || tb->shape() == QTabBar::RoundedBelow)
				return 11;
			else
				return 6;
		}
		
		case PM_TabBarTabShiftVertical:		// Keep tabs aligned.
		case PM_TabBarTabShiftHorizontal:	//
		case PM_TabBarTabOverlap: 		// Without overlap.
		{
			return 0;
		}
		
		case PM_TabBarScrollButtonWidth:
			return 13;			// Not too wide.
		
		case PM_MenuBarFrameWidth:
			return 1;
	
		case PM_MenuBarItemSpacing:
			return (_menubarHack) ? 0 : 4;
		
		case PM_ToolBarItemSpacing:
			return 1;
		
		case PM_ScrollBarSliderMin:
			return 21;
		
		case PM_ScrollBarExtent:
			return 17;
		
		case PM_DockWindowSeparatorExtent:
			return 7;
		
		case PM_SplitterWidth:
			return 7;
		
		case PM_ProgressBarChunkWidth:
			return 10;
		
		// The handle: 23x17
		case PM_SliderLength:
			return 23;
		
		case PM_SliderThickness:
		case PM_SliderControlThickness:
		{
			return 17;
		}
		case PM_SliderTickmarkOffset:
			return 0;

		// Size of the arrow on buttons
		case PM_MenuButtonIndicator:
			return 7;
	
		case PM_ExclusiveIndicatorWidth:	// Radiobutton size
		case PM_ExclusiveIndicatorHeight:	// 15x15
		case PM_IndicatorWidth:			// Checkbox size
		case PM_IndicatorHeight:		// 15x15
		{
			return 15;
		}
		
		case PM_SpinBoxFrameWidth:
			return 1;
	
		case PM_DefaultFrameWidth:
		{
			if ( (widget)
			&& (::qt_cast<QLineEdit*>(widget) 
			|| ::qt_cast<QTextEdit*>(widget) 
			|| ::qt_cast<QComboBox*>(widget) 
			|| ::qt_cast<QLabel*>(widget)) )
			{
				return 2;
			}
			else if (widget && ::qt_cast<QFrame*>(widget))
				return 1;
			else
				return 2;
		}
		
		case PM_PopupMenuFrameHorizontalExtra:
		case PM_PopupMenuFrameVerticalExtra:
		{
			return 0;
		}
		
		case PM_ButtonDefaultIndicator:
			return 0;
		
		case PM_ButtonMargin:
			return 2;
		
		case PM_ButtonShiftVertical:
		case PM_ButtonShiftHorizontal:
		{
			return 0;		// Don't shift!
		}
		
		default:
			return KStyle::pixelMetric(m, widget);
	}
}

QSize SerenityStyle::sizeFromContents(ContentsType t,
					const QWidget* widget,
					const QSize &s,
					const QStyleOption &opt) const
{
	if (!widget)
		return s;
	
	switch (t)
	{
		case CT_PopupMenuItem:
		{
			if (opt.isDefault())
				return s;
			
			const QPopupMenu* popup = dynamic_cast<const QPopupMenu*>(widget);
			QMenuItem* mi = opt.menuItem();
			int maxpmw = opt.maxIconWidth();
			int w = s.width(), h = s.height();
			bool checkable = popup->isCheckable();
			
			if (mi->custom())
			{
				w = mi->custom()->sizeHint().width();
				h = mi->custom()->sizeHint().height();
				if (! mi->custom()->fullSpan())
					h += 4;
			}
			else if (mi->widget())
			{
				// No change in this case.
			} 
			else if (mi->isSeparator())
			{
				w = 20;
				h = 5;	// Menu separator height.
			} 
			else
			{
				if (mi->pixmap())
				{
					h = QMAX(h, mi->pixmap()->height() + 2);
				}
				else
				{
					h = QMAX(h, 18);
					h = QMAX(h, popup->fontMetrics().height() + 4 );
				}
		
				if (mi->iconSet())
				{
					h = QMAX(h, mi->iconSet()->pixmap(QIconSet::Small, QIconSet::Normal).height() + 2);
				}
			}
			if (!mi->text().isNull() && (mi->text().find('\t') >= 0))
			{
				w += itemHMargin + itemFrame*2 + 7;
			} 
			else if (mi->popup())
			{
				w += 2 * arrowHMargin;
			}
			if (maxpmw)
			{
				w += maxpmw + 6;
			}
			if (checkable && maxpmw < 20)
			{
				w += 20 - maxpmw;
			}
			if (checkable || maxpmw > 0)
			{
				w += 12;
			}
			w += rightBorder;
			return QSize(w, h);
		}
	
		case CT_PushButton:
		{
			const QPushButton* button = static_cast<const QPushButton*>(widget);
			int w = s.width() + 2 * pixelMetric(PM_ButtonMargin, widget);
			int h = s.height() + 2 * pixelMetric(PM_ButtonMargin, widget);
			if ( button->text().isEmpty() && s.width() < 32 )
				return QSize(w, h);
				
			w += 20;
			
			if (button->isDefault() || button->autoDefault())
			{
				if (w<80 && !button->pixmap())
					w = 80;
			}
			if (h < 26)	h = 26; // Much better than 22!
			
			return QSize(w, h);
		}
	
		case CT_ToolButton:
		{
			if (widget->parent() && ::qt_cast<QToolBar*>(widget->parent()))
				return QSize(s.width()+8, s.height()+8);
		}
		default:
			return KStyle::sizeFromContents(t, widget, s, opt);
	}
	//return KStyle::sizeFromContents (t, widget, s, opt);
}

int SerenityStyle::styleHint(StyleHint stylehint,
				const QWidget* widget,
				const QStyleOption &option,
				QStyleHintReturn* returnData) const
{
	switch (stylehint)
	{
		case SH_PopupMenu_SubMenuPopupDelay:
			return _submenuDelay;
		
		case SH_EtchDisabledText:
		case SH_MainWindow_SpaceBelowMenuBar:
		case SH_ToolBox_SelectedPageTitleBold:
			return 0;
		
		case SH_PopupMenu_SpaceActivatesItem:
			return 1;
		
		case SH_TabBar_Alignment:
			if (_centerTabs) return AlignHCenter;

		case SH_LineEdit_PasswordCharacter:
		{
		    if (widget) {
			const QFontMetrics &fm = widget->fontMetrics();
			if (fm.inFont(QChar(0x25CF)))
			{
			    return 0x25CF;
			}
			else if (fm.inFont(QChar(0x2022)))
			{
			    return 0x2022;
			}
		    }
		    return '*';
		}
		
		default:
			return KStyle::styleHint(stylehint, widget, option, returnData);
	}
}

QPixmap SerenityStyle::stylePixmap( StylePixmap stylepixmap,
				const QWidget* widget,
				const QStyleOption &opt) const
{
	QPalette pal = QApplication::palette();
	QColor pixColor = pal.color(QPalette::Active, QColorGroup::Foreground);
	QString color = "# c ";
	color.append(pixColor.name());
	/// MXLS: HACK: To replace XPM[1] changes the color of the icon.
	switch (stylepixmap)
	{
		case SP_DockWindowCloseButton:
		{
			dock_window_close_xpm[1] = color.QString::latin1();
			return QPixmap(const_cast<const char**>(dock_window_close_xpm ));
		}
		case SP_TitleBarCloseButton:
		{
			serene_close_xpm[1] = color.QString::latin1();
			return QPixmap(const_cast<const char**>(serene_close_xpm));
		}
		case SP_TitleBarMaxButton:
		{
			serene_maximize_xpm[1] = color.QString::latin1();
			return QPixmap(const_cast<const char**>(serene_maximize_xpm));
		}
		case SP_TitleBarMinButton:
		{
			serene_minimize_xpm[1] = color.QString::latin1();
			return QPixmap(const_cast<const char**>(serene_minimize_xpm));
		}
		case SP_TitleBarNormalButton:
		{
			serene_restore_xpm[1] = color.QString::latin1();
			return QPixmap(const_cast<const char**>(serene_restore_xpm));
		}
		case SP_TitleBarShadeButton:
		{
			serene_shade_xpm[1] = color.QString::latin1();
			return QPixmap(const_cast<const char**>(serene_shade_xpm));
		}
		case SP_TitleBarUnshadeButton:
		{
			serene_unshade_xpm[1] = color.QString::latin1();
			return QPixmap(const_cast<const char**>(serene_unshade_xpm));
		}
		default:
			return KStyle::stylePixmap(stylepixmap, widget, opt);
	}
}

bool SerenityStyle::eventFilter(QObject* obj, QEvent* ev)
{
	if ( KStyle::eventFilter(obj, ev) )
		return true;
	
	if (! obj->isWidgetType()) return false;

	/// MouseOver effect
	if (::qt_cast<QScrollBar*>(obj))
	{
		QScrollBar* sb = static_cast<QScrollBar*>(obj);
		if (! sb->isEnabled())
			return false;
		//
		QRect addline, subline, subline2, subpage, addpage, slider;
		subline = querySubControlMetrics(CC_ScrollBar, sb, SC_ScrollBarSubLine);
		addline = querySubControlMetrics(CC_ScrollBar, sb, SC_ScrollBarAddLine);
		subpage = querySubControlMetrics(CC_ScrollBar, sb, SC_ScrollBarSubPage);
		addpage = querySubControlMetrics(CC_ScrollBar, sb, SC_ScrollBarAddPage);
		slider = querySubControlMetrics(CC_ScrollBar, sb, SC_ScrollBarSlider);
		subline2 = addline;
		if (SBSTF == 1) // ThreeButtonScrollBar
		{
		
			if (sb->orientation() == Qt::Horizontal)
				subline2.moveBy(-addline.width(), 0);
			else
				subline2.moveBy(0, -addline.height());
		}
		bool redraw = false;
		if (ev->type() == QEvent::Enter)
		{
			hoverWidget = sb;
			if (mouseWithin(sb, subline))
			{
				redraw = true;
				hoverThis = HOVER_SUBLINE;
			}
			else if ( (SBSTF == 1) && (mouseWithin(sb, subline2)) )
			{
				redraw = true;
				hoverThis = HOVER_SUBLINE2;
			}
			else if (mouseWithin(sb, addline))
			{
				redraw = true;
				hoverThis = HOVER_ADDLINE;
			}
			else if ( mouseWithin(sb, subpage) || mouseWithin(sb, addpage) )
			{
				hoverThis = HOVER_PAGES;
			}
			else if (mouseWithin(sb, slider))
			{
				redraw = true;
				hoverThis = HOVER_SLIDER;
			}
			else
			{
				hoverThis = HOVER_NONE;
			}
			if (redraw)
				sb->repaint(false);
			return false;
		}
		else if (ev->type() == QEvent::MouseMove)
		{
			hoverWidget = sb;
			if ( mouseWithin(sb, subline) 
			&& (hoverThis != HOVER_SUBLINE) ) 
			{
				redraw = true;
				hoverThis = HOVER_SUBLINE;
			}
			else if ( (SBSTF == 1) && (mouseWithin(sb, subline2)) 
			&& (hoverThis != HOVER_SUBLINE2) )
			{
				redraw = true;
				hoverThis = HOVER_SUBLINE2;
			}
			else if ( mouseWithin(sb, addline) 
			&& (hoverThis != HOVER_ADDLINE) ) 
			{
				redraw = true;
				hoverThis = HOVER_ADDLINE;
			}
			else if ( (mouseWithin(sb, subpage) || mouseWithin(sb, addpage)) 
			&& (hoverThis != HOVER_PAGES) ) 
			{
				redraw = true;
				hoverThis = HOVER_PAGES;
			}
			else if ( mouseWithin(sb, slider) 
			&& (hoverThis != HOVER_SLIDER) ) 
			{
				redraw = true;
				hoverThis = HOVER_SLIDER;
			}
			if (redraw)
				sb->repaint(false);
			return false;
		}
		else if ( (ev->type() == QEvent::Leave) && (obj == hoverWidget) )
		{
			hoverWidget = 0;
			hoverThis = HOVER_NONE;
			sb->repaint(false);
			return false;
		}
		return false;
	}
	//
	if (::qt_cast<QCheckBox*>(obj) || ::qt_cast<QRadioButton*>(obj))
	{
		QButton* button = static_cast<QButton*>(obj);
		if (! button->isEnabled())
			return false;
		//
		QRect work(button->rect());
		int left, top, width, height;
		work.rect(&left, &top, &width, &height);
		int right, bottom;
		work.coords(&left, &top, &right, &bottom);
		//
		QRect hoverRect = subRect(SR_CheckBoxFocusRect, button);
		if (_reverseLayout)
		{
			hoverRect.setRight(right);
		}
		else
		{
			hoverRect.setLeft(left);
		}
		bool redraw = false;
		bool within = mouseWithin(button, hoverRect);
		if (ev->type() == QEvent::Enter)
		{
			hoverWidget = button;
			if (within)
			{
				redraw = true;
				hoverThis = HOVER_CHECKRADIO;
			}
			else
			{
				hoverThis = HOVER_NONE;
			}
			if (redraw)
				button->repaint(false);
			return false;
		}
		else if (ev->type() == QEvent::MouseMove)
		{
			hoverWidget = button;
			if ( within && (hoverThis != HOVER_CHECKRADIO) )
			{
				redraw = true;
				hoverThis = HOVER_CHECKRADIO;
			}
			else if ( (!within) && (hoverThis != HOVER_NONE) )
			{
				redraw = true;
				hoverThis = HOVER_NONE;
			}
			if (redraw)
				button->repaint(false);
			return false;
		}
		else if ( (ev->type() == QEvent::Leave) && (obj == hoverWidget) )
		{
			hoverWidget = 0;
			hoverThis = HOVER_NONE;
			button->repaint(false);
			return false;
		}
		return false;
	}
	//
	if (::qt_cast<QSpinWidget*>(obj))
	{
		QSpinWidget* sw = static_cast<QSpinWidget*>(obj);
		if (! sw->isEnabled())
			return false;
		
		QRect work = querySubControlMetrics(CC_SpinWidget, sw, SC_SpinWidgetButtonField);
		if (_reverseLayout)
			work.moveLeft(0);
		int left, top, width, height;
		work.rect(&left, &top, &width, &height);
		int right, bottom;
		work.coords(&left, &top, &right, &bottom);
		height /= 2;
		QRect spinUp(left, top, width, height);
		QRect spinDown(left, bottom-height+1, width, height);
		
		bool redraw = false;
		bool withinUp = mouseWithin(sw, spinUp);
		bool withinDown = mouseWithin(sw, spinDown);
		if (ev->type() == QEvent::Enter)
		{
			hoverWidget = sw;
			if (withinUp)
				hoverThis = HOVER_SPIN_UP;
			else if (withinDown)
				hoverThis = HOVER_SPIN_DOWN;
			else
				hoverThis = HOVER_NONE;
			//
			sw->repaint(false);
			return false;
		}
		else if (ev->type() == QEvent::MouseMove)
		{
			hoverWidget = sw;
			if ( withinUp && (hoverThis != HOVER_SPIN_UP) )
			{
				redraw = true;
				hoverThis = HOVER_SPIN_UP;
			}
			else if ( withinDown && (hoverThis != HOVER_SPIN_DOWN) )
			{
				redraw = true;
				hoverThis = HOVER_SPIN_DOWN;
			}
			else if ( (!withinUp) && (!withinDown)
			&& (hoverThis != HOVER_NONE) ) 
			{
				redraw = true;
				hoverThis = HOVER_NONE;
			}
			if (redraw)
				sw->repaint(false);
			return false;
		}
		else if ( (ev->type() == QEvent::Leave) && (obj == hoverWidget) )
		{
			hoverWidget = 0;
			hoverThis = HOVER_NONE;
			sw->repaint(false);
			return false;
		}
		return false;
	}
	//
	if (::qt_cast<QComboBox*>(obj))
	{
		QComboBox* cb = static_cast<QComboBox*>(obj);
		if (! cb->isEnabled())
			return false;
		
		// Here, we deal only with editable comboboxes.
		if (cb->editable())
		{
			QRect arrow = querySubControlMetrics(CC_ComboBox, cb, SC_ComboBoxArrow);
			if (_reverseLayout)	arrow.moveLeft(0);
			bool redraw = false;
			bool within = mouseWithin(cb, arrow);
			if (ev->type() == QEvent::Enter)
			{
				hoverWidget = cb;
				if (within)
					hoverThis = HOVER_COMBO;
				else
					hoverThis = HOVER_NONE;
				//
				cb->repaint(false);
				return false;
			}
			else if (ev->type() == QEvent::MouseMove)
			{
				hoverWidget = cb;
				if (within && (hoverThis != HOVER_COMBO))
				{
					redraw = true;
					hoverThis = HOVER_COMBO;
				}
				else if ( (!within) && (hoverThis != HOVER_NONE) )
				{
					redraw = true;
					hoverThis = HOVER_NONE;
				}
				if (redraw)
					cb->repaint(false);
				return false;
			}
			else if ( (ev->type() == QEvent::Leave) && (obj == hoverWidget) )
			{
				hoverWidget = 0;
				hoverThis = HOVER_NONE;
				cb->repaint(false);
				return false;
			}
			return false;
		}
	}
	//
	if ( obj->isA("KDockButton_Private") )
	{
		if (ev->type() != QEvent::Paint)
			return false;
		//
		QButton* btn = static_cast<QButton*>(obj);
		QColorGroup cg = btn->colorGroup();
		QRect work(btn->rect());
		QPainter p(btn);
		if (btn->hasMouse())
		{
			renderButton(&p, work, cg, btn->isOn() || btn->isDown(), 
					true, ENABLED, !ALPHA, ToolButton_Type);
		}
		else
		{
			p.fillRect( work, QBrush(cg.background()) );
		}
		BitmapItem symbol = ARROWDOWN;	// Should never happen!
		QString bname = btn->name();
		if (! qstrcmp(bname, "DockCloseButton"))
			symbol = DOCKCLOSE;
		else if (! qstrcmp(bname, "ToDesktopButton"))
			symbol = DOCKOUT;
		else if (! qstrcmp(bname, "DockbackButton"))
			symbol = DOCKIN;
		else if (! qstrcmp(bname, "DockStayButton"))
			symbol = DOCKSTAY;
		
		renderHardMap(&p, work, cg.foreground(), symbol);
		return true;
	}
	//
	if ( ::qt_cast<QPushButton*>(obj) || ::qt_cast<QComboBox*>(obj) 
	|| ::qt_cast<QToolButton*>(obj) || ::qt_cast<QSlider*>(obj) 
	|| obj->inherits("QSplitterHandle") || obj->inherits("QDockWindowHandle") 
	|| obj->inherits("QDockWindowResizeHandle") ) 
	{
		if ( (ev->type() == QEvent::Enter) && (static_cast<QWidget*>(obj)->isEnabled()) )
		{
			QWidget* button = static_cast<QWidget*>(obj);
			hoverWidget = button;
			button->repaint(false);
			return false;
		}
		else if ( (ev->type() == QEvent::Leave) && (obj == hoverWidget) )
		{
			QWidget* button = static_cast<QWidget*>(obj);
			hoverWidget = 0;
			button->repaint(false);
			return false;
		}
	}
	
	// Focus highlight
	if (::qt_cast<QLineEdit*>(obj))
	{
		QLineEdit* lineEdit = static_cast<QLineEdit*>(obj);
		
		if (::qt_cast<QSpinWidget*>(lineEdit->parentWidget()))
		{
			QWidget* spinbox = lineEdit->parentWidget();
			if ((ev->type() == QEvent::FocusIn) || (ev->type() == QEvent::FocusOut))
			{
				spinbox->repaint(false);
			}
			return false;
		}
		
		if ((ev->type() == QEvent::FocusIn) || (ev->type() == QEvent::FocusOut))
		{
			lineEdit->repaint(false);
		}
		return false;
	}
	
	// Tabbar mouseover effect
	if (::qt_cast<QTabBar*>(obj))
	{
		if ((ev->type() == QEvent::Enter) && static_cast<QWidget*>(obj)->isEnabled())
		{
			QWidget* tabbar = static_cast<QWidget*>(obj);
			hoverWidget = tabbar;
			hoverTab = 0;
			tabbar->repaint(false);
		}
		else if (ev->type() == QEvent::MouseMove)
		{
			QTabBar* tabbar = dynamic_cast<QTabBar*>(obj);
			QMouseEvent* me = dynamic_cast<QMouseEvent*>(ev);
	
			if (tabbar && me)
			{
				// Avoid unnecessary repaints
				bool repaint = true;
				// Go through the tabbar and see which tabs are hovered by the mouse.
				int tabCount = 0;
				for (int i = 0; i < tabbar->count(); ++i)
				{
					QTab* tab = tabbar->tab(i);
					if (tab && tab->rect().contains(me->pos()))
					{
						tabCount++;
						if (tabCount < 2)
						{
							// Only one tab under the mouse.
							if (hoverTab==tab)
								repaint = false; // Has been updated before, no repaint necessary
							hoverTab = tab;
						} 
						else // Shouldn't happen. Overlap is set to 0.
						{
							repaint = true;
							hoverTab = 0; // Make sure the tabbar is repainted next time too.
						}
					}
				}
				if (repaint)
					tabbar->repaint(false);
			}
		}
		else if (ev->type() == QEvent::Leave)
		{
			QWidget* tabbar = static_cast<QWidget*>(obj);
			hoverWidget = 0;
			hoverTab = 0;
			tabbar->repaint(false);
		}
		return false;
	}
	
	if ( obj->parent() && obj->parent()->isA("QToolBox") )
	{
		QWidget* tbtab = dynamic_cast<QWidget*>(obj);
		if (tbtab->isEnabled())
		{
			if (ev->type() == QEvent::Enter)
			{
				hoverToolTab = true;
				tbtab->repaint(false);
			}
			else if (ev->type() == QEvent::Leave)
			{
				hoverToolTab = false;
				tbtab->repaint(false);
			}
			else if (ev->type() == QEvent::Move)
			{
				hoverToolTab = false;
			}
		}
		return false;
	}
	
	// Track show events for progress bars
	if ( _animateProgressBar && ::qt_cast<QProgressBar*>(obj) )
	{
		if ((ev->type() == QEvent::Show) && !animationTimer->isActive())
		{
			animationTimer->start(50, false);
		}
		return false;
	}
	
	if (! qstrcmp(obj->name(), "kde toolbar widget"))
	{
		QWidget* lb = static_cast<QWidget*>(obj);
		if (lb->backgroundMode() == Qt::PaletteButton)
			lb->setBackgroundMode(Qt::PaletteBackground);
		lb->removeEventFilter(this);
		return false;
	}
	
	/// MXLS: Prevents any change of the status bar background.
	if (::qt_cast<QStatusBar*>(obj))
	{
		if (ev->type() != QEvent::PaletteChange)
			return false;
		//
		QWidget* w = static_cast<QWidget*>(obj);
		QPalette pal = QApplication::palette();
		w->setPaletteBackgroundColor( pal.color(QPalette::Active, QColorGroup::Background) );
		w->setPaletteForegroundColor( pal.color(QPalette::Active, QColorGroup::Foreground) );
		return false;
	}

	if (_menubarHack)
	{
		if (::qt_cast<QMenuBar*>(obj) && (ev->type() == QEvent::Paint))
		{
			QMenuBar* menubar = static_cast<QMenuBar*>(obj);
			for (uint index = 0; index < menubar->count(); index++)
			{
				int id = menubar->QMenuData::idAt(index);
				QString entry = menubar->text(id);
				if (! entry.startsWith(" "))
				{
					entry = ' ' + entry + ' ';
					menubar->changeItem(id, entry);
				}
			}
			return false;
		}
	}
	
	if ( obj->isA("AppletHandleButton") )
	{
		if (ev->type() != QEvent::Paint)
			return false;
		//
		QButton* btn = static_cast<QButton*>(obj);
		QColorGroup cg = btn->colorGroup();
		QRect work(btn->rect());
		QPainter p(btn);
		QColor color = getColor(cg, MixedSurface);
		uint contourFlags = Square_Box|Serene_Drawing;
		if (btn->isDown())
		{
			color = colorMix(color, cg.highlight(), SRN_DOWN);
			contourFlags |= Is_Sunken;
		}
		else if (btn->hasMouse())
		{
			color = colorMix(color, getColor(cg, HoverColor), SRN_OVER);
		}
		p.fillRect(work, QBrush(color));
		renderSereneContour(&p, work, cg.background(), color, contourFlags);
		renderHardMap(&p, work, cg.foreground(), APPLETMENU);
		return true;
	}
	
	if (::qt_cast<KKeyButton*>(obj))
	{
		if (ev->type() != QEvent::Paint)
			return false;
		//
		QButton* button = static_cast<QButton*>(obj);
		QColorGroup cg = button->colorGroup();
		QRect work = button->rect();
		QRect outer = work;
		outer.addCoords(1, 1, -1, -1);
		int oLeft, oTop, oWidth, oHeight;
		outer.rect(&oLeft, &oTop, &oWidth, &oHeight);
		QRect inner = outer;
		inner.addCoords(7, 5, -7, -5);
		int iLeft, iTop, iWidth, iHeight;
		inner.rect(&iLeft, &iTop, &iWidth, &iHeight);
		QColor ground = getColor(cg, DefaultGround);
		QColor surface = colorMix(cg.base(), cg.text(), 172);
		bool down = button->isDown();
		bool mouseOver = button->hasMouse();
		if (down)
		{
			outer.addCoords(1, 1, -1, -1);
			outer.rect(&oLeft, &oTop, &oWidth, &oHeight);
			inner.addCoords(1, 1, -1, -1);
			inner.rect(&iLeft, &iTop, &iWidth, &iHeight);
		}
		else if (mouseOver)
		{

			surface = colorMix(surface, getColor(cg, HoverColor), SRN_OVER);
		}
		QPainter p(button);
		renderDiagonalGradient( &p, QRect(oLeft+1, oTop+1, oWidth-2, oHeight-2),
					gradientColor(surface, TOP, true), 
					surface, 
					gradientColor(surface, BOTTOM, true) );
		renderSereneContour(&p, outer, ground, surface, Button_Alike|Serene_Drawing);
		if (down)
		{
			renderSereneContour(&p, QRect(oLeft-1, oTop-1, oWidth+2, oHeight+2), 
						ground, ground, 
						Button_Alike|Is_Sunken|Serene_Drawing);
		}
		renderSereneContour(&p, work, ground, ground, 
					Button_Alike|Is_Sunken|Serene_Drawing);
		renderDiagonalGradient( &p, QRect(iLeft+1, iTop+1, iWidth-2, iHeight-2),
					gradientColor(surface, BOTTOM, true),
					surface, 
					gradientColor(surface, TOP, true) );
		renderSereneContour(&p, inner, surface, surface, 
					Button_Alike|Is_Sunken|Draw_AlphaBlend|Serene_Drawing);
		p.setPen(cg.text());
		p.drawText(inner, AlignVCenter|AlignHCenter|DontClip|SingleLine|ShowPrefix, 
				button->text());
		if ( (_drawFocusRect) && (button->hasFocus()) && !(down||mouseOver) )
		{
			p.setRasterOp(Qt::NotROP);
			p.setPen( colorMix(cg.highlight(), cg.text(), 160) );
			p.drawRect(iLeft-2, iTop-2, iWidth+4, iHeight+4);
		}
		return true;
	}
	
	if (::qt_cast<KColorButton*>(obj))
	{
		if (ev->type() != QEvent::Paint)
			return false;
		//
		KColorButton* button = static_cast<KColorButton*>(obj);
		QColorGroup cg = button->colorGroup();
		QColor surface = cg.button();
		QColor hilite = getColor(cg, HoverColor);
		QRect work = button->rect();
		QRect inner = subRect(SR_PushButtonFocusRect, button);
		bool enabled = button->isEnabled();
		bool mouseOver = button->hasMouse();
		QColor innerColor = button->color();
		bool valid = innerColor.isValid();
		if (enabled && mouseOver)
		{
			surface = colorMix(surface, hilite, SRN_OVER);
		}
		QPainter p(button);
		renderSurface(&p, work, getColor(cg, DefaultGround), 
					surface, hilite,
					Button_Alike);
		inner.addCoords(2, 2, -2, -2);
		if (enabled && valid)
		{
			p.fillRect( inner, QBrush(innerColor) );
		}
		inner.addCoords(-1, -1, 1, 1);
		renderSereneContour(&p, inner, surface, surface,
					Button_Alike|Is_Sunken|Keep_Color|Draw_AlphaBlend);
		if (button->text())
		{
			if (enabled && valid)
			{
				if (qGray(innerColor.rgb()) < 129)
					p.setPen(white);
				else
					p.setPen(black);
			}
			else
				p.setPen(cg.buttonText());
			//
			p.drawText(inner, AlignVCenter|AlignHCenter|DontClip|SingleLine|ShowPrefix, 
					button->text());
		}
		if ( (_drawFocusRect) && (button->hasFocus()) )
		{
			inner.addCoords(-1, -1, 1, 1);
			p.setPen( colorMix(cg.highlight(), cg.foreground(), 160) );
			p.drawRect(inner);
		}
		return true;
	}
	
	if (::qt_cast<KColorCombo*>(obj) || obj->inherits("ColourCombo"))
	{
		if (ev->type() != QEvent::Paint)
			return false;
		//
		QComboBox* combo = static_cast<QComboBox*>(obj);
		bool enabled = combo->isEnabled();
		bool mouseOver = combo->hasMouse();
		QColorGroup cg = combo->colorGroup();
		QColor ground = getColor(cg, DefaultGround);
		QColor surface = cg.button();
		QColor bSurface = getColor(cg, ComboMix, enabled);
		QColor arrowColor = colorMix(bSurface, cg.buttonText(), 96);
		QColor hilite = getColor(cg, HoverColor);
		QColor innerColor;
		if (obj->inherits("ColourCombo"))
		{
			int idx = combo->currentItem();
			const QPixmap* pix = combo->pixmap(idx);
			QImage img = pix->convertToImage();
			innerColor = img.pixel(0, 0);
		}
		else
		{
			KColorCombo* combo = static_cast<KColorCombo*>(obj);
			innerColor = combo->color();
		}
		bool valid = innerColor.isValid();
		QRect r(combo->rect());
		QRect work(r);
		int left, top, width, height;
		work.rect(&left, &top, &width, &height);
		int right, bottom;
		work.coords(&left, &top, &right, &bottom);
		//
		QRect contentArea, buttonArea;
		QRegion buttonRegion;
		int x, gx;
		if (_reverseLayout)
		{
			buttonArea = QRect(left, top, _controlWidth, height);
			contentArea = QRect(left+_controlWidth, top, 
						width-_controlWidth, height);
			x = contentArea.left();
			gx = buttonArea.right()-1;
			buttonRegion = buttonArea;
			buttonRegion += QRect(x, top, 2, 1);		// Some pixels
			buttonRegion += QRect(x, bottom, 2, 1);		// to look nice.
		}
		else
		{
			contentArea = QRect(left, top, 
						width-_controlWidth, height);
			buttonArea = QRect(contentArea.right()+1, top, 
						_controlWidth, height);
			x = contentArea.right();
			gx = x - 1;
			buttonRegion = buttonArea;
			buttonRegion += QRect(x-1, top, 2, 1);		// Some pixels
			buttonRegion += QRect(x-1, bottom, 2, 1);	// to look nice.
		}
		buttonRegion += QRect(x, top+1, 1, 1);		// More pixels
		buttonRegion += QRect(x, bottom-1, 1, 1);	// to look nicer.
		if (enabled && mouseOver)
		{
			surface = colorMix(surface, hilite, SRN_OVER);
			bSurface = colorMix(bSurface, hilite, SRN_OVER);
		}
		QPainter p(combo);
		renderSurface(&p, r, ground, 
					surface, hilite,
					Button_Alike);
		p.setClipRegion(buttonRegion, QPainter::CoordPainter);
		renderSurface(&p, r, ground, 
					bSurface, hilite,
					Button_Alike);
		p.setClipping(false);
		contentArea.addCoords(5, 5, -5, -5);
		if (enabled && valid)
			p.fillRect( contentArea, QBrush(innerColor) );
		contentArea.addCoords(-1, -1, 1, 1);
		renderSereneContour(&p, contentArea, surface, surface,
					Button_Alike|Is_Sunken|Keep_Color|Draw_AlphaBlend);
		renderSoftMap(&p, buttonArea, surface, arrowColor, ARROWDOWN);
		if ( (_drawFocusRect) && (combo->hasFocus()) )
		{
			contentArea.addCoords(-1, -1, 1, 1);
			renderContour(&p, contentArea, surface, 
					colorMix(cg.highlight(), cg.foreground(), 160),
					Square_Box|Draw_AlphaBlend);
		}
		return true;
	}
	
	if ( obj->isA("QWhatsThat") )
	{
		if (ev->type() != QEvent::Paint)
			return false;
		
		SereneWhatsThat* swt = (SereneWhatsThat*)obj;
		QString text = swt->text;
		QRect r = swt->rect();
		QColorGroup cg = swt->colorGroup();
		QPainter p(swt);
		QColor surface = getColor(cg, TipSurface);
		p.fillRect( r.left()+1, r.top()+1, r.width()-2, r.height()-2, QBrush(surface) );
		renderContour(&p, r, surface, colorMix(cg.text(), getColor(cg, TipContour)), 
				Button_Alike|Draw_AlphaBlend);
		r.addCoords(10, 10, -10, -10);
		if( QStyleSheet::mightBeRichText(text) )
		{
			QSimpleRichText* srt = new QSimpleRichText(text, swt->font());
			srt->draw(&p, r.left(), r.top(), r, qApp->palette().active(), 0);
		}
		else
		{
			p.drawText(r, AlignAuto + AlignTop + WordBreak + ExpandTabs, text);
		}
		return true;
	}
	
	if ( obj->isA("QTipLabel") || obj->inherits("QToolTip") )
	{
		if (ev->type() == QEvent::Show)
		{
			/// MXLS: Give some space to those tooltips!
			QLabel* label = static_cast<QLabel*>(obj);
			QSimpleRichText* srt = new QSimpleRichText(label->text(), label->font());
			srt->setWidth(label->width());
			label->resize(srt->widthUsed()+8, srt->height()+8);
			return true;
		}
		else if (ev->type() == QEvent::Paint)
		{
			QLabel* label = static_cast<QLabel*>(obj);
			QRect r = label->rect();
			QColorGroup cg = label->colorGroup();
			QPainter p(label);
			QColor surface = getColor(cg, TipSurface);
			p.fillRect( r.left()+1, r.top()+1, r.width()-2, r.height()-2, QBrush(surface) );
			renderContour(&p, r, surface, colorMix(cg.text(), getColor(cg, TipContour)), 
					Button_Alike|Draw_AlphaBlend);
			r.addCoords(4, 4, -4, -4);
			QSimpleRichText* srt = new QSimpleRichText(label->text(), label->font());
			srt->setWidth(r.width());
			srt->draw(&p, r.left(), r.top(), r, qApp->palette().active(), 0);
			return true;
		}
		return false;
	}
	
	if ( obj->isA("KonqFileTip") )
	{
		if(ev->type() == QEvent::Show)
		{
			QFrame* frame = dynamic_cast<QFrame*>(obj);
			QColorGroup cg = frame->colorGroup();
			QColor surface = colorMix(getColor(cg, TipBase), 
						getColor(cg, TipContour), SRN_MIX);
			frame->setFrameStyle(QFrame::NoFrame);
			frame->setBackgroundMode(NoBackground);
			//
			QObjectList* list = const_cast<QObjectList*>(frame->children());
			QObjectListIt it(*list);
			QObject* o;
			while ( (o = it.current()) != 0 ) {
				++it;
				if(dynamic_cast<QLabel*>(o))
				{
					QLabel* l = dynamic_cast<QLabel*>(o);
					l->setPaletteBackgroundColor(surface);
					l->setPaletteForegroundColor(cg.text()); /// MXLS: Doesn't work!
				}
			}
			return true;
		}
		else if (ev->type() == QEvent::Paint)
		{
			QWidget* widget = dynamic_cast<QWidget*>(obj);
			QRect r = widget->rect();
			QColorGroup cg = widget->colorGroup();
			QPainter p(widget);
			QColor surface = colorMix(getColor(cg, TipBase), 
						getColor(cg, TipContour), SRN_MIX);
			p.fillRect( r.left()+1, r.top()+1, r.width()-2, r.height()-2, QBrush(surface) );
			renderContour(&p, r, surface, colorMix(cg.text(), getColor(cg, TipContour)), 
					Button_Alike|Draw_AlphaBlend);
			return true;
		}
		return false;
	}
	
	/// MXLS: Send click on label to buddy.
	/// Don't "optimize" this test or else labels with a frame won't be styled.
	/// I mean: Keep it on one line with the "&&".
	if ( (::qt_cast<QLabel*>(obj)) && (ev->type() == QEvent::MouseButtonPress) )
	{
		QMouseEvent* what = static_cast<QMouseEvent*>(ev);
		if (what->button() != Qt::LeftButton) // Accept only the left button.
			return false;
		QLabel* label = static_cast<QLabel*>(obj);
		if (! label->buddy())		// No buddy.
			return false;
		QWidget* buddy = label->buddy();
		if (! buddy->isWidgetType())	// Safety measure.
			return false;
		buddy->setFocus();		// Set focus for all editable widgets.
		//
		// Don't click on a slider!
		if (! ::qt_cast<QSlider*>(buddy))
		{
			// Fake a mouse click for non-editable widgets.
			QMouseEvent me(QEvent::MouseButtonPress, 
					QPoint(0,0), 
					Qt::LeftButton, Qt::LeftButton);
			QApplication::sendEvent(buddy, &me);
			me = QMouseEvent(QEvent::MouseButtonRelease, 
					QPoint(0,0), 
					Qt::LeftButton, Qt::LeftButton);
			QApplication::sendEvent(buddy, &me);
		}
		//
		return false;
	}

	/// MXLS: Keep this one at the end to avoid any conflict.
	if (::qt_cast<QFrame*>(obj))
	{
		if (ev->type() != QEvent::Paint)
			return false;
		//
		QFrame* f = static_cast<QFrame*>(obj);
		int shape = f->frameShape();
		QColorGroup cg = f->colorGroup();
		if ( (shape == QFrame::HLine) || (shape == QFrame::VLine) )
		{
			QRect work = f->rect();
			int left, top, width, height;
			work.rect(&left, &top, &width, &height);
			int right, bottom;
			work.coords(&left, &top, &right, &bottom);
			QPainter p(f);
			p.setPen( getColor(cg, SeparatorColor) );
			if (shape == QFrame::HLine)
			{
				top += height/2;
				p.drawLine(left, top, right, top);
			}
			else // (shape == QFrame::VLine)
			{
				left += width/2;
				p.drawLine(left, top, left, bottom);
			}
			return true;
		}
		else if (shape == QFrame::GroupBoxPanel)
		{
			if (! dynamic_cast<QGroupBox*>(obj))
				return false;
			
			QGroupBox* gb = dynamic_cast<QGroupBox*>(obj);

			QColor ground = getColor(cg, DefaultGround);
			QRect work = gb->rect();
			QString title = gb->title();
			int left, top, width, height;
			work.rect(&left, &top, &width, &height);
			int right, bottom;
			work.coords(&left, &top, &right, &bottom);
			QPainter p(f);
			QFontMetrics fm = p.fontMetrics();
			int th = fm.height()+2;
			int middle = top+(th/2);
			if (gb->isFlat())
			{
				QColor surface = getColor(cg, SeparatorColor);
				p.setPen(surface);
				p.drawLine(left, middle, right, middle);
				if (title.length())
				{
					int tw = fm.width( title, visibleLen(fm, title, width) ) 
						+ 4*fm.width(QChar(' '));
					left += (width-tw)/2;
					renderFlatArea(&p, QRect(left, top, tw, th), 
							ground, surface, 
							Button_Alike);
					p.setPen(cg.buttonText());
					p.drawText(QRect(left, top, tw, th), 
							AlignHCenter|AlignVCenter|ShowPrefix|SingleLine, 
							title);
				}
			}
			else
			{
				if (title.length())
				{
					if (gb->isCheckable()) th += 3;
					QRect inner(left, top, width, th);
					QColor surface = getColor(cg, MixedSurface, f->isEnabled());
					QColor textColor = cg.foreground();
					p.fillRect( inner, QBrush(surface) );
					renderContour(&p, work, ground, 
							surface, Button_Alike);
					p.setPen(surface);
					p.drawPoint(left+1, top+th);
					p.drawPoint(right-1, top+th);
					p.setPen( colorMix(ground, surface) );
					p.drawLine(left+1, top+th+1, left+2, top+th);
					p.drawLine(right-1, top+th+1, right-2, top+th);
					if (! gb->isCheckable())
					{
						p.setPen(textColor);
						p.drawText(inner, 
							AlignHCenter|AlignVCenter|ShowPrefix|SingleLine, 
							title);
					}
					else
					{
						// Set checkbox and label background.
						// Qt will draw it later.
						((QWidget*)gb->child("qt_groupbox_checkbox", 0, false))
							->setPaletteBackgroundColor(surface);
					}
				}
				else
				{
					// No title, no frame.
					p.setPen(ground);
					p.drawRect(work);
				}
			}
			return true;
		}
		else if ( (shape == QFrame::WinPanel) || (shape == QFrame::Panel) 
		|| (shape == QFrame::Box) || (shape == QFrame::StyledPanel) )
		{
			if (f->frameWidth() < 1)
				return false;	// No frame.
			//
			QColor ground = getColor(cg, DefaultGround);
			QColor contour = getColor(cg, PanelContour);
			//
			QRect work = f->rect();
			int left, top, width, height;
			work.rect(&left, &top, &width, &height);
			int shadow = f->frameShadow();
			QPainter p(f);
			bool redraw = false;
			if (! strcmp(f->name(), "pannerdivider"))
			{
				/// MXLS: Gotcha!
				drawPrimitive(PE_DockWindowResizeHandle,
						&p, work, cg,
						f->hasMouse() ? Style_MouseOver : Style_Default);
				return true;
			}
			else if (::qt_cast<KRuler*>(obj))
			{
				/// MXLS: FIXME: Doesn't work with KOffice.
				contour = cg.background();
				if (contour != ground)
				{
					renderContour(&p, work, ground, 
							contour, Button_Alike);
				}
				else
				{
					p.setPen(contour);
					p.drawRect(work);
				}
				redraw = true;
			}
			else if (shape == QFrame::Box)
			{
				if (shadow == QFrame::Plain)
				{
					// ComboBox popup menu.
					renderContour(&p, work, ground, 
							getColor(cg, PopupContour), 
							Square_Box|Draw_AlphaBlend);
				}
				else
				{
					// Raised and sunken boxes.
					QRect inner = work;
					inner.addCoords(1, 1, -1, -1);
					uint outerFlag = No_Flags;
					uint innerFlag = No_Flags;
					if (shadow == QFrame::Raised)
						innerFlag = Is_Sunken;
					else
						outerFlag = Is_Sunken;
					//
					contour = cg.background();
					if (qGray(contour.rgb()) < 64)
						contour = brighter(contour, ADJUSTOR);
					//
					renderSereneContour(&p, inner, ground, 
							contour, Square_Box|innerFlag|Serene_Drawing);
					renderSereneContour(&p, work, ground, 
							contour, Square_Box|outerFlag|Serene_Drawing);
				}
				redraw = true;
			}
			else if (shadow == QFrame::Raised)
			{
				if (f->frameWidth() > 1)
				{
					if (shape == QFrame::WinPanel)
					{
						// Floating toolbar or sub-window.
						renderContour(&p, work, ground, 
								contour, Square_Box);
					}
					else
					{
						// Something almost invisible.
						renderContour(&p, work, ground, 
								getColor(cg, LightSurface),
								Square_Box);
					}
					redraw = true;
				}
				else if (::qt_cast<KAnimWidget*>(obj))
				{
					renderSereneContour(&p, work, ground, 
								getColor(cg, HoverColor), 
								Square_Box);
					redraw = true;
				}
				else if (shape == QFrame::StyledPanel)
				{
					// Something totally invisible.
					p.setPen(ground);
					p.drawRect(work);
				}
			}
			else if (shadow == QFrame::Sunken)
			{
				if (::qt_cast<QLabel*>(obj))
				{
					// Something visible around a text.
					renderContour(&p, work, ground, 
							getColor(cg, FrameContour),
							Button_Alike);
				}
				else if (kontactMode 
				|| ((f->parent()) && (! (f->parent())->isA("KMultiTabBar"))
				&& (shape != QFrame::StyledPanel)) )
				{
					// Something almost invisible.
					renderContour(&p, work, ground, 
							getColor(cg, LightSurface),
							Square_Box);
				}
				else if (shape == QFrame::StyledPanel)
				{
					// Or totally invisible.
					p.setPen(ground);
					p.drawRect(work);
				}
				redraw = true;
			}
			//
			if (redraw)
			{
				// Don't forget to draw the content!
				QPaintEvent* e = (QPaintEvent*)ev;
				p.setClipRegion(e->region().intersect(f->contentsRect()));
				SereneFrame* sf = (SereneFrame*)f;
				sf->drawContents(&p);
				//
				SereneWidget* sw = (SereneWidget*)f;
				QPaintEvent* event = new QPaintEvent(e->region().intersect(f->contentsRect()));
				sw->paintEvent(event);
				return true;
			}
		}
	}
	//
	return false;
}


QRegion SerenityStyle::maskPattern(const int width, const int height, int shift) const
{
	QRegion result;
	switch (_progressPattern)
	{
		case CandyBar:
		{
			int dir = _reverseLayout ? 1 : -1;
			for (int j = 0 ; j < height ; j+=2)
			{
				for (int i = shift ; i < width+shift ; i+=24)
				{ 
					result += QRegion(i, j, 8, 2);
				}
				shift += dir;
			}
			break;
		}
		case Checker:
		{
			int hh = height/2;
			for (int i = shift ; i < width+shift ; i+=24)
			{ 
				result += QRegion(i, 0, 8, hh);
				result += QRegion(i-8, hh, 8, hh+1);
			}
			break;
		}
		case Waves:
		{
			int hh = height/2;
			int yy = height/4;
			for (int i = shift ; i < width+shift ; i+=24)
			{ 
				result += QRegion(i, yy, 12, hh, QRegion::Ellipse);
				result += QRegion(i, hh, 24, hh+1);
				result -= QRegion(i+12, yy, 12, hh, QRegion::Ellipse);
			}
			break;
		}
		default: // Arrows
		{
			int hh = height/2;
			int dir = _reverseLayout ? -1 : 1;
			for (int j = 0; j < hh ; j++)
			{
				for (int i = shift ; i < width+shift ; i+=24)
				{ 
					result += QRegion(i, j, 8, 1);
				}
				shift += dir;
			}
			for (int j = hh; j < height ; j++)
			{
				for (int i = shift ; i < width+shift ; i+=24)
				{ 
					result += QRegion(i, j, 8, 1);
				}
				shift -= dir;
			}
			break;
		}
	}
	return result;
}

bool SerenityStyle::mouseWithin(const QWidget* hover, const QRect &here) const
{
	if (!hover)
		return false;
	
	QPoint origin( hover->mapToGlobal(QPoint(0, 0)) );
	QRect there(here);
	there.moveTopLeft( QPoint(origin.x()+here.x(), origin.y()+here.y()) );
	if ( there.contains(QCursor::pos()) )
		return true;
	else
		return false;
}

int SerenityStyle::visibleLen(const QFontMetrics fm, const QString str, const int width)
{
	int lenvisible = str.length();
	if (lenvisible)
	{
		int tw;
		while (lenvisible)
		{
			tw = fm.width( str, lenvisible ) + 4*fm.width(QChar(' '));
			if ( tw < width )
				break;
			lenvisible--;
		}
	}
	return lenvisible;
}

// Sanity check while reading config file
int SerenityStyle::limitedTo(int lowest, int highest, int variable)
{
	if ( (variable < lowest) || (variable > highest) )
		return (lowest < 0) ? 0 : lowest;
	else
		return variable;
}

/// MXLS: Copied from "kstyles.cpp". Needed by tree views.
QListViewItem* SerenityStyle::nextVisibleSibling(QListViewItem* item) const
{
	QListViewItem* sibling = item;
	do
	{
		sibling = sibling->nextSibling();
	}
	while (sibling && !sibling->isVisible());
	//
	return sibling;
}

QColor SerenityStyle::colorMix(const QColor &bgColor, const QColor &fgColor, const int alpha) const
{
	// Unnecessary, alpha is always a constant.
	//if (alpha > 255) alpha = 255;
	//if (alpha < 0) alpha = 0;
	int bred, bgrn, bblu, fred, fgrn, fblu;
	bgColor.getRgb(&bred, &bgrn, &bblu);
	fgColor.getRgb(&fred, &fgrn, &fblu);
	return QColor( fred + ((bred-fred)*alpha)/255,
			fgrn + ((bgrn-fgrn)*alpha)/255,
			fblu + ((bblu-fblu)*alpha)/255 );
}

QColor SerenityStyle::getColor(const QColorGroup &cg, const ColorType t, const bool enabled)  const
{
	QPalette pal = QApplication::palette();
	QColor backColor = pal.color(QPalette::Active, QColorGroup::Background);
	QColor foreColor = pal.color(QPalette::Active, QColorGroup::Foreground);
	QColor buttonColor = pal.color(QPalette::Active, QColorGroup::Button);
	QColor buttonText = pal.color(QPalette::Active, QColorGroup::ButtonText);
	QColor baseColor = pal.color(QPalette::Active, QColorGroup::Base);
	QColor textColor = pal.color(QPalette::Active, QColorGroup::Text);

	// These two ones must be returned unchanged.
	if (t == DefaultPen)
	{
		if (enabled)
			return foreColor;
		else
			return pal.color(QPalette::Disabled, QColorGroup::Foreground);
	}
	else if (t == ButtonPen)
	{
		if (enabled)
			return buttonText;
		else
			return pal.color(QPalette::Disabled, QColorGroup::ButtonText);
	}
	// And this one can't be changed
	else if (t == DefaultGround)
	{
		return backColor;
	}
	//
	// Eventually blend with a defined color. 
	if (_customGlobalColor)
	{
		foreColor = colorMix(foreColor, _globalColor);
		buttonText = colorMix(buttonText, _globalColor);
		textColor = colorMix(textColor, _globalColor);
	}
	//
	switch (t)
	{
		case MixedSurface:
		{
			if (enabled)
				return colorMix(backColor, foreColor, SRN_MIX);
			else
				return colorMix(backColor, cg.foreground(), SRN_MIX);
		}
		case LightSurface:
		{
			if (enabled)
				return colorMix(backColor, foreColor, SRN_LIGHT);
			else
				return colorMix(backColor, cg.foreground(), SRN_LIGHT);
		}
		case ButtonSurface:
		{
			if (enabled)
				return buttonColor;
			else
				return cg.button();
		}
		case ComboMix:
		{
			QColor c = cg.button();
			if (c == buttonColor)
			{
				if (enabled)
					return colorMix(buttonColor, buttonText, SRN_MIX);
				else
					return colorMix(c, 
							pal.color(QPalette::Disabled, QColorGroup::ButtonText), 
							SRN_MIX);
			}
			else
			{
				/// On HTML page with fancy colors
				return colorMix(c, cg.buttonText(), SRN_MIX);
			}
		}
		case FlatButtonSurface:
		{
			if (enabled)
				return colorMix(buttonColor, buttonText, SRN_MIX);
			else
				return colorMix(cg.button(), 
						pal.color(QPalette::Disabled, QColorGroup::ButtonText), 
						SRN_MIX);
		}
		case HoverColor:
		{
			if (_customOverHighlightColor)
				return _overHighlightColor;
			else
				return cg.highlight();
		}
		case ScrollerGroove:
		{
			QColor c = cg.button();
			if ( (!enabled) || (c == buttonColor) 
			|| ( c == pal.color(QPalette::Disabled, QColorGroup::Button)) )
			{
				/// MXLS: HACK: PE_(Add|Sub)Page are never 
				/// disabled --contrarly to PE_(Add|Sub)Line. 
				/// Only cg.button() changes so that they *look*
				/// disabled. So, in all cases, we take the 
				/// active colors.
				if (_scrollerScheme == 1)
					return colorMix(backColor, baseColor, SRN_MIX-24);
				else if (_scrollerScheme == 2)
					return colorMix(backColor, buttonColor, SRN_MIX-32);
				else
					return colorMix(backColor, foreColor, SRN_LIGHT);
			}
			else
			{
				/// On HTML page with fancy colors
				return cg.background();
			}
		}
		case ScrollerSurface:
		{
			QColor c = cg.button();
			if (c == buttonColor)
			{
				/// The scroller is either enabled or not shown at all...
				if (_scrollerScheme == 1)
				{
					c = colorMix(baseColor, textColor, SRN_MIX-24);
				}
				else if (_scrollerScheme == 2)
				{
					c = colorMix(buttonColor, buttonText, SRN_LIGHT);
				}
				else
				{
					c = colorMix(colorMix(backColor, foreColor, SRN_LIGHT), 
								foreColor, SRN_MIX);
				}
				/// ...but the slider can be disabled.
				if (!enabled)
					c = colorMix(baseColor, c);
			}
			/// On HTML page with fancy colors
			return c;
		}
		case ScrollerText:
		{
			if (cg.button() == buttonColor)
			{
				if (_scrollerScheme == 1)
					return textColor;
				else if (_scrollerScheme == 2)
					return buttonText;
				else
					return foreColor;
			}
			else
			{
				/// On HTML page with fancy colors
				return cg.buttonText();
			}
		}
		case FrameContour:
		{
			if (enabled)
				return colorMix(colorMix(backColor, foreColor), 
						buttonColor, 112);
			else
				return colorMix(colorMix(backColor, cg.foreground()), 
						cg.button(), 112);
		}
		case PanelContour:
		{
			if (enabled)
				return colorMix(backColor, foreColor, 184);
			else
				return colorMix(backColor, cg.foreground(), 184);
		}
		case Menubar_GradTop:
		{
			return colorMix(backColor, gradientColor(backColor, TOP));
		}
		case Menubar_GradBottom:
		{
			return colorMix(backColor, foreColor, SRN_LIGHT);
		}
		case Menubar_Solid:
		{
			if (_menuGrooveStyle == MuInvSolid)
				return colorMix(backColor, gradientColor(backColor, TOP));
			else
				return colorMix(backColor, foreColor, SRN_LIGHT);
		}
		case Menubar_2lines:
		{
			return colorMix(colorMix(backColor, foreColor), 
					buttonColor, 96);
		}
		case PopupContour:
		{
			if (enabled)
				return colorMix(buttonColor, buttonText, 192);
			else
				return colorMix(buttonColor, cg.buttonText(), 192);
		}
		case TabGroove_GradTop:
		{
			if (enabled)
				return gradientColor(colorMix(backColor, foreColor, SRN_LIGHT), TOP);
			else
				return gradientColor(backColor, TOP);
		}
		case TabGroove_GradBottom:
		{
			if (enabled)
				return colorMix(backColor, foreColor, SRN_MIX);
			else
				return colorMix(backColor, cg.foreground(), SRN_MIX);
		}
		case TabGroove_Solid:
		{
			if (_passiveTabStyle == InvSolidPaTab)
				return colorMix(backColor, gradientColor(backColor, TOP), 96);
			else
				return colorMix(backColor, foreColor, SRN_MIX);
		}
		case HeaderColor:
		{
			return colorMix(backColor, cg.base(), SRN_DOWN);
		}
		case ExpanderColor:
		{
			QColor c =  colorMix(colorMix(backColor, baseColor, SRN_MIX), 
						textColor, 240);
			if (qGray(backColor.rgb()) < 64)
				c = brighter(c, ADJUSTOR);
			return c;
		}
		case TreeBranchColor:
		{
			return colorMix(baseColor, textColor, 160);
		}
		case ModernCollapsed:
		{
			return colorMix(baseColor, textColor, 64);
		}
 		case ModernExpanded:
		{
			return colorMix(baseColor, textColor, 96);
		}
 		case ModernDotColor:
		{
			return colorMix(baseColor, textColor);
		}
		case ProgressGroove:
		{
			return colorMix(backColor, baseColor, 160);
		}
		case ProgressSurface:
		{
			return colorMix(backColor, textColor, 184);
		}
		case SeparatorColor:
		{
			if (enabled)
				return colorMix(colorMix(backColor, foreColor), 
						buttonColor, 96);
			else
				return colorMix(colorMix(backColor, cg.foreground()), 
						cg.button(), 96);
		}
		case TipContour:
		{
			switch (_tipTint)
			{
				case 1: return QColor(32, 160, 255);	// Blue
				case 2: return QColor(32, 255, 32);	// Green
				case 3: return QColor(255, 32, 32);	// Red
				case 4: return QColor(255, 160, 32);	// Orange
				case 5: return QColor(255, 32, 255);	// Violet
				case 6: return QColor(128, 128, 128);	// Grey
				case 7: return QColor(255, 255, 255);	// White
				default: return QColor(255, 255, 32);	// Yellow
			}
		}
		case TipSurface:
		{
			QColor c = colorMix(baseColor, 
						getColor(cg, TipContour), SRN_MIX);
			if (qGray(c.rgb()) < 64)
				c = brighter(c, ADJUSTOR);
			return c;
		}
		case TipBase:
		{
			QColor c = baseColor;
			if (qGray(c.rgb()) < 96)
				return QColor(255, 255, 255);
			else
				return c;
		}
		default:
			return backColor;
	}
}

QColor SerenityStyle::borderColor(const QColor baseColor, const bool top, const bool sunken) const
{
	if (top)
		return sunken ? darker(baseColor, HI_FACTOR) 
				: brighter(baseColor, MED_FACTOR);
	else
		return sunken ? brighter(baseColor, MED_FACTOR)
				: darker(baseColor, HI_FACTOR);
}

QColor SerenityStyle::gradientColor(const QColor baseColor, const bool top, const bool highContrast) const
{
	if (top)
		return brighter(baseColor, highContrast ? HI_FACTOR : LO_FACTOR);
	else
		return darker(baseColor, highContrast ? HI_FACTOR : LO_FACTOR);
}

QColor SerenityStyle::radioBorderColor(const QColor baseColor, const bool top, const bool sunken) const
{
	if (top)
		return sunken ? darker(baseColor, HI_FACTOR) 
				: brighter(baseColor, HI_FACTOR);
	else
		return sunken ? brighter(baseColor, HI_FACTOR)
				: darker(baseColor, HI_FACTOR);
}

QColor SerenityStyle::radioGradientColor(const QColor baseColor, const bool top) const
{
	if (top)
		return brighter(baseColor, MED_FACTOR);
	else
		return darker(baseColor, MED_FACTOR);
}

QColor SerenityStyle::brighter(const QColor baseColor, const int factor) const
{
	/// MXLS: Brighter means brighter, never darker!
	if (factor <= 0)
		return baseColor;
	int hue, sat, val;
	baseColor.getHsv(&hue, &sat, &val);
	if ((val == 0) || (hue == -1))
	{
		val += (255*factor)/100;
		if (val > 255)
			return colorMix(white, _globalColor, 248);
		else
			return QColor(val, val, val);
	}
 	/// Inspired by Qt3's "qcolor.cpp"
	val += (val*factor)/100;
	if (val > 255)				// Overflow
	{
		sat -= val-255;			// Adjust saturation
		if (sat < 0) sat = 0;
		val = 255;
	}
	QColor result;
	result.setHsv(hue, sat, val);
	return result;
}

QColor SerenityStyle::darker(const QColor baseColor, const int factor) const
{
	/// MXLS: Darker means darker, never brighter!
	if (factor <= 0)
		return baseColor;
	/// Inspired by Qt3's "qcolor.cpp"
	int hue, sat, val;
	baseColor.getHsv(&hue, &sat, &val);
	val -= (val*factor)/100;
	if (val < 0)				// Overflow
	{
		val = 0;
	}
	QColor result;
	result.setHsv(hue, sat, val);
	return result;
}

// EOF
