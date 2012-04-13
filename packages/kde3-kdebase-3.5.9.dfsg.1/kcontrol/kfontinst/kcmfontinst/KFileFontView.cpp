////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CKFileFontView
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 31/05/2003
// Version       : $Revision: 469380 $ $Date: 2005-10-11 01:03:01 +0200 (Di, 11 Okt 2005) $
//
////////////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2003
////////////////////////////////////////////////////////////////////////////////

//
// NOTE: HEAVILY copied from kfiledetailview.cpp...
//
//   Copyright (C) 1997 Stephan Kulow <coolo@kde.org>
//                 2000, 2001 Carsten Pfeiffer <pfeiffer@kde.org>
//

#include <qevent.h>
#include <qkeycode.h>
#include <qheader.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <kapplication.h>
#include <kfileitem.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kicontheme.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kurldrag.h>
#include "KFileFontView.h"

#define COL_NAME 0
#define COL_SIZE 1
#define COL_TYPE 2

class CKFileFontView::CKFileFontViewPrivate
{
    public:

    CKFileFontViewPrivate() : itsDropItem(0) {}

    CFontListViewItem *itsDropItem;
    QTimer            itsAutoOpenTimer;
};

CKFileFontView::CKFileFontView(QWidget *parent, const char *name)
              : KListView(parent, name),
                KFileView(),
                d(new CKFileFontViewPrivate())
{
    itsSortingCol = COL_NAME;
    itsBlockSortingSignal = false;
    setViewName(i18n("Detailed View"));

    addColumn(i18n("Name"));
    addColumn(i18n("Size"));
    addColumn(i18n("Type"));
    setShowSortIndicator(true);
    setAllColumnsShowFocus(true);
    setDragEnabled(false);

    connect(header(), SIGNAL(sectionClicked(int)), SLOT(slotSortingChanged(int)));
    connect(this, SIGNAL(returnPressed(QListViewItem *)), SLOT(slotActivate(QListViewItem *)));
    connect(this, SIGNAL(clicked(QListViewItem *, const QPoint&, int)), SLOT(selected( QListViewItem *)));
    connect(this, SIGNAL(doubleClicked(QListViewItem *, const QPoint &, int)), SLOT(slotActivate(QListViewItem *)));
    connect(this, SIGNAL(contextMenuRequested(QListViewItem *, const QPoint &, int)),
	    this, SLOT(slotActivateMenu(QListViewItem *, const QPoint &)));

    // DND
    connect(&(d->itsAutoOpenTimer), SIGNAL(timeout()), this, SLOT(slotAutoOpen()));
    setSelectionMode(KFileView::selectionMode());
    itsResolver = new KMimeTypeResolver<CFontListViewItem, CKFileFontView>(this);
}

CKFileFontView::~CKFileFontView()
{
    delete itsResolver;
    delete d;
}

void CKFileFontView::setSelected(const KFileItem *info, bool enable)
{
    if (info)
    {
        // we can only hope that this casts works
        CFontListViewItem *item = (CFontListViewItem*)info->extraData(this);

        if (item)
	    KListView::setSelected(item, enable);
    }
}

void CKFileFontView::setCurrentItem(const KFileItem *item)
{
    if (item)
    {
        CFontListViewItem *it = (CFontListViewItem*) item->extraData(this);

        if (it)
            KListView::setCurrentItem(it);
    }
}

KFileItem * CKFileFontView::currentFileItem() const
{
    CFontListViewItem *current = static_cast<CFontListViewItem*>(currentItem());

    return current ? current->fileInfo() : NULL;
}

void CKFileFontView::clearSelection()
{
    KListView::clearSelection();
}

void CKFileFontView::selectAll()
{
    if (KFile::NoSelection!=KFileView::selectionMode() && KFile::Single!=KFileView::selectionMode())
        KListView::selectAll(true);
}

void CKFileFontView::invertSelection()
{
    KListView::invertSelection();
}

void CKFileFontView::slotActivateMenu(QListViewItem *item,const QPoint& pos)
{
    if (!item)
        sig->activateMenu(0, pos);
    else
    {
        CFontListViewItem *i = (CFontListViewItem*) item;
        sig->activateMenu(i->fileInfo(), pos);
    }
}

void CKFileFontView::clearView()
{
    itsResolver->m_lstPendingMimeIconItems.clear();
    KListView::clear();
}

void CKFileFontView::insertItem(KFileItem *i)
{
    KFileView::insertItem(i);

    CFontListViewItem *item = new CFontListViewItem((QListView*) this, i);

    setSortingKey(item, i);

    i->setExtraData(this, item);

    if (!i->isMimeTypeKnown())
        itsResolver->m_lstPendingMimeIconItems.append(item);
}

void CKFileFontView::slotActivate(QListViewItem *item)
{
    if (item)
    {
        const KFileItem *fi = ((CFontListViewItem*)item)->fileInfo();

        if (fi)
            sig->activate(fi);
    }
}

void CKFileFontView::selected(QListViewItem *item)
{
    if (item && !(KApplication::keyboardMouseState() & (ShiftButton|ControlButton)) &&
         KGlobalSettings::singleClick())
    {
        const KFileItem *fi = ((CFontListViewItem*)item)->fileInfo();

        if (fi && (fi->isDir() || !onlyDoubleClickSelectsFiles()))
            sig->activate(fi);
    }
}

void CKFileFontView::highlighted( QListViewItem *item )
{
    if (item)
    {
        const KFileItem *fi = ((CFontListViewItem*)item)->fileInfo();

        if (fi)
            sig->highlightFile(fi);
    }
}

void CKFileFontView::setSelectionMode(KFile::SelectionMode sm)
{
    disconnect(SIGNAL(selectionChanged()), this);
    disconnect(SIGNAL(selectionChanged(QListViewItem *)), this);

    switch (sm)
    {
        case KFile::Multi:
            QListView::setSelectionMode(QListView::Multi);
            break;
        case KFile::Extended:
            QListView::setSelectionMode(QListView::Extended);
            break;
        case KFile::NoSelection:
            QListView::setSelectionMode(QListView::NoSelection);
            break;
        default: // fall through
        case KFile::Single:
            QListView::setSelectionMode(QListView::Single);
            break;
    }

    // for highlighting
    if (KFile::Multi==sm || KFile::Extended==sm)
        connect(this, SIGNAL(selectionChanged()), SLOT(slotSelectionChanged()));
    else
        connect(this, SIGNAL(selectionChanged(QListViewItem *)), SLOT(highlighted(QListViewItem * )));
}

bool CKFileFontView::isSelected(const KFileItem *i) const
{
    if (!i)
        return false;
    else
    {
        CFontListViewItem *item = (CFontListViewItem*) i->extraData(this);

        return (item && item->isSelected());
    }
}

void CKFileFontView::updateView(bool b)
{
    if (b)
    {
        QListViewItemIterator it((QListView*)this);

        for (; it.current(); ++it)
        {
            CFontListViewItem *item=static_cast<CFontListViewItem *>(it.current());

            item->setPixmap(0, item->fileInfo()->pixmap(KIcon::SizeSmall));
        }
    }
}

void CKFileFontView::updateView(const KFileItem *i)
{
    if (i)
    {
        CFontListViewItem *item = (CFontListViewItem*) i->extraData(this);

        if (item)
        {
            item->init();
            setSortingKey(item, i);
        }
    }
}

void CKFileFontView::setSortingKey(CFontListViewItem *item, const KFileItem *i)
{
    QDir::SortSpec spec = KFileView::sorting();

    if (spec&QDir::Size)
        item->setKey(sortingKey(i->size(), i->isDir(), spec));
    else
        item->setKey(sortingKey(i->text(), i->isDir(), spec));
}

void CKFileFontView::removeItem(const KFileItem *i)
{
    if (i)
    {
        CFontListViewItem *item = (CFontListViewItem*) i->extraData(this);

        itsResolver->m_lstPendingMimeIconItems.remove(item);
        delete item;

        KFileView::removeItem(i);
    }
}

void CKFileFontView::slotSortingChanged(int col)
{
    QDir::SortSpec sort = sorting();
    int sortSpec = -1;
    bool reversed = col == itsSortingCol && (sort & QDir::Reversed) == 0;
    itsSortingCol = col;

    switch(col)
    {
        case COL_NAME:
            sortSpec = (sort & ~QDir::SortByMask | QDir::Name);
            break;
        case COL_SIZE:
            sortSpec = (sort & ~QDir::SortByMask | QDir::Size);
            break;
        // the following columns have no equivalent in QDir, so we set it
        // to QDir::Unsorted and remember the column (itsSortingCol)
        case COL_TYPE:
            sortSpec = (sort & ~QDir::SortByMask | QDir::Time);
            break;
        default:
            break;
    }

    if (reversed)
        sortSpec|=QDir::Reversed;
    else
        sortSpec&=~QDir::Reversed;

    if (sort & QDir::IgnoreCase)
        sortSpec|=QDir::IgnoreCase;
    else
        sortSpec&=~QDir::IgnoreCase;

    KFileView::setSorting(static_cast<QDir::SortSpec>(sortSpec));

    KFileItem             *item;
    KFileItemListIterator it(*items());

    if ( sortSpec & QDir::Size )
    {
        for (; (item = it.current()); ++it )
        {
            CFontListViewItem *i = viewItem(item);
            i->setKey(sortingKey(item->size(), item->isDir(), sortSpec));
        }
    }
    else
        for (; (item = it.current()); ++it )
        {
            CFontListViewItem *i = viewItem(item);

            i->setKey(sortingKey(i->text(itsSortingCol), item->isDir(), sortSpec));
        }

    KListView::setSorting(itsSortingCol, !reversed);
    KListView::sort();

    if (!itsBlockSortingSignal)
        sig->changeSorting( static_cast<QDir::SortSpec>( sortSpec ) );
}

void CKFileFontView::setSorting(QDir::SortSpec spec)
{
    if (spec & QDir::Size)
        itsSortingCol=COL_SIZE;
    else
        itsSortingCol=COL_NAME;

    // inversed, because slotSortingChanged will reverse it
    if (spec & QDir::Reversed)
        spec = (QDir::SortSpec) (spec & ~QDir::Reversed);
    else
        spec = (QDir::SortSpec) (spec | QDir::Reversed);

    KFileView::setSorting((QDir::SortSpec) spec);

    // don't emit sortingChanged() when called via setSorting()
    itsBlockSortingSignal = true; // can't use blockSignals()
    slotSortingChanged(itsSortingCol);
    itsBlockSortingSignal = false;
}

void CKFileFontView::ensureItemVisible(const KFileItem *i)
{
    if (i)
    {
        CFontListViewItem *item = (CFontListViewItem*) i->extraData(this);
        
        if ( item )
            KListView::ensureItemVisible(item);
    }
}

// we're in multiselection mode
void CKFileFontView::slotSelectionChanged()
{
    sig->highlightFile(NULL);
}

KFileItem * CKFileFontView::firstFileItem() const
{
    CFontListViewItem *item = static_cast<CFontListViewItem*>(firstChild());

    return item ? item->fileInfo() : NULL;
}

KFileItem * CKFileFontView::nextItem(const KFileItem *fileItem) const
{
    if (fileItem)
    {
        CFontListViewItem *item = viewItem(fileItem);

        return item && item->itemBelow() ? ((CFontListViewItem*) item->itemBelow())->fileInfo() : NULL;
    }

    return firstFileItem();
}

KFileItem * CKFileFontView::prevItem(const KFileItem *fileItem) const
{
    if (fileItem)
    {
        CFontListViewItem *item = viewItem(fileItem);

        return item && item->itemAbove() ? ((CFontListViewItem*) item->itemAbove())->fileInfo() : NULL;
    }

    return firstFileItem();
}

void CKFileFontView::keyPressEvent(QKeyEvent *e)
{
    KListView::keyPressEvent(e);

    if (Key_Return==e->key() || Key_Enter==e->key())
        if (e->state() & ControlButton)
            e->ignore();
        else
            e->accept();
}

//
// mimetype determination on demand
//
void CKFileFontView::mimeTypeDeterminationFinished()
{
    // anything to do?
}

void CKFileFontView::determineIcon(CFontListViewItem *item)
{
    item->fileInfo()->determineMimeType();
    updateView(item->fileInfo());
}

void CKFileFontView::listingCompleted()
{
    itsResolver->start();
}

QDragObject *CKFileFontView::dragObject()
{
    // create a list of the URL:s that we want to drag
    KURL::List            urls;
    KFileItemListIterator it(* KFileView::selectedItems());
    QPixmap               pixmap;
    QPoint                hotspot;

    for ( ; it.current(); ++it )
        urls.append( (*it)->url() );

    if(urls.count()> 1)
        pixmap = DesktopIcon("kmultiple", KIcon::SizeSmall);
    if(pixmap.isNull())
        pixmap = currentFileItem()->pixmap(KIcon::SizeSmall);

    hotspot.setX(pixmap.width() / 2);
    hotspot.setY(pixmap.height() / 2);

    QDragObject *dragObject=new KURLDrag(urls, widget());

    if(dragObject)
        dragObject->setPixmap(pixmap, hotspot);

    return dragObject;
}

void CKFileFontView::slotAutoOpen()
{
    d->itsAutoOpenTimer.stop();

    if(d->itsDropItem)
    {
        KFileItem *fileItem = d->itsDropItem->fileInfo();

        if (fileItem && !fileItem->isFile() && (fileItem->isDir() || fileItem->isLink()))
            sig->activate(fileItem);
    }
}

bool CKFileFontView::acceptDrag(QDropEvent *e) const
{
#if 0   // Following doesn't seem to work, why???
    bool       ok=false;
    KURL::List urls;


    if((e->source()!=const_cast<CKFileFontView *>(this)) &&
       (QDropEvent::Copy==e->action() || QDropEvent::Move==e->action()) &&
       KURLDrag::decode(e, urls) && !urls.isEmpty())
    {
        KURL::List::Iterator it;

        ok=true;
        for(it=urls.begin(); ok && it!=urls.end(); ++it)
            if(!CFontEngine::isAFontOrAfm(QFile::encodeName((*it).path())))
                ok=false;
    }

    return ok;
#endif

    return KURLDrag::canDecode(e) && (e->source()!= const_cast<CKFileFontView*>(this)) &&
           (QDropEvent::Copy==e->action() || QDropEvent::Move==e->action());
}

void CKFileFontView::contentsDragEnterEvent(QDragEnterEvent *e)
{
    if (!acceptDrag(e)) // can we decode this ?
        e->ignore();            // No
    else
    {
        e->acceptAction();     // Yes

        if((dropOptions() & AutoOpenDirs))
        {
            CFontListViewItem *item = dynamic_cast<CFontListViewItem*>(itemAt(contentsToViewport(e->pos())));
            if (item)  // are we over an item ?
            {
                d->itsDropItem = item;
                d->itsAutoOpenTimer.start(autoOpenDelay()); // restart timer
            }
            else
            {
                d->itsDropItem = 0;
                d->itsAutoOpenTimer.stop();
            }
        }
    }
}

void CKFileFontView::contentsDragMoveEvent(QDragMoveEvent *e)
{
    if (!acceptDrag(e)) // can we decode this ?
        e->ignore();            // No
    else
    {
        e->acceptAction();     // Yes

        if ((dropOptions() & AutoOpenDirs))
        {
            CFontListViewItem *item = dynamic_cast<CFontListViewItem*>(itemAt(contentsToViewport(e->pos())));

            if (item)  // are we over an item ?
            {
                if (d->itsDropItem != item)
                {
                    d->itsDropItem = item;
                    d->itsAutoOpenTimer.start(autoOpenDelay()); // restart timer
                }
            }
            else
            {
                d->itsDropItem = 0;
                d->itsAutoOpenTimer.stop();
            }
        }
    }
}

void CKFileFontView::contentsDragLeaveEvent(QDragLeaveEvent *)
{
    d->itsDropItem = 0;
    d->itsAutoOpenTimer.stop();
}

void CKFileFontView::contentsDropEvent(QDropEvent *e)
{
    d->itsDropItem = 0;
    d->itsAutoOpenTimer.stop();

    if (!acceptDrag(e)) // can we decode this ?
        e->ignore();            // No
    else
    {
        e->acceptAction();     // Yes

        CFontListViewItem *item = dynamic_cast<CFontListViewItem*>(itemAt(contentsToViewport(e->pos())));
        KFileItem         *fileItem = item ? item->fileInfo() : 0;
        KURL::List        urls;

        emit dropped(e, fileItem);

        if(KURLDrag::decode(e, urls) && !urls.isEmpty())
        {
            emit dropped(e, urls, fileItem ? fileItem->url() : KURL());
            sig->dropURLs(fileItem, e, urls);
        }
    }
}

void CKFileFontView::readConfig(KConfig *kc, const QString &group)
{
    restoreLayout(kc, group.isEmpty() ? QString("CFileFontView") : group);
    slotSortingChanged(sortColumn());
}

void CKFileFontView::writeConfig(KConfig *kc, const QString &group)
{
    saveLayout(kc, group.isEmpty() ? QString("CFileFontView") : group);
}

/////////////////////////////////////////////////////////////////

void CFontListViewItem::init()
{
    CFontListViewItem::setPixmap(COL_NAME, itsInf->pixmap(KIcon::SizeSmall));

    setText(COL_NAME, itsInf->text());
    setText(COL_SIZE, itsInf->isDir() ? "" : KGlobal::locale()->formatNumber(itsInf->size(), 0));
    setText(COL_TYPE, itsInf->mimeComment());
}

void CKFileFontView::virtual_hook(int id, void *data)
{
    KListView::virtual_hook(id, data);
    KFileView::virtual_hook(id, data);
}

#include "KFileFontView.moc"
