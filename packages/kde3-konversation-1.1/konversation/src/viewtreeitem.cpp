/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

/*
  Copyright (C) 2006 Eike Hein <hein@kde.org>
*/

#include "viewtreeitem.h"
#include "konversationapplication.h"
#include "chatwindow.h"
#include "preferences.h"
#include "images.h"


#include <qpainter.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qtimer.h>

#include <kdebug.h>
#include <kglobalsettings.h>
#include <kstringhandler.h>


int ViewTreeItem::s_availableSortIndex = 0;

ViewTreeItem::ViewTreeItem(QListView* parent, const QString& name, ChatWindow* view)
    : QListViewItem(parent, name)
{
    m_sortIndex = s_availableSortIndex;
    s_availableSortIndex++;

    setView(view);
    setViewType(view->getType());

    m_color = KGlobalSettings::textColor();
    m_customColorSet = false;

    setOpen(true);
    setDragEnabled(true);

    m_isSeparator = false;
    m_isHighlighted = false;
    m_isTruncated = false;

    images = KonversationApplication::instance()->images();
    m_closeButtonShown = false;
    m_closeButtonEnabled = false;
}

ViewTreeItem::ViewTreeItem(QListViewItem* parent, const QString& name, ChatWindow* view, int sortIndex)
    : QListViewItem(parent, name)
{
    if (sortIndex != -1)
        setSortIndex(sortIndex);
    else
    {
        m_sortIndex = s_availableSortIndex;
        s_availableSortIndex++;
    }

    setView(view);
    setViewType(view->getType());

    m_color = KGlobalSettings::textColor();
    m_customColorSet = false;

    setOpen(true);
    setDragEnabled(true);

    m_isSeparator = false;
    m_isHighlighted = false;
    m_isTruncated = false;
    m_customColorSet = false;

    images = KonversationApplication::instance()->images();
    m_closeButtonShown = false;
    m_closeButtonEnabled = false;
}

ViewTreeItem::ViewTreeItem(QListViewItem* parent, QListViewItem* afterItem, const QString& name, ChatWindow* view)
    : QListViewItem(parent, afterItem, name)
{
    m_sortIndex = s_availableSortIndex;
    s_availableSortIndex++;

    setView(view);
    setViewType(view->getType());

    m_color = KGlobalSettings::textColor();
    m_customColorSet = false;

    setOpen(true);
    setDragEnabled(true);

    m_isSeparator = false;
    m_isHighlighted = false;
    m_isTruncated = false;
    m_customColorSet = false;

    images = KonversationApplication::instance()->images();
    m_closeButtonShown = false;
    m_closeButtonEnabled = false;
}

ViewTreeItem::ViewTreeItem(QListView* parent) : QListViewItem(parent)
{
    setView(0);
    setOpen(true);
    setDragEnabled(false);

    m_isSeparator = true;
    m_isHighlighted = false;
    m_isTruncated = false;
}

ViewTreeItem::~ViewTreeItem()
{
}

void ViewTreeItem::setSortIndex(int newSortIndex)
{
    m_sortIndex = newSortIndex;
}

int ViewTreeItem::getSortIndex() const
{
    return m_sortIndex;
}

void ViewTreeItem::setName(const QString& name)
{
    setText(0, name);
}

QString ViewTreeItem::getName() const
{
    return text(0);
}

bool ViewTreeItem::isTruncated() const
{
    return m_isTruncated;
}

void ViewTreeItem::setView(ChatWindow* view)
{
    m_view = view;
}

ChatWindow* ViewTreeItem::getView() const
{
    return m_view;
}

void ViewTreeItem::setViewType(ChatWindow::WindowType viewType)
{
    m_viewType = viewType;
}

ChatWindow::WindowType ViewTreeItem::getViewType() const
{
    return m_viewType;
}

void ViewTreeItem::setColor(QColor color)
{
    if (color != m_color)
    {
        m_color = color;
        m_customColorSet = true;
        repaint();
    }
}

QColor ViewTreeItem::getColor() const
{
    if (!m_customColorSet)
    {
        if (Preferences::inputFieldsBackgroundColor())
            return Preferences::color(Preferences::ChannelMessage);
        else
            return KGlobalSettings::textColor();
    }
    else
        return m_color;
}

void ViewTreeItem::setIcon(const QPixmap& pm)
{
    m_oldPixmap = pm;
    if (!m_closeButtonShown) setPixmap(0, pm);
}

void ViewTreeItem::setHighlighted(bool highlight)
{
    if (m_isHighlighted != highlight)
    {
        m_isHighlighted = highlight;
        repaint();
    }
}

void ViewTreeItem::setCloseButtonShown(bool show)
{
    if (!show && !m_closeButtonShown)
        return;

    if (show && m_closeButtonShown)
        return;

    if (show && !m_closeButtonShown)
    {
        setPixmap(0, images->getDisabledCloseIcon());
        m_closeButtonShown = true;
        m_closeButtonEnabled = false;
    }

    if (!show && m_closeButtonShown)
    {
        setPixmap(0, m_oldPixmap);
        m_closeButtonShown = false;
        m_closeButtonEnabled = false;
    }
}

void ViewTreeItem::setCloseButtonEnabled()
{
    if (m_closeButtonShown)
    {
        m_closeButtonEnabled = true;
        setPixmap(0, images->getCloseIcon());
    }
}

bool ViewTreeItem::getCloseButtonEnabled()
{
    return m_closeButtonEnabled;
}

bool ViewTreeItem::sortLast() const
{
    if (!m_isSeparator)
    {
        if (getViewType() == ChatWindow::Status
            || getViewType() == ChatWindow::Channel
            || getViewType() == ChatWindow::Query
            || getViewType() == ChatWindow::RawLog
            || getViewType() == ChatWindow::DccChat
            || getViewType() == ChatWindow::ChannelList)
        {
            return false;
        }
        else
            return true;
    }
    else
    {
        return true;
    }
}

bool ViewTreeItem::isSeparator() const
{
    return m_isSeparator;
}

int ViewTreeItem::compare(QListViewItem *i, int /* col */, bool /* ascending */) const
{
    ViewTreeItem* item = static_cast<ViewTreeItem*>(i);

    if (sortLast() == item->sortLast())
    {
        if (isSeparator() && !item->isSeparator())
            return -1;
        if (!isSeparator() && item->isSeparator())
            return 1;
        else
        {
            if (getSortIndex() == item->getSortIndex())
                return 0;
            else if (getSortIndex() < item->getSortIndex())
                return -1;
            else
                return 1;
        }
    }
    else if (sortLast())
        return 1;
    else
        return -1;
}

void ViewTreeItem::setup()
{
    widthChanged();

    if (!m_isSeparator)
    {
        int LED_ICON_SIZE = 14;
        int MARGIN = 2;

        QRect textRect = listView()->fontMetrics().boundingRect(0, 0, /*width=*/1, 500000, Qt::AlignAuto | Qt::AlignTop | Qt::ShowPrefix, text(/*column=*/0));
        int height = MARGIN + kMax(LED_ICON_SIZE, textRect.height()) + MARGIN;
        setHeight(height);
    }
    else
        setHeight(11);
}

QColor ViewTreeItem::mixColor(const QColor &color1, const QColor &color2)
{
    QColor mixedColor;
    mixedColor.setRgb( (color1.red()   + color2.red())   / 2,
                       (color1.green() + color2.green()) / 2,
                       (color1.blue()  + color2.blue())  / 2 );
    return mixedColor;
}

void ViewTreeItem::paintFocus(QPainter* /* p */, const QColorGroup& /* cg */, const QRect& /* r */)
{
    // Do nothing.
    return;
}

void ViewTreeItem::paintCell(QPainter* p, const QColorGroup& /* cg */, int /* column */, int width, int /* align */)
{
    // Workaround a Qt bug:
    // When the splitter is moved to hide the tree view and then the application is restarted,
    // Qt try to draw items with a negative size.
    if (width <= 0) return;

    int LED_ICON_SIZE = 14;
    int MARGIN = 2;

    // Bufferize the drawing of items.
    QPixmap buffer(width, height());
    QPainter painter(&buffer);

    QColor textColor = isSelected() ? KGlobalSettings::highlightedTextColor() : getColor();
    QColor background = isSelected() ? KGlobalSettings::highlightColor() : listView()->paletteBackgroundColor();
    if (m_isHighlighted) background = Preferences::inputFieldsBackgroundColor()
        ? Preferences::color(Preferences::AlternateBackground) : KGlobalSettings::alternateBackgroundColor();

    // Fill in background.
    painter.fillRect(0, 0, width, height(), background);

    QColor bgColor  = listView()->paletteBackgroundColor();
    QColor selColor = m_isHighlighted ? background : KGlobalSettings::highlightColor();
    QColor midColor = mixColor(bgColor, selColor);

    int iconWidth = pixmap(0) ? LED_ICON_SIZE : 0;
    int textWidth = width - MARGIN - iconWidth - MARGIN - MARGIN;

    if (!m_isSeparator)
    {
        // Draw the rounded rectangle.
        QRect textRect = listView()->fontMetrics().boundingRect(0, 0, /*width=*/1, 500000, Qt::AlignAuto | Qt::AlignTop | Qt::ShowPrefix, text(/*column=*/0));
        int xRound = MARGIN;
        int yRound = MARGIN;
        int hRound = height() - 2 * MARGIN;
        int wRound = kMin(LED_ICON_SIZE + MARGIN + textRect.width() + hRound/2,  width - MARGIN - MARGIN);

        if (wRound > 0)
        {
            QPixmap buffer(wRound * 2, hRound * 2);
            buffer.fill(background);
            QPainter pBuffer(&buffer);
            QColor colorRound = background;
            pBuffer.setPen(colorRound);
            pBuffer.setBrush(colorRound);

            // If the rectangle is higher than wide, don't overlap ellipses.
            if (wRound > hRound)
            {
                pBuffer.drawEllipse(0,                                 0, hRound * 2, hRound * 2);
                pBuffer.drawEllipse(wRound * 2 - hRound * 2, 0, hRound * 2, hRound * 2);
                pBuffer.fillRect(hRound*2/2, 0, wRound * 2 - hRound * 2, hRound * 2, colorRound);
            }
            else
                pBuffer.drawEllipse(0, 0, wRound * 2, hRound * 2);

            pBuffer.end();
            QImage imageToScale = buffer.convertToImage();
            QPixmap pmScaled;
            pmScaled.convertFromImage(imageToScale.smoothScale(wRound, hRound));
            painter.drawPixmap(xRound, yRound, pmScaled);
            textWidth -= hRound/2;
        }

        if (isSelected() || m_isHighlighted)
        {
            painter.setPen(bgColor);
            painter.drawPoint(0, 0);
            painter.drawPoint(1, 0);
            painter.drawPoint(0, 1);
            painter.drawPoint(0, height() - 1);
            painter.drawPoint(1, height() - 1);
            painter.drawPoint(0, height() - 2);
            painter.setPen(midColor);
            painter.drawPoint(2, 0);
            painter.drawPoint(0, 2);
            painter.drawPoint(2, height() - 1);
            painter.drawPoint(0, height() - 3);
        }
    }

    if (m_isHighlighted)
    {
        selColor = KGlobalSettings::highlightColor();
        midColor = mixColor(bgColor, selColor);
    }

    if (itemBelow() && itemBelow()->isSelected())
    {
        painter.setPen(selColor);
        painter.drawPoint(width - 1, height() - 1);
        painter.drawPoint(width - 2, height() - 1);
        painter.drawPoint(width - 1, height() - 2);
        painter.setPen(midColor);
        painter.drawPoint(width - 3, height() - 1);
        painter.drawPoint(width - 1, height() - 3);
    }

    if (itemAbove() && itemAbove()->isSelected())
    {
        painter.setPen(selColor);
        painter.drawPoint(width - 1, 0);
        painter.drawPoint(width - 2, 0);
        painter.drawPoint(width - 1, 1);
        painter.setPen(midColor);
        painter.drawPoint(width - 3, 0);
        painter.drawPoint(width - 1, 2);
    }

    if (!m_isSeparator)
    {
        // Draw icon.
        if (pixmap(0))
        {
            int yPixmap = (height() - pixmap(0)->height()) / 2;

            int xPixmap = MARGIN;

            if (pixmap(0)->width() < LED_ICON_SIZE)
                xPixmap  = MARGIN + ((LED_ICON_SIZE - pixmap(0)->width()) / 2);

            painter.drawPixmap(xPixmap, yPixmap, *pixmap(0));
        }

        // Enough space left to draw icon and text?
        if (textWidth > 0)
        {
            int xText = MARGIN;

            // Shift by icon width.
            if (pixmap(0))
                xText = MARGIN + LED_ICON_SIZE + MARGIN;

            QString text = getName();

            if (p->fontMetrics().width(text) > textWidth)
            {
                m_isTruncated = true;
                text = KStringHandler::rPixelSqueeze(text, p->fontMetrics(), textWidth);
            }
            else
                m_isTruncated = false;

            painter.setPen(textColor);
            painter.setFont(listView()->font());
            painter.drawText(xText, 0, textWidth, height(), Qt::AlignAuto | Qt::AlignVCenter, text);
        }
    }
    else
    {
        QColor lineColor = Preferences::inputFieldsBackgroundColor()
            ? Preferences::color(Preferences::AlternateBackground) : KGlobalSettings::alternateBackgroundColor();
        painter.setPen(lineColor);
        painter.drawLine(0, 5, width, 5);
    }

    painter.end();

    p->drawPixmap(0, 0, buffer);
}
