//
// C++ Implementation: kxkbtraywindow
//
// Description: 
//
//
// Author: Andriy Rysin <rysin@kde.org>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <qtooltip.h>

#include <kdebug.h>
#include <klocale.h>
#include <kiconeffect.h>
#include <kiconloader.h>
#include <kpopupmenu.h>
#include <kaction.h>
#include <kuniqueapplication.h>

#include "kxkbtraywindow.h"
#include "pixmap.h"
#include "rules.h"
#include "kxkbconfig.h"


KxkbLabelController::KxkbLabelController(QLabel* label_, QPopupMenu* contextMenu_) :
    label(label_),
    contextMenu(contextMenu_),
 	m_menuStartIndex(contextMenu_->count()),
	m_prevLayoutCount(0)
{
// 	kdDebug() << "Creating KxkbLabelController with " << label_ << ", " << contextMenu_ << endl;
// 	kdDebug() << "Creating KxkbLabelController with startMenuIndex " << m_menuStartIndex << endl;
}

void KxkbLabelController::setToolTip(const QString& tip)
{
	QToolTip::remove(label);
	QToolTip::add(label, tip);
}

void KxkbLabelController::setPixmap(const QPixmap& pixmap)
{
	KIconEffect iconeffect;
	label->setPixmap( iconeffect.apply(pixmap, KIcon::Panel, KIcon::DefaultState) );
}


void KxkbLabelController::setCurrentLayout(const LayoutUnit& layoutUnit)
{
	setToolTip(m_descriptionMap[layoutUnit.toPair()]);
	setPixmap( LayoutIcon::getInstance().findPixmap(layoutUnit.layout, m_showFlag, layoutUnit.displayName) );
}


void KxkbLabelController::setError(const QString& layoutInfo)
{
    QString msg = i18n("Error changing keyboard layout to '%1'").arg(layoutInfo);
	setToolTip(msg);

	label->setPixmap(LayoutIcon::getInstance().findPixmap("error", m_showFlag));
}


void KxkbLabelController::initLayoutList(const QValueList<LayoutUnit>& layouts, const XkbRules& rules)
{
//	KPopupMenu* menu = contextMenu();
	QPopupMenu* menu = contextMenu;
//	int index = menu->indexOf(0);

    m_descriptionMap.clear();
//    menu->clear();
//    menu->insertTitle( kapp->miniIcon(), kapp->caption() );

	for(int ii=0; ii<m_prevLayoutCount; ++ii) {
		menu->removeItem(START_MENU_ID + ii);
		kdDebug() << "remove item: " << START_MENU_ID + ii << endl;
	}
/*	menu->removeItem(CONFIG_MENU_ID);
	menu->removeItem(HELP_MENU_ID);*/
	
    KIconEffect iconeffect;
    
	int cnt = 0;
    QValueList<LayoutUnit>::ConstIterator it;
    for (it=layouts.begin(); it != layouts.end(); ++it)
    {
		const QString layoutName = (*it).layout;
		const QString variantName = (*it).variant;
		
		const QPixmap& layoutPixmap = LayoutIcon::getInstance().findPixmap(layoutName, m_showFlag, (*it).displayName);
        const QPixmap pix = iconeffect.apply(layoutPixmap, KIcon::Small, KIcon::DefaultState);
		
		QString fullName = i18n((rules.layouts()[layoutName]));
		if( variantName.isEmpty() == false )
			fullName += " (" + variantName + ")";
		contextMenu->insertItem(pix, fullName, START_MENU_ID + cnt, m_menuStartIndex + cnt);
		m_descriptionMap.insert((*it).toPair(), fullName);
		
		cnt++;
    }

	m_prevLayoutCount = cnt;
	
	// if show config, if show help
	if( menu->indexOf(CONFIG_MENU_ID) == -1 ) {
		contextMenu->insertSeparator();
		contextMenu->insertItem(SmallIcon("configure"), i18n("Configure..."), CONFIG_MENU_ID);
		if( menu->indexOf(HELP_MENU_ID) == -1 )
			contextMenu->insertItem(SmallIcon("help"), i18n("Help"), HELP_MENU_ID);
	}

/*    if( index != -1 ) { //not first start
		menu->insertSeparator();
		KAction* quitAction = KStdAction::quit(this, SIGNAL(quitSelected()), actionCollection());
        if (quitAction)
    	    quitAction->plug(menu);
    }*/
}

// void KxkbLabelController::mouseReleaseEvent(QMouseEvent *ev)
// {
//     if (ev->button() == QMouseEvent::LeftButton)
//         emit toggled();
//     KSystemTray::mouseReleaseEvent(ev);
// }

#include "kxkbtraywindow.moc"
