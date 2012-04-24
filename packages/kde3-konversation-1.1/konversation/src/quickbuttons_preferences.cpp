/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

/*
  Copyright (C) 2006 Dario Abatianni <eisfuchs@tigress.com>
  Copyright (C) 2006 John Tapsell <johnflux@gmail.com>
*/

#include "quickbuttons_preferences.h"
#include "config/preferences.h"

#include <qlabel.h>
#include <qpushbutton.h>
#include <qheader.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kconfig.h>
#include <klocale.h>
#include <klineedit.h>
#include <klistview.h>


QuickButtons_Config::QuickButtons_Config(QWidget* parent, const char* name)
 : QuickButtons_ConfigUI(parent, name)
{
  // reset flag to defined state (used to block signals when just selecting a new item)
  m_newItemSelected=false;

  // populate listview
  loadSettings();

  // make items react to drag & drop
  buttonListView->setSorting(-1,false);
  buttonListView->header()->setMovingEnabled(false);

  connect(buttonListView,SIGNAL (selectionChanged(QListViewItem*)),this,SLOT (entrySelected(QListViewItem*)) );
  connect(buttonListView,SIGNAL (clicked(QListViewItem*)),this,SLOT (entrySelected(QListViewItem*)) );
  connect(buttonListView,SIGNAL (moved()),this,SIGNAL (modified()) );

  connect(nameInput,SIGNAL (textChanged(const QString&)),this,SLOT (nameChanged(const QString&)) );
  connect(actionInput,SIGNAL (textChanged(const QString&)),this,SLOT (actionChanged(const QString&)) );

  connect(newButton,SIGNAL (clicked()),this,SLOT (addEntry()));
  connect(removeButton,SIGNAL (clicked()),this,SLOT (removeEntry()));
}

QuickButtons_Config::~QuickButtons_Config()
{
}

void QuickButtons_Config::loadSettings()
{
  setButtonsListView(Preferences::quickButtonList());

  // remember button list for hasChanged()
  m_oldButtonList=Preferences::quickButtonList();
}

// fill listview with button definitions
void QuickButtons_Config::setButtonsListView(const QStringList &buttonList)
{
  // clear listView
  buttonListView->clear();
  // go through the list
  for(unsigned int index=buttonList.count();index!=0;index--)
  {
    // get button definition
    QString definition=buttonList[index-1];
    // cut definition apart in name and action, and create a new listview item
    new KListViewItem(buttonListView,definition.section(',',0,0),definition.section(',',1));
  } // for
  buttonListView->setSelected(buttonListView->firstChild(), true);
}

// save quick buttons to configuration
void QuickButtons_Config::saveSettings()
{
  // get configuration object
  KConfig* config=kapp->config();

  // delete all buttons
  config->deleteGroup("Button List");
  // create new empty button group
  config->setGroup("Button List");

  // create empty list
  QStringList newList=currentButtonList();

  // check if there are any quick buttons in the list view
  if(newList.count())
  {
    // go through all buttons and save them into the configuration
    for(unsigned int index=0;index<newList.count();index++)
    {
      // write the current button's name and definition
      config->writeEntry(QString("Button%1").arg(index),newList[index]);
    } // for
  }
  // if there were no buttons at all, write a dummy entry to prevent KConfigXT from "optimizing"
  // the group out, which would in turn make konvi restore the default buttons
  else
    config->writeEntry("Empty List",QString());

  // set internal button list
  Preferences::setQuickButtonList(newList);

  // remember button list for hasChanged()
  m_oldButtonList=newList;
}

void QuickButtons_Config::restorePageToDefaults()
{
  setButtonsListView(Preferences::defaultQuickButtonList());
}

QStringList QuickButtons_Config::currentButtonList()
{
  // get first item of the button listview
  QListViewItem* item=buttonListView->firstChild();
  // create empty list
  QStringList newList;

  // go through all items and save them into the configuration
  while(item)
  {
    // remember button in internal list
    newList.append(item->text(0)+','+item->text(1));
    // get next item in the listview
    item=item->itemBelow();
  } // while

  // return list
  return newList;
}

bool QuickButtons_Config::hasChanged()
{
  return(m_oldButtonList!=currentButtonList());
}

// slots

// what to do when the user selects an item
void QuickButtons_Config::entrySelected(QListViewItem* quickButtonEntry)
{
  // play it safe, assume disabling all widgets first
  bool enabled=false;

  // check if there really was an item selected
  if(quickButtonEntry)
  {
    // remember to enable the editing widgets
    enabled=true;

    // tell the editing widgets not to emit modified() on signals now
    m_newItemSelected=true;
    // update editing widget contents
    nameInput->setText(quickButtonEntry->text(0));
    actionInput->setText(quickButtonEntry->text(1));
    // re-enable modified() signal on text changes in edit widgets
    m_newItemSelected=false;
  }
  // enable or disable editing widgets
  removeButton->setEnabled(enabled);
  nameLabel->setEnabled(enabled);
  nameInput->setEnabled(enabled);
  actionLabel->setEnabled(enabled);
  actionInput->setEnabled(enabled);
}

// what to do when the user change the name of a quick button
void QuickButtons_Config::nameChanged(const QString& newName)
{
  // get possible first selected item
  QListViewItem* item=buttonListView->selectedItem();

  // sanity check
  if(item)
  {
    // rename item
    item->setText(0,newName);
    // tell the config system that something has changed
    if(!m_newItemSelected) emit modified();
  }
}

// what to do when the user change the action definition of a quick button
void QuickButtons_Config::actionChanged(const QString& newAction)
{
  // get possible first selected item
  QListViewItem* item=buttonListView->selectedItem();

  // sanity check
  if(item)
  {
    // rename item
    item->setText(1,newAction);
    // tell the config system that something has changed
    if(!m_newItemSelected) emit modified();
  }
}

// add button pressed
void QuickButtons_Config::addEntry()
{
  // add new item at the bottom of list view
  KListViewItem* newItem=new KListViewItem(buttonListView,buttonListView->lastChild(),i18n("New"),QString());
  // if successful ...
  if(newItem)
  {
    // select new item and make it the current one
    buttonListView->setSelected(newItem,true);
    buttonListView->setCurrentItem(newItem);
    // set input focus on item name edit
    nameInput->setFocus();
    // select all text to make overwriting easier
    nameInput->selectAll();
    // tell the config system that something has changed
    emit modified();
  }
}

// remove button pressed
void QuickButtons_Config::removeEntry()
{
  // get possible first selected item
  QListViewItem* item=buttonListView->selectedItem();

  // sanity check
  if(item)
  {
    // get item below the current one
    QListViewItem* nextItem=item->itemBelow();
    // if there was none, get the one above
    if(!nextItem) nextItem=item->itemAbove();

    // remove the item from the list
    delete item;

    // check if we found the next item
    if(nextItem)
    {
      // select the item and make it the current ite,
      buttonListView->setSelected(nextItem,true);
      buttonListView->setCurrentItem(nextItem);
    }
    else
    {
      // no next item found, this means the list is empty
      entrySelected(0);
    }
    // tell the config system that somethig has changed
    emit modified();
  }
}

#include "quickbuttons_preferences.moc"
