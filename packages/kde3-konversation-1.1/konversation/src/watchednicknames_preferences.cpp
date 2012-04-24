/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

/*
  Copyright (C) 2006 Dario Abatianni <eisfuchs@tigress.com>
*/

#include "watchednicknames_preferences.h"
#include "watchednicknames_preferencesui.h"
#include "config/preferences.h"
#include "konversationapplication.h"
#include "konversationmainwindow.h"
#include "valuelistviewitem.h"

#include <qlabel.h>
#include <qcombobox.h>
#include <qcheckbox.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kpushbutton.h>
#include <klineedit.h>
#include <klocale.h>
#include <kdebug.h>


WatchedNicknames_Config::WatchedNicknames_Config(QWidget *parent, const char *name)
 : WatchedNicknames_ConfigUI(parent, name)
{
  notifyListView->setRenameable(0,false);
  notifyListView->setSorting(-1);
  // reset flag to defined state (used to block signals when just selecting a new item)
  newItemSelected=false;

  loadSettings();

  connect(kcfg_UseNotify,SIGNAL (toggled(bool)),this,SLOT (checkIfEmptyListview(bool)) );
  connect(newButton,SIGNAL (clicked()),this,SLOT (newNotify()) );
  connect(removeButton,SIGNAL (clicked()),this,SLOT (removeNotify()) );
  connect(notifyListView,SIGNAL (selectionChanged(QListViewItem*)),this,SLOT (entrySelected(QListViewItem*)) );
  connect(notifyListView,SIGNAL (clicked(QListViewItem*)),this,SLOT (entrySelected(QListViewItem*)) );

  connect(networkDropdown,SIGNAL (activated(const QString&)),this,SLOT (networkChanged(const QString&)) );
  connect(nicknameInput,SIGNAL (textChanged(const QString&)),this,SLOT (nicknameChanged(const QString&)) );

  connect(KonversationApplication::instance(), SIGNAL(serverGroupsChanged(const Konversation::ServerGroupSettings*)),
      this, SLOT(updateNetworkNames()));
}

WatchedNicknames_Config::~WatchedNicknames_Config()
{
}

void WatchedNicknames_Config::restorePageToDefaults()
{
}

// fill in the notify listview with groups and nicknames
void WatchedNicknames_Config::loadSettings()
{
  // cleanup, so we won't add duplicate items
  notifyListView->clear();
  networkDropdown->clear();
  // make sure all widgets are disabled
  notifyListView->clearSelection();
  enableEditWidgets(false);

  // get list of server networks
  Konversation::ServerGroupList serverGroupList = Preferences::serverGroupList();

  // iterate through all networks in the server group list
  for(unsigned int gIndex=0;gIndex<serverGroupList.count();gIndex++)
  {
    // add server group branch to the notify listview so we can add notify items
    addNetworkBranch(serverGroupList[gIndex]);
  }
  // remember current list for hasChanged()
  m_oldNotifyList=currentNotifyList();
}

// adds a new network branch to the listview
void WatchedNicknames_Config::addNetworkBranch(Konversation::ServerGroupSettingsPtr serverGroupList)
{
  // get the current notify list and an iterator
  QMap<int, QStringList> notifyList = Preferences::notifyList();

  ValueListViewItem* groupItem=new ValueListViewItem(serverGroupList->id(),notifyListView,notifyListView->lastChild(),serverGroupList->name());
  // get the group iterator to find all servers in the group
  QMapConstIterator<int, QStringList> groupIt=notifyList.find(serverGroupList->id());

  // get list of nicks for the current group
  QStringList nicks=groupIt.data();
  // add group to dropdown list
  networkDropdown->insertItem(serverGroupList->name(),0);
  // add nicknames to group branch (reverse order again)
  for(unsigned int index=nicks.count();index;index--)
  {
    new KListViewItem(groupItem,nicks[index-1]);
  } // for
  // unfold group branch
  notifyListView->setOpen(groupItem,true);
}

// save list of notifies permanently, taken from the listview
void WatchedNicknames_Config::saveSettings()
{
  // create new in-memory notify structure
  QMap<int,QStringList> notifyList;

  // get first notify group
  KListView* listView=notifyListView;
  QListViewItem* group=listView->firstChild();

  // loop as long as there are more groups in the listview
  while(group)
  {
    int groupId=static_cast<ValueListViewItem*>(group)->getValue();

    // later contains all nicks separated by blanks
    QString nicks;
    // get first nick in the group
    QListViewItem* nick=group->firstChild();
    // loop as long as there are still nicks in this group
    while(nick)
    {
      // add nick to string container and add a blank
      nicks+=nick->text(0)+' ';
      // get next nick in the group
      nick=nick->nextSibling();
    } // while

    // write nick list to in-memory notify qstringlist
    notifyList.insert(groupId,QStringList::split(' ',nicks.stripWhiteSpace()));
    // get next group
    group=group->nextSibling();
  } // while

  // update in-memory notify list
  Preferences::setNotifyList(notifyList);
  static_cast<KonversationApplication*>(kapp)->saveOptions(false);

  // remember current list for hasChanged()
  m_oldNotifyList=currentNotifyList();
}

// returns the currently edited notify list
QStringList WatchedNicknames_Config::currentNotifyList()
{
  // prepare list
  QStringList newList;

  // get first item
  KListView* listView=notifyListView;
  QListViewItem* item=listView->firstChild();

  // loop as long as there are more groups in the listview
  while(item)
  {
    newList.append(item->text(0));
    item=item->itemBelow();
  } // while

  // return list
  return newList;
}

bool WatchedNicknames_Config::hasChanged()
{
  // return true if something has changed
  return(m_oldNotifyList!=currentNotifyList());
}

// slots

void WatchedNicknames_Config::updateNetworkNames()
{
  // get first notify group
  KListView* listView=notifyListView;
  QListViewItem* group=listView->firstChild();

  // make sure all widgets are disabled
  listView->clearSelection();
  enableEditWidgets(false);

  // kill dropdown list, the networks might have been renamed
  networkDropdown->clear();

  // loop as long as there are more groups in the listview
  while(group)
  {
    // get the group id from the listview item
    int groupId=static_cast<ValueListViewItem*>(group)->getValue();
    // get the name of the group by having a look at the serverGroupSettings
    Konversation::ServerGroupSettingsPtr serverGroup=Preferences::serverGroupById(groupId);

    // check if the server group still exists
    if(serverGroup)
    {
      // get the new name of the server group
      QString serverGroupName=serverGroup->name();

      // update the name of the group in the listview
      group->setText(0,serverGroupName);

      // re-add group to dropdown list
      networkDropdown->insertItem(serverGroupName,-1);
      // get next group
      group=group->nextSibling();
    }
    else
    {
      // get the next group from the listview
      QListViewItem* tmp=group->nextSibling();
      // remove the group
      delete group;
      // set the current group
      group=tmp;
    }
  } // while

  // get list of server networks
  Konversation::ServerGroupList serverGroupList = Preferences::serverGroupList();

  // iterate through all networks in the server group list in reverse order
  // to find if any new networks have been added
  for(unsigned int gIndex=serverGroupList.count();gIndex;gIndex--)
  {
    // try to find the network id in the listview
    if(!getItemById(listView,serverGroupList[gIndex-1]->id()))
      // add new server group branch to the notify listview
      addNetworkBranch(serverGroupList[gIndex-1]);
  }
  // remember current list for hasChanged()
  m_oldNotifyList=currentNotifyList();

}

// check if an item with the given id exists in the listview
QListViewItem* WatchedNicknames_Config::getItemById(QListView* listView,int id)
{
  // get the first item in the listview
  QListViewItem* lookItem=listView->firstChild();
  // look for an item with the given id
  while(lookItem)
  {
    // return item if it matches
    if(static_cast<ValueListViewItem*>(lookItem)->getValue()==id) return lookItem;
    // otherwise jump to the next group
    lookItem=lookItem->nextSibling();
  } // while

  // not found, return 0
  return 0;
}

// helper function to disable "New" button on empty listview
void WatchedNicknames_Config::checkIfEmptyListview(bool state)
{
  // only enable "New" button if there is at least one group in the list
  if(!notifyListView->childCount()) state=false;
  newButton->setEnabled(state);
}

// add new notify entry
void WatchedNicknames_Config::newNotify()
{
  // get listview object and possible first selected item
  KListView* listView=notifyListView;
  QListViewItem* item=listView->selectedItem();

  // if there was an item selected, try to find the group it belongs to,
  // so the newly created item will go into the same group, otherwise
  // just create the new entry inside of the first group
  if(item)
  {
    if(item->parent()) item=item->parent();
  }
  else
    item=listView->firstChild();

  // finally insert new item
  item=new QListViewItem(item,i18n("New"));
  // make this item the current and selected item
  item->setSelected(true);
  listView->setCurrentItem(item);
  // update network and nickname inputs
  entrySelected(item);
  // unfold group branch in case it was empty before
  listView->ensureItemVisible(item);
  nicknameInput->setFocus();
  nicknameInput->selectAll();
  // tell the config system that something has changed
  emit modified();
}

// remove a notify entry from the listview
void WatchedNicknames_Config::removeNotify()
{
  // get listview pointer and the selected item
  KListView* listView=notifyListView;
  QListViewItem* item=listView->selectedItem();

  // sanity check
  if(item)
  {
    // check which item to highlight after we deleted the chosen one
    QListViewItem* itemAfter=item->itemBelow();
    if(!itemAfter) itemAfter=item->itemAbove();
    delete(item);

    // if there was an item, highlight it
    if(itemAfter)
    {
      itemAfter->setSelected(true);
      listView->setCurrentItem(itemAfter);
      // update input widgets
      entrySelected(itemAfter);
    }
    // tell the config system that something has changed
    emit modified();
  }
}

// what to do when the user selects an entry
void WatchedNicknames_Config::entrySelected(QListViewItem* notifyEntry)
{
  // play it safe, assume disabling all widgets first
  bool enabled=false;

  // if there actually was an entry selected ...
  if(notifyEntry)
  {
    // is this entry a nickname?
    QListViewItem* group=notifyEntry->parent();
    if(group)
    {
      // all edit widgets may be enabled
      enabled=true;
      // tell all now emitted signals that we just clicked on a new item, so they should
      // not emit the modified() signal.
      newItemSelected=true;
      // copy network name and nickname to edit widgets
      nicknameInput->setText(notifyEntry->text(0));
      networkDropdown->setCurrentText(group->text(0));
      // all signals will now emit the modified() signal again
      newItemSelected=false;
    }
  }
  enableEditWidgets(enabled);
}

// enable/disable edit widgets
void WatchedNicknames_Config::enableEditWidgets(bool enabled)
{
  removeButton->setEnabled(enabled);
  networkLabel->setEnabled(enabled);
  networkDropdown->setEnabled(enabled);
  nicknameLabel->setEnabled(enabled);
  nicknameInput->setEnabled(enabled);
}

// user changed the network this nickname is on
void WatchedNicknames_Config::networkChanged(const QString& newNetwork)
{
  // get listview pointer and selected entry
  KListView* listView=notifyListView;
  QListViewItem* item=listView->selectedItem();

  // sanity check
  if(item)
  {
    // get group the nickname is presently associated to
    QListViewItem* group=item->parent();
    // did the user actually change anything?
    if(group && group->text(0)!=newNetwork)
    {
      // find the branch the new network is in
      QListViewItem* lookGroup=listView->firstChild();
      while(lookGroup && (lookGroup->text(0)!=newNetwork)) lookGroup=lookGroup->nextSibling();
      // if it was found (should never fail)
      if(lookGroup)
      {
        // deselect nickname, unlink it and relink it to the new network
        item->setSelected(false);
        group->takeItem(item);
        lookGroup->insertItem(item);
        // make the moved nickname current and selected item
        item->setSelected(true);
        listView->setCurrentItem(item);
        // unfold group branch in case it was empty before
        listView->setOpen(lookGroup,true);
        // tell the config system that something has changed
        if(!newItemSelected) emit modified();
      }
    }
  }
}

// the user edited the nickname
void WatchedNicknames_Config::nicknameChanged(const QString& newNickname)
{
  // get listview pointer and selected item
  KListView* listView=notifyListView;
  QListViewItem* item=listView->selectedItem();

  // sanity check
  if(item)
  {
    // rename item
    item->setText(0,newNickname);
    // tell the config system that something has changed
    if(!newItemSelected) emit modified();
  }
}

#include "watchednicknames_preferences.moc"
