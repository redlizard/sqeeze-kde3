/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

/*
  Copyright (C) 2006 Eike Hein <hein@kde.org>
*/

#include "viewtree.h"
#include "viewtreeitem.h"
#include "preferences.h"
#include "chatwindow.h"
#include "server.h"
#include "channel.h"
#include "ircview.h"
#include "konsolepanel.h"

#include <qheader.h>
#include <qdragobject.h>
#include <qlistview.h>
#include <qptrlist.h>
#include <qpoint.h>
#include <qpainter.h>
#include <qtooltip.h>

#include <kdebug.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <kapplication.h>


class ViewTree::ToolTip : public QToolTip
{
    public:
        ToolTip(QWidget *parent, KListView *viewTree);
        virtual ~ToolTip() {}

    protected:
        virtual void maybeTip(const QPoint &pos);

    private:
        KListView* viewTree;
};

ViewTree::ToolTip::ToolTip(QWidget *parent, KListView *viewTree)
    : QToolTip(parent), viewTree(viewTree)
{
}

void ViewTree::ToolTip::maybeTip (const QPoint &pos)
{
    if (!parentWidget() || !viewTree) return;

    ViewTreeItem* view = static_cast<ViewTreeItem*>(viewTree->itemAt(pos));

    if (view && view->isTruncated()) tip(viewTree->itemRect(view), view->getName());
}


ViewTree::ViewTree(QWidget *parent)
    : KListView(parent)
{
    header()->hide();
    setHScrollBarMode(QScrollView::AlwaysOff);

    addColumn(i18n("Tabs"));
    setSortColumn(0);
    setSortOrder(Qt::Ascending);

    setResizeMode(QListView::AllColumns);
    setSelectionModeExt(KListView::Single);
    setRootIsDecorated(false);

    setDragEnabled(true);
    setAcceptDrops(true);
    setDropVisualizer(true);

    setShowToolTips(false);
    m_toolTip = new ViewTree::ToolTip(viewport(), this);

    // Controls whether or not to select the first view added
    // to the tree. Don't do so by default; only when told to
    // by the ViewContainer.
    m_selectFirstView = false;

    m_separator = 0;
    m_specialViewCount = 0;

    m_closeButtonItem = 0;
    m_enableCloseButtonTimer = new QTimer(this);

    m_middleClickItem = 0;

    connect(m_enableCloseButtonTimer, SIGNAL(timeout()), SLOT(enableCloseButton()));
    connect(this, SIGNAL(selectionChanged(QListViewItem*)), SLOT(announceSelection(QListViewItem*)));
    connect(this, SIGNAL(aboutToMove()), SLOT(slotAboutToMoveView()));
    connect(this, SIGNAL(moved()), SLOT(slotMovedView()));
}

ViewTree::~ViewTree()
{
    delete m_toolTip;
    m_toolTip = 0;
    emit setViewTreeShown(false);
}

void ViewTree::updateAppearance()
{
    if (Preferences::customTabFont())
        setFont(Preferences::tabFont());
    else
        setFont(KGlobalSettings::generalFont());

    QColor fg, bg;

    if (Preferences::inputFieldsBackgroundColor())
    {
        fg = Preferences::color(Preferences::ChannelMessage);
        bg = Preferences::color(Preferences::TextViewBackground);
    }
    else
    {
        bg = KGlobalSettings::baseColor();
        fg = KGlobalSettings::textColor();
    }

    setPalette(KApplication::palette());
    setPaletteForegroundColor(fg);
    setPaletteBackgroundColor(bg);
}

void ViewTree::addView(const QString& name, ChatWindow* view, const QIconSet &iconset, bool select, ChatWindow* afterView)
{
    ViewTreeItem* item = 0;
    ViewTreeItem* parent = 0;

    if (view->getType() != ChatWindow::DccChat)
        parent = getParentItemForView(view);

    if (parent)
    {
        if (afterView)
        {
            ViewTreeItem* afterItem = getItemForView(afterView);
            slotAboutToMoveView();
            item = new ViewTreeItem(parent, afterItem, name, view);
            slotMovedView();
        }
        else
            item = new ViewTreeItem(parent, name, view);
    }
    else
        item = new ViewTreeItem(this, name, view);

    if (item)
    {
        if (item->sortLast()) ++m_specialViewCount;

        item->setIcon(iconset.pixmap());

        if (select || m_selectFirstView)
        {
            setSelected(item, true);

            // The work is done - the first view is selected.
            m_selectFirstView = false;
        }

        toggleSeparator();

        // The tree may have been hidden previously as the last
        // view was closed.
        if (isHidden()) emit setViewTreeShown(true);
    }
}

void ViewTree::toggleSeparator()
{
    if (m_separator == 0 && m_specialViewCount > 0 && !(childCount() == m_specialViewCount))
        m_separator = new ViewTreeItem(this);

    if (m_separator && m_specialViewCount == 0)
    {
        delete m_separator;
        m_separator = 0;
    }

    if (m_separator && childCount() == (m_specialViewCount + 1))
    {
        delete m_separator;
        m_separator = 0;
    }

    sort();
}

void ViewTree::removeView(ChatWindow* view)
{
    ViewTreeItem* item = getItemForView(view);

    if (item)
    {
        // During the short delay between a close button-induced view deletion
        // and the actual removal from the list, mouse events may cause our
        // m_enableCloseButtonTimer to be activated again, and removeView() to
        // finish just before its timeout(), causing enableCloseButton() to hit
        // a dangling pointer. Hence, if the item to be deleted is identical to
        // m_closeButtonItem, stop the timer and set the pointer to 0.
        if (item == m_closeButtonItem)
        {
            m_enableCloseButtonTimer->stop();
            m_closeButtonItem = 0;
        }

        if (item->sortLast()) --m_specialViewCount;

        if (item->childCount() > 0)
        {
            while (item->firstChild() != 0)
            {
                ViewTreeItem* firstChild = static_cast<ViewTreeItem*>(item->firstChild());

                delete firstChild;
            }

            delete item;
        }
        else
            delete item;


        toggleSeparator();

        // Hide empty tree.
        if (childCount() == 0)
        {
            emit setViewTreeShown(false);
            m_selectFirstView = true;
        }
    }
}

void ViewTree::selectView(ChatWindow* view)
{
    // Repaint everything.
    triggerUpdate();

    ViewTreeItem* item = getItemForView(view);

    if (item && !item->isSelected())
        setSelected(item, true);
}

void ViewTree::selectFirstView(bool select)
{
    m_selectFirstView = select;
}

void ViewTree::setViewName(ChatWindow* view, const QString& name)
{
    ViewTreeItem* item = getItemForView(view);

    if (item) item->setName(name);
}


void ViewTree::setViewColor(ChatWindow* view, QColor color)
{
    ViewTreeItem* item = getItemForView(view);

    if (item) item->setColor(color);
}

void ViewTree::setViewIcon(ChatWindow* view, const QIconSet &iconset)
{
    ViewTreeItem* item = getItemForView(view);

    if (item) item->setIcon(iconset.pixmap());
}

void ViewTree::announceSelection(QListViewItem* item)
{
    unHighlight();

    ViewTreeItem* newItem = static_cast<ViewTreeItem*>(item);

    emit showView(newItem->getView());
}

bool ViewTree::canMoveViewUp(ChatWindow* view)
{
    ViewTreeItem* item = getItemForView(view);

    if (item)
        return canMoveItemUp(item);

    return false;
}

bool ViewTree::canMoveViewDown(ChatWindow* view)
{
    ViewTreeItem* item = getItemForView(view);

    if (item)
        return canMoveItemDown(item);

    return false;
}

bool ViewTree::canMoveItemUp(ViewTreeItem* item)
{
    if (item->isSeparator())
        return false;

    if (!item->itemAbove())
        return false;

    ViewTreeItem* itemAbove = static_cast<ViewTreeItem*>(item->itemAbove());

    if (item->sortLast() && !itemAbove->sortLast())
        return false;

    if (item->sortLast() && itemAbove->isSeparator())
        return false;

    if (item->depth() > 0 && itemAbove->depth() != item->depth())
        return false;

    return true;
}

bool ViewTree::canMoveItemDown(ViewTreeItem* item)
{
    if (item->isSeparator())
        return false;

    if (!item->itemBelow())
        return false;

    ViewTreeItem* itemBelow = static_cast<ViewTreeItem*>(item->itemBelow());

    if (!item->sortLast() && itemBelow->sortLast())
        return false;

    if (item->depth() > 0 && itemBelow->depth() != item->depth())
        return false;

    if (item->depth() == 0 && !item->sortLast() && itemBelow->depth() > 0)
    {
        int companionsBelow = 0;

        while ((itemBelow = static_cast<ViewTreeItem*>(itemBelow->itemBelow())) != 0)
        {
            if (!itemBelow->sortLast() && itemBelow->depth() == item->depth())
                ++companionsBelow;
        }

        if (!companionsBelow)
            return false;
    }

    return true;
}

void ViewTree::moveViewUp(ChatWindow* view)
{
    ViewTreeItem* item = getItemForView(view);

    if (canMoveItemUp(item))
    {
        ViewTreeItem* itemAbove = static_cast<ViewTreeItem*>(item->itemAbove());

        if (item->depth() == itemAbove->depth())
        {
            int newSortIndex = itemAbove->getSortIndex();
            int oldSortIndex = item->getSortIndex();

            item->setSortIndex(newSortIndex);
            itemAbove->setSortIndex(oldSortIndex);

            sort();
        }
        else if (item->depth() < itemAbove->depth())
        {
            ViewTreeItem* parent = static_cast<ViewTreeItem*>(itemAbove->parent());

            if (parent)
            {
                int newSortIndex = parent->getSortIndex();
                int oldSortIndex = item->getSortIndex();

                item->setSortIndex(newSortIndex);
                parent->setSortIndex(oldSortIndex);

                sort();
            }
        }
    }
}

void ViewTree::moveViewDown(ChatWindow* view)
{
    ViewTreeItem* item = getItemForView(view);

    if (canMoveItemDown(item))
    {
        ViewTreeItem* itemBelow = static_cast<ViewTreeItem*>(item->itemBelow());

        if (item->depth() == itemBelow->depth())
        {
            int newSortIndex = itemBelow->getSortIndex();
            int oldSortIndex = item->getSortIndex();

            item->setSortIndex(newSortIndex);
            itemBelow->setSortIndex(oldSortIndex);

            sort();
        }
        else if (item->depth() < itemBelow->depth())
        {
            while ((itemBelow = static_cast<ViewTreeItem*>(itemBelow->itemBelow())) != 0)
            {
                if (!itemBelow->sortLast() && itemBelow->depth() == item->depth())
                    break;
            }

            int newSortIndex = itemBelow->getSortIndex();
            int oldSortIndex = item->getSortIndex();

            item->setSortIndex(newSortIndex);
            itemBelow->setSortIndex(oldSortIndex);

            sort();
        }
    }
}

void ViewTree::slotAboutToMoveView()
{
    setSortColumn(-1);
}

void ViewTree::slotMovedView()
{
    int newSortIndex = 0;

    ViewTreeItem* tempItem = static_cast<ViewTreeItem*>(this->firstChild());

    while (tempItem)
    {
        tempItem->setSortIndex(newSortIndex);
        ++newSortIndex;
        tempItem = static_cast<ViewTreeItem*>(tempItem->itemBelow());
    }

    setSortColumn(0);

    triggerUpdate();

    emit syncTabBarToTree();
}

void ViewTree::unHighlight()
{
    ViewTreeItem* item = static_cast<ViewTreeItem*>(firstChild());

    while (item)
    {
        item->setHighlighted(false);
        item = static_cast<ViewTreeItem*>(item->itemBelow());
    }
}

void ViewTree::hideCloseButtons(ViewTreeItem* exception)
{
    ViewTreeItem* item = static_cast<ViewTreeItem*>(firstChild());

    if (exception)
    {
        while (item)
        {
            if (item != exception) item->setCloseButtonShown(false);
            item = static_cast<ViewTreeItem*>(item->itemBelow());
        }
    }
    else
    {
        while (item)
        {
            item->setCloseButtonShown(false);
            item = static_cast<ViewTreeItem*>(item->itemBelow());
        }
    }
}

void ViewTree::enableCloseButton()
{
    if (m_closeButtonItem) m_closeButtonItem->setCloseButtonEnabled();
}

bool ViewTree::isAboveIcon(QPoint point, ViewTreeItem* item)
{
    QPoint inItem = point - itemRect(item).topLeft();

    int MARGIN = 2;
    int LED_ICON_SIZE = 14;

    int horizOffset = MARGIN + depthToPixels(item->depth());
    int vertOffset = (item->height() - LED_ICON_SIZE) / 2;

    if ((inItem.x() > horizOffset && inItem.x() < (LED_ICON_SIZE + horizOffset))
        && (inItem.y() > vertOffset && inItem.y() < (LED_ICON_SIZE + vertOffset)))
    {
        return true;
    }
    else
        return false;
}

void ViewTree::contentsMousePressEvent(QMouseEvent* e)
{
    QPoint vp = contentsToViewport(e->pos());

    // Don't allow selecting the separator via the mouse.
    if (itemAt(vp) == m_separator)
        return;

    ViewTreeItem* item = static_cast<ViewTreeItem*>(itemAt(vp));

    // Prevent selection being undone by a stray click into
    // the empty area of the widget by only passing on the
    // mouse event if it's on a list item.
    if (item)
    {
        // Don't change the selected item when the user only
        // wants to get the context menu for a non-selected
        // item.
        if (e->button() == RightButton && !item->isSelected())
            return;

        if (Preferences::closeButtons() && e->button() == LeftButton && isAboveIcon(vp, item))
        {
            m_pressedAboveCloseButton = true;
            if (!item->getCloseButtonEnabled()) KListView::contentsMousePressEvent(e);
        }
        else
        {
            m_pressedAboveCloseButton = false;
            KListView::contentsMousePressEvent(e);
        }
        m_middleClickItem = (Preferences::middleClickClose() && e->button() == MidButton) ? item : 0;
    }
}

void ViewTree::contentsMouseReleaseEvent(QMouseEvent* e)
{
    QPoint vp = contentsToViewport(e->pos());
    ViewTreeItem* item = static_cast<ViewTreeItem*>(itemAt(vp));

    if (!item && e->button() == RightButton)
        return;

    if (item)
    {
        if (Preferences::closeButtons() && e->button() == LeftButton
            && isAboveIcon(vp, item) && m_pressedAboveCloseButton
            && item->getCloseButtonEnabled())
        {
            emit closeView(item->getView());
        }

        if (Preferences::middleClickClose() && e->button() == MidButton
            && item == m_middleClickItem)
        {
            emit closeView(item->getView());

            m_middleClickItem = 0;
        }
    }
    else
        KListView::contentsMouseReleaseEvent(e);
}

void ViewTree::contentsMouseMoveEvent(QMouseEvent* e)
{
    QPoint vp = contentsToViewport(e->pos());
    ViewTreeItem* item = static_cast<ViewTreeItem*>(itemAt(vp));

    if (item && item->isSeparator())
        return;

    // Cancel middle-click close.
    if (item != m_middleClickItem) m_middleClickItem = 0;

    // Allow dragging only with the middle mouse button, just
    // like for the tab bar.
    if ((e->state() & MidButton) == MidButton)
        KListView::contentsMouseMoveEvent(e);
    else if ((e->state() & LeftButton) == LeftButton)
    {
        if (item && (item != selectedItem()) && !item->isSeparator())
            setSelected(item, true);
    }

    if (Preferences::closeButtons())
    {
        if (!(e->state() & LeftButton) && !(e->state() & MidButton) && !(e->state() & RightButton))
        {
            if (item)
            {
                hideCloseButtons(item);

                if (isAboveIcon(vp, item))
                {
                    item->setCloseButtonShown(true);
                    m_closeButtonItem = item;
                    if (!m_enableCloseButtonTimer->isActive())
                        m_enableCloseButtonTimer->start(QApplication::doubleClickInterval(), true);
                }
                else
                {
                    m_closeButtonItem = 0;
                    item->setCloseButtonShown(false);
                    m_enableCloseButtonTimer->stop();
                }
            }
            else
            {
                hideCloseButtons();
            }
        }
    }
}

void ViewTree::contentsContextMenuEvent(QContextMenuEvent* e)
{
    QPoint vp = contentsToViewport(e->pos());
    ViewTreeItem* atpos = static_cast<ViewTreeItem*>(itemAt(vp));

    if (atpos && !atpos->isSeparator())
    {
        if (!atpos->isSelected()) atpos->setHighlighted(true);
        emit showViewContextMenu(atpos->getView(),e->globalPos());
    }

    KListView::contentsContextMenuEvent(e);
}

void ViewTree::contentsWheelEvent(QWheelEvent* e)
{
    if (e->delta() > 0)
        selectUpper(true);
    else
        selectLower(true);

    if (selectedItem())
    {
        ChatWindow* view = static_cast<ViewTreeItem*>(selectedItem())->getView();
        if (view) view->adjustFocus();
    }
}

void ViewTree::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Key_Up)
        selectUpper();
    else if (e->key() == Key_Down)
        selectLower();
    else
    {
        ViewTreeItem* item = static_cast<ViewTreeItem*>(selectedItem());
        if (item && item->getView() && item->getView()->isInsertSupported())
        {
            KApplication::sendEvent(item->getView()->getTextView(), e);
            item->getView()->adjustFocus();
        }
        else if (item && item->getView() && item->getView()->getType() == ChatWindow::Konsole)
        {
            KonsolePanel* panel = static_cast<KonsolePanel*>(item->getView());
            KApplication::sendEvent(panel->getWidget(), e);
            item->getView()->adjustFocus();
        }
    }
}

void ViewTree::selectUpper(bool wrap)
{
    if (!selectedItem()) return;

    ViewTreeItem* itemAbove = static_cast<ViewTreeItem*>(selectedItem()->itemAbove());

    if (itemAbove)
    {
        if (itemAbove->isSeparator())
            itemAbove = static_cast<ViewTreeItem*>(m_separator->itemAbove());

        setSelected(itemAbove, true);
    }
    else
    {
        if (wrap) setSelected(lastItem(), true);
    }

    ensureItemVisible(selectedItem());
}

void ViewTree::selectLower(bool wrap)
{
    if (!selectedItem()) return;

    ViewTreeItem* itemBelow = static_cast<ViewTreeItem*>(selectedItem()->itemBelow());

    if (itemBelow)
    {
        if (itemBelow->isSeparator())
            itemBelow = static_cast<ViewTreeItem*>(m_separator->itemBelow());

        setSelected(itemBelow, true);
    }
    else
    {
        if (wrap) setSelected(firstChild(), true);
    }

    ensureItemVisible(selectedItem());
}

void ViewTree::resizeEvent(QResizeEvent* e)
{
    KListView::resizeEvent(e);

    emit sizeChanged();
}

void ViewTree::findDrop(const QPoint &pos, QListViewItem *&parent, QListViewItem *&after)
{
    QPoint p (contentsToViewport(pos));

    QListViewItem *atpos = itemAt(p);

    QListViewItem *above;

    if (!atpos)
        above = lastItem();
    else
    {
        // Get the closest item before us ('atpos' or the one above, if any).
        if (p.y() - itemRect(atpos).topLeft().y() < (atpos->height()/2))
            above = atpos->itemAbove();
        else
            above = atpos;
    }

    ViewTreeItem* itemAbove = static_cast<ViewTreeItem*>(above);
    ViewTreeItem* dragItem = static_cast<ViewTreeItem*>(selectedItem());

    if (above)
    {
        if (dragItem->sortLast())
        {
            if (itemAbove->sortLast())
            {
                after = itemAbove;
                parent = after->parent();
                return;

            }
            else
            {
                after = m_separator;
                parent = after->parent();
                return;
            }
        }
        else if (dragItem->depth() == 0)
        {
            if (itemAbove->sortLast())
            {
                after = m_separator->itemAbove();
                after = (!after || after->depth() == 0) ? after : after->parent();
                parent = 0L;
                return;
            }
            else if (above->depth() == dragItem->depth())
            {
                after = above;
                parent = 0L;
                return;
            }
            else
            {
                after = above->parent();
                parent = 0L;
                return;
            }
        }
        else
        {
            if (!itemAbove->getView() || itemAbove->sortLast())
            {
                after = getLastChild(dragItem->parent());
                parent = after ? after->parent() : 0L;
                return;
            }
            else if (itemAbove->getView()->getServer() != dragItem->getView()->getServer())
            {
                if (itemIndex(itemAbove) > itemIndex(dragItem))
                {
                    after = getLastChild(dragItem->parent());
                    parent = after ? after->parent() : 0L;
                    return;
                }
                else
                {
                    after = 0L;
                    parent = dragItem->parent();
                    return;
                }
            }
            else
            {
                if (above == dragItem->parent())
                    after = 0L;
                else
                    after = above;

                parent = dragItem->parent();
                return;
            }
        }
    }
    else
    {
        if (dragItem->sortLast())
        {
            after = m_separator;
            parent = after->parent();
            return;
        }
        else if (dragItem->depth() == 0)
        {
            after = 0L;
            parent = 0L;
            return;
        }
        else
        {
            after = 0L;
            parent = dragItem->parent();
            return;
        }
    }

    after = 0L;
    parent = 0L;
}

QDragObject* ViewTree::dragObject()
{
    if (!currentItem())
        return 0;

    QListViewItem* item = selectedItem();

    if (!item->dragEnabled())
        return 0;

    return new QStoredDrag("application/x-qlistviewitem", viewport());
}

QPtrList<ChatWindow> ViewTree::getSortedViewList()
{
    QPtrList<ChatWindow> viewList;

    ViewTreeItem* item = static_cast<ViewTreeItem*>(firstChild());

    while (item)
    {
        if (!item->isSeparator()) viewList.append(item->getView());

        item = static_cast<ViewTreeItem*>(item->itemBelow());
    }

    return viewList;
}

ViewTreeItem* ViewTree::getItemForView(ChatWindow* view)
{
    ViewTreeItem* item = static_cast<ViewTreeItem*>(firstChild());

    while (item)
    {
        if (item->getView() && item->getView()==view)
        {
            return item;
            break;
        }

        item = static_cast<ViewTreeItem*>(item->itemBelow());
    }

    return 0;
}

ViewTreeItem* ViewTree::getParentItemForView(ChatWindow* view)
{
    Server* server = view->getServer();

    ViewTreeItem* item = static_cast<ViewTreeItem*>(firstChild());

    while (item)
    {
        if (item->getViewType() == ChatWindow::Status
            && item->getView()
            && item->getView()->getServer() == server)
        {
            return item;
            break;
        }

        item = static_cast<ViewTreeItem*>(item->itemBelow());
    }

    return 0;
}

ViewTreeItem* ViewTree::getLastChild(QListViewItem* parent)
{
    ViewTreeItem* item = static_cast<ViewTreeItem*>(parent);
    Server* server = item->getView()->getServer();
    ViewTreeItem* lastChild = 0;

    while (item->getView() && item->getView()->getServer() == server)
    {
        lastChild = item;
        item = static_cast<ViewTreeItem*>(item->itemBelow());
    }

    return lastChild;
}

void ViewTree::paintEmptyArea(QPainter* p, const QRect& rect)
{
    KListView::paintEmptyArea(p, rect);

    ViewTreeItem* last = static_cast<ViewTreeItem*>(lastItem());

    if (last && last->isSelected())
    {
        int y = last->itemPos() + last->height();
        int x = visibleWidth();

        if (!rect.contains(x-1, y+2))
            return;

        QColor bgColor  = paletteBackgroundColor();
        QColor selColor = KGlobalSettings::highlightColor();
        QColor midColor = last->mixColor(bgColor, selColor);

        p->setPen(selColor);
        p->drawPoint(x - 1, y);
        p->drawPoint(x - 2, y);
        p->drawPoint(x - 1, y + 1);
        p->setPen(midColor);
        p->drawPoint(x - 3, y);
        p->drawPoint(x - 1, y + 2);
    }
}

#include "viewtree.moc"
