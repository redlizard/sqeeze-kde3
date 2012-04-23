/* 
 *
 * $Id: k3bdatamultisessioncombobox.cpp 619556 2007-01-03 17:38:12Z trueg $
 * Copyright (C) 2005 Sebastian Trueg <trueg@k3b.org>
 *
 * This file is part of the K3b project.
 * Copyright (C) 1998-2007 Sebastian Trueg <trueg@k3b.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#include "k3bdatamultisessioncombobox.h"

#include <klocale.h>
#include <kconfig.h>

#include <qwhatsthis.h>
#include <qtooltip.h>


static const int s_autoIndex = 0;
static const int s_noneIndex = 1;
static const int s_startIndex = 2;
static const int s_continueIndex = 3;
static const int s_finishIndex = 4;


K3bDataMultiSessionCombobox::K3bDataMultiSessionCombobox( QWidget* parent, const char* name )
  : QComboBox( parent, name ),
    m_forceNoMultiSession(false)
{
  init( false );

  QToolTip::add( this, i18n("Select the Multisession Mode for the project.") );
  QWhatsThis::add( this, i18n("<p><b>Multisession Mode</b>"
			      "<p><b>Auto</b><br>"
			      "Let K3b decide which mode to use. The decision will be based "
			      "on the size of the project (does it fill the whole media) and "
			      "the state of the inserted media (appendable or not)."
			      "<p><b>No Multisession</b><br>"
			      "Create a single-session CD or DVD and close the disk."
			      "<p><b>Start Multisession</b><br>"
			      "Start a multisession CD or DVD, not closing the disk to "
			      "allow further sessions to be apppended."
			      "<p><b>Continue Multisession</b><br>"
			      "Continue an appendable data CD (as for example created in "
			      "<em>Start Multisession</em> mode) and add another session "
			      "without closing the disk to "
			      "allow further sessions to be apppended."
			      "<p><b>Finish Multisession</b><br>"
			      "Continue an appendable data CD (as for example created in "
			      "<em>Start Multisession</em> mode), add another session, "
			      "and close the disk."
			      "<p><em>In the case of DVD+RW and DVD-RW restricted overwrite media "
			      "K3b will not actually create multiple sessions but grow the "
			      "file system to include the new data.</em>") );
}


K3bDataMultiSessionCombobox::~K3bDataMultiSessionCombobox()
{
}


void K3bDataMultiSessionCombobox::init( bool force )
{
  m_forceNoMultiSession = force;

  clear();

  insertItem( i18n("Auto"), s_autoIndex );
  insertItem( i18n("No Multisession"), s_noneIndex );
  if( !m_forceNoMultiSession ) {
    insertItem( i18n("Start Multisession"), s_startIndex );
    insertItem( i18n("Continue Multisession "), s_continueIndex );
    insertItem( i18n("Finish Multisession "), s_finishIndex );
  }
}


K3bDataDoc::MultiSessionMode K3bDataMultiSessionCombobox::multiSessionMode() const
{
  switch( currentItem() ) {
  case s_noneIndex:
    return K3bDataDoc::NONE;
  case s_startIndex:
    return K3bDataDoc::START;
  case s_continueIndex:
    return K3bDataDoc::CONTINUE;
  case s_finishIndex:
    return K3bDataDoc::FINISH;
  default:
    return K3bDataDoc::AUTO;
  }
}


void K3bDataMultiSessionCombobox::saveConfig( KConfigBase* c )
{
  QString s;
  switch( currentItem() ) {
  case s_autoIndex:
    s = "auto";
    break;
  case s_noneIndex:
    s = "none";
    break;
  case s_startIndex:
    s = "start";
    break;
  case s_continueIndex:
    s = "continue";
    break;
  case s_finishIndex:
    s = "finish";
    break;
  }

  c->writeEntry( "multisession mode", s );
}


void K3bDataMultiSessionCombobox::loadConfig( KConfigBase* c )
{
  QString s = c->readEntry( "multisession mode" );
  if( s == "none" )
    setMultiSessionMode( K3bDataDoc::NONE );
  else if( s == "start" )
    setMultiSessionMode( K3bDataDoc::START );
  else if( s == "continue" )
    setMultiSessionMode( K3bDataDoc::CONTINUE );
  else if( s == "finish" )
    setMultiSessionMode( K3bDataDoc::FINISH );
  else
    setMultiSessionMode( K3bDataDoc::AUTO );
}


void K3bDataMultiSessionCombobox::setMultiSessionMode( K3bDataDoc::MultiSessionMode m )
{
  switch( m ) {
  case K3bDataDoc::AUTO:
    setCurrentItem( s_autoIndex );
    break;
  case K3bDataDoc::NONE:
    setCurrentItem( s_noneIndex );
    break;
  case K3bDataDoc::START:
    if( !m_forceNoMultiSession )
      setCurrentItem( s_startIndex );
    break;
  case K3bDataDoc::CONTINUE:
    if( !m_forceNoMultiSession )
      setCurrentItem( s_continueIndex );
    break;
  case K3bDataDoc::FINISH:
    if( !m_forceNoMultiSession )
      setCurrentItem( s_finishIndex );
    break;
  }
}


void K3bDataMultiSessionCombobox::setForceNoMultisession( bool f )
{
  if( f != m_forceNoMultiSession ) {
    K3bDataDoc::MultiSessionMode m = multiSessionMode();
    init( f );
    setMultiSessionMode( m );
  }
}

#include "k3bdatamultisessioncombobox.moc"
