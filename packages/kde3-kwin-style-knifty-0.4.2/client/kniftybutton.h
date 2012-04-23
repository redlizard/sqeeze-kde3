/***************************************************************************
*   Copyright (C) 2003 by Sandro Giessl                                   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/

#ifndef KNIFTYBUTTON_H
#define KNIFTYBUTTON_H

#include <qbutton.h>
#include "knifty.h"

namespace KNifty {

class KniftyClient;

class KniftyButton : public QButton
{
public:
    KniftyButton(KniftyClient *parent=0, const char *name=0, const QString &tip=NULL, ButtonType type = ButtonHelp, int size = 18, int btns = LeftButton);
    ~KniftyButton();

    void setSticky(bool sticky) { isSticky = sticky; repaint(false); }
    void setMaximized(bool maximized) { isMaximized = maximized; repaint(false); }
    bool isTool() const;
    QSize sizeHint() const; ///< Return size hint.
    int lastMousePress() const { return m_lastMouse; }
    void reset() { repaint(false); }
    KniftyClient * client() { return m_client; }

private:
    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void drawButton(QPainter *painter);

private:
    int inverseBwColor(QColor color);
    KniftyClient *m_client;
    int m_lastMouse;
    int m_realizeButtons;

    int m_size;

    ButtonType m_type;
    bool hover;
    bool isSticky, isMaximized;
};

} // namespace KNifty

#endif // KNIFTYBUTTON_H
