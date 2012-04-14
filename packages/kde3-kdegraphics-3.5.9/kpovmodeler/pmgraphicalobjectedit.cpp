/*
**************************************************************************
                                 description
                             --------------------
    copyright            : (C) 2000-2002 by Andreas Zehender
    email                : zehender@kde.org
**************************************************************************

**************************************************************************
*                                                                        *
*  This program is free software; you can redistribute it and/or modify  *
*  it under the terms of the GNU General Public License as published by  *
*  the Free Software Foundation; either version 2 of the License, or     *
*  (at your option) any later version.                                   *
*                                                                        *
**************************************************************************/


#include "pmgraphicalobjectedit.h"
#include "pmgraphicalobject.h"

#include <qcheckbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <klocale.h>

const int c_minValue = -1000;
const int c_maxValue = 1000;

PMGraphicalObjectEdit::PMGraphicalObjectEdit( QWidget* parent, const char* name )
      : Base( parent, name )
{
   m_pDisplayedObject = 0;
}

void PMGraphicalObjectEdit::createBottomWidgets( )
{
   QGridLayout* gl = new QGridLayout( topLayout( ), 2, 2 );
   m_pNoShadowButton = new QCheckBox( i18n( "No shadow" ), this );
   gl->addWidget( m_pNoShadowButton, 0, 0 );
   m_pNoImageButton = new QCheckBox( i18n( "No image" ), this );
   gl->addWidget( m_pNoImageButton, 0, 1 );
   m_pNoReflectionButton = new QCheckBox( i18n( "No reflection" ), this );
   gl->addWidget( m_pNoReflectionButton, 1, 0 );
   m_pDoubleIlluminateButton = new QCheckBox( i18n( "Double illuminate" ), this );
   gl->addWidget( m_pDoubleIlluminateButton, 1, 1 );
   m_pExport = new QCheckBox( i18n( "Export to renderer" ), this );
   topLayout( )->addWidget( m_pExport );

   QHBoxLayout* hl = new QHBoxLayout( topLayout( ) );
   hl->addWidget( new QLabel( i18n( "Visibility level: " ), this ) );
   m_pVisibilityLevel = new QSpinBox( c_minValue, c_maxValue, 1, this );
   hl->addWidget( m_pVisibilityLevel );
   m_pResultingVisibility = new QLabel( QString( "(  )" ), this );
   hl->addWidget( m_pResultingVisibility );
   hl->addSpacing( 10 );
   m_pRelativeVisibility = new QCheckBox( i18n( "Relative" ), this );
   hl->addWidget( m_pRelativeVisibility );
   hl->addStretch( 1 );

   connect( m_pNoShadowButton, SIGNAL( clicked( ) ), SIGNAL( dataChanged( ) ) );
   connect( m_pNoImageButton, SIGNAL( clicked( ) ), SIGNAL( dataChanged( ) ) );
   connect( m_pNoReflectionButton, SIGNAL( clicked( ) ), SIGNAL( dataChanged( ) ) );
   connect( m_pDoubleIlluminateButton, SIGNAL( clicked( ) ), SIGNAL( dataChanged( ) ) );
   connect( m_pRelativeVisibility, SIGNAL( clicked( ) ),
            SLOT( slotRelativeChanged( ) ) );
   connect( m_pVisibilityLevel, SIGNAL( valueChanged( int ) ),
            SLOT( slotLevelChanged( int ) ) );
   connect( m_pExport, SIGNAL( clicked( ) ), SIGNAL( dataChanged( ) ) );


   Base::createBottomWidgets( );
}

void PMGraphicalObjectEdit::displayObject( PMObject* o )
{
   if( o->isA( "GraphicalObject" ) )
   {
      bool readOnly = o->isReadOnly( );

      m_pDisplayedObject = ( PMGraphicalObject* ) o;
      m_pNoShadowButton->setChecked( m_pDisplayedObject->noShadow( ) );
      m_pNoShadowButton->setEnabled( !readOnly );
      m_pNoImageButton->setChecked( m_pDisplayedObject->noImage( ) );
      m_pNoImageButton->setEnabled( !readOnly );
      m_pNoReflectionButton->setChecked( m_pDisplayedObject->noReflection( ) );
      m_pNoReflectionButton->setEnabled( !readOnly );
      m_pDoubleIlluminateButton->setChecked( m_pDisplayedObject->doubleIlluminate( ) );
      m_pDoubleIlluminateButton->setEnabled( !readOnly );
      bool sb = m_pVisibilityLevel->signalsBlocked( );
      m_pVisibilityLevel->blockSignals( true );
      m_pVisibilityLevel->setValue( m_pDisplayedObject->visibilityLevel( ) );
      m_pVisibilityLevel->setEnabled( !readOnly );
      m_pVisibilityLevel->blockSignals( sb );
      sb = m_pRelativeVisibility->signalsBlocked( );
      m_pRelativeVisibility->blockSignals( true );
      m_pRelativeVisibility->setChecked( m_pDisplayedObject->isVisibilityLevelRelative( ) );
      m_pRelativeVisibility->setEnabled( !readOnly );
      m_pRelativeVisibility->blockSignals( sb );
      m_pExport->setChecked( m_pDisplayedObject->exportPovray( ) );
      m_pExport->setEnabled( !readOnly );

      recalculateResultingVisibility( );

      Base::displayObject( o );
   }
   else
      kdError( PMArea ) << "PMGraphicalObjectEdit: Can't display object\n";
}

void PMGraphicalObjectEdit::saveContents( )
{
   if( m_pDisplayedObject )
   {
      m_pDisplayedObject->setNoShadow( m_pNoShadowButton->isChecked( ) );
      m_pDisplayedObject->setNoImage( m_pNoImageButton->isChecked( ) );
      m_pDisplayedObject->setNoReflection( m_pNoReflectionButton->isChecked( ) );
      m_pDisplayedObject->setDoubleIlluminate( m_pDoubleIlluminateButton->isChecked( ) );
      m_pDisplayedObject->setVisibilityLevel( m_pVisibilityLevel->value( ) );
      m_pDisplayedObject->setVisibilityLevelRelative( m_pRelativeVisibility->isChecked( ) );
      m_pDisplayedObject->setExportPovray( m_pExport->isChecked( ) );
      Base::saveContents( );
   }
}

bool PMGraphicalObjectEdit::isDataValid( )
{
   return Base::isDataValid( );
}

void PMGraphicalObjectEdit::slotRelativeChanged(  )
{
   recalculateResultingVisibility( );
   emit dataChanged( );
}

void PMGraphicalObjectEdit::slotLevelChanged( int )
{
   recalculateResultingVisibility( );
   emit dataChanged( );
}

void PMGraphicalObjectEdit::recalculateResultingVisibility( )
{
   PMObject* o = m_pDisplayedObject->parent( );
   PMGraphicalObject* go = 0;
   int level = 0;
   bool absoluteFound = false;

   level = m_pVisibilityLevel->value( );
   if( !m_pRelativeVisibility->isChecked( ) )
      absoluteFound = true;

   for( ; o && !absoluteFound; o = o->parent( ) )
   {
      if( o->isA( "GraphicalObject" ) )
      {
         go = ( PMGraphicalObject* ) o;
         level += go->visibilityLevel( );
         if( !go->isVisibilityLevelRelative( ) )
            absoluteFound = true;
      }
   }
   m_pResultingVisibility->setText( QString( "(%1)" ).arg( level ) );
}

#include "pmgraphicalobjectedit.moc"
