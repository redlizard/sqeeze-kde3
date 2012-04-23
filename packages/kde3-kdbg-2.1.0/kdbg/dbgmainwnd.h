// $Id$

// Copyright by Johannes Sixt
// This file is under GPL, the GNU General Public Licence

#ifndef DBGMAINWND_H
#define DBGMAINWND_H

#include <qtimer.h>
#include <kdockwidget.h>
#include "mainwndbase.h"
#include "regwnd.h"

class KRecentFilesAction;
class WinStack;
class QListBox;
class QCString;
class ExprWnd;
class BreakpointTable;
class ThreadList;
class MemoryWindow;
struct DbgAddr;

class DebuggerMainWnd : public KDockMainWindow, public DebuggerMainWndBase
{
    Q_OBJECT
public:
    DebuggerMainWnd(const char* name);
    ~DebuggerMainWnd();

    bool debugProgram(const QString& exe, QCString lang);

protected:
    // session properties
    virtual void saveProperties(KConfig*);
    virtual void readProperties(KConfig*);
    // settings
    void saveSettings(KConfig*);
    void restoreSettings(KConfig*);

    // statusbar texts
    void updateLineStatus(int lineNo);	/* zero-based line number */

    void initToolbar();
    void initKAction();

    // view windows
    WinStack* m_filesWindow;
    QListBox* m_btWindow;
    ExprWnd* m_localVariables;
    WatchWindow* m_watches;
    RegisterView* m_registers;
    BreakpointTable* m_bpTable;
    TTYWindow* m_ttyWindow;
    ThreadList* m_threads;
    MemoryWindow* m_memoryWindow;

    QTimer m_backTimer;

    // recent execs in File menu
    KRecentFilesAction* m_recentExecAction;

protected:
    virtual void closeEvent(QCloseEvent* e);
    virtual TTYWindow* ttyWindow();
    virtual QString createOutputWindow();

    KDockWidget* dockParent(QWidget* w);
    bool isDockVisible(QWidget* w);
    bool canChangeDockVisibility(QWidget* w);
    void dockUpdateHelper(QString action, QWidget* w);
    void fixDockConfig(KConfig* c, bool upgrade);

    QString makeSourceFilter();

    // to avoid flicker when the status bar is updated,
    // we store the last string that we put there
    QString m_lastActiveStatusText;
    bool m_animRunning;

signals:
    void setTabWidth(int tabWidth);

public slots:
    virtual void updateUI();
    virtual void updateLineItems();
    void slotFileChanged();
    void slotLineChanged();
    void slotAddWatch();
    void slotAddWatch(const QString& text);
    void slotNewFileLoaded();
    void slotNewStatusMsg();
    void slotDebuggerStarting();
    void slotToggleBreak(const QString&, int, const DbgAddr&, bool);
    void slotEnaDisBreak(const QString&, int, const DbgAddr&);
    void slotTermEmuExited();
    void slotProgramStopped();
    void slotBackTimer();
    void slotRecentExec(const KURL& url);
    void slotLocalsPopup(QListViewItem*, const QPoint& pt);
    void slotLocalsToWatch();
    void slotEditValue();

    void slotFileOpen();
    void slotFileQuit();
    void slotFileExe();
    void slotFileCore();
    void slotFileGlobalSettings();
    void slotFileProgSettings();
    void slotViewToolbar();
    void slotViewStatusbar();
    void slotExecUntil();
    void slotExecAttach();
    void slotExecArgs();
    void intoBackground();
    void slotConfigureKeys();
};

#endif // DBGMAINWND_H
