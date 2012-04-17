/* Domino widget style for KDE 3
   Copyright (C) 2006 Michael Lentner <michaell@gmx.net>

   based on the KDE style "Plastik":
   Copyright (C) 2003 Sandro Giessl <ceebx@users.sourceforge.net>

   based on the KDE style "dotNET":
   Copyright (C) 2001-2002, Chris Lee <clee@kde.org>
					Carsten Pfeiffer <pfeiffer@kde.org>
					Karol Szwed <gallium@kde.org>
   Drawing routines completely reimplemented from KDE3 HighColor, which was
   originally based on some stuff from the KDE2 HighColor.

   based on drawing routines of the style "Keramik":
   Copyright (c) 2002 Malte Starostik <malte@kde.org>
			 (c) 2002,2003 Maksim Orlovich <mo002j@mail.rochester.edu>
   based on the KDE3 HighColor Style
   Copyright (C) 2001-2002 Karol Szwed		<gallium@kde.org>
			 (C) 2001-2002 Fredrik Höglund	<fredrik@kde.org>
   Drawing routines adapted from the KDE2 HCStyle,
   Copyright (C) 2000 Daniel M. Duley		<mosfet@kde.org>
			 (C) 2000 Dirk Mueller			<mueller@kde.org>
			 (C) 2001 Martijn Klingens		<klingens@kde.org>
   Progressbar code based on KStyle,
   Copyright (C) 2001-2002 Karol Szwed <gallium@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */


#include <kglobalsettings.h>
#include <qpainter.h>
#include <qtabbar.h>
#include <qprogressbar.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qstyleplugin.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <kpopupmenu.h>
#include <qradiobutton.h>
#include <qslider.h>
#include <qsettings.h>
#include <kpixmap.h>
#include <qgroupbox.h>
#include <qmenudata.h>
#include <qdockarea.h>
#include <qobjectlist.h>
#include <qheader.h>
#include <klistview.h>
#include <qstatusbar.h>
#include <qcombobox.h>
#include <qtabwidget.h>
#include <ktoolbar.h>
#include <qlabel.h>
#include <qsimplerichtext.h>
#include <qiconview.h>
#include <kconfig.h>
#include <qapplication.h>
#include <qtoolbox.h>
#include <qstylesheet.h>
// #include <qsplitter.h>
#include <qwidgetstack.h>
#include <qtimer.h>
#include <kmultitabbar.h>
#include <qspinbox.h>
#include <qcursor.h>
#include <qtextedit.h>
#include <kdeversion.h>
#include <kimageeffect.h>

#include <X11/Xlib.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "domino.h"

#include <X11/Xft/Xft.h>
// #include <X11/extensions/Xrender.h>
#include <X11/extensions/shape.h>



// popupmenu item constants...
static const int itemHMargin = 6;
static const int itemFrame = 7;
static const int arrowHMargin = 6;

ButtonContour::ButtonContour()
{
	
	state = Contour_Default;
	alpha_mode = false;
		
	for(int t = 0; t < num_types; t++)
		for(int s = 0; s < num_states; s++)
			created[t][s] = 0;
	
	for(int s = 0; s < num_states; s++)
		shadowCreated[s] = 0;
		
	ContourType ty;
	ty = Contour_Sunken;
	for(int s = 0; s < num_states; s++) {
		c1[ty][s] = qRgba(31, 31, 31, 32);  // top first shadow line
		c4[ty][s] = qRgba(255, 255, 255, 78); // bottom first shadow line
		c5[ty][s] = qRgba(255, 255, 255, 30); // bottom second shadow line
		c6[ty][s] = qRgba(217, 217, 217, 34); // first left shadow line
	}
	ty = Contour_Raised;
	for(int s = 0; s < num_states; s++) {
		c1[ty][s] = qRgba(0, 0, 0, 14);  // top first shadow line
		c4[ty][s] = qRgba(0, 0, 0, 26); // bottom first shadow line
		c5[ty][s] = qRgba(0, 0, 0, 10); // bottom second shadow line
		c6[ty][s] = qRgba(0, 0, 0, 12); // first left shadow line
	}
	ty = Contour_Simple;
	for(int s = 0; s < num_states; s++) {
		c1[ty][s] = qRgba(0, 0, 0, 0);  // top first shadow line
		c4[ty][s] = qRgba(0, 0, 0, 0); // bottom first shadow line
		c5[ty][s] = qRgba(0, 0, 0, 0); // bottom second shadow line
		c6[ty][s] = qRgba(0, 0, 0, 0); // first left shadow line
	}
	drawButtonSunkenShadow = true;
}


ButtonContour::~ButtonContour()
{
	
	for(int t = 0; t < num_types; t++) {
		for(int s = 0; s < num_states; s++) {
			if(created[t][s]) {
				delete btnEdges[t][s];
				delete btnVLines[t][s];
				delete btnHLines[t][s];
			}
		}
	}
	for(int s = 0; s < num_states; s++) {
		if(shadowCreated[s])
			delete buttonShadowRectangular[s];
	}
}

RubberWidget::RubberWidget(Window window )
	: QWidget()
{
	setWState(WState_Disabled|WState_Polished);
	QWidget::create( window, true, true);
}

RubberWidget::~RubberWidget()
{
}



Rubber::Rubber(uint col)
{

	rubber = 0;
	parent = 0;
	window = 0;
	mask = QRegion();
	Display *dpy = qt_xdisplay();
	int screen = qt_xscreen();
	visual = 0;
	colormap = 0;
	int event_base, error_base;
	
	if ( XRenderQueryExtension( dpy, &event_base, &error_base ) )
	{
		int nvi;
		XVisualInfo templ;
		templ.screen  = screen;
		templ.depth   = 32;
		templ.c_class = TrueColor;
		XVisualInfo *xvi = XGetVisualInfo( dpy, VisualScreenMask | VisualDepthMask
				| VisualClassMask, &templ, &nvi );

		for ( int i = 0; i < nvi; i++ ) {
			XRenderPictFormat *format = XRenderFindVisualFormat( dpy, xvi[i].visual );
			if ( format->type == PictTypeDirect && format->direct.alphaMask ) {
				visual = xvi[i].visual;
				colormap = XCreateColormap( dpy, RootWindow( dpy, screen ), visual, AllocNone );
				break;
			}
		}
	}
	
	wsa.border_pixel = 0;
	wsa.colormap = colormap;
	wsa.background_pixel = 0x00000000;
	wsa.override_redirect = true;
	
	color = col;
}

void Rubber::create(QRect& r, QRegion& m) {
	if(parent)
		return;
	
	mask = m;
	
	wsa.background_pixel = 0x00000000;
	Window parentwin = XCreateWindow(qt_xdisplay(), qApp->desktop()->winId(), r.x(), r.y(), r.width(), r.height(), 0, 32, InputOutput, visual, CWBackPixel|CWColormap|CWBorderPixel|CWOverrideRedirect, &wsa);
	parent = new RubberWidget(parentwin);
	
	wsa.background_pixel = color;
	window = XCreateWindow(qt_xdisplay(), parentwin, 0, 0, r.width(), r.height(), 0, 32, InputOutput, visual, CWBackPixel|CWColormap|CWBorderPixel|CWOverrideRedirect, &wsa);
	rubber = new RubberWidget(window);
	
#ifdef ShapeInput
	QBitmap bm(r.width(), r.height(), true);
	XShapeCombineMask(qt_xdisplay(), rubber->winId(), ShapeInput, 0, 0, bm.handle(), ShapeSet);
#endif
	XShapeCombineRegion( qt_xdisplay(), rubber->winId(), ShapeBounding, 0, 0, mask.handle(), ShapeSet );
	
	rubber->show();
	parent->show();
	
	XFlush(qt_xdisplay());
}

Rubber::~Rubber()
{
}

void Rubber::updateMask(QRegion& m) {
	if(mask != m)
		XShapeCombineRegion( qt_xdisplay(), rubber->winId(), ShapeBounding, 0, 0, m.handle(), ShapeSet );
}


// -- Style Plugin Interface -------------------------
class DominoStylePlugin : public QStylePlugin
{
	public:
		DominoStylePlugin() {}
		~DominoStylePlugin() {}

		QStringList keys() const {
			return QStringList() << "Domino";
		}

		QStyle* create( const QString& key ) {
			if (key.lower() == "domino")
				return new DominoStyle;
			return 0;
		}
};

KDE_Q_EXPORT_PLUGIN( DominoStylePlugin )
// -- end --

	DominoStyle::DominoStyle() : KStyle( KStyle::Default /*AllowMenuTransparency*/, ThreeButtonScrollBar )
{
	konsoleMode = false;
	kickerMode = false;
	konquerorMode = false;
	flatMode = false;
	viewPortPressed = false;
	oldRubberRect = QRect();
	rubber = 0;
	ignoreNextFocusRect = false;
#if KDE_VERSION < 0x30506
	compositeManagerRunning = true;
#endif
	configMode = "0"; // temporaly to avoid cache problems between the configs previewStyle and this style
	spinWidgetDown = false;
	
	popupTimer = new QTimer(this);
	
	QColor background = qApp->palette().active().background();
	QColor buttonColor = qApp->palette().active().button();
	QColor highlight = qApp->palette().active().highlight();
	QSettings settings;
// 	_contrast = settings.readNumEntry("/Qt/KDE/contrast", 0);
	
	settings.beginGroup("/domino/Settings");
	
	//konqTabBarContrast = 150;
	konqTabBarContrast = settings.readNumEntry("/konqTabBarContrast", 15)+100;
	
	_animateProgressBar = settings.readBoolEntry("/animateProgressBar", true);
	_centerTabs = settings.readBoolEntry("/centerTabs", false);
	_drawTriangularExpander = settings.readBoolEntry("/drawTriangularExpander", true);
	_customCheckMarkColor = settings.readBoolEntry("/customCheckMarkColor", false);
	_checkMarkColor.setNamedColor( settings.readEntry("/checkMarkColor", "black") );
	groupBoxSettings.tintBackground = settings.readBoolEntry("/tintGroupBoxBackground", true);
	groupBoxSettings.customColor = settings.readBoolEntry("/customGroupBoxBackgroundColor", false);
	groupBoxSettings.brightness = settings.readNumEntry("/groupBoxBrightness", 10);
	groupBoxSettings.color = settings.readEntry("/groupBoxBackgroundColor", darkenColor(background, 10).name());
	bool customToolTipColor = settings.readBoolEntry("/customToolTipColor", false);
	_toolTipColor = customToolTipColor? settings.readEntry("/toolTipColor", QColor(218,218,179).name()) : QColor(218,218,179);
	//_toolTipColor = QColor(255,255,220); // yellow
	_smoothScrolling = settings.readBoolEntry("/smoothScrolling", true);
	_buttonHeightAdjustment = settings.readNumEntry("/buttonHeightAdjustment", 0);
	_customPopupMenuColor = settings.readBoolEntry("/customPopupMenuColor", false);
	_popupMenuColor = _customPopupMenuColor ? settings.readEntry("/popupMenuColor", background.name()) : background.name();
	_customSelMenuItemColor = settings.readBoolEntry("/customSelMenuItemColor", false);
	_selMenuItemColor = _customSelMenuItemColor ? settings.readEntry("/selMenuItemColor", highlight.name()) : highlight.name();
	_drawPopupMenuGradient = settings.readBoolEntry("/drawPopupMenuGradient", true);
	_indentPopupMenuItems = settings.readBoolEntry("/indentPopupMenuItems", true);
	_toolBtnAsBtn = settings.readBoolEntry("/drawToolButtonAsButton", true);
	_highlightToolBtnIcon = settings.readBoolEntry("/highlightToolBtnIcon", false);
	
	rubberBandType = (RubberType) settings.readNumEntry("/rubberBandType", DistRubber);
	if(rubberBandType == ArgbRubber) {
		int dummy;
		if(XQueryExtension(qt_xdisplay(), "Composite", &dummy, &dummy, &dummy)) {
			QColor c = settings.readEntry("/rubberBandColor", highlight.name());
			int a = settings.readNumEntry("/rubberBandOpacity", 30)*255/100;
			_argbRubberBandColor = qRgba(c.red(), c.green(), c.blue(), a);
			
			rubber = new Rubber(preMultiplyColor(_argbRubberBandColor));
		}
		else
			rubberBandType = DistRubber;
	}
	
	
	focusIndicatorSettings.indicateFocus = settings.readBoolEntry("/indicateFocus", true);
	if(focusIndicatorSettings.indicateFocus) {
		focusIndicatorSettings.color = settings.readEntry("/indicatorColor", highlight.name());
		focusIndicatorSettings.opacity = settings.readNumEntry("/indicatorColorOpacity", 60)*255/100;
		focusIndicatorSettings.buttonColor = settings.readEntry("/indicatorButtonColor", highlight.name());
		focusIndicatorSettings.buttonOpacity = settings.readNumEntry("/indicatorColorButtonOpacity", 60)*255/100;
	}
	focusIndicatorSettings.drawUnderline = settings.readBoolEntry("/drawFocusUnderline", true);
	
	buttonContour = new ButtonContour();
	buttonContour->setDefaultType((ContourType)settings.readNumEntry("/buttonContourType", Contour_Raised), false); // we need to know the ContourType to adjust the contour colors
	buttonContour->setColor(Contour_Default, settings.readEntry("/buttonContourColor", background.dark(250).name()));
	buttonContour->setColor(Contour_DefaultButton, settings.readEntry("/buttonDefaultContourColor", highlight.name()));
	buttonContour->setColor(Contour_MouseOver, settings.readEntry("/buttonMouseOverContourColor", background.dark(250).name()));
	buttonContour->setColor(Contour_Pressed, settings.readEntry("/buttonPressedContourColor", background.dark(250).name()));
	buttonContour->drawButtonSunkenShadow = settings.readBoolEntry("/drawButtonSunkenShadow", true);
	buttonContour->setDefaultType(buttonContour->defaultType); // create this time the tinted pixmaps
	
	if(buttonContour->defaultType == Contour_Raised)
		buttonContour->drawButtonSunkenShadow = false;
	
	_customSliderHandleContourColor = settings.readBoolEntry("/customSliderHandleContourColor", false);
	
	bool drawTextEffect = settings.readBoolEntry("/drawTextEffect", true);
	if(drawTextEffect) {
		
		textEffectSettings.mode = settings.readNumEntry("/textEffectMode", 0) ? TextEffect_Everywhere : TextEffect_OnlyButton;
		textEffectSettings.color = settings.readEntry("/textEffectColor", Qt::white.name());
		textEffectSettings.opacity = settings.readNumEntry("/textEffectOpacity", 60)*255/100;
		textEffectSettings.buttonColor = settings.readEntry("/textEffectButtonColor", Qt::white.name());
		textEffectSettings.buttonOpacity = settings.readNumEntry("/textEffectButtonOpacity", 60)*255/100;
		int textEffectPos = settings.readNumEntry("/textEffectPos", 5);
		switch(textEffectPos) {
			case 0:
				textEffectSettings.pos = QPoint(-1,-1);
				break;
			case 1:
				textEffectSettings.pos = QPoint(0,-1);
				break;
			case 2:
				textEffectSettings.pos = QPoint(1,-1);
				break;
			case 3:
				textEffectSettings.pos = QPoint(1,0);
				break;
			case 4:
				textEffectSettings.pos = QPoint(1,1);
				break;
			case 5:
				textEffectSettings.pos = QPoint(0,1);
				break;
			case 6:
				textEffectSettings.pos = QPoint(-1,1);
				break;
			case 7:
				textEffectSettings.pos = QPoint(-1,0);
				break;
			default:
				textEffectSettings.pos = QPoint(0,1);
		}
		
		int textEffectButtonPos = settings.readNumEntry("/textEffectButtonPos", 5);
		switch(textEffectButtonPos) {
			case 0:
				textEffectSettings.buttonPos = QPoint(-1,-1);
				break;
			case 1:
				textEffectSettings.buttonPos = QPoint(0,-1);
				break;
			case 2:
				textEffectSettings.buttonPos = QPoint(1,-1);
				break;
			case 3:
				textEffectSettings.buttonPos = QPoint(1,0);
				break;
			case 4:
				textEffectSettings.buttonPos = QPoint(1,1);
				break;
			case 5:
				textEffectSettings.buttonPos = QPoint(0,1);
				break;
			case 6:
				textEffectSettings.buttonPos = QPoint(-1,1);
				break;
			case 7:
				textEffectSettings.buttonPos = QPoint(-1,0);
				break;
			default:
				textEffectSettings.buttonPos = QPoint(0,1);
		}
	}
	else
		textEffectSettings.mode = TextEffect_None;
	
	btnSurface.numGradients = settings.readNumEntry("/btnSurface_numGradients", 2);
	btnSurface.g1Color1 = settings.readEntry("/btnSurface_g1Color1", buttonColor.name());
	btnSurface.g1Color2 = settings.readEntry("/btnSurface_g1Color2", buttonColor.dark(120).name());
	btnSurface.g2Color1 = settings.readEntry("/btnSurface_g2Color1", buttonColor.dark(120).name());
	btnSurface.g2Color2 = settings.readEntry("/btnSurface_g2Color2", buttonColor.dark(110).name());
	btnSurface.background = settings.readEntry("/btnSurface_background", buttonColor.name());
	btnSurface.g1Top = settings.readNumEntry("/btnSurface_g1Top", 0);
	btnSurface.g1Bottom = settings.readNumEntry("/btnSurface_g1Bottom", 50);
	btnSurface.g2Top = settings.readNumEntry("/btnSurface_g2Top", 50);
	btnSurface.g2Bottom = settings.readNumEntry("/btnSurface_g2Bottom", 100);
	
	activeTabTopSurface.numGradients = settings.readNumEntry("/activeTabTopSurface_numGradients", 2);
	activeTabTopSurface.g1Color1 = settings.readEntry("/activeTabTopSurface_g1Color1", background.light(110).name());
	activeTabTopSurface.g1Color2 = settings.readEntry("/activeTabTopSurface_g1Color2", background.name());
	activeTabTopSurface.g2Color1 = settings.readEntry("/activeTabTopSurface_g2Color1", background.name());
	activeTabTopSurface.g2Color2 = settings.readEntry("/activeTabTopSurface_g2Color2", background.name());
	activeTabTopSurface.background = settings.readEntry("/activeTabTopSurface_background", background.name());
	activeTabTopSurface.g1Top = settings.readNumEntry("/activeTabTopSurface_g1Top", 0);
	activeTabTopSurface.g1Bottom = settings.readNumEntry("/activeTabTopSurface_g1Bottom", 50);
	activeTabTopSurface.g2Top = settings.readNumEntry("/activeTabTopSurface_g2Top", 50);
	activeTabTopSurface.g2Bottom = settings.readNumEntry("/activeTabTopSurface_g2Bottom", 100);
	
	tabTopSurface.numGradients = settings.readNumEntry("/tabTopSurface_numGradients", 2);
	tabTopSurface.g1Color1 = settings.readEntry("/tabTopSurface_g1Color1", background.light(110).name());
	tabTopSurface.g1Color2 = settings.readEntry("/tabTopSurface_g1Color2", background.dark(110).name());
	tabTopSurface.g2Color1 = settings.readEntry("/tabTopSurface_g2Color1", background.dark(110).name());
	tabTopSurface.g2Color2 = settings.readEntry("/tabTopSurface_g2Color2", background.dark(110).name());
	tabTopSurface.background = settings.readEntry("/tabTopSurface_background", background.dark(110).name());
	tabTopSurface.g1Top = settings.readNumEntry("/tabTopSurface_g1Top", 0);
	tabTopSurface.g1Bottom = settings.readNumEntry("/tabTopSurface_g1Bottom", 50);
	tabTopSurface.g2Top = settings.readNumEntry("/tabTopSurface_g2Top", 50);
	tabTopSurface.g2Bottom = settings.readNumEntry("/tabTopSurface_g2Bottom", 100);
	
	activeTabBottomSurface.numGradients = settings.readNumEntry("/activeTabBottomSurface_numGradients", 2);
	activeTabBottomSurface.g1Color1 = settings.readEntry("/activeTabBottomSurface_g1Color1", background.name());
	activeTabBottomSurface.g1Color2 = settings.readEntry("/activeTabBottomSurface_g1Color2", background.name());
	activeTabBottomSurface.g2Color1 = settings.readEntry("/activeTabBottomSurface_g2Color1", background.name());
	activeTabBottomSurface.g2Color2 = settings.readEntry("/activeTabBottomSurface_g2Color2", background.dark(120).name());
	activeTabBottomSurface.background = settings.readEntry("/activeTabBottomSurface_background", background.name());
	activeTabBottomSurface.g1Top = settings.readNumEntry("/activeTabBottomSurface_g1Top", 0);
	activeTabBottomSurface.g1Bottom = settings.readNumEntry("/activeTabBottomSurface_g1Bottom", 50);
	activeTabBottomSurface.g2Top = settings.readNumEntry("/activeTabBottomSurface_g2Top", 50);
	activeTabBottomSurface.g2Bottom = settings.readNumEntry("/activeTabBottomSurface_g2Bottom", 100);
	
	tabBottomSurface.numGradients = settings.readNumEntry("/tabBottomSurface_numGradients", 2);
	tabBottomSurface.g1Color1 = settings.readEntry("/tabBottomSurface_g1Color1", background.dark(110).name());
	tabBottomSurface.g1Color2 = settings.readEntry("/tabBottomSurface_g1Color2", background.dark(110).name());
	tabBottomSurface.g2Color1 = settings.readEntry("/tabBottomSurface_g2Color1", background.dark(110).name());
	tabBottomSurface.g2Color2 = settings.readEntry("/tabBottomSurface_g2Color2", background.dark(120).name());
	tabBottomSurface.background = settings.readEntry("/tabBottomSurface_background", background.dark(110).name());
	tabBottomSurface.g1Top = settings.readNumEntry("/tabBottomSurface_g1Top", 0);
	tabBottomSurface.g1Bottom = settings.readNumEntry("/tabBottomSurface_g1Bottom", 50);
	tabBottomSurface.g2Top = settings.readNumEntry("/tabBottomSurface_g2Top", 50);
	tabBottomSurface.g2Bottom = settings.readNumEntry("/tabBottomSurface_g2Bottom", 100);
	
	scrollBarSurface.numGradients = settings.readNumEntry("/scrollBarSurface_numGradients", 2);
	scrollBarSurface.g1Color1 = settings.readEntry("/scrollBarSurface_g1Color1", buttonColor.name());
	scrollBarSurface.g1Color2 = settings.readEntry("/scrollBarSurface_g1Color2", buttonColor.dark(120).name());
	scrollBarSurface.g2Color1 = settings.readEntry("/scrollBarSurface_g2Color1", buttonColor.dark(120).name());
	scrollBarSurface.g2Color2 = settings.readEntry("/scrollBarSurface_g2Color2", buttonColor.dark(110).name());
	scrollBarSurface.background = settings.readEntry("/scrollBarSurface_background", buttonColor.name());
	scrollBarSurface.g1Top = settings.readNumEntry("/scrollBarSurface_g1Top", 0);
	scrollBarSurface.g1Bottom = settings.readNumEntry("/scrollBarSurface_g1Bottom", 50);
	scrollBarSurface.g2Top = settings.readNumEntry("/scrollBarSurface_g2Top", 50);
	scrollBarSurface.g2Bottom = settings.readNumEntry("/scrollBarSurface_g2Bottom", 100);
	
	scrollBarGrooveSurface.numGradients = settings.readNumEntry("/scrollBarGrooveSurface_numGradients", 0);
	scrollBarGrooveSurface.g1Color1 = settings.readEntry("/scrollBarGrooveSurface_g1Color1", background.name());
	scrollBarGrooveSurface.g1Color2 = settings.readEntry("/scrollBarGrooveSurface_g1Color2", background.dark(120).name());
	scrollBarGrooveSurface.g2Color1 = settings.readEntry("/scrollBarGrooveSurface_g2Color1", background.dark(120).name());
	scrollBarGrooveSurface.g2Color2 = settings.readEntry("/scrollBarGrooveSurface_g2Color2", background.dark(110).name());
	scrollBarGrooveSurface.background = settings.readEntry("/scrollBarGrooveSurface_background", background.dark(150).name());
	scrollBarGrooveSurface.g1Top = settings.readNumEntry("/scrollBarGrooveSurface_g1Top", 0);
	scrollBarGrooveSurface.g1Bottom = settings.readNumEntry("/scrollBarGrooveSurface_g1Bottom", 50);
	scrollBarGrooveSurface.g2Top = settings.readNumEntry("/scrollBarGrooveSurface_g2Top", 50);
	scrollBarGrooveSurface.g2Bottom = settings.readNumEntry("/scrollBarGrooveSurface_g2Bottom", 100);

	headerSurface.numGradients = settings.readNumEntry("/headerSurface_numGradients", 2);
	headerSurface.g1Color1 = settings.readEntry("/headerSurface_g1Color1", background.name());
	headerSurface.g1Color2 = settings.readEntry("/headerSurface_g1Color2", background.dark(120).name());
	headerSurface.g2Color1 = settings.readEntry("/headerSurface_g2Color1", background.dark(120).name());
	headerSurface.g2Color2 = settings.readEntry("/headerSurface_g2Color2", background.dark(110).name());
	headerSurface.background = settings.readEntry("/headerSurface_background", background.name());
	headerSurface.g1Top = settings.readNumEntry("/headerSurface_g1Top", 0);
	headerSurface.g1Bottom = settings.readNumEntry("/headerSurface_g1Bottom", 50);
	headerSurface.g2Top = settings.readNumEntry("/headerSurface_g2Top", 50);
	headerSurface.g2Bottom = settings.readNumEntry("/headerSurface_g2Bottom", 100);

	// checkBoxes + radioButtons
	checkItemSurface.numGradients = settings.readNumEntry("/checkItemSurface_numGradients", 2);
	checkItemSurface.g1Color1 = settings.readEntry("/checkItemSurface_g1Color1", buttonColor.name());
	checkItemSurface.g1Color2 = settings.readEntry("/checkItemSurface_g1Color2", buttonColor.dark(120).name());
	checkItemSurface.g2Color1 = settings.readEntry("/checkItemSurface_g2Color1", buttonColor.dark(120).name());
	checkItemSurface.g2Color2 = settings.readEntry("/checkItemSurface_g2Color2", buttonColor.dark(110).name());
	checkItemSurface.background = settings.readEntry("/checkItemSurface_background", buttonColor.name());
	checkItemSurface.g1Top = settings.readNumEntry("/checkItemSurface_g1Top", 0);
	checkItemSurface.g1Bottom = settings.readNumEntry("/checkItemSurface_g1Bottom", 50);
	checkItemSurface.g2Top = settings.readNumEntry("/checkItemSurface_g2Top", 50);
	checkItemSurface.g2Bottom = settings.readNumEntry("/checkItemSurface_g2Bottom", 100);

	settings.endGroup();
	
	if(!strcmp(qApp->argv()[0], "konqueror")) {
		KGlobal::config()->setGroup("General");
		minTabLength = KGlobal::config()->readNumEntry("MinimumTabLength", 3);
	}
	else
		minTabLength = 0;
	
	if(!strcmp(qApp->className(), "KApplication")) {
		QString oldgroup = KGlobal::config()->group();
		KGlobal::config()->setGroup("KDE");
		macStyleBar = KGlobal::config()->readBoolEntry("macStyle", false);
		KGlobal::config()->setGroup(oldgroup); // reset the group, do not interfere with the application
		KApplicationMode = true;
	}
	else {
		macStyleBar = false;
		KApplicationMode = false;
	}
	
	
	hoverWidget = 0;
	horizontalLine = 0;
	verticalLine = 0;
	sideRepaint = 0;
	hoveredToolBoxTab = 0;

	checkMark = createCheckMark(_customCheckMarkColor? _checkMarkColor : qApp->palette().active().foreground());
	radioIndicator = createRadioIndicator(_customCheckMarkColor? _checkMarkColor : qApp->palette().active().foreground());
	
	popupFrame = new QPixmap(tintImage(qembed_findImage("popup5"), _popupMenuColor.dark(150) ));
	
	// setup pixmap cache...
	pixmapCache = new QIntCache<CacheEntry>(150000, 499);
	pixmapCache->setAutoDelete(true);

	if ( _animateProgressBar )
	{
		animationTimer = new QTimer( this );
		connect( animationTimer, SIGNAL(timeout()), this, SLOT(updateProgressPos()) );
	}

	scrollDistance = 0;
	scrollWidget = 0;
	scrollTimer = new QTimer(this);
	connect( scrollTimer, SIGNAL(timeout()), this, SLOT(updateScrollPos()) );
	
	QColor tabContour2 = qApp->palette().active().background().dark(150);
	border1= new QPixmap(tintImage(qembed_findImage("border1"), tabContour2));
}

void DominoStyle::updateProgressPos()
{
	
	QProgressBar* pb;
	//Update the registered progressbars.
	QMap<QWidget*, int>::iterator iter;
	bool visible = false;
	for (iter = progAnimWidgets.begin(); iter != progAnimWidgets.end(); iter++)
	{	
		if ( !dynamic_cast<QProgressBar*>(iter.key()) )
			continue;
		
		pb = dynamic_cast<QProgressBar*>(iter.key());
		if ( iter.key() -> isEnabled() && 
				   pb->progress() != pb->totalSteps() && pb->progress())
		{
			// update animation Offset of the current Widget
			iter.data() = (iter.data() + 1) % 20;
			iter.key()->update();
		}
		if (iter.key()->isVisible())
			visible = true;
	}
	if (!visible)
		animationTimer->stop();
}

void DominoStyle::updateScrollPos()
{
	if(dynamic_cast<QScrollView*>(scrollWidget) && scrollDistance != 0) {
		int scroll;
		if(scrollDistance < 0) {
			if(scrollDistance < -200)
				scroll = +45;
			else if(scrollDistance < -100)
				scroll = +30;
			else
				scroll = +15;
			
			scrollDistance += scroll;
			scrollDistance = QMIN(scrollDistance, 0);
		}
		else {
			if(scrollDistance > 200)
				scroll = -45;
			else if(scrollDistance > 100)
				scroll = -30;
			else
				scroll = -15;
			
			scrollDistance += scroll;
			scrollDistance = QMAX(scrollDistance, 0);
		}
		
		if(scrollVertical)
			scrollWidget->scrollBy(0, scroll);
		else
			scrollWidget->scrollBy(scroll, 0);
	}
	else if(scrollTimer->isActive()) {
		scrollTimer->stop();
	}
}


DominoStyle::~DominoStyle()
{
	delete pixmapCache;
	delete horizontalLine;
	delete verticalLine;
	
	delete checkMark;
	delete radioIndicator;
	delete border1;
	delete popupFrame;
	
	delete buttonContour;
}

void DominoStyle::polish(QApplication* app)
{
	if(!qstrcmp(app->name(), "konsole"))
		konsoleMode = true;
	else if(!qstrcmp(app->name(), "konqueror"))
		konquerorMode = true;
	else if(!qstrcmp(app->name(), "kicker"))
		kickerMode = true;
	
	QPixmapCache::clear();
}

void DominoStyle::polish(QPalette &p) {
	
	p.setBrush(QColorGroup::Button, p.active().background());

	if(!_customPopupMenuColor)
		_popupMenuColor = qApp->palette().active().background();
	if(!_customSelMenuItemColor)
		_selMenuItemColor = qApp->palette().active().highlight();
	
	QColor tabContour2 = qApp->palette().active().background().dark(150);
	delete border1;
	border1= new QPixmap(tintImage(qembed_findImage("border1"), tabContour2));
	delete popupFrame;
	popupFrame = new QPixmap(tintImage(qembed_findImage("popup5"), _popupMenuColor.dark(150)));
	if(!_customCheckMarkColor) {
		delete checkMark;
		checkMark = createCheckMark(qApp->palette().active().foreground());
		delete radioIndicator;
		radioIndicator = createRadioIndicator(qApp->palette().active().foreground());
	}
}

void DominoStyle::polish(QWidget* widget)
{
	
	if(!strcmp(widget->name(), "__khtml")) { // is it a khtml widget...?
		khtmlWidgets[widget] = true;
		connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(khtmlWidgetDestroyed(QObject*)));
	}
	else if((!strcmp(widget->name(), "kfile button") || (widget->isA("KLineEdit") && widget->parentWidget())) && !strcmp(widget->parentWidget()->name(), "__khtml")) {
		widget->setBackgroundMode(Qt::NoBackground);
		widget->parentWidget()->setBackgroundMode(Qt::NoBackground);
		khtmlWidgets[widget] = true;
		connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(khtmlWidgetDestroyed(QObject*)));
	}
	
	if(dynamic_cast<QPushButton*>(widget)) {
		
		setDominoButtonPalette(widget, Palette_Widget);
		
		if (widget->isA("KDockButton_Private"))
			widget->setFixedSize(13, 13); // 12x12 pixmap + 1px top and right spacing
		else {
			((DominoQWidget*)widget)->setWFlags(Qt::WNoAutoErase);
			if(widget->parentWidget() && widget->parentWidget()->paletteBackgroundPixmap() && !khtmlWidgets.contains(widget)) {
				widget->setBackgroundMode(Qt::PaletteButton);
				widget->setBackgroundPixmap(*widget->parentWidget()->paletteBackgroundPixmap());
				widget->setBackgroundOrigin(setPixmapOrigin(widget->parentWidget()->backgroundOrigin()));
			}
		}
		
		widget->installEventFilter(this);
	}
	else if(dynamic_cast<QCheckBox*>(widget)) {
		((DominoQWidget*)widget)->setWFlags(Qt::WNoAutoErase);
		widget->installEventFilter(this);
	}
	else if(dynamic_cast<KToolBarButton*>(widget)) {
		KToolBarButton* tb = static_cast<KToolBarButton*>(widget);
		//setDominoButtonPalette(widget); // ### changing the palette breaks them!
		
		if(tb->popupDelay() < 1)
			tb->setPopupDelay(5); // now we have time to draw a pressed button
		widget->installEventFilter(this);
	}
	else if(dynamic_cast<QToolButton*>(widget)) {
		QToolButton* tb = static_cast<QToolButton*>(widget);
		tb->setBackgroundMode(Qt::PaletteBackground);
		tb->setPaletteForegroundColor(tb->palette().active().buttonText());
		setDominoButtonPalette(widget, Palette_Widget);
		
		if(tb->popupDelay()  < 1) {
			disconnect(tb, SIGNAL(pressed()), tb, SLOT(popupPressed()));
			connect(tb, SIGNAL(pressed()), this, SLOT(toolPopupPressed()));
			connect(tb, SIGNAL(released()), this, SLOT(toolPopupReleased()));
		}
		
		if(tb->popupDelay() < 1)
			tb->setPopupDelay(5);
		
		if(tb->parentWidget() && tb->parentWidget()->paletteBackgroundPixmap()) {
			tb->setPaletteBackgroundPixmap(*tb->parentWidget()->paletteBackgroundPixmap());
			tb->setBackgroundOrigin(setPixmapOrigin(tb->parentWidget()->backgroundOrigin()));
		}
		
// 		if(widget->isA("TaskContainer"))
// 			((DominoQWidget*)widget)->setWFlags(Qt::WNoAutoErase);
// 		else
			widget->installEventFilter(this);
			((DominoQWidget*)widget)->setWFlags(Qt::WNoAutoErase);
	}
	else if(dynamic_cast<QRadioButton*>(widget)) {
		((DominoQWidget*)widget)->setWFlags(Qt::WNoAutoErase);
		widget->installEventFilter(this);
	}
	else if(dynamic_cast<QComboBox*>(widget)) {
		QComboBox* cb = static_cast<QComboBox*>(widget);
		
		((DominoQWidget*)widget)->setWFlags(Qt::WNoAutoErase);
		if(widget->parentWidget() && widget->parentWidget()->paletteBackgroundPixmap() && !khtmlWidgets.contains(widget)) {
			widget->setBackgroundMode(Qt::PaletteButton);
			widget->setBackgroundOrigin(setPixmapOrigin(widget->parentWidget()->backgroundOrigin()));
		}
		
		setDominoButtonPalette(widget, Palette_Widget);
		if(cb->listBox())
			cb->listBox()->setFrameStyle(QFrame::Panel|QFrame::Sunken);
		widget->installEventFilter(this);
	}
	else if(dynamic_cast<QGroupBox*>(widget)) {
		QGroupBox* gb = dynamic_cast<QGroupBox*>(widget);
		gb->setAlignment(Qt::AlignHCenter);
		
		if(!gb->isFlat() && !gb->lineWidth() <= 0) {
			if(gb->layout() && gb->layout()->margin() < 7)
				gb->layout()->setMargin(7);
			bool groupBoxFrame = gb->frameShape() == QFrame::GroupBoxPanel;
			if(groupBoxFrame)
				((DominoQWidget*)widget)->setWFlags(Qt::WNoAutoErase);
			
			if(groupBoxSettings.tintBackground && groupBoxFrame) {
				QColor color = getGroupBoxColor(gb);
				QPalette pal(widget->palette());
				int h, s, v;
				pal.active().foreground().hsv(&h, &s, &v);
				pal.setColor(QPalette::Active, QColorGroup::Background, color);
				pal.setColor(QPalette::Inactive, QColorGroup::Background, color);
				pal.setColor(QPalette::Disabled, QColorGroup::Background, color);
				pal.setColor(QPalette::Disabled, QColorGroup::Text, alphaBlendColors(pal.active().foreground(), color, 40));
				gb->setPalette(pal);
				
				if(gb->isCheckable()) {
					((QCheckBox*)gb->child("qt_groupbox_checkbox", 0, false))->setPaletteBackgroundColor(gb->parentWidget()->paletteBackgroundColor());
				}
			}
			connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(groupBoxDestroyed(QObject*)));
		}
		widget->installEventFilter(this);
	}
	else if(dynamic_cast<QSpinWidget*>(widget)) {
		 // Don't like this widget... too much private stuff and the spinBox/spinWidget mess.
		// We check here if the spinBox has a backgroundPixmap. If it's a spinWidget without a spinBox, well, maybe we should fill the widget black then >:)
		if(widget->parentWidget() && widget->parentWidget()->parentWidget() && widget->parentWidget()->parentWidget()->paletteBackgroundPixmap()) {
			QWidget::BackgroundOrigin origin = widget->parentWidget()->backgroundOrigin();
			if(origin == QWidget::WidgetOrigin) {
				widget->parentWidget()->setBackgroundOrigin(QWidget::ParentOrigin);
				widget->setBackgroundOrigin(QWidget::AncestorOrigin);
			}
			else
				widget->setBackgroundOrigin(setPixmapOrigin(origin));
		}
		((DominoQWidget*)widget)->setWFlags(Qt::WNoAutoErase);
		setDominoButtonPalette(widget, Palette_Widget);
		widget->installEventFilter(this);
	}
	else if(dynamic_cast<QSpinBox*>(widget)) {
		connect(dynamic_cast<QSpinBox*>(widget), SIGNAL(valueChanged(int)), this, SLOT(spinBoxValueChanged(int)));
	}
	else if(dynamic_cast<QLineEdit*>(widget)) {
		QLineEdit* le = dynamic_cast<QLineEdit*>(widget);
		QRect r = le->rect();
		le->setFrameRect(QRect(r.x()+1, r.y(), r.width()-1, r.height()));
		widget->installEventFilter(this);
	}
	else if(dynamic_cast<QScrollBar*>(widget)) {
		widget->installEventFilter(this);
	}
	else if(dynamic_cast<QStatusBar*>(widget)) {
		widget->installEventFilter(this);
	}
	else if(dynamic_cast<QHeader*>(widget)) {
		setDominoButtonPalette(widget, Palette_Widget);
		widget->installEventFilter(this);
	}
	else if(dynamic_cast<QPopupMenu*>(widget)) {
		
		if(widget->isA("PanelKMenu"))
			widget->setPaletteForegroundColor(alphaBlendColors(_popupMenuColor, qApp->palette().active().foreground(), 150));
		
		widget->setBackgroundMode( NoBackground );
		widget->installEventFilter(this);
	}
	else if(dynamic_cast<QTabWidget*>(widget)) {
		widget->installEventFilter(this);
	}
	else if(dynamic_cast<QTabBar*>(widget)) {
		QTabBar* tb = dynamic_cast<QTabBar*>(widget);
		if(widget->parentWidget() && widget->parentWidget()->isA("KonqFrameTabs")) {
			tb->setShape(QTabBar::RoundedBelow);
			widget->setEraseColor(widget->paletteBackgroundColor().dark(konqTabBarContrast));
		}
		connect(tb, SIGNAL(layoutChanged()), this, SLOT(updateTabWidgetFrame()));
		connect(tb, SIGNAL(selected(int)), this, SLOT(updateTabWidgetFrame()));
		setDominoButtonPalette(widget, Palette_Widget);
		widget->installEventFilter(this);
	}
	else if(dynamic_cast<QIconView*>(widget)) {
		((DominoQWidget*)dynamic_cast<QIconView*>(widget)->viewport())->setWFlags(Qt::WNoAutoErase);
		((DominoQWidget*)widget)->setWFlags(Qt::WNoAutoErase);
		dynamic_cast<QIconView*>(widget)->viewport()->installEventFilter(this); // rubberband
		if(strcmp(widget->className(), "KDIconView"))
			widget->installEventFilter(this);
	}
	else if(dynamic_cast<QListView*>(widget)) {
		dynamic_cast<QListView*>(widget)->viewport()->installEventFilter(this); // rubberband
		if(widget->isA("KonqTreeViewWidget")) { // konqi frames
			QFrame* f = dynamic_cast<QFrame*>(widget);
			f->setFrameStyle(QFrame::Box|QFrame::Plain);
			f->setLineWidth(1);
		}
		
		if(!widget->isA("KopeteContactListView") && !widget->isA("ViewTree")) // they have their own mousewheel handling
			widget->installEventFilter(this);
	}
	else if(widget->isA("KActiveLabel")) { // before QScrollView
		if(dynamic_cast<QGroupBox*>(widget->parentWidget()))
			widget->setPaletteBackgroundColor(widget->parentWidget()->paletteBackgroundColor());
	}
	else if(dynamic_cast<QTextEdit*>(widget)) {
		dynamic_cast<QTextEdit*>(widget)->viewport()->installEventFilter(this);
		widget->installEventFilter(this);
	}
	else if(dynamic_cast<QScrollView*>(widget)) {
		QFrame* f = dynamic_cast<QFrame*>(widget);
		if((konquerorMode && widget->isA("KHTMLView")) || f->parentWidget() && !strcmp(f->parentWidget()->name(), "KonqFrame")) { // konqi frames
			f->setFrameStyle(QFrame::Box|QFrame::Plain);
			f->setLineWidth(1);
			widget->installEventFilter(this);
		}
		else if(_smoothScrolling && !widget->isA("TaskBar") && !widget->isA("Gwenview::ImageView"))
			widget->installEventFilter(this);
	}
	else if(dynamic_cast<QSlider*>(widget)) {
		((DominoQWidget*)widget)->setWFlags(Qt::WNoAutoErase);
	}
	else if(dynamic_cast<QListBox*>(widget)) {
		widget->installEventFilter(this);
	}
	else if(dynamic_cast<KToolBarSeparator*>(widget) || dynamic_cast<QToolBar*>(widget)) {
		widget->setBackgroundMode(Qt::PaletteBackground);
	}

	else if(widget->isA("KonqCheckBox")) {
		widget->installEventFilter(this);
	}
	else if(widget->isA("QTipLabel")) {
		widget->installEventFilter(this);
	}
	else if(widget->isA("QWhatsThat")) {
		widget->installEventFilter(this);
	}
	else if(widget->isA("KonqFileTip")) {
		widget->installEventFilter(this);
	}

	else if(dynamic_cast<QButton*>(widget) && widget->parentWidget() && widget->parentWidget()->isA("QToolBox")) {
		widget->installEventFilter(this);
	}
	else if( dynamic_cast<QProgressBar*>(widget) && _animateProgressBar)
	{
		setDominoButtonPalette(widget, Palette_Widget);
		
		progAnimWidgets[widget] = 0;
		connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(progressBarDestroyed(QObject*)));
// 		if (!animationTimer->isActive())
// 			animationTimer->start( 50, false );
		widget->installEventFilter(this);
	}
	else if(dynamic_cast<KPopupTitle*>(widget)) {
		widget->installEventFilter(this);
	}
	else if(dynamic_cast<QFrame*>(widget)) {
		QFrame* f = dynamic_cast<QFrame*>(widget);
		
		switch (f->frameShape()) {
			case QFrame::Box:
				f->setFrameStyle(QFrame::StyledPanel|QFrame::Sunken);
				f->setLineWidth(2);
				break;
			case QFrame::Panel:
				if(kickerMode && f->parentWidget() &&  f->parentWidget()->isA("ClockApplet")) {
					f->setLineWidth(0);
					break;
				}
			case QFrame::WinPanel:
				f->setLineWidth(1);
				widget->installEventFilter(this);
				break;
			case QFrame::LineEditPanel: {
				widget->installEventFilter(this);
				break;
			}
			case QFrame::TabWidgetPanel:
				if(dynamic_cast<QWidgetStack*>(widget)) {
					if(f->parentWidget() && f->parentWidget()->isA("KonqFrameTabs"))
						break;
					
					f->setLineWidth(4);
					f->setFrameRect(QRect(0, 0, f->width(), f->height()-1));
					f->setMargin(0);

					QWidget* w = widget->parentWidget();
					for(int i=0; i<3; i++) { // the third parentcheck is for the tabWidget on kcm khtml_plugins
						if(!w) break;
						if(dynamic_cast<QGroupBox*>(w)) {
							widget->setPaletteBackgroundColor(qApp->palette().active().background());
								break;
						}
						w = w->parentWidget();
					}
					
					if(dynamic_cast<QWidgetStack*>(widget)->widget(0)) // only if it has contents
						((DominoQWidget*)widget)->setWFlags(Qt::WNoAutoErase);
				}
			case QFrame::HLine:
			case QFrame::VLine: {
				widget->installEventFilter(this);
				break;
				}
			case QFrame::StyledPanel:
				if(dynamic_cast<QDockWindow*>(widget)) {
					widget->installEventFilter(this);
				}
				else if(widget->isA("QWorkspaceChild")) {
					f->setFrameStyle(QFrame::Panel|QFrame::Sunken);
					f->setLineWidth(1);
					widget->installEventFilter(this);
				}
				else if(widget->isA("KAnimWidget"))
					f->setLineWidth(0);
				
			default:
				break;
		}
	}
// 	else if (dynamic_cast<KDockSplitter*>(widget)) {
// 		widget->installEventFilter(this);
// 	} else if (dynamic_cast<KDockWidgetAbstractHeaderDrag*>(widget)) {
// 		widget->installEventFilter(this);
	else if(widget->isA("QDateTimeEditor")) {
		widget->installEventFilter(this);
	}
	else if(widget->inherits("QDockWindowHandle") || widget->isA("QDockWindowResizeHandle") ||
		        widget->isA("AppletHandleDrag")) { // !qstrcmp(widget->name(), "qt_dockwidget_internal")
		widget->installEventFilter(this);
	}
	else if(widget->isA("KDockWidget") && widget->parentWidget() && widget->parentWidget()->parentWidget()
		       && widget->parentWidget()->parentWidget()->isA("Sidebar_Widget")) {
		widget->setEraseColor(widget->palette().active().base());
	}
	
// 	widget->installEventFilter(this);
	//qDebug("className %s, name: %s", widget->className(), widget->name());
	
}

void DominoStyle::unPolish(QWidget* widget)
{
	
	if( !strcmp(widget->name(), "__khtml") || (!strcmp(widget->name(), "kfile button") && !strcmp(widget->parentWidget()->name(), "__khtml"))) { // is it a khtml widget...?
		khtmlWidgets.remove(widget);
	}

	if (dynamic_cast<QPushButton*>(widget) || 
		   dynamic_cast<QSpinWidget*>(widget) || dynamic_cast<QSlider*>(widget) ||
		   dynamic_cast<QRadioButton*>(widget) ||
		   dynamic_cast<QListBox*>(widget) || dynamic_cast<QListView*>(widget) ||
		   dynamic_cast<QStatusBar*>(widget) || widget->isA("AppletHandleDrag") ||
		   widget->isA("QDockWindowHandle"))
	{
		((DominoQWidget*)widget)->clearWFlags(Qt::WNoAutoErase);
		widget->removeEventFilter(this);
	}
	else if(widget->isA("KonqCheckBox") || widget->isA("QWhatsThat") || widget->isA("QTipLabel")) {
		widget->removeEventFilter(this);
		widget->setPaletteBackgroundColor(qApp->palette().active().background());
		widget->setBackgroundMode(Qt::PaletteBackground);
	}
	else if(dynamic_cast<QTabWidget*>(widget) || dynamic_cast<QTabBar*>(widget)) {
		widget->removeEventFilter(this);
		widget->setBackgroundMode(Qt::PaletteBackground);
	}
	else if(dynamic_cast<QCheckBox*>(widget)) {
		((DominoQWidget*)widget)->clearWFlags(Qt::WNoAutoErase);
		widget->removeEventFilter(this);
	}
	else if(dynamic_cast<QComboBox*>(widget)) { // after KonqCheckBox
		((DominoQWidget*)widget)->clearWFlags(Qt::WNoAutoErase);
		widget->removeEventFilter(this);
	}
	else if(dynamic_cast<QHeader*>(widget)) {
		widget->removeEventFilter(this);
		widget->setBackgroundMode(Qt::PaletteButton);
	}
	else if(dynamic_cast<QPopupMenu*>(widget)) {
		widget->removeEventFilter(this);
		widget->setBackgroundMode(Qt::PaletteBackground);
		static_cast<QFrame*>(widget)->setLineWidth(2);
		widget->clearMask();
	}
	else if(dynamic_cast<KToolBarButton*>(widget)) {
		widget->removeEventFilter(this);
		widget->setBackgroundMode(Qt::PaletteBackground);
	}
	else if(dynamic_cast<QGroupBox*>(widget)) {
		widget->removeEventFilter(this);
		((DominoQWidget*)widget)->clearWFlags(Qt::WNoAutoErase);
		QGroupBox* gb = dynamic_cast<QGroupBox*>(widget);
		gb->setBackgroundMode(Qt::PaletteBackground);
		gb->setPalette(qApp->palette());
		gb->setAlignment(Qt::AlignLeft|Qt::AlignBottom);
	}
	else if(dynamic_cast<QLineEdit*>(widget)) {
		widget->removeEventFilter(this);
		widget->setBackgroundMode(Qt::PaletteBase);
		QLineEdit* le = dynamic_cast<QLineEdit*>(widget);
		QRect r = le->rect();
		le->setFrameRect(QRect(r.x()-1, r.y(), r.width()+1, r.height()));
		widget->clearMask();
	}
	else if(dynamic_cast<QProgressBar*>(widget) ) {
		progAnimWidgets.remove(widget);
	}
	else if(dynamic_cast<KToolBarSeparator*>(widget) || dynamic_cast<QToolBar*>(widget) ) {
		widget->removeEventFilter(this);
		widget->setBackgroundMode(Qt::PaletteButton);
	}
	if(widget->isA("KonqFileTip")) {
		widget->removeEventFilter(this);
		widget->setPaletteBackgroundColor(widget->palette().active().background());
		QFrame* f = dynamic_cast<QFrame*>(widget);
		f->setMargin(1);
		f->setFrameStyle(QFrame::Plain|QFrame::Box);

		QObjectList *list = const_cast<QObjectList *>(widget->children());
		QObjectListIt it( *list );
		QObject * o;
		while ( (o = it.current()) != 0 ) {
			++it;
			if(dynamic_cast<QLabel*>(o)) {
				QLabel* l = dynamic_cast<QLabel*>(o);
				l->setBackgroundOrigin(QWidget::WidgetOrigin);
				l->setBackgroundMode(Qt::PaletteBackground);
				l->setPaletteBackgroundColor(l->palette().active().background());
			}
		}
	}
	else if(dynamic_cast<QScrollView*>(widget)) {
		QFrame* f = dynamic_cast<QFrame*>(widget);
		if((konquerorMode && widget->isA("KHTMLView")) || f->parentWidget() && !strcmp(f->parentWidget()->name(), "KonqFrame")) { // konqi frames
			widget->removeEventFilter(this);
			f->setFrameStyle(QFrame::NoFrame|QFrame::Plain);
			f->setLineWidth(0);
		}
	}
	else if(dynamic_cast<KPopupTitle*>(widget)) {
		widget->removeEventFilter(this);
	}
	else if(dynamic_cast<QFrame*>(widget)) {
		widget->removeEventFilter(this);
		QFrame* f = dynamic_cast<QFrame*>(widget);
		switch(f->frameShape()) {
			case QFrame::Panel:
				if(widget->isA("QWorkspaceChild")) {
					f->setFrameStyle(QFrame::StyledPanel|QFrame::Sunken);
					f->setLineWidth(2);
				}
				break;
			case QFrame::WinPanel:
				f->setLineWidth(2);
				break;
			case QFrame::TabWidgetPanel:
				f->setLineWidth(2);
				f->setMargin(2);
				f->setFrameRect(QRect(0, 0, 0, 0));
				break;
			case QFrame::StyledPanel:
				if(widget->isA("KAnimWidget"))
					f->setLineWidth(2);
				break;
			default:
				break;
		}
	}

	
	KStyle::unPolish(widget);
}

void DominoStyle::khtmlWidgetDestroyed(QObject* obj)
{
	khtmlWidgets.remove(static_cast<QWidget*>(obj));
}

void DominoStyle::groupBoxDestroyed(QObject* obj)
{
	groupBoxPixmaps.remove(static_cast<QGroupBox*>(obj));
}

void DominoStyle::progressBarDestroyed(QObject* obj)
{
	progAnimWidgets.remove(static_cast<QWidget*>(obj));
}


void DominoStyle::renderContour(QPainter *p, const QRect &r, const QColor &backgroundColor, const QColor &contour, const uint flags) const {
	
	if((r.width() <= 0)||(r.height() <= 0))
		return;
	
	const bool drawLeft = flags&Draw_Left;
	const bool drawRight = flags&Draw_Right;
	const bool drawTop = flags&Draw_Top;
	const bool drawBottom = flags&Draw_Bottom;
//	const bool disabled = flags&Is_Disabled;
	const bool alphaBlend = flags&Draw_AlphaBlend;

	const bool roundUpperLeft = flags&Round_UpperLeft;
	const bool roundUpperRight = flags&Round_UpperRight;
	const bool roundBottomLeft = flags&Round_BottomLeft;
	const bool roundBottomRight = flags&Round_BottomRight;
	const bool isButton = flags&Is_Button;
	const bool isHorizontal = flags&Is_Horizontal;
	const bool isScrollBarBtn = flags&Is_ScrollBarBtn;
	
	QColor appButtonColor = qApp->palette().active().button();
	
	if(isButton) {
		
		QRect sr = QRect(r.x(), r.y()+2, r.width(), r.height()-5); // surface rect
		int top = sr.y()+sr.height()*btnSurface.g1Top/100;
		int bottom = sr.height()*btnSurface.g1Bottom/100;
		int top2 = sr.y()+sr.height()*btnSurface.g2Top/100;
		int bottom2 = sr.y()+sr.height()*btnSurface.g2Bottom/100-top2;

		int left = sr.x()+sr.width()*btnSurface.g1Top/100;
		int width = sr.width()*btnSurface.g1Bottom/100;
		int left2 = sr.x()+sr.width()*btnSurface.g2Top/100;
		int width2 = sr.width()*btnSurface.g2Bottom/100-left2;
		
		// darker surface colors for the inner part of the contour
		QColor insideBackground;
		QColor gr1TopColor;
		QColor gr1BottomColor;
		QColor gr2TopColor;
		QColor gr2BottomColor;
		
		const int alphaInside = qAlpha(buttonContour->color3());
		
		insideBackground = alphaBlendColors(buttonContour->color3(),
				((contour != appButtonColor) ? alphaBlendColors(contour, btnSurface.background, 127) : btnSurface.background), alphaInside);

		switch (btnSurface.numGradients) {
			case 2:{
				
				gr1TopColor = alphaBlendColors(buttonContour->color3(),
						((contour != appButtonColor) ? alphaBlendColors(contour, btnSurface.g1Color1, 127) : btnSurface.g1Color1), alphaInside);
				
				gr1BottomColor = alphaBlendColors(buttonContour->color3(),
						((contour != appButtonColor) ? alphaBlendColors(contour, btnSurface.g1Color2, 127) : btnSurface.g1Color2), alphaInside);
				
				gr2TopColor = alphaBlendColors(buttonContour->color3(),
						((contour != appButtonColor) ? alphaBlendColors(contour, btnSurface.g2Color1, 127) : btnSurface.g2Color1), alphaInside);
				
				gr2BottomColor = alphaBlendColors(buttonContour->color3(),
						((contour != appButtonColor) ? alphaBlendColors(contour, btnSurface.g2Color2, 127) : btnSurface.g2Color2), alphaInside);
				
				break;
			}
			case 1: {
				gr1TopColor = alphaBlendColors(buttonContour->color3(),
						((contour != appButtonColor) ? alphaBlendColors(contour, btnSurface.g1Color1, 127) : btnSurface.g1Color1), alphaInside);
				
				gr1BottomColor = alphaBlendColors(buttonContour->color3(),
						((contour != appButtonColor) ? alphaBlendColors(contour, btnSurface.g1Color2, 127) : btnSurface.g1Color2), alphaInside);
				break;
			}
			default: {
				insideBackground = insideBackground;
				gr1TopColor = insideBackground;
				gr1BottomColor = insideBackground;
				gr2TopColor = insideBackground;
				gr2BottomColor = insideBackground;
				break;
			}
		}
	
		if(flags & Is_Down) {
			gr1TopColor = darkenColor(gr1TopColor, 15);
			gr1BottomColor = darkenColor(gr1BottomColor, 15);
			gr2TopColor = darkenColor(gr2TopColor, 15);
			gr2BottomColor = darkenColor(gr2BottomColor, 15);
			insideBackground = darkenColor(insideBackground, 15);
		}
	
		
		
		QRegion clip;
		if(drawTop)
			clip += QRegion(r.left()+6, r.top()+2, r.width()-12, 1);
		if(drawBottom)
			clip += QRegion(r.left()+6, r.bottom()-3, r.width()-12, 1);
		if(drawLeft)
			clip += QRegion(r.left()+2, r.top()+6, 1, r.height()-13);
		if(drawRight)
			clip += QRegion(r.right()-2, r.top()+6, 1, r.height()-13);
		p->setClipRegion(clip);
		
		
		p->fillRect(sr, insideBackground);
		switch(btnSurface.numGradients) {
			case 1:
				if(isHorizontal)
					renderGradient(p, QRect(sr.x(), top, sr.width(), bottom), gr1TopColor, gr1BottomColor, true);
				else
					renderGradient(p, QRect(left, sr.y(), width, sr.height()), gr1TopColor, gr1BottomColor, false);
				break;
			case 2:
				if(isHorizontal) {
					renderGradient(p, QRect(r.x(), top, r.width(), bottom), gr1TopColor, gr1BottomColor, true);
					renderGradient(p, QRect(r.x(), top2, r.width(), bottom2), gr2TopColor, gr2BottomColor, true);
				}
				else {
					renderGradient(p, QRect(left, sr.y(), width, sr.height()), gr1TopColor,gr1BottomColor, false);
					renderGradient(p, QRect(left2, sr.y(), width2, sr.height()), gr2TopColor,gr2BottomColor, false);
				}
			default:
				break;
		}
		p->setClipping(false);
		
		
		const QColor color1 = alphaBlendColors(buttonContour->color1(), backgroundColor, qAlpha(buttonContour->color1())); // 1.top
		const QColor color4 = alphaBlendColors(buttonContour->color4(), backgroundColor, qAlpha(buttonContour->color4())); // 3.bottom
		const QColor color5 = alphaBlendColors(buttonContour->color5(), backgroundColor, qAlpha(buttonContour->color5())); // 4.bottom
		const QColor color2 = alphaBlendColors(buttonContour->color2(), backgroundColor, qAlpha(buttonContour->color2())); // 2.top 2.left  2.bottom
		const QColor color6 = alphaBlendColors(buttonContour->color6(), backgroundColor, qAlpha(buttonContour->color6())); // 1.left
		
		
		if(roundUpperLeft)
			p->drawPixmap(r.left(), r.top(), *buttonContour->buttonEdges(), 0, 0, 6, 6);
		
		if(roundUpperRight)
			p->drawPixmap(r.right()-5, r.top(), *buttonContour->buttonEdges(), 6, 0, 6, 6);
		
		if(roundBottomLeft)
			p->drawPixmap(r.left(), r.bottom()-6, *buttonContour->buttonEdges(), 0, 6, 6, 7);
		
		if(roundBottomRight)
			p->drawPixmap(r.right()-5, r.bottom()-6, *buttonContour->buttonEdges(), 6, 6, 6, 7);
		
		if(drawTop) {
			if(!alphaBlend) {
				p->setPen(color1);
				p->drawLine(r.left()+6, r.top(), r.right()-6, r.top());
				p->setPen(color2);
				p->drawLine(r.left()+6, r.top()+1, r.right()-6, r.top()+1);
			}
			else {
				p->drawTiledPixmap(r.left()+6, r.top(), r.width()-12, 2, *buttonContour->buttonHLines(), 0, 0);
			}
		}
		
		if(drawBottom) {
			if(!alphaBlend) {
				p->setPen(color2);
				p->drawLine(r.left()+6, r.bottom()-2, r.right()-6, r.bottom()-2);
				p->setPen(color4);
				p->drawLine(r.left()+6, r.bottom()-1, r.right()-6, r.bottom()-1);
				p->setPen(color5);
				p->drawLine(r.left()+6, r.bottom(), r.right()-6, r.bottom());
			}
			else {
				p->drawTiledPixmap(r.left()+6, r.bottom()-2, r.width()-12, 3, *buttonContour->buttonHLines(), 0, 4);
			}
		}
		
		if(drawLeft) {
			if(!alphaBlend) {
				p->setPen(color6);
				p->drawLine(r.left(), r.top()+6, r.left(), r.bottom()-7);
				p->setPen(color2);
				p->drawLine(r.left()+1, r.top()+6, r.left()+1, r.bottom()-7);
			}
			else {
				p->drawTiledPixmap(r.left(), r.top()+6, 2, r.height()-13, *buttonContour->buttonVLines(), 0, 0);
			}
		}
		
		if(drawRight) {
			if(!alphaBlend) {
				p->setPen(color2);
				p->drawLine(r.right()-1, r.top()+6, r.right()-1, r.bottom()-7);
				p->setPen(color6);
				p->drawLine(r.right(), r.top()+6, r.right(), r.bottom()-7);
			}
			else {
				p->drawTiledPixmap(r.right()-1, r.top()+6, 2, r.height()-13, *buttonContour->buttonVLines(), 4, 0);
			}
		}
		
		
		
		
	}
	else if(isScrollBarBtn) {
		
		QColor background;
		QColor gr1TopColor;
		QColor gr1BottomColor;
		QColor gr2TopColor;
		QColor gr2BottomColor;
		
		
		if(contour != appButtonColor) {
			gr1TopColor = alphaBlendColors(contour, scrollBarSurface.g1Color1, 127);
			gr1BottomColor = alphaBlendColors(contour, scrollBarSurface.g1Color2, 127);
			gr2TopColor = alphaBlendColors(contour, scrollBarSurface.g2Color1, 127);
			gr2BottomColor = alphaBlendColors(contour, scrollBarSurface.g2Color2, 127);
			background = alphaBlendColors(contour, scrollBarSurface.background, 127);
		}
		else {
			gr1TopColor = scrollBarSurface.g1Color1;
			gr1BottomColor = scrollBarSurface.g1Color2;
			gr2TopColor = scrollBarSurface.g2Color1;
			gr2BottomColor = scrollBarSurface.g2Color2;
			background = scrollBarSurface.background;
		}
		
		QColor btnContour = buttonContour->contourColor(Contour_Default);
		QColor whiteColor( qRgba(Qt::white.red(), Qt::white.green(), Qt::white.blue(), 23)); // first white layer
		QColor blackColor( qRgba(btnContour.red(), btnContour.green(), btnContour.blue(), 35)); // second contour layer
		
		
		gr1TopColor = blendColors(gr1TopColor, whiteColor);
		gr1TopColor = blendColors(gr1TopColor, blackColor);
		
		gr1BottomColor = blendColors(gr1BottomColor, whiteColor);
		gr1BottomColor = blendColors(gr1BottomColor, blackColor);
		
		gr2TopColor = blendColors(gr2TopColor, whiteColor);
		gr2TopColor = blendColors(gr2TopColor, blackColor);
		
		gr2BottomColor = blendColors(gr2BottomColor, whiteColor);
		gr2BottomColor = blendColors(gr2BottomColor, blackColor);
		
		background = blendColors(background, whiteColor);
		background = blendColors(background, blackColor);
		
		
		
		
		QRegion clip;
		if(drawTop)
			clip += QRegion(r.left(), r.top()+1, r.width(), 1);
		if(drawBottom)
			clip += QRegion(r.left(), r.bottom()-1, r.width(), 1);
		if(drawLeft)
			clip += QRegion(r.left()+1, r.top(), 1, r.height());
		if(drawRight)
			clip += QRegion(r.right()-1, r.top(), 1, r.height());
		
		p->setClipRegion(clip);
		
		
		QRect sr = r;
		int top = sr.y()+sr.height()*scrollBarSurface.g1Top/100;
		int bottom = sr.height()*scrollBarSurface.g1Bottom/100;
		int top2 = sr.y()+sr.height()*scrollBarSurface.g2Top/100;
		int bottom2 = sr.height()*scrollBarSurface.g2Bottom/100-top2;
		int left = sr.x()+sr.width()*scrollBarSurface.g1Top/100;
		int width = sr.width()*scrollBarSurface.g1Bottom/100;
		int left2 = sr.x()+sr.width()*scrollBarSurface.g2Top/100;
		int width2 = sr.width()*scrollBarSurface.g2Bottom/100-left2;
		
		p->fillRect(sr, background);
		switch(scrollBarSurface.numGradients) {
			case 1:
				if(isHorizontal)
					renderGradient(p, QRect(sr.x(), top, sr.width(), bottom), gr1TopColor, gr1BottomColor, true);
				else
					renderGradient(p, QRect(left, sr.y(), width, sr.height()), gr1TopColor, gr1BottomColor, false);
				break;
			case 2:
				if(isHorizontal) {
					renderGradient(p, QRect(r.x(), top, r.width(), bottom), gr1TopColor, gr1BottomColor, true);
					renderGradient(p, QRect(r.x(), top2, r.width(), bottom2), gr2TopColor, gr2BottomColor, true);
				}
				else {
					renderGradient(p, QRect(left, sr.y(), width, sr.height()), gr1TopColor,gr1BottomColor, false);
					renderGradient(p, QRect(left2, sr.y(), width2, sr.height()), gr2TopColor,gr2BottomColor, false);
				}
			default:
				break;
		}
		p->setClipping(false);
		
		
		
		QColor color1 = alphaBlendColors(buttonContour->contourColor(Contour_Default), backgroundColor, 203); // like btn contour
		QColor color2 = alphaBlendColors(QColor(255, 255, 255), color1, 25);
		
		p->setPen(color1);
		if(drawTop) {
			p->drawLine(r.left(), r.top(), r.right(), r.top()); // top
		}
		if(drawBottom) {
			p->drawLine(r.left(), r.bottom(), r.right(), r.bottom()); // bottom
		}
		if(drawLeft) {
			p->drawLine(r.left(), r.top(), r.left(), r.bottom()); // left
		}
		if(drawRight) {
			p->drawLine(r.right(), r.top(), r.right(), r.bottom()); // right
		}
		
		
		color1 = blendColors(color1, whiteColor);
		p->setPen(color2);
		if(roundUpperLeft) {
			renderPixel(p, QPoint(r.x()+1, r.y()+1), 127, color1, color1, true);
			p->drawPoint(r.x(), r.y());
		}
		if(roundUpperRight) {
			renderPixel(p, QPoint(r.right()-1, r.top()+1), 127, color1, color1, true);
			p->drawPoint(r.right(), r.y());
			
		}
		if(roundBottomLeft) {
			renderPixel(p, QPoint(r.x()+1, r.bottom()-1), 127, color1, color1, true);
			p->drawPoint(r.x(), r.bottom());
		}
		if(roundBottomRight) {
			renderPixel(p, QPoint(r.right()-1, r.bottom()-1), 127, color1, color1, true);
			p->drawPoint(r.right(), r.bottom());
		}
	}
	return;
}


void DominoStyle::renderSurface(QPainter *p,
								 const QRect &r,
								 const QColor &backgroundColor,
								 const QColor &buttonColor,
								 const QColor &/*highlightColor*/,
								 const uint flags) const
{
	
	if((r.width() <= 0)||(r.height() <= 0))
		return;
	
	const DSurface* ds = 0;
	const bool isButton = flags&Is_Button;
	const bool isScrollBarBtn = flags&Is_ScrollBarBtn;
	const bool isScrollBarGroove = flags&Is_ScrollBarGroove;
	const bool isCheckItem = flags&Is_CheckItem;
	const bool isHeader = flags&Is_Header;
	const bool isTabTop = flags&Is_TabTop;
	const bool isActiveTabTop = flags&Is_ActiveTabTop;
	const bool isTabBottom = flags&Is_TabBottom;
	const bool isActiveTabBottom = flags&Is_ActiveTabBottom;
	const bool horizontal = flags&Is_Horizontal;
	const bool isDown = flags&Is_Down;
	
	if(isButton)
		ds = &btnSurface;
	else if(isCheckItem)
		ds = &checkItemSurface;
	else if(isScrollBarBtn)
		ds = &scrollBarSurface;
	else if(isScrollBarGroove)
		ds = &scrollBarGrooveSurface;
	else if(isHeader)
		ds = &headerSurface;
	else if(isTabTop)
		ds = &tabTopSurface;
	else if(isActiveTabTop)
		ds = &activeTabTopSurface;
	else if(isTabBottom)
		ds = &tabBottomSurface;
	else if(isActiveTabBottom)
		ds = &activeTabBottomSurface;
	
	
	
	
	
	QColor g1Color1;
	QColor g1Color2;
	QColor g2Color1;
	QColor g2Color2;
	QColor background;

	if(buttonColor != qApp->palette().active().button()) {
		g2Color1 = alphaBlendColors(buttonColor, ds->g2Color1, 127);
		g2Color2 = alphaBlendColors(buttonColor, ds->g2Color2, 127);
		g1Color1 = alphaBlendColors(buttonColor, ds->g1Color1, 127);
		g1Color2 = alphaBlendColors(buttonColor, ds->g1Color2, 127);
		background = alphaBlendColors(buttonColor, ds->background, 127);
	}
	else {
		g1Color1 = ds->g1Color1;
		g1Color2 = ds->g1Color2;
		g2Color1 = ds->g2Color1;
		g2Color2 = ds->g2Color2;
		background = ds->background;
	}
	if(isDown) {
		g1Color1 = darkenColor(g1Color1, 15);
		g1Color2 = darkenColor(g1Color2, 15);
		g2Color1 = darkenColor(g2Color1, 15);
		g2Color2 = darkenColor(g2Color2, 15);
		background = darkenColor(background, 15);
	}

	if(isButton) { // button / toolButton / comboBox / spinWidget / slider / progressBar
		
		if(!flatMode) {
			QRegion mask(r.x()+2, r.y(), r.width()-4, r.height());
			mask += QRegion(r.x()+1, r.y()+1, r.width()-2, r.height()-2);
			mask += QRegion(r.x(), r.y()+2, r.width(), r.height()-4);
			p->setClipRegion(mask, QPainter::CoordPainter);
		}
		
		
		QRect re = r;
		if(re.x() < 0 ) {
			re.setWidth(re.width()+QABS(re.x()));
			re.setX(0);
		}
		else if(re.y() < 0 ) {
			re.setHeight(re.height()+QABS(re.y()));
			re.setY(0);
		}

		QPixmap pix(re.width(), re.height());
		pix.fill(background);
		QPainter painter(&pix);

		int top = re.height()*btnSurface.g1Top/100;
		int bottom = re.height()*btnSurface.g1Bottom/100;
		int top2 = re.height()*btnSurface.g2Top/100;
		int bottom2 = re.height()*btnSurface.g2Bottom/100-top2;
		
		int left = re.width()*btnSurface.g1Top/100;
		int width = re.width()*btnSurface.g1Bottom/100;
		int left2 = re.width()*btnSurface.g2Top/100;
		int width2 = re.width()*btnSurface.g2Bottom/100-left2;
		
		switch (btnSurface.numGradients) {
			case 1:{
				if(horizontal) {
					renderGradient(&painter, QRect(0, top, re.width(), bottom), g1Color1, g1Color2, true);
				}
				else {
					renderGradient(&painter, QRect(left, 0, width, re.height()), g1Color1, g1Color2, false);
				}
				break;
			}
			case 2: {
				if(horizontal) {
					renderGradient(&painter, QRect(0, top, re.width(), bottom), g1Color1, g1Color2, true);
					renderGradient(&painter, QRect(0, top2, re.width(), bottom2), g2Color1, g2Color2, true);
				}
				else {
					renderGradient(&painter, QRect(left, 0, width, re.height()), g1Color1, g1Color2, false);
					renderGradient(&painter, QRect(left2, 0, width2, re.height()), g2Color1, g2Color2, false);
				}
			}
			default: break;
		}
		
		p->drawPixmap(r.x(), r.y(), pix);
		p->setClipping(false);

	}
	else if(isTabTop || isActiveTabTop || isTabBottom || isActiveTabBottom) {
		
		int top = r.y()+r.height()*ds->g1Top/100;
		int bottom = r.height()*ds->g1Bottom/100;
		int top2 = r.y()+r.height()*ds->g2Top/100;
		int bottom2 = r.height()*ds->g2Bottom/100-top2;
		
		p->fillRect(r, background);
		switch (ds->numGradients) {
			case 1:{
				renderGradient(p, QRect(r.x(), top, r.width(), bottom), g1Color1, g1Color2, true);
				break;
			}
			case 2: {
				renderGradient(p, QRect(r.x(), top, r.width(), bottom), g1Color1, g1Color2, true);
				renderGradient(p, QRect(r.x(), top2, r.width(), bottom2), g2Color1, g2Color2, true);
			}
			default: break;
		}
	}
	else if(isScrollBarBtn) {

		int top = r.height()*scrollBarSurface.g1Top/100;
		int bottom = r.height()*scrollBarSurface.g1Bottom/100;
		int top2 = r.height()*scrollBarSurface.g2Top/100;
		int bottom2 = r.height()*scrollBarSurface.g2Bottom/100-top2;
		int left = r.width()*scrollBarSurface.g1Top/100;
		int width = r.width()*scrollBarSurface.g1Bottom/100;
		int left2 = r.width()*scrollBarSurface.g2Top/100;
		int width2 = r.width()*scrollBarSurface.g2Bottom/100-left2;
		
		p->fillRect(r, background);
		switch (scrollBarSurface.numGradients) {
			case 1: {
				if(horizontal) {
					renderGradient(p, QRect(r.left(), top, r.width(), bottom), g1Color1, g1Color2, true);
				}
				else {
					renderGradient(p, QRect(left, r.top(), width, r.height()), g1Color1, g1Color2, false);
				}
				break;
			}
			case 2: {
				if(horizontal) {
					renderGradient(p, QRect(r.left(), top, r.width(), bottom), g1Color1, g1Color2, true);
					renderGradient(p, QRect(r.left(),  top2, r.width(), bottom2), g2Color1, g2Color2, true);
				}
				else {
					renderGradient(p, QRect(left, r.top(), width, r.height()), g1Color1, g1Color2, false);
					renderGradient(p, QRect(left2, r.top(), width2, r.height()), g2Color1, g2Color2, false);
				}
			}
			default:
				break;
		}
		
		
	}
	else if(isScrollBarGroove) {

		int top = r.height()*scrollBarGrooveSurface.g1Top/100;
		int bottom = r.height()*scrollBarGrooveSurface.g1Bottom/100;
		int top2 = r.height()*scrollBarGrooveSurface.g2Top/100;
		int bottom2 = r.height()*scrollBarGrooveSurface.g2Bottom/100-top2;
		int left = r.width()*scrollBarGrooveSurface.g1Top/100;
		int width = r.width()*scrollBarGrooveSurface.g1Bottom/100;
		int left2 = r.width()*scrollBarGrooveSurface.g2Top/100;
		int width2 = r.width()*scrollBarGrooveSurface.g2Bottom/100-left2;
		
		p->fillRect(r, background);
		switch (scrollBarGrooveSurface.numGradients) {
			case 1: {
				if(horizontal) {
					renderGradient(p, QRect(r.left(), top, r.width(), bottom), g1Color1, g1Color2, true);
				}
				else {
					renderGradient(p, QRect(left, r.top(), width, r.height()), g1Color1, g1Color2, false);
				}
				break;
			}
			case 2: {
				if(horizontal) {
					renderGradient(p, QRect(r.left(), top, r.width(), bottom), g1Color1, g1Color2, true);
					renderGradient(p, QRect(r.left(),  top2, r.width(), bottom2), g2Color1, g2Color2, true);
				}
				else {
					renderGradient(p, QRect(left, r.top(), width, r.height()), g1Color1, g1Color2, false);
					renderGradient(p, QRect(left2, r.top(), width2, r.height()), g2Color1, g2Color2, false);
				}
			}
			default:
				break;
		}
		
	}
	else if(isCheckItem) {
		
		QPixmap pix(r.width(), r.height());
		pix.fill(background);
		QPainter painter(&pix);

		int top = r.height()*checkItemSurface.g1Top/100;
		int bottom = r.height()*checkItemSurface.g1Bottom/100;
		int top2 = r.height()*checkItemSurface.g2Top/100;
		int bottom2 = r.height()*checkItemSurface.g2Bottom/100-top2;
		
		switch(checkItemSurface.numGradients) {
			case 1:{
				renderGradient(&painter, QRect(0, top, r.width(), bottom), g1Color1, g1Color2, true);
				break;
			}
			case 2: {
				renderGradient(&painter, QRect(0, top, r.width(), bottom), g1Color1, g1Color2, true);
				renderGradient(&painter, QRect(0, top2, r.width(), bottom2), g2Color1, g2Color2, true);
				break;
			}
			default: break;
		}
		painter.end();
		p->drawPixmap(QRect(r.x(), r.y()+3, r.width(), r.height()-7), pix);
		
	}
	else if(isHeader) {
		QHeader* hdr = dynamic_cast<QHeader*>(p->device());
		bool needErasePixmap = false;
		QRect re = r;
		bool horizontal = false;
		bool drawTop = false;
		
		if(hdr) {
			if(hdr->orientation() == Qt::Horizontal) {
				horizontal = true;
				bool hasErasePixmap = hdr->erasePixmap();
				if(hasErasePixmap && !isDown) {
					hdr->erase(r);
					return;
				}
				else if(!hasErasePixmap && !isDown) {
					needErasePixmap = true;
					re = QRect(0, 0, 10, r.height());
				}
			}
			else if(hdr->sectionAt(r.top()+hdr->offset()) == 0 && r.y() == 0)
				drawTop = true;
		}
		
		QPixmap pix(re.width(), re.height());
		pix.fill(background);
		QPainter painter(&pix);
		
		int top = re.height()*headerSurface.g1Top/100;
		int bottom = re.height()*headerSurface.g1Bottom/100;
		int top2 = re.height()*headerSurface.g2Top/100;
		int bottom2 = re.height()*headerSurface.g2Bottom/100-top2;
		
		switch (headerSurface.numGradients) {
			case 1:{
				renderGradient(&painter, QRect(0, top, re.width(), bottom), g1Color1, g1Color2, true);
				break;
			}
			case 2: {
				renderGradient(&painter, QRect(0, top, re.width(), bottom), g1Color1, g1Color2, true);
				renderGradient(&painter, QRect(0, top2, re.width(), bottom2), g2Color1, g2Color2, true);
				break;
			}
			default: break;
		}
		
		painter.setPen(alphaBlendColors(buttonContour->contourColor(Contour_Default), backgroundColor, 179));
		if(horizontal || drawTop)
			painter.drawLine(0, re.y(), re.width(), re.y());
		painter.drawLine(0, re.height()-1, re.width(), re.height()-1);
		
		if(needErasePixmap) {
			hdr->setUpdatesEnabled(false);
			hdr->setErasePixmap(pix);
			hdr->setUpdatesEnabled(true);
			hdr->erase(r);
		}
		else {
			// same problem as with the scrollbars, if we don't clip, the text above will be randomly erased.
			p->setClipRect(re);
			p->drawPixmap(r.x(), r.y(), pix);
			p->setClipping(false);
		}
	}
}


void DominoStyle::renderPixel(QPainter *p,
			const QPoint &pos,
			const int alpha,
			const QColor &color,
			const QColor &background,
			bool fullAlphaBlend) const
{	 
	if(fullAlphaBlend)
	// full alpha blend: paint into an image with alpha buffer and convert to a pixmap ...
	{
		QRgb rgb = color.rgb();
		// generate a quite unique key -- use the unused width field to store the alpha value.
		CacheEntry search(cAlphaDot, alpha, 0, rgb);
		int key = search.key();

		CacheEntry *cacheEntry;
		if( (cacheEntry = pixmapCache->find(key)) ) {
			if( search == *cacheEntry ) { // match! we can draw now...
				if(cacheEntry->pixmap)
					p->drawPixmap(pos, *(cacheEntry->pixmap) );
				return;
			} else { //Remove old entry in case of a conflict!
				pixmapCache->remove( key );
			}
		}


		QImage aImg(1,1,32); // 1x1
		aImg.setAlphaBuffer(true);
		aImg.setPixel(0,0,qRgba(qRed(rgb),qGreen(rgb),qBlue(rgb),alpha));
		QPixmap *result = new QPixmap(aImg);

		p->drawPixmap(pos, *result);

		// add to the cache...
		CacheEntry *toAdd = new CacheEntry(search);
		toAdd->pixmap = result;
		bool insertOk = pixmapCache->insert( key, toAdd, result->depth()/8);
		if(!insertOk)
			delete result;
	} else
	// don't use an alpha buffer: calculate the resulting color from the alpha value, the fg- and the bg-color.
	{
		QRgb rgb_a = color.rgb();
		QRgb rgb_b = background.rgb();
		int a = alpha;
		if(a>255) a = 255;
		if(a<0) a = 0;
		int a_inv = 255 - a;

		QColor res	= QColor( qRgb(qRed(rgb_b)*a_inv/255 + qRed(rgb_a)*a/255,
							  qGreen(rgb_b)*a_inv/255 + qGreen(rgb_a)*a/255,
							  qBlue(rgb_b)*a_inv/255 + qBlue(rgb_a)*a/255) );
		p->setPen(res);
		p->drawPoint(pos);
	} 
}

void DominoStyle::renderButtonShadow(QPainter *p, const QRect &r, const uint flags) const {
	
	if((!buttonContour->drawButtonSunkenShadow && buttonContour->defaultType == Contour_Sunken) || (buttonContour->defaultType != Contour_Sunken && !flatMode))
		return;
	
	
	if(flags&Round_UpperLeft) {
		bitBlt(p->device(), r.left(), r.top(), buttonContour->shadowRound(), 0, 0, 5, 5, Qt::CopyROP);
	}
	else if(flags&Rectangular_UpperLeft){
		bitBlt(p->device(), r.left(), r.top(), buttonContour->shadowRectangular(), 0, 0, 5, 5, Qt::CopyROP);
	}
	if(flags&Round_UpperRight) {
		bitBlt(p->device(), r.right()-6, r.top(), buttonContour->shadowRound(), 5, 0, 5, 5, Qt::CopyROP);
	}
	else if(flags&Rectangular_UpperRight){
		bitBlt(p->device(), r.right()-6, r.top(), buttonContour->shadowRectangular(), 5, 0, 5, 5, Qt::CopyROP);
	}
	if(flags&Round_BottomLeft) {
		bitBlt(p->device(), r.left(), r.bottom()-5, buttonContour->shadowRound(), 0, 5, 4, 4, Qt::CopyROP);
	}
	else if(flags&Rectangular_BottomLeft)
		bitBlt(p->device(), r.left(), r.bottom()-5, buttonContour->shadowRectangular(), 0, 5, 4, 4, Qt::CopyROP);
	if(flags&Round_BottomRight) {
		bitBlt(p->device(), r.right()-5, r.bottom()-5, buttonContour->shadowRound(), 4, 5, 4, 4, Qt::CopyROP);
	}
	else if(flags&Rectangular_BottomRight)
		bitBlt(p->device(), r.right()-5, r.bottom()-5, buttonContour->shadowRectangular(), 4, 5, 4, 4, Qt::CopyROP);
	
	if(flags&Draw_Top)
		p->drawTiledPixmap(r.left()+5, r.top(), r.width()-12, 3, *buttonContour->shadowHLines(), 0, 0);
	if(flags&Draw_Left)
		p->drawTiledPixmap(r.left(), r.top()+5, 3, r.height()-11, *buttonContour->shadowVLines(), 0, 0);
	if(flags&Draw_Right)
		p->drawTiledPixmap(r.right()-4, r.top()+5, 3, r.height()-11, *buttonContour->shadowVLines(), 3, 0);
	if(flags&Draw_Bottom)
		p->drawTiledPixmap(r.left()+4, r.bottom()-3, r.width()-10, 2, *buttonContour->shadowHLines(), 0, 3);
	
}

void DominoStyle::renderButton(QPainter *p,
							   const QRect &r,
							   const QColorGroup &g,
							   bool sunken,
							   bool /*mouseOver*/,
							   bool horizontal,
							   bool enabled,
							   bool khtmlMode) const
{
	
	uint contourFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom;

	if(!enabled) contourFlags|=Is_Disabled;
	
	if(khtmlMode || buttonContour->alphaMode() /* || kickerMode*/) {
		contourFlags|=Draw_AlphaBlend;
	}

	uint surfaceFlags = Is_Button;
	if(horizontal) {
		 surfaceFlags|=Is_Horizontal;
		 contourFlags|=Is_Horizontal;
	}
	if(!enabled)
		surfaceFlags|=Is_Disabled;
	else {
		if(sunken) {
			surfaceFlags|=Is_Down;
			contourFlags|=Is_Down;
		}
	}

// 	QColor buttonColor = khtmlMode ? g.button() : qApp->palette().active().background();
	
	if (!flatMode) {
		contourFlags |= Round_UpperLeft|Round_UpperRight|Round_BottomLeft|Round_BottomRight|Is_Button;
		renderSurface(p, QRect(r.x()+2, r.y()+2, r.width()-4, r.height()-5), g.background(), g.button(), g.button(), surfaceFlags);
		renderContour(p, r, g.background(), g.button(), contourFlags);
		if(sunken) {
			renderButtonShadow(p, QRect(r.x()+2, r.y()+2, r.width()-2, r.height()-3), contourFlags);
		}
	}
	else {
		renderSurface(p, QRect(r.x()-3, r.y()-1, r.width()+6, r.height()+2), g.background(), g.button(), g.button(), surfaceFlags);

		if(sunken) {
			contourFlags |= Rectangular_UpperLeft|Rectangular_UpperRight|Rectangular_BottomLeft|Rectangular_BottomRight;
			renderButtonShadow(p, QRect(r.x()-2, r.y()-2, r.width()+4, r.height()+4), contourFlags);
		}
	}
}

void DominoStyle::renderHeader(QPainter *p,
								const QRect &r,
								const QColorGroup &g,
								bool sunken,
								bool /*mouseOver*/,
								bool /*horizontal*/,
								bool /*enabled*/) const
{
	p->save();
	bool drawLeft = true;
	QHeader* hd = dynamic_cast<QHeader*>(p->device());
	if(hd && hd->orientation() == Qt::Horizontal)
		drawLeft = r.x() == 0 && hd->sectionAt(r.left()+hd->offset()) == 0;
	
	uint surfaceFlags = Is_Header|Is_Horizontal;
	if(sunken)
		surfaceFlags |= Is_Down;
	renderSurface(p, QRect(r.left(), r.top(), r.width(), r.height()), g.button(), g.button(), g.button(), surfaceFlags);
	
// 	p->setPen(buttonContour->contourColor(Contour_Default));
	p->setPen(alphaBlendColors(buttonContour->contourColor(Contour_Default), g.background(), 179));
	if(drawLeft)
		p->drawLine(r.left(), r.top(), r.left(), r.bottom());
	if(!hd)
		p->drawLine(r.right(), r.top(), r.left(), r.top());
	p->drawLine(r.right(), r.top(), r.right(), r.bottom());
	p->restore();
}


void DominoStyle::renderGradient(QPainter *painter,
								  const QRect &rect,
								  const QColor &c1,
								  const QColor &c2,
								  bool horizontal) const
{
	if((rect.width() <= 0)||(rect.height() <= 0))
		return;

	// generate a quite unique key for this surface.
	CacheEntry search(cGradientTile,
					  horizontal ? 0 : rect.width(),
					  horizontal ? rect.height() : 0,
					  c1.rgb(), c2.rgb(), horizontal );
	int key = search.key();

	CacheEntry *cacheEntry;
	if( (cacheEntry = pixmapCache->find(key)) ) {
		if( search == *cacheEntry ) { // match! we can draw now...
			if(cacheEntry->pixmap) {
				painter->drawTiledPixmap(rect, *(cacheEntry->pixmap) );
			}
			return;
		} else {
			// Remove old entry in case of a conflict!
			// This shouldn't happen very often, see comment in CacheEntry.
			pixmapCache->remove(key);
		}
	}

	// there wasn't anything matching in the cache, create the pixmap now...
	QPixmap *result = new QPixmap(horizontal ? 10 : rect.width(),
								  horizontal ? rect.height() : 10);
	QPainter p(result);

	int r_w = result->rect().width();
	int r_h = result->rect().height();
	int r_x, r_y, r_x2, r_y2;
	result->rect().coords(&r_x, &r_y, &r_x2, &r_y2);

	int rDiff, gDiff, bDiff;
	int rc, gc, bc;

	register int x, y;

	rDiff = ( c2.red())	  - (rc = c1.red());
	gDiff = ( c2.green()) - (gc = c1.green());
	bDiff = ( c2.blue())  - (bc = c1.blue());

	register int rl = rc << 16;
	register int gl = gc << 16;
	register int bl = bc << 16;

	int rdelta = ((1<<16) / (horizontal ? r_h : r_w)) * rDiff;
	int gdelta = ((1<<16) / (horizontal ? r_h : r_w)) * gDiff;
	int bdelta = ((1<<16) / (horizontal ? r_h : r_w)) * bDiff;

	// these for-loops could be merged, but the if's in the inner loop
	// would make it slow
	if(horizontal) {
		for ( y = 0; y < r_h; y++ ) {
			rl += rdelta;
			gl += gdelta;
			bl += bdelta;

			p.setPen(QColor(rl>>16, gl>>16, bl>>16));
			p.drawLine(r_x, r_y+y, r_x2, r_y+y);
		}
	} else {
		for( x = 0; x < r_w; x++) {
			rl += rdelta;
			gl += gdelta;
			bl += bdelta;

			p.setPen(QColor(rl>>16, gl>>16, bl>>16));
			p.drawLine(r_x+x, r_y, r_x+x, r_y2);
		}
	}

	p.end();

	// draw the result...
	painter->drawTiledPixmap(rect, *result);

	// insert into cache using the previously created key.
	CacheEntry *toAdd = new CacheEntry(search);
	toAdd->pixmap = result;
	bool insertOk = pixmapCache->insert( key, toAdd, result->width()*result->height()*result->depth()/8 );

	if(!insertOk)
		delete result;
}

void DominoStyle::renderTab(QPainter *p,
							const QRect &r,
							const QColorGroup &g,
							bool /*mouseOver*/,
							const bool selected,
							const bool bottom,
							const TabPosition pos,
							const bool /*triangular*/,
							const bool /*cornerWidget*/,
							const bool konqTab) const
{
	
	const bool isFirst = (pos == First);
	const bool isLast = (pos == Last);
	const bool isSingle = (pos == Single);
	
	QPixmap buffer(r.width(), r.height());
	QPainter painter(&buffer);
	
	QColor tabContour2 = g.background().dark(150);
	QPixmap* tabEdges;
	if(!(tabEdges = QPixmapCache::find(QString::number(tabContour2.pixel(), 16)+"tabEdges"))) {
		QImage img1 = tintImage(qembed_findImage("border1"), tabContour2);
		QImage img2 = qembed_findImage("tabInsideWhite");
		blend(img1, img2, img2);
		tabEdges = new QPixmap(img2);
		QPixmapCache::insert(QString::number(tabContour2.pixel(), 16)+"tabEdges", tabEdges);
	}
	
	QPixmap* tabHLines;
	if(!(tabHLines = QPixmapCache::find("tabHLines"))) {
		tabHLines = new QPixmap(qembed_findImage("tabHLines"));
		QPixmapCache::insert("tabHLines", tabHLines);
	}
	
	QPixmap* tabVLines;
	if(!(tabVLines = QPixmapCache::find("tabVLines"))) {
		tabVLines = new QPixmap(qembed_findImage("tabVLines"));
		QPixmapCache::insert("tabVLines", tabVLines);
	}
	
	
	if(konqTab) {
		
		const QColor background = g.background().dark(konqTabBarContrast);
		
		if (selected) {
			
			///painter.fillRect(0, r.height()-9, r.width(), 9, background); // rect below gradient
			///painter.fillRect(0, 0, r.width(), r.height()-9, g.background()); // rect above gradient
			painter.fillRect(0, 0, r.width(), 2, g.background()); // rect above gradient
			
			painter.translate(0, 2);
			renderSurface(&painter, QRect(1, 0, r.width()-2, r.height()-3), g.background(), g.button(), g.button(), Is_ActiveTabBottom);
			painter.translate(0, -2);

			
			QPixmap* tabEdges2;
			QString position = isFirst ? "first" : isLast ? "last" : isSingle ? "single" : "middle";
			if(!(tabEdges2 = QPixmapCache::find(position+"tabEdges2-k-sel"+QString::number(g.background().pixel(), 16)+QString::number(r.height())))) {
				tabEdges2 = new QPixmap(16, 8);
				tabEdges2->fill(background);
				QPainter painter2(tabEdges2);
				
				// parts of the unselected tabs under the rounded edges
				if(!isSingle) {
					if(!isFirst) {
						painter2.translate(0, -r.height()+12);
						renderSurface(&painter2, QRect(0, 0, 3, r.height()-5-3), g.background(), g.button(), g.button(), Is_TabBottom);
						painter2.resetXForm();
						painter2.drawTiledPixmap(0, 3, 3, 1, *tabHLines, 0, 2); // left
						painter2.setPen(alphaBlendColors(QColor(0,0,0), background, 79)); // bottomShadow1
						painter2.drawLine(0, 4, 1, 4);
						painter2.setPen(alphaBlendColors(QColor(0,0,0), background, 30)); // bottomShadow2
						painter2.drawPoint(2,4); // last point from bottomShadow1 a tad lighter (will be overpainted by the circle)
						painter2.drawLine(0, 5, 3, 5);
					}
					if(!isLast) {
						painter2.translate(0, -r.height()+12);
						renderSurface(&painter2, QRect(13, 0, 3, r.height()-5-3), g.background(), g.button(), g.button(), Is_TabBottom);
						painter2.resetXForm();
						painter2.drawTiledPixmap(13, 3, 3, 1, *tabHLines, 0, 2); // right
						painter2.setPen(alphaBlendColors(QColor(0,0,0), background, 79)); // bottomShadow1
						painter2.drawLine(14, 4, 15, 4);
						painter2.setPen(alphaBlendColors(QColor(0,0,0), background, 30)); // bottomShadow2
						painter2.drawPoint(13,4);
						painter2.drawLine(12, 5, 15, 5);
					}
				}
				// fill the rect under the gradients (faster as clipping the gradients)
				painter2.fillRect(QRect(0, 6, 16, 2), background);
				painter2.end();
				
				QPixmap tabEdgesMask = qembed_findImage("groupBoxMask"); // this fits because the tab frame is just an inverted groupbox frame, but pssst ;)
				painter2.begin(&tabEdgesMask);
				painter2.translate(0, -r.height()+19);
				renderSurface(&painter2, QRect(0, 0, 16, r.height()-3), g.background(), g.button(), g.button(), Is_ActiveTabBottom);
				painter2.end();
				bitBlt(tabEdges2, 0, 0, &tabEdgesMask, 0, 8);
				
				QPixmap circle = qembed_findImage("tabWidget");
				bitBlt(tabEdges2, 0, 0, &circle, 0, 8);
				
				QPixmapCache::insert(position+"tabEdges2-k-sel"+QString::number(g.background().pixel(), 16)+QString::number(r.height()), tabEdges2);
			}
			
			
			
			painter.setPen(alphaBlendColors(QColor(0,0,0), background, 79)); // bottomShadow1
			painter.drawLine(8, r.height()-3, r.width()-9, r.height()-3);
			painter.setPen(alphaBlendColors(QColor(0,0,0), background, 30)); // bottomShadow2
			painter.drawLine(8, r.height()-2, r.width()-9, r.height()-2);
			
			painter.setPen(alphaBlendColors(QColor(0,0,0), background, 35));
			painter.translate(0,3);
			if(isFirst || isSingle)
				painter.drawLine(0, -1, 0, r.height()-13); // left
			else
				renderTabSeparator(&painter, QRect(0, 0, 1, r.height()-5-3), g.button(), false);
			if(isLast || isSingle)
				painter.drawLine(r.width()-1, -1, r.width()-1, r.height()-13); // right
			else
				renderTabSeparator(&painter, QRect(r.width()-1, 0, 1, r.height()-5-3), g.button(), false);
			painter.translate(0,-3);
			
			bitBlt(&buffer, 0, r.height()-9, tabEdges2, 0, 0, 8, 8);
			bitBlt(&buffer, r.width()-8, r.height()-9, tabEdges2, 8, 0, 8, 8);
			
			painter.drawTiledPixmap(8, r.height()-4, r.width()-16, 1, *tabHLines, 0, 2);
			painter.drawTiledPixmap(1, 2, 1, r.height()-11, *tabVLines); // left
			painter.drawTiledPixmap(r.width()-2, 2, 1, r.height()-11, *tabVLines); // right
			
			
			
			painter.setPen(alphaBlendColors(QColor(0,0,0), g.background(), 20));
			painter.drawPoint(0,1);
			painter.drawPoint(r.width()-1,1);
			painter.setPen(alphaBlendColors(QColor(0,0,0), g.background(), 50));
			painter.drawPoint(0,2);
			painter.drawPoint(r.width()-1,2);
			//QColor tmp = alphaBlendColors(QColor(255,255,255), g.background(), 14);
			QColor tmp = g.background();
			painter.setPen(alphaBlendColors(tmp, activeTabBottomSurface.g1Top == 0 ? activeTabBottomSurface.g1Color1 : activeTabBottomSurface.background, 127));
// 			painter.drawPoint(1,2);
// 			painter.drawPoint(r.width()-2,2);
			painter.drawLine(1,2, r.width()-2, 2);
			
			// under the tab
			painter.setPen(background);
			painter.drawLine(0, r.height()-1, r.width(), r.height()-1);
			
		}
		else { // unselected
			
			painter.setPen(g.background());// 0. tabbarline
			painter.drawLine(0, 0, r.width(), 0);
			painter.setPen(g.background().dark(120));//QColor(110,110,110) // 1. tabbarline
			painter.drawLine(0, 1, r.width(), 1);
			painter.setPen(g.background().dark(konqTabBarContrast+20)); //QColor(110,110,110)
			
			
			// topline
// 			painter.setPen(g.background().dark(150-15));
			painter.setPen(g.background().dark(150-15));
			painter.drawLine(0, 2, r.width(), 2);
			
			
			if(isFirst || isLast) {
				
				painter.translate(0, 3);
				renderSurface(&painter, QRect(1, 0, r.width()-2, r.height()-5-3), g.background(), g.button(), g.button(), Is_TabBottom);
				painter.translate(0, -3);
				
				
				QPixmap* tabEdges2;
				if(!(tabEdges2 = QPixmapCache::find("tabEdges2-k"+QString::number(g.background().pixel(), 16)+QString::number(r.height())))) {
					tabEdges2 = new QPixmap(16, 8);
					tabEdges2->fill(background);
				
					QPixmap tabEdgesMask = qembed_findImage("groupBoxMask");
					QPainter painter2(&tabEdgesMask);
					painter2.translate(0, -r.height()+19+3);
					renderSurface(&painter2, QRect(1, 0, 14, r.height()-5-3), g.background(), g.button(), g.button(), Is_TabBottom);
					painter2.end();
					bitBlt(tabEdges2, 0, 0, &tabEdgesMask, 0, 8);
				
					QPixmap circle = qembed_findImage("tabWidget");
					bitBlt(tabEdges2, 0, 0, &circle, 0, 8);
				
					QPixmapCache::insert("tabEdges2-k"+QString::number(g.background().pixel(), 16)+QString::number(r.height()), tabEdges2);
				}
				
				
				if(isFirst) {
					
					painter.translate(0,3);
					renderTabSeparator(&painter, QRect(r.width()-1, 0, 1, r.height()-5-3), g.button(), false);// right
					painter.translate(0,-3);
					
					painter.setPen(alphaBlendColors(QColor(0,0,0), background, 79)); // bottomShadow1
					painter.drawLine(8, r.height()-5, r.width()-1, r.height()-5);
					painter.setPen(alphaBlendColors(QColor(0,0,0), background, 30)); // bottomShadow2
					painter.drawLine(8, r.height()-4, r.width()-1, r.height()-4);
					
					painter.setPen(alphaBlendColors(QColor(0,0,0), background, 35));
					painter.drawLine(0, 2, 0, r.height()-11); // left
					
					
					bitBlt(&buffer, 0, r.height()-11, tabEdges2, 0, 0, 8, 8);
					
					painter.drawTiledPixmap(8, r.height()-6, r.width()-8, 1, *tabHLines, 0, 2); // bottom
					painter.drawTiledPixmap(1, 3, 1, r.height()-14, *tabVLines); // left
					
				}
				else {
					
					painter.translate(0,3);
					renderTabSeparator(&painter, QRect(0, 0, 1, r.height()-5-3), g.button(), false); // left
					painter.translate(0,-3);
					
					painter.setPen(alphaBlendColors(QColor(0,0,0), background, 79)); // bottomShadow1
					painter.drawLine(0, r.height()-5, r.width()-8, r.height()-5);
					painter.setPen(alphaBlendColors(QColor(0,0,0), background, 30)); // bottomShadow2
					painter.drawLine(0, r.height()-4, r.width()-8, r.height()-4);
						
					painter.setPen(alphaBlendColors(QColor(0,0,0), background, 35));
					painter.drawLine(r.width()-1, 2, r.width()-1, r.height()-11); // right
					
					
					bitBlt(&buffer, r.width()-8, r.height()-11, tabEdges2, 8, 0, 8, 8);
					
					painter.drawTiledPixmap(0, r.height()-6, r.width()-8, 1, *tabHLines, 0, 2); // bottom
					painter.drawTiledPixmap(r.width()-2, 3, 1, r.height()-14, *tabVLines); // right
				}
			}
			else { // middle
				
				painter.translate(0,3);
				renderTabSeparator(&painter, QRect(0, 0, 1, r.height()-5-3), g.button(), false); // left
				renderTabSeparator(&painter, QRect(r.width()-1, 0, 1, r.height()-5-3), g.button(), false);// right
				painter.translate(0,-3);
				
				painter.translate(0, 3);
				renderSurface(&painter, QRect(1, 0, r.width()-2, r.height()-5-3), g.background(), g.button(), g.button(), Is_TabBottom);
				painter.translate(0, -3);
				
				painter.setPen(alphaBlendColors(QColor(0,0,0), background, 79)); // bottomShadow1
				painter.drawLine(0, r.height()-5, r.width()-1, r.height()-5);
				painter.setPen(alphaBlendColors(QColor(0,0,0), background, 30)); // bottomShadow2
				painter.drawLine(0, r.height()-4, r.width()-1, r.height()-4);
				
				painter.drawTiledPixmap(0, r.height()-6, r.width(), 1, *tabHLines, 0, 2); // bottom
				
			}
			painter.fillRect(QRect(0, r.height()-3, r.width(), 3), background);
		}
		p->drawPixmap(r.left(), r.top(), buffer);
	}
	else { // standard tabs
		
		if(bottom) {
			if (selected) {
	
				renderSurface(&painter, QRect(1, 0, r.width()-2, r.height()-2), g.background(), g.button(), g.button(), Is_ActiveTabBottom);
				
				
				QPixmap* tabEdges2;
				QString position = isFirst ? "first" : isLast ? "last" : isSingle ? "single" : "middle";
				if(!(tabEdges2 = QPixmapCache::find(position+"tabEdges2-bottom-sel"+QString::number(g.background().pixel(), 16)+QString::number(r.height())))) {
					tabEdges2 = new QPixmap(16, 8);
					tabEdges2->fill(g.background());
					QPainter painter2(tabEdges2);
					
					if(!isSingle) {
						if(!isFirst) {
							painter2.translate(0, -r.height()+8);
							renderSurface(&painter2, QRect(0, 0, 2, r.height()-5), g.background(), g.button(), g.button(), Is_TabBottom);
							painter2.resetXForm();
							painter2.drawTiledPixmap(0, 2, 2, 1, *tabHLines, 0, 2); // left
							painter2.setPen(alphaBlendColors(QColor(0,0,0), g.background(), 79));
							painter2.drawLine(0, 3, 1,3);
							painter2.setPen(alphaBlendColors(QColor(0,0,0), g.background(), 30));
							painter2.drawLine(0, 4, 3, 4);
						}
						if(!isLast) {
							
							painter2.translate(0, -r.height()+8);
							renderSurface(&painter2, QRect(14, 0, 2, r.height()-5), g.background(), g.button(), g.button(), Is_TabBottom);
							painter2.resetXForm();
							painter2.drawTiledPixmap(14, 2, 2, 1, *tabHLines, 0, 2); // right
							painter2.setPen(alphaBlendColors(QColor(0,0,0), g.background(), 79));
							painter2.drawLine(14, 3, 15, 3);
							painter2.setPen(alphaBlendColors(QColor(0,0,0), g.background(), 30));
							painter2.drawLine(12, 4, 15, 4);
						}
					}
					painter2.fillRect(QRect(0, 5, 16, 3), g.background());
					painter2.end();
					
				
					QPixmap tabEdgesMask = qembed_findImage("groupBoxMask");
					painter2.begin(&tabEdgesMask);
					painter2.translate(0, -r.height()+16);
					renderSurface(&painter2, QRect(1, 0, 14, r.height()-2), g.background(), g.button(), g.button(), Is_ActiveTabBottom);
					painter2.end();
					bitBlt(tabEdges2, 0, 0, &tabEdgesMask, 0, 8);
				
					QPixmap circle = qembed_findImage("tabWidget");
					bitBlt(tabEdges2, 0, 0, &circle, 0, 8);
				
					QPixmapCache::insert(position+"tabEdges2-bottom-sel"+QString::number(g.background().pixel(), 16)+QString::number(r.height()), tabEdges2);
				}
				
				
				painter.setPen(alphaBlendColors(QColor(0,0,0), g.background(), 79)); // bottomShadow1
				painter.drawLine(8, r.height()-2, r.width()-9, r.height()-2);
				painter.setPen(alphaBlendColors(QColor(0,0,0), g.background(), 30)); // bottomShadow2
				painter.drawLine(8, r.height()-1, r.width()-9, r.height()-1);
				
				painter.setPen(alphaBlendColors(QColor(0,0,0), g.background(), 35));
				if(isFirst || isSingle)
					painter.drawLine(0, 0, 0, r.height()-8); // left
				else
					renderTabSeparator(&painter, QRect(0, 0, 1, r.height()-5), g.button(), false);
				if(isLast || isSingle)
					painter.drawLine(r.width()-1, 0, r.width()-1, r.height()-8); // right
				else
					renderTabSeparator(&painter, QRect(r.width()-1, 0, 1, r.height()-5), g.button(), false);
				
				bitBlt(&buffer, 0, r.bottom()-7, tabEdges2, 0, 0, 8, 8);
				bitBlt(&buffer, r.width()-8, r.bottom()-7, tabEdges2, 8, 0, 8, 8);
				
				
				painter.drawTiledPixmap(8, r.height()-3, r.width()-16, 1, *tabHLines, 0, 2); // bottom
				painter.drawTiledPixmap(1, 0, 1, r.height()-8, *tabVLines); // left
				painter.drawTiledPixmap(r.width()-2, 0, 1, r.height()-8, *tabVLines); // right
				
				painter.setPen(alphaBlendColors(QColor(0,0,0), g.background(), 35)); // clip the first pixels of the separators
				painter.drawPoint(0, 0);
				painter.drawPoint(r.width()-1, 0);
				
			}
			else { // unselected
				
				renderSurface(&painter, QRect(1, 0, r.width()-2, r.height()-5), g.background(), g.button(), g.button(), Is_TabBottom);
				
				
				if(isFirst || isLast) {
					
					QPixmap* tabEdges2;
					if(!(tabEdges2 = QPixmapCache::find("tabEdges2-bottom"+QString::number(g.background().pixel(), 16)+QString::number(r.height())))) {
						tabEdges2 = new QPixmap(16, 8);
						tabEdges2->fill(g.background());
				
						QPixmap tabEdgesMask = qembed_findImage("groupBoxMask");
						QPainter painter2(&tabEdgesMask);
						painter2.translate(0, -r.height()+19);
						renderSurface(&painter2, QRect(1, 0, 14, r.height()-5), g.background(), g.button(), g.button(), Is_TabBottom);
						painter2.end();
						bitBlt(tabEdges2, 0, 0, &tabEdgesMask, 0, 8);
				
						QPixmap circle = qembed_findImage("tabWidget");
						bitBlt(tabEdges2, 0, 0, &circle, 0, 8);
				
						QPixmapCache::insert("tabEdges2-bottom"+QString::number(g.background().pixel(), 16)+QString::number(r.height()), tabEdges2);
					}
					
					
					if(isFirst) {
						renderTabSeparator(&painter, QRect(r.width()-1, 0, 1, r.height()-5), g.button(), false);// right
						
						painter.setPen(alphaBlendColors(QColor(0,0,0), g.background(), 79)); // bottomShadow1
						painter.drawLine(8, r.height()-5, r.width()-1, r.height()-5);
						painter.setPen(alphaBlendColors(QColor(0,0,0), g.background(), 30)); // bottomShadow2
						painter.drawLine(8, r.height()-4, r.width()-1, r.height()-4);
						
						painter.setPen(alphaBlendColors(QColor(0,0,0), g.background(), 35));
						painter.drawLine(0, 1, 0, r.height()-12); // left
						
						bitBlt(&buffer, 0, r.height()-11, tabEdges2, 0, 0, 8, 8);
						
						painter.drawTiledPixmap(1, 1, 1, r.height()-12, *tabVLines); // left
						painter.drawTiledPixmap(8, r.height()-6, r.width()-8, 1, *tabHLines, 0, 2); // bottom
						
						painter.setPen(alphaBlendColors(QColor(0,0,0), g.background(), 55));
						painter.drawLine(0, 0, r.width()-1, 0); // top
						
					}
					else {
						renderTabSeparator(&painter, QRect(0, 0, 1, r.height()-5), g.button(), false); // left
						
						painter.setPen(alphaBlendColors(QColor(0,0,0), g.background(), 79)); // bottomShadow1
						painter.drawLine(0, r.height()-5, r.width()-8, r.height()-5);
						painter.setPen(alphaBlendColors(QColor(0,0,0), g.background(), 30)); // bottomShadow2
						painter.drawLine(0, r.height()-4, r.width()-8, r.height()-4);
						
						painter.setPen(alphaBlendColors(QColor(0,0,0), g.background(), 35));
						painter.drawLine(r.width()-1, 1, r.width()-1, r.height()-12); // right
						
						bitBlt(&buffer, r.width()-8, r.height()-11, tabEdges2, 8, 0, 8, 8);
						
						painter.drawTiledPixmap(r.width()-2, 1, 1, r.height()-12, *tabVLines); // right
						painter.drawTiledPixmap(0, r.height()-6, r.width()-8, 1, *tabHLines, 0, 2); // bottom
						
						painter.setPen(alphaBlendColors(QColor(0,0,0), g.background(), 55));
						painter.drawLine(0, 0, r.width()-1, 0); // top
					}
				}
				else { // middle
					renderTabSeparator(&painter, QRect(0, 0, 1, r.height()-5), g.button(), false); // left
					renderTabSeparator(&painter, QRect(r.width()-1, 0, 1, r.height()-5), g.button(), false);// right
					
					painter.setPen(alphaBlendColors(QColor(0,0,0), g.background(), 79)); // bottomShadow1
					painter.drawLine(0, r.height()-5, r.width()-1, r.height()-5);
					painter.setPen(alphaBlendColors(QColor(0,0,0), g.background(), 30)); // bottomShadow2
					painter.drawLine(0, r.height()-4, r.width()-1, r.height()-4);
					painter.drawTiledPixmap(0, r.height()-6, r.width(), 1, *tabHLines, 0, 2); // bottom
					
					painter.setPen(alphaBlendColors(QColor(0,0,0), g.background(), 55));
					painter.drawLine(0, 0, r.width()-1, 0); // top
				}
				
				painter.fillRect(QRect(0, r.height()-3, r.width(), 3), g.background()); // rect under the tab, clips also the tabSeperator
				
			}
		}
		else { // above
			if (selected) {
				
				renderSurface(&painter, QRect(1, 1, r.width()-2, r.height()), g.background(), g.button(), g.button(), Is_ActiveTabTop);
				
				QPixmap* tabEdges2;
				QString position = isFirst ? "first" : isLast ? "last" : isSingle ? "single" : "middle";
				if(!(tabEdges2 = QPixmapCache::find(position+"tabEdges2-top-sel"+QString::number(g.background().pixel(), 16)+QString::number(r.height())))) {
					tabEdges2 = new QPixmap(16,8);
					tabEdges2->fill(g.background());
					QPainter painter2(tabEdges2);
					
					if(!isSingle) {
						painter2.setPen(alphaBlendColors(QColor(0,0,0), g.background(), 25));
						if(!isFirst) {
							renderSurface(&painter2, QRect(0, 4, 2, r.height()), g.background(), g.button(), g.button(), Is_TabTop);
							painter2.drawLine(0, 3, 1, 3); // topLine
							painter2.drawTiledPixmap(0, 4, 2, 2, *tabHLines);
						}
						if(!isLast) {
							renderSurface(&painter2, QRect(14, 4, 2, r.height()), g.background(), g.button(), g.button(), Is_TabTop);
							painter2.drawLine(14, 3, 15, 3); // topLine
							painter2.drawTiledPixmap(14, 4, 2, 2, *tabHLines);
						}
					}
					painter2.end();
					
					QPixmap tabEdgesMask = qembed_findImage("groupBoxMask");
					painter2.begin(&tabEdgesMask);
					renderSurface(&painter2, QRect(1, 1, 14, r.height()), g.background(), g.button(), g.button(), Is_ActiveTabTop);
					painter2.end();
					bitBlt(tabEdges2, 0, 0, &tabEdgesMask);
				
					QPixmap circle = qembed_findImage("tabWidget");
					bitBlt(tabEdges2, 0, 0, &circle);
				
					QPixmapCache::insert(position+"tabEdges2-top-sel"+QString::number(g.background().pixel(), 16)+QString::number(r.height()), tabEdges2);
				}
				
				///////////////////////////////////////////////
				painter.setPen(alphaBlendColors(QColor(0,0,0), g.background(), 35));
				///painter.setPen(alphaBlendColors(QColor(0,0,0), g.background(), 40));
				if(isFirst || isSingle)
					painter.drawLine(0, 8, 0, r.height()-1); // left
				else
					renderTabSeparator(&painter, QRect(0, 4, 1, r.height()), g.button(), true);
				if(isLast || isSingle)
					painter.drawLine(r.width()-1, 8, r.width()-1, r.height()-1); // right
				else
					renderTabSeparator(&painter, QRect(r.width()-1, 4, 1, r.height()), g.button(), true);
				
				painter.setPen(alphaBlendColors(QColor(0,0,0), g.background(), 25));
				painter.drawLine(8, 0, r.width()-9, 0); // top
				
				
				bitBlt(&buffer, 0, 0, tabEdges2, 0, 0, 8, 8, Qt::CopyROP);
				bitBlt(&buffer, r.width()-8, 0, tabEdges2, 8, 0, 8, 8, Qt::CopyROP);
				
				painter.drawTiledPixmap(8, 1, r.width()-16, 2, *tabHLines); // top
				painter.drawTiledPixmap(1, 8, 1, r.height()-1, *tabVLines); // left
				painter.drawTiledPixmap(r.width()-2, 8, 1, r.height()-1, *tabVLines); // right
				
				painter.setPen(alphaBlendColors(QColor(0,0,0), g.background(), 35)); // clip the last pixels of the separators
				painter.drawPoint(0, r.height()-1);
				painter.drawPoint(r.width()-1, r.height()-1);
				
				
			}
			else { // unselected
				
				painter.fillRect(0, 0, r.width(), 10, g.background());
				painter.setPen(g.background().dark(150));
				
				renderSurface(&painter, QRect(1, 4, r.width()-2, r.height()), g.background(), g.button(), g.button(), Is_TabTop);
				
				if(isFirst || isLast) {
					
					QPixmap* tabEdges2;
					if(!(tabEdges2 = QPixmapCache::find("tabEdges2-top"+QString::number(g.background().pixel(), 16)+QString::number(r.height())))) {
						tabEdges2 = new QPixmap(16, 8);
						tabEdges2->fill(g.background());
				
						QPixmap tabEdgesMask = qembed_findImage("groupBoxMask");
						QPainter painter2(&tabEdgesMask);
						renderSurface(&painter2, QRect(1, 1, r.width()-2, r.height()), g.background(), g.button(), g.button(), Is_TabTop);
						painter2.end();
						bitBlt(tabEdges2, 0, 0, &tabEdgesMask);
				
						QPixmap circle = qembed_findImage("tabWidget");
						bitBlt(tabEdges2, 0, 0, &circle);
				
						QPixmapCache::insert("tabEdges2-top"+QString::number(g.background().pixel(), 16)+QString::number(r.height()), tabEdges2);
					}
					
					
					
					if(isFirst) {
						// darker separator
						renderTabSeparator(&painter, QRect(r.width()-1, 4, 1, r.height()), g.button(), true); // left
						
						painter.setPen(alphaBlendColors(QColor(0,0,0), g.background(), 25));
						painter.drawLine(8, 3, r.width()-1, 3); // topLine
						
						painter.setPen(alphaBlendColors(QColor(0,0,0), g.background(), 35));
						painter.drawLine(0, 10, 0, r.height()-1); // left
						painter.drawLine(1, r.height()-1, r.width()-1, r.height()-1); // bottom
						
						painter.drawTiledPixmap(8, 4, r.width()-1, 2, *tabHLines); // top
						painter.drawTiledPixmap(1, 11, 1, r.height()-12, *tabVLines); // left
						
						bitBlt(&buffer, 0, 3, tabEdges2, 0, 0, 8, 8);

					}
					else {
						// darker separator
						renderTabSeparator(&painter, QRect(0, 4, 1, r.height()), g.button(), true); // right
						
						painter.setPen(alphaBlendColors(QColor(0,0,0), g.background(), 25));
						painter.drawLine(0, 3, r.width()-9, 3); // topLine
						
						painter.setPen(alphaBlendColors(QColor(0,0,0), g.background(), 35));
						painter.drawLine(r.width()-1, 10, r.width()-1, r.height()-1); // right
						painter.drawLine(0, r.height()-1, r.width()-2, r.height()-1); // bottom
						
						painter.drawTiledPixmap(0, 4, r.width()-8, 2, *tabHLines); // top
						painter.drawTiledPixmap(r.width()-2, 11, 1, r.height()-12, *tabVLines); // right
						
						bitBlt(&buffer, r.width()-8, 3, tabEdges2, 8, 0, 8, 8);
					}
					
				}
				else {  // middle
					
					renderTabSeparator(&painter, QRect(r.width()-1, 4, 1, r.height()), g.button(), true); // left
					renderTabSeparator(&painter, QRect(0, 4, 1, r.height()), g.button(), true); // right
					
					painter.setPen(alphaBlendColors(QColor(0,0,0), g.background(), 25));
					painter.drawLine(0, 3, r.width()-1, 3); // top
					painter.drawTiledPixmap(0, 4, r.width(), 2, *tabHLines);
					
					painter.setPen(alphaBlendColors(QColor(0,0,0), g.background(), 35));
					painter.drawLine(0, r.height()-1, r.width()-1, r.height()-1); // bottom
						
				}
			}
		}
		p->drawPixmap(r.left(), r.top(), buffer);
	}
}

void DominoStyle::drawKStylePrimitive(KStylePrimitive kpe,
									  QPainter *p,
									  const QWidget* widget,
									  const QRect &r,
									  const QColorGroup &cg,
									  SFlags flags,
									  const QStyleOption& opt) const
{
	//return KStyle::drawKStylePrimitive(kpe, p, widget, r, cg, flags, opt);

//	SLIDER
//	------
	switch( kpe ) {
		case KPE_SliderGroove: {
			
			bool horizontal = ((const QSlider*)widget)->orientation() == Horizontal;
			
			QRect re;
			if(horizontal)
				re.setRect(r.left(), r.y()+r.height()/2-3, r.width(), 4);
			else
				re.setRect(r.x()+r.width()/2-3, r.top(), 4, r.height());
			
			bool parentBgPix = widget->parentWidget() ? widget->parentWidget()->paletteBackgroundPixmap() : 0;
			QString hori = horizontal ? "1" : "0";
			QString trans = parentBgPix ? "1" : "0";
			QPixmap* sliderGroove;
			if(!(sliderGroove = QPixmapCache::find(QString::number(cg.background().pixel(), 16)+"sliderGroove"+hori+trans))) {

				if(parentBgPix) { // ### the pix are transparent!
					if(!horizontal) {
						QWMatrix m;
						m.rotate(-90);
						sliderGroove = new QPixmap(qembed_findImage("slGHBody").xForm(m));
					}
					else
						sliderGroove = new QPixmap(qembed_findImage("slGHBody"));
				}
				else {
					const QColor c1 = alphaBlendColors(QColor(0,0,0), cg.background(), 11);
					const QColor c2 = alphaBlendColors(QColor(53,53,53), cg.background(), 155);
					const QColor c3 = alphaBlendColors(QColor(67,67,67), cg.background(), 146);
					const QColor c4 = alphaBlendColors(QColor(83,83,83), cg.background(), 140);
					const QColor c5 = alphaBlendColors(QColor(101,101,101), cg.background(), 134);
					const QColor c6 = alphaBlendColors(QColor(254,254,254), cg.background(), 43);
					const QColor c7 = alphaBlendColors(QColor(255,255,255), cg.background(), 9);
					
					sliderGroove = new QPixmap(7, 7);
					QPainter painter(sliderGroove);
					QRect rr = sliderGroove->rect();
					
					if (horizontal) {
						painter.setPen(c1);
						painter.drawLine(0, 0, 7, 0);
						painter.setPen(c2);
						painter.drawLine(0, 1, 7, 1);
						painter.setPen(c3);
						painter.drawLine(0, 2, 7, 2);
						painter.setPen(c4);
						painter.drawLine(0, 3, 7, 3);
						painter.setPen(c5);
						painter.drawLine(0, 4, 7, 4);
						painter.setPen(c6);
						painter.drawLine(0, 5, 7, 5);
						painter.setPen(c7);
						painter.drawLine(0, 6, 7, 6);
					}
					else {
						painter.setPen(c1);
						painter.drawLine(0, 0, 0, 7);
						painter.setPen(c2);
						painter.drawLine(1, 0, 1 , 7);
						painter.setPen(c3);
						painter.drawLine(2, 0, 2 , 7);
						painter.setPen(c4);
						painter.drawLine(3, 0, 3, 7);
						painter.setPen(c5);
						painter.drawLine(4, 0, 4, 7);
						painter.setPen(c6);
						painter.drawLine(5, 0, 5, 7);
						painter.setPen(c7);
						painter.drawLine(6, 0, 6, 7);
					}
				}
				QPixmapCache::insert(QString::number(cg.background().pixel(), 16)+"sliderGroove"+hori+trans, sliderGroove);
			}
			
			if (horizontal) {
				QPixmap* slGh;
				if(parentBgPix || !(slGh = QPixmapCache::find(QString::number(cg.background().pixel(), 16)+"slGh"))) { // rounded edges
					
					if(parentBgPix)
						slGh = new QPixmap(qembed_findImage("slGh"));
					else {
						slGh = new QPixmap(6, 7);
						slGh->fill(cg.background());
						QPixmap pix = qembed_findImage("slGh");
						bitBlt(slGh, 0, 0, &pix, 0, 0, -1, -1, Qt::CopyROP);
					}
					QPixmapCache::insert(QString::number(cg.background().pixel(), 16)+"slGh",slGh );
				}
				
				bitBlt(p->device(), re.x(), re.y(), slGh, 0, 0, 3, 7, Qt::CopyROP);
				bitBlt(p->device(), r.width()-3, re.y(), slGh, 3, 0, 3, 7, Qt::CopyROP);
				p->drawTiledPixmap(re.x()+3, re.y(), r.width()-6, 7, *sliderGroove);
			}
			else {
				QPixmap* slGv;
				if(parentBgPix || !(slGv = QPixmapCache::find(QString::number(cg.background().pixel(), 16)+"slGv"))) {
					if(parentBgPix)
						slGv = new QPixmap(qembed_findImage("slGv"));
					else {
						slGv = new QPixmap(7, 6);
						slGv->fill(cg.background());
						QPixmap pix = qembed_findImage("slGv");
						bitBlt(slGv, 0, 0, &pix, 0, 0, -1, -1, Qt::CopyROP);
					}
					QPixmapCache::insert(QString::number(cg.background().pixel(), 16)+"slGv",slGv );
				}
				bitBlt(p->device(), re.x(), re.y(), slGv, 0, 0, 7, 3, Qt::CopyROP);
				bitBlt(p->device(), re.x(), re.height()-3, slGv, 0, 3, 7, 3, Qt::CopyROP);
				p->drawTiledPixmap(re.x(), re.y()+3, 7, r.height()-6, *sliderGroove);
			}
			
			break;
		}

		case KPE_SliderHandle: {
			const QSlider* slider = (const QSlider*)widget;
			
			int y, x;
			if(slider->orientation() == Horizontal) {
				x = r.x();
				y =r.y()+r.height()/2-8+1;
			}
			else {
				y = r.y()+1;
				x = r.x()+r.width()/2-8;
			}
			
			QPixmap* handle;
			if(!(handle = QPixmapCache::find("sliderHandle"+QString::number(cg.button().pixel(), 16)))) {
				QPixmap insidePix =  qembed_findImage("sliderHandle2inside"); // a filled antialiased circle to draw the gradients on
				QPainter painter(&insidePix);
				flatMode=true;
				renderSurface(&painter, QRect(1, 0, 13, 15), cg.background(), cg.button(), cg.button(), Is_Button|Is_Horizontal);
				flatMode=false;
				QImage inside = insidePix.convertToImage();
				
				QImage circle = tintImage(qembed_findImage("sliderHandle2circle"),
						_customSliderHandleContourColor ? buttonContour->contourColor(Contour_Default) : alphaBlendColors(black, cg.button(), 110));
				
				QImage shadow =  qembed_findImage("sliderHandle2shadow");
				blend(shadow, inside, inside);
				blend(circle, inside, inside);
				handle = new QPixmap(inside);
				QPixmapCache::insert("sliderHandle"+QString::number(cg.button().pixel(), 16), handle );
			}
			bitBlt(p->device(), x, y, handle, 0, 0, -1, -1, Qt::CopyROP);
			
			break;
		}

		case KPE_ListViewExpander: {
			
			QWidget* w = dynamic_cast<QWidget*>(p->device());
			p->setPen( cg.text() );
			QColor bg = w ? w->paletteBackgroundColor() : cg.background();
			QString hasBgPix = w && w->paletteBackgroundPixmap() ? "1" : "0";
			QString expanded = flags & Style_Off ? "1" : "0";
			QPixmap* lve;
			
			if(_drawTriangularExpander) {
				
				if(!(lve = QPixmapCache::find(QString::number(bg.pixel(), 16)+QString::number(cg.text().pixel(), 16)+"lveT"+hasBgPix+expanded))) {
					QWMatrix m;
					m.rotate(90);
					if(w && !w->paletteBackgroundPixmap()) {
						QImage img = tintImage(qembed_findImage("lveTriangular"), cg.text());
						int w=7, h=9;
						if(flags & Style_Off) {
							w=9, h=7;
							img = img.xForm(m);
						}
						QImage imgBG(w,h,32);
						imgBG.fill(bg.rgb());
						bitBlt(&imgBG, 0, 0, &img);
						lve = new QPixmap(imgBG);
					}
					else {
						if(flags & Style_Off)
							lve = new QPixmap(tintImage(qembed_findImage("lveTriangular").xForm(m), cg.text()));
						else
							lve = new QPixmap(tintImage(qembed_findImage("lveTriangular"), cg.text()));
					}
					QPixmapCache::insert(QString::number(bg.pixel(), 16)+QString::number(cg.text().pixel(), 16)+"lveT"+hasBgPix+expanded, lve);
				}
				
				p->drawPixmap(r.x(), r.y(), *lve);

			}
			else {
				
				if(!(lve = QPixmapCache::find(QString::number(bg.pixel(), 16)+QString::number(cg.text().pixel(), 16)+"lveC"+hasBgPix+expanded))) {
					if(w && !w->paletteBackgroundPixmap()) {
						lve = new QPixmap(11,11);
						lve->fill(bg);
						QPainter painter(lve);
						painter.drawPixmap(0, 0, tintImage(qembed_findImage("lve"), cg.text()));
						
						// plus or minus
						painter.drawLine( 3, 5, 7, 5 );
						if ( flags & Style_On ) // Collapsed = On
							painter.drawLine( 5, 3, 5, 7 );
						
						QPixmapCache::insert(QString::number(bg.pixel(), 16)+QString::number(cg.text().pixel(), 16)+"lveC"+hasBgPix+expanded, lve);
					}
					else {
						QImage img = QImage(11, 11, 32);
						img.setAlphaBuffer(true);
						for(int x=0; x < img.width(); ++x) {
							for(int y=0; y < img.height(); ++y) {
								img.setPixel(x, y, qRgba(0, 0, 0, 0));
							}
						}
						
						
						QImage img2 = tintImage(qembed_findImage("lve"), cg.text());
						bitBlt(&img, 0, 0, &img2);
						
						int r, b, g;
						int a = 255;
						cg.text().getRgb(&r, &g, &b);
						
						// minus
						img.setPixel(3, 5, qRgba(r, b, g, a));
						img.setPixel(4, 5, qRgba(r, b, g, a));
						img.setPixel(5, 5, qRgba(r, b, g, a));
						img.setPixel(6, 5, qRgba(r, b, g, a));
						img.setPixel(7, 5, qRgba(r, b, g, a));
						
						
						if ( flags & Style_Off )
							lve = new QPixmap(img);
						QPixmapCache::insert(QString::number(bg.pixel(), 16)+QString::number(cg.text().pixel(), 16)+"lveC"+hasBgPix+"1", new QPixmap(img));
						
						// plus
						img.setPixel(5, 3, qRgba(r, b, g, a));
						img.setPixel(5, 4, qRgba(r, b, g, a));
						img.setPixel(5, 6, qRgba(r, b, g, a));
						img.setPixel(5, 7, qRgba(r, b, g, a));
						
						if ( flags & Style_On )
							lve = new QPixmap(img);
						
						QPixmapCache::insert(QString::number(bg.pixel(), 16)+QString::number(cg.text().pixel(), 16)+"lveC"+hasBgPix+"0", new QPixmap(img));
					}
				}
				p->drawPixmap(r.x()-1,r.y()-1, *lve);
			}
			break;
		}

		// copied and slightly modified from KStyle.
		case KPE_ListViewBranch: {
			if(_drawTriangularExpander) break;
			// Typical Windows style listview branch element (dotted line).
			
			// Create the dotline pixmaps if not already created
			if ( !verticalLine )
			{
				// make 128*1 and 1*128 bitmaps that can be used for
				// drawing the right sort of lines.
				verticalLine   = new QBitmap( 1, 129, true );
				horizontalLine = new QBitmap( 128, 1, true );
				QPointArray a( 64 );
				QPainter p2;
				p2.begin( verticalLine );
	
				int i;
				for( i=0; i < 64; i++ )
					a.setPoint( i, 0, i*2+1 );
				p2.setPen( color1 );
				p2.drawPoints( a );
				p2.end();
				QApplication::flushX();
				verticalLine->setMask( *verticalLine );
	
				p2.begin( horizontalLine );
				for( i=0; i < 64; i++ )
					a.setPoint( i, i*2+1, 0 );
				p2.setPen( color1 );
				p2.drawPoints( a );
				p2.end();
				QApplication::flushX();
				horizontalLine->setMask( *horizontalLine );
			}
	
			p->setPen( cg.mid() );
	
			if (flags & Style_Horizontal)
			{
				int point = r.x();
				int other = r.y();
				int end = r.x()+r.width();
				int thickness = r.height();
	
				while( point < end )
				{
					int i = 128;
					if ( i+point > end )
						i = end-point;
					p->drawPixmap( point, other, *horizontalLine, 0, 0, i, thickness );
					point += i;
				}
	
			} else {
				int point = r.y();
				int other = r.x();
				int end = r.y()+r.height();
				int thickness = r.width();
				int pixmapoffset = (flags & Style_NoChange) ? 0 : 1;	// ### Hackish
	
				while( point < end )
				{
					int i = 128;
					if ( i+point > end )
						i = end-point;
					p->drawPixmap( other, point, *verticalLine, 0, pixmapoffset, thickness, i );
					point += i;
				}
			}
			break;
		}
		
		default:
			KStyle::drawKStylePrimitive(kpe, p, widget, r, cg, flags, opt);
	}
}


void DominoStyle::drawPrimitive(PrimitiveElement pe,
								QPainter *p,
								const QRect &r,
								const QColorGroup &cg,
								SFlags flags,
								const QStyleOption &opt ) const
{
	bool down	= flags & Style_Down;
	bool on		= flags & Style_On;
	bool sunken = flags & Style_Sunken;
	bool horiz	= flags & Style_Horizontal;
	const bool enabled = flags & Style_Enabled;
	const bool mouseOver = flags & Style_MouseOver;

//	bool hasFocus = flags & Style_HasFocus;

	int x = r.x();
	int y = r.y();
	int w = r.width();
	int h = r.height();


	
	//return KStyle::drawPrimitive(pe, p, r, cg, flags, opt);
	
	switch(pe) {

		case PE_FocusRect: {
			
			if(rubberBandType == DistRubber || (rubberBandType == ArgbRubber && !compositeManagerRunning)) {
				p->drawWinFocusRect( r );
				break;
			}
			
			QWidget* w =  dynamic_cast<QWidget*>(p->device());
			if(rubberBandType == ArgbRubber && compositeManagerRunning && viewPortPressed && dynamic_cast<QListView*>(w->parentWidget())) {
				QListView* lv = dynamic_cast<QListView*>(w->parentWidget());
				if(!lv->currentItem())
					break;
				QRect sr = lv->itemRect(lv->currentItem());
				QRect rr = r.normalize();
				int step = lv->rootIsDecorated() ? (lv->currentItem()->depth()+1)*lv->treeStepSize() : lv->currentItem()->depth()*lv->treeStepSize();
				if(dynamic_cast<QCheckListItem*>(lv->currentItem()))
					step += 16+lv->itemMargin()+4;
				
				// why is the focusrect misused as a rubberband!?
				if((lv->selectionMode() == QListView::Multi || lv->selectionMode() == QListView::Extended) && 
								!((sr.y() == rr.y()) && (sr.height() == rr.height()) && (sr.x()+step == rr.x()))) {
					
					QScrollView* sv = static_cast<QScrollView*>(w);
					
					QPoint po = w->mapFrom(sv, r.topLeft());
					QRect re = QRect(po.x(), po.y(), r.width(), r.height());
					
					drawPrimitive(PE_RubberBand, p, r, cg, flags, opt);
					break;
				}
			}
			if(ignoreNextFocusRect) {
				ignoreNextFocusRect= false;
				break;
			}
			
			QColor color = alphaBlendColors(cg.highlight(), cg.foreground(), 160);
			
			if(w && (w->isA("QViewportWidget") || dynamic_cast<QWidget*>(p->device())->isA("RegionGrabber"))) { // QListView, QListBox, KSnapshot...
				// we can't be sure if the pixamp edges will be erased, so only a simple rect.
				p->setPen(QPen(color, 1, Qt::SolidLine));
				p->drawRect(r);
				break;
			}
			
			QPixmap* edges;
			if(!(edges = QPixmapCache::find("fre"+QString::number(color.pixel(), 16)))) {
				edges = new QPixmap(tintImage(qembed_findImage("focusRectEdges"), color));
				QPixmapCache::insert("fre"+QString::number(color.pixel(), 16), edges);
			}
			
			p->drawPixmap(r.left(), r.top(), *edges, 0, 0, 3, 3);
			p->drawPixmap(r.left(), r.bottom()-2, *edges, 0, 3, 3, 3);
			p->drawPixmap(r.right()-2, r.top(), *edges, 3, 0, 3, 3);
			p->drawPixmap(r.right()-2, r.bottom()-2, *edges, 3, 3, 3, 3);
			
			p->setPen(color);
			p->drawLine(r.left()+3, r.top(), r.right()-2, r.y());
			p->drawLine(r.left()+3, r.bottom(), r.right()-2, r.bottom());
			p->drawLine(r.left(), r.top()+3, r.left(), r.bottom()-2);
			p->drawLine(r.right(), r.top()+3, r.right(), r.bottom()-2);
			
			break;
		}
		
		case PE_HeaderSection: { // also taskbar buttons
			if(dynamic_cast<QWidget*>(p->device()) && dynamic_cast<QWidget*>(p->device())->isA("PanelKMenu")) {
				QWidget* w = dynamic_cast<QWidget*>(p->device());
				
				if(w->parentWidget() && w->parentWidget()->paletteBackgroundPixmap()) {
					
					QPixmap pix(w->width(), w->height());
					QPainter painter(&pix);
					QPoint point = QPoint(w->geometry().x(), w->geometry().y());
					painter.drawTiledPixmap( 0, 0, w->width(), w->height(),
							*w->parentWidget()->backgroundPixmap(),
							point.x(), point.y() );
					// if the PopupTitle is hovered by the mouse, we don't have access to the parent background pixmap,
					// so it must have have its own one.
					w->setUpdatesEnabled(false);
					w->setErasePixmap(pix);
					w->setUpdatesEnabled(true);
					w->erase();
				}
				else
					w->erase(r);
				break;
			}
			
			renderHeader(p, r, cg, (on||down||sunken), mouseOver, true, enabled );
			break;
		}
		
		
	// BUTTONS
	// -------
		
		case PE_ButtonTool:
		case PE_ButtonDropDown:
		case PE_ButtonBevel:
		case PE_ButtonCommand: {
			
			// the above promitives are not called from within the style
			if(dynamic_cast<QPixmap*>(p->device())) // more likely a backgroundPixmap than a doublebuffer
				buttonContour->setAlphaMode();
			
			renderButton(p, r, cg, (on||down), mouseOver, true, enabled, khtmlWidgets.contains(opt.widget()) );
			buttonContour->reset();
			break;
		}
		
		case PE_ButtonDefault: {
			break;
		}

		case PE_SpinWidgetPlus:
		case PE_SpinWidgetMinus: {
			p->setPen( cg.buttonText() );

			int l = QMIN( w-2, h-2 );
			// make the length even so that we get a nice symmetric plus...
			if(l%2 != 0)
				--l;
			QPoint c = r.center();

			p->drawLine( c.x()-l/2, c.y(), c.x()+l/2, c.y() );
			if ( pe == PE_SpinWidgetPlus ) {
				p->drawLine( c.x(), c.y()-l/2, c.x(), c.y()+l/2 );
			}
			break;
		}

		case PE_ScrollBarSlider: {
			
			QColor buttonColor = cg.button();
			
			QPixmap buffer;
			if(horiz)
				buffer.resize(r.width()-14, r.height());
			else
				buffer.resize(r.width(), r.height()-14);
			if(buffer.isNull()) break;
			
			QRect br = buffer.rect();
			QPainter painter(&buffer);

			if(horiz) {
				uint surfaceFlags = Is_ScrollBarBtn|Is_Horizontal;
				renderSurface(&painter, br, cg.button(), cg.button(), buttonColor, surfaceFlags);

				uint contourFlags = Is_ScrollBarBtn|Is_Horizontal|Draw_Top|Draw_Bottom;
				renderContour(&painter, QRect(-2, 0, br.width()+4, br.height()), cg.button(), buttonColor, contourFlags);

				p->drawPixmap(r.left()+7, r.y(), buffer);
			}
			else {
				uint surfaceFlags = Is_ScrollBarBtn;
				renderSurface(&painter, br, cg.button(), cg.button(), buttonColor, surfaceFlags);
				
				uint contourFlags = Is_ScrollBarBtn|Draw_Left|Draw_Right;
				renderContour(&painter, QRect(0, -2, br.width(), br.height()+4), cg.button(), buttonColor, contourFlags);

				bitBlt(p->device(), r.x(), r.y()+7, &buffer, 0, 0, -1, -1, Qt::CopyROP);
			}
			break;
		}

		case PE_ScrollBarAddPage: { // bottom / right
			
			QColor buttonColor = cg.button();
			QColor contourColor = alphaBlendColors(buttonContour->contourColor(Contour_Default), buttonColor, 203);
			QPixmap* addPageBtn;
			QPixmap buffer;
			
			uint surfaceFlags = Is_ScrollBarBtn;
			if(horiz)
				surfaceFlags |= Is_Horizontal;
			
			int sl = 6;
			if(sliderlen < 14)
				sl = sliderlen/2-1;
			
			//// button part
			if(horiz) {
				buffer.resize(r.width()+sl, r.height());
				if(buffer.isNull()) break;
				
				if(!(addPageBtn = QPixmapCache::find(QString::number(buttonColor.pixel(), 16)+"ah"+configMode ))) {

					QPixmap gradientPix = qembed_findImage("scrollBtnHMask");
					QPainter painter(&gradientPix);
					renderSurface(&painter, QRect(0, 0, gradientPix.width(), gradientPix.height()), buttonColor, buttonColor, buttonColor, surfaceFlags);
					QImage img1 = gradientPix.convertToImage();
					QImage img2 = qembed_findImage("scrollBtnHShadow");
					QImage img3 = tintImage(qembed_findImage("scrollBtnHContour"), contourColor);
					
					blend(img2, img1, img1);
					blend(img3, img1, img1);
					
					addPageBtn = new QPixmap(img1);
					
					QPixmapCache::insert(QString::number(buttonColor.pixel(), 16)+"ah"+configMode, addPageBtn);
				}
			}
			else {
				buffer.resize(r.width(), r.height()+sl);
				if(buffer.isNull()) break;
				if(!(addPageBtn = QPixmapCache::find(QString::number(buttonColor.pixel(), 16)+"av"+configMode ))) {

					QPixmap gradientPix = qembed_findImage("scrollBtnVMask");
					QPainter painter(&gradientPix);
					renderSurface(&painter, QRect(0, 0, gradientPix.width(), gradientPix.height()), buttonColor, buttonColor, buttonColor, surfaceFlags);
					QImage img1 = gradientPix.convertToImage();
					QImage img2 = qembed_findImage("scrollBtnVShadow");
					QImage img3 = tintImage(qembed_findImage("scrollBtnVContour"), contourColor);
					
					blend(img2, img1, img1);
					blend(img3, img1, img1);
					
					addPageBtn = new QPixmap(img1);
					
					QPixmapCache::insert(QString::number(buttonColor.pixel(), 16)+"av"+configMode, addPageBtn);
				}
			}
			
			QRect br(buffer.rect() );
			QPainter bp(&buffer);
			
			//// groove
			if(sliderlen >= 10) {
				surfaceFlags = Is_ScrollBarGroove;
				if(horiz)
					surfaceFlags |= Is_Horizontal;
				
				QColor grooveLine = alphaBlendColors(black, buttonColor, 80);
				renderSurface(&bp, QRect(0, 0, br.width(), br.height()), grooveLine, grooveLine, grooveLine, surfaceFlags);
				bp.setClipRegion(horiz ? QRect(0, 1, br.width(), br.height()-2) : QRect(1, 0, br.width()-2, br.height()));
				renderSurface(&bp, QRect(0, 0, br.width(), br.height()), buttonColor, buttonColor, buttonColor, surfaceFlags);
				bp.setClipping(false);
			}
			else {
				renderSurface(&bp, QRect(0, 0, br.width(), br.height()), buttonColor, buttonColor, buttonColor, surfaceFlags);
				uint contourFlags = Is_ScrollBarBtn;
				contourFlags |= (horiz)? Draw_Top|Draw_Bottom : Draw_Left|Draw_Right;
				renderContour(&bp, horiz?QRect(-2, 0, br.width()+4, br.height()):QRect(0, -2, br.width(), br.height()+4), buttonColor, buttonColor, contourFlags);
			}
			
			
			
			QString docked = (!horiz && r.height() == 1) || (horiz && r.width() == 1) ? "1" : "0";
			QPixmap* sliderCap;
			if(horiz) {
				if(!(sliderCap = QPixmapCache::find("sHCap"+QString::number(buttonColor.pixel(), 16)+docked+configMode))) {
					if(r.width() == 1) {
						QPixmap gradientPix(16, 15);
						QPainter painter(&gradientPix);
						renderSurface(&painter, QRect(0, 0, gradientPix.width(), gradientPix.height()), cg.button(), cg.button(), buttonColor, Is_ScrollBarBtn|Is_Horizontal);
						QImage img1 = gradientPix.convertToImage();
						
						QImage img2 = qembed_findImage("scrollSliderHDockedShadow");
						QImage img3 = tintImage(qembed_findImage("scrollSliderHDockedContour"), contourColor);
						
						bitBlt(&img1, 0, 0, &img2);
						bitBlt(&img1, 0, 0, &img3);
						
						sliderCap = new QPixmap(img1);
					}
					else {
						QPixmap gradientPix = qembed_findImage("scrollSliderHMask");
						QPainter painter(&gradientPix);
						
						renderSurface(&painter, QRect(0, 0, gradientPix.width(), gradientPix.height()), cg.button(), cg.button(), buttonColor, Is_ScrollBarBtn|Is_Horizontal);
						QImage img1 = gradientPix.convertToImage();
						QImage img2 = qembed_findImage("scrollSliderHShadow");
						QImage img3 = tintImage(qembed_findImage("scrollSliderHContour"), contourColor);
						
						blend(img2, img1, img1);
						blend(img3, img1, img1);
						
						sliderCap = new QPixmap(img1);
					}
					QPixmapCache::insert("sHCap"+QString::number(buttonColor.pixel(), 16)+docked+configMode, sliderCap);
				}
			}
			else {
				if(!(sliderCap = QPixmapCache::find("sVCap"+QString::number(buttonColor.pixel(), 16)+docked+configMode))) {
					if(r.height() == 1) {
						QPixmap gradientPix(15, 16);
						QPainter painter(&gradientPix);
						renderSurface(&painter, QRect(0, 0, gradientPix.width(), gradientPix.height()), cg.button(), cg.button(), buttonColor, Is_ScrollBarBtn);
						QImage img1 = gradientPix.convertToImage();
						
						QImage img2 = qembed_findImage("scrollSliderVDockedShadow");
						QImage img3 = tintImage(qembed_findImage("scrollSliderVDockedContour"), contourColor);
						
						bitBlt(&img1, 0, 0, &img2);
						bitBlt(&img1, 0, 0, &img3);
						
						sliderCap = new QPixmap(img1);
					}
					else {
						QPixmap gradientPix = qembed_findImage("scrollSliderVMask");
						QPainter painter(&gradientPix);
						
						renderSurface(&painter, QRect(0, 0, gradientPix.width(), gradientPix.height()), cg.button(), cg.button(), buttonColor, Is_ScrollBarBtn);
						QImage img1 = gradientPix.convertToImage();
						QImage img2 = qembed_findImage("scrollSliderVShadow");
						QImage img3 = tintImage(qembed_findImage("scrollSliderVContour"), contourColor);
						
						blend(img2, img1, img1);
						blend(img3, img1, img1);
						
						sliderCap = new QPixmap(img1);
					}
					QPixmapCache::insert("sVCap"+QString::number(buttonColor.pixel(), 16)+docked+configMode, sliderCap);
				}
			}
			
			
			////////////
			
			if(horiz) {
				if(sliderlen >= 10) {
					bp.drawPixmap(br.right()-8, 0, *addPageBtn, 9, 0, 9, 15);
					bp.drawPixmap(-6+sl, 0, *sliderCap, 8, 0, 8, 15);
				}
				bitBlt(p->device(), r.x()-sl, r.y(), &buffer, 0, 0, br.width(), br.height(), Qt::CopyROP);
			}
			else {
				if(sliderlen >= 10) {
					bp.drawPixmap(0, br.bottom()-8, *addPageBtn, 0, 9, 15, 9);
					bp.drawPixmap(0, (br.height() <= 6)? br.bottom()-6 :br.top(), *sliderCap, 0, 8, 15, 8);
				}
				bitBlt(p->device(), r.x(), r.y()-sl, &buffer, 0, 0, br.width(), br.height(), Qt::CopyROP);
			}
			break;
		}

		case PE_ScrollBarSubPage: { // top / left
			
			QColor buttonColor = cg.button();
			QColor contourColor = alphaBlendColors(buttonContour->contourColor(Contour_Default), buttonColor, 203);
			QPixmap* subPageBtn;
			QPixmap buffer;

			uint surfaceFlags = Is_ScrollBarBtn;
			
			if(horiz)
				surfaceFlags |= Is_Horizontal;
			
			
			int sl = 6;
			if(sliderlen < 14)
				sl = sliderlen/2;
			
			
			//// button part
			if(horiz) {
				buffer.resize(r.width()+sl, r.height());
				if(buffer.isNull()) break;
				
				if(!(subPageBtn = QPixmapCache::find(QString::number(buttonColor.pixel(), 16)+"sh"+configMode ))) {

					QPixmap gradientPix = qembed_findImage("scrollBtnHMask");
					QPainter painter(&gradientPix);
					renderSurface(&painter, QRect(0, 0, gradientPix.width(), gradientPix.height()), buttonColor, buttonColor, buttonColor, surfaceFlags);
					QImage img1 = gradientPix.convertToImage();
					QImage img2 = qembed_findImage("scrollBtnHShadow");
					QImage img3 = tintImage(qembed_findImage("scrollBtnHContour"), contourColor);
					
					blend(img2, img1, img1);
					blend(img3, img1, img1);
					
					subPageBtn = new QPixmap(img1);
					
					QPixmapCache::insert(QString::number(buttonColor.pixel(), 16)+"sh"+configMode, subPageBtn);
				}
			}
			else {
				buffer.resize(r.width(), r.height()+sl);
				if(buffer.isNull()) break;
				
				if(!(subPageBtn = QPixmapCache::find(QString::number(buttonColor.pixel(), 16)+"sv"+configMode ))) {

					QPixmap gradientPix = qembed_findImage("scrollBtnVMask");
					QPainter painter(&gradientPix);
					renderSurface(&painter, QRect(0, 0, gradientPix.width(), gradientPix.height()), buttonColor, buttonColor, buttonColor, surfaceFlags);
					QImage img1 = gradientPix.convertToImage();
					QImage img2 = qembed_findImage("scrollBtnVShadow");
					QImage img3 = tintImage(qembed_findImage("scrollBtnVContour"), contourColor);
					
					blend(img2, img1, img1);
					blend(img3, img1, img1);
					
					subPageBtn = new QPixmap(img1);

					QPixmapCache::insert(QString::number(buttonColor.pixel(), 16)+"sv"+configMode, subPageBtn);
				}
			}

			QRect br(buffer.rect());
			QPainter bp(&buffer);
			
			//// groove
			if(sliderlen >= 10) {
				surfaceFlags = Is_ScrollBarGroove;
				if(horiz)
					surfaceFlags |= Is_Horizontal;
				
				QColor grooveLine = alphaBlendColors(black, buttonColor, 80);
				renderSurface(&bp, QRect(0, 0, br.width(), br.height()), grooveLine, grooveLine, grooveLine, surfaceFlags);
				bp.setClipRegion(horiz ? QRect(0, 1, br.width(), br.height()-2) : QRect(1, 0, br.width()-2, br.height()));
				renderSurface(&bp, QRect(0, 0, br.width(), br.height()), buttonColor, buttonColor, buttonColor, surfaceFlags);
				bp.setClipping(false);
			}
			else {
				renderSurface(&bp, QRect(0, 0, br.width(), br.height()), buttonColor, buttonColor, buttonColor, surfaceFlags);
				uint contourFlags = Is_ScrollBarBtn;
				contourFlags |= (horiz)? Draw_Top|Draw_Bottom : Draw_Left|Draw_Right;
				renderContour(&bp, horiz?QRect(-2, 0, br.width()+4, br.height()):QRect(0, -2, br.width(), br.height()+4), buttonColor, buttonColor, contourFlags);
			}
			
			
			QString docked = (!horiz && r.bottom() == 15) || (horiz && r.right() == 15) ? "1" : "0";
			QPixmap* sliderCap;
			if(horiz) {
				if(!(sliderCap = QPixmapCache::find("sHCap"+QString::number(buttonColor.pixel(), 16)+docked+configMode))) {
					
					if(r.right() == 15) {
						QPixmap gradientPix(16, 15);
						QPainter painter(&gradientPix);
						renderSurface(&painter, QRect(0, 0, gradientPix.width(), gradientPix.height()), cg.button(), cg.button(), buttonColor, Is_ScrollBarBtn|Is_Horizontal);
						QImage img1 = gradientPix.convertToImage();
						QImage img2 = qembed_findImage("scrollSliderHDockedShadow");
						QImage img3 = tintImage(qembed_findImage("scrollSliderHDockedContour"), contourColor);
						
						bitBlt(&img1, 0, 0, &img2);
						bitBlt(&img1, 0, 0, &img3);
						
						sliderCap = new QPixmap(img1);
					}
					else {
						QPixmap gradientPix = qembed_findImage("scrollSliderHMask");
						QPainter painter(&gradientPix);
						renderSurface(&painter, QRect(0, 0, gradientPix.width(), gradientPix.height()), cg.button(), cg.button(), buttonColor, Is_ScrollBarBtn|Is_Horizontal);
						QImage img1 = gradientPix.convertToImage();
						QImage img2 = qembed_findImage("scrollSliderHShadow");
						QImage img3 = tintImage(qembed_findImage("scrollSliderHContour"), contourColor);
						
						blend(img2, img1, img1);
						blend(img3, img1, img1);
						
						sliderCap = new QPixmap(img1);
					}
					
					QPixmapCache::insert("sHCap"+QString::number(buttonColor.pixel(), 16)+docked+configMode, sliderCap);
				}
			}
			else {
				if(!(sliderCap = QPixmapCache::find("sVCap"+QString::number(buttonColor.pixel(), 16)+docked+configMode))) {
					
					if(r.bottom() == 15) {
						QPixmap gradientPix(15, 16);
						QPainter painter(&gradientPix);
						renderSurface(&painter, QRect(0, 0, gradientPix.width(), gradientPix.height()), cg.button(), cg.button(), buttonColor, Is_ScrollBarBtn);
						QImage img1 = gradientPix.convertToImage();
						
						QImage img2 = qembed_findImage("scrollSliderVDockedShadow");
						QImage img3 = tintImage(qembed_findImage("scrollSliderVDockedContour"), contourColor);
						
						bitBlt(&img1, 0, 0, &img2);
						bitBlt(&img1, 0, 0, &img3);
						
						sliderCap = new QPixmap(img1);
					}
					else {
						QPixmap gradientPix = qembed_findImage("scrollSliderVMask");
						QPainter painter(&gradientPix);
						
						renderSurface(&painter, QRect(0, 0, gradientPix.width(), gradientPix.height()), cg.button(), cg.button(), buttonColor, Is_ScrollBarBtn);
						QImage img1 = gradientPix.convertToImage();
						QImage img2 = qembed_findImage("scrollSliderVShadow");
						QImage img3 = tintImage(qembed_findImage("scrollSliderVContour"), contourColor);
						
						blend(img2, img1, img1);
						blend(img3, img1, img1);
						
						sliderCap = new QPixmap(img1);
					}
					
					
					QPixmapCache::insert("sVCap"+QString::number(buttonColor.pixel(), 16)+docked+configMode, sliderCap);
				}
			}
			
			if(horiz) {
				if(sliderlen >= 10) {
					bp.drawPixmap(0, 0, *subPageBtn, 0, 0, 9, 15);
					bp.drawPixmap((br.width() <= 6)? -2 : br.width()-8, 0, *sliderCap, 0, 0, 8, 15);
				}
			}
			else {
				if(sliderlen >= 10) {
					bp.drawPixmap(0, 0, *subPageBtn, 0, 0, 15, 9);
					bp.drawPixmap(0, br.bottom()-sl-2+1, *sliderCap, 0, 0, 15, 8);
				}
			}
			
			bitBlt(p->device(), r.x(), r.y(), &buffer, 0, 0, br.width(), br.height(), Qt::CopyROP);
			break;
		}

	// SCROLLBAR BUTTONS
	// -----------------

		case PE_ScrollBarSubLine: { // top / left button
			
			uint contourFlags = Is_ScrollBarBtn;
			
			QRect cr = r;
			QRect ar = r;
			QRect sr = r;
			
			if(horiz) {
				contourFlags |= Is_Horizontal|Draw_Top|Draw_Bottom;
				if(r.x() == 0) { // = single Button
					sr.setX(2);
					sr.setWidth(13);
					ar.setX(3);
					contourFlags |= Draw_Left|Round_BottomLeft|Round_UpperLeft;
				}
				else {
					cr.setWidth(17);
					cr.setX(cr.x()-2);
				}
			}
			else {
				ar.setX(1);
				contourFlags |= Draw_Left|Draw_Right;
				if(r.y() == 0) {
					sr.setY(2);
					sr.setHeight(13);
					ar.setY(3);
					contourFlags |= Draw_Top|Round_UpperLeft|Round_UpperRight;
				}
				else {
					cr.setHeight(17);
					cr.setY(cr.y()-2);
				}
			}
			
			if(horiz) {
				renderSurface(p, QRect(sr.x(), sr.y(), sr.width(), sr.height()), cg.button(), cg.button(), cg.button(), Is_ScrollBarBtn|Is_Horizontal);
				renderContour(p, QRect(cr.x(), cr.y(), cr.width(), cr.height()), cg.button(), cg.button(), contourFlags);
			}
			else {
				p->setClipRegion(QRect(sr.x()+2, sr.y(), sr.width()-4, sr.height()));
				renderSurface(p, QRect(sr.x(), sr.y(), sr.width(), sr.height()), cg.button(), cg.button(), cg.button(), Is_ScrollBarBtn);
				p->setClipping(false);
				renderContour(p, QRect(cr.x(), cr.y(), cr.width(), cr.height()), cg.button(), cg.button(), contourFlags);
			}
			
			QColor color = dynamic_cast<QWidget*>(p->device()) ? dynamic_cast<QWidget*>(p->device())->palette().active().buttonText() : cg.buttonText();
			p->setPen(color);
			drawPrimitive((horiz ? PE_ArrowLeft : PE_ArrowUp), p, ar, cg, flags);

			break;
		}

		case PE_ScrollBarAddLine: { // bottom / right button
			
			QRect cr = r;
			QRect ar = r;
			QRect sr = r;
			if(horiz) {
				sr.setWidth(13);
				ar.setX(r.x()-4);
			}
			else {
				sr.setHeight(13);
				ar.setY(r.y()-2);
				ar.setX(r.x()+1);
			}

			if(horiz) {
				uint contourFlags = Is_ScrollBarBtn|Is_Horizontal|Draw_Top|Draw_Bottom|Draw_Right|Round_UpperRight|Round_BottomRight;
				renderSurface(p, QRect(sr.x(), sr.y(), sr.width(), sr.height()), cg.button(), cg.button(), cg.button(), Is_ScrollBarBtn|Is_Horizontal);
				renderContour(p, QRect(cr.x(), cr.y(), cr.width(), cr.height()), cg.button(), cg.button(), contourFlags);
			}
			else {
				p->setClipRegion(QRect(sr.x()+2, sr.y(), sr.width()-4, sr.height()));
				renderSurface(p, QRect(sr.x(), sr.y(), sr.width(), sr.height()), cg.button(), cg.button(), cg.button(), Is_ScrollBarBtn);
				p->setClipping(false);
				uint contourFlags = Is_ScrollBarBtn|Draw_Bottom|Draw_Left|Draw_Right|Round_BottomLeft|Round_BottomRight;
				renderContour(p, QRect(cr.x(), cr.y(), cr.width(), cr.height()), cg.button(), cg.button(), contourFlags);
			}
			
			QColor color = dynamic_cast<QWidget*>(p->device()) ? dynamic_cast<QWidget*>(p->device())->palette().active().buttonText() : cg.buttonText();
			p->setPen(color);
			drawPrimitive((horiz ? PE_ArrowRight : PE_ArrowDown), p, ar, cg, flags);
			
			
			break;
		}
		
		case PE_ScrollBarSubLine2: { // second top / left button // ### remove
			
			uint contourFlags = Is_ScrollBarBtn;
			
			QRect cr = r;
			QRect ar = r;
			QRect sr = r;
			
			if(horiz) {
				contourFlags |= Is_Horizontal|Draw_Top|Draw_Bottom;
			}
			else {
				ar.setX(1);
				contourFlags |= Draw_Left|Draw_Right;
			}
			
			if(horiz) {
				renderSurface(p, QRect(sr.x(), sr.y(), sr.width(), sr.height()), cg.button(), cg.button(), cg.button(), Is_ScrollBarBtn|Is_Horizontal);
				renderContour(p, QRect(cr.x(), cr.y(), cr.width(), cr.height()), cg.button(), cg.button(), contourFlags);
			}
			else {
				p->setClipRegion(QRect(sr.x()+2, sr.y(), sr.width()-4, sr.height()));
				renderSurface(p, QRect(sr.x(), sr.y(), sr.width(), sr.height()), cg.button(), cg.button(), cg.button(), Is_ScrollBarBtn);
				p->setClipping(false);
				renderContour(p, QRect(cr.x(), cr.y(), cr.width(), cr.height()), cg.button(), cg.button(), contourFlags);
			}
			
			QColor color = dynamic_cast<QWidget*>(p->device()) ? dynamic_cast<QWidget*>(p->device())->palette().active().buttonText() : cg.buttonText();
			p->setPen(color);
			drawPrimitive((horiz ? PE_ArrowLeft : PE_ArrowUp), p, ar, cg, flags);
			
			break;
		}

	// CHECKBOXES
	// ----------
		case PE_Indicator: {
			break;
		}

		case PE_IndicatorMask: {
			p->fillRect (r.x()+3, r.y()+3, r.width()-6, r.height()-7, color1);
			break;
		}

	// RADIOBUTTONS
	// ------------
		case PE_ExclusiveIndicator: {
			break;
		}

		case PE_ExclusiveIndicatorMask: {

			QPointArray pa(8);
			pa.setPoint(0, 3,6);
			pa.setPoint(1, 6,3);
			pa.setPoint(2, 10,3);
			pa.setPoint(3, 13,6);
			pa.setPoint(4, 13,10);
			pa.setPoint(5, 10,13);
			pa.setPoint(6, 6,13);
			pa.setPoint(7, 3,10);

			p->setBrush(color1);
			p->setPen(color1);
			p->drawPolygon(pa);
			break;
		}

	// GENERAL PANELS
	// --------------
		case PE_Splitter: {
			
			p->fillRect(r, cg.background());
			int ch, cs, cv;
			cg.background().getHsv(ch, cs, cv);
			if(cv < 100)
				p->setPen(alphaBlendColors(cg.background(), white, 120));
			else
				p->setPen(alphaBlendColors(cg.background(), black, 180));

			p->setPen(dGetColor(cg.background(), 55));
			if (w > h) {
				int ycenter = r.height()/2;
				for(int k =r.width()/2-5; k < r.width()/2+5; k+=3) {
					p->drawRect(k, ycenter-1, 2,2);
				}
			}
			else {
				int xcenter = r.width()/2;
				for(int k =r.height()/2-5; k < r.height()/2+5; k+=3) {
					p->drawRect( xcenter-1, k, 2,2);
				}
			}
			
			break;
		}

		case PE_GroupBoxFrame:
		case PE_PanelGroupBox: { // QFrame::GroupBoxPanel
			break;
		}

		case PE_WindowFrame: {
			if ( opt.isDefault() || opt.lineWidth() <= 0 )
				break;
			
			p->setPen(cg.background().dark(120));
			p->drawRect(r);
			p->setPen(cg.background().light(110));
			p->drawRect(QRect(r.x()+1, r.y()+1, r.width()-2, r.height()-2));
			
			break;
		}

		case PE_Panel: { // QFrame::StyledPanel
			if(kickerMode)
				break;
			
			p->save();
			// inside rect
			p->setPen(cg.background().dark(120));
			p->drawLine(r.x()+1, r.y()+1, r.right()-1, r.y()+1); //top
			p->drawLine(r.x()+1, r.bottom()-1, r.right()-1, r.bottom()-1);
			p->drawLine(r.x()+1, r.y()+1, r.x()+1, r.bottom()-1);
			p->drawLine(r.right()-1, r.y()+1, r.right()-1, r.bottom()-1);
			
			// outside rect
			p->setPen(cg.background());
			p->drawLine(r.x(), r.y(), r.right(), r.y()); //top
			p->setPen(alphaBlendColors(white, cg.background(), 30));
			p->drawLine(r.x(), r.y()+1, r.x(), r.bottom());
			p->drawLine(r.right(), r.y()+1, r.right(), r.bottom());
			p->setPen(alphaBlendColors(white, cg.background(), 50));
			p->drawLine(r.x()+1, r.bottom(), r.right()-1, r.bottom());
			
			p->restore(); // maybe we are a KArrowButton and need to draw the arrow
			break;
		}

		case PE_PanelLineEdit: { // QFrame::LineEditPanel
			break;
		}

		case PE_PanelTabWidget: { // QFrame::TabWidgetPanel
			
			// we get a wrong clip rect
			p->setClipping(false);
			
			bool roundTopLeft = true;
			bool roundTopRight = true;
			bool roundBottomLeft = true;
			bool roundBottomRight = true;
			bool tabsAbove = true;
			
// 			p->fillRect(QRect(r.x()-10, r.y()-10, r.width()+20, r.height()+20), yellow);
			
			QRect tr;
			QWidget* w = dynamic_cast<QWidget*>(p->device());
			QTabWidget* tw = w ? dynamic_cast<QTabWidget*>(w->parentWidget()) : 0;
			bool tabBarHidden = tw && !((DominoQTabWidget*)tw)->tabBar()->isVisible();
			if(tw && !tabBarHidden) {
				
				tabsAbove = tw->tabPosition() == QTabWidget::Top;
				
				if(tw->count() != 0) {
					
					QTabBar* tb = ((DominoQTabWidget*)tw)->tabBar();
					QRect gr = tb->geometry();
					
					if(tb->isVisible()) {
						tr = tb->tabAt(tw->currentPageIndex())->rect();
						tr.setHeight(3);
						tr.moveBy(gr.x(), tabsAbove ? 0 : r.height()-2);
					}
					
					if(!tw->cornerWidget(Qt::TopLeft) && gr.x() < 7) {
						if(tabsAbove)
							roundTopLeft = false;
						else
							roundBottomLeft = false;
					}
					if(!tw->cornerWidget(Qt::TopRight) && gr.x()+gr.width() > tw->width()-7) {
						if(tabsAbove)
							roundTopRight = false;
						else
							roundBottomRight = false;
					}
				}
			}
			QBrush background = tw ? tw->colorGroup().brush(QColorGroup::Background) : cg.brush(QColorGroup::Background);
			QColor bottomGradient = alphaBlendColors(QColor(0,0,0), cg.background(), 11);
			QColor topGradient = alphaBlendColors(QColor(255,255,255), cg.background(), 15);
			QColor tabContour2 = cg.background().dark(150);
			
			
			QString tabPos = tabsAbove ? "1" : "0";
			QPixmap* tabWEdges;
			if(!(tabWEdges = QPixmapCache::find("tabWEdges"+tabPos+QString::number(background.color().pixel(), 16)+QString::number(cg.background().pixel(), 16)))) {
				tabWEdges = new QPixmap(16,16);
				QPainter painter(tabWEdges);
				painter.fillRect(tabWEdges->rect(), background);
				painter.end();
				
				QPixmap tabWEdgesMask = qembed_findImage("groupBoxMask");
				painter.begin(&tabWEdgesMask);
				painter.fillRect(tabWEdgesMask.rect(), cg.brush(QColorGroup::Background));
				painter.end();
				bitBlt(tabWEdges, 0, 0, &tabWEdgesMask);
				
				QPixmap circle = qembed_findImage("tabWidget");
				bitBlt(tabWEdges, 0, 0, &circle);
				
				QPixmapCache::insert("tabWEdges"+tabPos+QString::number(background.color().pixel(), 16)+QString::number(cg.background().pixel(), 16), tabWEdges);
			}
			
			
			QRect re = QRect(r.x(), r.y(), r.width(), r.height()+1);
			QColor bg = cg.background();
			QColor bg2 = background.color(); // parent color
			
			// top
			QColor c1 = alphaBlendColors(QColor(0,0,0), bg2, 25);
			QColor c2 = alphaBlendColors(QColor(255,255,255), bg, 70);
			QColor c3 = alphaBlendColors(QColor(255,255,255), bg, 30);
				
			// left + right
			QColor c4 = alphaBlendColors(QColor(0,0,0), bg2, 40);
// 			QColor c5 = alphaBlendColors(QColor(255,255,255), bg, 30);
			QColor c6 = alphaBlendColors(QColor(255,255,255), bg, 4);
				
			// bottom
			QColor c7 = alphaBlendColors(QColor(249,249,249), bg, 25);
			QColor c8 = alphaBlendColors(QColor(0,0,0), bg2, 79);
			QColor c9 = alphaBlendColors(QColor(0,0,0), bg2, 30);
			
			// top
			p->setPen(c1);
			p->drawLine(re.left(), re.y(), re.right(), re.y());
			p->setPen(c2);
			p->drawLine(re.left(), re.y()+1, re.right(), re.y()+1);
			p->setPen(c3);
			p->drawLine(re.left(), re.y()+2, re.right(), re.y()+2);
			// bottom
			p->setPen(c7);
			p->drawLine(re.left()+1, re.bottom()-2, re.right(), re.bottom()-2);
			p->setPen(c8);
			p->drawLine(re.left()+1, re.bottom()-1, re.right(), re.bottom()-1);
			p->setPen(c9);
			p->drawLine(re.left(), re.bottom(), re.right(), re.bottom());
			// left
			p->setPen(c4);
			p->drawLine(re.left(), re.y()+1, re.left(), re.bottom()-1);
			p->setPen(c3);
			p->drawLine(re.left()+1, re.y()+2, re.left()+1, re.bottom()-2);
			p->setPen(c6);
			p->drawLine(re.left()+2, re.y()+3, re.left()+2, re.bottom()-3);
			// right
			p->drawLine(re.right()-2, re.y()+3, re.right()-2, re.bottom()-3);
			p->setPen(c3);
			p->drawLine(re.right()-1, re.y()+2, re.right()-1, re.bottom()-2);
			p->setPen(c4);
			p->drawLine(re.right(), re.y()+1, re.right(), re.bottom()-1);
			
			
			// unpainted area
			p->setPen(bg);
			p->drawLine(re.left()+3, re.y()+3, re.right()-3, re.y()+3);
			p->drawLine(re.left()+3, re.bottom()-4, re.right()-3, re.bottom()-4);
			p->drawLine(re.left()+3, re.bottom()-3, re.right()-3, re.bottom()-3);
			p->drawLine(re.left()+3, re.y()+3, re.left()+3, re.bottom()-4);
			p->drawLine(re.right()-3, re.y()+3, re.right()-3, re.bottom()-4);
			
			
			if(roundTopLeft) {
				bitBlt(p->device(), re.x(), re.y(), tabWEdges, 0, 0, 8, 8, Qt::CopyROP);
			}
			if(roundTopRight) {
				bitBlt(p->device(), re.width()-8, re.y(), tabWEdges, 8, 0, 8, 8, Qt::CopyROP);
			}
			if(roundBottomLeft) {
				bitBlt(p->device(), re.x(), re.height()-8, tabWEdges, 0, 8, 8, 8, Qt::CopyROP);
			}
			if(roundBottomRight) {
				bitBlt(p->device(), re.width()-8, re.height()-8, tabWEdges, 8, 8, 8, 8, Qt::CopyROP);
			}
			
			
			// paint over the frame to not disturb the transition from the tabWidget to the current tab
			if(tr.isValid()) {
				p->fillRect(tr, cg.background()); // ### shrink
				
				if(tabsAbove) {
					QColor p1 = alphaBlendColors(QColor(255,255,255), bg, 50);
					QColor p2 = alphaBlendColors(QColor(255,255,255), bg, 25);
					QColor p3 = alphaBlendColors(QColor(255,255,255), bg, 10);
					
					p->setPen(p1);
					p->drawPoint(tr.left(), tr.top()+1);
					p->drawPoint(tr.right(), tr.top()+1);
					p->setPen(p2);
					p->drawPoint(tr.left()+1, tr.top()+1);
					p->drawPoint(tr.left(), tr.top()+2);
					p->drawPoint(tr.right()-1, tr.top()+1);
					p->drawPoint(tr.right(), tr.top()+2);
					p->setPen(p3);
					p->drawPoint(tr.left()+1, tr.top()+2);
					p->drawPoint(tr.right()-1, tr.top()+2);
				}
				else {
					QColor p1 = alphaBlendColors(QColor(255,255,255), bg, 17);
					QColor p2 = alphaBlendColors(QColor(255,255,255), bg, 10);
					
					p->setPen(p1);
					p->drawPoint(tr.left(), tr.top());
					p->drawPoint(tr.right(), tr.top());
					
					p->setPen(p2);
					p->drawPoint(tr.left()+1, tr.top());
					p->drawPoint(tr.right()-1, tr.top());
					
				}
				
			}
			
			break;
		}

		case PE_PanelPopup: { // QFrame::PopupPanel
			
			QPopupMenu* pm = dynamic_cast<QPopupMenu*>(p->device());
			QColor background = dynamic_cast<QListBox*>(p->device()) ? cg.base() : _customPopupMenuColor ? _popupMenuColor : cg.background();
			
			QRegion mask(x, y+5, w, h-10);
			mask += QRegion(x+5, y, w-10, h);
			mask += QRegion(x+1, y+2, w-2, h-4);
			mask += QRegion(x+2, y+1, w-4, h-2);
			
			if(_drawPopupMenuGradient) {
				QPixmap buffer(r.width(), r.height());
				QPainter bp(&buffer);
				renderGradient(&bp, r, lightenColor(background, 18), darkenColor(background, 12), true);
			
				if(pm) {
					pm->setUpdatesEnabled(false);
					pm->setPaletteBackgroundPixmap(buffer);
					pm->setUpdatesEnabled(true);
				}
				else {
					p->setClipRegion(mask);
					p->drawPixmap(r, buffer);
					p->setClipping(false);
				}
			}
			else {
				if(pm) {
					pm->setUpdatesEnabled(false);
					pm->setPaletteBackgroundColor(background);
					pm->setUpdatesEnabled(true);
				}
			}
			
			if(pm) {
				if(pm->isA("PrettyPopupMenu") || pm->isA("Amarok::Menu") || pm->isA("Digikam::DPopupMenu")) {
					// these popups have a side pixmap and are drawing the frame after the contents
					QRect cr = pm->contentsRect();
					if(QApplication::reverseLayout())
						pm->erase(mask-QRegion(cr.width()+3, cr.y(), r.width()-cr.width()-6, cr.height()-1));
					else
						pm->erase(mask-QRegion(r.x()+3, cr.y(), r.width()-cr.width()-6, cr.height()-1));
					((DominoQFrame*)pm)->drawContents(p );
				}
				else
					pm->erase(mask);
			}
			
			
			bitBlt(p->device(), r.x(), r.y(), popupFrame, 0, 0, 5, 5, Qt::CopyROP);
			bitBlt(p->device(), r.width()-5, r.y(), popupFrame, 5, 0, 5, 5, Qt::CopyROP);
			bitBlt(p->device(), r.x(), r.bottom()-4, popupFrame, 0, 5, 5, 5, Qt::CopyROP);
			bitBlt(p->device(), r.width()-5, r.bottom()-4, popupFrame, 5, 5, 5, 5, Qt::CopyROP);
			
			p->setPen(background.dark(150));
			p->drawLine(r.x()+5, r.y(), r.width()-6, r.y());
			p->drawLine(r.x()+5, r.bottom(), r.width()-6, r.bottom());
			p->drawLine(r.x(), r.y()+5, r.x(), r.bottom()-5);
			p->drawLine(r.width()-1, r.y()+5, r.width()-1, r.bottom()-5);

			break;
		}

	// MENU / TOOLBAR PANEL
	// --------------------
		case PE_PanelMenuBar: // QFrame::MenuBarPanel
		case PE_PanelDockWindow: { // QFrame::ToolBarPanel

			p->setPen(cg.background());
			// overpainting the menus leads to flicker so we draw only the frame here.
			if(dynamic_cast<QWidget*>(p->device()))
				p->drawRect(r);
			else
				p->fillRect(r, cg.brush(QColorGroup::Background)); // colorpreview in kcontrol
			break;
		}

		case PE_StatusBarSection: {
			break;
		}

		case PE_TabBarBase: {
			break;
		}
		
	// TOOLBAR/DOCK WINDOW HANDLE
	// --------------------------
		case PE_DockWindowResizeHandle: {
			QWidget* w = dynamic_cast<QWidget*>(p->device()); // QDockWindowResizeHandle
			
			if(w && !w->erasePixmap()) {
				QPixmap pix(r.size());
				QPainter painter(&pix);
				renderGradient(&painter, r, lightenColor(cg.background(), 40), darkenColor(cg.background(), 30), horiz);
				w->setErasePixmap(pix);
			}
			break;
		}

		case PE_DockWindowHandle: {
			QWidget* widget = dynamic_cast<QWidget*>(p->device()); // qt_dockwidget_internal - QDockWindowHandle
			
			int ch, cs, cv;
			cg.background().getHsv(ch, cs, cv);
			if(cv < 100)
				p->setPen(alphaBlendColors(cg.background(), white, 120));
			else
				p->setPen(alphaBlendColors(cg.background(), black, 180));
			
			if(widget && widget == hoverWidget) {
				if (w > h) {
					int ycenter = r.height()/2;
					for(int k = r.width()/2-5; k < r.width()/2+5; k+=3) {
						p->drawRect(k, ycenter-1, 2, 2);
					}
				}
				else {
					int xcenter = r.width()/2;
					for(int k = r.height()/2-5; k < r.height()/2+5; k+=3) {
						p->drawRect(xcenter-1, k, 2, 2);
					}
				}
			}
			break;
		}

	// TOOLBAR SEPARATOR
	// -----------------
		case PE_DockWindowSeparator: {
			if(!_toolBtnAsBtn) {
				int ch, cs, cv;
				cg.background().getHsv(ch, cs, cv);
				if(cv < 100)
					p->setPen(alphaBlendColors(cg.background(), white, 150));
				else
					p->setPen(alphaBlendColors(cg.background(), black, 150));
				
				if(horiz) {
					int x = r.width()/2;
					for(int i = 2; i < r.height()-4; i += 3)
						p->drawPoint(x, i);
				}
				else {
					int y = r.height()/2;
					for(int i = 2; i < r.width()-4; i += 3)
						p->drawPoint(i, y);
				}
			}
			break;
		}
		case PE_CheckMark: {
			if( flags & Style_On ) {
				if(flags & Style_Enabled)
					p->drawPixmap(r.x()+1, r.y()+1, *checkMark, 0, 0, 16, 17);
				else
					p->drawPixmap(r.x()+1, r.y()+1, *checkMark, 16, 0, 16, 17);
			}
			else if ( flags & Style_Off ) {
			}
			else {// tristate
				p->setPen(_customCheckMarkColor ? _checkMarkColor : qApp->palette().active().foreground());
				p->drawLine(6,6,11,6);
				p->drawLine(6,8,11,8);
				p->drawLine(6,10,11,10);
			}
			
			break;
		}

		case PE_SpinWidgetUp:
		case PE_SpinWidgetDown: {
			QPixmap* arrow;
			bool isEnabled = (flags & Style_Enabled);
			QString enabled = isEnabled ? "1" : "0";
			QColor color = p->pen().color();
			
			if(!(arrow = QPixmapCache::find(QString::number(color.pixel(), 16)+"spinArrows"+enabled))) {
				QImage ar = tintImage(qembed_findImage("spinBoxArrows"), color);
				if(!isEnabled) { // make it transparent
					ar = setImageOpacity(ar, 25);
				}
				arrow = new QPixmap(ar);
				QPixmapCache::insert(QString::number(color.pixel(), 16)+"spinArrows"+enabled, arrow);
			}
			
			if(textEffectSettings.mode) {
				QPixmap* effectArrow;
				if(!(effectArrow = QPixmapCache::find(QString::number(textEffectSettings.buttonColor.pixel(), 16)+"spinEArrows"+enabled))) {
					QImage img = tintImage(qembed_findImage("spinBoxArrows"), textEffectSettings.buttonColor);
					int opacity = textEffectSettings.buttonOpacity*100/255;
					effectArrow = new QPixmap(setImageOpacity(img, !isEnabled ? opacity*25/100 : opacity));
					QPixmapCache::insert(QString::number(textEffectSettings.buttonColor.pixel(), 16)+"spinEArrows"+enabled, effectArrow);
				}
				bitBlt(p->device(), r.x()+7/2+1+textEffectSettings.buttonPos.x(), r.y()+r.height()/2-3+textEffectSettings.buttonPos.y(), effectArrow, 0, pe == PE_SpinWidgetDown ? 5 : 0, 7, 5);
				
			}
			
			bitBlt(p->device(), r.x()+7/2+1, r.y()+r.height()/2-3, arrow, 0, pe == PE_SpinWidgetDown ? 5 : 0, 7, 5);
			break;
			}
		case PE_HeaderArrow:
		case PE_ArrowUp:
		case PE_ArrowDown:
		case PE_ArrowLeft:
		case PE_ArrowRight: {
			
			QRect re = r;
			int x2, w2, h2;
			int x = -4;
			switch (pe) {
				case PE_ArrowUp: {
					
					x2 = 10;
					w2 = 7;
					h2 = 5;
					break;
				}
				case PE_ArrowDown: {
					x2 = 17;
					w2 = 7;
					h2 = 5;
					break;
				}
				case PE_ArrowLeft: {
					x += 1;
					x2 = 0;
					w2 = 5;
					h2 = 7;
					break;
				}
				case PE_ArrowRight: {
					x += 2;
					x2 = 5;
					w2 = 5;
					h2 = 7;
					break;
				}
				default: {
					if(flags & Style_Up) {
						x2 = 10;
						w2 = 7;
						h2 = 5;
					}
					else {
						x2 = 17;
						w2 = 7;
						h2 = 5;
					}
					break;
				}
			}
			
			
			QPixmap* arrow;
			bool isEnabled = (flags & Style_Enabled);
			QString enabled = isEnabled ? "1" : "0";
			QColor color = p->pen().color();
			
			if(!(arrow = QPixmapCache::find(QString::number(color.pixel(), 16)+"scrollBarArrows"+enabled))) {
				QImage ar = tintImage(qembed_findImage("scrollBarArrows"), color);
				if(!isEnabled) {
					ar = setImageOpacity(ar, 25);
				}
				arrow = new QPixmap(ar);
				QPixmapCache::insert(QString::number(color.pixel(), 16)+"scrollBarArrows"+enabled, arrow);
			}
			
			if(/*pe == PE_HeaderArrow &&*/ textEffectSettings.mode) {
				QPixmap* effectArrow;
				if(!(effectArrow = QPixmapCache::find(QString::number(textEffectSettings.buttonColor.pixel(), 16)+"scrollBarEArrows"+enabled))) {
					QImage img = tintImage(qembed_findImage("scrollBarArrows"), textEffectSettings.buttonColor);
					int opacity = textEffectSettings.buttonOpacity*100/255;
					effectArrow = new QPixmap(setImageOpacity(img, !isEnabled ? opacity*25/100 : opacity));
					QPixmapCache::insert(QString::number(textEffectSettings.buttonColor.pixel(), 16)+"scrollBarEArrows"+enabled, effectArrow);
				}
				p->drawPixmap(re.x()+re.width()/2+x+textEffectSettings.buttonPos.x(), re.y()+re.height()/2-3+textEffectSettings.buttonPos.y(), *effectArrow, x2, 0, w2, h2);
			}
			
			p->drawPixmap(re.x()+re.width()/2+x, re.y()+re.height()/2-3, *arrow, x2, 0, w2, h2);
			break;
		}

		case PE_CheckListIndicator: {
			// the rect in not correctly calculated in Qt, so the controller rect is 2px above the drawn rect.
			
			QCheckListItem *item = opt.checkListItem();
			QListView *lv = item->listView();
			if(!item)
				return;
			
			QString state = flags& Style_On ? "1" : "0";
			state += flags& Style_Selected ? "1" : "0";
			state += flags& Style_NoChange ? "1" : "0";
			
			QColor background = flags& Style_Selected ? cg.highlight() : cg.base();
			QColor foreground = cg.text();
			
			QPixmap* pix;
			if(!(pix = QPixmapCache::find(QString::number(background.pixel(), 16)+QString::number(foreground.pixel(), 16)+"checkListIndicator"+state))) {
				pix = new QPixmap(r.x()+lv->itemMargin()+r.width()+4, item->height());
				pix->fill(background);
				QPainter painter(pix);
				
				painter.setPen(alphaBlendColors(foreground, background, 127));
				painter.drawLine(r.x()+1, r.y()+1, r.x()+12, r.y()+1); // t
				painter.drawLine(r.x()+1, r.y()+14, r.x()+12, r.y()+14); // b
				painter.drawLine(r.x(), r.y()+2, r.x(), r.y()+13); // l
				painter.drawLine(r.x()+13, r.y()+2, r.x()+13, r.y()+13); // r
				
				painter.setPen(alphaBlendColors(foreground, background, 80));
				painter.drawPoint(r.x(), r.y()+1);
				painter.drawPoint(r.x()+13, r.y()+1);
				painter.drawPoint(r.x(), r.y()+14);
				painter.drawPoint(r.x()+13, r.y()+14);
				
				
				if(flags& Style_NoChange) {
					painter.fillRect(r.x()+1, r.y()+2, 12, 12, dGetColor(background, 30));
					painter.drawPixmap(r.x()-1, r.y(), tintImage(qembed_findImage("checkmark"), foreground));
				}
				if(flags& Style_On)
					painter.drawPixmap(r.x()-1, r.y(), tintImage(qembed_findImage("checkmark"), foreground));
				
				QPixmapCache::insert(QString::number(background.pixel(), 16)+QString::number(foreground.pixel(), 16)+"checkListIndicator"+state, pix);
			}
			
			p->drawPixmap(0, 0, *pix);
			break;
		}
		
		case PE_CheckListExclusiveIndicator: {
			QColor background = flags& Style_Selected ? cg.highlight() : cg.base();
			p->drawPixmap(r.left()+3, r.top()+1, tintImage(qembed_findImage("checkListExclusiveIndicator"), alphaBlendColors(cg.text(), background, 127)));

			if(flags&Style_On)
				p->drawPixmap(r.x()+7, r.y()+5, tintImage(qembed_findImage("rb-dot"),cg.text()));
			break;
		}
		
		case PE_SizeGrip: {
			
			bool parentBgPix = cg.brush(QColorGroup::Background).pixmap();
			QString trans = parentBgPix ? "1" : "0";
			QPixmap* grip;
			if(!(grip = QPixmapCache::find(QString::number(cg.background().pixel(), 16)+"sizeGrip"+trans))) {
				if(parentBgPix)
					grip = new QPixmap(qembed_findImage("sizeGrip"));
				else {
					grip = new QPixmap(4, 4);
					grip->fill(cg.background());
					QPixmap tmpPix = qembed_findImage("sizeGrip");
					bitBlt(grip, 0, 0, &tmpPix, 0, 0, -1, -1, Qt::CopyROP);
				}
				QPixmapCache::insert(QString::number(cg.background().pixel(), 16)+"sizeGrip"+trans, grip);
			}
			
			bitBlt(p->device(), r.width()-(QApplication::reverseLayout() ? 12 : 6), r.height()-12, grip, 0, 0, -1, -1, Qt::CopyROP);
			bitBlt(p->device(), r.width()-6, r.height()-6, grip, 0, 0, -1, -1, Qt::CopyROP);
			bitBlt(p->device(), r.width()-12, r.height()-6, grip, 0, 0, -1, -1, Qt::CopyROP);
			break;
		}
		
		
		
		
		case PE_RubberBand: {
			
#if KDE_VERSION >= 0x30506
			if(rubberBandType != ArgbRubber || !compositeManagerRunning) {
#else
			if(rubberBandType != ArgbRubber) {
#endif
				if(rubberBandType == LineRubber) {
					p->save();
					p->setPen(QPen(black, 1, Qt::SolidLine));
					p->drawRect(r);
					p->restore();
				}
				else {
					KStyle::drawPrimitive(pe, p, r, cg, flags, opt);
				}
				break;
			}
			
			
			
			bool normalized = r.width() < 0 || r.height() < 0;
			QRect re = normalized? r.normalize(): r;
			
			if(re.y() < 0)
				re.setY(-3);
			if(re.x() < 0)
				re.setX(-3);
			
			
			if(!viewPortPressed || re == oldRubberRect || re.width() == 0 || re.height() == 0) {
				break;
			}

			QColor fg = alphaBlendColors(_argbRubberBandColor, black, 127);
			const QWidget* w = dynamic_cast<QWidget*>(p->device());
			if(!w)
				w = rubberViewPort;
			
			QPoint po = w->mapToGlobal(QPoint(0,0));
			QRect gr = QRect(po.x(), po.y(), w->width(), w->height());
			
			QRegion mask(re.x()+1, re.y(), re.width()-2, re.height());
			mask += QRegion(re.x(), re.y()+1, re.width(), re.height()-2);
			
			rubber->create(gr, mask);
			
			
			GC gc2 = XCreateGC(qt_xdisplay(), rubber->window, 0, 0);
			
			/////// erasing
			XRectangle xrect;
			XRectangle outline[4];
			int num = 0;
			if(re.y() < oldRubberRect.y()) {// ^ top
				outline[num].x = oldRubberRect.x();
				outline[num].y = oldRubberRect.y();
				outline[num].width = oldRubberRect.width();
				outline[num].height = 1;
				num += 1;
				
			}
			if(oldRubberRect.y()+oldRubberRect.height() < re.y()+re.height()) {// v bottom
				outline[num].x = oldRubberRect.x();
				outline[num].y = oldRubberRect.y()+oldRubberRect.height()-1;
				outline[num].width = oldRubberRect.width();
				outline[num].height = 1;
				num += 1;
			}
			if(re.x() < oldRubberRect.x()) {// < left
				outline[num].x = oldRubberRect.x();
				outline[num].y = oldRubberRect.y();
				outline[num].width = 1;
				outline[num].height = oldRubberRect.height()-1;
				num += 1;
			}
			
			if(oldRubberRect.x()+oldRubberRect.width() < re.x()+re.width()) {// > right
				outline[num].x = oldRubberRect.x()+oldRubberRect.width()-1;
				outline[num].y = oldRubberRect.y();
				outline[num].width = 1;
				outline[num].height = oldRubberRect.height()-1;
				num += 1;
			}
			
			for(int i = 0; i<num; i++)
				XClearArea(qt_xdisplay(), rubber->window, outline[i].x, outline[i].y, outline[i].width, outline[i].height, false);
			
			xrect.x = oldRubberRect.x()+1;
			xrect.y = oldRubberRect.y()+1;
			xrect.width = 2;
			xrect.height = 2;
			XClearArea(qt_xdisplay(), rubber->window, xrect.x, xrect.y, xrect.width, xrect.height, false);
			
			xrect.x = oldRubberRect.x()+1;
			xrect.y = oldRubberRect.y()+oldRubberRect.height()-3;
			xrect.width = 2;
			xrect.height = 2;
			XClearArea(qt_xdisplay(), rubber->window, xrect.x, xrect.y, xrect.width, xrect.height, false);
			
			xrect.x = oldRubberRect.x()+oldRubberRect.width()-3;
			xrect.y = oldRubberRect.y()+1;
			xrect.width = 2;
			xrect.height = 2;
			XClearArea(qt_xdisplay(), rubber->window, xrect.x, xrect.y, xrect.width, xrect.height, false);
			
			xrect.x = oldRubberRect.x()+oldRubberRect.width()-3;
			xrect.y = oldRubberRect.y()+oldRubberRect.height()-3;
			xrect.width = 2;
			xrect.height = 2;
			XClearArea(qt_xdisplay(), rubber->window, xrect.x, xrect.y, xrect.width, xrect.height, false);
			
			
			rubber->updateMask(mask);
			
			///////////// painting
			
			XSetForeground(qt_xdisplay(), gc2, preMultiplyColor(qRgba(fg.red(), fg.green(), fg.blue(),153)));
			XDrawRectangle(qt_xdisplay(), rubber->window, gc2, re.x(), re.y(), re.width()-1, re.height()-1);
			
			// inside
			XRectangle xrects[8];
			// top left
			xrects[0].x = re.x()+2;
			xrects[0].y = re.y()+1;
			xrects[0].width = 1;
			xrects[0].height = 1;
			
			xrects[1].x = re.x()+1;
			xrects[1].y = re.y()+2;
			xrects[1].width = 1;
			xrects[1].height = 1;
			// top right
			xrects[2].x = re.x()+re.width()-3;
			xrects[2].y = re.y()+1;
			xrects[2].width = 1;
			xrects[2].height = 1;
			
			xrects[3].x = re.x()+re.width()-2;
			xrects[3].y = re.y()+2;
			xrects[3].width = 1;
			xrects[3].height = 1;
			// bottom left
			xrects[4].x = re.x()+1;
			xrects[4].y = re.y()+re.height()-3;
			xrects[4].width = 1;
			xrects[4].height = 1;
			
			xrects[5].x = re.x()+2;
			xrects[5].y = re.y()+re.height()-2;
			xrects[5].width = 1;
			xrects[5].height = 1;
			// bottom right
			xrects[6].x = re.x()+re.width()-2;
			xrects[6].y = re.y()+re.height()-3;
			xrects[6].width = 1;
			xrects[6].height = 1;
			
			xrects[7].x = re.x()+re.width()-3;
			xrects[7].y = re.y()+re.height()-2;
			xrects[7].width = 1;
			xrects[7].height = 1;
			
			QColor blendc = qRgba(fg.red(), fg.green(), fg.blue(), 36);
			XSetForeground(qt_xdisplay(), gc2, preMultiplyColor(blendColors(_argbRubberBandColor, blendc).rgb()));
			XSetClipRectangles(qt_xdisplay(), gc2, 0, 0, xrects, 8, Unsorted);
			XDrawRectangle(qt_xdisplay(), rubber->window, gc2, re.x()+1, re.y()+1, re.width()-3, re.height()-3);
			
			XRectangle xrects2[8];
			// top left
			xrects2[0].x = re.x()+2;
			xrects2[0].y = re.y();
			xrects2[0].width = 1;
			xrects2[0].height = 1;
			
			xrects2[1].x = re.x()+1;
			xrects2[1].y = re.y()+1;
			xrects2[1].width = 1;
			xrects2[1].height = 1;
			
			xrects2[2].x = re.x();
			xrects2[2].y = re.y()+2;
			xrects2[2].width = 1;
			xrects2[2].height = 1;
			// top right
			xrects2[3].x = re.x()+re.width()-3;
			xrects2[3].y = re.y();
			xrects2[3].width = 1;
			xrects2[3].height = 1;
			
			xrects2[4].x = re.x()+re.width()-2;
			xrects2[4].y = re.y()+1;
			xrects2[4].width = 1;
			xrects2[4].height = 1;
			
			xrects2[5].x = re.x()+re.width()-1;
			xrects2[5].y = re.y()+2;
			xrects2[5].width = 1;
			xrects2[5].height = 1;
			// bottom left
			xrects2[6].x = re.x()+2;
			xrects2[6].y = re.y()+re.height()-1;
			xrects2[6].width = 1;
			xrects2[6].height = 1;
			
			xrects2[7].x = re.x()+1;
			xrects2[7].y = re.y()+re.height()-2;
			xrects2[7].width = 1;
			xrects2[7].height = 1;
			
			xrects2[8].x = re.x();
			xrects2[8].y = re.y()+re.height()-3;
			xrects2[8].width = 1;
			xrects2[8].height = 1;
			// bottom right
			xrects2[9].x = re.x()+re.width()-3;
			xrects2[9].y = re.y()+re.height()-1;
			xrects2[9].width = 1;
			xrects2[9].height = 1;
			
			xrects2[10].x = re.x()+re.width()-2;
			xrects2[10].y = re.y()+re.height()-2;
			xrects2[10].width = 1;
			xrects2[10].height = 1;
			
			xrects2[11].x = re.x()+re.width()-1;
			xrects2[11].y = re.y()+re.height()-3;
			xrects2[11].width = 1;
			xrects2[11].height = 1;
			
			
			XSetForeground(qt_xdisplay(), gc2, preMultiplyColor(qRgba(fg.red(), fg.green(), fg.blue(), 121)));
			XSetClipRectangles(qt_xdisplay(), gc2, 0, 0, xrects2, 12, Unsorted);
			XDrawRectangle(qt_xdisplay(), rubber->window, gc2, re.x()+1, re.y()+1, re.width()-3, re.height()-3);
			XDrawRectangle(qt_xdisplay(), rubber->window, gc2, re.x(), re.y(), re.width()-1, re.height()-1);
			
			// outermost pixels
			XRectangle xrects3[8];
			// top left
			xrects3[0].x = re.x()+1;
			xrects3[0].y = re.y();
			xrects3[0].width = 1;
			xrects3[0].height = 1;
			
			xrects3[1].x = re.x();
			xrects3[1].y = re.y()+1;
			xrects3[1].width = 1;
			xrects3[1].height = 1;
			// top right
			xrects3[2].x = re.x()+re.width()-2;
			xrects3[2].y = re.y();
			xrects3[2].width = 1;
			xrects3[2].height = 1;
			
			xrects3[3].x = re.x()+re.width()-1;
			xrects3[3].y = re.y()+1;
			xrects3[3].width = 1;
			xrects3[3].height = 1;
			// bottom left
			xrects3[4].x = re.x()+1;
			xrects3[4].y = re.y()+re.height()-1;
			xrects3[4].width = 1;
			xrects3[4].height = 1;
			
			xrects3[5].x = re.x();
			xrects3[5].y = re.y()+re.height()-2;
			xrects3[5].width = 1;
			xrects3[5].height = 1;
			// bottom right
			xrects3[6].x = re.x()+re.width()-2;
			xrects3[6].y = re.y()+re.height()-1;
			xrects3[6].width = 1;
			xrects3[6].height = 1;
			
			xrects3[7].x = re.x()+re.width()-1;
			xrects3[7].y = re.y()+re.height()-2;
			xrects3[7].width = 1;
			xrects3[7].height = 1;
			
			XSetForeground(qt_xdisplay(), gc2, preMultiplyColor(qRgba(fg.red(), fg.green(), fg.blue(), 37)));
			XSetClipRectangles(qt_xdisplay(), gc2, 0, 0, xrects3, 8, Unsorted);
			XDrawRectangle(qt_xdisplay(), rubber->window, gc2, re.x(), re.y(), re.width()-1, re.height()-1);
			
			XFlush(qt_xdisplay());
			XFreeGC(qt_xdisplay(),gc2);
			oldRubberRect = re;
			
			break;
		}
		
		default: {
			return KStyle::drawPrimitive(pe, p, r, cg, flags, opt);
		}
	}
}

void DominoStyle::drawControl(ControlElement element,
							  QPainter *p,
							  const QWidget *widget,
							  const QRect &r,
							  const QColorGroup &cg,
							  SFlags flags,
							  const QStyleOption& opt) const
{
	
	const bool enabled = (flags & Style_Enabled);

	//return KStyle::drawControl(element, p, widget, r, cg, flags, opt);
	
	switch (element) {

		case CE_ToolButtonLabel: {
			
			if(widget->isA("KToolBarButton")) {
				DominoKToolBarButton* tb= (DominoKToolBarButton *) widget;
	
				QStyle::SFlags flags   = QStyle::Style_Default;
				QStyle::SCFlags active = QStyle::SC_None;
	
				if (tb->isDown()) {
					flags  |= QStyle::Style_Down;
					active |= QStyle::SC_ToolButton;
				}
				if (tb->isEnabled()) 	flags |= QStyle::Style_Enabled;
				if (tb->isOn()) 		flags |= QStyle::Style_On;
				if (tb->isEnabled() && tb->hasMouse())	flags |= QStyle::Style_Raised;
				if (tb->hasFocus())	flags |= QStyle::Style_HasFocus;
				
				int dx, dy;
				QFont tmp_font(KGlobalSettings::toolBarFont());
				QFontMetrics fm(tmp_font);
				QRect textRect;
				int textFlags = 0;
				
				
				QPixmap pixmap;
				if (tb->iconTextMode() != KToolBar::TextOnly) {
					pixmap =tb->iconSet().pixmap( QIconSet::Automatic,
						tb->isEnabled() ? (tb->isActive() ? QIconSet::Active : QIconSet::Normal) :
								QIconSet::Disabled,
						tb->isOn() ? QIconSet::On : QIconSet::Off );
					
					QImage img = pixmap.convertToImage();
					if((_toolBtnAsBtn || tb->isToggleButton()) && (tb->isDown() || tb->isOn()))
						pixmap = KImageEffect::fade(img, 0.10, black);
					else if(tb->isDown() || tb->isOn())
						pixmap = KImageEffect::fade(img, 0.15, black);
					else if(_highlightToolBtnIcon && tb->hasMouse() && tb->isEnabled())
						pixmap = KImageEffect::fade(img, 0.25, white);
					
				}
				
				QPixmap outlinePixmap = 0;
				bool drawOutlinePixmap = false;
				if(flags &QStyle::Style_HasFocus && focusIndicatorSettings.indicateFocus && !pixmap.isNull()) {
					outlinePixmap = renderOutlineIcon(&pixmap);
					drawOutlinePixmap = true;
				}
				
				if (tb->iconTextMode() == KToolBar::IconOnly) // icon only
				{
					if( !pixmap.isNull())
					{
						
						dx = ( tb->width() - pixmap.width() ) / 2;
						dy = ( tb->height()-1 - pixmap.height() ) / 2;
						
						if(drawOutlinePixmap) {
							p->drawPixmap( dx-1, dy-1, outlinePixmap );
						}
						
						p->drawPixmap( dx, dy, pixmap );
					}
				}
				
				else if (tb->iconTextMode() == KToolBar::IconTextRight) // icon and text (if any)
				{
					if( !pixmap.isNull())
					{
						dx = 4;
						dy = ( tb->height()-1 - pixmap.height() ) / 2;
						
						if(drawOutlinePixmap) {
							p->drawPixmap( dx-1, dy-1, outlinePixmap );
						}
						
						p->drawPixmap( dx, dy, pixmap );
					}
					if (!tb->textLabel().isNull())
					{
						textFlags = AlignVCenter|AlignLeft;
						if (!pixmap.isNull())
							dx = 4 + pixmap.width() + 2;
						else
							dx = 4;
						dy = 0;
	
						textRect = QRect(dx, dy, tb->width()-dx, tb->height()-1);
					}
				}
				else if (tb->iconTextMode() == KToolBar::TextOnly)
				{
					if (!tb->textLabel().isNull())
					{
						textFlags = AlignVCenter|AlignLeft;
						dx = (tb->width() - fm.width(tb->textLabel())) / 2;
						dy = (tb->height()-1 - fm.lineSpacing()) / 2;
	
						textRect = QRect( dx, dy, fm.width(tb->textLabel()), fm.lineSpacing() );
					}
				}
				else if (tb->iconTextMode() == KToolBar::IconTextBottom)
				{
					if( !pixmap.isNull())
					{
						dx = (tb->width() - pixmap.width()) / 2;
						dy = (tb->height()-1 - fm.lineSpacing() - pixmap.height()) / 2;
						
						if(drawOutlinePixmap) {
							p->drawPixmap( dx-1, dy-1, outlinePixmap );
						}
						
						p->drawPixmap( dx, dy, pixmap );
					}
	
					if (!tb->textLabel().isNull())
					{
						textFlags = AlignBottom|AlignHCenter;
						dx = (tb->width() - fm.width(tb->textLabel())) / 2;
						dy = tb->height()-1 - fm.lineSpacing() - 4;
	
						textRect = QRect( dx, dy, fm.width(tb->textLabel()), fm.lineSpacing() );
					}
				}
				if (!tb->textLabel().isNull() && !textRect.isNull())
				{
					p->setFont(KGlobalSettings::toolBarFont());
					QColor pencolor;
					if (!tb->isEnabled())
						pencolor = tb->palette().disabled().dark();
					// replaced through icon highlights
// 					else if(tb->isRaised())
// 						pencolor = KGlobalSettings::toolBarHighlightColor();
					else
						pencolor =  tb->colorGroup().buttonText();
					
					dominoDrawItem( p, textRect, textFlags, cg, enabled, 0, tb->textLabel(), -1, &pencolor, textEffectSettings.mode > 0 ? 1 : 0);
				}
			}
			else { // QToolButton - bye bye win95 iconset mode :)
				
				const QToolButton *toolbutton = (const QToolButton *) widget;
				QRect rect = r;
				Qt::ArrowType arrowType = opt.isDefault() ? Qt::DownArrow : opt.arrowType();
				
				if (!opt.isDefault()) {
					PrimitiveElement pe;
					switch (arrowType) {
						case Qt::LeftArrow:  pe = PE_ArrowLeft;  break;
						case Qt::RightArrow: pe = PE_ArrowRight; break;
						case Qt::UpArrow:    pe = PE_ArrowUp;    break;
						default:
						case Qt::DownArrow:  pe = PE_ArrowDown;  break;
					}
					
					drawPrimitive(pe, p, rect, cg, flags, opt);
				}
				else {
					
					QColor btext = toolbutton->paletteForegroundColor();

					if (toolbutton->iconSet().isNull() && ! toolbutton->text().isNull() && ! toolbutton->usesTextLabel()) {
						int alignment = AlignCenter | ShowPrefix;
						if (!styleHint(SH_UnderlineAccelerator, widget, QStyleOption::Default, 0))
							alignment |= NoAccel;
						dominoDrawItem( p, rect, alignment, cg, enabled, 0, toolbutton->text(), toolbutton->text().length(), &btext, textEffectSettings.mode > 0 ? 1 : 0);
					}
					else {
						QPixmap pm;
						QIconSet::Size size = toolbutton->usesBigPixmap() ? QIconSet::Large : QIconSet::Small;
						QIconSet::State state = toolbutton->isOn() ? QIconSet::On : QIconSet::Off;
						QIconSet::Mode mode;
/*						if (!toolbutton->isEnabled())
							mode = QIconSet::Disabled;
						else*/ if (flags & (Style_Down | Style_On) || (flags & Style_Raised) && (flags & Style_AutoRaise))
							mode = QIconSet::Active;
						else
							mode = QIconSet::Normal;
						
						pm = toolbutton->iconSet().pixmap( size, mode, state );
						
						if(!toolbutton->isEnabled())
							pm = disableIcon(&pm);
						
						QImage img = pm.convertToImage();
						if((_toolBtnAsBtn || toolbutton->isToggleButton()) && (toolbutton->isDown() || toolbutton->isOn()))
							pm = KImageEffect::fade(img, 0.10, black);
						else if(toolbutton->isDown() || toolbutton->isOn())
							pm = KImageEffect::fade(img, 0.15, black);
						else if(_highlightToolBtnIcon && toolbutton->hasMouse() && toolbutton->isEnabled())
							pm = KImageEffect::fade(img, 0.25, white);
						
						QPixmap outlinePixmap = 0;
						bool drawOutlinePixmap = false;
						if(flags &QStyle::Style_HasFocus && focusIndicatorSettings.indicateFocus) {
							outlinePixmap = renderOutlineIcon(&pm);
							drawOutlinePixmap = true;
						}
						
						if ( toolbutton->usesTextLabel() ) {
							p->setFont( toolbutton->font() );
							QRect pr = rect, tr = rect;
							int alignment = ShowPrefix;
							if (!styleHint(SH_UnderlineAccelerator, widget, QStyleOption::Default, 0))
								alignment |= NoAccel;

							if ( toolbutton->textPosition() == QToolButton::Under ) {
								int fh = p->fontMetrics().height();
								pr.addCoords( 0, 1, 0, -fh-3 );
								tr.addCoords( 0, pr.bottom(), 0, -3 );
								
								if(drawOutlinePixmap)
									drawItem( p, pr, AlignCenter, cg, TRUE, &outlinePixmap, QString::null );
								
								drawItem( p, pr, AlignCenter, cg, TRUE, &pm, QString::null );
								alignment |= AlignCenter;
							} else {
								pr.setWidth( pm.width() + 8 );
								tr.addCoords( pr.right(), 0, 0, 0 );
								
								if(drawOutlinePixmap)
									drawItem( p, pr, AlignCenter, cg, TRUE, &outlinePixmap, QString::null );
								
								drawItem( p, pr, AlignCenter, cg, TRUE, &pm, QString::null );
								alignment |= AlignLeft | AlignVCenter;
							}
							dominoDrawItem( p, tr, alignment, cg, enabled, 0, toolbutton->textLabel(), toolbutton->textLabel().length(), &btext, textEffectSettings.mode > 0 ? 1 : 0);
						}
						else {
							if(drawOutlinePixmap)
								drawItem( p, rect, AlignCenter, cg, TRUE, &outlinePixmap, QString::null );
							
							drawItem( p, rect, AlignCenter, cg, TRUE, &pm, QString::null );
						}
					}
				}
			}
			break;
		}

	// PROGRESSBAR
	// -----------
		case CE_ProgressBarGroove: {
			drawPrimitive(PE_Panel, p, r, cg, flags, opt);
			break;
		}

		case CE_ProgressBarContents: {
			
			const QProgressBar *pb = dynamic_cast<const QProgressBar*>(widget);
			int steps = pb->totalSteps();
			
			const QColor bg = enabled? cg.base(): cg.background(); // background
			const QColor fg = cg.background();
			bool reverseLayout = QApplication::reverseLayout();
			
			QPixmap* prBgShadow;
			
			int h, s, v, bv;
			bg.getHsv(&h, &s, &v);
			cg.background().getHsv(&h, &s, &bv);
			int diff = KMAX(v-bv, 0);
			
			// if we don't adjust the shadow to the background, it will look strange
			QColor tintColor = alphaBlendColors(Qt::black, bg, diff+30);
			
			if(!(prBgShadow = QPixmapCache::find(QString::number(bg.pixel(), 16)+QString::number(cg.background().pixel(), 16)+"prBgShadow"))) {
				prBgShadow = new QPixmap(4,4);
				prBgShadow->fill(bg);
				QPainter painter(prBgShadow);
				painter.drawPixmap(0, 0, tintImage(qembed_findImage("progressShadow2"), tintColor));
				QPixmapCache::insert(QString::number(bg.pixel(), 16)+QString::number(cg.background().pixel(), 16)+"prBgShadow", prBgShadow);
			}
			
			QRect surface;
			QRect rightBg;
			QRect leftBg;
			p->setClipRegion(r);
			
			
			if( steps == 0 ) { // Busy indicator
				static const int barWidth = 10;
				int progress = pb->progress() % (2*(r.width()-barWidth));
				if( progress < 0)
					progress = 0;
				if( progress > r.width()-barWidth )
					progress = (r.width()-barWidth)-(progress-(r.width()-barWidth));
				
				surface  = QRect(reverseLayout? r.right()-progress+1: r.x()+progress+1, r.top(), barWidth-2, r.height());
				rightBg = reverseLayout ? QRect(r.x(), r.y(), r.width()-(progress-1), r.height()) :
						QRect(r.x()+progress+barWidth-1, r.y(), r.width()-(progress+barWidth-1), r.height());
				
				if(rightBg.width()+surface.width() < r.width()) {
					leftBg = reverseLayout ? QRect(rightBg.width()+surface.width(), r.y(), r.width()-(rightBg.width()+surface.width()-2), r.height()) :
							QRect(r.x(), r.y(), r.x()+progress+1, r.height());
				}
			}
			else {
				double percent = static_cast<double>(pb->progress()) / static_cast<double>(steps);
				int w = static_cast<int>(r.width() * percent);
				if(w < 0)
					w = 0;
				int w2 = r.width()-(r.width()-w);
				rightBg = QRect(reverseLayout? r.left(): r.left()+w, r.top(), r.width()-w, r.height());
				surface = QRect(reverseLayout? r.right()-w2+1: r.left(), r.top(), w2, r.height());
			}
			
			
			
			QPixmap* surfaceTile;
			if(!(surfaceTile = QPixmapCache::find("prTile"+QString::number(surface.height())+QString::number(bg.pixel(), 16)))) {
				surfaceTile = new QPixmap(20, surface.height());
				QPainter tilePainter(surfaceTile);
				
				flatMode=true;
				renderSurface(&tilePainter, QRect(0, 0, 20, surface.height()), fg,  cg.background(), fg , Is_Button|Is_Horizontal);
				flatMode=false;
				
				QPixmap pix = qembed_findImage("progressTile");
				tilePainter.drawTiledPixmap(0,0, 20, surface.height(), pix);
				
				QPixmapCache::insert("prTile"+QString::number(surface.height())+QString::number(bg.pixel(), 16), surfaceTile);
			}
			
			
			int staticShift = 0;
			int animShift = 0;
			
			// find the animation Offset for the current Widget
			QWidget* nonConstWidget = const_cast<QWidget*>(widget);
			QMapConstIterator<QWidget*, int> iter = progAnimWidgets.find(nonConstWidget);
			if (iter != progAnimWidgets.end())
				animShift = iter.data();
			
			int counter = 0;
			while(counter < (surface.width()+20)) {
				counter += 20;
				if (reverseLayout) {
					p->drawPixmap(surface.right()-counter+animShift+staticShift+1, r.top(), *surfaceTile);
				}
				else {
					p->drawPixmap(surface.left()+counter-20-animShift+staticShift, r.top(), *surfaceTile);
				}
			}
			
			
			// empty area
			
			QColor color_d = alphaBlendColors(tintColor, bg, 170);
			QColor color_m = alphaBlendColors(tintColor, bg, 85);
			QColor color_l = alphaBlendColors(tintColor, bg, 25);
			
			if(leftBg.isValid()) {
				p->setClipRegion(leftBg);
				p->fillRect(leftBg, bg);
				
				p->setPen(color_d);
				p->drawLine(leftBg.x()+2, leftBg.y(), leftBg.right()-2, leftBg.y());
				p->setPen(color_m);
				p->drawLine(leftBg.x()+2, leftBg.y()+1, leftBg.right()-2, leftBg.y()+1);
				p->setPen(color_l);
				p->drawLine(leftBg.x()+2, leftBg.y()+2, leftBg.right()-2, leftBg.y()+2);
				p->drawLine(leftBg.x()+2, leftBg.bottom(), leftBg.right()-2, leftBg.bottom());

				//p->setPen(color_l);
				p->drawLine(leftBg.x()+1, leftBg.y()+3, leftBg.x()+1, leftBg.bottom()-1); // l
				p->drawLine(leftBg.right()-1, leftBg.y()+3, leftBg.right()-1, leftBg.bottom()-1); // r
			
				p->setPen(color_m);
				p->drawLine(leftBg.x(), leftBg.y()+3, leftBg.x(), leftBg.bottom()-1); // l
				p->drawLine(leftBg.right(), leftBg.y()+3, leftBg.right(), leftBg.bottom()-1); // r
			
				p->drawPixmap(leftBg.right()-1, leftBg.y(), *prBgShadow, 2, 0, 2, 3); // tr
				p->drawPixmap(leftBg.x(), leftBg.y(), *prBgShadow, 0, 0, 2, 3);
				p->drawPixmap(leftBg.right()-1, leftBg.bottom(), *prBgShadow, 2, 3, 2, 1); // br
				p->drawPixmap(leftBg.x(), leftBg.bottom(), *prBgShadow, 0, 3, 2, 1);
			}
			
			p->setClipRegion(rightBg);
			p->fillRect(rightBg, bg);
			
			p->setPen(color_d);
			p->drawLine(rightBg.x()+2, rightBg.y(), rightBg.right()-2, rightBg.y());
			p->setPen(color_m);
			p->drawLine(rightBg.x()+2, rightBg.y()+1, rightBg.right()-2, rightBg.y()+1);
			p->setPen(color_l);
			p->drawLine(rightBg.x()+2, rightBg.y()+2, rightBg.right()-2, rightBg.y()+2);
			p->drawLine(rightBg.x()+2, rightBg.bottom(), rightBg.right()-2, rightBg.bottom());

			//p->setPen(color_l);
			p->drawLine(rightBg.x()+1, rightBg.y()+3, rightBg.x()+1, rightBg.bottom()-1); // l
			p->drawLine(rightBg.right()-1, rightBg.y()+3, rightBg.right()-1, rightBg.bottom()-1); // r
			
			p->setPen(color_m);
			p->drawLine(rightBg.x(), rightBg.y()+3, rightBg.x(), rightBg.bottom()-1); // l
			p->drawLine(rightBg.right(), rightBg.y()+3, rightBg.right(), rightBg.bottom()-1); // r
			
			
			p->drawPixmap(rightBg.right()-1, rightBg.y(), *prBgShadow, 2, 0, 2, 3); // tr
			p->drawPixmap(rightBg.x(), rightBg.y(), *prBgShadow, 0, 0, 2, 3);
			p->drawPixmap(rightBg.right()-1, rightBg.bottom(), *prBgShadow, 2, 3, 2, 1); // br
			p->drawPixmap(rightBg.x(), rightBg.bottom(), *prBgShadow, 0, 3, 2, 1);
			
			if(rightBg.width() == 2) { // must overpaint two lighter pixel
				p->drawPixmap(rightBg.right(), rightBg.y()+2, *prBgShadow, 0, 3, 1, 1); // br
				p->drawPixmap(rightBg.right(), rightBg.bottom(), *prBgShadow, 0, 3, 1, 1); // br
			}
			
			p->setClipping(false);

			break;
		}
		case CE_ProgressBarLabel: {
			const QProgressBar *progressbar = (const QProgressBar *) widget;
			QFont font = p->font();
			font.setBold(false);
			p->setFont(font);
			p->setPen(cg.buttonText());
			//p->drawText(r, AlignCenter | SingleLine, progressbar->progressString());
			dominoDrawItem( p, r, AlignCenter | SingleLine, cg, false, 0, progressbar->progressString(), -1,&cg.buttonText(), false);
			break;
		}
		
		case CE_CheckBox: {
			
			QColorGroup g = cg;
			if(!khtmlWidgets.contains(widget))
				g.setColor(QColorGroup::Button, qApp->palette().active().background());
			
			QPixmap insidePix = qembed_findImage("checkbox2inside");
			QPainter painter(&insidePix);
			renderSurface(&painter, QRect(3, 0, 14, 19), g.background(), g.button(), g.button(), Is_CheckItem);
			painter.end();
			QImage inside = insidePix.convertToImage();
			
			QImage checkbox;
			QImage shadow = qembed_findImage("checkbox2shadow");
			QImage circle = tintImage(qembed_findImage("checkbox2rect"), buttonContour->contourColor(Contour_Default));
			blend(shadow, inside, checkbox);
			blend(circle, checkbox, checkbox);
			
			if(flags&Style_Down) {
				QImage pressed = tintImage(qembed_findImage("checkbox2pressed"), buttonContour->contourColor(Contour_Pressed));
				blend(pressed, checkbox, checkbox);
			}
			
			QPixmap pix = QPixmap(checkbox);
			if(flags&Style_On || flags&Style_NoChange) {
				painter.begin(&pix);
				drawPrimitive(PE_CheckMark, &painter, r, cg, flags);
				painter.end();
			}
			
			p->drawPixmap(r.left(), r.top(), pix);
			break;
		}
		
		case CE_CheckBoxLabel: {
			const QCheckBox *cb = (const QCheckBox *) widget;

			int alignment = QApplication::reverseLayout() ? AlignRight : AlignLeft;
			if (!styleHint(SH_UnderlineAccelerator, widget, QStyleOption::Default, 0))
				alignment |= NoAccel;
			
			QRect re = focusIndicatorSettings.drawUnderline ? subRect(SR_CheckBoxFocusRect, cb) : r;
			
			if (flags& Style_HasFocus && focusIndicatorSettings.indicateFocus)
				drawFocusIndicator(p, re, alignment | AlignVCenter |ShowPrefix, cg, flags & Style_Enabled, cb->pixmap(), cb->text(), -1, focusIndicatorSettings.color, focusIndicatorSettings.drawUnderline);

			drawItem(p, r, alignment | AlignVCenter | ShowPrefix, cg,
				flags & Style_Enabled, cb->pixmap(), cb->text());
			break;
		}

	// RADIOBUTTONS
	// ------------
		case CE_RadioButton: {
			QColorGroup g = cg;
			if(!khtmlWidgets.contains(widget))
				g.setColor(QColorGroup::Button, qApp->palette().active().background());
			
			QString down = flags& Style_Down ? "1" : "0";
			
			QPixmap* radio;
			if((flags & Domino_noCache) || !(radio = QPixmapCache::find("tabEdges"+down))) {
				QImage radioImg;
				
				QPixmap insidePix = qembed_findImage("radio3inside");
				QPainter painter(&insidePix);
				renderSurface(&painter, QRect(r.left(), r.top(), r.width(), r.height()+1), g.background(), g.button(), g.background(), Is_CheckItem);
				QImage inside = insidePix.convertToImage();
				
				QImage shadow = qembed_findImage("radio3shadow");
				QImage circle = tintImage(qembed_findImage("radio3circle"), buttonContour->contourColor(Contour_Default));
				blend(shadow, inside, inside);
				blend(circle, inside, radioImg);
				
				if(flags&Style_Down) {
					QImage pressed = tintImage(qembed_findImage("radio3pressed"), buttonContour->contourColor(Contour_Default));
					blend(pressed, radioImg, radioImg);
				}
				
				radio = new QPixmap(radioImg);
				if(!(flags & Domino_noCache))
					QPixmapCache::insert("radio"+down, radio);
			
			}

			if(flags & Style_On) {
				if(flags & Style_Enabled)
					bitBlt(radio, 6, 6, radioIndicator, 0, 0, 5, 5);
				else
					bitBlt(radio, 6, 6, radioIndicator, 5, 0, 5, 5);
			}
			
			bitBlt(p->device(), r.x(), r.y(), radio);
			break;
		}
		
		case CE_RadioButtonLabel: {
			const QRadioButton *radiobutton = (const QRadioButton *) widget;

			int alignment = QApplication::reverseLayout() ? AlignRight : AlignLeft;
			if (!styleHint(SH_UnderlineAccelerator, widget, QStyleOption::Default, 0))
			alignment |= NoAccel;
			
			QRect re = focusIndicatorSettings.drawUnderline ? subRect(SR_RadioButtonFocusRect, radiobutton) : r;
			
			if (flags& Style_HasFocus && focusIndicatorSettings.indicateFocus)
				drawFocusIndicator(p, re, alignment | AlignVCenter |ShowPrefix, cg, flags & Style_Enabled, radiobutton->pixmap(), radiobutton->text(), -1, focusIndicatorSettings.color, focusIndicatorSettings.drawUnderline);
			
			drawItem(p, r, alignment | AlignVCenter | ShowPrefix, cg,
				flags & Style_Enabled, radiobutton->pixmap(), radiobutton->text());
			
			break;
		}

	// TABS
	// ----
		case CE_TabBarTab: {
			
			QTabBar * tb = (QTabBar *) widget;
			
			bool konqTab = false;
			bool kickoffTab = false;
			if(tb->parentWidget()) {
				const QWidget* w = tb->parentWidget();
				if(w->isA("KonqFrameTabs"))
					konqTab = true;
				else if(!strcmp(w->name(), "SUSE::Kickoff::KMenu"))
					kickoffTab = true;
			}

			QTabBar::Shape tbs = tb->shape();
			bool selected = false;
			bool reverseLayout = QApplication::reverseLayout();
			if (flags & Style_Selected) selected = true;
			TabPosition pos;
			if (tb->count() == 1) {
				pos = Single;
			} else if ((tb->indexOf(opt.tab()->identifier()) == 0)) {
				pos = reverseLayout?Last:First;
			} else if (tb->indexOf(opt.tab()->identifier()) == tb->count() - 1) {
				pos = reverseLayout?First:Last;
			} else {
				pos = Middle;
			}
			
			switch (tbs) {
				case QTabBar::TriangularAbove:
					renderTab(p, r, cg, (flags & Style_MouseOver), selected, false, pos, true, false, konqTab);
					break;
				case QTabBar::RoundedAbove:
					renderTab(p, r, cg, (flags & Style_MouseOver), selected, false, pos, false, false, konqTab);
					break;
				case QTabBar::TriangularBelow:
					renderTab(p, r, cg, (flags & Style_MouseOver), selected, true, pos, true, false, konqTab);
					break;
				case QTabBar::RoundedBelow:
					renderTab(p, r, cg, (flags & Style_MouseOver), selected, true, pos, false, false, konqTab);
					break;
				default:
					KStyle::drawControl(element, p, widget, r, cg, flags, opt);
			}
			
			// Qt3 uses this painter to draw the iconSet between CE_TabBarTab and CE_TabBarLabel. If we clip the painter here
			// we will have a clean area to draw the icon at the right position in CE_TabBarLabel.
			if(!kickoffTab)
				p->setClipRect(QRect());
			break;
		}

		case CE_PushButton: {
			
			QPushButton *button = (QPushButton *)widget;
			bool khtmlMode = khtmlWidgets.contains(button);
			bool isDefault = enabled && button->isDefault();
			bool isHoverWidget = enabled && button == hoverWidget;
			bool isSunken = flags & Style_On || flags & Style_Down;
			
			uint contourFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom;
			uint surfaceFlags = Is_Button|Is_Horizontal;
			flatMode = button->isFlat();
			
			QColor bg;
			if(!khtmlMode && !button->paletteBackgroundPixmap()) {
				
				if(dynamic_cast<QGroupBox*>(button->parentWidget())) {
					bg = button->parentWidget()->paletteBackgroundColor();
				}
				else {
					bg = cg.background();
				}
				
				p->fillRect(QRect(r.x(), r.y(), 6, 6), bg);
				p->fillRect(QRect(r.width()-6, r.y(), 6, 6), bg);
				p->fillRect(QRect(r.width()-6, r.height()-7, 6,7), bg);
				p->fillRect(QRect(r.x(), r.height()-7, 6, 7), bg);
			}
			else {
				buttonContour->setAlphaMode();
				contourFlags|=Draw_AlphaBlend;
			}
			if(khtmlMode) contourFlags|=Draw_AlphaBlend;
			if(!enabled) {
				contourFlags|=Is_Disabled;
				surfaceFlags|=Is_Disabled;
			}
			if(isSunken) {
				surfaceFlags|=Is_Down;
				contourFlags|=Is_Down;
			}
			
			if(button->isDown())
				buttonContour->setState(Contour_Pressed);
			else if(isHoverWidget)
				buttonContour->setState(Contour_MouseOver);
			else if(isDefault)
				buttonContour->setState(Contour_DefaultButton);
			
			if(!flatMode) {
				renderSurface(p, QRect(r.x()+2, r.y()+2, r.width()-4, r.height()-5), cg.background(), cg.button(), cg.button(), surfaceFlags);
				
				contourFlags |= Round_UpperLeft|Round_UpperRight|Round_BottomLeft|Round_BottomRight|Is_Button|Is_Horizontal;
 				renderContour(p, r, bg, cg.button(), contourFlags);
				
				if(isSunken) {
					renderButtonShadow(p, QRect(r.x()+2, r.y()+2, r.width()-2, r.height()-3), contourFlags);
				}
			}
			else {
				renderSurface(p, QRect(r.x()-3, r.y()-1, r.width()+6, r.height()+2), cg.background(), cg.button(), cg.button(), surfaceFlags);
				
				QColor contourColor = alphaBlendColors(buttonContour->contourColor(buttonContour->state), cg.background(), 179);
				p->setPen(contourColor);
				if(dynamic_cast<KMultiTabBarTab*>(button)) {
					
					const QWidget* parent = button->parentWidget();
					bool horizontal = parent->width() >= parent->height();
					
					KMultiTabBar* tabBar = static_cast<KMultiTabBar*>(widget->parentWidget()->parentWidget()->parentWidget()->parentWidget());
					
					KMultiTabBarTab* tab;
					QPtrList<KMultiTabBarTab>* list = tabBar->tabs();
					for(tab = list->first(); tab; tab = list->next()) {
						if(horizontal && tab->x()+tab->width() == widget->x())
							break;
						else if(tab->y()+tab->height() == widget->y())
							break;
					}
					
					if(tab) {
						QPainter painter(tab);
						painter.setPen(contourColor);
						QRect re = tab->rect();
						if(horizontal)
							painter.drawLine(re.right(), re.y(), re.right(), re.bottom()); // right lines
						else
							painter.drawLine(re.x(), re.bottom(), re.right(), re.bottom());
					}
					
					if(horizontal) {
						if(button->x() == parent->x())
							p->drawLine(r.x(), r.y(), r.x(), r.bottom()); // first left line
						p->drawLine(r.right(), r.y(), r.right(), r.bottom()); // right lines
					}
					else {
						if(button->y() == parent->y())
							p->drawLine(r.right(), r.y(), r.right(), r.bottom());
						p->drawLine(r.x(), r.y(), r.x(), r.bottom());
					}
					// middle lines
					p->drawLine(r.x(), r.y(), r.right(), r.y());
					p->drawLine(r.x(), r.bottom(), r.right(), r.bottom());
					
// 					if(isSunken) {
// 						contourFlags = Draw_Top|Draw_Left|Draw_Right|Draw_Bottom|Rectangular_UpperLeft|Rectangular_UpperRight|Rectangular_BottomLeft|Rectangular_BottomRight;
// 						renderButtonShadow(p, QRect(r.x()-1, r.y()-1, r.width()+5, r.height()+2), contourFlags);
// 						renderButtonShadow(p, re), contourFlags);
// 					}
					
					
				}
				else {
					p->drawRect(r);
				}
				flatMode = false;
			}
			
			buttonContour->reset();
			break;
		}
		
		case CE_PushButtonLabel:
		{
			
			int x, y, w, h;
			r.rect( &x, &y, &w, &h );
			
			const QPushButton* button = static_cast<const QPushButton *>( widget );
			bool cornArrow = false;
			bool hasFocus = button->hasFocus();

			p->setPen(button->colorGroup().buttonText());
			
			// Does the button have a popup menu?
			if ( button->popup() )
			{
				int dx = pixelMetric( PM_MenuButtonIndicator, widget );
				if ( button->iconSet() && !button->iconSet()->isNull()	&&
					(dx + button->iconSet()->pixmap (QIconSet::Small, QIconSet::Normal, QIconSet::Off ).width()) >= w )
				{
					cornArrow = true; //To little room. Draw the arrow in the corner, don't adjust the widget
				}
				else
				{
					drawPrimitive( PE_ArrowDown, p, visualRect( QRect(x + w - dx - 8, y + 3, dx, h - 3), r ), cg, flags, opt );
					w -= dx;
				}
			}
			
			
			// Draw the icon if there is one
			if ( button->iconSet() && !button->iconSet()->isNull() )
			{
				QIconSet::Mode	mode  = QIconSet::Disabled;
				QIconSet::State state = QIconSet::Off;
				
				if (button->isEnabled())
					mode = button->hasFocus() ? QIconSet::Active : QIconSet::Normal;
				if (button->isToggleButton() && button->isOn())
					state = QIconSet::On;
				
				QPixmap pixmap = button->iconSet()->pixmap( QIconSet::Small, mode, state );
				bool focusIcon = hasFocus && focusIndicatorSettings.indicateFocus && !focusIndicatorSettings.drawUnderline;
				int pw = pixmap.width();
				
				if(button->text().isEmpty()) {
					int bpw = button->pixmap() ? button->pixmap()->width() : 0;
					if(focusIcon) {
						QPixmap outlinePixmap = renderOutlineIcon(&pixmap);
						p->drawPixmap( x + (w - bpw -outlinePixmap.width())/2, y + (h - outlinePixmap.height())/ 2, outlinePixmap );
					}
					p->drawPixmap( x + (w - bpw - pw)/2, y + (h- pixmap.height()) / 2, pixmap );
				}
				else {
					int cw = p->fontMetrics().size(Qt::ShowPrefix, button->text()).width()+pw;
					if(focusIcon) {
						QPixmap outlinePixmap = renderOutlineIcon(&pixmap);
						p->drawPixmap( x + (w - cw)/2 - 1 , y + (h - outlinePixmap.height())/ 2, outlinePixmap );
					}
					p->drawPixmap( x + (w - cw)/2 , y + (h - pixmap.height())/ 2, pixmap );
					
				}
				
				if (cornArrow) //Draw over the icon
					drawPrimitive( PE_ArrowDown, p, visualRect( QRect(x + w - 6, x + h - 6, 7, 7), r ), cg, flags, opt );
				
				x += pw + 4;
				w -= pw + 4;
			}
			
			QRect re = focusIndicatorSettings.drawUnderline ? subRect(SR_PushButtonFocusRect, button) : QRect(x, y, w, h);
			
			if(hasFocus && focusIndicatorSettings.indicateFocus && !button->text().isNull())
				drawFocusIndicator(p, re, AlignCenter|ShowPrefix, cg, button->isEnabled(), 0, button->text(), -1, focusIndicatorSettings.buttonColor, focusIndicatorSettings.drawUnderline, true);
			
			QPixmap buttonPixmap;
			if(button->pixmap()) {
				if(!enabled)
					buttonPixmap = disableIcon(button->pixmap());
				else
					buttonPixmap = *button->pixmap();
			}
			dominoDrawItem( p, QRect(x, y, w, h), AlignCenter|ShowPrefix, button->colorGroup(),
					enabled, button->pixmap() ? &buttonPixmap : 0, button->text(), -1, &button->colorGroup().buttonText(), textEffectSettings.mode > 0 ? 1 : 0 );

			break;
		}
		case CE_TabBarLabel: {
			p->setClipping(false); // was set in CE_TabBarTab to clip the icon drawing
			
			if ( opt.isDefault() )
				break;
			const QTabBar * tb = (const QTabBar *) widget;
			QTab * t = opt.tab();
			bool konqTab = false;
			bool kickoffTab = false;
			int shift, cshift;
			
			switch (tb->shape()) {
				case QTabBar::RoundedAbove:
					shift = 4;
					cshift = 3;
					break;
				case QTabBar::RoundedBelow:
					if((konqTab = tb->parentWidget() && tb->parentWidget()->isA("KonqFrameTabs")) ||
							(kickoffTab = tb->parentWidget() && !strcmp(tb->parentWidget()->name(), "SUSE::Kickoff::KMenu"))) {
						shift = 0;
						cshift = 0;
					}
					else {
						shift = -3;
						cshift = -1;
					}
					break;
				case QTabBar::TriangularAbove:
					shift = 4;
					cshift = 3;
					break;
				case QTabBar::TriangularBelow:
					shift = -5;
					cshift = -3;
					break;
				default:
					shift = 0;
					cshift = 0;
			}

			QRect tr = r;
			bool currentTab =  t->identifier() == tb->currentTab();
			if (!currentTab)
				tr.setTop( tr.top() +shift );
			else
				tr.setTop( tr.top() +cshift );

			int alignment = AlignCenter | ShowPrefix;
			
			if (flags& Style_HasFocus && focusIndicatorSettings.indicateFocus && !t->text().isEmpty())
				drawFocusIndicator(p, QRect(tr.x(), tr.y(), tr.width(), tr.height()), alignment, cg, flags & Style_Enabled, 0, t->text(), -1, focusIndicatorSettings.buttonColor, focusIndicatorSettings.drawUnderline, true);
			
			if(t->iconSet() && !tb->isA("KickoffTabBar")) {
				bool enabled = t->isEnabled() && tb->isEnabled();
				QPixmap pixmap;
				
				if(!KApplicationMode) {
					QIconSet::Mode mode = (t->identifier() && tb->keyboardFocusTab()) ? QIconSet::Active : QIconSet::Normal;
					pixmap = t->iconSet()->pixmap( QIconSet::Small, mode );
					if(!enabled)
						pixmap = disableIcon(&pixmap);
				}
				else {
					QIconSet::Mode mode = enabled ? QIconSet::Normal : QIconSet::Disabled;
					if ( mode == QIconSet::Normal && (t->identifier() && tb->keyboardFocusTab()) )
						mode = QIconSet::Active;
					pixmap = t->iconSet()->pixmap( QIconSet::Small, mode );
				}
				
				int pixw = pixmap.width();
				int pixh = pixmap.height();
				QRect ir = r;
				ir.setLeft( tr.left() - pixw - 4 );
				ir.setRight( tr.right() - 2 );
				int yoff = 0;
				if(!currentTab) {
					yoff = pixelMetric(QStyle::PM_TabBarTabShiftVertical, tb);
				}
				
				switch(tb->shape()) {
					case QTabBar::RoundedAbove:
					case QTabBar::TriangularAbove:
						ir.setHeight(ir.height()+4);
						break;
					default:
						if(!konqTab)
							ir.setHeight(ir.height()-2);
				}
				
				p->drawPixmap( ir.left() + 2, ir.center().y()-pixh/2+ yoff, pixmap );
			}
			
			QColor textColor = cg.foreground();
			if(konqTab)
				textColor = cg.foreground() != tb->palette().active().foreground() ? cg.foreground() : tb->palette().active().foreground();
			else if(konsoleMode)
				textColor = tb->palette().active().foreground();
			
			dominoDrawItem( p, tr, alignment, cg, flags & Style_Enabled, 0, t->text(), -1, &textColor/*&cg.foreground()*/, textEffectSettings.mode > 0 ? 1 : 0);
			break;
		}

		case CE_MenuBarItem: {
			
			bool active	 = flags & Style_Active;
			bool focused = flags & Style_HasFocus;
			bool down = flags & Style_Down;
			
			if (active && focused) {
				
				if(!macStyleBar)
					p->fillRect(r, cg.background()); // gtk-qt engine fix
				
				QColor c = down ? dGetColor(cg.background(), 15) : dGetColor(cg.background(), 25);
				
				p->fillRect(QRect(r.left()+3,r.y()+1, r.width()-7, r.height()), c);
				p->fillRect(QRect(r.left(),r.top()+4, 3, r.height()-7), c);
				p->fillRect(QRect(r.right()-3, r.y()+4, 3, r.height()-7), c);
		
				QPixmap pix = tintImage(qembed_findImage("menuItem"), c);
				p->drawPixmap(r.left(),r.y()+1, pix, 0, 0, 3, 3);
				p->drawPixmap(r.left(),r.bottom()-2, pix, 0, 3, 3, 3);
				p->drawPixmap(r.right()-3,r.y()+1, pix, 3, 0, 3, 3);
				p->drawPixmap(r.right()-3,r.bottom()-2, pix, 3, 3, 3, 3);
			}
			
			p->setPen(cg.foreground() );
			drawItem(p, r, AlignVCenter | AlignHCenter | ShowPrefix | DontClip | SingleLine, cg, true, 0,
				 opt.menuItem()->text(), -1, &cg.foreground() );
			break;
		}

	// POPUPMENU ITEM (highlighted on mouseover)
	// ------------------------------------------
		case CE_PopupMenuItem: {
			
			const QPopupMenu *popupmenu = static_cast< const QPopupMenu * >( widget );
			QMenuItem *mi = opt.menuItem();
			
			if ( !mi )
				break;
			
			bool checked = popupmenu->isCheckable() && mi->isChecked();
			int tab = opt.tabWidth();
			int maxpmw = mi->iconSet() || _indentPopupMenuItems ? opt.maxIconWidth() : 0;
			int cmwidth = _indentPopupMenuItems ? maxpmw ? 0 : 12 : 16; // checkmark, visible 10
			if(!_indentPopupMenuItems && checked)
				maxpmw += cmwidth-1;
			bool enabled = mi->isEnabled();
			bool active = flags & Style_Active;
			bool reverse = QApplication::reverseLayout();
			bool separator = mi->isSeparator();
			
			
			QColor color;
			if(separator || (active && enabled)) {
				
				color = separator ? dGetColor(_popupMenuColor, 20) : _selMenuItemColor;
				float val = 0;
				int ph = popupmenu->rect().height()/2;
				int part = separator ? QMAX(1, ph/r.y()+r.height()) : QMAX(1, ph/r.height());
					
				if(_drawPopupMenuGradient) {
					if(r.y()+r.height()/2 < ph) {
						for( int i=r.y(); i < ph; i += part) {
							val += 0.5;
						}
						color = lightenColor(color, (int)val);
					}
					else {
						for( int i=0; i < r.y()-ph; i += part) {
							val += 0.5;
						}
						color = darkenColor(color, (int)val);
					}
				}
			}
			
			
			
			// Draw the menu item background
			if (active) {
				if (enabled) {
					QColor c = color;
					p->fillRect(QRect(r.x()+3, r.y(), r.width()-6, r.height()), c);
					p->fillRect(QRect(r.x(), r.y()+3, 3, r.height()-6), c);
					p->fillRect(QRect(r.x()+r.width()-3, r.y()+3, 3, r.height()-6), c);
					
					QPixmap pix = tintImage(qembed_findImage("menuItem"), c);
					bitBlt(p->device(), r.x(), r.y(), &pix, 0, 0, 3, 3, Qt::CopyROP);
					bitBlt(p->device(), r.x(), r.bottom()-2, &pix, 0, 3, 3, 3, Qt::CopyROP);
					bitBlt(p->device(), r.x()+r.width()-3, r.y(), &pix, 3, 0, 3, 3, Qt::CopyROP);
					bitBlt(p->device(), r.x()+r.width()-3, r.bottom()-2, &pix, 3, 3, 3, 3, Qt::CopyROP);
					
				}
			}
			
			// Are we a menu item separator?
			if(separator) {
				p->setPen(color);
				p->drawLine(r.x(), r.y()+2, r.right(), r.y()+2);
				break;
			}
			
			if(checked) {
				// We only have to draw the background if the menu item is inactive -
				// if it's active the "pressed" background is already drawn
				QRect cmr = visualRect( QRect( r.x()+itemFrame , r.y() + 1, QMIN(QMAX(maxpmw, cmwidth), r.height() - 2), r.height() - 2 ), r );
				
				// Draw the checkmark
				SFlags cflags = Style_On;
				if (enabled)
					cflags |= Style_Enabled;
				
				if(reverse && _indentPopupMenuItems)
					drawPrimitive( PE_CheckMark, p, QRect(cmr.x() + (maxpmw ? (maxpmw-16)/2-4 : -1), cmr.y(), cmr.width(), cmr.height()), cg, cflags );
				else if(_indentPopupMenuItems)
					drawPrimitive( PE_CheckMark, p, QRect(cmr.x() + (maxpmw ? (maxpmw-16)/2-3 : -6), cmr.y(), cmr.width(), cmr.height()), cg, cflags );
				else
					drawPrimitive( PE_CheckMark, p, QRect(reverse ? cmr.x()+cmr.width()-15 : cmr.x()-3, cmr.y(), cmr.width(), cmr.height()), cg, cflags );
				
				
				
			}
			if(!(_indentPopupMenuItems && checked) && mi->iconSet()) {
				QRect cr = visualRect( QRect( (reverse ? r.x()-2 : r.x()) + itemFrame, r.y() + 1, maxpmw -1 , r.height() - 2 ), r );
				QIconSet::Mode mode;

				// Select the correct icon from the iconset
				QPixmap pixmap;
				if(KApplicationMode) {
					if (active)
						mode = enabled ? QIconSet::Active : QIconSet::Disabled;
					else
						mode = enabled ? QIconSet::Normal : QIconSet::Disabled;
					pixmap = mi->iconSet()->pixmap(QIconSet::Small, mode);
				}
				else {
					pixmap = mi->iconSet()->pixmap(QIconSet::Small, active ? QIconSet::Active : QIconSet::Normal);
					if(!enabled)
						pixmap = disableIcon(&pixmap);
				}
				
				if(checked && !reverse)
					cr.setX(cr.x()+cmwidth);
				p->drawPixmap( cr.x(), cr.y(), pixmap );
			}

			// Time to draw the menu item label...
			int xm =  itemFrame + maxpmw; // X position margin
			if(_indentPopupMenuItems)
				xm += cmwidth;
			
			int xp = reverse ? // X position
					r.x() + tab+ itemFrame*2 + itemHMargin + maxpmw +cmwidth + (_indentPopupMenuItems ? 16 : 0) - xm:
					r.x() + xm;
			

			// Label width (minus the width of the accelerator portion)
			int tw = r.width() - xm - tab - arrowHMargin - itemHMargin * 3 - itemFrame + 1;

			// Set the color for enabled and disabled text
			// (used for both active and inactive menu items)
			p->setPen( enabled ? cg.buttonText() : cg.mid() );

			// This color will be used instead of the above if the menu item
			// is active and disabled at the same time. (etched text)
			QColor discol = cg.mid();

			// Does the menu item draw it's own label?
			if(mi->custom()) {
				int m = 2;
				// Save the painter state in case the custom
				// paint method changes it in some way
				mi->custom()->paint( p, cg, active, enabled, xp, r.y()+m, tw, r.height()-2*m );
			}
			else {
				// The menu item doesn't draw it's own label
				QString s = mi->text();
				// Does the menu item have a text label?
				if ( !s.isNull() ) {
					int t = s.find( '\t' );
					int m = 2;
					int text_flags = AlignVCenter | ShowPrefix | DontClip | SingleLine;
					text_flags |= reverse ? AlignRight : AlignLeft;

					QColor draw = (active && enabled) ? cg.highlightedText() : cg.foreground();
					p->setPen(draw);

					// Does the menu item have a tabstop? (for the accelerator text)
					if ( t >= 0 ) {
						
						int arrowMargin = (mi->popup() || _indentPopupMenuItems ) ? itemHMargin + arrowHMargin : 0;
						
						int tabx = reverse ? r.x() + arrowMargin + itemFrame : r.x() + r.width() - tab - arrowMargin- itemFrame;

						// Draw the right part of the label (accelerator text)
						//p->drawText( tabx, r.y()+m, tab, r.height()-2*m, text_flags, s.mid( t+1 ) );
						dominoDrawItem( p, QRect(tabx, r.y()+m, tab, r.height()-2*m), text_flags, cg, flags & Style_Enabled, 0, s.mid( t+1 ), -1, &draw);
						s = s.left( t );
					}

					// Draw the left part of the label (or the whole label
					// if there's no accelerator)
					//p->drawText( xp, r.y()+m, tw, r.height()-2*m, text_flags, s, t );
					dominoDrawItem( p, QRect(xp, r.y()+m, tw, r.height()-2*m), text_flags, cg, flags & Style_Enabled, 0, s, t, &draw);
					p->setPen(cg.text());
				}

				// The menu item doesn't have a text label
				// Check if it has a pixmap instead
				else if ( mi->pixmap() ) {
					QPixmap *pixmap = mi->pixmap();

					// Draw the pixmap
					if ( pixmap->depth() == 1 )
						p->setBackgroundMode( OpaqueMode );

					int diffw = ( ( r.width() - pixmap->width() ) / 2 )
									+ ( ( r.width() - pixmap->width() ) % 2 );
					p->drawPixmap( r.x()+diffw, r.y()+1, *pixmap );

					if ( pixmap->depth() == 1 )
						p->setBackgroundMode( TransparentMode );
				}
			}

			// Does the menu item have a submenu?
			if ( mi->popup() ) {
				PrimitiveElement arrow = reverse ? PE_ArrowLeft : PE_ArrowRight;
				
				int dim = pixelMetric(PM_MenuButtonIndicator) - 1;
				QRect vr = visualRect( QRect( r.x() + r.width() - 5 - 1 - dim,
							r.y() + r.height() / 2 - dim / 2, dim, dim), r );

				TextEffect textEffectMode_save = textEffectSettings.mode;
				if(textEffectSettings.mode == TextEffect_OnlyButton)
					textEffectSettings.mode = TextEffect_None;
				if(enabled) {
					p->setPen(active? cg.highlightedText(): cg.foreground());
					drawPrimitive( arrow, p, vr, cg, Style_Enabled);
				}
				else {
					p->setPen(cg.mid());
					drawPrimitive( arrow, p, vr, cg, Style_Default);
				}
				textEffectSettings.mode = textEffectMode_save;
			}
			break;
		}

		case CE_ToolBoxTab: {
			
			bool selected= (flags & Style_Selected && ((QToolBox*)widget)->currentItem());
			
			QPixmap pix(widget->size());
			QPainter painter(&pix);
			
			QColor background = (flags&Style_Down && !selected) ? darkenColor(cg.background(), 15) : cg.background();
			
			painter.fillRect(QRect(5, 0, r.width()-10, r.height()), background);
			renderGradient(&painter, QRect(r.x(), r.y(), 5, r.height()), background.light(120), background, false);
			renderGradient(&painter, QRect(r.width()-5, r.y(), 5, r.height()), background, background.light(115), false);
			painter.setPen(cg.background().dark(120));
			painter.drawLine(r.x(), r.bottom(), r.width(), r.bottom());
			painter.setPen((hoveredToolBoxTab && !selected) ? dGetColor(cg.background(), 95) : dGetColor(cg.background(), 45));
			
			PrimitiveElement arrow;
			QRect re = QRect(r.x()+6,r.y(),r.x()+10,r.height());
			if(selected) {
				arrow = PE_ArrowDown;
				re.setY(re.y()+1);
			}
			else {
				arrow = PE_ArrowRight;
			}
			drawPrimitive(arrow, &painter, re, cg, Style_Enabled);
			
			p->drawPixmap(r.x(), r.y(), pix);
			break;
		}
		
		case CE_HeaderLabel:
		{
			QRect rect = r;
			const QHeader* header = (const QHeader *) widget;
			int section = opt.headerSection();

			QIconSet* icon = header->iconSet( section );
			if ( icon ) {
				QPixmap pixmap = icon->pixmap( QIconSet::Small,
						flags & Style_Enabled ?
								QIconSet::Normal : QIconSet::Disabled );
				int pixw = pixmap.width();
				int pixh = pixmap.height();
				// "pixh - 1" because of tricky integer division

				QRect pixRect = rect;
				pixRect.setY( rect.center().y() - (pixh - 1) / 2 );
				drawItem ( p, pixRect, AlignVCenter, cg, flags & Style_Enabled,
					   &pixmap, QString::null );
				if (QApplication::reverseLayout())
					rect.setRight(rect.right() - pixw - 2);
				else
					rect.setLeft(rect.left() + pixw + 2);
			}
			
			if (rect.isValid())
				dominoDrawItem ( p, rect, AlignVCenter, cg, flags & Style_Enabled,
					0, header->label( section ), -1, &(cg.buttonText()), textEffectSettings.mode > 0 ? 1 : 0 );
			break;
		}
		default:
		  KStyle::drawControl(element, p, widget, r, cg, flags, opt);
	}
}

void DominoStyle::drawControlMask(ControlElement element,
								  QPainter *p,
								  const QWidget *w,
								  const QRect &r,
								  const QStyleOption &opt) const
{

	switch (element) {
		case CE_PushButton: {
			break;
		}
		case CE_CheckBox: {
			drawPrimitive(PE_IndicatorMask, p, r, w->colorGroup(), Style_Default, opt);
			break;
		}
		case CE_RadioButton : {
			drawPrimitive(PE_ExclusiveIndicatorMask, p, r, w->colorGroup(), Style_Default, opt);
			break;
		}
		default: {
			break;
			//KStyle::drawControlMask (element, p, w, r, opt);
		}
	}
}

void DominoStyle::drawComplexControl(ComplexControl control,
									 QPainter *p,
									 const QWidget *widget,
									 const QRect &r,
									 const QColorGroup &cg,
									 SFlags flags,
									 SCFlags controls,
									 SCFlags active,
									 const QStyleOption& opt) const
{
	const bool reverseLayout = QApplication::reverseLayout();

	const bool enabled = (flags & Style_Enabled);

	//return KStyle::drawComplexControl(control, p, widget, r, cg, flags, controls, active, opt);
	
	switch(control) {

	// COMBOBOX
	// --------
		case CC_ComboBox: {

			static const unsigned int handleWidth = 15;
			const QComboBox *cb = dynamic_cast<const QComboBox*>(widget);
			bool editable = cb && cb->editable();
			bool isDown = cb && cb->listBox() && cb->listBox()->isVisible();
			bool hasFocus =(flags&Style_HasFocus);
			const QColor inputColor = enabled ? (editable ? cg.base() : cg.button()) : cg.background();
			const QColor bg = (cb && cb->parentWidget()) ?  cb->parentWidget()->paletteBackgroundColor() : cg.background();
			bool hasPixmap = cb && cb->count() > 0 && cb->pixmap(cb->currentItem());
			uint contourFlags = 0;
			bool isKhtmlWidget = khtmlWidgets.contains(cb);
			
			if(isKhtmlWidget || (cb && cb->parentWidget() && cb->parentWidget()->paletteBackgroundPixmap())) {
				contourFlags |= Draw_AlphaBlend;
				buttonContour->setAlphaMode();
			}
			else {
				p->fillRect(QRect(r.x(), r.y(), 6, r.height()), bg);
				p->fillRect(QRect(r.width()-6, r.y(), 6, r.height()), bg);
			}
			
			if(hasPixmap && editable && r.width() > 40) {
				int x = reverseLayout ? handleWidth+3 : 6;
				QRegion mask(x, 0, 17, r.height());
				if(!reverseLayout)
					mask += QRegion(x-2, 4, 2, r.height()-9);
				p->setClipRegion(mask);
			}
			
			contourFlags |= Round_UpperLeft|Round_BottomLeft|Round_UpperRight|Round_BottomRight|Draw_Right|Draw_Top|
					Draw_Bottom|Draw_Left|Is_Button|Is_Horizontal;
			uint surfaceFlags = Is_Button|Is_Horizontal;
			if(isDown) {
				surfaceFlags |= Is_Down;
				contourFlags |= Is_Down;
			}
			renderSurface(p, QRect(r.left()+2, r.top()+2, r.width()-4, r.height()-5), cg.background(), cg.button(), cg.background(), surfaceFlags);
			
			if(isDown)
				buttonContour->setState(Contour_Pressed);
			else if(enabled && cb == hoverWidget)
				buttonContour->setState(Contour_MouseOver);
			renderContour(p, r, bg, cg.button(), contourFlags);
			
			
			if(editable) {
				QColor lineEditColor = enabled ? cg.base() : cg.background();
				QString isEnabled = enabled ? "1" : "0";
				QPixmap* lineEditBtnEdges; // shared with spinwidget
				if(!(lineEditBtnEdges = QPixmapCache::find(QString::number(cg.button().pixel(), 16)+QString::number(lineEditColor.pixel(), 16)+"lineEditBtnEdges"+isEnabled))) { // + buttoncolor
					
					// To not make the button contour appear thicker with dark button gradiens, we need a tad smaller rounding for the lineEdit edges.
					QPixmap pixmap(1, r.height());
					QPainter painter(&pixmap);
					flatMode = true;
					renderSurface(&painter, QRect(0, 0, 1, r.height()), cg.background(), cg.button(), cg.background(), surfaceFlags);
					flatMode = false;
					QImage img = pixmap.convertToImage();
					
					// Test, if we have a dark button. Not accurate, but...
					QColor c1 = QColor(img.pixel(0, r.y()+4));
					QColor c2 = QColor(img.pixel(0, r.height()-6));
					int v, v1, v2;
					c1.getHsv(&v, &v ,&v1);
					c2.getHsv(&v, &v ,&v2);
					QImage edges = tintImage(qembed_findImage( (v1+v2)/2 < 200 ? "lineEditBtnEdges-d" : "lineEditBtnEdges"), lineEditColor);
					
					if(enabled) {
						QImage shadow = qembed_findImage("lineEditBtnShadow");
						blend(shadow, edges, edges);
					}
					lineEditBtnEdges = new QPixmap(edges);
					QPixmapCache::insert(QString::number(cg.button().pixel(), 16)+QString::number(lineEditColor.pixel(), 16)+"lineEditBtnEdges"+isEnabled, lineEditBtnEdges);
				}
				
				if(reverseLayout) {
					bitBlt(p->device(), r.width()-6, r.y()+3, lineEditBtnEdges, 3, 0, 3, 3);
					bitBlt(p->device(), r.width()-6, r.height()-7, lineEditBtnEdges, 3, 3, 3, 3);
				}
				else {
					bitBlt(p->device(), r.x()+3, r.y()+3, lineEditBtnEdges, 0, 0, 3, 3);
					bitBlt(p->device(), r.x()+3, r.height()-7, lineEditBtnEdges, 0, 3, 3, 3);
				}
			}
			
			
			QRect ar(reverseLayout ? r.left()+3 : r.width()-handleWidth-1, r.height()/2-2, handleWidth, 6);
			if(hasFocus && focusIndicatorSettings.indicateFocus && enabled && cb) {
				QRect editField = querySubControlMetrics(control, widget, SC_ComboBoxEditField);
				int pw = hasPixmap ? (cb->pixmap(cb->currentItem()))->width()+2 : 0;
				
				if(!editable) {
					int fmheight = p->fontMetrics().height();
					if(reverseLayout)
						editField.addCoords(pw+17, editField.height()/2-fmheight/2-1,0 , 0);
					else
						editField.addCoords(2+pw, editField.height()/2-fmheight/2-1,0 , 0);
					QRect re = focusIndicatorSettings.drawUnderline ? subRect(SR_ComboBoxFocusRect, cb) : editField;
					drawFocusIndicator(p, re, AlignLeft, cg, true, 0, cb->currentText(), -1, focusIndicatorSettings.buttonColor, focusIndicatorSettings.drawUnderline, true);
				}
			}
			
			if(isDown) {
				uint shadowFlags = Draw_Top|Draw_Right|Draw_Bottom|Draw_Left;
				QRect sr;
				if(editable) {
					shadowFlags |= reverseLayout? Round_UpperLeft|Round_BottomLeft|Rectangular_UpperRight|Rectangular_BottomRight :
							Rectangular_UpperLeft|Rectangular_BottomLeft|Round_UpperRight|Round_BottomRight;
					sr = QRect(reverseLayout ? r.left()+2 : r.width()-handleWidth-3, r.y()+2, handleWidth+3, r.height()-3);
				}
				else {
					shadowFlags |= Round_UpperLeft|Round_BottomLeft|Round_UpperRight|Round_BottomRight;
					sr = QRect(r.x()+2, r.y()+2, r.width()-2, r.height()-3);
				}
				
				renderButtonShadow(p, sr, shadowFlags);
			}
			buttonContour->reset();
			
			p->setPen(cb ? cb->palette().active().buttonText() : qApp->palette().active().buttonText());
			uint aflags = Style_Default|Style_Raised;
			if(enabled)
				aflags |= Style_Enabled;
			drawPrimitive(PE_ArrowDown, p, ar, cg, aflags);
			
			p->setPen(cg.foreground());
			break;
		}
		
	// TOOLBUTTON
	// ----------
		case CC_ToolButton: {
			
			QToolButton *tb = (QToolButton *) widget;
			
			if(_toolBtnAsBtn && ::qt_cast<KToolBarButton*>(tb)) {
				
				const bool reverse = QApplication::reverseLayout();
				KToolBar *bar = dynamic_cast<KToolBar*>(tb->parentWidget());
				if(!bar) break;
				KToolBarButton* btn = ::qt_cast<KToolBarButton*>(tb);
				KToolBarButton* prevBtn = 0;
				KToolBarButton* nextBtn = 0;
				bool prevBtnIsOn = false;
				bool nextBtnIsOn = false;
				bool isOn = flags &Style_On || flags &Style_Down;
				bool isDown = flags &Style_Down;
				const bool horizontal = bar->orientation() == Qt::Horizontal;
				
				QRect cr = r , sr = r, bsr = r;
				int index = bar->itemIndex(btn->id());
				
				if(bar->getButton(bar->idAt(index-1))) {
					prevBtn = bar->getButton(bar->idAt(index-1));
					if((!reverse) ? (horizontal) ? prevBtn->x() + prevBtn->width() != btn->x() : prevBtn->y() + prevBtn->height() != btn->y() :
							(horizontal) ? prevBtn->x() != btn->x()+btn->width() : prevBtn->y() + prevBtn->height() != btn->y()
					  )
						prevBtn = 0;
					else
						prevBtnIsOn = prevBtn->isOn();
				}
				if(bar->getButton(bar->idAt(index+1))) {
					nextBtn= bar->getButton(bar->idAt(index+1));
					if((!reverse) ? (horizontal) ? nextBtn->x() != btn->x() + btn->width() : nextBtn->y() != btn->y() + btn->height() :
							(horizontal) ? nextBtn->x() + nextBtn->width() != btn->x() : nextBtn->y() != btn->y() + btn->height()
					   )
						nextBtn = 0;
					else
						nextBtnIsOn = nextBtn->isOn();
				}
				
				bool drawSeparator = false;
				uint contourFlags = Is_Button;
				uint surfaceFlags = Is_Button;
				
				if(horizontal) {
					surfaceFlags |=Is_Horizontal;
					contourFlags |=Is_Horizontal;
				}
				
				if(isOn) {
					surfaceFlags |= Is_Down;
					contourFlags |= Is_Down;
				}
				
				QPixmap buffer(btn->size());
				QPainter painter(&buffer);
				
				if(tb->parentWidget() && tb->parentWidget()->paletteBackgroundPixmap()) {
					painter.drawTiledPixmap(0, 0,  tb->width(), tb->height(), *tb->parentWidget()->paletteBackgroundPixmap(), tb->geometry().x(), tb->geometry().y());
					contourFlags |= Draw_AlphaBlend;
				}
				else
					painter.fillRect(buffer.rect(), cg.brush(QColorGroup::Background));

				
				if(prevBtn) {
					if(nextBtn) {
						// middle
						if(horizontal) {
							contourFlags |= Draw_Top|Draw_Bottom;
							cr.setLeft(r.left()-6);
							cr.setWidth(r.width()+12);
							bsr.setLeft(r.left()-2);
							bsr.setWidth(r.width()+3);
							sr.addCoords(-1, 2, 0, -3);
							drawSeparator = true;
						}
						else {
							contourFlags |= Draw_Left|Draw_Right;
							cr.setY(r.y()-6);
							cr.setHeight(r.height()+13);
							bsr.addCoords(2, 0, 0, 1);
							sr.addCoords(2, -1, -2, 0);
							drawSeparator = true;
						}
					}
					else {
						// right / bottom
						if(!horizontal) {
							contourFlags |= Draw_Bottom|Draw_Right|Draw_Left|Round_BottomRight|Round_BottomLeft;
							cr.setY(r.y()-6);
							cr.setHeight(r.height()+6);
							bsr.addCoords(2, 0, 0, -1);
							sr.addCoords(2, -1, -2, -3);
						}
						else if(!reverse) {
							contourFlags |= Draw_Top|Draw_Bottom|Draw_Right|Round_UpperRight|Round_BottomRight;
							cr.setLeft(r.left()-6);
							cr.setWidth(r.width()+5);
							bsr.setLeft(r.left()-2);
							bsr.setWidth(r.width()+1);
							sr.addCoords(-1, 2, -3, -3);
							
						}
						else { // reverse
							contourFlags |= Draw_Top|Draw_Bottom|Draw_Left|Round_UpperLeft|Round_BottomLeft;
							cr.setWidth(r.width()+6);
							bsr.setWidth(r.width()+1);
							sr.addCoords(2, 2, 0, -3);
							drawSeparator = true;
						}
					}
				}
				else {
					if(nextBtn) {
						// left / top
						if(!horizontal) {
							contourFlags |= Draw_Top|Draw_Left|Draw_Right|Round_UpperLeft|Round_UpperRight;
							cr.setHeight(r.height()+7);
							bsr.addCoords(2, 2, 0, +1);
							sr.addCoords(2, 2, -2, 0);
							drawSeparator = true;
						}
						else if(!reverse) {
							contourFlags |= Draw_Top|Draw_Bottom|Draw_Left|Round_UpperLeft|Round_BottomLeft;
							cr.setWidth(r.width()+6);
							bsr.setWidth(r.width()+1);
							sr.addCoords(2, 2, 0, -3);
							drawSeparator = true;
						}
						else { //reverse
							contourFlags |= Draw_Top|Draw_Bottom|Draw_Right|Round_UpperRight|Round_BottomRight;
							cr.setLeft(r.left()-6);
							cr.setWidth(r.width()+5);
							bsr.setLeft(r.left()-2);
							bsr.setWidth(r.width()+1);
							sr.addCoords(-1, 2, -3, -3);
						}
					}
					else {
						// full
						if(horizontal) {
							cr.setWidth(r.width()-1);
							bsr.setWidth(r.width()-1);
							sr.addCoords(2, 2, -3, -3);
						}
						else {
							bsr.addCoords(2, 2, 0, -1);
							sr.addCoords(2, 2, -2, -3);
						}
						contourFlags |= Draw_Top|Draw_Bottom|Draw_Left|Draw_Right|Round_UpperLeft|Round_BottomLeft|
								Round_UpperRight|Round_BottomRight;
						
					}
				}
				
				
				renderSurface(&painter, sr, cg.background(), cg.background(), cg.background(), surfaceFlags);
				drawControl(CE_ToolButtonLabel, &painter, widget, r, cg, flags,opt);
				renderContour(&painter, cr, cg.background(), cg.background(), contourFlags);
				
				
				/////////////////////////////////////////// drop down indicator
				
				if(tb->popup()) {
					QString isEnabled = enabled ? "1" : "0";
					QPixmap* arrow;
					QColor color = btn->palette().active().buttonText();
					if(!(arrow = QPixmapCache::find(QString::number(color.pixel(), 16)+"arrow_down"+isEnabled))) {
						QImage ar = tintImage(qembed_findImage("arrow_down"), color);
						if(!isEnabled) {
							ar = setImageOpacity(ar, 25);
						}
						arrow = new QPixmap(ar);
						QPixmapCache::insert(QString::number(color.pixel(), 16)+"arrow_down"+isEnabled, arrow);
					}
					
					QPoint point;
					if(prevBtn)
						if(nextBtn)
							if(horizontal)
								point = QPoint(cr.width()-19, cr.bottom()-8);
							else
								point = QPoint(cr.width()-8, cr.bottom()-12);
						else
							if(!horizontal)
								point = QPoint(cr.width()-8, cr.bottom()-8);
							else if(!reverse)
								point = QPoint(cr.width()-15, cr.bottom()-8);
							else // reverse
								point = QPoint(cr.width()-13, cr.bottom()-8);
					else
						if(nextBtn)
							if(!horizontal)
								point = QPoint(cr.width()-8, cr.bottom()-12);
							else if(!reverse)
								point = QPoint(cr.width()-13, cr.bottom()-8);
							else //reverse
								point = QPoint(cr.width()-15, cr.bottom()-8);
						else
							if(horizontal)
								point = QPoint(cr.width()-9, cr.bottom()-8);
							else
								point = QPoint(cr.width()-8, cr.bottom()-8);
							
					bitBlt(&buffer, point.x(), point.y(), arrow);
					
				}
				
				
				/////////////////////////////////////////// separator
				
				if(drawSeparator) { // on the right side of the button
					
					ContourState cState;
					if(reverse && horizontal) {
						if( ((enabled && !isDown) || (prevBtn && prevBtn == hoverWidget)) && (hoverWidget && (tb == hoverWidget || (prevBtn == hoverWidget && prevBtn->isEnabled() && !prevBtn->isDown()))) )
							cState = Contour_MouseOver;
						else if(isDown || (prevBtn && prevBtn->isDown()))
							cState = Contour_Pressed;
						else
							cState = Contour_Default;
					}
					else {
						if( ((enabled && !isDown) || (nextBtn &&nextBtn == hoverWidget)) && (hoverWidget && (tb == hoverWidget || (nextBtn == hoverWidget && nextBtn->isEnabled() && !nextBtn->isDown()))) )
							cState = Contour_MouseOver;
						else if(isDown || (nextBtn && nextBtn->isDown()))
							cState = Contour_Pressed;
						else
							cState = Contour_Default;
					}
					
					painter.setPen(alphaBlendColors(buttonContour->contourColor(cState), cg.background(), 218));
					if(horizontal)
						painter.drawLine(r.width()-1, r.top()+1, r.width()-1, r.bottom()-2);
					else
						painter.drawLine(r.x()+1, r.bottom(), r.width()-2, r.bottom());
					
				}
				
				
				/////////////////////////////////////////// button shadow
				
				if(isOn && buttonContour->drawButtonSunkenShadow) {
					uint shadowFlags;
					if(prevBtnIsOn) { // or above is on
						if(nextBtnIsOn) { // or below is on
							if(horizontal) {
								shadowFlags = Draw_Top|Draw_Bottom;
								bsr.addCoords(-3, 2, +5, -1);
							}
							else {
								shadowFlags = Draw_Left|Draw_Right;
								bsr.addCoords(0, -5, 0, +4);
							}
						}
						else {
							if(horizontal) {
								if(!reverse) {
									shadowFlags = Draw_Top|Draw_Bottom|Draw_Right;
									shadowFlags |= contourFlags& Round_UpperRight ? Round_UpperRight : Rectangular_UpperRight;
									shadowFlags |= contourFlags& Round_BottomRight ? Round_BottomRight : Rectangular_BottomRight;
									bsr.addCoords(-3, 2, 0, -1);
								}
								else {
									shadowFlags = Draw_Top|Draw_Bottom|Draw_Left;
									shadowFlags |= contourFlags& Round_UpperLeft ? Round_UpperLeft : Rectangular_UpperLeft;
									shadowFlags |= contourFlags& Round_BottomLeft ? Round_BottomLeft : Rectangular_BottomLeft;
									bsr.addCoords(2, 2, 5, -1);
								}
							}
							else {
								shadowFlags = Draw_Bottom|Draw_Left|Draw_Right;
								shadowFlags |= contourFlags& Round_BottomLeft ? Round_BottomLeft : Rectangular_BottomLeft;
								shadowFlags |= contourFlags& Round_BottomRight ? Round_BottomRight : Rectangular_BottomRight;
								bsr.setY(bsr.y()-5);
							}
						}
					}
					else if(nextBtnIsOn) {
						if(horizontal) {
							if(!reverse) {
								shadowFlags = Draw_Top|Draw_Bottom|Draw_Left;
								shadowFlags |= (contourFlags& Round_UpperLeft) ? Round_UpperLeft : Rectangular_UpperLeft;
								shadowFlags |= (contourFlags& Round_BottomLeft) ? Round_BottomLeft : Rectangular_BottomLeft;
								bsr.addCoords(2, 2, 5, -1);
							}
							else {
								shadowFlags = Draw_Top|Draw_Bottom|Draw_Right;
								shadowFlags |= contourFlags& Round_UpperRight ? Round_UpperRight : Rectangular_UpperRight;
								shadowFlags |= contourFlags& Round_BottomRight ? Round_BottomRight : Rectangular_BottomRight;
								bsr.addCoords(-3, 2, 0, -1);
							}
						}
						else {
							shadowFlags = Draw_Top|Draw_Left|Draw_Right;
							shadowFlags |= contourFlags& Round_UpperLeft ? Round_UpperLeft : Rectangular_UpperLeft;
							shadowFlags |= contourFlags& Round_UpperRight ? Round_UpperRight : Rectangular_UpperRight;
							bsr.setHeight(bsr.height()+4);
						}
					}
					else {
						if(horizontal) {
							shadowFlags = Draw_Top|Draw_Bottom|Draw_Left|Draw_Right;
							shadowFlags |= contourFlags& Round_UpperLeft ? Round_UpperLeft : Rectangular_UpperLeft;
							shadowFlags |= contourFlags& Round_UpperRight ? Round_UpperRight : Rectangular_UpperRight;
							shadowFlags |= contourFlags& Round_BottomRight ? Round_BottomRight : Rectangular_BottomRight;
							shadowFlags |= contourFlags& Round_BottomLeft ? Round_BottomLeft : Rectangular_BottomLeft;
							bsr.addCoords(2, 2, 0, -1);
						}
						else {
							shadowFlags = Draw_Top|Draw_Bottom|Draw_Left|Draw_Right;
							shadowFlags |= contourFlags& Round_UpperLeft ? Round_UpperLeft : Rectangular_UpperLeft;
							shadowFlags |= contourFlags& Round_UpperRight ? Round_UpperRight : Rectangular_UpperRight;
							shadowFlags |= contourFlags& Round_BottomRight ? Round_BottomRight : Rectangular_BottomRight;
							shadowFlags |= contourFlags& Round_BottomLeft ? Round_BottomLeft : Rectangular_BottomLeft;
						}
					}
					renderButtonShadow(&painter, bsr, shadowFlags);
				}
				
				btn->setUpdatesEnabled(false);
				btn->setErasePixmap(buffer);
				btn->setUpdatesEnabled(true);
				btn->erase();
				
			}
			else if(_toolBtnAsBtn) { // not a KToolBarButton
				
				if(kickerMode && !(flags&(Style_Raised)))
					break;
				
				const QRect br = querySubControlMetrics(control, widget, SC_ToolButton, opt);
				if(br.width() < 16 || br.height()< 16)
					break;
				
				QColorGroup g = widget->colorGroup();
				QColor bg;
				if(widget->parentWidget() && widget->paletteBackgroundColor() != widget->parentWidget()->paletteBackgroundColor()) {
					bg = widget->parentWidget()->paletteBackgroundColor();
					g.setColor(QColorGroup::Background, bg);
				}
				else
					bg = widget->paletteBackgroundColor();
				
				if(!widget->paletteBackgroundPixmap()) {
					p->fillRect(QRect(br.x(), br.y(), 6, 6), bg);
					p->fillRect(QRect(br.width()-6, br.y(), 6, 6), bg);
					p->fillRect(QRect(br.width()-6, br.height()-7, 6, 7), bg);
					p->fillRect(QRect(br.x(), br.height()-7, 6, 7), bg);
				}
				else {
					buttonContour->setAlphaMode();
				}
				
				if(/*flags &Style_On || */flags &Style_Down)
					buttonContour->setState(Contour_Pressed);
				if(enabled && tb == hoverWidget)
					buttonContour->setState(Contour_MouseOver);
				else if(flags & Style_Default)
					buttonContour->setState(Contour_DefaultButton);
				
				renderButton(p, br, g, (flags & Style_On || flags & Style_Down), flags & Style_MouseOver, true, enabled, khtmlWidgets.contains(opt.widget()) );
				buttonContour->reset();
				
				
				if(tb->popup()) {
					QString isEnabled = enabled ? "1" : "0";
					QPixmap* arrow;
					QColor color = tb->palette().active().buttonText();
					if(!(arrow = QPixmapCache::find(QString::number(color.pixel(), 16)+"arrow_down"+isEnabled))) {
						QImage ar = tintImage(qembed_findImage("arrow_down"), color);
						if(!isEnabled) {
							ar = setImageOpacity(ar, 25);
						}
						arrow = new QPixmap(ar);
						QPixmapCache::insert(QString::number(color.pixel(), 16)+"arrow_down"+isEnabled, arrow);
					}
					bitBlt(p->device(), r.width()-8, r.bottom()-8, arrow);
				}
				
			}
			else {
				
				bool isKbtn = ::qt_cast<KToolBarButton*>(tb);
				QPixmap buffer(tb->size());
				QPainter painter(&buffer);
				
				QColor bg = cg.background();
				
				if(tb->parentWidget() && tb->parentWidget()->paletteBackgroundPixmap()) {
					painter.drawTiledPixmap(0, 0,  tb->width(), tb->height(), *tb->parentWidget()->paletteBackgroundPixmap(), tb->geometry().x(), tb->geometry().y());
				}
				else if(tb->parentWidget() && widget->paletteBackgroundColor() != widget->parentWidget()->paletteBackgroundColor()) {
					bg = widget->parentWidget()->paletteBackgroundColor();
					painter.fillRect(buffer.rect(), bg);
				}
				else
					painter.fillRect(buffer.rect(), cg.brush(QColorGroup::Background));
				
				
				QRect fr = QRect(1, 1, r.width()-2, r.height()-2);
				bool isOn = flags &Style_On;
				if(isOn) {
					QRegion reg(fr.x()+2, fr.y()+1, fr.width()-4, fr.height()-2);
					reg += QRegion(fr.x()+1, fr.y()+2, fr.width()-2, fr.height()-4);
					painter.setClipRegion(reg);
					painter.fillRect(r, alphaBlendColors(black, bg, 15));
					painter.setClipping(false);
				}
				if(isKbtn)
					drawControl(CE_ToolButtonLabel, &painter, widget, r, cg, flags,opt);
				
				if(isOn) {
					QColor color = buttonContour->contourColor(Contour_Default);
					QPixmap* frame;
					if(!(frame = QPixmapCache::find(QString::number(color.pixel(), 16)+"toolBtnFrameThin"))) {
						frame = new QPixmap(tintImage(qembed_findImage("toolBtnFrameThin"), color));
						QPixmapCache::insert(QString::number(color.pixel(), 16)+"toolBtnFrameThin", frame);
					}
					
					bitBlt(&buffer, fr.x(), fr.y(), frame, 0, 0, 4, 4);
					bitBlt(&buffer, fr.width()-3, fr.y(), frame, 4, 0, 4, 4);
					bitBlt(&buffer, fr.x(), fr.height()-3, frame, 0, 4, 4, 4);
					bitBlt(&buffer, fr.width()-3, fr.height()-3, frame, 4, 4, 4, 4);
						
					painter.setPen(alphaBlendColors(color, bg, 195));
					painter.drawLine(fr.x()+4, fr.y(), fr.width()-4, fr.y());
					painter.drawLine(fr.x(), fr.y()+4, fr.x(), fr.height()-4);
					painter.drawLine(fr.width(), fr.y()+4, fr.width(), fr.height()-4);
					painter.drawLine(fr.x()+4, fr.height(), fr.width()-4, fr.height());
				}
				
				if(tb->popup() && (!tb->iconSet().isNull() || tb->text() || tb->pixmap())) {
					QString isEnabled = enabled ? "1" : "0";
					QPixmap* arrow;
					QColor color = tb->palette().active().buttonText();
					if(!(arrow = QPixmapCache::find(QString::number(color.pixel(), 16)+"arrow_down"+isEnabled))) {
						QImage ar = tintImage(qembed_findImage("arrow_down"), color);
						if(!isEnabled) {
							ar = setImageOpacity(ar, 25);
						}
						arrow = new QPixmap(ar);
						QPixmapCache::insert(QString::number(color.pixel(), 16)+"arrow_down"+isEnabled, arrow);
					}
					bitBlt(&buffer, r.width()-8, r.bottom()-4, arrow);
				}
				
				if(isKbtn) {
					tb->setUpdatesEnabled(false);
					tb->setErasePixmap(buffer);
					tb->erase();
					tb->setUpdatesEnabled(true);
				}
				else
					p->drawPixmap(0, 0, buffer);
			}
			
			p->setPen(cg.buttonText() );
			break;
		}

	// SPINWIDGETS
	// -----------
		case CC_SpinWidget: {
			
			if(active == SC_SpinWidgetUp || active == SC_SpinWidgetDown)
				break;
			
			static const unsigned int handleWidth = 15;
			const QSpinWidget *sw = dynamic_cast<const QSpinWidget *>(widget);
			const QColor bg = cg.background();
			SFlags sflags = flags;
			PrimitiveElement pe;
			bool enabled = sw && sw->isEnabled();
			const QColor buttonColor = enabled ? cg.button() : cg.background();
			
			uint contourFlags = Round_UpperLeft|Round_BottomLeft|Round_UpperRight|Round_BottomRight|Draw_Right|Draw_Top|
					Draw_Bottom|Draw_Left|Is_Button|Is_Horizontal;
			
			// fill edges
			if(sw && !sw->paletteBackgroundPixmap()) {
				p->fillRect(QRect(r.x(), r.y(), 6, r.height()), bg);
				p->fillRect(QRect(r.width()-6, r.y(), 6, r.height()), bg);
			}
			else {
				contourFlags |= Draw_AlphaBlend;
				buttonContour->setAlphaMode();
			}
			
			uint surfaceFlags = Is_Button|Is_Horizontal;
			if(flags& Style_Down) {
				surfaceFlags |= Is_Down;
				contourFlags |= Is_Down;
			}
			renderSurface(p, QRect(r.x()+2, r.y()+2, r.width()-4, r.height()-5), cg.background(), cg.button(), cg.button(), surfaceFlags);
			
			if(flags & Style_Down)
				buttonContour->setState(Contour_Pressed);
			else if(enabled && sw == hoverWidget)
				buttonContour->setState(Contour_MouseOver);
			renderContour(p, r, cg.background(), cg.button(), contourFlags);
			
			if(flags & Style_Down)
				renderButtonShadow(p, QRect(r.x()+2, r.y()+2, r.width()-2, r.height()-3), contourFlags);
			buttonContour->reset();
			
			
// 			QColor lineEditColor = enabled ? cg.base() : cg.background();
			QColor lineEditColor = enabled ? cg.base() : dynamic_cast<QSpinBox*>(sw->parentWidget()) ?
					((DominoQSpinBox*)sw->parentWidget())->editor()->palette().active().background(): qApp->palette().active().background();
			QString isEnabled = enabled ? "1" : "0";
			QPixmap* lineEditBtnEdges;
			
			if(!(lineEditBtnEdges = QPixmapCache::find(QString::number(cg.button().pixel(), 16)+QString::number(lineEditColor.pixel(), 16)+"lineEditBtnEdges"+isEnabled))) {
				
				// To not make the button contour appear thicker with dark button gradiens, we need a tad smaller rounding for the lineEdit edges.
				QPixmap pixmap(1, r.height());
				QPainter painter(&pixmap);
				flatMode = true;
				renderSurface(&painter, QRect(0, 0, 1, r.height()), cg.background(), cg.button(), cg.button(), surfaceFlags);
				flatMode = false;
				QImage img = pixmap.convertToImage();
				
				// Test, if we have a dark button. Not accurate, but...
				QColor c1 = QColor(img.pixel(0, r.y()+4));
				QColor c2 = QColor(img.pixel(0, r.height()-6));
				int v, v1, v2;
				c1.getHsv(&v, &v ,&v1);
				c2.getHsv(&v, &v ,&v2);
				
				QImage edges = tintImage(qembed_findImage( (v1+v2)/2 < 127 ? "lineEditBtnEdges-d" : "lineEditBtnEdges"), lineEditColor);
				
				if(enabled) {
					QImage shadow = qembed_findImage("lineEditBtnShadow");
					blend(shadow, edges, edges);
				}
				lineEditBtnEdges = new QPixmap(edges);
				QPixmapCache::insert(QString::number(cg.button().pixel(), 16)+QString::number(lineEditColor.pixel(), 16)+"lineEditBtnEdges"+isEnabled, lineEditBtnEdges);
			}
			
			if(reverseLayout) {
				bitBlt(p->device(), r.width()-6, r.y()+3, lineEditBtnEdges, 3, 0, 3, 3);
				bitBlt(p->device(), r.width()-6, r.height()-7, lineEditBtnEdges, 3, 3, 3, 3);
			}
			else {
				bitBlt(p->device(), r.x()+3, r.y()+3, lineEditBtnEdges, 0, 0, 3, 3);
				bitBlt(p->device(), r.x()+3, r.height()-7, lineEditBtnEdges, 0, 3, 3, 3);
			}
			
			
			QRect upRect = QRect(reverseLayout ? r.left()+2 : r.right()-handleWidth-1, r.height()/2-7, handleWidth, 6);
			QRect downRect = QRect(reverseLayout ? r.left()+2 : r.right()-handleWidth-1, r.height()/2+1, handleWidth, 6);
			// icons...
			//p->setPen(cg.foreground());
			p->setPen(sw ? sw->palette().active().buttonText() : qApp->palette().active().buttonText());
			
			sflags = Style_Default;
			if(enabled)
				sflags |= Style_Enabled;
			if (active == SC_SpinWidgetUp) {
				sflags |= Style_On;
				sflags |= Style_Sunken;
			} else
				sflags |= Style_Raised;
				
			if (sw->buttonSymbols() == QSpinWidget::PlusMinus) {
				pe = PE_SpinWidgetPlus;
				upRect.moveTop(upRect.y()+1);
			}
			else
				pe = PE_SpinWidgetUp;
			drawPrimitive(pe, p, upRect, cg, sflags);

			sflags = Style_Default;
			if(enabled)
				sflags |= Style_Enabled;
			if (active == SC_SpinWidgetDown) {
				sflags |= Style_On;
				sflags |= Style_Sunken;
			} else
				sflags |= Style_Raised;
			if (sw->buttonSymbols() == QSpinWidget::PlusMinus)
				pe = PE_SpinWidgetMinus;
			else
				pe = PE_SpinWidgetDown;
			drawPrimitive(pe, p, downRect, cg, sflags);
			
			break;
		}

		case CC_ScrollBar: { // TODO: find a way to draw pressed buttons
			
			const QScrollBar *sb = (const QScrollBar*)widget;
			bool   maxedOut   = (sb->minValue()    == sb->maxValue());
			bool   horizontal = (sb->orientation() == Qt::Horizontal);
			SFlags sflags     = ((horizontal ? Style_Horizontal : Style_Default) |
					(maxedOut   ? Style_Default : Style_Enabled));

			QRect  addline, subline, subline2, addpage, subpage, slider, first, last;
			subline = querySubControlMetrics(control, widget, SC_ScrollBarSubLine, opt);
			addline = querySubControlMetrics(control, widget, SC_ScrollBarAddLine, opt);
			subpage = querySubControlMetrics(control, widget, SC_ScrollBarSubPage, opt);
			addpage = querySubControlMetrics(control, widget, SC_ScrollBarAddPage, opt);
			slider  = querySubControlMetrics(control, widget, SC_ScrollBarSlider,  opt);
// 			first   = querySubControlMetrics(control, widget, SC_ScrollBarFirst,   opt);
// 			last    = querySubControlMetrics(control, widget, SC_ScrollBarLast,    opt);
// 			subline2 = addline;

// 			if (horizontal)
// 				subline2.moveBy(-addline.width(), 0);
// 			else
// 				subline2.moveBy(0, -addline.height());
			
			// Draw the up/left button set
			if ((controls & SC_ScrollBarSubLine) && subline.isValid()) {
				drawPrimitive(PE_ScrollBarSubLine, p, subline, cg,
					      sflags | (active == SC_ScrollBarSubLine ?
							      Style_Down : Style_Default));
				
				
				subline2 = addline;
				if (horizontal)
					subline2.moveBy(-addline.width(), 0);
				else
					subline2.moveBy(0, -addline.height());
				
				if (subline2.isValid())
					drawPrimitive((QStyle::PrimitiveElement)PE_ScrollBarSubLine2, p, subline2, cg,
						sflags | (active == SC_ScrollBarSubLine ?
								Style_Down : Style_Default));
			}

			if ((controls & SC_ScrollBarAddLine) && addline.isValid())
				drawPrimitive(PE_ScrollBarAddLine, p, addline, cg,
					      sflags | ((active == SC_ScrollBarAddLine) ?
							      Style_Down : Style_Default));

			if ((controls & SC_ScrollBarSubPage) && subpage.isValid())
				drawPrimitive(PE_ScrollBarSubPage, p, subpage, cg,
					      sflags | ((active == SC_ScrollBarSubPage) ?
							      Style_Down : Style_Default));

			if ((controls & SC_ScrollBarAddPage) && addpage.isValid())
				drawPrimitive(PE_ScrollBarAddPage, p, addpage, cg,
					      sflags | ((active == SC_ScrollBarAddPage) ?
							      Style_Down : Style_Default));
			
// 			if ((controls & SC_ScrollBarFirst) && first.isValid())
// 				drawPrimitive(PE_ScrollBarFirst, p, first, cg,
// 					      sflags | ((active == SC_ScrollBarFirst) ?
// 							      Style_Down : Style_Default));
// 
// 			if ((controls & SC_ScrollBarLast) && last.isValid())
// 				drawPrimitive(PE_ScrollBarLast, p, last, cg,
// 					      sflags | ((active == SC_ScrollBarLast) ?
// 							      Style_Down : Style_Default));

			if ((controls & SC_ScrollBarSlider) && slider.isValid()) {
				drawPrimitive(PE_ScrollBarSlider, p, slider, cg,
					      sflags | ((active == SC_ScrollBarSlider) ?
							      Style_Down : Style_Default));
			}
			break;
		}
		
		
		
		case CC_TitleBar: {
			
			QRect ir;
			bool down;
			QColorGroup g = widget->colorGroup();
			
			if ( controls & SC_TitleBarLabel ) {
				QPixmap pix(10, r.height());
				pix.fill(g.background());
				
				QPainter painter(&pix);
				renderGradient(&painter, QRect(0, 0, 10, r.height()/2), g.background().light(120), g.background(), true);
				painter.setPen(g.background().dark(105));
				painter.drawLine(0, r.bottom(), 10, r.bottom());
				p->drawTiledPixmap(0,0, r.width(), r.height(), pix);
				
				ir = visualRect( querySubControlMetrics( CC_TitleBar, widget, SC_TitleBarLabel ), widget );
				p->setPen( g.foreground() );
				p->drawText(ir.x()+2, ir.y(), ir.width()-2, ir.height(), AlignCenter | AlignVCenter | SingleLine, widget->caption() );
			}
			if ( controls & SC_TitleBarCloseButton ) {
				ir = visualRect( querySubControlMetrics( CC_TitleBar, widget, SC_TitleBarCloseButton ), widget );
				down = active & SC_TitleBarCloseButton;
				p->drawPixmap(ir.x(), ir.y(), qembed_findImage("DockCloseButton"), 0, down ? 12 : 0, 12, 12);
			}
			if ( controls & SC_TitleBarMaxButton ) {
				ir = visualRect( querySubControlMetrics( CC_TitleBar, widget, SC_TitleBarMaxButton ), widget );
				down = active & SC_TitleBarMaxButton;
				p->drawPixmap(ir.x(), ir.y(), qembed_findImage("OverlapButton"), 0, down ? 12 : 0, 12, 12);
			}
			if ( controls & SC_TitleBarNormalButton) {
				ir = visualRect( querySubControlMetrics( CC_TitleBar, widget, SC_TitleBarNormalButton ), widget );
				down = active & SC_TitleBarNormalButton;
				p->drawPixmap(ir.x(), ir.y(), qembed_findImage("ToDesktopButton"), 0, down ? 12 : 0, 12, 12);
			}
			if ( controls & SC_TitleBarMinButton) {
				ir = visualRect( querySubControlMetrics( CC_TitleBar, widget, SC_TitleBarMinButton ), widget );
				down = active & SC_TitleBarMinButton;
				p->drawPixmap(ir.x(), ir.y(), qembed_findImage("DockbackButton"), 0, down ? 12 : 0, 12, 12);
			}
			if ( controls & SC_TitleBarSysMenu ) {
				if ( widget->icon() ) {
					ir = visualRect( querySubControlMetrics( CC_TitleBar, widget, SC_TitleBarSysMenu ), widget );
					drawItem( p, ir, AlignCenter, widget->colorGroup(), TRUE, widget->icon(), QString::null );
				}
			}
			
			break;
		}
		default:
			KStyle::drawComplexControl(control, p, widget,
										r, cg, flags, controls,
										active, opt);
			break;
	}
}

QRect DominoStyle::subRect(SubRect r, const QWidget *widget) const
{
	switch (r) {
		case SR_PushButtonContents: {
			const QPushButton* btn = static_cast<const QPushButton*>(widget);
			QRect rw = widget->rect();
			if(btn->text().isEmpty())
				return QRect(rw.left()+2, rw.top()+2, rw.width()-4, rw.height()-4);
			return QRect(rw.left()+2, rw.top()+3, rw.width()-4, rw.height()-7);
		}
		case SR_PushButtonFocusRect:
		{
			QRect rect, wrect(widget->rect());
			const QPushButton *button = (const QPushButton *) widget;
			int dbw1 = 0, dbw2 = 0;
			if (button->isDefault() || button->autoDefault()) {
				dbw1 = pixelMetric(PM_ButtonDefaultIndicator, widget);
				dbw2 = dbw1 * 2;
			}

			int dfw1 = pixelMetric(PM_DefaultFrameWidth, widget) * 2,
			dfw2 = dfw1 * 2;

			rect.setRect(wrect.x()      + dfw1 + dbw1,
			wrect.y()      + dfw1 + dbw1,
			wrect.width()  - dfw2 - dbw2,
			wrect.height() - dfw2 - dbw2);
			return rect;
		}
		case SR_ComboBoxFocusRect: {
			return QRect(4, 3, widget->width()-8, widget->height()-7);
// 			return querySubControlMetrics( CC_ComboBox, widget, SC_ComboBoxEditField );
		}
		case SR_CheckBoxFocusRect:
		{
			QRect cr = subRect(SR_CheckBoxContents, widget);
	    		QRect fr = KStyle::subRect(r, widget);
			if(cr.width() <= 0) // no text
				fr.setHeight(fr.height()+2);
			else
				fr.addCoords(+2, 0, -2, 0);
	    		return fr;
		}
		case SR_RadioButtonFocusRect:
		{
			QRect cr = subRect(SR_CheckBoxContents, widget);
			QRect fr = KStyle::subRect(r, widget);
			if(cr.width() <= 0) // no text
				fr.setHeight(fr.height()+2);
			else
				fr.addCoords(+2, 0, -2, 0);
			return fr;
		}
		case SR_ProgressBarGroove: {
			return QRect(widget->rect());
		}
		case SR_ProgressBarContents:
		case SR_ProgressBarLabel: {
			QRect rw = widget->rect();
			return QRect(rw.left()+2, rw.top()+2, rw.width()-4, rw.height()-4 );
		}
		case SR_ToolBoxTabContents: {
			QRect rw = widget->rect();
			return QRect(rw.left()+18, rw.top(), rw.width()-18, rw.height());
		}
		default: {
			return KStyle::subRect(r, widget);
		}
	}
}



QRect DominoStyle::querySubControlMetrics(ComplexControl control,
										  const QWidget *widget,
										  SubControl subcontrol,
										  const QStyleOption &opt) const
{
	if (!widget) {
		return QRect();
	}

	QRect r(widget->rect());
	switch (control) {
		case CC_ComboBox: {
			switch (subcontrol) {
				case SC_ComboBoxEditField: {
					const QComboBox* cb = dynamic_cast<const QComboBox *>(widget);
					if(!cb) // maybe kdevelop's KComboView
						return QRect(r.left()+4, r.top()+3, r.width()-4-15-3, r.height()-7);
					if(!cb->editable())
						return QRect(r.left()+3, r.top()+3, r.width()-4-15-2, r.height()-6);
					
					return QRect(r.left()+3, r.top()+3, r.width()-4-15-2, r.height()-7);
				}
				default: {
					return KStyle::querySubControlMetrics(control, widget, subcontrol, opt);
				}
			}
			break;
		}
		case CC_SpinWidget: {
			const int fw = 2; // Frame width...
			const int buttonsWidth = 15;
			const bool heightDividable = ((r.height()%2) == 0);
			
			const int buttonsLeft = r.right()-buttonsWidth+1-fw;
			switch (subcontrol) {
				case SC_SpinWidgetUp: { // rect without 2px frame
					return QRect(buttonsLeft, r.top()+3, buttonsWidth+1, r.height()/2-3 );
				}
				case SC_SpinWidgetDown: {
					if(heightDividable) {
						return QRect(buttonsLeft, r.top()+r.height()/2+1, buttonsWidth+1, r.height()/2-3);
					} else {
						return QRect(buttonsLeft, r.top()+r.height()/2+1, buttonsWidth+1, r.height()/2-2);
					}
				}
				case SC_SpinWidgetFrame: {
					return QRect(r.left(), r.top(), r.width(), r.height() );
				}
				case SC_SpinWidgetEditField: {
					return QRect(r.left()+1+fw, r.top()+1+fw, r.width()-(buttonsWidth+2+fw*2), r.height()-3-fw*2);
				}
				case SC_SpinWidgetButtonField: {
					return QRect(buttonsLeft, r.top()+2, buttonsWidth, r.height()-5);
				}
				default: {
					return KStyle::querySubControlMetrics(control, widget, subcontrol, opt);
				}
			}
			break;
		}

		case CC_ScrollBar: {
			
			const QScrollBar *scrollbar = (const QScrollBar *) widget;
			int sliderstart = scrollbar->sliderStart();
			const int buttonw = 15 /*- (fw * 2)*/;
			const int buttonh = 15 /*- (fw * 2)*/;

			int maxlen = ((scrollbar->orientation() == Qt::Horizontal) ?
					scrollbar->width() : scrollbar->height()) - buttonh * 3;

			// calculate slider length
			if (scrollbar->maxValue() != scrollbar->minValue()) {
				uint range = scrollbar->maxValue() - scrollbar->minValue();
				sliderlen = (scrollbar->pageStep() * maxlen) /
						(range + scrollbar->pageStep());

				if ( sliderlen < 32 || range > INT_MAX/2 )
					sliderlen = 32;
				if ( sliderlen > maxlen )
					sliderlen = maxlen;
			}
			else
				sliderlen = maxlen;
			
			switch(subcontrol) {
				case SC_ScrollBarAddLine: { // bottom/right button
					if (scrollbar->orientation() == Qt::Horizontal) {
						return QRect(scrollbar->width() - buttonw , 0, buttonw, buttonh);
					} else {
						return QRect(0, scrollbar->height() - buttonh, buttonw, buttonh);
					}
				}
				case SC_ScrollBarSubLine: { // top/left button
					return QRect(0, 0, buttonw, buttonh);
				}
				case SC_ScrollBarSubPage: // top/left
					if (scrollbar->orientation() == Qt::Horizontal)
						return QRect(buttonw, 0, sliderstart-buttonw+1, buttonw);
					return QRect(0, buttonh, buttonw, sliderstart-buttonw+1);

				case SC_ScrollBarAddPage: // bottom/left
					if (scrollbar->orientation() == Qt::Horizontal)
						return QRect(sliderstart + sliderlen-1, 0, maxlen - sliderstart - sliderlen + buttonw+1, buttonw);
					return QRect(0, sliderstart + sliderlen-1, buttonw, maxlen - sliderstart - sliderlen + buttonw+1);

				case SC_ScrollBarGroove:
					if (scrollbar->orientation() == Qt::Horizontal)
						return QRect(buttonw , 0, maxlen, buttonw);
					return QRect(0, buttonw, buttonw, maxlen);

				case SC_ScrollBarSlider:
					if (scrollbar->orientation() == Qt::Horizontal)
						return QRect(sliderstart, 0, sliderlen, buttonw);
					return QRect(0, sliderstart, buttonw, sliderlen);
				default:
					break;
			}
		}
		default: {
			return KStyle::querySubControlMetrics(control, widget, subcontrol, opt);
		}
	}
}

int DominoStyle::pixelMetric(PixelMetric m, const QWidget *widget) const
{
	//return KStyle::pixelMetric(m, widget);
	
	switch(m) {
	// FRAMES
	// ------
		case PM_DefaultFrameWidth: {
			
			if(widget) {
				if(dynamic_cast<const QPopupMenu*>(widget))
					return 3;
				
				// stolen from baghira
				if (dynamic_cast<QTabWidget*>(widget->parentWidget()) && widget->parentWidget()->parentWidget() && (widget->parentWidget()->parentWidget()->isA("KonqMainWindow") || widget->parentWidget()->parentWidget()->isA("KonqFrameContainer")))
					return 0;
			
			}
			return 2;
		}
		case PM_MenuBarFrameWidth:
			return 1;
		case PM_SpinBoxFrameWidth:
			return 2;
			
	// CHECKBOXES / RADIO BUTTONS
	// --------------------------
		case PM_IndicatorWidth:		// Checkbox 18x19
				return 18;
		case PM_IndicatorHeight:
			return 19;

		case PM_ExclusiveIndicatorWidth:	// Radiobutton 17x17
		case PM_ExclusiveIndicatorHeight:
			return 17;

	// TABS
	// ----
		case PM_TabBarTabVSpace: {
			const QTabBar * tb = (const QTabBar *) widget;
			
			switch(tb->shape()) {
				case QTabBar::RoundedAbove:
				case QTabBar::RoundedBelow:
					return 12;
				default:
					return 4;
			}
			
		}
		case PM_TabBarTabShiftVertical: {
			
			const QTabBar * tb = (const QTabBar *) widget;
			switch (tb->shape()) {
				case QTabBar::RoundedAbove:
					return 1;
				case QTabBar::RoundedBelow:
					return (tb->parentWidget() && tb->parentWidget()->isA("KonqFrameTabs"))? 0 : -1;
				default:
					return 0;
			}
		}
		case PM_TabBarTabShiftHorizontal: {
			return 0;
		}
		case PM_TabBarTabOverlap: {
			return 1;
		}
		case PM_TabBarBaseOverlap: {
			const QTabWidget * tw = (const QTabWidget *) widget;
			if(((DominoQTabWidget*)tw)->tabBar()->isHidden() || tw->isA("KonqFrameTabs"))
				return 0;
			return (tw->tabPosition() == QTabWidget::Top) ? -3 : -4;
		}
#if (QT_VERSION >= 0x030300) // requires Qt 3.3
	// extra space between menubar items
		case PM_MenuBarItemSpacing: {
			return 6;
		}
#endif

// #if (QT_VERSION >= 0x030300) // requires Qt 3.3
//	   // extra space between toolbar items
//		   case PM_ToolBarItemSpacing: {
//			   return 4;
//		   }
// #endif

	// SCROLL BAR
// 		case PM_ScrollBarSliderMin: {
// 			return 21;
// 		}

		case PM_ScrollBarExtent: {
			return 15;
		}

		case PM_DockWindowSeparatorExtent:
			return 6;

	// SPLITTERS
	// ---------
		case PM_SplitterWidth: {
			return 6;
		}

	// PROGRESSBARS
	// ------------
		case PM_ProgressBarChunkWidth:
			return 10;

	// SLIDER
	// ------
		case PM_SliderLength:
		case PM_SliderThickness:
			return 15;

	// MENU INDICATOR
	// --------------
		case PM_MenuButtonIndicator:
			return 8;

		case PM_ButtonDefaultIndicator: {
			return 0;
		}

		case PM_ButtonMargin: {
			//return 2;
			return 4;
		}
		case PM_HeaderMargin:
			return 2;
		case PM_ButtonShiftVertical:
		case PM_ButtonShiftHorizontal: {
			return 0;
		}
 		case PM_CheckListControllerSize: // radio controller
		case PM_CheckListButtonSize: { // box controller
			return 16;
		}
// 		case PM_PopupMenuFrameVerticalExtra:
// 			return 1;
// 		case PM_PopupMenuFrameHorizontalExtra:
// 			return 1;
		default:
			return KStyle::pixelMetric(m, widget);
	}
}


QSize DominoStyle::sizeFromContents(ContentsType t,
									const QWidget *widget,
									const QSize &s,
									const QStyleOption &opt) const
{
	//return KStyle::sizeFromContents (t, widget, s, opt);
	
	switch (t) {

		case CT_PushButton:
		{
			const QPushButton* btn = static_cast<const QPushButton*>(widget);

			int w = s.width();
			int h = s.height();
			if ( btn->text().isEmpty() && s.width() < 32 ) {
				w += pixelMetric(PM_ButtonMargin, widget)*2;
				h += pixelMetric(PM_ButtonMargin, widget)*2;
				return QSize(w, h);
			}
			
			int bm = pixelMetric(PM_ButtonMargin, widget);
			int fw = pixelMetric(PM_DefaultFrameWidth, widget) * 2;
			w += bm + fw + 22;
			h += bm + fw;
			return QSize(w, h < 26 ? 26+_buttonHeightAdjustment : h+4 );
		}
		case CT_ComboBox: {
			int bm = pixelMetric(PM_ButtonMargin, widget);
			int fw = pixelMetric(PM_DefaultFrameWidth, widget) * 2;
			int w = s.width();
			int h = s.height();
			w += bm + fw + 22;
			h += bm + fw-3;
			return QSize(w, h < 26 ? 26+_buttonHeightAdjustment : h+4 );
		}
		
		case CT_TabBarTab: {
			// konqi = tabbar
			// other = tab control
			// this is total crap but it helps to reduce the flicker in konqi.
			/////////////////////
			QTabBar* tb = 0;
			if(::qt_cast<QTabBar*>(widget))
				tb = ::qt_cast<QTabBar*>(widget);
			else
				return QSize(s.width(), s.height());
				
			if(!strcmp(widget->name(), "tabbar") && widget->parentWidget()->isA("KonqFrameTabs")) {
				
				QTab* tab = opt.tab();
				int count = tb->count();

				int fm = tb->fontMetrics().width("about:blank");
				if(tb->parentWidget()->width() <= opt.tab()->rect().x()+44+fm)
					return QSize(s.width(), s.height());

				int dots = tb->fontMetrics().width(".");
				if((dots*minTabLength+44)*count >= tb->width())  //  44 = frame+iconwidth
					return QSize(s.width(), s.height());

				QSize sz = KStyle::sizeFromContents( t, widget, s, opt );
	
				QString str;
				str.fill(' ', minTabLength);
				if( tab->text().isEmpty() || tab->text() == str) {
					tab->setText("about:blank");
					//return QSize(  fm+44, 26 );
					return QSize(  fm+44, s.height());
				}
				return QSize(s.width(), s.height());
			}
			else {
				switch (tb->shape()) {
					case QTabBar::RoundedAbove:
					case QTabBar::RoundedBelow:
						return QSize( s.width(), s.height()-2 );
					default:
						return QSize( s.width(), s.height() ); // triangular
				}
			}
		}
		
		case CT_ToolButton: {
			if(dynamic_cast<const QToolButton*>(widget)) {
				if(dynamic_cast<QTabWidget*>(widget->parentWidget())) // smaller cornerWidget
					return QSize( s.width()+9, s.height()+7 );
				return QSize( s.width()+11, s.height()+7 );
			}
			return KStyle::sizeFromContents (t, widget, s, opt); // KMultiTabBar buttons
		}
		case CT_SpinBox: {
			QSize sz = KStyle::sizeFromContents( t, widget, s, opt );
			int h = sz.height()%2 != 0 ? sz.height()+1 : sz.height();
			return ( QSize( sz.width(), h < 24 ? 24 : h) );
		}
		
		case CT_Header: {
			QSize sz = KStyle::sizeFromContents( t, widget, s, opt );
			return QSize(sz.width(), sz.height()-2);
		}
		
		case CT_PopupMenuItem: {
			if (!widget || opt.isDefault())
				return s;
// 			static const int itemHMargin = 6;
// 			static const int itemFrame = 7;
// 			static const int arrowHMargin = 6;
			
			const QPopupMenu *popup = dynamic_cast<const QPopupMenu *>(widget);
			QMenuItem *mi = opt.menuItem();
			int maxpmw = opt.maxIconWidth();
			int w = s.width(), h = s.height(); // includes pixmap width
			int cmwidth = 16;
			
			
			if (mi->custom()) {
				w = mi->custom()->sizeHint().width();
				h = mi->custom()->sizeHint().height();
				if (!mi->custom()->fullSpan() )
					h += 4;
			} else if (mi->widget()) {
			// don't change the size in this case.
			} else if (mi->isSeparator()) {
				w = 20;
				h = 5;
			} else {
				if (mi->pixmap()) {
					h = QMAX(h, mi->pixmap()->height() + 2);
				} else {
					h = QMAX(h, 16 + 2 );
					h = QMAX(h, popup->fontMetrics().height() + 4 );
				}

				if (mi->iconSet()) {
					h = QMAX(h, mi->iconSet()->pixmap(QIconSet::Small, QIconSet::Normal).height() + 2);
				}
			}
			
			if(_indentPopupMenuItems) {
				w += itemHMargin + arrowHMargin + itemFrame*2;
			}
			else {
				w += itemFrame*2;
				
				if (mi->popup()) {
					w += itemHMargin + arrowHMargin;
				}
				if (maxpmw) {
					w += itemHMargin;
				}
			}
			
			int t;
			if (!mi->text().isNull() && (t = mi->text().find('\t')) && t >= 0 && mi->text().at(t+1) != 0) {
				// space between text and accel + checkmark width (in guiStyle "Windows" isCheckable() returns always true...)
				w += 18+cmwidth;
			}
			else
				w += cmwidth;
			
			return QSize(w, h);
		}
		default:
			return KStyle::sizeFromContents (t, widget, s, opt);
	}

	return KStyle::sizeFromContents (t, widget, s, opt);
}

int DominoStyle::styleHint( StyleHint stylehint,
								   const QWidget *widget,
								   const QStyleOption &option,
								   QStyleHintReturn* returnData ) const
{
	switch (stylehint) {
		case SH_PopupMenu_SubMenuPopupDelay:
			//return 96; // Motif-like delay...
			return 250;
		case SH_GroupBox_TextLabelVerticalAlignment: {
			return Qt::AlignTop;
		}
		case SH_ScrollBar_BackgroundMode: {
			return Qt::PaletteBackground;
		}
		case SH_ScrollBar_ScrollWhenPointerLeavesControl:
			// if set to false (the default) it will also scroll when the subPage button is pressed, so no choice here (Qt 3.6)
			return true;
		case SH_TabBar_Alignment: {
			return _centerTabs ? AlignHCenter : AlignLeft;
		}
		case SH_ToolBox_SelectedPageTitleBold: {
			return false;
		}
		case SH_LineEdit_PasswordCharacter:
		{
			if (widget) {
				const QFontMetrics &fm = widget->fontMetrics();
				if (fm.inFont(QChar(0x25CF))) {
					return 0x25CF;
				} else if (fm.inFont(QChar(0x2022))) {
					return 0x2022;
				}
			}
			return '*';
		}
		default:
			return KStyle::styleHint(stylehint, widget, option, returnData);
	}
}

QPixmap DominoStyle::stylePixmap(StylePixmap stylepixmap, const QWidget* widget, const QStyleOption& opt) const {
	
	switch (stylepixmap) {
		case SP_DockWindowCloseButton:
		case SP_TitleBarCloseButton:
			return QPixmap(qembed_findImage("DockCloseButton").copy(0,0,12,12));
		case SP_TitleBarMinButton:
			return QPixmap(qembed_findImage("DockbackButton").copy(0,0,12,12));
		case SP_TitleBarMaxButton:
			return QPixmap(qembed_findImage("OverlapButton").copy(0,0,12,12));
		case SP_TitleBarNormalButton:
			return QPixmap(qembed_findImage("ToDesktopButton").copy(0,0,12,12));
		default:
			return KStyle::stylePixmap(stylepixmap, widget, opt);
	}
}


QColor DominoStyle::lightenColor(const QColor &color, const int value) const
{
	if(value < 0)
		return darkenColor(color, QABS(value));
	int r, g, b;
	color.getRgb(&r, &g, &b);
	r += value;
	if(r > 255)
		r =255;
	g += value;
	if(g > 255)
		g =255;
	b += value;
	if(b > 255)
		b =255;
	return QColor(r, g, b);
}

QColor DominoStyle::darkenColor(const QColor &color, const int value) const
{
	if(value < 0)
		return lightenColor(color, QABS(value));
	int r, g, b;
	color.getRgb(&r, &g, &b);
	r -= value;
	if(r < 0)
		r = 0;
	g -= value;
	if(g < 0)
		g = 0;
	b -= value;
	if(b < 0)
		b = 0;
	return QColor(r, g, b);
}

QColor DominoStyle::dGetColor(const QColor &color, const int value) const
{
	int h, s, v;
	color.getHsv(&h, &s, &v);
	return (v<127)? lightenColor(color, value): darkenColor(color, value);
}


QPixmap* DominoStyle::renderLineEditShadow(QWidget* widget, QRect r, QColor bg, uint flags, bool /*reverse*/) {
	
	bool drawLeft = flags& Draw_Left;
	bool drawRight = flags& Draw_Right;
	
	int x, y, w, h;
	r.rect(&x, &y, &w, &h);
	if(!drawLeft) {
		x = -3;
		w += 3;
	}
	else if(!drawRight)
		w += 3;
	QRect rect(x, y, w, h);
	
	
	QColor tintColor;
	QString enabled = widget->isEnabled() ? "1" : "0";
	if(!widget->isEnabled())
		tintColor =bg;
	else
		tintColor = alphaBlendColors(Qt::black, bg, 30+35);
	
	QPixmap* prBgShadow;
	if(!(prBgShadow = QPixmapCache::find(QString::number(bg.pixel(), 16)+QString::number(qApp->palette().active().background().pixel(), 16)+"leBgShadow"+enabled))) {
		prBgShadow = new QPixmap(4,4);
		prBgShadow->fill(bg);
		QPixmap pix = tintImage(qembed_findImage("progressShadow2"), tintColor);
		bitBlt(prBgShadow, 0, 0, &pix);
		QPixmapCache::insert(QString::number(bg.pixel(), 16)+QString::number(qApp->palette().active().background().pixel(), 16)+"leBgShadow"+enabled, prBgShadow);
	}
	
	
	QPixmap* pix = new QPixmap(rect.size());
	pix->fill(bg);
	
	if(widget->isEnabled()) {
		QPainter p(pix);
		
		QColor color_d = alphaBlendColors(tintColor, bg, 170);
		QColor color_m = alphaBlendColors(tintColor, bg, 85);
		QColor color_l = alphaBlendColors(tintColor, bg, 25);
		
		p.setPen(color_d);
		p.drawLine(rect.x()+2, rect.y(), rect.right()-2, rect.y());
		p.setPen(color_m);
		p.drawLine(rect.x()+2, rect.y()+1, rect.right()-2, rect.y()+1);
		p.setPen(color_l);
		p.drawLine(rect.x()+2, rect.y()+2, rect.right()-2, rect.y()+2);
		p.drawLine(rect.x()+2, rect.bottom(), rect.right()-2, rect.bottom());
	
		//p.setPen(color_l);
		p.drawLine(rect.x()+1, rect.y()+3, rect.x()+1, rect.bottom()-1); // l
		p.drawLine(rect.right()-1, rect.y()+3, rect.right()-1, rect.bottom()-1); // r
				
		p.setPen(color_m);
		p.drawLine(rect.x(), rect.y()+3, rect.x(), rect.bottom()-1); // l
		p.drawLine(rect.right(), rect.y()+3, rect.right(), rect.bottom()-1); // r
		
		bitBlt(pix, rect.right()-1, rect.y(), prBgShadow, 2, 0, 2, 3, Qt::CopyROP);
		bitBlt(pix, rect.x(), rect.y(), prBgShadow, 0, 0, 2, 3, Qt::CopyROP);
		bitBlt(pix, rect.right()-1, rect.bottom(), prBgShadow, 2, 3, 2, 1, Qt::CopyROP);
		bitBlt(pix, rect.x(), rect.bottom(), prBgShadow, 0, 3, 2, 1, Qt::CopyROP);
		
		p.end();
	}
	return pix;
}

QColor DominoStyle::getGroupBoxColor(const QWidget* w) const {
	QWidget* parent = w->parentWidget();
	int tint = groupBoxSettings.customColor ? 0 : groupBoxSettings.brightness;
	while(dynamic_cast<QGroupBox*>(parent)) {
		parent = parent->parentWidget();
		tint += groupBoxSettings.brightness;
	}
	
	return QColor(darkenColor(groupBoxSettings.customColor ? groupBoxSettings.color :
			qApp->palette().active().background(), tint));
	
}

QPixmap* DominoStyle::createCheckMark(const QColor color) const {
	
	QImage checkMark_enabled = qembed_findImage("checkmark");
	QImage checkMark_disabled =  setImageOpacity(checkMark_enabled, 25);
	
	QImage img(32, 17, 32); // left enabled, right disabled
	img.setAlphaBuffer(true);
	for(int x=0; x < checkMark_enabled.width(); ++x) {
		for(int y=0; y < checkMark_enabled.height(); ++y) {
			img.setPixel(x, y, checkMark_enabled.pixel(x, y));
		}
	}
	for(int x=0; x < checkMark_disabled.width(); ++x) {
		for(int y=0; y < checkMark_disabled.height(); ++y) {
			img.setPixel(x+16, y, checkMark_disabled.pixel(x, y));
		}
	}
	
	return new QPixmap(tintImage(img, color));
}

QPixmap* DominoStyle::createRadioIndicator(const QColor color) const {
	
	QImage dot_enabled = tintImage(qembed_findImage("rb-dot"),color);
	
	blend(qembed_findImage("rb-dot-shadow"), dot_enabled, dot_enabled);
	QImage dot_disabled = setImageOpacity(dot_enabled, 25);

	QImage img(10, 5, 32);
	img.setAlphaBuffer(true);
	for(int x=0; x < dot_enabled.width(); ++x) {
		for(int y=0; y < dot_enabled.height(); ++y) {
			img.setPixel(x, y, dot_enabled.pixel(x, y));
		}
	}
	for(int x=0; x < dot_disabled.width(); ++x) {
		for(int y=0; y < dot_disabled.height(); ++y) {
			img.setPixel(x+5, y, dot_disabled.pixel(x, y));
		}
	}
	
	return new QPixmap(img);
}


void DominoStyle::drawFocusIndicator(QPainter *p, const QRect &r, int flags, const QColorGroup cg, bool /*enabled*/,
			const QPixmap* /*pixmap*/, const QString & text, int /*len = -1*/, const QColor& highlight, bool drawLine, bool isButton) const
{

	if(drawLine) {
// 		p->setPen(cg.foreground());
// 		QPixmap edges = tintImage(qembed_findImage("indicatorEdges"), cg.foreground());
// 		bitBlt(p->device(), r.x(), r.bottom()-1, &edges, 0, 0, 2, 1, Qt::CopyROP);
// 		bitBlt(p->device(), r.right()-2, r.bottom()-1, &edges, 2, 0, 2, 1, Qt::CopyROP);
// 		p->drawLine(r.x()+2, r.bottom()-1, r.right()-3, r.bottom()-1);
		QFont font = p->font();
		font.setUnderline(true);
		p->setFont(font);
	}
	else {
		QRect br = p->fontMetrics().boundingRect(r.x(), r.y(), r.width(), r.height(), flags, text);
		QPainter pixPainter;
		QPixmap textPixmap(br.width()+2, br.height()+2);
		textPixmap.fill(QColor(0,0,0));
		pixPainter.begin(&textPixmap);
		pixPainter.setPen(white);
		pixPainter.setFont(p->font());
		pixPainter.drawText( QRect(1, 1, br.width()-1, br.height()-1), flags, text );
		pixPainter.end();
		
		
		DominoQPixmap* pixm = (DominoQPixmap*)&textPixmap;
		
		uint height = pixm->height();
		uint width = pixm->width();
		
		pixm->data->alphapm = new QPixmap;
		
		DominoQPixmap* alphapm = (DominoQPixmap*)pixm->data->alphapm;
		
		alphapm->data->w = width;
		alphapm->data->h = height;
		alphapm->data->d = 8;
		alphapm->hd = XCreatePixmap(alphapm->x11Display(), RootWindow(alphapm->x11Display(), alphapm->x11Screen()), width, height, 8);
		alphapm->rendhd = (HANDLE) XftDrawCreateAlpha( alphapm->x11Display(), alphapm->hd, 8 );
		
		XImage *axi = XCreateImage(alphapm->x11Display(), (Visual *) alphapm->x11Visual(), 8, ZPixmap, 0, 0, width, height, 8, 0);
		axi->data = (char *) malloc((unsigned) axi->bytes_per_line * height);
		
		XImage* xi = XGetImage( qt_xdisplay(), pixm->handle(), 0, 0, width, height, AllPlanes, ZPixmap );
		
		register int alpha;
		register float AXIS_FACTOR = 2.0;
		register float DIAGONAL_FACTOR = 1.0;
		register double alphaShadow;
		register int customalpha = isButton ? focusIndicatorSettings.buttonOpacity : focusIndicatorSettings.opacity;
		
		for(register uint x = 1; x < width-1; ++x) {
			for(register uint y = 1; y < height-1; ++y) {
				
				alphaShadow = (qBlue(XGetPixel(xi,x-1,y-1)) * DIAGONAL_FACTOR +
						qBlue(XGetPixel(xi,x-1,y  )) * AXIS_FACTOR +
						qBlue(XGetPixel(xi,x-1,y+1)) * DIAGONAL_FACTOR +
						qBlue(XGetPixel(xi,x ,y-1)) * AXIS_FACTOR +
						0                         +
						qBlue(XGetPixel(xi,x ,y+1)) * AXIS_FACTOR +
						qBlue(XGetPixel(xi,x+1,y-1)) * DIAGONAL_FACTOR +
						qBlue(XGetPixel(xi,x+1,y  )) * AXIS_FACTOR +
						qBlue(XGetPixel(xi,x+1,y+1)) * DIAGONAL_FACTOR) / 2.0; // / multiplicationFactor
					
				alpha = (int)alphaShadow;
				XPutPixel(axi, x, y, QMIN(255, alpha) *customalpha >> 8);
			}
		}
			
		GC gc = XCreateGC(alphapm->x11Display(), alphapm->hd, 0, 0);
		XPutImage(alphapm->x11Display(), alphapm->hd, gc, axi, 0, 0, 0, 0, width, height);
		
		textPixmap.fill(highlight);
		
		QRect fr = QRect(br.x(), br.y(), QMIN((int)width-2, r.width()) , QMIN((int)height-2, r.height()));
		if(fr.x() < r.x()) {
			p->setClipRegion(r);
			fr.setWidth(fr.width()+abs(fr.x())+2);
		}
		if(fr.isValid())
			p->drawPixmap(fr.x(), fr.y(), textPixmap, 1, 1, fr.width() , fr.height());
		if(fr.x() < r.x())
			p->setClipping(false);
		
		if(axi)
			XDestroyImage(axi);
		if(xi)
			XDestroyImage(xi);
		XFreeGC(alphapm->x11Display(), gc);
		
		p->setPen(cg.foreground());
	}
}

QPixmap DominoStyle::renderOutlineIcon(const QPixmap* icon) const
{
	QImage img = icon->convertToImage();
	QImage outlineIcon(img.width()+2, img.height()+2, 32);
	outlineIcon.setAlphaBuffer(true);
	
	register unsigned int *outlineIconData = (unsigned int*) outlineIcon.bits();
	register uint total = outlineIcon.width()*outlineIcon.height();
	for (register uint current = 0 ; current < total ; ++current ) {
		outlineIconData[ current ] = 0x00000000;
	}
	
	bitBlt(&outlineIcon, 0, 0, &img);
	bitBlt(&outlineIcon, 1, 0, &img);
	bitBlt(&outlineIcon, 2, 0, &img);
	bitBlt(&outlineIcon, 0, 1, &img);
	bitBlt(&outlineIcon, 2, 1, &img);
	bitBlt(&outlineIcon, 0, 2, &img);
	bitBlt(&outlineIcon, 1, 2, &img);
	bitBlt(&outlineIcon, 2, 2, &img);
	
	register uint customalpha = focusIndicatorSettings.buttonOpacity;
	for (register uint current = 0 ; current < total ; ++current ) {
		outlineIconData[ current ] = qRgba( focusIndicatorSettings.buttonColor.red(), focusIndicatorSettings.buttonColor.green(), focusIndicatorSettings.buttonColor.blue(), qAlpha(outlineIconData[ current ]) * customalpha >> 8);
	}
	
	return QPixmap(outlineIcon);
}




void DominoStyle::drawItem( QPainter *p, const QRect &r,
			    int flags, const QColorGroup &g, bool enabled,
			    const QPixmap *pixmap, const QString& text, int len,
			    const QColor* penColor ) const
{
	dominoDrawItem(p, r, flags, g, enabled, pixmap, text, len, penColor, false);
}

void DominoStyle::dominoDrawItem( QPainter *p, const QRect &r,
		       int flags, const QColorGroup &g, bool enabled,
		       const QPixmap *pixmap, const QString& text, int len,
		       const QColor* penColor, bool buttonTextEffect ) const
{
	
	int x = r.x();
	int y = r.y();
	int w = r.width();
	int h = r.height();

	p->setPen( penColor?*penColor:g.foreground() );
	if ( pixmap ) {
		QPixmap  pm( *pixmap );
		bool clip = (flags & Qt::DontClip) == 0;
		if ( clip ) {
			if ( pm.width() < w && pm.height() < h ) {
				clip = FALSE;
			} else {
				p->save();
				QRegion cr = QRect(x, y, w, h);
				if (p->hasClipping())
					cr &= p->clipRegion(QPainter::CoordPainter);
				p->setClipRegion(cr);
			}
		}
		if ( (flags & Qt::AlignVCenter) == Qt::AlignVCenter )
			y += h/2 - pm.height()/2;
		else if ( (flags & Qt::AlignBottom) == Qt::AlignBottom)
			y += h - pm.height();
		if ( (flags & Qt::AlignRight) == Qt::AlignRight )
			x += w - pm.width();
		else if ( (flags & Qt::AlignHCenter) == Qt::AlignHCenter )
			x += w/2 - pm.width()/2;
		else if ( ((flags & Qt::AlignLeft) != Qt::AlignLeft) && QApplication::reverseLayout() ) // AlignAuto && rightToLeft
			x += w - pm.width();

		p->drawPixmap( x, y, pm );
		if ( clip )
			p->restore();
	}
	else if ( !text.isNull() ) {
		

		if(textEffectSettings.mode > 1 || buttonTextEffect) { // 0=none, 1=only buttons, 2=everywhere
			
			QRect br = p->fontMetrics().boundingRect(x, y, w, h, flags, text);
			QPainter pixPainter;
			QPixmap textPixmap(br.width(), br.height());
			textPixmap.fill(QColor(0,0,0));
			pixPainter.begin(&textPixmap);
			pixPainter.setPen(white);
			pixPainter.setFont(p->font());
			pixPainter.drawText( QRect(0,0,br.width(), br.height()), flags, text );
			pixPainter.end();
			
			DominoQPixmap* pixm = (DominoQPixmap*)&textPixmap;
			
			uint height = pixm->height();
			uint width = pixm->width();
			
			pixm->data->alphapm = new QPixmap;
			
			DominoQPixmap* alphapm = (DominoQPixmap*)pixm->data->alphapm;
			
			alphapm->data->w = width;
			alphapm->data->h = height;
			alphapm->data->d = 8;
			alphapm->hd = XCreatePixmap(alphapm->x11Display(), RootWindow(alphapm->x11Display(), alphapm->x11Screen()), width, height, 8);
			alphapm->rendhd = (HANDLE) XftDrawCreateAlpha( alphapm->x11Display(), alphapm->hd, 8 );
			
			XImage *axi = XCreateImage(alphapm->x11Display(), (Visual *) alphapm->x11Visual(), 8, ZPixmap, 0, 0, width, height, 8, 0);
			axi->data = (char *) malloc((unsigned) axi->bytes_per_line * height);
			XImage* xi = XGetImage( qt_xdisplay(), pixm->handle(), 0, 0, width, height, AllPlanes, ZPixmap );
			
			register int alpha;
			register int customalpha = buttonTextEffect ? textEffectSettings.buttonOpacity : textEffectSettings.opacity;
			for(register uint y = 0; y < height; ++y) {
				for(register uint x = 0; x < width; ++x) {
					alpha = (XGetPixel(xi, x, y)& 0xff ) * customalpha >> 8;  // blue as alpha
					XPutPixel(axi, x, y, alpha);
				}
			}

			GC gc = XCreateGC(alphapm->x11Display(), alphapm->hd, 0, 0);
			XPutImage(alphapm->x11Display(), alphapm->hd, gc, axi, 0, 0, 0, 0, width, height);
			
			
			if(buttonTextEffect) {
				textPixmap.fill(textEffectSettings.buttonColor);
				
				QRect tr = QRect(br.x()+textEffectSettings.buttonPos.x(), br.y()+textEffectSettings.buttonPos.y(), r.width(), r.height());
				if(tr.x() < r.x()) {
					p->setClipRegion(r);
					tr.setWidth(tr.width()+r.x()+abs(tr.x()));
				}
				p->drawPixmap(tr.x(), tr.y(), textPixmap, 0, 0, tr.width(), tr.height());
				if(tr.x() < r.x())
					p->setClipping(false);
			}
			else {
				textPixmap.fill(textEffectSettings.color);
				
				QRect tr = QRect(br.x()+textEffectSettings.pos.x(), br.y()+textEffectSettings.pos.y(), r.width(), r.height());
				if(tr.x() < r.x()) {
					p->setClipRegion(r);
					tr.setWidth(tr.width()+r.x()+abs(tr.x()));
				}
				p->drawPixmap(tr.x(), tr.y(), textPixmap, 0, 0, tr.width(), tr.height());
				if(tr.x() < r.x())
					p->setClipping(false);
			}
			
			if(axi);
				XDestroyImage(axi);
			if(xi)
				XDestroyImage(xi);
			XFreeGC(alphapm->x11Display(), gc);
		}
		
		if(!enabled)
			p->setPen(g.text());
		p->drawText( r.x(), r.y(), r.width(), r.height(), flags, text, len );
		
		
	}
}


QWidget::BackgroundOrigin DominoStyle::setPixmapOrigin(QWidget::BackgroundOrigin origin)
{
		switch(origin) {
			case QWidget::WidgetOrigin:
				return QWidget::ParentOrigin;
			case QWidget::ParentOrigin:
				return QWidget::AncestorOrigin;
			default:
				return origin;
				break;
		}
}

void DominoStyle::setDominoButtonPalette(QWidget* widget, PaletteTyp type) {
	QPalette pal;
	switch(type) {
		case Palette_Application:
			pal = qApp->palette();
			break;
		case Palette_Parent:
			pal = widget->parentWidget()->palette();
			break;
		case Palette_Widget:
			pal = widget->palette();
		default:
			break;
	}
	
	int h, s, v;
	pal.active().buttonText().hsv(&h, &s, &v);
	QColor disabledColor = alphaBlendColors(pal.active().buttonText(), v < 127 ? white : black, 150);
	pal.setColor(QPalette::Disabled, QColorGroup::Text, disabledColor);
	
	widget->setPalette(pal);
}


void DominoStyle::updateTabWidgetFrame() {
	
	QTabWidget* tw = dynamic_cast<QTabWidget*>(sender()->parent());
	QWidget* w = tw ? tw->currentPage() : 0;
	if(!w)
		    return;
	QFrame* f = dynamic_cast<QFrame*>(w->parentWidget());
	if(f) {
		QRect r = f->rect();
		if(tw->tabPosition() == QTabWidget::Top)
			r= QRect(r.x(), r.y(), r.width(), 3);
		else
			r= QRect(r.x(), r.y()+r.height()-4, r.width(), 2);
		f->repaint(r, false);
	}
}

QPixmap DominoStyle::disableIcon(const QPixmap* icon) const {
	
	QImage img = icon->convertToImage();
	register uint pixels = img.depth() > 8 ? img.width()*img.height() :  img.numColors();
	register uint *data = img.depth() > 8 ? (unsigned int *)img.bits() : (unsigned int *)img.colorTable();
	register uint val, i;
	for(i=0; i < pixels; ++i){
		val = qGray(data[i]);
		data[i] = qRgba(val, val, val, qAlpha(data[i]));
	}
	img = setImageOpacity(img, 50);
	
	return QPixmap(img);
}


// same as in renderSurface() but with darker colors
void DominoStyle::renderTabSeparator(QPainter *p, const QRect &r, const QColor buttonColor, bool tabAbove) const {
	
	const DSurface* ds = tabAbove ? &tabTopSurface : &tabBottomSurface;
	
	int top = r.y()+r.height()*ds->g1Top/100;
	int bottom = r.height()*ds->g1Bottom/100;
	int top2 = r.y()+r.height()*ds->g2Top/100;
	int bottom2 = r.height()*ds->g2Bottom/100-top2;
	
	QColor c1;
	QColor c2;
	QColor c3;
	QColor c4;
	QColor bg;
	
	if(buttonColor != qApp->palette().active().button()) {
		c1 = alphaBlendColors(buttonColor, ds->g1Color1, 127);
		c2 = alphaBlendColors(buttonColor, ds->g1Color2, 127);
		c3 = alphaBlendColors(buttonColor, ds->g2Color1, 127);
		c4 = alphaBlendColors(buttonColor, ds->g2Color2, 127);
		bg = alphaBlendColors(buttonColor, ds->background, 127);
	}
	else {
		c1 = ds->g1Color1;
		c2 = ds->g1Color2;
		c3 = ds->g2Color1;
		c4 = ds->g2Color2;
		bg = ds->background;
	}
	
	const int alpha = 35; // 55, 25
	p->fillRect(r, alphaBlendColors(QColor(0,0,0),bg , alpha));
	switch(ds->numGradients) {
		case 1: {
			c1 = alphaBlendColors(QColor(0,0,0), c1, alpha);
			c2 = alphaBlendColors(QColor(0,0,0), c2, alpha);
			renderGradient(p, QRect(r.x(), top, r.width(), bottom), c1, c2, true);
			break;
		}
		case 2: {
			c1 = alphaBlendColors(QColor(0,0,0), c1, alpha);
			c2 = alphaBlendColors(QColor(0,0,0), c2, alpha);
			c3 = alphaBlendColors(QColor(0,0,0), c3, alpha);
			c4 = alphaBlendColors(QColor(0,0,0), c4, alpha);
			renderGradient(p, QRect(r.x(), top, r.width(), bottom), c1, c2, true);
			renderGradient(p, QRect(r.x(), top2, r.width(), bottom2), c3, c4, true);
		}
		default:
			break;
	}
}

void DominoStyle::renderGroupBox(QObject *obj, QEvent *ev) {
	
	QGroupBox* gb = dynamic_cast<QGroupBox*>(obj);
	bool paintEvent = ev->type() == QEvent::Paint;
	QRect r = gb->rect();
	
	if(gb->isFlat()) {
		
		QString title = gb->title();
		int titleHeight = gb->fontMetrics().height();
		int titleWidth = gb->fontMetrics().width(title);
		if(title.contains("&"))
			titleWidth -= gb->fontMetrics().width("&");
		QPainter p(gb);
// 		p.setPen(qApp->palette().active().background().dark(120));
		p.setPen(alphaBlendColors(gb->paletteForegroundColor(), gb->paletteBackgroundColor(), 40));
		p.drawLine(0, titleHeight/2-2, r.width()/2-titleWidth/2-4, titleHeight/2-2);
		p.drawLine(r.width()/2+titleWidth/2+4, titleHeight/2-2, r.width(), titleHeight/2-2);
		
		QColorGroup cg = gb->parentWidget()->colorGroup();
		dominoDrawItem(&p, QRect(r.width()/2-titleWidth/2, -1, r.width(), titleHeight), ShowPrefix, cg, Style_Enabled, 0, title, -1, &cg.foreground(), false);
		p.setPen(cg.foreground());
		((DominoQGroupBox*)gb)->drawContents(&p);
	}
	else {
		QString title = gb->title();
		int titleHeight = gb->fontMetrics().height();
		QColor outsideColor = gb->parentWidget() ? gb->parentWidget()->paletteBackgroundColor() : gb->paletteBackgroundColor();
		QColor insideColor = groupBoxSettings.tintBackground ? getGroupBoxColor(gb) : gb->paletteBackgroundColor();
		bool pixmapExists = groupBoxPixmaps.contains(gb);
		
		if(paintEvent && pixmapExists) {
			QPainter p;
			if(!title.isEmpty()) {
				int titleWidth = gb->fontMetrics().width(title);
				if(title.contains("&"))
					titleWidth -= gb->fontMetrics().width("&");
			
				QPainter p(groupBoxPixmaps[gb]);
				QRect titleRect = QRect((r.width()-titleWidth)/2, -1, r.width(), titleHeight);
				p.fillRect(titleRect, outsideColor);
				dominoDrawItem(&p, titleRect, ShowPrefix, gb->colorGroup(), true, 0, title, -1, &gb->foregroundColor(), false);
				p.end();
			}
			// only needed for the case if a disabled groupbox gets disabled for a second time.
			// Qt3 just changes the palette to disabled without checking if it already is (and removes here the "hidden" background pixmap)
			bitBlt(gb, 0, 0, groupBoxPixmaps[gb]);
			p.begin(gb);
			((DominoQGroupBox*)gb)->drawContents(&p);
			return;
		}
		
		bool isCheckable = gb->isCheckable();
		int x = r.x();
		int y = isCheckable ? r.y()+titleHeight+5 : (title.isNull() || title.isEmpty()) ? QMAX(r.y()-1, 0) : r.y()+titleHeight-1;
		int w = r.width();
		int h = r.height();
		
// 		if(y+16 > h || 16 > w) // don't overlap the pixmaps
// 			return;
			
		
		QPixmap buffer(r.width(), r.height());
		
		QPainter p(&buffer);
		p.fillRect(QRect(0, 0, w, y), outsideColor);
		p.fillRect(QRect(x+3, y+3, w-6, h-6-y), insideColor);
		
		QPixmap* gbPix = 0;
		if(!(gbPix = QPixmapCache::find(QString::number(outsideColor.pixel(), 16)+QString::number(insideColor.pixel(), 16)+"groupbox"))) {
			gbPix = new QPixmap(16, 16);
			gbPix->fill(outsideColor);
			
			QPixmap groupBoxPixMask = qembed_findImage("groupBoxMask");
			groupBoxPixMask.fill(insideColor);
			bitBlt(gbPix, 0, 0, &groupBoxPixMask);
					
			QPixmap groupBoxPix = qembed_findImage("groupBox");
			bitBlt(gbPix, 0, 0, &groupBoxPix);
			
			QPixmapCache::insert(QString::number(outsideColor.pixel(), 16)+QString::number(insideColor.pixel(), 16)+"groupbox", gbPix);
		}
				
		bitBlt(&buffer, 0, y, gbPix, 0, 0, 8, 8);
		bitBlt(&buffer, w-8, y, gbPix, 8, 0, 8, 8);
		bitBlt(&buffer, 0, h-8, gbPix, 0, 8, 8, 8);
		bitBlt(&buffer, w-8, h-8, gbPix, 8, 8, 8, 8 );
		
		// top
		QColor c1 = alphaBlendColors(QColor(255,255,255), outsideColor, 25);
		QColor c2 = alphaBlendColors(QColor(0,0,0), insideColor, 70);
		QColor c3 = alphaBlendColors(QColor(0,0,0), insideColor, 30);
		// left + right
		QColor c4 = alphaBlendColors(QColor(255,255,255), outsideColor, 40);
		QColor c5 = c3; //alphaBlendColors(QColor(0,0,0), insideColor, 30);
		QColor c6 = alphaBlendColors(QColor(0,0,0), insideColor, 4);
		// bottom
		QColor c7 = alphaBlendColors(QColor(6,6,6), insideColor, 25);
		QColor c8 = alphaBlendColors(QColor(255,255,255), outsideColor, 79);
		QColor c9 = alphaBlendColors(QColor(255,255,255), outsideColor, 30);
		
		
		// top
		p.setPen(c1);
		p.drawLine(r.left()+8, y, r.right()-8, y);
		p.setPen(c2);
		p.drawLine(r.left()+8, y+1, r.right()-8, y+1);
		p.setPen(c3);
		p.drawLine(r.left()+8, y+2, r.right()-8, y+2);
		// bottom
		p.setPen(c7);
		p.drawLine(r.left()+8, r.bottom()-2, r.right()-8, r.bottom()-2);
		p.setPen(c8);
		p.drawLine(r.left()+8, r.bottom()-1, r.right()-8, r.bottom()-1);
		p.setPen(c9);
		p.drawLine(r.left()+8, r.bottom(), r.right()-8, r.bottom());
		// left / right
		p.setPen(c4);
		p.drawLine(r.left(), y+8, r.left(), r.bottom()-8);
		p.drawLine(r.right(), y+8, r.right(), r.bottom()-8);
		p.setPen(c5);
		p.drawLine(r.left()+1, y+8, r.left()+1, r.bottom()-8);
		p.drawLine(r.right()-1, y+8, r.right()-1, r.bottom()-8);
		p.setPen(c6);
		p.drawLine(r.left()+2, y+8, r.left()+2, r.bottom()-8);
		p.drawLine(r.right()-2, y+8, r.right()-2, r.bottom()-8);
		
		p.end();
		
		if(ev->type() == QEvent::Resize)
			XSetWindowBackgroundPixmap( gb->x11Display(), gb->winId(), buffer.handle());
		else if(paintEvent) {
			if(!title.isEmpty()) {
				int titleWidth = gb->fontMetrics().width(title);
				if(title.contains("&"))
					titleWidth -= gb->fontMetrics().width("&");
				
				p.begin(&buffer);
				QRect titleRect = QRect((r.width()-titleWidth)/2, -1, r.width(), titleHeight);
				p.fillRect(titleRect, outsideColor);
				dominoDrawItem(&p, titleRect, ShowPrefix, gb->colorGroup(), true, 0, title, -1, &gb->foregroundColor(), false);
				p.end();
			}
			bitBlt(gb, 0, 0, &buffer);
			
			p.begin(gb);
			((DominoQGroupBox*)gb)->drawContents(&p);
		}
		
		
		if(pixmapExists)
			delete groupBoxPixmaps[gb];
		groupBoxPixmaps[gb] = new QPixmap(buffer);
		
	}
}

void DominoStyle::spinBoxValueChanged(int value) {
	QSpinBox* sb = (QSpinBox*)sender();
	if(spinWidgetDown) {
		spinWidgetDown = !(value == sb->maxValue() || value == sb->minValue());
		if(!spinWidgetDown) {
			// if we repaint directly, the last mousePressEvent wouldn't be down
			QTimer::singleShot( 200, ((DominoQSpinBox*)sb)->editor()->parentWidget(), SLOT(repaint()) );
		}
	}
}

uint DominoStyle::preMultiplyColor(const uint &c) const {
	XRenderColor clr;
	uint  color = c << 8 | qAlpha(c);

	uint red = (color >> 24) & 0xff;
	uint green = (color >> 16) & 0xff;
	uint blue = (color >> 8) & 0xff;
	uint alpha = (color >> 0) & 0xff;

	red = red * alpha / 255;
	green = green * alpha / 255;
	blue = blue * alpha / 255;

	clr.red = (red << 8) + red;
	clr.green = (green << 8) + green;
	clr.blue = (blue << 8) + blue;
	clr.alpha = (alpha << 8) + alpha;
	
	return qRgba(clr.red, clr.green, clr.blue, clr.alpha);
}

void DominoStyle::toolPopupPressed() {
	QToolButton* tb = (QToolButton*)sender();
	if(!tb )
		return;
	if(tb->popup() && tb->isEnabled() && tb->isDown()) {
		popupButton = tb;
		connect(popupTimer, SIGNAL(timeout()), this, SLOT(popupToolMenu()));
		popupTimer->start(600, true);
	}
}

void DominoStyle::popupToolMenu() {
	if(!popupButton )
		return;
	
	if(popupButton->popup() && !popupButton->popup()->isVisible() && popupButton->isEnabled() && popupButton->isDown()) {
		popupButton->openPopup();
	}
	popupButton = 0;
}

void DominoStyle::toolPopupReleased() {
	if(!popupButton)
		return;
	
	if(popupTimer->isActive()) {
		popupTimer->stop();
		popupButton = 0;
	}
}

#include "eventfilter.cpp"
#include "domino.moc"
