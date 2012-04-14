/***************************************************************************
    begin                : Thu Dec 23 1999
    copyright            : (C) 1999 by John Birch
    email                : jbb@kdevelop.org
	
                          Adapted for ruby debugging
                          --------------------------
    begin                : Mon Nov 1 2004
    copyright            : (C) 2004 by Richard Dale
    email                : Richard_Dale@tipitina.demon.co.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dbgtoolbar.h"
#include "debuggerpart.h"
#include "dbgcontroller.h"

#include <kdockwindow.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <kwin.h>
#include <kwinmodule.h>

#include <qapplication.h>
#include <qcursor.h>
#include <qframe.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

// **************************************************************************
// **************************************************************************
// **************************************************************************

// Implements a floating toolbar for the debugger.

// Unfortunately, I couldn't get the KToolBar to work nicely when it
// was floating, so I was forced to write these classes. I'm not sure whether
// I didn't try hard enough or ... and I've forgotten what the problems were
// now.

// The problem with using this is that it will not dock as a normal toolbar.
// I'm not convince that this is a real problem though.

// So, if you can get it to work as a KToolBar, and it works well when the
// app is running, then all these classes can be removed.

// This code is very specific to the internal debugger in kdevelop.

namespace RDBDebugger
{

// **************************************************************************
// **************************************************************************
// **************************************************************************

// This just allows the user to click on the toolbar and drag it somewhere else.
// I would have preferred to use normal decoration on the toolbar and removed
// the iconify, close, etc buttons from the window title but again I kept running
// into problems. Instead, I used no decoration and this class. Also this looks
// similar to the KToolBar floating style.
class DbgMoveHandle : public QFrame
{
public:
    DbgMoveHandle(DbgToolBar *parent=0, const char * name=0, WFlags f=0);
    virtual ~DbgMoveHandle();

    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);

private:
    DbgToolBar* toolBar_;
    QPoint      offset_;
    bool        moving_;
};

// **************************************************************************

DbgMoveHandle::DbgMoveHandle(DbgToolBar *parent, const char * name, WFlags f)
    : QFrame(parent, name, f),
      toolBar_(parent),
      offset_(QPoint(0,0)),
      moving_(false)
{
    setFrameStyle(QFrame::Panel|QFrame::Raised);
    setFixedHeight(12);
}

// **************************************************************************

DbgMoveHandle::~DbgMoveHandle()
{
}

// **************************************************************************

void DbgMoveHandle::mousePressEvent(QMouseEvent *e)
{
    QFrame::mousePressEvent(e);
    if (moving_)
        return;

    if (e->button() == RightButton) {
        KPopupMenu *menu = new KPopupMenu(this);
	menu->insertTitle(i18n("Debug Toolbar"));
        menu->insertItem(i18n("Dock to Panel"),
                         parent(), SLOT(slotDock()));
        menu->insertItem(i18n("Dock to Panel && Iconify KDevelop"),
                         parent(), SLOT(slotIconifyAndDock()));
        menu->popup(e->globalPos());
    } else {
        moving_ = true;
        offset_ = parentWidget()->pos() - e->globalPos();
        setFrameStyle(QFrame::Panel|QFrame::Sunken);
        QApplication::setOverrideCursor(QCursor(sizeAllCursor));
        setPalette(QPalette(colorGroup().background()));
        repaint();
    }
}

// **************************************************************************

void DbgMoveHandle::mouseReleaseEvent(QMouseEvent *e)
{
    QFrame::mouseReleaseEvent(e);
    moving_ = false;
    offset_ = QPoint(0,0);
    setFrameStyle(QFrame::Panel|QFrame::Raised);
    QApplication::restoreOverrideCursor();
    setPalette(QPalette(colorGroup().background()));
    repaint();
}

// **************************************************************************

void DbgMoveHandle::mouseMoveEvent(QMouseEvent *e)
{
    QFrame::mouseMoveEvent(e);
    if (!moving_)
        return;

    toolBar_->move(e->globalPos() + offset_);
}

// **************************************************************************
// **************************************************************************
// **************************************************************************

// This class adds text _and_ a pixmap to a button. Why doesn't QPushButton
// support that? It only allowed text _or_ pixmap.
class DbgButton : public QPushButton
{
public:
    DbgButton(const QPixmap &pixmap, const QString &text,
              DbgToolBar *parent, const char *name=0);
    virtual ~DbgButton() {};
    void drawButtonLabel(QPainter *painter);
    QSize sizeHint() const;

private:
    QPixmap pixmap_;
};

// **************************************************************************

DbgButton::DbgButton(const QPixmap& pixmap, const QString& text,
                     DbgToolBar* parent, const char* name)
    : QPushButton(parent, name),
      pixmap_(pixmap)
{
    setText(text);
}

// **************************************************************************

void DbgButton::drawButtonLabel(QPainter *painter)
{
    // We always have a pixmap (today...)
    // Centre it if there's no text

    bool hasText = !text().isEmpty();
    int x = ((hasText ? height() : width()) - pixmap_.width()) / 2;
    int y = (height() - pixmap_.height()) / 2;
    painter->drawPixmap(x, y, pixmap_);

    if (hasText) {
        painter->setPen(colorGroup().text());
        painter->drawText(height()+2, 0, width()-(height()+2), height(), AlignLeft|AlignVCenter, text());
    }
}

// **************************************************************************

QSize DbgButton::sizeHint() const
{
    if (text().isEmpty())
        return pixmap_.size();
    else
        return QPushButton::sizeHint();
}

// **************************************************************************
// **************************************************************************
// **************************************************************************

DbgDocker::DbgDocker(QWidget* parent, DbgToolBar* toolBar, const QPixmap& pixmap) :
    KSystemTray(parent, "DbgDocker"),
    toolBar_(toolBar)
{
    setPixmap(pixmap);
    QToolTip::add( this, i18n("KDevelop ruby debugger: Click to execute one line of code (\"step\")") );
}

// **************************************************************************

void DbgDocker::mousePressEvent(QMouseEvent *e)
{
    if (!rect().contains( e->pos()))
        return;

    switch (e->button()) {
    case LeftButton:
        {
            // Not really a click, but it'll hold for the time being !!!
            emit clicked();
            break;
        }
    case RightButton:
        {
            KPopupMenu* menu = new KPopupMenu(this);
	    menu->insertTitle(i18n("Debug Toolbar"));
            menu->insertItem(i18n("Activate"),                        toolBar_, SLOT(slotUndock()));
            menu->insertItem(i18n("Activate (KDevelop gets focus)"),  toolBar_, SLOT(slotActivateAndUndock()));
            menu->popup(e->globalPos());
            break;
        }
    default:
        break;
    }
}

// **************************************************************************
// **************************************************************************
// **************************************************************************

DbgToolBar::DbgToolBar(RubyDebuggerPart* part,
                       QWidget* parent, const char* name)
    : QFrame(0, name),
      part_(part),
      activeWindow_(0),
      winModule_(0),
      bKDevFocus_(0),
      bPrevFocus_(0),
      appIsActive_(false),
      docked_(false),
      docker_(0),
      dockWindow_(new KSystemTray(parent))
{
    winModule_  = new KWinModule(this);
    docker_ = new DbgDocker(parent, this, BarIcon("dbgnext"));
    connect(docker_, SIGNAL(clicked()), part_, SLOT(slotStepOver()));

    // Must have noFocus set so that we can see what window was active.
    // see slotDbgKdevFocus() for more comments
    // I do not want the user to be able to "close" this widget. If we have any
    // decoration then they can and that is bad.
    // This widget is closed when the debugger finishes i.e. they press "Stop"

    // Do we need NoFocus???
    KWin::setState(winId(), NET::StaysOnTop | NET::Modal | NET::SkipTaskbar);
//    KWin::setType(winId(), NET::Override);    // So it has no decoration
    KWin::setType(winId(), NET::Dock);

    setFocusPolicy(NoFocus);
    setFrameStyle( QFrame::Box | QFrame::Plain );
    setLineWidth(4);
    setMidLineWidth(0);

    QBoxLayout* topLayout     = new QVBoxLayout(this);

    QBoxLayout* nextLayout    = new QHBoxLayout();
    QBoxLayout* stepLayout    = new QHBoxLayout();
    QBoxLayout* focusLayout   = new QHBoxLayout();

    DbgMoveHandle*  moveHandle= new DbgMoveHandle(this);

    QPushButton*  bRun        = new DbgButton(BarIcon("dbgrun"),        i18n("Run"),        this);
    QPushButton*  bInterrupt  = new DbgButton(BarIcon("player_pause"),  i18n("Interrupt"),  this);
    QPushButton*  bNext       = new DbgButton(BarIcon("dbgnext"),       i18n("Step Over"),  this);
    QPushButton*  bStep       = new DbgButton(BarIcon("dbgstep"),       i18n("Step Into"),    this);
    QPushButton*  bFinish     = new DbgButton(BarIcon("dbgstepout"),    i18n("Step Out"),   this);
    QPushButton*  bRunTo      = new DbgButton(BarIcon("dbgrunto"),      i18n("Run to Cursor"),   this);
    bPrevFocus_ = new DbgButton(BarIcon("dbgmemview"),    QString::null,      this);
    bKDevFocus_ = new DbgButton(BarIcon("kdevelop"),      QString::null,      this);

  connect(bRun,        SIGNAL(clicked()), part_,  SLOT(slotRun()));
  connect(bInterrupt,  SIGNAL(clicked()), part_,  SLOT(slotPause()));
  connect(bNext,       SIGNAL(clicked()), part_,  SLOT(slotStepOver()));
  connect(bStep,       SIGNAL(clicked()), part_,  SLOT(slotStepInto()));
  connect(bFinish,     SIGNAL(clicked()), part_,  SLOT(slotStepOut()));
  connect(bRunTo,      SIGNAL(clicked()), part_,  SLOT(slotRunToCursor()));
  connect(bKDevFocus_, SIGNAL(clicked()), this,   SLOT(slotKdevFocus()));
  connect(bPrevFocus_, SIGNAL(clicked()), this,   SLOT(slotPrevFocus()));

    QToolTip::add( bRun,        i18n("Continue with application execution, may start the application") );
    QToolTip::add( bInterrupt,  i18n("Interrupt the application execution") );
    QToolTip::add( bNext,       i18n("Execute one line of code, but run through methods") );
    QToolTip::add( bStep,       i18n("Execute one line of code, stepping into methods if appropriate") );
    QToolTip::add( bFinish,     i18n("Execute to end of current stack frame") );
    QToolTip::add( bRunTo,      i18n("Continues execution until the cursor position is reached.") );
    QToolTip::add( bKDevFocus_, i18n("Set focus on KDevelop") );
    QToolTip::add( bPrevFocus_, i18n("Set focus on window that had focus when KDevelop got focus") );

    QWhatsThis::add( bRun,        i18n("Continue with application execution. May start the application.") );
    QWhatsThis::add( bInterrupt,  i18n("Interrupt the application execution.") );
    QWhatsThis::add( bNext,       i18n("Execute one line of code, but run through methods.") );

    QWhatsThis::add( bStep,       i18n("Execute one line of code, stepping into methods if appropriate.") );
    
    QWhatsThis::add( bFinish,     i18n("Execute to end of current stack frame.") );
    QWhatsThis::add( bRunTo,      i18n("Continues execution until the cursor position is reached.") );
    QWhatsThis::add( bKDevFocus_, i18n("Set focus on KDevelop.") );
    QWhatsThis::add( bPrevFocus_, i18n("Set focus on window that had focus when KDevelop got focus.") );

    topLayout->addWidget(moveHandle);
    topLayout->addWidget(bRun);
    topLayout->addLayout(nextLayout);
    topLayout->addLayout(stepLayout);
    topLayout->addWidget(bFinish);
    topLayout->addWidget(bRunTo);
    topLayout->addWidget(bInterrupt);
    topLayout->addLayout(focusLayout);

    focusLayout->addWidget(bKDevFocus_);
    focusLayout->addWidget(bPrevFocus_);

    stepLayout->addWidget(bStep);

    nextLayout->addWidget(bNext);

//     int w = QMAX(bRun->sizeHint().width(), bFinish->sizeHint().width());
//     w = QMAX(w, bInterrupt->sizeHint().width());
//     w = QMAX(w, bView->sizeHint().width());

    // they should have the same height, so don't be too fussy
//     int h = bFinish->sizeHint().height();
//
//     bNext->setMinimumHeight(h);
//     bNexti->setMinimumHeight(h);
//     bStep->setMinimumHeight(h);
//     bStepi->setMinimumHeight(h);
//     bKDevFocus_->setMinimumHeight(h);
//     bPrevFocus_->setMinimumHeight(h);

//    setMinimumSize(w+10, h*7);
//    setMaximumSize(w+10, h*7);

    setAppIndicator(appIsActive_);
    topLayout->activate();
}

// **************************************************************************

DbgToolBar::~DbgToolBar()
{
    slotUndock();
}

// **************************************************************************

void DbgToolBar::slotKdevFocus()
{
    // I really want to be able to set the focus on the _application_ being debugged
    // but this is the best compromise I can come up with. All we do is save the
    // window that had focus when they switch to the kdevelop window. To do this
    // the toolbar _cannot_ accept focus.
    // If anyone has a way of determining what window the app is _actually_ running on
    // then please fix and send a patch.

    if (winModule_->activeWindow() != topLevelWidget()->winId())
        activeWindow_ = winModule_->activeWindow();

    KWin::activateWindow(topLevelWidget()->winId());
}

// **************************************************************************

void DbgToolBar::slotPrevFocus()
{
    KWin::activateWindow(activeWindow_);
}

// **************************************************************************

// If the app is active then the app button is highlighted, otherwise
// kdev button is highlighted.
void DbgToolBar::slotDbgStatus(const QString&, int state)
{
    bool appIndicator = state & s_appBusy;
    if (appIndicator != appIsActive_) {
        setAppIndicator(appIndicator);
        appIsActive_ = appIndicator;
    }
}

// **************************************************************************

void DbgToolBar::setAppIndicator(bool appIndicator)
{
    if (appIndicator) {
        bPrevFocus_->setPalette(QPalette(colorGroup().mid()));
        bKDevFocus_->setPalette(QPalette(colorGroup().background()));
    } else {
        bPrevFocus_->setPalette(QPalette(colorGroup().background()));
        bKDevFocus_->setPalette(QPalette(colorGroup().mid()));
    }
}

// **************************************************************************

void DbgToolBar::slotDock()
{
    if (docked_)
        return;

    //  Q_ASSERT(!docker_);
    hide();

    docker_->show();
    docked_ = true;
}

// **************************************************************************

void DbgToolBar::slotIconifyAndDock()
{
    if (docked_)
        return;

    //  KWin::iconifyWindow(ckDevelop_->winId(), true);
    slotDock();
}

// **************************************************************************

void DbgToolBar::slotUndock()
{
    if (!docked_)
        return;

    show();
    docker_->hide();
    docked_ = false;
}

// **************************************************************************

void DbgToolBar::slotActivateAndUndock()
{
    if (!docked_)
        return;

    KWin::activateWindow(topLevelWidget()->winId());
    slotUndock();
}

}

// **************************************************************************
#include "dbgtoolbar.moc"
