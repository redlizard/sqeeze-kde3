/*
   Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>

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

#include <qbuttongroup.h>
#include <qnamespace.h>
#include <qtoolbutton.h>
#include <qlabel.h>
#include <qtooltip.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qtoolbar.h>
#include <qdockwindow.h>

#include <kdebug.h>
#include <kparts/event.h>
#include <klocale.h>
#include <ktoolbar.h>
#include <kiconloader.h>
#include <kseparator.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kactionclasses.h>

#include <KoMainWindow.h>
#include "KoToolBox.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

KoToolBox::KoToolBox( KMainWindow *mainWin, const char* name, KInstance* instance, int numberOfTooltypes )
    : KToolBar( mainWin, Qt::DockLeft, false, name, true, true), m_instance(instance)
{
    setFullSize( false );
    setMargin(2);

    m_buttonGroup = new QButtonGroup( 0L );
    m_buttonGroup->setExclusive( true );
    connect( m_buttonGroup, SIGNAL( pressed( int ) ), this, SLOT( slotButtonPressed( int ) ) );

    m_tools.setAutoDelete( true );
    // Create separate lists for the various sorts of tools
    for (int i = 0; i < numberOfTooltypes ; ++i) {
        ToolList * tl = new ToolList();
        m_tools.append(tl);
    }
}

KoToolBox::~KoToolBox()
{
    delete m_buttonGroup;
}


void KoToolBox::slotPressButton( int id )
{
    m_buttonGroup->setButton( id );
    slotButtonPressed( id );
}

void KoToolBox::slotButtonPressed( int id )
{
    if( id != m_buttonGroup->selectedId() && m_buttonGroup->selected() ) {
        m_buttonGroup->selected()->setDown( false );
    }
    m_idToActionMap.at(id)->activate();

}

void KoToolBox::registerTool( KAction *tool, int toolType, Q_UINT32 priority )
{
    uint prio = priority;
    ToolList * tl = m_tools.at(toolType);
    while( (*tl)[prio] ) prio++;
    (*tl)[prio] = tool;
}

QToolButton *KoToolBox::createButton(QWidget * parent,  const char* iconName, QString tooltip)
{
    QToolButton *button = new QToolButton(parent);

    if ( iconName && *iconName ) {
        QPixmap pixmap = BarIcon( iconName, m_instance );
        button->setPixmap( pixmap );
        button->setToggleButton( true );
    }

    if ( !tooltip.isEmpty() ) {
        QToolTip::add( button, tooltip );
    }
    return button;
}


void KoToolBox::setupTools()
{
    int id = 0;
    // Loop through tooltypes
    for (uint i = 0; i < m_tools.count(); ++i) {
        ToolList * tl = m_tools.at(i);

        if (!tl) continue;
        if (tl->isEmpty()) continue;

        ToolArea *tools = new ToolArea( this );
        ToolList::Iterator it;
        for ( it = tl->begin(); it != tl->end(); ++it )
        {
            KAction *tool = it.data();
            if(! tool)
                continue;
            QToolButton *bn = createButton(tools->getNextParent(), tool->icon().latin1(), tool->toolTip());
            tools->add(bn);
            m_buttonGroup->insert( bn, id++ );
            m_idToActionMap.append( tool );
        }
        if (i < m_tools.count() -1) addSeparator();
        m_toolBoxes.append(tools);
    }
    // select first (select tool)
    m_buttonGroup->setButton( 0 );
    m_numberOfButtons = id;
}


void KoToolBox::setOrientation ( Qt::Orientation o )
{
    if ( barPos() == Floating ) { // when floating, make it a standing toolbox.
        o = o == Qt::Vertical ? Qt::Horizontal : Qt::Vertical;
    }

    QDockWindow::setOrientation( o );

    for (uint i = 0; i < m_toolBoxes.count(); ++i) {
        ToolArea *t = m_toolBoxes.at(i);
        t->setOrientation(o);
    }
}


void KoToolBox::enableTools(bool enable)
{
    if (m_tools.isEmpty()) return;
    if (!m_buttonGroup) return;
    if (m_numberOfButtons <= 0) return;

    for (uint i = 0; i < m_tools.count(); ++i) {
        ToolList * tl = m_tools.at(i);

        if (!tl) continue;

        ToolList::Iterator it;
        for ( it = tl->begin(); it != tl->end(); ++it )
            if (it != 0 && it.data())
                it.data()->setEnabled(enable);
    }
    m_buttonGroup->setEnabled(enable);
    for (Q_UINT32 i = 0; i < m_numberOfButtons; ++i) {
        m_buttonGroup->find( i )->setEnabled( enable );
    }
}

void KoToolBox::slotSetTool(const QString & toolname)
{
    for (uint i = 0; i < m_idToActionMap.count(); ++i) {
        KAction * a = m_idToActionMap.at(i);
        if (!a || a->name() != toolname) continue;

        m_buttonGroup->setButton(i);
        return;

    }
}


// ----------------------------------------------------------------
//                         class ToolArea


ToolArea::ToolArea(QWidget *parent)
    : QWidget(parent), m_left(true)
{
    m_layout = new QBoxLayout(this, QBoxLayout::LeftToRight, 0, 0, 0);
    QWidget *w = new QWidget(this);
    m_layout->addWidget(w);

    QGridLayout *grid = new QGridLayout(w, 2, 2);
    m_leftRow = new QWidget(w);
    grid->addWidget(m_leftRow, 0, 0);
    m_leftLayout = new QBoxLayout(m_leftRow, QBoxLayout::TopToBottom, 0, 1, 0);

    w = new QWidget(this);
    m_layout->addWidget(w);

    grid = new QGridLayout(w, 2, 2);
    m_rightRow = new QWidget(w);
    grid->addWidget(m_rightRow, 0, 0);
    m_rightLayout = new QBoxLayout(m_rightRow, QBoxLayout::TopToBottom, 0, 1, 0);
}


ToolArea::~ToolArea()
{
}


void ToolArea::add(QWidget *button)
{
    if (m_left)
        m_leftLayout->addWidget(button);
    else
        m_rightLayout->addWidget(button);
    button->show();
    m_left = !m_left;
}


QWidget* ToolArea::getNextParent()
{
    if (m_left)
        return m_leftRow;
    return m_rightRow;
}


void ToolArea::setOrientation ( Qt::Orientation o )
{
    QBoxLayout::Direction  dir = (o != Qt::Horizontal 
				  ? QBoxLayout::TopToBottom
				  : QBoxLayout::LeftToRight);
    m_leftLayout->setDirection(dir);
    m_rightLayout->setDirection(dir);

    m_layout->setDirection(o == Qt::Horizontal
			   ? QBoxLayout::TopToBottom
			   : QBoxLayout::LeftToRight);
}


#include "KoToolBox.moc"
