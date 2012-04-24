/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

/*
  Copyright (C) 2006-2008 Eike Hein <hein@kde.org>
*/

#include "viewcontainer.h"
#include "queuetuner.h"
#include "viewtree.h"
#include "konversationapplication.h"
#include "notificationhandler.h"
#include "images.h"
#include "irccharsets.h"
#include "ircview.h"
#include "logfilereader.h"
#include "konsolepanel.h"
#include "urlcatcher.h"
#include "dcctransferpanel.h"
#include "dcctransfermanager.h"
#include "dccchat.h"
#include "statuspanel.h"
#include "channel.h"
#include "query.h"
#include "rawlog.h"
#include "channellistpanel.h"
#include "nicksonline.h"
#include "insertchardialog.h"
#include "irccolorchooser.h"
#include "joinchanneldialog.h"
#include "servergroupsettings.h"

#include <qsplitter.h>
#include <qpopupmenu.h>

#include <kdebug.h>
#include <klocale.h>
#include <ktabwidget.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kglobalsettings.h>


ViewContainer::ViewContainer(KonversationMainWindow* window):
        m_vbox(0), m_queueTuner(0)
{
    m_window = window;

    images = KonversationApplication::instance()->images();

    m_tabWidget = 0;
    m_viewTree = 0;

    m_urlCatcherPanel = 0;
    m_nicksOnlinePanel = 0;

    m_insertCharDialog = 0;

    m_queryViewCount = 0;

    m_viewTreeSplitter = new QSplitter(m_window, "view_tree_splitter");
    m_viewTreeSplitter->setOpaqueResize(KGlobalSettings::opaqueResize());
    m_saveSplitterSizesLock = true;

    // The tree needs to be initialized before the tab widget so that it
    // may assume a leading role in view selection management.
    if (Preferences::tabPlacement()==Preferences::Left) setupViewTree();

    setupTabWidget();

    initializeSplitterSizes();

    m_dccPanel = new DccTransferPanel(m_tabWidget);
    m_dccPanel->hide();
    m_dccPanelOpen = false;
    connect(m_dccPanel, SIGNAL(updateTabNotification(ChatWindow*,const Konversation::TabNotifyType&)), this, SLOT(setViewNotification(ChatWindow*,const Konversation::TabNotifyType&)));
}

ViewContainer::~ViewContainer()
{
}

void ViewContainer::showQueueTuner(bool p)
{
    if (p)
        m_queueTuner->open();
    else
        m_queueTuner->close();
}

///Use this instead of setting m_frontServer directly so we can emit the frontServerChanging signal easily.
void ViewContainer::setFrontServer(Server* newserver)
{
    if (m_frontServer == QGuardedPtr<Server>(newserver))
        return;
    emit frontServerChanging(newserver);
    m_frontServer = newserver;
}

void ViewContainer::prepareShutdown()
{
    if (!m_tabWidget) return;

    deleteDccPanel();
    closeNicksOnlinePanel();

    for (int i = 0; i < m_tabWidget->count(); ++i)
        m_tabWidget->page(i)->blockSignals(true);

    m_tabWidget->blockSignals(true);

    m_tabWidget = 0;
}

void ViewContainer::initializeSplitterSizes()
{
    if (m_viewTree && !m_viewTree->isHidden())
    {
        QValueList<int> sizes = Preferences::treeSplitterSizes();

        if (sizes.isEmpty())
            sizes << 145 << (m_window->width()-145);
        m_viewTreeSplitter->setSizes(sizes);

        m_saveSplitterSizesLock = false;
    }
}

void ViewContainer::saveSplitterSizes()
{
    if (!m_saveSplitterSizesLock)
    {
        Preferences::setTreeSplitterSizes(m_viewTreeSplitter->sizes());
        m_saveSplitterSizesLock = false;
    }
}

void ViewContainer::setupTabWidget()
{
    m_popupViewIndex = -1;

    m_vbox = new QVBox(m_viewTreeSplitter, "main_window_right_side");
    m_tabWidget = new KTabWidget(m_vbox, "main_window_tab_widget");
    m_queueTuner = new QueueTuner(m_vbox, this);
    m_queueTuner->hide();

    m_tabWidget->setTabReorderingEnabled(true);
    m_tabWidget->setTabCloseActivatePrevious(true);

    m_vbox->hide();    //m_tabWidget->hide();

    KPushButton* closeBtn = new KPushButton(m_tabWidget);
    closeBtn->setPixmap(KGlobal::iconLoader()->loadIcon("tab_remove", KIcon::Small));
    closeBtn->resize(22, 22);
    closeBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_tabWidget->setCornerWidget(closeBtn);
    connect(closeBtn, SIGNAL(clicked()), this, SLOT(closeCurrentView()));

    connect(m_tabWidget, SIGNAL(currentChanged(QWidget*)), this, SLOT (switchView(QWidget*)));
    connect(m_tabWidget, SIGNAL(closeRequest(QWidget*)), this, SLOT(closeView(QWidget*)));
    connect(m_tabWidget, SIGNAL(contextMenu(QWidget*, const QPoint&)), this, SLOT(showViewContextMenu(QWidget*, const QPoint&)));
    connect(m_tabWidget, SIGNAL(mouseMiddleClick(QWidget*)), this, SLOT(closeViewMiddleClick(QWidget*)));

    updateTabWidgetAppearance();
}

void ViewContainer::setupViewTree()
{
    m_viewTree = new ViewTree(m_viewTreeSplitter);
    m_viewTreeSplitter->setResizeMode(m_viewTree, QSplitter::KeepSize);
    m_viewTree->hide();

    connect(KonversationApplication::instance(), SIGNAL(appearanceChanged()), m_viewTree, SLOT(updateAppearance()));
    connect(this, SIGNAL(viewChanged(ChatWindow*)), m_viewTree, SLOT(selectView(ChatWindow*)));
    connect(this, SIGNAL(removeView(ChatWindow*)), m_viewTree, SLOT(removeView(ChatWindow*)));
    connect(this, SIGNAL(contextMenuClosed()), m_viewTree, SLOT(unHighlight()));
    connect(m_viewTree, SIGNAL(setViewTreeShown(bool)), this, SLOT(setViewTreeShown(bool)));
    connect(m_viewTree, SIGNAL(showView(ChatWindow*)), this, SLOT(showView(ChatWindow*)));
    connect(m_viewTree, SIGNAL(closeView(ChatWindow*)), this, SLOT(closeView(ChatWindow*)));
    connect(m_viewTree, SIGNAL(showViewContextMenu(QWidget*, const QPoint&)), this, SLOT(showViewContextMenu(QWidget*, const QPoint&)));
    connect(m_viewTree, SIGNAL(sizeChanged()), this, SLOT(saveSplitterSizes()));
    connect(m_viewTree, SIGNAL(syncTabBarToTree()), this, SLOT(syncTabBarToTree()));

    KAction* action;

    action = actionCollection()->action("move_tab_left");

    if (action)
    {
        action->setText(i18n("Move Tab Up"));
        action->setIcon("1uparrow");
    }

    action = actionCollection()->action("move_tab_right");

    if (action)
    {
        action->setText(i18n("Move Tab Down"));
        action->setIcon("1downarrow");
    }

    // If the tab widget already exists we may need to sync the ViewTree
    // with an already-populated tab bar.
    if (m_tabWidget)
    {
        // Explicitly move to the left since we've been added after the
        // tab widget.
        m_viewTreeSplitter->moveToFirst(m_viewTree);

        if (m_tabWidget->count() > 0)
        {
            // Add StatusPanels first, or curious tab bar sortings may break
            // the tree hierarchy.
            for (int i = 0; i < m_tabWidget->count(); ++i)
            {
                ChatWindow* view = static_cast<ChatWindow*>(m_tabWidget->page(i));

                if (view->getType() == ChatWindow::Status)
                {
                    if (view == m_frontView)
                        m_viewTree->addView(view->getName(), view, m_tabWidget->tabIconSet(view), true);
                    else
                        m_viewTree->addView(view->getName(), view, m_tabWidget->tabIconSet(view));
                }
            }

            for (int i = 0; i < m_tabWidget->count(); ++i)
            {
                ChatWindow* view = static_cast<ChatWindow*>(m_tabWidget->page(i));

                if (!view->getType() == ChatWindow::Status)
                {
                    if (view == m_frontView)
                        m_viewTree->addView(view->getName(), view, m_tabWidget->tabIconSet(view), true);
                    else
                        m_viewTree->addView(view->getName(), view, m_tabWidget->tabIconSet(view));
                }
            }

            syncTabBarToTree();
        }
    }
    else
    {
        // Since the ViewTree was created before the tab widget, it
        // is free to select the first view added to the tree. Other-
        // wise the currently focused view would have been selected
        // by the tabbar/viewtree sync loop above. This ensures a
        // properly selected list item in the tree on app startup.
        m_viewTree->selectFirstView(true);
    }
}

void ViewContainer::setViewTreeShown(bool show)
{
    if (m_viewTree)
    {
        if (!show)
        {
            m_saveSplitterSizesLock = true;
            m_viewTree->hide();
        }
        else
        {
            m_viewTree->show();
            initializeSplitterSizes();
            m_saveSplitterSizesLock = false;
        }
    }
}

void ViewContainer::removeViewTree()
{
    disconnect(KonversationApplication::instance(), SIGNAL(appearanceChanged()), m_viewTree, SLOT(updateAppearance()));
    disconnect(this, SIGNAL(viewChanged(ChatWindow*)), m_viewTree, SLOT(selectView(ChatWindow*)));
    disconnect(this, SIGNAL(removeView(ChatWindow*)), m_viewTree, SLOT(removeView(ChatWindow*)));
    disconnect(this, SIGNAL(contextMenuClosed()), m_viewTree, SLOT(unHighlight()));
    disconnect(m_viewTree, SIGNAL(setViewTreeShown(bool)), this, SLOT(setViewTreeShown(bool)));
    disconnect(m_viewTree, SIGNAL(showView(ChatWindow*)), this, SLOT(showView(ChatWindow*)));
    disconnect(m_viewTree, SIGNAL(closeView(ChatWindow*)), this, SLOT(closeView(ChatWindow*)));
    disconnect(m_viewTree, SIGNAL(showViewContextMenu(QWidget*, const QPoint&)), this, SLOT(showViewContextMenu(QWidget*, const QPoint&)));
    disconnect(m_viewTree, SIGNAL(sizeChanged()), this, SLOT(saveSplitterSizes()));
    disconnect(m_viewTree, SIGNAL(syncTabBarToTree()), this, SLOT(syncTabBarToTree()));

    KAction* action;

    action = actionCollection()->action("move_tab_left");

    if (action)
    {
        action->setText(i18n("Move Tab Left"));
        action->setIcon("1leftarrow");
    }

    action = actionCollection()->action("move_tab_right");

    if (action)
    {
        action->setText(i18n("Move Tab Right"));
        action->setIcon("1rightarrow");
    }

    delete m_viewTree;
    m_viewTree = 0;
}

void ViewContainer::syncTabBarToTree()
{
    QPtrList<ChatWindow> viewList = m_viewTree->getSortedViewList();

    if (m_tabWidget && !viewList.isEmpty())
    {
        QPtrListIterator<ChatWindow> it(viewList);
        ChatWindow* view;
        int index = 0;
        int oldIndex = 0;

        while ((view = it.current()) != 0)
        {
            ++it;

            oldIndex = m_tabWidget->indexOf(view);

            if (!(oldIndex == index))
                m_tabWidget->moveTab(oldIndex, index);

            ++index;
        }
    }

    updateViewActions(m_tabWidget->currentPageIndex());
}

void ViewContainer::updateAppearance()
{
    if (Preferences::tabPlacement()==Preferences::Left && m_viewTree == 0)
    {
        m_saveSplitterSizesLock = true;
        setupViewTree();
    }

    if (!(Preferences::tabPlacement()==Preferences::Left) && m_viewTree)
    {
        m_saveSplitterSizesLock = true;
        removeViewTree();
    }

    updateViews();
    updateTabWidgetAppearance();

    KToggleAction* action = static_cast<KToggleAction*>(actionCollection()->action("hide_nicknamelist"));
    action->setChecked(!Preferences::showNickList());

    if(m_insertCharDialog)
    {
        QFont font;

        if (Preferences::customTextFont())
            font = Preferences::textFont();
        else
            font = KGlobalSettings::generalFont();

        m_insertCharDialog->setFont(font);
    }
}

void ViewContainer::updateTabWidgetAppearance()
{
    if (!m_tabWidget) return;

    m_tabWidget->setTabBarHidden((Preferences::tabPlacement()==Preferences::Left));

    if (Preferences::customTabFont())
        m_tabWidget->setFont(Preferences::tabFont());
    else
        m_tabWidget->setFont(KGlobalSettings::generalFont());

    m_tabWidget->setTabPosition((Preferences::tabPlacement()==Preferences::Top) ?
        QTabWidget::Top : QTabWidget::Bottom);

    if (Preferences::showTabBarCloseButton() && !(Preferences::tabPlacement()==Preferences::Left))
        m_tabWidget->cornerWidget()->show();
    else
        m_tabWidget->cornerWidget()->hide();

    m_tabWidget->setHoverCloseButton(Preferences::closeButtons());

    #if KDE_IS_VERSION(3,4,0)
    m_tabWidget->setAutomaticResizeTabs(Preferences::useMaxSizedTabs());
    #endif
}

void ViewContainer::updateViewActions(int index)
{
    if (!m_tabWidget) return;

    KAction* action;

    ChatWindow* view = static_cast<ChatWindow*>(m_tabWidget->page(index));

    if (m_tabWidget->count() > 0 && view)
    {
        ChatWindow::WindowType viewType = view->getType();
        Server* server = view->getServer();
        bool insertSupported = view->isInsertSupported();

        if (m_viewTree)
        {
            action = actionCollection()->action("move_tab_left");
            if (action) action->setEnabled(m_viewTree->canMoveViewUp(view));

            action = actionCollection()->action("move_tab_right");
            if (action) action->setEnabled(m_viewTree->canMoveViewDown(view));
        }
        else if (m_tabWidget)
        {
            action = actionCollection()->action("move_tab_left");
            if (action) action->setEnabled(index > 0);

            action = actionCollection()->action("move_tab_right");
            if (action) action->setEnabled(index < (m_tabWidget->count() - 1));
        }

        if (server && (viewType == ChatWindow::Status || server == m_frontServer))
        {
            action = actionCollection()->action("reconnect_server");
            if (action) action->setEnabled(true);


            action = actionCollection()->action("disconnect_server");
            if (action) action->setEnabled(server->isConnected());


            action = actionCollection()->action("join_channel");
            if (action) action->setEnabled(server->isConnected());
        }
        else
        {
            action = actionCollection()->action("reconnect_server");
            if (action) action->setEnabled(false);


            action = actionCollection()->action("disconnect_server");
            if (action) action->setEnabled(false);


            action = actionCollection()->action("join_channel");
            if (action) action->setEnabled(false);
        }

        KToggleAction* notifyAction = static_cast<KToggleAction*>(actionCollection()->action("tab_notifications"));
        if (notifyAction)
        {
            notifyAction->setEnabled(viewType == ChatWindow::Channel || viewType == ChatWindow::Query ||
                                     viewType == ChatWindow::Status || viewType == ChatWindow::Konsole ||
                                     viewType == ChatWindow::DccTransferPanel || viewType == ChatWindow::RawLog);
            notifyAction->setChecked(view->notificationsEnabled());
        }

        KToggleAction* autoJoinAction = static_cast<KToggleAction*>(actionCollection()->action("tab_autojoin"));
        Channel* channel = static_cast<Channel*>(view);
        if (autoJoinAction && viewType == ChatWindow::Channel && channel->getServer()->getServerGroup())
        {
            autoJoinAction->setEnabled(true);
            autoJoinAction->setChecked(channel->autoJoin());
        }
        else if (!(viewType != ChatWindow::Channel && index != m_tabWidget->currentPageIndex()))
        {
            autoJoinAction->setEnabled(false);
            autoJoinAction->setChecked(false);
        }

        action = actionCollection()->action("rejoin_channel");
        if (action) action->setEnabled(viewType == ChatWindow::Channel && channel->rejoinable());

        action = actionCollection()->action("close_queries");
        if (action) action->setEnabled(m_queryViewCount > 0);

        action = actionCollection()->action("clear_tabs");
        if (action) action->setEnabled(true);

        action = actionCollection()->action("toggle_away");
        if (action) action->setEnabled(true);

        action = actionCollection()->action("next_tab");
        if (action) action->setEnabled(true);

        action = actionCollection()->action("previous_tab");
        if (action) action->setEnabled(true);

        action = actionCollection()->action("next_active_tab");
        if (action) action->setEnabled(true);

        action = actionCollection()->action("close_tab");
        if (action) action->setEnabled(true);

        if (index == m_tabWidget->currentPageIndex())
        {
            // The following only need to be updated when this run is related
            // to the active tab, e.g. when it was just changed.

            action = actionCollection()->action("insert_marker_line");
            if (action)  action->setEnabled(insertSupported);

            action = actionCollection()->action("insert_character");
            if (action) action->setEnabled(insertSupported);

            action = actionCollection()->action("irc_colors");
            if (action) action->setEnabled(insertSupported);

            action = actionCollection()->action("clear_lines");
            if (action) action->setEnabled(insertSupported && view->getTextView()->hasLines());

            action = actionCollection()->action("clear_window");
            if (action) action->setEnabled(insertSupported);

            action = actionCollection()->action("edit_find");
            if (action)
            {
                action->setText(i18n("Find Text..."));
                action->setEnabled(view->searchView());
                action->setToolTip(i18n("Search for text in the current tab"));
            }

            action = actionCollection()->action("edit_find_next");
            if (action) action->setEnabled(view->searchView());

            action = actionCollection()->action("edit_find_last");
            if (action) action->setEnabled(view->searchView());

            KToggleAction* channelListAction = static_cast<KToggleAction*>(actionCollection()->action("open_channel_list"));
            if (channelListAction)
            {
                if (m_frontServer)
                {
                    QString name = m_frontServer->getDisplayName();
                    name = name.replace('&', "&&");
                    channelListAction->setEnabled(true);
                    channelListAction->setChecked(m_frontServer->getChannelListPanel());
                    channelListAction->setText(i18n("Channel &List for %1").arg(name));
                }
                else
                {
                    channelListAction->setEnabled(false);
                    channelListAction->setChecked(false);
                    channelListAction->setText(i18n("Channel &List"));
                }
            }

            action = actionCollection()->action("open_logfile");
            if (action)
            {
                action->setEnabled(!view->logFileName().isEmpty());
                if (view->logFileName().isEmpty())
                    action->setText(i18n("&Open Logfile"));
                else
                {
                    QString name = view->getName();
                    name = name.replace('&', "&&");
                    action->setText(i18n("&Open Logfile for %1").arg(name));
                }
            }

            action = actionCollection()->action("hide_nicknamelist");
            if (action) action->setEnabled(view->getType() == ChatWindow::Channel);

            action = actionCollection()->action("channel_settings");
            if (action && view->getType() == ChatWindow::Channel)
            {
                action->setEnabled(true);
                action->setText(i18n("&Channel Settings for %1...").arg(view->getName()));
            }
            else if (action)
            {
                action->setEnabled(false);
                action->setText(i18n("&Channel Settings..."));
            }
        }
    }
    else
    {
        action = actionCollection()->action("move_tab_left");
        if (action) action->setEnabled(false);

        action = actionCollection()->action("move_tab_right");
        if(action) action->setEnabled(false);

        action = actionCollection()->action("next_tab");
        if (action) action->setEnabled(false);

        action = actionCollection()->action("previous_tab");
        if (action) action->setEnabled(false);

        action = actionCollection()->action("close_tab");
        if (action) action->setEnabled(false);

        action = actionCollection()->action("next_active_tab");
        if (action) action->setEnabled(false);

        action = actionCollection()->action("tab_notifications");
        if (action) action->setEnabled(false);

        action = actionCollection()->action("tab_autojoin");
        if (action) action->setEnabled(false);

        action = actionCollection()->action("rejoin_channel");
        if (action) action->setEnabled(false);

        action = actionCollection()->action("insert_marker_line");
        if (action) action->setEnabled(false);

        action = actionCollection()->action("insert_character");
        if (action) action->setEnabled(false);

        action = actionCollection()->action("irc_colors");
        if (action) action->setEnabled(false);

        action = actionCollection()->action("clear_lines");
        if (action) action->setEnabled(false);

        action = actionCollection()->action("clear_window");
        if (action) action->setEnabled(false);

        action = actionCollection()->action("clear_tabs");
        if (action) action->setEnabled(false);

        action = actionCollection()->action("edit_find");
        if (action) action->setEnabled(false);

        action = actionCollection()->action("edit_find_next");
        if (action) action->setEnabled(false);

        action = actionCollection()->action("edit_find_last");
        if (action) action->setEnabled(false);

        action = actionCollection()->action("open_channel_list");
        if (action) action->setEnabled(false);

        action = actionCollection()->action("open_logfile");
        if (action) action->setEnabled(false);

        action = actionCollection()->action("toggle_away");
        if (action) action->setEnabled(false);

        action = actionCollection()->action("join_channel");
        if (action) action->setEnabled(false);

        action = actionCollection()->action("disconnect_server");
        if (action) action->setEnabled(false);

        action = actionCollection()->action("reconnect_server");
        if (action) action->setEnabled(false);

        action = actionCollection()->action("hide_nicknamelist");
        if (action) action->setEnabled(false);

        action = actionCollection()->action("channel_settings");
        if (action) action->setEnabled(false);

        action = actionCollection()->action("close_queries");
        if (action) action->setEnabled(false);
    }
}

void ViewContainer::updateFrontView()
{
    if (!m_tabWidget) return;

    ChatWindow* view = static_cast<ChatWindow*>(m_tabWidget->currentPage());

    if (!view) return;

    // Make sure that only views with info output get to be the m_frontView
    if (m_frontView)
    {
        disconnect(m_frontView, SIGNAL(updateInfo(const QString &)), this, SIGNAL(setStatusBarInfoLabel(const QString &)));
    }

    if (view->canBeFrontView())
    {
        m_frontView = view;

        connect(view, SIGNAL(updateInfo(const QString &)), this, SIGNAL(setStatusBarInfoLabel(const QString &)));
        view->emitUpdateInfo();
    }
    else
    {
        QString viewName = Konversation::removeIrcMarkup(view->getName());

        if(viewName != "ChatWindowObject")
            emit setStatusBarInfoLabel(viewName);
        else
            emit clearStatusBarInfoLabel();
    }

    switch (view->getType())
    {
        case ChatWindow::Channel:
        case ChatWindow::Query:
        case ChatWindow::Status:
        case ChatWindow::ChannelList:
        case ChatWindow::RawLog:
            emit setStatusBarLagLabelShown(true);
            break;

        default:
            emit setStatusBarLagLabelShown(false);
            break;
    }

    // Make sure that only text views get to be the m_searchView
    if (view->searchView()) m_searchView = view;

    updateViewActions(m_tabWidget->currentPageIndex());
}

void ViewContainer::updateViews(const Konversation::ServerGroupSettings* serverGroup)
{
    if (!m_tabWidget) return;

    for (int i = 0; i < m_tabWidget->count(); ++i)
    {
        ChatWindow* view = static_cast<ChatWindow*>(m_tabWidget->page(i));

        if (serverGroup)
        {
            if (view->getType() == ChatWindow::Status && view->getServer()->getServerGroup() == serverGroup)
            {
                QString label = view->getServer()->getDisplayName();

                if (!label.isEmpty() && m_tabWidget->tabLabel(view) != label)
                {
                    if (m_tabWidget) m_tabWidget->setTabLabel(view, label);
                    if (m_viewTree) m_viewTree->setViewName(view, label);

                    if (view == m_frontView)
                    {
                        emit setStatusBarInfoLabel(label);
                        emit setWindowCaption(label);
                    }

                    static_cast<StatusPanel*>(view)->updateName();
                }
            }

            if (i == m_tabWidget->currentPageIndex())
                updateViewActions(m_tabWidget->currentPageIndex());
        }

        if (m_viewTree)
        {
            if (!Preferences::tabNotificationsLeds() && !Preferences::closeButtons())
                m_viewTree->setViewIcon(view, QIconSet());

            if (Preferences::closeButtons() && !Preferences::tabNotificationsLeds())
                 m_viewTree->setViewIcon(view, images->getCloseIcon());


            if (!Preferences::tabNotificationsText())
                m_viewTree->setViewColor(view, m_window->colorGroup().foreground());
        }
        else if (m_tabWidget)
        {
            if (!Preferences::tabNotificationsLeds() && !Preferences::closeButtons())
                m_tabWidget->setTabIconSet(view, QIconSet());

            if (Preferences::closeButtons() && !Preferences::tabNotificationsLeds())
                m_tabWidget->setTabIconSet(view, images->getCloseIcon());

            if (!Preferences::tabNotificationsText())
                m_tabWidget->setTabColor(view, m_window->colorGroup().foreground());
        }

        if (Preferences::tabNotificationsLeds() || Preferences::tabNotificationsText())
        {
            if (view->currentTabNotification()==Konversation::tnfNone)
                unsetViewNotification(view);
            else if (view->currentTabNotification()==Konversation::tnfNormal && !Preferences::tabNotificationsMsgs())
                unsetViewNotification(view);
            else if (view->currentTabNotification()==Konversation::tnfPrivate && !Preferences::tabNotificationsPrivate())
                unsetViewNotification(view);
            else if (view->currentTabNotification()==Konversation::tnfSystem && !Preferences::tabNotificationsSystem())
                unsetViewNotification(view);
            else if (view->currentTabNotification()==Konversation::tnfControl && !Preferences::tabNotificationsEvents())
                unsetViewNotification(view);
            else if (view->currentTabNotification()==Konversation::tnfNick && !Preferences::tabNotificationsNick())
                unsetViewNotification(view);
            else if (view->currentTabNotification()==Konversation::tnfHighlight && !Preferences::tabNotificationsHighlights())
                unsetViewNotification(view);
            else if (view==m_tabWidget->currentPage())
                unsetViewNotification(view);
            else
                setViewNotification(view, view->currentTabNotification());
        }
    }
}

void ViewContainer::updateViewIcons()
{
    if (!m_tabWidget) return;

    for (int i = 0; i < m_tabWidget->count(); ++i)
    {
        ChatWindow* view = static_cast<ChatWindow*>(m_tabWidget->page(i));

        if (Preferences::closeButtons() && !Preferences::tabNotificationsLeds())
        {
            if (m_viewTree)
                m_viewTree->setViewIcon(view, images->getCloseIcon());
            else if (m_tabWidget)
                m_tabWidget->setTabIconSet(view, images->getCloseIcon());
        }
    }
}

void ViewContainer::setViewNotification(ChatWindow* view, const Konversation::TabNotifyType& type)
{
    if (!view || view == m_tabWidget->currentPage())
        return;

    if (type < Konversation::tnfControl && (m_activeViewOrderList.find(view) == m_activeViewOrderList.end()))
        m_activeViewOrderList.append(view);

    if (!Preferences::tabNotificationsLeds() && !Preferences::tabNotificationsText())
        return;

    if (m_viewTree)
    {
        switch (type)
        {
            case Konversation::tnfNormal:
                if (Preferences::tabNotificationsMsgs())
                {
                    if (Preferences::tabNotificationsLeds())
                        m_viewTree->setViewIcon(view, images->getMsgsLed(true));
                    if (Preferences::tabNotificationsText())
                        m_viewTree->setViewColor(view, Preferences::tabNotificationsMsgsColor());
                }
                break;

            case Konversation::tnfPrivate:
                if (Preferences::tabNotificationsPrivate())
                {
                    if (Preferences::tabNotificationsLeds())
                        m_viewTree->setViewIcon(view, images->getPrivateLed(true));
                    if (Preferences::tabNotificationsText())
                        m_viewTree->setViewColor(view, Preferences::tabNotificationsPrivateColor());
                }
                break;

            case Konversation::tnfSystem:
                if (Preferences::tabNotificationsSystem())
                {
                    if (Preferences::tabNotificationsLeds())
                        m_viewTree->setViewIcon(view, images->getSystemLed(true));
                    if (Preferences::tabNotificationsText())
                        m_viewTree->setViewColor(view, Preferences::tabNotificationsSystemColor());
                }
                break;

            case Konversation::tnfControl:
                if (Preferences::tabNotificationsEvents())
                {
                    if (Preferences::tabNotificationsLeds())
                        m_viewTree->setViewIcon(view, images->getEventsLed());
                    if (Preferences::tabNotificationsText())
                        m_viewTree->setViewColor(view, Preferences::tabNotificationsEventsColor());
                }
                break;

            case Konversation::tnfNick:
                if (Preferences::tabNotificationsNick())
                {
                    if (Preferences::tabNotificationsOverride() && Preferences::highlightNick())
                    {
                        if (Preferences::tabNotificationsLeds())
                            m_viewTree->setViewIcon(view, images->getLed(Preferences::highlightNickColor(),true));
                        if (Preferences::tabNotificationsText())
                            m_viewTree->setViewColor(view, Preferences::highlightNickColor());
                    }
                    else
                    {
                        if (Preferences::tabNotificationsLeds())
                            m_viewTree->setViewIcon(view, images->getNickLed());
                        if (Preferences::tabNotificationsText())
                            m_viewTree->setViewColor(view, Preferences::tabNotificationsNickColor());
                    }
                }
                else
                {
                    setViewNotification(view,Konversation::tnfNormal);
                }
                break;

            case Konversation::tnfHighlight:
                if (Preferences::tabNotificationsHighlights())
                {
                    if (Preferences::tabNotificationsOverride() && view->highlightColor().isValid())
                    {
                        if (Preferences::tabNotificationsLeds())
                            m_viewTree->setViewIcon(view, images->getLed(view->highlightColor(),true));
                        if (Preferences::tabNotificationsText())
                            m_viewTree->setViewColor(view, view->highlightColor());
                    }
                    else
                    {
                        if (Preferences::tabNotificationsLeds())
                            m_viewTree->setViewIcon(view, images->getHighlightsLed());
                        if (Preferences::tabNotificationsText())
                            m_viewTree->setViewColor(view, Preferences::tabNotificationsHighlightsColor());
                    }
                }
                else
                {
                    setViewNotification(view,Konversation::tnfNormal);
                }
                break;

            default:
                break;
        }
    }
    else if (m_tabWidget)
    {
        switch (type)
        {
            case Konversation::tnfNormal:
                if (Preferences::tabNotificationsMsgs())
                {
                    if (Preferences::tabNotificationsLeds())
                        m_tabWidget->setTabIconSet(view, images->getMsgsLed(true));
                    if (Preferences::tabNotificationsText())
                        m_tabWidget->setTabColor(view, Preferences::tabNotificationsMsgsColor());
                }
                break;

            case Konversation::tnfPrivate:
                if (Preferences::tabNotificationsPrivate())
                {
                    if (Preferences::tabNotificationsLeds())
                        m_tabWidget->setTabIconSet(view, images->getPrivateLed(true));
                    if (Preferences::tabNotificationsText())
                        m_tabWidget->setTabColor(view, Preferences::tabNotificationsPrivateColor());
                }
                break;

            case Konversation::tnfSystem:
                if (Preferences::tabNotificationsSystem())
                {
                    if (Preferences::tabNotificationsLeds())
                        m_tabWidget->setTabIconSet(view, images->getSystemLed(true));
                    if (Preferences::tabNotificationsText())
                        m_tabWidget->setTabColor(view, Preferences::tabNotificationsSystemColor());
                }
                break;

            case Konversation::tnfControl:
                if (Preferences::tabNotificationsEvents())
                {
                    if (Preferences::tabNotificationsLeds())
                        m_tabWidget->setTabIconSet(view, images->getEventsLed());
                    if (Preferences::tabNotificationsText())
                        m_tabWidget->setTabColor(view, Preferences::tabNotificationsEventsColor());
                }
                break;

            case Konversation::tnfNick:
                if (Preferences::tabNotificationsNick())
                {
                    if (Preferences::tabNotificationsOverride() && Preferences::highlightNick())
                    {
                        if (Preferences::tabNotificationsLeds())
                            m_tabWidget->setTabIconSet(view, images->getLed(Preferences::highlightNickColor(),true));
                        if (Preferences::tabNotificationsText())
                            m_tabWidget->setTabColor(view, Preferences::highlightNickColor());
                    }
                    else
                    {
                        if (Preferences::tabNotificationsLeds())
                            m_tabWidget->setTabIconSet(view, images->getNickLed());
                        if (Preferences::tabNotificationsText())
                            m_tabWidget->setTabColor(view, Preferences::tabNotificationsNickColor());
                    }
                }
                else
                {
                    setViewNotification(view,Konversation::tnfNormal);
                }
                break;

            case Konversation::tnfHighlight:
                if (Preferences::tabNotificationsHighlights())
                {
                    if (Preferences::tabNotificationsOverride() && view->highlightColor().isValid())
                    {
                        if (Preferences::tabNotificationsLeds())
                            m_tabWidget->setTabIconSet(view, images->getLed(view->highlightColor(),true));
                        if (Preferences::tabNotificationsText())
                            m_tabWidget->setTabColor(view, view->highlightColor());
                    }
                    else
                    {
                        if (Preferences::tabNotificationsLeds())
                            m_tabWidget->setTabIconSet(view, images->getHighlightsLed());
                        if (Preferences::tabNotificationsText())
                            m_tabWidget->setTabColor(view, Preferences::tabNotificationsHighlightsColor());
                    }
                }
                else
                {
                    setViewNotification(view,Konversation::tnfNormal);
                }
                break;

            default:
                break;
        }
    }
}

void ViewContainer::unsetViewNotification(ChatWindow* view)
{
    if (m_viewTree)
    {
        if (Preferences::tabNotificationsLeds())
        {
            switch (view->getType())
            {
                case ChatWindow::Channel:
                case ChatWindow::DccChat:
                    m_viewTree->setViewIcon(view, images->getMsgsLed(false));
                    break;

                case ChatWindow::Query:
                    m_viewTree->setViewIcon(view, images->getPrivateLed(false));
                    break;

                case ChatWindow::Status:
                    m_viewTree->setViewIcon(view, images->getServerLed(false));
                    break;

                default:
                    m_viewTree->setViewIcon(view, images->getSystemLed(false));
                    break;
            }
        }

        QColor textColor = (Preferences::inputFieldsBackgroundColor()
            ? Preferences::color(Preferences::ChannelMessage) : m_window->colorGroup().foreground());

        if (view->getType() == ChatWindow::Channel)
        {
            Channel *channel = static_cast<Channel*>(view);

            if (!channel->joined())
                textColor = KonversationApplication::instance()->palette(m_viewTree).disabled().text();
        }
        else if (view->getType() == ChatWindow::Query)
        {
            if (!view->getServer()->isConnected())
                textColor = KonversationApplication::instance()->palette(m_tabWidget).disabled().text();
        }

        m_viewTree->setViewColor(view, textColor);
    }
    else if (m_tabWidget)
    {
        if (Preferences::tabNotificationsLeds())
        {
            switch (view->getType())
            {
                case ChatWindow::Channel:
                case ChatWindow::DccChat:
                    m_tabWidget->setTabIconSet(view, images->getMsgsLed(false));
                    break;

                case ChatWindow::Query:
                    m_tabWidget->setTabIconSet(view, images->getPrivateLed(false));
                    break;

                case ChatWindow::Status:
                    m_tabWidget->setTabIconSet(view, images->getServerLed(false));
                    break;

                default:
                    m_tabWidget->setTabIconSet(view, images->getSystemLed(false));
                    break;
            }
        }

        QColor textColor = m_window->colorGroup().foreground();

        if (view->getType() == ChatWindow::Channel)
        {
            Channel *channel = static_cast<Channel*>(view);

            if (!channel->joined())
                textColor = KonversationApplication::instance()->palette(m_tabWidget).disabled().text();
        }
        else if (view->getType() == ChatWindow::Query)
        {
            if (!view->getServer()->isConnected())
                textColor = KonversationApplication::instance()->palette(m_tabWidget).disabled().text();
        }

        m_tabWidget->setTabColor(view, textColor);
    }

    QValueList<ChatWindow*>::iterator it = m_activeViewOrderList.find(view);

    if (it != m_activeViewOrderList.end())
        m_activeViewOrderList.remove(it);
}

void ViewContainer::toggleViewNotifications()
{
    ChatWindow* view = 0;

    if (m_popupViewIndex == -1)
        view = static_cast<ChatWindow*>(m_tabWidget->currentPage());
    else
        view = static_cast<ChatWindow*>(m_tabWidget->page(m_popupViewIndex));

    if (view)
    {
        if (!view->notificationsEnabled())
        {
            view->setNotificationsEnabled(true);
            updateViews();
            KToggleAction* action = static_cast<KToggleAction*>(actionCollection()->action("tab_notifications"));
            if (action) action->setChecked(view->notificationsEnabled());
        }
        else
        {
            view->setNotificationsEnabled(false);
            unsetViewNotification(view);
            KToggleAction* action = static_cast<KToggleAction*>(actionCollection()->action("tab_notifications"));
            if (action) action->setChecked(view->notificationsEnabled());
        }
    }

    m_popupViewIndex = -1;
}

void ViewContainer::toggleAutoJoin()
{
    Channel* channel = 0;

    if (m_popupViewIndex == -1)
        channel = static_cast<Channel*>(m_tabWidget->currentPage());
    else
        channel = static_cast<Channel*>(m_tabWidget->page(m_popupViewIndex));

    if (channel && channel->getType() == ChatWindow::Channel)
    {
        bool autoJoin = channel->autoJoin();

        channel->setAutoJoin(!autoJoin);

        emit autoJoinToggled(channel->getServer()->getServerGroup());
    }
}

void ViewContainer::addView(ChatWindow* view, const QString& label, bool weinitiated)
{
    ChatWindow *tmp_ChatWindow;
    int placement = -1;
    ChatWindow::WindowType wtype;
    QIconSet iconSet;

    connect(KonversationApplication::instance(), SIGNAL(appearanceChanged()), view, SLOT(updateAppearance()));
    connect(view, SIGNAL(setStatusBarTempText(const QString&)), this, SIGNAL(setStatusBarTempText(const QString&)));
    connect(view, SIGNAL(clearStatusBarTempText()), this, SIGNAL(clearStatusBarTempText()));
    connect(view, SIGNAL(closing(ChatWindow*)), this, SIGNAL(removeView(ChatWindow*)));
    connect(view, SIGNAL(closing(ChatWindow*)), this, SLOT(cleanupAfterClose(ChatWindow*)));

    // Please be careful about changing any of the grouping behavior in here,
    // because it needs to match up with the sorting behavior of the tree list,
    // otherwise they may become out of sync, wreaking havoc with the move
    // actions. Yes, this would do well with a more reliable approach in the
    // future. Then again, while this is ugly, it's also very fast.
    switch (view->getType())
    {
        case ChatWindow::Channel:
            if (Preferences::tabNotificationsLeds())
                iconSet = images->getMsgsLed(false);
            else if (Preferences::closeButtons())
                iconSet = images->getCloseIcon();

            for (int sindex = 0; sindex < m_tabWidget->count(); sindex++)
            {
                tmp_ChatWindow = static_cast<ChatWindow *>(m_tabWidget->page(sindex));

                if (tmp_ChatWindow->getType() == ChatWindow::Status && tmp_ChatWindow->getServer() == view->getServer())
                {
                    for (int index = sindex + 1; index < m_tabWidget->count(); index++)
                    {
                        tmp_ChatWindow = static_cast<ChatWindow *>(m_tabWidget->page(index));
                        wtype = tmp_ChatWindow->getType();

                        if (wtype != ChatWindow::Channel && wtype != ChatWindow::RawLog)
                        {
                            placement = index;
                            break;
                        }
                    }

                    break;
                }
            }

            break;

        case ChatWindow::RawLog:
            if (Preferences::tabNotificationsLeds())
                iconSet = images->getSystemLed(false);
            else if (Preferences::closeButtons())
                iconSet = images->getCloseIcon();

            for (int sindex = 0; sindex < m_tabWidget->count(); sindex++)
            {
                tmp_ChatWindow = static_cast<ChatWindow *>(m_tabWidget->page(sindex));

                if (tmp_ChatWindow->getType() == ChatWindow::Status && tmp_ChatWindow->getServer() == view->getServer())
                {
                    placement = sindex + 1;
                    break;
                }
            }

            break;

        case ChatWindow::Query:
            if (Preferences::tabNotificationsLeds())
                iconSet = images->getPrivateLed(false);
            else if (Preferences::closeButtons())
                iconSet = images->getCloseIcon();

            for (int sindex = 0; sindex < m_tabWidget->count(); sindex++)
            {
                tmp_ChatWindow = static_cast<ChatWindow *>(m_tabWidget->page(sindex));

                if (tmp_ChatWindow->getType() == ChatWindow::Status && tmp_ChatWindow->getServer() == view->getServer())
                {
                    for (int index = sindex + 1; index < m_tabWidget->count(); index++)
                    {
                        tmp_ChatWindow = static_cast<ChatWindow *>(m_tabWidget->page(index));
                        wtype = tmp_ChatWindow->getType();

                        if (wtype != ChatWindow::Channel && wtype != ChatWindow::RawLog && wtype != ChatWindow::Query)
                        {
                            placement = index;
                            break;
                        }
                    }

                    break;
                }
            }

            break;

        case ChatWindow::DccChat:
            if (Preferences::tabNotificationsLeds())
                iconSet = images->getMsgsLed(false);
            else if (Preferences::closeButtons())
                iconSet = images->getCloseIcon();

            for (int sindex = 0; sindex < m_tabWidget->count(); sindex++)
            {
                tmp_ChatWindow = static_cast<ChatWindow*>(m_tabWidget->page(sindex));
                wtype = tmp_ChatWindow->getType();

                if (wtype != ChatWindow::Status && wtype != ChatWindow::Channel
                    && wtype != ChatWindow::RawLog && wtype != ChatWindow::Query
                    && wtype != ChatWindow::DccChat && wtype != ChatWindow::ChannelList)
                {
                    placement = sindex;
                    break;
                }
            }
            break;

        case ChatWindow::Status:
            if (Preferences::tabNotificationsLeds())
                iconSet = images->getServerLed(false);
            else if (Preferences::closeButtons())
                iconSet = images->getCloseIcon();

            if (m_viewTree)
            {
                for (int sindex = 0; sindex < m_tabWidget->count(); sindex++)
                {
                    tmp_ChatWindow = static_cast<ChatWindow *>(m_tabWidget->page(sindex));

                    if (tmp_ChatWindow->getType() != ChatWindow::Channel
                        && tmp_ChatWindow->getType() != ChatWindow::Status
                        && tmp_ChatWindow->getType() != ChatWindow::RawLog
                        && tmp_ChatWindow->getType() != ChatWindow::Query
                        && tmp_ChatWindow->getType() != ChatWindow::DccChat)
                    {
                        placement = sindex;
                        break;
                    }
                }
            }
            break;

        case ChatWindow::ChannelList:
            if (Preferences::tabNotificationsLeds())
                iconSet = images->getSystemLed(false);
            else if (Preferences::closeButtons())
                iconSet = images->getCloseIcon();

            for (int sindex = 0; sindex < m_tabWidget->count(); sindex++)
            {
                tmp_ChatWindow = static_cast<ChatWindow *>(m_tabWidget->page(sindex));

                if (tmp_ChatWindow->getServer() == view->getServer())
                    placement = sindex + 1;
            }

            break;

        default:
            if (Preferences::tabNotificationsLeds())
                iconSet = images->getSystemLed(false);
            else if (Preferences::closeButtons())
                iconSet = images->getCloseIcon();
            break;
    }

    m_tabWidget->insertTab(view, iconSet, label, placement);
    m_vbox->show();//m_tabWidget->show();

    if (m_viewTree)
    {
        if (placement != -1 && m_tabWidget->page(placement-1))
        {
            ChatWindow* after = static_cast<ChatWindow*>(m_tabWidget->page(placement-1));
            m_viewTree->addView(label, view, iconSet, false, after);
        }
        else
            m_viewTree->addView(label, view, iconSet);
    }

    // Check, if user was typing in old input line
    bool doBringToFront=false;

    if (Preferences::focusNewQueries() && view->getType()==ChatWindow::Query && !weinitiated)
        doBringToFront = true;

    if (Preferences::bringToFront() && view->getType()!=ChatWindow::RawLog)
        doBringToFront = true;

    // make sure that bring to front only works when the user wasn't typing something
    if (m_frontView && view->getType() != ChatWindow::UrlCatcher && view->getType() != ChatWindow::Konsole)
    {
        if (!m_frontView->getTextInLine().isEmpty())
            doBringToFront = false;
    }

    if (doBringToFront) showView(view);

    updateViewActions(m_tabWidget->currentPageIndex());
}

void ViewContainer::switchView(QWidget* newView)
{
    ChatWindow* view = static_cast<ChatWindow*>(newView);

    emit viewChanged(view);

    if (m_frontView)
    {
        m_frontView->resetTabNotification();

        disconnect(m_frontView, SIGNAL(updateInfo(const QString &)), this, SIGNAL(setStatusBarInfoLabel(const QString &)));

        if (Preferences::automaticRememberLine() && m_frontView->isInsertSupported())
            m_frontView->getTextView()->insertRememberLine();
    }

    m_frontView = 0;
    m_searchView = 0;

    setFrontServer(view->getServer());

    // display this server's lag time
    if (m_frontServer)
    {
        updateStatusBarSSLLabel(m_frontServer);
        updateStatusBarLagLabel(m_frontServer, m_frontServer->getLag());
    }

    emit clearStatusBarTempText();

    updateFrontView();

    unsetViewNotification(view);

    view->resetTabNotification();

    if (!m_viewTree || !m_viewTree->hasFocus()) view->adjustFocus();

    if (view->isInsertSupported()) view->getTextView()->cancelRememberLine();

    updateViewEncoding(view);

    QString tabName = Konversation::removeIrcMarkup(view->getName());

    if (tabName != "ChatWindowObject")
        emit setWindowCaption(tabName);
    else
        emit setWindowCaption(QString());
}

void ViewContainer::showView(ChatWindow* view)
{
    // Don't bring Tab to front if TabWidget is hidden. Otherwise QT gets confused
    // and shows the Tab as active but will display the wrong pane
    if (m_tabWidget && m_tabWidget->isVisible())
        m_tabWidget->showPage(view);
}

void ViewContainer::goToView(int page)
{
    if (page == m_tabWidget->currentPageIndex())
      return;

    if (page > m_tabWidget->count())
        return;

    if (page >= m_tabWidget->count())
        page = 0;
    else if (page < 0)
        page = m_tabWidget->count() - 1;

    if (page >= 0)
        m_tabWidget->setCurrentPage(page);


    m_popupViewIndex = -1;
}

void ViewContainer::showNextView()
{
    goToView(m_tabWidget->currentPageIndex()+1);
}

void ViewContainer::showPreviousView()
{
    goToView(m_tabWidget->currentPageIndex()-1);
}

void ViewContainer::moveViewLeft()
{
    int index;

    if (m_popupViewIndex == -1)
        index = m_tabWidget->currentPageIndex();
    else
        index = m_popupViewIndex;

    if (index)
    {
        if (m_viewTree)
        {
            ChatWindow* view = static_cast<ChatWindow*>(m_tabWidget->page(index));
            m_viewTree->moveViewUp(view);
            syncTabBarToTree();
        }
        else if (m_tabWidget)
        {
            m_tabWidget->moveTab(index, index - 1);
            updateViewActions(index - 1);
        }
    }

    m_popupViewIndex = -1;
}

void ViewContainer::moveViewRight()
{
    int index;

    if (m_popupViewIndex == -1)
        index = m_tabWidget->currentPageIndex();
    else
        index = m_popupViewIndex;

    if (index < (m_tabWidget->count() - 1))
    {
        if (m_viewTree)
        {
            ChatWindow* view = static_cast<ChatWindow*>(m_tabWidget->page(index));
            m_viewTree->moveViewDown(view);
            syncTabBarToTree();
        }
        else if (m_tabWidget)
        {
            m_tabWidget->moveTab(index, index + 1);
            updateViewActions(index + 1);
        }
    }

    m_popupViewIndex = -1;
}

void ViewContainer::closeView(QWidget* view)
{
    ChatWindow* viewToClose = static_cast<ChatWindow*>(view);

    closeView(viewToClose);
}

void ViewContainer::closeView(ChatWindow* view)
{
    if (view)
    {
        ChatWindow::WindowType viewType = view->getType();

        bool closeConfirmed = true;

        switch (viewType)
        {
            case ChatWindow::DccTransferPanel:
                closeDccPanel();
                break;
            case ChatWindow::UrlCatcher:
                closeUrlCatcher();
                break;
            case ChatWindow::NicksOnline:
                closeNicksOnlinePanel();
                break;
            default:
                closeConfirmed = view->closeYourself();
                break;
        }
    }
}

void ViewContainer::cleanupAfterClose(ChatWindow* view)
{
    if (view == m_frontView) m_frontView = 0;

    if (m_tabWidget)
    {
        m_tabWidget->removePage(view);
        emit removeView(view);

        if (m_tabWidget->count() <= 0)
        {
            m_saveSplitterSizesLock = true;
            m_vbox->hide();
            emit resetStatusBar();
            emit setWindowCaption(QString::null);
        }
    }

    // Remove the view from the active view list if it's still on it
    QValueList<ChatWindow*>::iterator it = m_activeViewOrderList.find(view);

    if (it != m_activeViewOrderList.end())
        m_activeViewOrderList.remove(it);

    if (view->getType() == ChatWindow::Query)
        --m_queryViewCount;

    if (m_queryViewCount == 0 && actionCollection())
    {
        KAction* action = actionCollection()->action("close_queries");
        if (action) action->setEnabled(false);
    }
}

void ViewContainer::closeViewMiddleClick(QWidget* view)
{
    if (Preferences::middleClickClose())
        closeView(view);
}

void ViewContainer::closeCurrentView()
{
    if (m_popupViewIndex == -1)
        closeView(m_tabWidget->currentPage());
    else
        closeView(m_tabWidget->page(m_popupViewIndex));

    m_popupViewIndex = -1;
}

void ViewContainer::changeViewCharset(int index)
{
    ChatWindow* chatWin;

    if (m_popupViewIndex == -1)
        chatWin = static_cast<ChatWindow*>(m_tabWidget->currentPage());
    else
        chatWin = static_cast<ChatWindow*>(m_tabWidget->page(m_popupViewIndex));

    if (chatWin)
    {
        if (index == 0)
            chatWin->setChannelEncoding(QString());
        else
            chatWin->setChannelEncoding(Konversation::IRCCharsets::self()->availableEncodingShortNames()[index - 1]);
    }

    m_popupViewIndex = -1;
}

void ViewContainer::updateViewEncoding(ChatWindow* view)
{
    if (view)
    {
        ChatWindow::WindowType viewType = view->getType();
        KSelectAction* codecAction = static_cast<KSelectAction*>(actionCollection()->action("tab_encoding"));

        if (codecAction)
        {
            if(viewType == ChatWindow::Channel || viewType == ChatWindow::Query || viewType == ChatWindow::Status)
            {
                codecAction->setEnabled(view->isChannelEncodingSupported());
                QString encoding = view->getChannelEncoding();

                if(m_frontServer)
                {
                    codecAction->changeItem(0, i18n("Default encoding", "Default ( %1 )").arg(m_frontServer->getIdentity()->getCodecName()));
                }

                if(encoding.isEmpty())
                {
                    codecAction->setCurrentItem(0);
                }
                else
                {
                    codecAction->setCurrentItem(Konversation::IRCCharsets::self()->shortNameToIndex(encoding) + 1);
                }
            }
            else
            {
                codecAction->setEnabled(false);
            }
        }
    }
}

void ViewContainer::showViewContextMenu(QWidget* tab, const QPoint& pos)
{
    m_popupViewIndex = m_tabWidget->indexOf(tab);

    updateViewActions(m_popupViewIndex);
    QPopupMenu* menu = static_cast<QPopupMenu*>(m_window->factory()->container("tabContextMenu", m_window));

    if (!menu) return;

    ChatWindow* view = static_cast<ChatWindow*>(tab);
    KToggleAction* autoJoinAction = static_cast<KToggleAction*>(actionCollection()->action("tab_autojoin"));
    KAction* rejoinAction = actionCollection()->action("rejoin_channel");

    if (view)
    {
        ChatWindow::WindowType viewType = view->getType();

        updateViewEncoding(view);

        if (viewType == ChatWindow::Channel)
        {
            autoJoinAction->plug(menu, 1);

            Channel *channel = static_cast<Channel*>(view);
            if (channel->rejoinable() && rejoinAction)
            {
                rejoinAction->plug(menu, 0);
                rejoinAction->setEnabled(true);
            }
        }

        if (viewType == ChatWindow::Status)
        {
            QPtrList<KAction> serverActions;
            KAction* action = actionCollection()->action("disconnect_server");
            if (action) serverActions.append(action);
            action = actionCollection()->action("reconnect_server");
            if (action) serverActions.append(action);
            action = actionCollection()->action("join_channel");
            if (action) serverActions.append(action);
            action = new KActionSeparator();
            if (action) serverActions.append(action);
            m_window->plugActionList("server_actions", serverActions);
            m_contextServer = view->getServer();
        }
        else
            m_contextServer = 0;
    }

    if (menu->exec(pos) == -1)
    {
        m_popupViewIndex = -1;
        view = static_cast<ChatWindow*>(m_tabWidget->currentPage());

        if (view) updateViewEncoding(view);
    }

    autoJoinAction->unplug(menu);
    rejoinAction->unplug(menu);

    m_window->unplugActionList("server_actions");

    emit contextMenuClosed();

    updateViewActions(m_tabWidget->currentPageIndex());
}

QString ViewContainer::currentViewTitle()
{
    if (m_frontServer)
    {
        if (m_frontView && m_frontView->getType() == ChatWindow::Channel)
            return m_frontView->getName();
        else
            return m_frontServer->getDisplayName();
    }
    else
    {
        return QString();
    }
}

QString ViewContainer::currentViewURL(bool passNetwork)
{
    QString url;
    QString channel;
    QString port;
    QString server;

    if (m_frontServer && m_frontView)
    {
        updateFrontView();

        if (m_frontView->getType() == ChatWindow::Channel)
            channel = m_frontView->getName();

        if (passNetwork)
            server = m_frontServer->getDisplayName();
        else
        {
            server = m_frontServer->getServerName();
            port = ':'+QString::number(m_frontServer->getPort());
        }

        if (server.contains(':')) // IPv6
            server = '['+server+']';

        url = "irc://"+server+port+'/'+channel;
    }

    return url;
}

int ViewContainer::getViewIndex(QWidget* widget)
{
    return m_tabWidget->indexOf(widget);
}

void ViewContainer::clearView()
{
    if (m_frontView) m_frontView->getTextView()->clear();
}

void ViewContainer::clearAllViews()
{
    int total=m_tabWidget->count()-1;
    ChatWindow* nextPage;

    for(int i=0;i<=total;i++)
    {
        nextPage=static_cast<ChatWindow*>(m_tabWidget->page(i));

        if(nextPage && nextPage->getTextView())
            nextPage->getTextView()->clear();
    }
}

void ViewContainer::findText()
{
    if (!m_searchView)
    {
        KMessageBox::sorry(m_window,
            i18n("You can only search in text fields."),
            i18n("Find Text Information"));
    }
    else
    {
        m_searchView->getTextView()->search();
    }
}

void ViewContainer::findNextText()
{
    if (m_searchView) m_searchView->getTextView()->searchAgain();
}

void ViewContainer::findPrevText()
{
    if (m_searchView) m_searchView->getTextView()->searchNext(true);
}

void ViewContainer::appendToFrontmost(const QString& type,const QString& message,ChatWindow* serverView, bool parseURL)
{
    if (!m_tabWidget) return;

    if (!serverView) // e.g. DCOP info call
    {
        if (m_frontView) // m_frontView == NULL if canBeFrontView() == false for active ChatWindow
            serverView = m_frontView->getServer()->getStatusView();
        else if (m_frontServer) // m_fronView == NULL && m_frontServer != NULL if ChannelListPanel is active.
            serverView = m_frontServer->getStatusView();
    }

    // This might happen if canBeFrontView() is false for active ChatWindow
    // and the view does not belong to any server (e.g. DCC Status View).
    // Discard message in this case.
    if (!serverView) return;

    updateFrontView();

    if (!m_frontView ||                           // Check if the m_frontView can actually display text or ...
                                                  // if it does not belong to this server or...
        serverView->getServer()!=m_frontView->getServer() ||
                                                  // if the user decided to force it.
        Preferences::redirectServerAndAppMsgToStatusPane())
    {
        // if not, take server specified fallback view instead
        serverView->appendServerMessage(type,  message, parseURL);
        // FIXME: this signal should be sent from the status panel instead, so it
        //        can be using the correct highlight color, would be more consistent
        //        anyway!
        // FIXME newText(serverView,QString::null,true);
    }
    else
        m_frontView->appendServerMessage(type, message, parseURL);
}

void ViewContainer::insertCharacter()
{
    QFont font;

    if (Preferences::customTextFont())
        font = Preferences::textFont();
    else
        font = KGlobalSettings::generalFont();

    if (!m_insertCharDialog)
    {
        m_insertCharDialog = new Konversation::InsertCharDialog(font.family(), m_window);
        connect(m_insertCharDialog, SIGNAL(insertChar(const QChar&)), this, SLOT(insertChar(const QChar&)));
    }

    m_insertCharDialog->setFont(font);
    m_insertCharDialog->show();
}

void ViewContainer::insertChar(const QChar& chr)
{
    ChatWindow* view = static_cast<ChatWindow*>(m_tabWidget->currentPage());

    if (view) view->appendInputText(chr, true/*fromCursor*/);
}

void ViewContainer::insertIRCColor()
{
    IRCColorChooser dlg(m_window);

    if (dlg.exec() == KDialog::Accepted) m_frontView->appendInputText(dlg.color(), true/*fromCursor*/);
}

void ViewContainer::clearViewLines()
{
    if (m_frontView && m_frontView->isInsertSupported())
    {
        m_frontView->getTextView()->clearLines();

        KAction* action = actionCollection()->action("clear_lines");
        if (action) action->setEnabled(false);
    }
}

void ViewContainer::insertRememberLine()
{
    if (Preferences::automaticRememberLine())
    {
        if (m_frontView && m_frontView->isInsertSupported())
            m_frontView->getTextView()->insertRememberLine();
    }
}

void ViewContainer::insertRememberLines(Server* server)
{
    for (int i = 0; i <  m_tabWidget->count(); ++i)
    {
        ChatWindow* view = static_cast<ChatWindow*>(m_tabWidget->page(i));

        if (view->getServer() == server && view->isInsertSupported())
            view->getTextView()->insertRememberLine();
    }
}

void ViewContainer::cancelRememberLine()
{
    if (m_frontView && m_frontView->isInsertSupported())
    {
        m_frontView->getTextView()->cancelRememberLine();

        KAction* action = actionCollection()->action("clear_lines");
        if (action) action->setEnabled(m_frontView->getTextView()->hasLines());
    }
}

void ViewContainer::insertMarkerLine()
{
    if (Preferences::markerLineInAllViews())
    {
        int total = m_tabWidget->count()-1;
        ChatWindow* view;

        for (int i = 0; i <= total; ++i)
        {
            view = static_cast<ChatWindow*>(m_tabWidget->page(i));

            if (view->isInsertSupported()) view->getTextView()->insertMarkerLine();
        }
    }
    else
    {
        if (m_frontView && m_frontView->isInsertSupported())
            m_frontView->getTextView()->insertMarkerLine();
    }

    if (m_frontView && m_frontView->isInsertSupported())
    {
        KAction* action = actionCollection()->action("clear_lines");
        if (action) action->setEnabled(m_frontView->getTextView()->hasLines());
    }
}

void ViewContainer::openLogFile()
{
    if (m_frontView)
    {
        ChatWindow* view=static_cast<ChatWindow*>(m_frontView);
        ChatWindow::WindowType viewType=view->getType();
        if (viewType==ChatWindow::Channel || viewType==ChatWindow::Query ||
            viewType==ChatWindow::Status || viewType==ChatWindow::DccChat)
        {
            openLogFile(view->getName(), view->logFileName());
        }
    }
}

void ViewContainer::openLogFile(const QString& caption, const QString& file)
{
    if (!file.isEmpty())
    {
        LogfileReader* logReader = new LogfileReader(m_tabWidget, file);
        addView(logReader, i18n("Logfile of %1").arg(caption));
        logReader->setServer(0);
    }
}

void ViewContainer::addKonsolePanel()
{
    KonsolePanel* panel=new KonsolePanel(m_tabWidget);
    panel->setName(i18n("Konsole"));
    addView(panel, i18n("Konsole"));
    connect(panel, SIGNAL(updateTabNotification(ChatWindow*,const Konversation::TabNotifyType&)), this, SLOT(setViewNotification(ChatWindow*,const Konversation::TabNotifyType&)));
    connect(panel, SIGNAL(closeView(ChatWindow*)), this, SLOT(closeView(ChatWindow*)));
}

void ViewContainer::addUrlCatcher()
{
    // if the panel wasn't open yet
    if (m_urlCatcherPanel==0)
    {
        m_urlCatcherPanel=new UrlCatcher(m_tabWidget);
        addView(m_urlCatcherPanel, i18n("URL Catcher"));
        KonversationApplication *konvApp=static_cast<KonversationApplication *>(KApplication::kApplication());
        connect(konvApp,SIGNAL(catchUrl(const QString&,const QString&)),
            m_urlCatcherPanel, SLOT(addUrl(const QString&,const QString&)) );
        connect(m_urlCatcherPanel, SIGNAL(deleteUrl(const QString&,const QString&)),
            konvApp, SLOT(deleteUrl(const QString&,const QString&)) );
        connect(m_urlCatcherPanel, SIGNAL(clearUrlList()),
            konvApp, SLOT(clearUrlList()));

        QStringList urlList=konvApp->getUrlList();
        for(unsigned int index=0;index<urlList.count();index++)
        {
            QString urlItem=urlList[index];
            m_urlCatcherPanel->addUrl(urlItem.section(' ',0,0),urlItem.section(' ',1,1));
        }                                         // for
        (dynamic_cast<KToggleAction*>(actionCollection()->action("open_url_catcher")))->setChecked(true);
    }
    else
        closeUrlCatcher();
}

void ViewContainer::closeUrlCatcher()
{
    // if there actually is a dcc panel
    if (m_urlCatcherPanel)
    {
        delete m_urlCatcherPanel;
        m_urlCatcherPanel = 0;
        (dynamic_cast<KToggleAction*>(actionCollection()->action("open_url_catcher")))->setChecked(false);
    }
}

void ViewContainer::toggleDccPanel()
{
    if (m_dccPanel==0 || !m_dccPanelOpen)
        addDccPanel();
    else
        closeDccPanel();
}

void ViewContainer::addDccPanel()
{
    if (!m_dccPanelOpen)
    {
        addView(m_dccPanel, i18n("DCC Status"));
        m_dccPanelOpen=true;
        (dynamic_cast<KToggleAction*>(actionCollection()->action("open_dccstatus_window")))->setChecked(true);
    }
}

void ViewContainer::closeDccPanel()
{
    // if there actually is a dcc panel
    if (m_dccPanel)
    {
        // hide it from view, does not delete it
        emit removeView(m_dccPanel);
        if (m_tabWidget) m_tabWidget->removePage(m_dccPanel);
        m_dccPanelOpen=false;
        (dynamic_cast<KToggleAction*>(actionCollection()->action("open_dccstatus_window")))->setChecked(false);
    }
}

void ViewContainer::deleteDccPanel()
{
    if (m_dccPanel)
    {
        closeDccPanel();
        delete m_dccPanel;
        m_dccPanel=0;
    }
}

DccTransferPanel* ViewContainer::getDccPanel()
{
    return m_dccPanel;
}

void ViewContainer::addDccChat(const QString& myNick,const QString& nick,const QStringList& arguments,bool listen)
{
    if (!listen) // Someone else initiated dcc chat
    {
        KonversationApplication* konv_app=static_cast<KonversationApplication*>(KApplication::kApplication());
        konv_app->notificationHandler()->dccChat(m_frontView, nick);
    }

    if (m_frontServer)
    {
        DccChat* dccChatPanel=listen
            ? new DccChat(m_tabWidget, listen, m_frontServer, myNick, nick )
            : new DccChat(m_tabWidget, listen, m_frontServer, myNick, nick, arguments[1], arguments[2].toInt() );

        connect(dccChatPanel, SIGNAL(updateTabNotification(ChatWindow*,const Konversation::TabNotifyType&)), this, SLOT(setViewNotification(ChatWindow*,const Konversation::TabNotifyType&)));

        // This needs to be here as addView will change m_frontServer if focus new tabs is enabled.
        addView(dccChatPanel, dccChatPanel->getName());
    }
}

StatusPanel* ViewContainer::addStatusView(Server* server)
{
    StatusPanel* statusView = new StatusPanel(m_tabWidget);

    statusView->setServer(server);
    statusView->setIdentity(server->getIdentity());

    if (server->getServerGroup()) statusView->setNotificationsEnabled(server->getServerGroup()->enableNotifications());

    // Get group name for tab if available
    QString label = server->getDisplayName();
    statusView->setName(label);

    QObject::connect(server, SIGNAL(sslInitFailure()), this, SIGNAL(removeStatusBarSSLLabel()));
    QObject::connect(server, SIGNAL(sslConnected(Server*)), this, SIGNAL(updateStatusBarSSLLabel(Server*)));

    // ... then put it into the tab widget, otherwise we'd have a race with server member
    addView(statusView, label);

    connect(statusView, SIGNAL(updateTabNotification(ChatWindow*,const Konversation::TabNotifyType&)),
        this, SLOT(setViewNotification(ChatWindow*,const Konversation::TabNotifyType&)));
    connect(statusView, SIGNAL(sendFile()), server, SLOT(requestDccSend()));
    connect(server, SIGNAL(awayState(bool)), statusView, SLOT(indicateAway(bool)) );

    // make sure that m_frontServer gets set on adding the first status panel, too,
    // since there won't be a switchView happening
    if (!m_frontServer) setFrontServer(server);

    return statusView;
}

RawLog* ViewContainer::addRawLog(Server* server)
{
    RawLog* rawLog = new RawLog(m_tabWidget);
    rawLog->setServer(server);
    rawLog->setLog(false);

    if (server->getServerGroup()) rawLog->setNotificationsEnabled(server->getServerGroup()->enableNotifications());

    addView(rawLog, i18n("Raw Log"));

    connect(rawLog, SIGNAL(updateTabNotification(ChatWindow*,const Konversation::TabNotifyType&)),
        this, SLOT(setViewNotification(ChatWindow*,const Konversation::TabNotifyType&)));

    return rawLog;
}

void ViewContainer::reconnectFrontServer()
{
    Server* server = 0;

    if (m_contextServer)
        server = m_contextServer;
    else
        server = m_frontServer;

    if (server) server->reconnect();
}

void ViewContainer::disconnectFrontServer()
{
    Server* server = 0;

    if (m_contextServer)
        server = m_contextServer;
    else
        server = m_frontServer;

    if (server && server->isConnected())
        server->disconnect();
}

void ViewContainer::showJoinChannelDialog()
{
    Server* server = 0;

    if (m_contextServer)
        server = m_contextServer;
    else
        server = m_frontServer;

    if (!server)
        return;

    Konversation::JoinChannelDialog dlg(server, m_window);

    if (dlg.exec() == QDialog::Accepted)
        server->sendJoinCommand(dlg.channel(), dlg.password());
}

void ViewContainer::connectionStateChanged(Server* server, Konversation::ConnectionState state)
{
    Server* updateServer = 0;

    if (m_contextServer)
        updateServer = m_contextServer;
    else
        updateServer = m_frontServer;

    if (updateServer && updateServer == server)
    {
        KAction* action = actionCollection()->action("disconnect_server");
        if (action)
            action->setEnabled(state == Konversation::SSConnected);

        action = actionCollection()->action("join_channel");
        if (action)
            action->setEnabled(state == Konversation::SSConnected);

        if (m_frontView && m_frontView->getServer() == server
            && m_frontView->getType() == ChatWindow::Channel)
        {
            ChatWindow* view = m_frontView;
            Channel* channel = static_cast<Channel*>(view);

            action = actionCollection()->action("rejoin_channel");
            if (action) action->setEnabled(state == Konversation::SSConnected && channel->rejoinable());
        }
    }
}

void ViewContainer::channelJoined(Channel* channel)
{
    ChatWindow* view = m_frontView;

    if (view == channel)
    {
        KAction* action = actionCollection()->action("rejoin_channel");
        if (action) action->setEnabled(false);
    }
}

Channel* ViewContainer::addChannel(Server* server, const QString& name)
{
    Channel* channel=new Channel(m_tabWidget, name);
    channel->setServer(server);
    channel->setName(name); //still have to do this for now
    addView(channel, name);

    connect(this, SIGNAL(updateChannelAppearance()), channel, SLOT(updateAppearance()));
    connect(channel, SIGNAL(updateTabNotification(ChatWindow*,const Konversation::TabNotifyType&)), this, SLOT(setViewNotification(ChatWindow*,const Konversation::TabNotifyType&)));
    connect(server, SIGNAL(awayState(bool)), channel, SLOT(indicateAway(bool)) );
    connect(channel, SIGNAL(joined(Channel*)), this, SLOT(channelJoined(Channel*)));

    return channel;
}

void ViewContainer::rejoinChannel()
{
    Channel* channel = 0;

    if (m_popupViewIndex == -1)
        channel = static_cast<Channel*>(m_tabWidget->currentPage());
    else
        channel = static_cast<Channel*>(m_tabWidget->page(m_popupViewIndex));

    if (channel && channel->getType() == ChatWindow::Channel)
        channel->rejoin();
}

void ViewContainer::openChannelSettings()
{
    if (m_frontView->getType() == ChatWindow::Channel)
    {
        Channel* channel = static_cast<Channel*>(m_tabWidget->currentPage());
        channel->showOptionsDialog();
    }
}

void ViewContainer::toggleChannelNicklists()
{
    KToggleAction* action = static_cast<KToggleAction*>(actionCollection()->action("hide_nicknamelist"));

    if (action)
    {
        Preferences::setShowNickList(!action->isChecked());
        Preferences::writeConfig();

        emit updateChannelAppearance();
    }
}

Query* ViewContainer::addQuery(Server* server, const NickInfoPtr& nickInfo, bool weinitiated)
{
    QString name = nickInfo->getNickname();
    Query* query=new Query(m_tabWidget, name);
    query->setServer(server);
    query->setNickInfo(nickInfo); //still have to do this
    addView(query, name, weinitiated);

    // About to increase the number of queries, so enable the close action
    if (m_queryViewCount == 0)
        actionCollection()->action("close_queries")->setEnabled(true);

    ++m_queryViewCount;

    connect(query, SIGNAL(updateTabNotification(ChatWindow*,const Konversation::TabNotifyType&)), this, SLOT(setViewNotification(ChatWindow*,const Konversation::TabNotifyType&)));
    connect(query, SIGNAL(updateQueryChrome(ChatWindow*, const QString &)), this, SLOT(updateQueryChrome(ChatWindow*, const QString &)));
    connect(server, SIGNAL(awayState(bool)), query, SLOT(indicateAway(bool)));

    return query;
}

void ViewContainer::updateQueryChrome(ChatWindow* view, const QString& name)
{
    //FIXME: updateQueryChrome is a last minute fix for 0.19 because
    // the updateInfo mess is indecipherable. Replace with a sane and
    // encompassing system.

    QString newName = Konversation::removeIrcMarkup(name);

    if (!newName.isEmpty() && m_tabWidget->tabLabel(view) != newName)
    {
        if (m_viewTree) m_viewTree->setViewName(view, newName);
        if (m_tabWidget) m_tabWidget->setTabLabel(view, newName);
    }

    if (!newName.isEmpty() && view==m_frontView)
        emit setWindowCaption(newName);
}

void ViewContainer::closeQueries()
{
    int total=m_tabWidget->count()-1;
    int operations = 0;
    ChatWindow* nextPage;

    for (int i=0; i <=total; i++)
    {
        if (operations > total)
            break;

        nextPage = static_cast<ChatWindow*>(m_tabWidget->page(i));

        if (nextPage && nextPage->getType()==ChatWindow::Query)
        {
            closeView(nextPage);
            if (m_tabWidget->indexOf(nextPage) == -1) --i;
        }
        ++operations;
    }

    m_queryViewCount = 0;

    actionCollection()->action("close_queries")->setEnabled(false);
}

ChannelListPanel* ViewContainer::addChannelListPanel(Server* server)
{
    ChannelListPanel* channelListPanel=new ChannelListPanel(m_tabWidget);
    channelListPanel->setServer(server);
    addView(channelListPanel, i18n("Channel List"));

    KToggleAction* action = static_cast<KToggleAction*>(actionCollection()->action("open_channel_list"));
    if ((server == m_frontServer) && action) action->setChecked(true);

    return channelListPanel;
}

void ViewContainer::openChannelList(const QString& filter, bool getList)
{
    if (m_frontServer)
    {
        ChannelListPanel* panel = m_frontServer->getChannelListPanel();

        if (panel)
        {
            closeView(panel);
            KToggleAction* action = static_cast<KToggleAction*>(actionCollection()->action("open_channel_list"));
            if (action) action->setChecked(false);
        }
        else
        {
            int ret = KMessageBox::Continue;

            if (filter.isEmpty())
            {
                ret = KMessageBox::warningContinueCancel(m_window,i18n("Using this function may result in a lot "
                      "of network traffic. If your connection is not fast "
                      "enough, it is possible that your client will be "
                      "disconnected by the server."), i18n("Channel List Warning"),
                      KStdGuiItem::cont(), "ChannelListWarning");
            }

            if (ret != KMessageBox::Continue) return;

            panel = m_frontServer->addChannelListPanel();

            panel->setFilter(filter);

            if(getList) panel->applyFilterClicked();
        }
    }
    else
    {
        KMessageBox::information(m_window,
            i18n(
            "The channel list can only be opened from a "
            "query, channel or status window to find out, "
            "which server this list belongs to."
            ),
            i18n("Channel List"),
            "ChannelListNoServerSelected");
    }
}

void ViewContainer::openNicksOnlinePanel()
{
    if (!m_nicksOnlinePanel)
    {
        m_nicksOnlinePanel=new NicksOnline(m_window);
        addView(m_nicksOnlinePanel, i18n("Watched Nicks Online"));
        connect(m_nicksOnlinePanel, SIGNAL(editClicked()), m_window, SLOT(openNotify()));
        connect(m_nicksOnlinePanel, SIGNAL(doubleClicked(const QString&,const QString&)), m_window, SLOT(notifyAction(const QString&,const QString&)));
        connect(m_nicksOnlinePanel, SIGNAL(showView(ChatWindow*)), this, SLOT(showView(ChatWindow*)));
        connect(m_window, SIGNAL(nicksNowOnline(Server*)), m_nicksOnlinePanel, SLOT(updateServerOnlineList(Server*)));
        (dynamic_cast<KToggleAction*>(actionCollection()->action("open_nicksonline_window")))->setChecked(true);
    }
    else
    {
        closeNicksOnlinePanel();
        (dynamic_cast<KToggleAction*>(actionCollection()->action("open_nicksonline_window")))->setChecked(false);
    }

}

void ViewContainer::closeNicksOnlinePanel()
{
    if(m_nicksOnlinePanel)
    {
        delete m_nicksOnlinePanel;
        m_nicksOnlinePanel = 0;
    }
    (dynamic_cast<KToggleAction*>(actionCollection()->action("open_nicksonline_window")))->setChecked(false);
}

void ViewContainer::showNextActiveView()
{
    if (!m_activeViewOrderList.isEmpty())
    {
        ChatWindow* prev = m_activeViewOrderList.first();
        ChatWindow* view = prev;

        QValueList<ChatWindow*>::ConstIterator it;

        for (it = m_activeViewOrderList.begin(); it != m_activeViewOrderList.end(); ++it)
        {
            if ((*it)->currentTabNotification() < prev->currentTabNotification())
                view = (*it);
        }

        m_tabWidget->setCurrentPage(m_tabWidget->indexOf(view));
    }
}

/*!
    \fn ViewContainer::frontServerChanging(Server *newServer)

    This signal is emitted immediately before the front server is changed.

    If the server is being removed this will fire with a null pointer.
*/

#include "viewcontainer.moc"
