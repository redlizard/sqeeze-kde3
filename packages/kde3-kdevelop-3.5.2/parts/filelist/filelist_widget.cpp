/***************************************************************************
 *   Copyright (C) 2004 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *   Copyright (C) 2005 by Jens Herden                                     *
 *   jens@kdewebdev.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qheader.h>
#include <qpixmap.h>

#include <kparts/part.h>
#include <klibloader.h>
#include <kurl.h>
#include <kdebug.h>
#include <klocale.h>
#include <kpopupmenu.h>

#include <kdevcore.h>
#include <kdevpartcontroller.h>

#include "projectviewpart.h"
#include "filelist_widget.h"
#include "filelist_item.h"


/**
 * 
 * @param part 
 * @return 
 */
FileListWidget::FileListWidget(ProjectviewPart *part, QWidget *parent)
 : KListView(parent), QToolTip( viewport() ), _part( part )
{
	addColumn( "" );
	header()->hide();
	setRootIsDecorated( false );
	setResizeMode( QListView::LastColumn );
	setAllColumnsShowFocus( true );

	setSelectionMode( QListView::Extended );

//	connect( _part->partController(), SIGNAL( partAdded(KParts::Part*) ), this, SLOT(partAdded(KParts::Part*)) );
//	connect( _part->partController(), SIGNAL( partRemoved(KParts::Part*) ), this, SLOT(partRemoved()) );
	connect( _part->partController(), SIGNAL( partAdded(KParts::Part*) ), this, SLOT(startRefreshTimer()) );
	connect( _part->partController(), SIGNAL( partRemoved(KParts::Part*) ), this, SLOT(startRefreshTimer()) );
	connect( _part->partController(), SIGNAL( activePartChanged(KParts::Part*) ), this, SLOT( activePartChanged(KParts::Part* )) );

	connect( this, SIGNAL( executed( QListViewItem * ) ), this, SLOT( itemClicked( QListViewItem * ) ) );
	connect( this, SIGNAL( returnPressed( QListViewItem * ) ), this, SLOT( itemClicked( QListViewItem * ) ) );

	connect( this, SIGNAL( contextMenuRequested ( QListViewItem *, const QPoint & , int ) ),
		this, SLOT( popupMenu(QListViewItem *, const QPoint & , int ) ) );

	connect( _part->partController(), SIGNAL(documentChangedState(const KURL &, DocumentState)),
		this, SLOT(documentChangedState(const KURL&, DocumentState )) );

	connect( _part->partController(), SIGNAL(partURLChanged(KParts::ReadOnlyPart * )), this, SLOT(refreshFileList()) );

	setItemMargin(10);
	
	connect( &m_refreshTimer, SIGNAL(timeout()), this, SLOT(refreshFileList()) );
	
	startRefreshTimer();
}


FileListWidget::~FileListWidget()
{}

void FileListWidget::startRefreshTimer( )
{
	m_refreshTimer.start( 100, true );
}

void FileListWidget::maybeTip( QPoint const & p )
{
	FileListItem * item = static_cast<FileListItem*>( itemAt( p ) );
	QRect r = itemRect( item );

	if ( item && r.isValid() )
	{
		const QPixmap * pixmap = item->pixmap(0);
		if ( pixmap && ( p.x() <= pixmap->width() ) )
		{
			QString message;
			switch( item->state() )
			{
				case Modified:
					message = i18n("This file has unsaved changes.");
					break;
				case Dirty:
					message = i18n("This file has changed on disk since it was last saved.");
					break;
				case DirtyAndModified:
					message = i18n("Conflict: this file has changed on disk and has unsaved changes.");
					break;
				default:
					message = item->url().prettyURL();
			}

			tip( r, message );
		}
		else
		{
			tip( r, item->url().prettyURL() );
		}
	}
}

FileListItem * FileListWidget::itemForURL( KURL const & url )
{
	FileListItem * item = static_cast<FileListItem*>( firstChild() );
	while ( item )
	{
		if ( item->url() == url )
		{
			return item;
		}
		item = static_cast<FileListItem*>( item->nextSibling() );
	}
	return 0L;
}

void FileListWidget::refreshFileList( )
{
	QStringList selections = storeSelections();
    int scrollbarPos = verticalScrollBar()->value();

	KListView::clear();

	KURL::List list = _part->partController()->openURLs();
	QValueListIterator<KURL> it = list.begin();
	while ( it != list.end() )
	{
		FileListItem * item = new FileListItem( this, *it );
		item->setState( _part->partController()->documentState( *it ) );
		++it;
	}

	restoreSelections( selections );

	if ( selections.isEmpty() && firstChild() )
	{
		firstChild()->setSelected( true );
	}

    verticalScrollBar()->setValue( scrollbarPos );

	activePartChanged( _part->partController()->activePart() );
}

/*
void FileListWidget::partAdded( KParts::Part * part )
{
	KParts::ReadOnlyPart * ro_part = dynamic_cast<KParts::ReadOnlyPart*>( part );
	if ( ro_part )
	{
		new FileListItem( this, ro_part->url() );
	}

	activePartChanged( _part->partController()->activePart() );
}

void FileListWidget::partRemoved()
{
	FileListItem * item = static_cast<FileListItem*>( firstChild() );
	while ( item )
	{
		if ( ! _part->partController()->partForURL( item->url() ) )
		{
			delete item;
			break;
		}
		item = static_cast<FileListItem*>( item->nextSibling() );
	}

	activePartChanged( _part->partController()->activePart() );
}
*/

void FileListWidget::itemClicked( QListViewItem * item )
{
	if ( !item ) return;

	FileListItem * listItem = static_cast<FileListItem*>( item );
	_part->partController()->editDocument( listItem->url() );
}

void FileListWidget::activePartChanged( KParts::Part * part )
{
	KParts::ReadOnlyPart * ro_part = dynamic_cast<KParts::ReadOnlyPart*>( part );
	if ( ro_part )
	{
		FileListItem * item = static_cast<FileListItem*>( firstChild() );
		while ( item )
		{
			if ( item->url() == ro_part->url() )
			{
				FileListItem::setActive( item );
				break;
			}
			item = static_cast<FileListItem*>( item->nextSibling() );
		}
	}
	repaintContents();
}

void FileListWidget::documentChangedState( const KURL & url, DocumentState state )
{
	FileListItem * item = itemForURL( url );
	if ( item )
	{
		item->setState( state );
	}
}

void FileListWidget::popupMenu( QListViewItem * item, const QPoint & p, int )
{
	if ( item )
	{
		KPopupMenu popup;
		popup.insertTitle( i18n("File List") );
		popup.insertItem( i18n("Close Selected"), this, SLOT(closeSelectedFiles()) );
		popup.insertItem( i18n("Save Selected"), this, SLOT(saveSelectedFiles()) );
		popup.insertItem( i18n("Reload Selected"), this, SLOT(reloadSelectedFiles()) );

		FileContext context( getSelectedURLs() );
        _part->core()->fillContextMenu( &popup, &context );

		popup.exec(p);
	}
}

KURL::List FileListWidget::getSelectedURLs( )
{
	KURL::List list;
	FileListItem * item = static_cast<FileListItem*>( firstChild() );
	while ( item )
	{
		if ( item->isSelected() )
		{
			list << item->url();
		}
		item = static_cast<FileListItem*>( item->nextSibling() );
	}
	return list;
}

void FileListWidget::closeSelectedFiles( )
{
	_part->partController()->closeFiles( getSelectedURLs() );
}

void FileListWidget::saveSelectedFiles( )
{
	_part->partController()->saveFiles( getSelectedURLs() );
}

void FileListWidget::reloadSelectedFiles( )
{
	_part->partController()->revertFiles( getSelectedURLs() );
}

QStringList FileListWidget::storeSelections()
{
	QStringList list;
	QListViewItem  * item = firstChild();
	while ( item )
	{
		if ( item->isSelected()  )
		{
			list << item->text(0);
		}
		item = item->nextSibling();
	}
	return list;
}

void FileListWidget::restoreSelections(const QStringList & list)
{
	QListViewItem * item = firstChild();
	while ( item )
	{
		if ( list.contains( item->text(0) ) )
		{
			item->setSelected( true );
		}
		item = item->nextSibling();
	}
}

#include "filelist_widget.moc"

// kate: space-indent off; indent-width 4; tab-width 4; show-tabs off;
