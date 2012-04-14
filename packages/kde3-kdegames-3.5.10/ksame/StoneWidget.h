/* Yo Emacs, this is -*- C++ -*- */
/*
 *   ksame 0.4 - simple Game
 *   Copyright (C) 1997,1998  Marcus Kreutzberger
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef _STONEWIDGET
#define _STONEWIDGET

#include <qwidget.h>
#include "StoneField.h"

struct StoneSlice;

class StoneWidget : public QWidget {
    Q_OBJECT

    int modified;
    //     int marked;            // # of marked stones

    int stones_x, stones_y;
    int sizex, sizey;
    int field_width, field_height;

    QPtrList<QPoint> history;
    StoneField stonefield;

    // picture number of stonemovie
    int slice;

    StoneSlice **map;

public:
    StoneWidget( QWidget *parent=0, int x=10,int y=10);
    ~StoneWidget();

    unsigned int board();
    int score();
    int marked();
    QSize size();
    int colors();
    virtual QSize sizeHint() const;

    bool undoPossible() const;

    void newGame(unsigned int board, int colors);
    void reset();
    void unmark();
    int undo(int count=1);

    // test for game end
    bool isGameover();
    // if isGameover(): finished with bonus?
    bool hasBonus();
    // test for unchanged start position
    bool isOriginalBoard();

    virtual void readProperties(KConfig *conf);
    virtual void saveProperties(KConfig *conf);
    int count(int color);

protected:
    void timerEvent( QTimerEvent *e );
    void paintEvent( QPaintEvent *e );
    void mousePressEvent ( QMouseEvent *e);
    void mouseMoveEvent ( QMouseEvent *e);

    // properties of the stone picture
    int stone_width,stone_height; // size of one stone
    int maxcolors;         // number of different stones (Y direction)
    int maxslices;         // number of pictures per movie (X direction)

signals:
    // A new game begins
    void s_newgame();

    void s_colors(int colors);
    void s_board(int board);
    void s_score(int score);
    void s_marked(int m);

    void s_gameover();

    // The stone (x,y) was clicked(removed),
    // all neighbor stones disappear without further signals
    void s_remove(int x,int y);

    void s_sizechanged();
};

#endif // _STONEWIDGET

