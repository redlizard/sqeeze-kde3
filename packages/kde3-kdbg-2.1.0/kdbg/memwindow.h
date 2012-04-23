// $Id$

// Copyright by Johannes Sixt
// This file is under GPL, the GNU General Public Licence

#ifndef MEMWINDOW_H
#define MEMWINDOW_H

#include <qpopupmenu.h>
#include <qlistview.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qdict.h>
#include <qptrlist.h>
#include <qmap.h>

class KDebugger;
class KConfigBase;
struct MemoryDump;

class MemoryWindow : public QWidget
{
    Q_OBJECT
public:
    MemoryWindow(QWidget* parent, const char* name);
    ~MemoryWindow();

    void setDebugger(KDebugger* deb) { m_debugger = deb; }

protected:
    KDebugger* m_debugger;
    QComboBox m_expression;

    QListView m_memory;
    QMap<QString,QString> m_old_memory;

    QVBoxLayout m_layout;

    unsigned m_format;
    QDict<unsigned> m_formatCache;

    QPopupMenu m_popup;

    virtual bool eventFilter(QObject* o, QEvent* ev);
    void handlePopup(QMouseEvent* ev);
    void displayNewExpression(const QString& expr);

public slots:
    void slotNewExpression(const QString&);
    void slotTypeChange(int id);
    void slotNewMemoryDump(const QString&, QList<MemoryDump>&);
    void saveProgramSpecific(KConfigBase* config);
    void restoreProgramSpecific(KConfigBase* config);
};

#endif // MEMWINDOW_H
