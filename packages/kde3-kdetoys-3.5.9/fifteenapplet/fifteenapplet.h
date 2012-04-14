/*****************************************************************

Copyright (c) 2001 Matthias Elter <elter@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef __fifteenapplet_h__
#define __fifteenapplet_h__

#include "qttableview.h"
#include <qmemarray.h>

#include <kpanelapplet.h>

class QPopupMenu;
class KAboutData;

class PiecesTable : public QtTableView
{
    Q_OBJECT

public:
    PiecesTable(QWidget* parent = 0, const char* name = 0);
    QPopupMenu* popup() { return _menu; }

protected:
    void resizeEvent(QResizeEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);

    void paintCell(QPainter *, int row, int col);

    void initMap();
    void initColors();
    void checkwin();

protected slots:
    void randomizeMap();
    void resetMap();

private:
    QMemArray<int>    _map;
    QMemArray<QColor> _colors;
    QPopupMenu    *_menu;
    int _activeRow, _activeCol;
    bool            _randomized;
};

class FifteenApplet : public KPanelApplet
{
    Q_OBJECT

public:
    FifteenApplet(const QString& configFile, Type t = Stretch, int actions = 0,
                  QWidget *parent = 0, const char *name = 0);

    int widthForHeight(int height) const;
    int heightForWidth(int width) const;

    void about();

private:
    PiecesTable *_table;
    KAboutData   *_aboutData;
};

#endif
