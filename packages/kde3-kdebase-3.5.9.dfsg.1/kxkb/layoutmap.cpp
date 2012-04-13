//
// C++ Implementation: layoutmap
//
// Description: 
//
//
// Author: Andriy Rysin <rysin@kde.org>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "layoutmap.h"

#include "x11helper.h"


LayoutMap::LayoutMap(const KxkbConfig& kxkbConfig_):
	m_kxkbConfig(kxkbConfig_),
	m_currentWinId( X11Helper::UNKNOWN_WINDOW_ID )
{
}

// private
void LayoutMap::clearMaps()
{
	m_appLayouts.clear();
	m_winLayouts.clear();
	m_globalLayouts.clear();
	//setCurrentWindow( -1 );
}

void LayoutMap::reset()
{
	clearMaps();
	setCurrentWindow( X11Helper::UNKNOWN_WINDOW_ID );
}



void LayoutMap::setCurrentWindow(WId winId)
{
	m_currentWinId = winId;
	if( m_kxkbConfig.m_switchingPolicy == SWITCH_POLICY_WIN_CLASS )
		m_currentWinClass = X11Helper::getWindowClass(winId, qt_xdisplay());
}

// private
//LayoutQueue& 
QPtrQueue<LayoutState>& LayoutMap::getCurrentLayoutQueueInternal(WId winId)
{
	if( winId == X11Helper::UNKNOWN_WINDOW_ID )
		return m_globalLayouts;
	
	switch( m_kxkbConfig.m_switchingPolicy ) {
		case SWITCH_POLICY_WIN_CLASS: {
//			QString winClass = X11Helper::getWindowClass(winId, qt_xdisplay());
			return m_appLayouts[ m_currentWinClass ];
		}
		case SWITCH_POLICY_WINDOW:
			return m_winLayouts[ winId ];

		default:
			return m_globalLayouts;
	}
}

// private
//LayoutQueue& 
QPtrQueue<LayoutState>& LayoutMap::getCurrentLayoutQueue(WId winId)
{
	QPtrQueue<LayoutState>& layoutQueue = getCurrentLayoutQueueInternal(winId);
	if( layoutQueue.count() == 0 ) {
		initLayoutQueue(layoutQueue);
		kdDebug() << "map: Created queue for " << winId << " size: " << layoutQueue.count() << endl;
	}
	
	return layoutQueue;
}

LayoutState& LayoutMap::getCurrentLayout() {
	return *getCurrentLayoutQueue(m_currentWinId).head();
}

LayoutState& LayoutMap::getNextLayout() {
	LayoutQueue& layoutQueue = getCurrentLayoutQueue(m_currentWinId);
	LayoutState* layoutState = layoutQueue.dequeue();
	layoutQueue.enqueue(layoutState);
	
	kdDebug() << "map: Next layout: " << layoutQueue.head()->layoutUnit.toPair() 
			<< " group: " << layoutQueue.head()->layoutUnit.defaultGroup << " for " << m_currentWinId << endl;
	
	return *layoutQueue.head();
}

void LayoutMap::setCurrentGroup(int group) {
	getCurrentLayout().group = group;
}

void LayoutMap::setCurrentLayout(const LayoutUnit& layoutUnit) {
	LayoutQueue& layoutQueue = getCurrentLayoutQueue(m_currentWinId);
	kdDebug() << "map: Storing layout: " << layoutUnit.toPair() 
			<< " group: " << layoutUnit.defaultGroup << " for " << m_currentWinId << endl;
	
	int queueSize = (int)layoutQueue.count();
	for(int ii=0; ii<queueSize; ii++) {
		if( layoutQueue.head()->layoutUnit == layoutUnit )
			return;	// if present return when it's in head
		
		LayoutState* layoutState = layoutQueue.dequeue();
		if( ii < queueSize - 1 ) {
			layoutQueue.enqueue(layoutState);
		}
		else {
			delete layoutState;
			layoutQueue.enqueue(new LayoutState(layoutUnit));
		}
	}
	for(int ii=0; ii<queueSize - 1; ii++) {
		LayoutState* layoutState = layoutQueue.dequeue();
		layoutQueue.enqueue(layoutState);
	}
}

// private
void LayoutMap::initLayoutQueue(LayoutQueue& layoutQueue) {
	int queueSize = ( m_kxkbConfig.m_stickySwitching ) 
			? m_kxkbConfig.m_stickySwitchingDepth : m_kxkbConfig.m_layouts.count();
	for(int ii=0; ii<queueSize; ii++) {
		layoutQueue.enqueue( new LayoutState(m_kxkbConfig.m_layouts[ii]) );
	}
}
