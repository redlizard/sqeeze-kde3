/***************************************************************************
                          freeverbsetupimpl.cpp  -  description
                             -------------------
    begin                : Son Nov 2 2003
    copyright            : (C) 2003 by Robert Vogl
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

#include <iostream>
using namespace std;
 
// QT includes
#include <qslider.h>
#include <qlcdnumber.h>

// KDE includes
#include <kdebug.h>

// App specific includes
#include "freeverbsetupimpl.h"

FreeverbSetupImpl::FreeverbSetupImpl(QWidget *parent, const char *name, bool modal, KConfig *config )
 : Freeverb_Setup(parent,name,modal), m_config(config) {

  // initialize Widgets
  Init();

}
FreeverbSetupImpl::~FreeverbSetupImpl(){
}


void FreeverbSetupImpl::sliderRoomChanged(int value)
{
  LCD_room->display( 99-value );
}


void FreeverbSetupImpl::sliderDampChanged(int value)
{
  LCD_damp->display( 99-value );
}


void FreeverbSetupImpl::sliderWetChanged(int value)
{
  LCD_wet->display( 99-value );
}


void FreeverbSetupImpl::sliderDryChanged(int value)
{
  LCD_dry->display( 99-value );
}


void FreeverbSetupImpl::sliderWidthChanged(int value)
{
  LCD_width->display( 99-value );
}


void FreeverbSetupImpl::saveWasClicked()
{
  kdDebug(100200) << "FreeverbSetupImpl::saveWasClicked()" << endl;
  // Save freeverb configuration
  m_config->setGroup("Synth_FREEVERB");
  m_config->writeEntry( "room",   LCD_room->intValue() );  
  m_config->writeEntry( "damp",   LCD_damp->intValue() );
  m_config->writeEntry( "wet",    LCD_wet->intValue() );
  m_config->writeEntry( "dry",    LCD_dry->intValue() );
  m_config->writeEntry( "width",  LCD_width->intValue() );
  m_config->sync();
}

void FreeverbSetupImpl::Init()
{
  // Read freeverb configuration
  m_config->setGroup("Synth_FREEVERB");
   LCD_room->display( m_config->readNumEntry("room", 50) );
   slider_room->setValue( 99-LCD_room->intValue() );

   LCD_damp->display( m_config->readNumEntry("damp", 50) );
   slider_damp->setValue( 99-LCD_damp->intValue() );
   
   LCD_wet->display( m_config->readNumEntry("wet", 50) );
   slider_wet->setValue( 99-LCD_wet->intValue() );
   
   LCD_dry->display( m_config->readNumEntry("dry", 50) );
   slider_dry->setValue( 99-LCD_dry->intValue() );
   
   LCD_width->display( m_config->readNumEntry("width", 50) );
   slider_width->setValue( 99-LCD_width->intValue() );
}


void FreeverbSetupImpl::slotOKwasClicked()
{
  // save configuration
  saveWasClicked();
  // simply close the dialog
  this->accept();
}



#include "freeverbsetupimpl.moc"
