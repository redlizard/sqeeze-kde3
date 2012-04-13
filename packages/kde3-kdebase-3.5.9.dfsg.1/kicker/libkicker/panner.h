/*****************************************************************

Copyright (c) 1996-2000 the kicker authors. See file AUTHORS.

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

#ifndef __panner_h__
#define __panner_h__

#include <qscrollview.h>

class QBoxLayout;
class KArrowButton;

class KDE_EXPORT Panner : public QScrollView
{
    Q_OBJECT

public:
    Panner( QWidget* parent, const char* name = 0 );
    ~Panner();

    QSize minimumSizeHint() const { return QWidget::minimumSizeHint(); }

    Qt::Orientation orientation() const { return _orient; }
    virtual void setOrientation(Orientation orientation);

public slots:
    virtual void resizeContents( int w, int h );
    void scrollRightDown();
    void scrollLeftUp();

protected:
    void resizeEvent(QResizeEvent *ev);
    void contentsWheelEvent(QWheelEvent *){;}
    void viewportWheelEvent(QWheelEvent *){;}

private:
    void createScrollButtons();
    void updateScrollButtons();

    Orientation       _orient;
    QBoxLayout       *_layout;
    KArrowButton     *_luSB; // Left Scroll Button
    KArrowButton     *_rdSB; // Right Scroll Button
};

#endif
