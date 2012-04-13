/*****************************************************************

Copyright (c) 2001 Matthias Elter <elter@kde.org>
Copyright (c) 2002 John Firebaugh <jfirebaugh@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include "tasklmbmenu.h"
#include "tasklmbmenu.moc"

#include <qpainter.h>
#include <qstyle.h>

#include <kdebug.h>
#include <kglobalsettings.h>

#include "global.h"

TaskMenuItem::TaskMenuItem(const QString &text,
                           bool active, bool minimized, bool attention)
  : QCustomMenuItem(),
    m_text(text),
    m_isActive(active),
    m_isMinimized(minimized),
    m_demandsAttention(attention),
    m_attentionState(true)
{
}

TaskMenuItem::~TaskMenuItem()
{
}

void TaskMenuItem::paint(QPainter *p, const QColorGroup &cg,
                         bool highlighted, bool /*enabled*/,
                         int x, int y, int w, int h )
{
    if (m_isActive)
    {
        QFont font = p->font();
        font.setBold(true);
        p->setFont(font);
    }

    if (highlighted)
    {
        p->setPen(cg.highlightedText());
    }
    else if (m_isMinimized)
    {
        p->setPen(QPen(KickerLib::blendColors(cg.background(), cg.text())));
    }
    else if (m_demandsAttention && !m_attentionState)
    {
        p->setPen(cg.mid());
    }

    p->drawText(x, y, w, h, AlignAuto|AlignVCenter|DontClip|ShowPrefix, m_text);
}

QSize TaskMenuItem::sizeHint()
{
    QFont font = QFont();
    if (m_isActive)
    {
        font.setBold(true);
    }
    return QFontMetrics(font).size(AlignAuto|AlignVCenter|DontClip|ShowPrefix,
                                   m_text);
}

/*****************************************************************************/

TaskLMBMenu::TaskLMBMenu(const Task::List& tasks, QWidget *parent, const char *name)
  : QPopupMenu(parent, name),
    m_tasks(tasks),
    m_lastDragId(-1),
    m_attentionState(false)
{
    fillMenu();

    setAcceptDrops(true); // Always enabled to activate task during drag&drop.

    m_dragSwitchTimer = new QTimer(this, "DragSwitchTimer");
    connect(m_dragSwitchTimer, SIGNAL(timeout()), SLOT(dragSwitch()));
}

void TaskLMBMenu::fillMenu()
{
    setCheckable(true);

    Task::List::iterator itEnd = m_tasks.end();
    for (Task::List::iterator it = m_tasks.begin(); it != itEnd; ++it)
    {
        Task::Ptr t = (*it);

        QString text = t->visibleName().replace("&", "&&");

        TaskMenuItem *menuItem = new TaskMenuItem(text,
                                                  t->isActive(),
                                                  t->isIconified(),
                                                  t->demandsAttention());
        int id = insertItem(QIconSet(t->pixmap()), menuItem);
        connectItem(id, t, SLOT(activateRaiseOrIconify()));
        setItemChecked(id, t->isActive());

        if (t->demandsAttention())
        {
            m_attentionState = true;
            m_attentionMap.append(menuItem);
        }
    }

    if (m_attentionState)
    {
        m_attentionTimer = new QTimer(this, "AttentionTimer");
        connect(m_attentionTimer, SIGNAL(timeout()), SLOT(attentionTimeout()));
        m_attentionTimer->start(500, true);
    }
}

void TaskLMBMenu::attentionTimeout()
{
    m_attentionState = !m_attentionState;

    for (QValueList<TaskMenuItem*>::const_iterator it = m_attentionMap.constBegin();
         it != m_attentionMap.constEnd();
         ++it)
    {
        (*it)->setAttentionState(m_attentionState);
    }

    update();

    m_attentionTimer->start(500, true);
}

void TaskLMBMenu::dragEnterEvent( QDragEnterEvent* e )
{
    // ignore task drags
    if (TaskDrag::canDecode(e))
    {
        return;
    }

    int id = idAt(e->pos());

    if (id == -1)
    {
        m_dragSwitchTimer->stop();
        m_lastDragId = -1;
    }
    else if (id != m_lastDragId)
    {
        m_lastDragId = id;
        m_dragSwitchTimer->start(1000, true);
    }

    QPopupMenu::dragEnterEvent( e );
}

void TaskLMBMenu::dragLeaveEvent( QDragLeaveEvent* e )
{
    m_dragSwitchTimer->stop();
    m_lastDragId = -1;

    QPopupMenu::dragLeaveEvent(e);

    hide();
}

void TaskLMBMenu::dragMoveEvent( QDragMoveEvent* e )
{
    // ignore task drags
    if (TaskDrag::canDecode(e))
    {
        return;
    }

    int id = idAt(e->pos());

    if (id == -1)
    {
        m_dragSwitchTimer->stop();
        m_lastDragId = -1;
    }
    else if (id != m_lastDragId)
    {
        m_lastDragId = id;
        m_dragSwitchTimer->start(1000, true);
    }

    QPopupMenu::dragMoveEvent(e);
}

void TaskLMBMenu::dragSwitch()
{
    bool ok = false;
    Task::Ptr t = m_tasks.at(indexOf(m_lastDragId), &ok);
    if (ok)
    {
        t->activate();

        for (unsigned int i = 0; i < count(); ++i)
        {
            setItemChecked(idAt(i), false );
        }

        setItemChecked( m_lastDragId, true );
    }
}

void TaskLMBMenu::mousePressEvent( QMouseEvent* e )
{
    if (e->button() == LeftButton)
    {
        m_dragStartPos = e->pos();
    }
    else
    {
        m_dragStartPos = QPoint();
    }

    QPopupMenu::mousePressEvent(e);
}

void TaskLMBMenu::mouseReleaseEvent(QMouseEvent* e)
{
    m_dragStartPos = QPoint();
    QPopupMenu::mouseReleaseEvent(e);
}

void TaskLMBMenu::mouseMoveEvent(QMouseEvent* e)
{
    if (m_dragStartPos.isNull())
    {
        QPopupMenu::mouseMoveEvent(e);
        return;
    }

    int delay = KGlobalSettings::dndEventDelay();
    QPoint newPos(e->pos());

    if ((m_dragStartPos - newPos).manhattanLength() > delay)
    {
        int index = indexOf(idAt(m_dragStartPos));
        if (index != -1)
        {
            bool ok = false;
            Task::Ptr task = m_tasks.at(index, &ok);
            if (ok)
            {
                Task::List tasks;
                tasks.append(task);
                TaskDrag* drag = new TaskDrag(tasks, this);
                drag->setPixmap(task->pixmap());
                drag->dragMove();
            }
        }
    }

    QPopupMenu::mouseMoveEvent(e);
}

