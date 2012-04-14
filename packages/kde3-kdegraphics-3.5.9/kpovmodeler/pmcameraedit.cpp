/*
**************************************************************************
                                 description
                             --------------------
    copyright            : (C) 2000-2001 by Andreas Zehender
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


#include "pmcameraedit.h"
#include "pmcamera.h"
#include "pmvectoredit.h"
#include "pmlineedits.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qcheckbox.h>

#include <klocale.h>
#include <kmessagebox.h>


PMCameraEdit::PMCameraEdit( QWidget* parent, const char* name )
      : Base( parent, name )
{
   m_pDisplayedObject = 0;
}

void PMCameraEdit::createTopWidgets( )
{
   Base::createTopWidgets( );

   QGridLayout* layout;
   QLabel* label;

   m_pCameraType = new QComboBox( false, this );
   m_pCameraType->insertItem( i18n( "Perspective" ) );
   m_pCameraType->insertItem( i18n( "Orthographic" ) );
   m_pCameraType->insertItem( i18n( "Fish Eye" ) );
   m_pCameraType->insertItem( i18n( "Ultra Wide Angle" ) );
   m_pCameraType->insertItem( i18n( "Omnimax" ) );
   m_pCameraType->insertItem( i18n( "Panoramic" ) );
   m_pCameraType->insertItem( i18n( "Cylinder" ) );

   m_pCylinderType = new QComboBox( false, this );
   m_pCylinderType->insertItem( i18n( "1: Vertical, Fixed Viewpoint" ) );
   m_pCylinderType->insertItem( i18n( "2: Horizontal, Fixed Viewpoint" ) );
   m_pCylinderType->insertItem( i18n( "3: Vertical, Variable Viewpoint" ) );
   m_pCylinderType->insertItem( i18n( "4: Horizontal, Variable Viewpoint" ) );

   m_pLocation = new PMVectorEdit( "x", "y", "z", this );
   m_pSky = new PMVectorEdit( "x", "y", "z", this );
   m_pDirection = new PMVectorEdit( "x", "y", "z", this );
   m_pRight = new PMVectorEdit( "x", "y", "z", this );
   m_pUp = new PMVectorEdit( "x", "y", "z", this );
   m_pLookAt = new PMVectorEdit( "x", "y", "z", this );
   m_pAngle = new PMFloatEdit( this );
   m_pAngle->setValidation( true, 0.0, true, 360.0 );

   QHBoxLayout* hl = new QHBoxLayout( topLayout( ) );
   hl->addWidget( new QLabel( i18n( "Camera type:" ), this ) );
   hl->addWidget( m_pCameraType );
   hl = new QHBoxLayout( topLayout( ) );
   m_pCylinderTypeLabel = new QLabel( i18n( "Cylinder type:" ), this );
   hl->addWidget( m_pCylinderTypeLabel );
   hl->addWidget( m_pCylinderType );

   layout = new QGridLayout( topLayout( ), 7, 2 );
   layout->addWidget( new QLabel( i18n( "Location:" ), this ), 0, 0 );
   layout->addWidget( m_pLocation, 0, 1 );

   layout->addWidget( new QLabel( i18n( "Sky:" ), this ), 1, 0 );
   layout->addWidget( m_pSky, 1, 1 );

   layout->addWidget( new QLabel( i18n( "Direction:" ), this ), 2, 0 );
   layout->addWidget( m_pDirection, 2, 1 );

   layout->addWidget( new QLabel( i18n( "Right:" ), this ), 3, 0 );
   layout->addWidget( m_pRight, 3, 1 );

   layout->addWidget( new QLabel( i18n( "Up:" ), this ), 4, 0 );
   layout->addWidget( m_pUp, 4, 1 );

   layout->addWidget( new QLabel( i18n( "Look at:" ), this ), 5, 0 );
   layout->addWidget( m_pLookAt, 5, 1 );

   m_pEnableAngle = new QCheckBox( i18n( "Angle:" ), this );
   layout->addWidget( m_pEnableAngle, 6, 0 );
   layout->addWidget( m_pAngle, 6, 1, AlignLeft );

   m_pFocalBlur = new QCheckBox( i18n( "Focal blur" ), this );
   topLayout( )->addWidget( m_pFocalBlur );

   m_pAperture = new PMFloatEdit( this );
   m_pAperture->setValidation( true, 0, false, 0 );
   m_focalWidgets.append( m_pAperture );
   m_pBlurSamples = new PMIntEdit( this );
   m_pBlurSamples->setValidation( true, 0, false, 0 );
   m_focalWidgets.append( m_pBlurSamples );
   m_pFocalPoint = new PMVectorEdit( "x", "y", "z", this );
   m_focalWidgets.append( m_pFocalPoint );
   m_pConfidence = new PMFloatEdit( this );
   m_pConfidence->setValidation( true, 0, true, 1 );
   m_focalWidgets.append( m_pConfidence );
   m_pVariance = new PMFloatEdit( this );
   m_pVariance->setValidation( true, 0, false, 0 );
   m_focalWidgets.append( m_pVariance );

   layout = new QGridLayout( topLayout( ), 5, 2 );
   label = new QLabel( i18n( "Aperture:" ), this );
   m_focalWidgets.append( label );
   layout->addWidget( label, 0, 0 );
   layout->addWidget( m_pAperture, 0, 1 );
   label = new QLabel( i18n( "Blur samples:" ), this );
   m_focalWidgets.append( label );
   layout->addWidget( label, 1, 0 );
   layout->addWidget( m_pBlurSamples, 1, 1 );
   label = new QLabel( i18n( "Focal point:" ), this );
   m_focalWidgets.append( label );
   layout->addWidget( label, 2, 0 );
   layout->addWidget( m_pFocalPoint, 2, 1 );
   label = new QLabel( i18n( "Confidence:" ), this );
   m_focalWidgets.append( label );
   layout->addWidget( label, 3, 0 );
   layout->addWidget( m_pConfidence, 3, 1 );
   label = new QLabel( i18n( "Variance:" ), this );
   m_focalWidgets.append( label );
   layout->addWidget( label, 4, 0 );
   layout->addWidget( m_pVariance, 4, 1 );

   m_pExport = new QCheckBox( i18n( "Export to renderer" ), this );
   topLayout( )->addWidget( m_pExport );

   connect( m_pLocation, SIGNAL( dataChanged( ) ), SIGNAL( dataChanged( ) ) );
   connect( m_pDirection, SIGNAL( dataChanged( ) ), SLOT( slotDirectionChanged( ) ) );
   connect( m_pRight, SIGNAL( dataChanged( ) ), SIGNAL( dataChanged( ) ) );
   connect( m_pRight, SIGNAL( dataChanged( ) ), SLOT( slotRightChanged( ) ) );
   connect( m_pUp, SIGNAL( dataChanged( ) ), SIGNAL( dataChanged( ) ) );
   connect( m_pSky, SIGNAL( dataChanged( ) ), SIGNAL( dataChanged( ) ) );
   connect( m_pLookAt, SIGNAL( dataChanged( ) ), SIGNAL( dataChanged( ) ) );
   connect( m_pEnableAngle, SIGNAL( toggled( bool ) ),
            SLOT( slotAngleToggled( bool ) ) );
   connect( m_pAngle, SIGNAL( dataChanged( ) ), SLOT( slotAngleChanged( ) ) );
   connect( m_pCameraType, SIGNAL( activated( int ) ),
            SLOT( slotCameraTypeActivated( int ) ) );
   connect( m_pCylinderType, SIGNAL( activated( int ) ),
            SLOT( slotCylinderTypeActivated( int ) ) );

   connect( m_pFocalBlur, SIGNAL( toggled( bool ) ),
            SLOT( slotFocalBlurToggled( bool ) ) );
   connect( m_pAperture, SIGNAL( dataChanged( ) ), SIGNAL( dataChanged( ) ) );
   connect( m_pBlurSamples, SIGNAL( dataChanged( ) ), SIGNAL( dataChanged( ) ) );
   connect( m_pFocalPoint, SIGNAL( dataChanged( ) ), SIGNAL( dataChanged( ) ) );
   connect( m_pVariance, SIGNAL( dataChanged( ) ), SIGNAL( dataChanged( ) ) );
   connect( m_pConfidence, SIGNAL( dataChanged( ) ), SIGNAL( dataChanged( ) ) );
   connect( m_pExport, SIGNAL( clicked( ) ), SIGNAL( dataChanged( ) ) );
}

void PMCameraEdit::displayObject( PMObject* o )
{
   if( o->isA( "Camera" ) )
   {
      bool readOnly = o->isReadOnly( );
      m_pDisplayedObject = ( PMCamera* ) o;

      m_pCameraType->setCurrentItem( m_pDisplayedObject->cameraType( ) );
      m_pCameraType->setEnabled( !readOnly );
      slotCameraTypeActivated( m_pDisplayedObject->cameraType( ) );
      m_pCylinderType->setCurrentItem( m_pDisplayedObject->cylinderType( ) - 1 );
      m_pCylinderType->setEnabled( !readOnly );
      m_pLocation->setVector( m_pDisplayedObject->location( ) );
      m_pLocation->setReadOnly( readOnly );
      m_pSky->setVector( m_pDisplayedObject->sky( ) );
      m_pSky->setReadOnly( readOnly );
      m_pDirection->setVector( m_pDisplayedObject->direction( ) );
      m_pDirection->setReadOnly( readOnly );
      m_pRight->setVector( m_pDisplayedObject->right( ) );
      m_pRight->setReadOnly( readOnly );
      m_pUp->setVector( m_pDisplayedObject->up( ) );
      m_pUp->setReadOnly( readOnly );
      m_pLookAt->setVector( m_pDisplayedObject->lookAt( ) );
      m_pLookAt->setReadOnly( readOnly );

      m_pEnableAngle->setChecked( m_pDisplayedObject->isAngleEnabled( ) );
      m_pEnableAngle->setEnabled( !readOnly );
      m_pAngle->setValue( m_pDisplayedObject->angle( ) );
      m_pAngle->setReadOnly( readOnly );
      slotAngleToggled( m_pDisplayedObject->isAngleEnabled( ) );
      slotRightChanged( );
      m_pFocalBlur->setChecked( m_pDisplayedObject->isFocalBlurEnabled( ) );
      slotFocalBlurToggled( m_pDisplayedObject->isFocalBlurEnabled( ) );
      m_pFocalBlur->setEnabled( !readOnly );

      m_pAperture->setValue( m_pDisplayedObject->aperture( ) );
      m_pAperture->setReadOnly( readOnly );
      m_pBlurSamples->setValue( m_pDisplayedObject->blurSamples( ) );
      m_pBlurSamples->setReadOnly( readOnly );
      m_pFocalPoint->setVector( m_pDisplayedObject->focalPoint( ) );
      m_pFocalPoint->setReadOnly( readOnly );
      m_pConfidence->setValue( m_pDisplayedObject->confidence( ) );
      m_pConfidence->setReadOnly( readOnly );
      m_pVariance->setValue( m_pDisplayedObject->variance( ) );
      m_pVariance->setReadOnly( readOnly );

      m_pExport->setChecked( m_pDisplayedObject->exportPovray( ) );
      m_pExport->setEnabled( !readOnly );

      Base::displayObject( o );
   }
   else
      kdError( PMArea ) << "PMCameraEdit: Can't display object\n";
}

void PMCameraEdit::saveContents( )
{
   int index;
   if( m_pDisplayedObject )
   {
      Base::saveContents( );
      index = m_pCameraType->currentItem( );
      m_pDisplayedObject->setCameraType( ( PMCamera::CameraType ) index );
      if( index == 6 )
         m_pDisplayedObject->setCylinderType( m_pCylinderType->currentItem( ) + 1 );
      m_pDisplayedObject->setLocation( m_pLocation->vector( ) );
      m_pDisplayedObject->setDirection( m_pDirection->vector( ) );
      m_pDisplayedObject->setRight( m_pRight->vector( ) );
      m_pDisplayedObject->setUp( m_pUp->vector( ) );
      m_pDisplayedObject->setSky( m_pSky->vector( ) );
      m_pDisplayedObject->setLookAt( m_pLookAt->vector( ) );
      m_pDisplayedObject->enableAngle( m_pEnableAngle->isChecked( ) );
      if( m_pEnableAngle->isChecked( ) )
         m_pDisplayedObject->setAngle( m_pAngle->value( ) );

      m_pDisplayedObject->enableFocalBlur( m_pFocalBlur->isChecked( ) );
      if( m_pFocalBlur->isChecked( ) )
      {
         m_pDisplayedObject->setAperture( m_pAperture->value( ) );
         m_pDisplayedObject->setBlurSamples( m_pBlurSamples->value( ) );
         m_pDisplayedObject->setFocalPoint( m_pFocalPoint->vector( ) );
         m_pDisplayedObject->setConfidence( m_pConfidence->value( ) );
         m_pDisplayedObject->setVariance( m_pVariance->value( ) );
      }
      m_pDisplayedObject->setExportPovray( m_pExport->isChecked( ) );
   }
}

bool PMCameraEdit::isDataValid( )
{
   if( !m_pLocation->isDataValid( ) )
      return false;
   if( !m_pSky->isDataValid( ) )
      return false;
   if( approxZero( m_pSky->vector( ).abs( ) ) )
   {
      KMessageBox::error( this, i18n( "The sky vector may not be a null vector." ),
                          i18n( "Error" ) );
      m_pSky->setFocus( );
      return false;
   }
   if( !m_pDirection->isDataValid( ) )
      return false;
   if( approxZero( m_pDirection->vector( ).abs( ) ) )
   {
      KMessageBox::error( this, i18n( "The direction vector may not be a null vector." ),
                          i18n( "Error" ) );
      m_pDirection->setFocus( );
      return false;
   }
   if( !m_pRight->isDataValid( ) )
      return false;
   if( approxZero( m_pRight->vector( ).abs( ) ) )
   {
      KMessageBox::error( this, i18n( "The right vector may not be a null vector." ),
                          i18n( "Error" ) );
      m_pRight->setFocus( );
      return false;
   }
   if( !m_pUp->isDataValid( ) )
      return false;
   if( approxZero( m_pUp->vector( ).abs( ) ) )
   {
      KMessageBox::error( this, i18n( "The up vector may not be a null vector." ),
                          i18n( "Error" ) );
      m_pDirection->setFocus( );
      return false;
   }
   if( !m_pLookAt->isDataValid( ) )
      return false;
   if( m_pEnableAngle->isChecked( ) && !m_pAngle->isDataValid( ) )
      return false;

   if( ( m_pCameraType->currentItem( ) == 0 ) && m_pEnableAngle->isChecked( ) )
   {
      double angle = m_pAngle->value( );
      if( angle >= 180.0 )
      {
         KMessageBox::error( this, i18n( "Angle has to be smaller than 180"
                                         " degrees for that camera type." ),
                             i18n( "Error" ) );
         m_pAngle->setFocus( );
         return false;
      }
   }

   if( m_pFocalBlur->isChecked( ) )
   {
      if( !m_pAperture->isDataValid( ) )
         return false;
      if( !m_pBlurSamples->isDataValid( ) )
         return false;
      if( !m_pFocalPoint->isDataValid( ) )
         return false;
      if( !m_pConfidence->isDataValid( ) )
         return false;
      if( !m_pVariance->isDataValid( ) )
         return false;
   }

   return Base::isDataValid( );
}

void PMCameraEdit::slotCameraTypeActivated( int index )
{
   if( index == 6 )
   {
      m_pCylinderType->show( );
      m_pCylinderTypeLabel->show( );
   }
   else
   {
      m_pCylinderType->hide( );
      m_pCylinderTypeLabel->hide( );
   }

   if( ( index == 1 ) || ( index == 4 ) || ( index == 5 ) )
   {
      m_pAngle->hide( );
      m_pEnableAngle->hide( );
   }
   else
   {
      m_pAngle->show( );
      m_pEnableAngle->show( );
   }

   if( index == 0 )
      m_pFocalBlur->show( );
   else
      m_pFocalBlur->hide( );
   enableFocalWidgets( m_pFocalBlur->isChecked( ) && ( index == 0 ) );
   emit sizeChanged( );
   emit dataChanged( );
}

void PMCameraEdit::slotCylinderTypeActivated( int )
{
   emit dataChanged( );
}

void PMCameraEdit::slotFocalBlurToggled( bool on )
{
   enableFocalWidgets( on && ( m_pCameraType->currentItem( ) == 0 ) );
   emit dataChanged( );
}

void PMCameraEdit::slotAngleToggled( bool on )
{
   m_pAngle->setEnabled( on );
   emit dataChanged( );
}

void PMCameraEdit::slotAngleChanged( )
{
   /*
   if( ( m_pCameraType->currentItem( ) == 0 ) && m_pEnableAngle->isChecked( ) )
   {
      // Only change direction's value in perspective and with an enabled angle
      disconnect( m_pDirection, SIGNAL( dataChanged( ) ), 0, 0 );
      m_pDirection->setVector( 0.5 * m_pRight->vector( ) / tan( 2 * deg2Rad( m_pAngle->value( ) ) ) );
      connect( m_pDirection, SIGNAL( dataChanged( ) ), SLOT( slotDirectionChanged( ) ) );
   }
   */
   emit dataChanged( );
}

void PMCameraEdit::slotDirectionChanged( )
{
   calculateCameraAngle( );
   emit dataChanged( );
}

void PMCameraEdit::slotRightChanged( )
{
   calculateCameraAngle( );
   emit dataChanged( );
}

void PMCameraEdit::calculateCameraAngle( )
{
   if( ( m_pCameraType->currentItem( ) == 0 ) && !m_pEnableAngle->isChecked( ) )
   {
      // Only change angle's value in perspective and with a disabled angle
      double rl = m_pRight->vector( ).abs( );
      double dl = m_pDirection->vector( ).abs( );
      if( ( rl > 0 ) && ( dl > 0 ) )
      {
         bool sb = m_pAngle->signalsBlocked( );
         m_pAngle->blockSignals( true );
         m_pAngle->setValue( rad2Deg( 2 * atan2( 0.5 * rl, dl ) ) );
         m_pAngle->blockSignals( sb );
      }
   }
}

void PMCameraEdit::enableFocalWidgets( bool on )
{
   QPtrListIterator<QWidget> it( m_focalWidgets );

   for( ; it.current( ); ++it )
   {
      if( on )
         it.current( )->show( );
      else
         it.current( )->hide( );
   }
   emit sizeChanged( );
}

#include "pmcameraedit.moc"
