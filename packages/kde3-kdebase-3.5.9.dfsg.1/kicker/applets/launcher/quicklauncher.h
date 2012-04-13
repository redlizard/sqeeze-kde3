/*****************************************************************

Copyright (c) 2000 Bill Nagel

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

#ifndef __quicklauncher_h__
#define __quicklauncher_h__

#include <dcopobject.h>
#include <qimage.h>
#include <qstring.h>
#include <qvaluevector.h>
#include <kpanelapplet.h>
#include <map>

#include "flowgridmanager.h"
#include "prefs.h"
#include "quickbutton.h"

class ConfigDlg;
class QPopupMenu;
class QuickButtonGroup;
class PopularityStatistics;
class KAction;

typedef QuickButtonGroup ButtonGroup;

class QuickLauncher: public KPanelApplet, public DCOPObject
{
    Q_OBJECT
    K_DCOP

k_dcop:
    void serviceStartedByStorageId(QString starter, QString storageId);

public:
    enum {DEFAULT_ICON_DIM=QuickButton::DEFAULT_ICON_DIM};
    enum {SIZE_AUTO=0};
    
    struct PopularityInfo {
        float popularity;
    };
    
    QuickLauncher(const QString& configFile, Type t = Normal, int actions = 0,
                  QWidget *parent = 0, const char *name = 0);
    ~QuickLauncher();
    int widthForHeight(int height) const;
    int heightForWidth(int width) const;
    void addApp(QString url, int index, bool manuallyAdded);
    virtual void action(Action a);

public slots:
    void addApp(QString url, bool manuallyAdded);
    void addAppBeforeManually(QString url, QString sender);
    void removeAppManually(QuickButton *button);
    void removeApp(QString url, bool manuallyRemoved);
    void removeApp(int index, bool manuallyRemoved);
    void removeAppManually(int index);
    void saveConfig();
    void about();
    
protected:
    int findApp(QString url);
    int findApp(QuickButton *button);
    
    void mousePressEvent(QMouseEvent *e);
    void resizeEvent(QResizeEvent*);
    void dragEnterEvent(QDragEnterEvent *e);
    void dragLeaveEvent(QDragLeaveEvent *e);
    void dragMoveEvent(QDragMoveEvent *e);
    void dropEvent(QDropEvent *e);
    void refreshContents();
    void setRefreshEnabled(bool enable);
    void setConserveSpace(bool conserve_space);
    void setDragEnabled(bool conserve_space);
    
    bool conserveSpace() const { return m_manager->conserveSpace(); }
    bool isDragEnabled() const { return m_settings->dragEnabled(); }
    
    void buildPopupMenu();
    void loadConfig();
    
    void mergeButtons(int index);
    void clearTempButtons();
    int  dimension() const;

protected slots:
    void slotConfigure();
    void slotSettingsDialogChanged();
    void fillRemoveAppsMenu();
    void slotOwnServiceExecuted(QString serviceMenuId);
    void slotAdjustToCurrentPopularity();
    void slotStickyToggled();

protected:
    void updateInsertionPosToStatusQuo();
    void updateStickyHighlightLayer();
    QuickButton* createButton(QString url);
    virtual void paintEvent(QPaintEvent* e);
    virtual void positionChange(Position);
    
    QPopupMenu *m_popup;
    QPopupMenu *m_appletPopup;
    QPopupMenu *m_removeAppsMenu;
    QuickButtonGroup *m_buttons, *m_newButtons, *m_oldButtons, *m_dragButtons;
    int m_space, m_border;
    QSize m_buttonSize;
    FlowGridManager *m_manager;
    int m_dropLen, m_dropPos, m_minPanelDim;
    bool m_dragAccepted, m_refreshEnabled, m_needsSave, m_needsRefresh;
    std::map<QString, int> m_appOrdering;
    Prefs* m_settings;
    KAction *m_configAction;
    ConfigDlg *m_configDialog;
    PopularityStatistics* m_popularity;
    QImage m_stickyHighlightLayer;
    QTimer *m_saveTimer;
};

#endif
