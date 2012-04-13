/* This file is part of the KDE project
   Copyright (c) 2004 Kevin Ottens <ervin ipsquad net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <kaboutapplication.h>
#include <kdebug.h>
#include <kpopupmenu.h>
#include <kiconloader.h>

#include "mediaapplet.h"

#include "preferencesdialog.h"

extern "C"
{
	KDE_EXPORT KPanelApplet* init( QWidget *parent, const QString configFile)
	{
		KGlobal::locale()->insertCatalogue("mediaapplet");
		return new MediaApplet(configFile, KPanelApplet::Normal,
			KPanelApplet::About | KPanelApplet::Preferences,
			parent, "mediaapplet");
	}
}

MediaApplet::MediaApplet(const QString& configFile, Type type, int actions, QWidget *parent, const char *name)
	: KPanelApplet(configFile, type, actions, parent, name),
	mButtonSizeSum(0)
{
	if (!parent)
		setBackgroundMode(X11ParentRelative);

	setAcceptDrops(true);
	
	loadConfig();

	mpDirLister = new KDirLister();

	connect( mpDirLister, SIGNAL( clear() ),
	         this, SLOT( slotClear() ) );
	connect( mpDirLister, SIGNAL( started(const KURL&) ),
	         this, SLOT( slotStarted(const KURL&) ) );
	connect( mpDirLister, SIGNAL( completed() ),
	         this, SLOT( slotCompleted() ) );
	connect( mpDirLister, SIGNAL( newItems( const KFileItemList & ) ),
	         this, SLOT( slotNewItems( const KFileItemList & ) ) );
	connect( mpDirLister, SIGNAL( deleteItem( KFileItem * ) ),
	         this, SLOT( slotDeleteItem( KFileItem * ) ) );
	connect( mpDirLister, SIGNAL( refreshItems( const KFileItemList & ) ),
	         this, SLOT( slotRefreshItems( const KFileItemList & ) ) );

	reloadList();
}

MediaApplet::~MediaApplet()
{
	delete mpDirLister;
	
	while (!mButtonList.isEmpty())
	{
		MediumButton *b = mButtonList.first();
		mButtonList.remove(b);
		delete b;
	}
	
        KGlobal::locale()->removeCatalogue("mediaapplet");
}

void MediaApplet::about()
{
	KAboutData data("mediaapplet",
	                I18N_NOOP("Media Applet"),
	                "1.0",
	                I18N_NOOP("\"media:/\" ioslave frontend applet"),
	                KAboutData::License_GPL_V2,
	                "(c) 2004, Kevin Ottens");

	data.addAuthor("Kevin \'ervin\' Ottens",
	               I18N_NOOP("Maintainer"),
	               "ervin ipsquad net",
	               "http://ervin.ipsquad.net");

	data.addCredit("Joseph Wenninger",
	               I18N_NOOP("Good mentor, patient and helpful. Thanks for all!"),
	               "jowenn@kde.org");

	KAboutApplication dialog(&data);
	dialog.exec();
}

void MediaApplet::preferences()
{
	PreferencesDialog dialog(mMedia);

	dialog.setExcludedMediumTypes(mExcludedTypesList);
	dialog.setExcludedMedia(mExcludedList);

	if(dialog.exec())
	{
		mExcludedTypesList = dialog.excludedMediumTypes();
		mExcludedList = dialog.excludedMedia();
		saveConfig();
		reloadList();
	}
}

int MediaApplet::widthForHeight( int /*height*/ ) const
{
	return mButtonSizeSum;
}

int MediaApplet::heightForWidth( int /*width*/ ) const
{
	return mButtonSizeSum;
}

void MediaApplet::resizeEvent( QResizeEvent *)
{
	arrangeButtons();
}

void MediaApplet::arrangeButtons()
{
	int button_size = 1;
	int x_offset = 0;
	int y_offset = 0;

	// Determine upper bound for the button size
	MediumButtonList::iterator it;
	MediumButtonList::iterator end = mButtonList.end();
	for ( it = mButtonList.begin(); it != end; ++it )
	{
		MediumButton *button = *it;
		
		button_size = std::max(button_size,
		                       orientation() == Vertical ?
				           button->heightForWidth(width()) :
					   button->widthForHeight(height()) );
//		                           button->widthForHeight(height()) :
//		                           button->heightForWidth(width()) );
	}
	
	int kicker_size;
	if (orientation() == Vertical)
	{
		kicker_size = width();
	}
	else
	{
		kicker_size = height();
	}
	
	unsigned int max_packed_buttons = kicker_size / button_size;
	// Center icons if we only have one column/row
	if (mButtonList.count() < max_packed_buttons)
	{
		max_packed_buttons = QMAX(uint(1), mButtonList.count());
	}
        max_packed_buttons = QMAX(uint(1), max_packed_buttons);

	int padded_button_size = kicker_size / max_packed_buttons;
	mButtonSizeSum = 0;
	unsigned int pack_count = 0;
	
	// Arrange the buttons. If kicker is more than twice as high/wide
	// as the maximum preferred size of an icon, we put several icons
	// in one column/row
	for ( it = mButtonList.begin(); it != end; ++it )
	{
		MediumButton *button = *it;

		button->move(x_offset, y_offset);
		button->setPanelPosition(position());

		if(pack_count == 0)
		{
			mButtonSizeSum += button_size;
		}
		
		++pack_count;

		if(orientation() == Vertical)
                {
			if (pack_count < max_packed_buttons)
			{
				x_offset += padded_button_size;
			}
			else
			{
				x_offset = 0;
				y_offset += button_size;
				pack_count = 0;
			}
			
			button->resize(padded_button_size, button_size);
		}
		else
		{
			if (pack_count < max_packed_buttons)
			{
				y_offset += padded_button_size;
                        }
			else
			{
				y_offset = 0;
				x_offset += button_size;
				pack_count = 0;
			}
			
			button->resize(button_size, padded_button_size);
		}
	}
	
	updateGeometry();
	emit updateLayout();
}

void MediaApplet::slotClear()
{
	kdDebug()<<"MediaApplet::slotClear"<<endl;

	while (!mButtonList.isEmpty())
	{
		MediumButton *b = mButtonList.first();
		mButtonList.remove(b);
		delete b;
	}
	
	arrangeButtons();
}

void MediaApplet::slotStarted(const KURL &/*url*/)
{

}

void MediaApplet::slotCompleted()
{
	mMedia = mpDirLister->items(KDirLister::AllItems);
}

void MediaApplet::slotNewItems(const KFileItemList &entries)
{
	kdDebug()<<"MediaApplet::slotNewItems"<<endl;

	for(KFileItemListIterator it(entries); it.current(); ++it)
	{
		kdDebug() << "item: " << it.current()->url() << endl;

		bool found = false;
		MediumButtonList::iterator it2;
		MediumButtonList::iterator end = mButtonList.end();
		for ( it2 = mButtonList.begin(); it2 != end; ++it2 )
		{
			MediumButton *button = *it2;
			
			if(button->fileItem().url()==it.current()->url())
			{
				found = true;
				button->setFileItem(*it.current());
				break;
			}
		}
		
		if(!found && !mExcludedList.contains(it.current()->url().url()) )
		{
			MediumButton *button = new MediumButton(this, *it.current());
			button->show();
			mButtonList.append(button);
		}
	}

	arrangeButtons();
}

void MediaApplet::slotDeleteItem(KFileItem *fileItem)
{
	kdDebug()<<"MediumApplet::slotDeleteItem:"<< fileItem->url() << endl;

	MediumButtonList::iterator it;
	MediumButtonList::iterator end = mButtonList.end();
	for ( it = mButtonList.begin(); it != end; ++it )
	{
		MediumButton *button = *it;
		
		if(button->fileItem().url()==fileItem->url())
		{
			mButtonList.remove(button);
			delete button;
			break;
		}
	}
	slotCompleted();
	arrangeButtons();
}

void MediaApplet::slotRefreshItems(const KFileItemList &entries)
{
	for(KFileItemListIterator it(entries); it.current(); ++it)
	{
		kdDebug()<<"MediaApplet::slotRefreshItems:"<<(*it.current()).url().url()<<endl;

		QString mimetype = (*it.current()).mimetype();
		bool found = false;

		kdDebug()<<"mimetype="<<mimetype<<endl;

		MediumButtonList::iterator it2;
		MediumButtonList::iterator end = mButtonList.end();
		for ( it2 = mButtonList.begin(); it2 != end; ++it2 )
		{
			MediumButton *button = *it2;
			
			if(button->fileItem().url()==(*it.current()).url())
			{
				if(mExcludedTypesList.contains(mimetype))
				{
					mButtonList.remove(button);
					delete button;
				}
				else
				{
					button->setFileItem(*it.current());
				}
				found = true;
				break;
			}
		}

		if(!found && !mExcludedTypesList.contains(mimetype) && !mExcludedList.contains(it.current()->url().url()) )
		{
			MediumButton *button = new MediumButton(this, *it.current());
			button->show();
			mButtonList.append(button);
		}
	}

	arrangeButtons();
}

void MediaApplet::positionChange(Position)
{
	arrangeButtons();
}

void MediaApplet::loadConfig()
{
	KConfig *c = config();
	c->setGroup("General");

	if(c->hasKey("ExcludedTypes"))
	{
		mExcludedTypesList = c->readListEntry("ExcludedTypes",';');
	}
	else
	{
		mExcludedTypesList.clear();
		mExcludedTypesList << "media/hdd_mounted";
		mExcludedTypesList << "media/hdd_unmounted";
		mExcludedTypesList << "media/nfs_mounted";
		mExcludedTypesList << "media/nfs_unmounted";
		mExcludedTypesList << "media/smb_mounted";
		mExcludedTypesList << "media/smb_unmounted";
	}

	if(c->hasKey("ExcludedMedia"))
	{
		mExcludedList = c->readListEntry("ExcludedMedia",';');
	}
	else
	{
		mExcludedList.clear();
	}

}

void MediaApplet::saveConfig()
{
	KConfig *c = config();
	c->setGroup("General");

	c->writeEntry("ExcludedTypes", mExcludedTypesList, ';');
	c->writeEntry("ExcludedMedia", mExcludedList, ';');

	c->sync();
}

void MediaApplet::reloadList()
{
	mpDirLister->stop();
	
	while (!mButtonList.isEmpty())
	{
		MediumButton *b = mButtonList.first();
		mButtonList.remove(b);
		delete b;
	}
	
	mpDirLister->clearMimeFilter();
	mpDirLister->setMimeExcludeFilter(mExcludedTypesList);
	mpDirLister->openURL(KURL("media:/"));
}

void MediaApplet::mousePressEvent(QMouseEvent *e)
{
	if(e->button()==RightButton)
	{
		KPopupMenu menu(this);

		menu.insertTitle(i18n("Media"));
		menu.insertItem(SmallIcon("configure"), i18n("&Configure..."), 1);

		int choice = menu.exec(this->mapToGlobal(e->pos()));

		if(choice==1)
		{
			preferences();
		}
	}
}

#include "mediaapplet.moc"
