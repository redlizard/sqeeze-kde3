/***************************************************************************
*   Copyright (C) 2004-2006 by Thomas Fischer                             *
*   fischer@unix-ag.uni-kl.de                                             *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/

#ifndef _KBIBTEXPART_H_
#define _KBIBTEXPART_H_

#include <kparts/part.h>
#include <kparts/factory.h>

#include <element.h>
#include <xsltransform.h>
#include <documentwidget.h>
#include <settingsdlg.h>

class QWidget;
class QTextEdit;
class QFile;
class QLabel;
class QTabWidget;
class QPainter;
class QListViewItem;
class QProgressDialog;
class KURL;
class KListView;
class KAction;
class KActionMenu;
class KToggleAction;
class KMainWindow;
class KBibTeX::WebQuery;

class KBibTeXPart : public KParts::ReadWritePart
{
    Q_OBJECT

public:
    KBibTeXPart( QWidget *parentWidget, const char *widgetName,
                 QObject *parent, const char *name );

    virtual ~KBibTeXPart();

    void setReadWrite( bool rw );

    void setModified( bool modified );

    bool saveAs();
    bool queryClose();
    virtual bool closeURL();

public slots:
    virtual bool save();

signals:
    void signalAddRecentURL( const KURL& );

protected:
    virtual bool openFile();
    virtual bool saveFile();

protected slots:
    void slotFileSaveAs();
    void slotFileMerge();
    void slotFileExport();
    void slotFileStatistics();
    void slotPreferences();

private slots:
    bool slotNewElement();
    void slotSearchWebsites( int id );
    void slotToggleShowSpecialElements();
    void slotDeferredInitialization();
    void slotUpdateMenu( int numSelectedItems );
    void slotUndoChanged( bool undoAvailable );
    void slotUseInPipe();

private:
    enum SearchDirection
    {
        sdForward, sdBackward
    };
    enum SaveStatus
    {
        ssSave, ssExport, ssFailed
    };

    int m_defInitCounter;
    QLabel *m_preview;
    QProgressDialog *m_progressDialog;
    KBibTeX::DocumentWidget *m_documentWidget;
    KBibTeX::SettingsDlg *m_settingsDlg;
    KAction *m_actionFileMerge;
    KAction *m_actionFileExport;
    KAction *m_actionFileStatistics;
    KAction *m_actionEditUndo;
    KAction *m_actionElementSendToLyX;
    KAction *m_actionEditElement;
    KAction *m_actionDeleteElement;
    KAction *m_actionEditCut;
    KAction *m_actionEditCopy;
    KAction *m_actionEditCopyRef;
    KAction *m_actionEditPaste;
    KAction *m_actionEditSelectAll;
    KAction *m_actionEditFind;
    KAction *m_actionEditFindNext;
    KAction *m_actionPreferences;
    KAction *m_actionSearchOnlineDatabases;
    KActionMenu *m_actionViewDocument;
    KActionMenu *m_actionAssignKeywords;
    KActionMenu *m_actionMenuSearchWebsites;
    KActionMenu *m_actionViewShowColumns;
    KAction *m_actionSearchNCBI;
    KToggleAction *m_actionUseInPipe;
    KToggleAction *m_actionShowComments;
    KToggleAction *m_actionShowMacros;
    KMainWindow *m_mainWindow;
    QValueList<KBibTeX::WebQuery*> webQueryList;
    bool m_initializationDone;
    QFile *m_inPipe;

    void readSettings();
    void writeSettings();

    void setupGUI( QWidget *parentWidget, const char *name );
    void setupActions();

signals:
    void progressCancel();
};

class KInstance;
class KAboutData;

class KBibTeXPartFactory : public KParts::Factory
{
    Q_OBJECT
public:
    KBibTeXPartFactory();
    virtual ~KBibTeXPartFactory();
    virtual KParts::Part* createPartObject( QWidget * parentWidget, const char * widgetName,
                                            QObject * parent, const char * name,
                                            const char * classname, const QStringList & /* args */ );
    static KInstance* instance();

private:
    static KInstance* s_instance;
    static KAboutData* s_about;
};

#endif // _KBIBTEXPART_H_
