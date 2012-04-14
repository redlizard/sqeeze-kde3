/***************************************************************************
                          fxsetupimpl.cpp  -  description
                             -------------------
    begin                : Mo Nov 24 2003
    copyright            : (C) 2003 by voglrobe
    email                : voglrobe@saphir
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// QT includes
#include <qstringlist.h>
#include <qpushbutton.h>

// KDE includes
#include <kdebug.h>

// App specific inlcudes 
#include "fxsetupimpl.h"
// #include "freeverbsetupimpl.h"
#include "fxpluginhandler.h"

FX_SetupImpl::FX_SetupImpl(QWidget *parent, const char *name,
        KConfig *config,
        FXPluginHandler *fxpluginhandler )
 : FX_Setup(parent,name), m_config(config), m_fxpluginhandler(fxpluginhandler)
{ 
   m_fxpluginhandler->getPlugins(pluginlist);
  
    Init(pluginlist);
}


FX_SetupImpl::~FX_SetupImpl()
{
}

void FX_SetupImpl::slotAdd()
{
  // move effect from Available-List to Active-List
  int index = listBox_Available->currentItem();
  if ( index == -1 )
    return;
  listBox_Active   ->insertItem( listBox_Available->text(index), -1 );
  listBox_Available->removeItem( index );
  pushButton_removeAll->setEnabled(true);
}


void FX_SetupImpl::slotRemove()
{
  // move effect from Active-List to Available-List
  int index = listBox_Active->currentItem();
  if ( index == -1 )
    return;
  listBox_Available->insertItem( listBox_Active->text(index), -1 );
  listBox_Active   ->removeItem( index );

  if (listBox_Active->count() == 0)
    pushButton_removeAll->setEnabled(false);
}


void FX_SetupImpl::slotRemoveAll()
{
  kdDebug() << "FX_SetupImpl::slotRemoveAll()" << endl;
  
  // move all from Active-List to Available-List
  for(uint i=0; i<listBox_Active->count(); i++){
    listBox_Available->insertItem( listBox_Active->text((int)i) );
  }
  listBox_Active->clear();
  pushButton_removeAll->setEnabled(false);
}


void FX_SetupImpl::slotReload()
{
    kdDebug() << "FX_SetupImpl::slotReload()" << endl;
    
    Init(pluginlist);
}


void FX_SetupImpl::slotConfigureEffect(QListBoxItem *item)
{
  m_fxpluginhandler->showEffectGUI(item->text());
}


void FX_SetupImpl::Init(QStringList c_avail)
{
  m_config->setGroup("Effect Stack Configuration");
  QStringList conf_active = m_config->readListEntry("Activated");
  QStringList c_active;
  QStringList::Iterator sit, it;
  
  listBox_Available->clear();
  listBox_Active->clear();
  c_active.clear();
  
  pushButton_removeAll->setEnabled(false);
  
  for (sit=conf_active.begin(); sit!=conf_active.end(); ++sit){
      it = c_avail.find(*sit);
      if ( it!=c_avail.end() ){ // active plugin as per config-file in pluginlist found
          c_active.append(*sit); // append to active list
          c_avail.remove(*sit); // remove active plugin from the list of avail plugins
      }
  }
    
  if ( !c_active.isEmpty() ){
      pushButton_removeAll->setEnabled(true);
  }
  
  // Fill ListBoxes
  for(it=c_avail.begin(); it!=c_avail.end(); ++it){
    listBox_Available->insertItem( (*it), -1 );  
  }
  for(it=c_active.begin(); it!=c_active.end(); ++it){
    listBox_Active->insertItem( (*it), -1 );
  }
}


void FX_SetupImpl::slotSaveWasClicked()
{
  m_config->setGroup("Effect Stack Configuration");

  // Read ListBox Available FX
  QStringList slist;
  for(uint i=0; i<listBox_Available->count(); i++){
    slist.append( listBox_Available->text( (int)i) );  
  }
  // Write StringList
  m_config->writeEntry("Available", slist);

  // Read ListBox Activated FX
  slist.clear();
  QString t;
  for(uint i=0; i<listBox_Active->count(); i++){
    t = listBox_Active->text( (int)i );
    slist.append( t );
  }
  // Write StringList
  m_config->writeEntry("Activated", slist);
  m_config->sync();
  slist.clear();
}


#include "fxsetupimpl.moc"
