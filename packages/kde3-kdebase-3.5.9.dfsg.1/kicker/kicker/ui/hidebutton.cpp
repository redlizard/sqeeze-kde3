/* This file is part of the KDE project
   Copyright (C) 2003-2004 Nadeem Hasan <nhasan@kde.org>
   Copyright (C) 2004 Aaron J. Seigo <aseigo@kde.org>

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

#include "hidebutton.h"

#include <qpainter.h>

#include <kapplication.h>
#include <kcursor.h>
#include <kglobalsettings.h>
#include <kiconeffect.h>
#include <kiconloader.h>
#include <kicontheme.h>
#include <kipc.h>
#include <kstandarddirs.h>

HideButton::HideButton(QWidget *parent, const char *name)
    : QButton(parent, name),
      m_highlight(false),
      m_arrow(Qt::LeftArrow)
{
    setBackgroundOrigin(AncestorOrigin);

    connect(kapp, SIGNAL(settingsChanged(int)), SLOT(slotSettingsChanged(int)));
    connect(kapp, SIGNAL(iconChanged(int)), SLOT(slotIconChanged(int)));

    kapp->addKipcEventMask(KIPC::SettingsChanged);
    kapp->addKipcEventMask(KIPC::IconChanged);

    slotSettingsChanged(KApplication::SETTINGS_MOUSE);
}

void HideButton::drawButton(QPainter *p)
{
    if (m_arrow == Qt::LeftArrow)
    {
        p->setPen(colorGroup().mid());
        p->drawLine(width()-1, 0, width()-1, height());
    }
    else if (m_arrow == Qt::RightArrow)
    {
        p->setPen(colorGroup().mid());
        p->drawLine(0, 0, 0, height());
    }
    else if (m_arrow == Qt::UpArrow)
    {
        p->setPen(colorGroup().mid());
        p->drawLine(0, height()-1, width(), height()-1);
    }
    else if (m_arrow == Qt::DownArrow)
    {
        p->setPen(colorGroup().mid());
        p->drawLine(0, 0, width(), 0);
    }

    drawButtonLabel(p);
}

void HideButton::drawButtonLabel(QPainter *p)
{
    if (pixmap())
    {
        QPixmap pix = m_highlight? m_activeIcon : m_normalIcon;

        if (isOn() || isDown())
        {
            p->translate(2, 2);
        }

        QPoint origin(2, 2);

        if (pix.height() < (height() - 4))
        {
            origin.setY(origin.y() + ((height() - pix.height()) / 2));
        }

        if (pix.width() < (width() - 4))
        {
            origin.setX(origin.x() + ((width() - pix.width()) / 2));
        }

        p->drawPixmap(origin, pix);
    }
}

void HideButton::setPixmap(const QPixmap &pix)
{
    QButton::setPixmap(pix);
    generateIcons();
}

void HideButton::setArrowType(Qt::ArrowType arrow)
{
    m_arrow = arrow;
    switch (arrow)
    {
        case Qt::LeftArrow:
            setPixmap(SmallIcon("1leftarrow"));
        break;

        case Qt::RightArrow:
            setPixmap(SmallIcon("1rightarrow"));
        break;

        case Qt::UpArrow:
            setPixmap(SmallIcon("1uparrow"));
        break;

        case Qt::DownArrow:
        default:
            setPixmap(SmallIcon("1downarrow"));
        break;
    }
}

void HideButton::generateIcons()
{
    if (!pixmap())
    {
        return;
    }

    QImage image = pixmap()->convertToImage();
    image = image.smoothScale(size() - QSize(4, 4), QImage::ScaleMin);

    KIconEffect effect;

    m_normalIcon = effect.apply(image, KIcon::Panel, KIcon::DefaultState);
    m_activeIcon = effect.apply(image, KIcon::Panel, KIcon::ActiveState);
}

void HideButton::slotSettingsChanged(int category)
{
    if (category != KApplication::SETTINGS_MOUSE)
    {
        return;
    }

    bool changeCursor = KGlobalSettings::changeCursorOverIcon();

    if (changeCursor)
    {
        setCursor(KCursor::handCursor());
    }
    else
    {
        unsetCursor();
    }
}

void HideButton::slotIconChanged(int group)
{
    if (group != KIcon::Panel)
    {
        return;
    }

    generateIcons();
    repaint(false);
}

void HideButton::enterEvent(QEvent *e)
{
    m_highlight = true;

    repaint(false);
    QButton::enterEvent(e);
}

void HideButton::leaveEvent(QEvent *e)
{
    m_highlight = false;

    repaint(false);
    QButton::enterEvent(e);
}

void HideButton::resizeEvent(QResizeEvent *)
{
    generateIcons();
}

#include "hidebutton.moc"

// vim:ts=4:sw=4:et
