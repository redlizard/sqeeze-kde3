/* This file is part of the KDE project
   Copyright (C) 2003-2004 Nadeem Hasan <nhasan@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef HIDEBUTTON_H
#define HIDEBUTTON_H

#include <qbutton.h>
#include <qpixmap.h>

class HideButton : public QButton
{
  Q_OBJECT

  public:
    HideButton(QWidget *parent, const char *name = 0);
    void setArrowType(Qt::ArrowType arrow);
    void setPixmap(const QPixmap &pix);

  protected:
    void drawButton(QPainter *p);
    void drawButtonLabel(QPainter *p);
    void generateIcons();

    void enterEvent(QEvent *e);
    void leaveEvent( QEvent *e );
    void resizeEvent(QResizeEvent *e);

    bool m_highlight;
    QPixmap m_normalIcon;
    QPixmap m_activeIcon;
    Qt::ArrowType m_arrow;

  protected slots:
    void slotSettingsChanged( int category );
    void slotIconChanged( int group );
};

#endif // HIDEBUTTON_H

// vim:ts=4:sw=4:et
