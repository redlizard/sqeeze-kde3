//
// Author: Remi Villatel <maxilys@tele2.fr>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef PIXMAPS_H
#define PIXMAPS_H

#include <qpixmap.h>
#include <qpainter.h>
#include <qimage.h>
#include <qcolor.h>

class Pixmaps
{
public:
	static QPixmap *active_help_normal;
	static QPixmap *active_max_normal;
	static QPixmap *active_min_normal;
	static QPixmap *active_close_normal;
	static QPixmap *active_sticky_normal;
	static QPixmap *active_above_normal;
	static QPixmap *active_below_normal;
	static QPixmap *active_menu_normal;

	static QPixmap *active_help_hover;
	static QPixmap *active_max_hover;
	static QPixmap *active_min_hover;
	static QPixmap *active_close_hover;
	static QPixmap *active_sticky_hover;
	static QPixmap *active_above_hover;
	static QPixmap *active_below_hover;
	static QPixmap *active_menu_hover;

	static QPixmap *active_help_sunken;
	static QPixmap *active_max_sunken;
	static QPixmap *active_min_sunken;
	static QPixmap *active_close_sunken;
	static QPixmap *active_sticky_sunken;
	static QPixmap *active_above_sunken;
	static QPixmap *active_below_sunken;
	static QPixmap *active_menu_sunken;

	static QPixmap *inactive_help_normal;
	static QPixmap *inactive_max_normal;
	static QPixmap *inactive_min_normal;
	static QPixmap *inactive_close_normal;
	static QPixmap *inactive_sticky_normal;
	static QPixmap *inactive_above_normal;
	static QPixmap *inactive_below_normal;
	static QPixmap *inactive_menu_normal;

	static QPixmap *inactive_help_hover;
	static QPixmap *inactive_max_hover;
	static QPixmap *inactive_min_hover;
	static QPixmap *inactive_close_hover;
	static QPixmap *inactive_sticky_hover;
	static QPixmap *inactive_above_hover;
	static QPixmap *inactive_below_hover;
	static QPixmap *inactive_menu_hover;

	static QPixmap *inactive_help_sunken;
	static QPixmap *inactive_max_sunken;
	static QPixmap *inactive_min_sunken;
	static QPixmap *inactive_close_sunken;
	static QPixmap *inactive_sticky_sunken;
	static QPixmap *inactive_above_sunken;
	static QPixmap *inactive_below_sunken;
	static QPixmap *inactive_menu_sunken;

	static bool pixmapsCreated;

	enum GradientDirections
	{
		HorizontalGradient =	1,
		VerticalGradient =	2
	};

	static void createPixmaps();
	static void deletePixmaps();

	static void unicity(int* pattern, QImage& work, int rr, int gg, int bb);

	static void renderGradient(QPainter* p, 
					const QRect &r, 
					const QColor &startColor, 
					const QColor &endColor, 
					const int direction, 
					const bool convex = true);

	static void renderDiagonalGradient(QPainter* p, 
					const QRect &r, 
					const QColor &startColor, 
					const QColor &middleColor, 
					const QColor &endColor, 
					const bool convex = true);

// 	static void renderQuadriGradient(QPainter* p, 
// 					const QRect &r, 
// 					const QColor &topLeft, 
// 					const QColor &topRight, 
// 					const QColor &bottomLeft, 
// 					const QColor &bottomRight);

	static void renderZenGradient(QPainter* p, 
					const QRect &r, 
					const QColor &startColor, 
					const QColor &middleColor, 
					const QColor &endColor, 
					const bool convex = true);

	static void renderSurface(QPainter *p,
				 	QRect r,
				 	QColor surfaceColor,
				 	QColor highlightColor,
				 	uint flags, 
					bool active);

	static void renderFlatArea(QPainter* p,
				 	QRect r,
				 	QColor surfaceColor,
					bool active);

	static void renderCarving(QPainter* p,
					QRect r,
					QColor contour,
					bool active);

	static void titleFillGround(QPainter* p,
					QRect r,
					QColor ground,
					QColor titlebar,
					int style);

	static QColor brighter(const QColor baseColor, const int factor);

	static QColor darker(const QColor baseColor, const int factor);

};

#endif
