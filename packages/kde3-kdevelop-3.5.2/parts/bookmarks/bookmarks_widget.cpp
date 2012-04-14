/***************************************************************************
 *   Copyright (C) 2003 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qdict.h>
#include <qheader.h>
#include <qtooltip.h>
#include <qpair.h>
#include <qstylesheet.h>

#include <kparts/part.h>
#include <klibloader.h>
#include <kurl.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kpopupmenu.h>
#include <klocale.h>

#include <kdevpartcontroller.h>
#include <kdevcore.h>


#include "bookmarks_part.h"
#include "bookmarks_widget.h"
#include "bookmarks_config.h"

namespace
{

// shamelessly lifted from kdelibs/kate/part/kateviewhelpers.cpp
static const char* const bookmark_xpm[]={
"12 12 4 1",
"b c #808080",
"a c #000080",
"# c #0000ff",
". c None",
"........###.",
".......#...a",
"......#.##.a",
".....#.#..aa",
"....#.#...a.",
"...#.#.a.a..",
"..#.#.a.a...",
".#.#.a.a....",
"#.#.a.a.....",
"#.#a.a......",
"#...a.......",
".aaa........"};

}

class BookmarkItem : public QListViewItem
{
public:
	BookmarkItem( QListView * parent, KURL const & url )
			: QListViewItem( parent, url.fileName() ),
			_url( url ), _line( -1 ), _isBookmark( false )
	{}

	BookmarkItem( QListViewItem * parent, KURL const & url, QPair<int,QString> mark )
			: QListViewItem( parent, QString::number( mark.first +1 ).rightJustify( 5 ) ),
			_url( url ), _line( mark.first ), _isBookmark( true ) 
	{
		BookmarksWidget * lv = static_cast<BookmarksWidget*>( listView() );
		BookmarksConfig::CodeLineType codeline = lv->config()->codeline();
		
		if ( codeline == BookmarksConfig::Never )
		{
			return;
		}
			
		if ( codeline == BookmarksConfig::Token )
		{
			if ( mark.second.startsWith( lv->config()->token() ) )
			{
				setText( 0, text( 0 ) + "  " + mark.second );
			}
			return;
		}
		
		setText( 0, text( 0 ) + "  " + mark.second );
	}

	KURL url()
	{
		return _url;
	}
	int line()
	{
		return _line;
	}

	QString tipText()
	{
		if ( _isBookmark )
		{
			BookmarksWidget * w = static_cast<BookmarksWidget*> ( listView() );
			QStringList list = w->getContext( _url, _line );
			
			QString code = "<qt><table><tr><td><pre>";
			for ( uint i = 0; i < list.count(); i++)
			{
				QString temp = QStyleSheet::escape( list[i] );
				
				if ( i == (list.count() / 2) )	// count() is always odd
				{
					temp = "<b>" + temp + "</b>";
				}
				code += temp + "\n";
			}
			code += "</pre></td></tr></table></qt>";

			return code;
		}
		else
		{
			return _url.prettyURL();
		}
	}

	bool isBookmark()
	{
		return _isBookmark;
	}

private:
	KURL _url;
	int _line;
	bool _isBookmark;
	QString _code;

};

BookmarksWidget::BookmarksWidget(BookmarksPart *part)
		: KListView(0, "bookmarks widget"), QToolTip( viewport() ),
		_part( part )

{
	addColumn( QString::null );
	header()->hide();
	setRootIsDecorated( true );
	setResizeMode( QListView::LastColumn );
	setAllColumnsShowFocus( true );
// 	setSortColumn( -1 );

	connect( this, SIGNAL( executed( QListViewItem * ) ), this, SLOT( itemClicked( QListViewItem * ) ) );
	connect( this, SIGNAL( returnPressed( QListViewItem * ) ), this, SLOT( itemClicked( QListViewItem * ) ) );
	connect( this, SIGNAL( contextMenuRequested ( QListViewItem *, const QPoint & , int ) ),
		this, SLOT( popupMenu(QListViewItem *, const QPoint & , int ) ) );
}


BookmarksWidget::~BookmarksWidget()
{}

void BookmarksWidget::maybeTip(const QPoint &p)
{
//	kdDebug(0) << "ToolTip::maybeTip()" << endl;
	
	if ( ! _part->config()->toolTip() ) return;

	BookmarkItem * item = dynamic_cast<BookmarkItem*>( itemAt( p ) );
	QRect r = itemRect( item );

	if ( item && r.isValid() )
	{
		tip( r, item->tipText() );
	}
}

void BookmarksWidget::update( QDict<EditorData> const & map )
{
//	kdDebug(0) << "BookmarksWidget::update()" << endl;

	QListView::clear();

	QDictIterator<EditorData> it( map );
	while ( it.current() )
	{
		if ( ! it.current()->marks.isEmpty() )
		{
			createURL( it.current() );
		}
		++it;
	}
}

void BookmarksWidget::updateURL( EditorData * data )
{
//	kdDebug(0) << "BookmarksWidget::updateURL()" << endl;

	// remove the node that contains 'data'
	removeURL( data->url );

	// create it again with new data
	createURL( data );
}

void BookmarksWidget::createURL( EditorData * data )
{
//	kdDebug(0) << "BookmarksWidget::createURL()" << endl;

	if ( data )
	{
		QListViewItem * file = new BookmarkItem( this, data->url );
		file->setOpen( true );
		file->setPixmap( 0, SmallIcon( "document" ) );

		QValueListIterator< QPair<int,QString> > it = data->marks.begin();
		while ( it != data->marks.end() )
		{
			QListViewItem * item = new BookmarkItem( file, data->url, *it );
			item->setPixmap( 0, QPixmap((const char**)bookmark_xpm) );
			++it;
		}
	}
}

bool BookmarksWidget::removeURL( KURL const & url )
{
//	kdDebug(0) << "BookmarksWidget::removeURL()" << endl;

	QListViewItem * item = firstChild();
	while ( item )
	{
		BookmarkItem * bm = static_cast<BookmarkItem*>(item);
		if ( bm->url() == url )
		{
			delete item;
			return true;
		}
		item = item->nextSibling();
	}
	return false;
}

void BookmarksWidget::doEmitRemoveBookMark()
{
//	kdDebug(0) << "BookmarksWidget::doEmitRemoveBookMark()" << endl;

	if ( _selectedItem->isBookmark() )
	{
		emit removeBookmarkForURL( _selectedItem->url(), _selectedItem->line() );
	}
	else
	{
		emit removeAllBookmarksForURL( _selectedItem->url() );
	}
}

void BookmarksWidget::popupMenu( QListViewItem * item, const QPoint & p, int )
{
//	kdDebug(0) << "BookmarksWidget::contextMenuRequested()" << endl;

	if ( item )
	{
		_selectedItem = static_cast<BookmarkItem *>(item);

		KPopupMenu popup;

		if ( _selectedItem->isBookmark() )
		{
			popup.insertTitle( _selectedItem->url().fileName() + i18n(", line ")
				+ QString::number( _selectedItem->line() +1 ) );

			popup.insertItem( i18n("Remove This Bookmark"), this, SLOT( doEmitRemoveBookMark() ) );
		}
		else
		{
			popup.insertTitle( _selectedItem->url().fileName() + i18n( ", All" ) );
			popup.insertItem( i18n("Remove These Bookmarks"), this, SLOT( doEmitRemoveBookMark() ) );
		}
		
		popup.insertSeparator();
		
		popup.insertItem( i18n( "Collapse All" ), this, SLOT(collapseAll()) );
		popup.insertItem( i18n( "Expand All" ), this, SLOT(expandAll()) );
		
		popup.exec(p);
	}

}

void BookmarksWidget::itemClicked( QListViewItem * clickedItem )
{
//	kdDebug(0) << "BookmarksWidget::itemClicked()" << endl;

	if ( ! clickedItem )
		return;

	BookmarkItem * item = static_cast<BookmarkItem*>( clickedItem );
	_part->partController()->editDocument( item->url(), item->line() );

}

BookmarksConfig * BookmarksWidget::config( )
{
	return _part->config();
}

QStringList BookmarksWidget::getContext( KURL const & url, unsigned int line )
{
	return _part->getContext( url, line, config()->context() );
}

void BookmarksWidget::collapseAll( )
{
	QListViewItem * it = firstChild();
	while( it )
	{
		it->setOpen( false );
		it = it->nextSibling();
	}
}

void BookmarksWidget::expandAll( )
{
	QListViewItem * it = firstChild();
	while( it )
	{
		it->setOpen( true );
		it = it->nextSibling();
	}
}


#include "bookmarks_widget.moc"

// kate: space-indent off; indent-width 4; tab-width 4; show-tabs off;
