//
// Author: Remi Villatel <maxilys@tele2.fr>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef PIXMAPS_H
#define PIXMAPS_H

#include <qpixmap.h>
#include <qpainter.h>
#include <qimage.h>


class Pixmaps
{
public:
	static QPixmap *active_button_ground;
	static QPixmap *inactive_button_ground;

	static QImage *active_help_normal;
	static QImage *active_max_normal;
	static QImage *active_restore_normal;
	static QImage *active_min_normal;
	static QImage *active_close_normal;
	static QImage *active_sticky_normal;
	static QImage *active_unsticky_normal;
	static QImage *active_above_normal;
	static QImage *active_unabove_normal;
	static QImage *active_below_normal;
	static QImage *active_unbelow_normal;
	static QImage *active_menu_normal;

	static QImage *active_help_hover;
	static QImage *active_max_hover;
	static QImage *active_restore_hover;
	static QImage *active_min_hover;
	static QImage *active_close_hover;
	static QImage *active_sticky_hover;
	static QImage *active_unsticky_hover;
	static QImage *active_above_hover;
	static QImage *active_unabove_hover;
	static QImage *active_below_hover;
	static QImage *active_unbelow_hover;
	static QImage *active_menu_hover;

	static QImage *active_help_sunken;
	static QImage *active_max_sunken;
	static QImage *active_restore_sunken;
	static QImage *active_min_sunken;
	static QImage *active_close_sunken;
	static QImage *active_sticky_sunken;
	static QImage *active_unsticky_sunken;
	static QImage *active_above_sunken;
	static QImage *active_unabove_sunken;
	static QImage *active_below_sunken;
	static QImage *active_unbelow_sunken;
	static QImage *active_menu_sunken;

	static QImage *inactive_help_normal;
	static QImage *inactive_max_normal;
	static QImage *inactive_restore_normal;
	static QImage *inactive_min_normal;
	static QImage *inactive_close_normal;
	static QImage *inactive_sticky_normal;
	static QImage *inactive_unsticky_normal;
	static QImage *inactive_above_normal;
	static QImage *inactive_unabove_normal;
	static QImage *inactive_below_normal;
	static QImage *inactive_unbelow_normal;
	static QImage *inactive_menu_normal;

	static QImage *inactive_help_hover;
	static QImage *inactive_max_hover;
	static QImage *inactive_restore_hover;
	static QImage *inactive_min_hover;
	static QImage *inactive_close_hover;
	static QImage *inactive_sticky_hover;
	static QImage *inactive_unsticky_hover;
	static QImage *inactive_above_hover;
	static QImage *inactive_unabove_hover;
	static QImage *inactive_below_hover;
	static QImage *inactive_unbelow_hover;
	static QImage *inactive_menu_hover;

	static QImage *inactive_help_sunken;
	static QImage *inactive_max_sunken;
	static QImage *inactive_restore_sunken;
	static QImage *inactive_min_sunken;
	static QImage *inactive_close_sunken;
	static QImage *inactive_sticky_sunken;
	static QImage *inactive_unsticky_sunken;
	static QImage *inactive_above_sunken;
	static QImage *inactive_unabove_sunken;
	static QImage *inactive_below_sunken;
	static QImage *inactive_unbelow_sunken;
	static QImage *inactive_menu_sunken;

	static bool pixmapsCreated;

	static void createPixmaps();
	static void deletePixmaps();

	static void unicity(int* pattern, QImage& work, int rr, int gg, int bb);
};

#endif
