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

#ifndef _KBIBTEXSHELL_H_
#define _KBIBTEXSHELL_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kapplication.h>
#include <kparts/mainwindow.h>

class QWidget;
class QFrame;

class KToggleAction;
class KConfig;
class KRecentFilesAction;

class KBibTeXShell : public KParts::MainWindow
{
    Q_OBJECT

public:
    KBibTeXShell( QWidget* parentWidget = 0, const char* name = 0 );

    virtual ~KBibTeXShell();

    bool openURL( const KURL& url );

    KParts::ReadWritePart *part();

    //config file functions
public:
    void readConfig( KConfig * );
    void writeConfig( KConfig * );

    void readConfig();
    void writeConfig();

public:
    static QString encoding();

public slots:
    void slotAddRecentURL( const KURL& );

protected:
    virtual bool queryClose( );

private slots:
    void slotFileNew();
    void slotFileOpen();
    void slotFileOpen( const KURL& url );
    void slotFileClose();
    void optionsShowStatusbar();
    void optionsConfigureKeys();
    void optionsConfigureToolbars();

    void applyNewToolbarConfig();

    void slotActionStatusText( const QString &text );

private:
    KParts::ReadWritePart *m_part;
    QWidget* m_parentWidget;

    KToggleAction *m_statusbarAction;

    KRecentFilesAction *m_recentFiles;

    void saveProperties( KConfig * );
    void readProperties( KConfig * );

    void setupAccel();
    void setupActions();

    void initializePart( KParts::ReadWritePart* );

};

#endif // _KBIBTEXSHELL_H_
