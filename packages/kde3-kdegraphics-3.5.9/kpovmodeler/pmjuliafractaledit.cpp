/*
**************************************************************************
                                 description
                             --------------------
    copyright            : (C) 2002 by Andreas Zehender
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


#include "pmjuliafractaledit.h"
#include "pmjuliafractal.h"
#include "pmvectoredit.h"
#include "pmlineedits.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <klocale.h>
#include <kmessagebox.h>

PMJuliaFractalEdit::PMJuliaFractalEdit( QWidget* parent, const char* name )
      : Base( parent, name )
{
   m_pDisplayedObject = 0;
}

void PMJuliaFractalEdit::createTopWidgets( )
{
   Base::createTopWidgets( );
   
   QHBoxLayout* hl;
   QGridLayout* gl;

   topLayout( )->addWidget( new QLabel( i18n( "Julia parameter:" ), this ) );
   m_pJuliaParameter = new PMVectorEdit( "", "i", "j", "k", this );
   topLayout( )->addWidget( m_pJuliaParameter );

   hl = new QHBoxLayout( topLayout( ) );
   hl->addWidget( new QLabel( i18n( "Algebra type:" ), this ) );
   m_pAlgebraType = new QComboBox( false, this );
   m_pAlgebraType->insertItem( i18n( "Quaternion" ) );
   m_pAlgebraType->insertItem( i18n( "Hypercomplex" ) );
   hl->addWidget( m_pAlgebraType );
   hl->addStretch( 1 );

   hl = new QHBoxLayout( topLayout( ) );
   hl->addWidget( new QLabel( i18n( "Function type:" ), this ) );
   m_pFunctionType = new QComboBox( false, this );
   m_pFunctionType->insertItem( "sqr" );
   m_pFunctionType->insertItem( "cube" );
   m_pFunctionType->insertItem( "exp" );
   m_pFunctionType->insertItem( "reciprocal" );
   m_pFunctionType->insertItem( "sin" );
   m_pFunctionType->insertItem( "asin" );
   m_pFunctionType->insertItem( "sinh" );
   m_pFunctionType->insertItem( "asinh" );
   m_pFunctionType->insertItem( "cos" );
   m_pFunctionType->insertItem( "acos" );
   m_pFunctionType->insertItem( "cosh" );
   m_pFunctionType->insertItem( "acosh" );
   m_pFunctionType->insertItem( "tan" );
   m_pFunctionType->insertItem( "atan" );
   m_pFunctionType->insertItem( "tanh" );
   m_pFunctionType->insertItem( "atanh" );
   m_pFunctionType->insertItem( "log" );
   m_pFunctionType->insertItem( "pwr" );
   hl->addWidget( m_pFunctionType );
   hl->addStretch( 1 );

   hl = new QHBoxLayout( topLayout( ) );
   m_pExponentsLabel = new QLabel( i18n( "Exponent:" ), this );
   hl->addWidget( m_pExponentsLabel );
   m_pExponents = new PMVectorEdit( "", "i", this );
   hl->addWidget( m_pExponents );
   hl->addStretch( 1 );

   hl = new QHBoxLayout( topLayout( ) );
   gl = new QGridLayout( hl, 2, 2 );
   gl->addWidget( new QLabel( i18n( "Maximum iterations:" ), this ), 0, 0 );
   m_pMaxIterations = new PMIntEdit( this );
   m_pMaxIterations->setValidation( true, 1, false, 0 );
   gl->addWidget( m_pMaxIterations, 0, 1 );
   gl->addWidget( new QLabel( i18n( "Precision:" ), this ), 1, 0 );
   m_pPrecision = new PMFloatEdit( this );
   m_pPrecision->setValidation( true, 1.0, false, 0.0 );
   gl->addWidget( m_pPrecision, 1, 1 );
   hl->addStretch( 1 );
   
   topLayout( )->addWidget( new QLabel( i18n( "Slice normal:" ), this ) );
   m_pSliceNormal = new PMVectorEdit( "", "i", "j", "k", this );
   topLayout( )->addWidget( m_pSliceNormal );

   hl = new QHBoxLayout( topLayout( ) );
   hl->addWidget( new QLabel( i18n( "Slice distance:" ), this ) );
   m_pSliceDistance = new PMFloatEdit( this );
   hl->addWidget( m_pSliceDistance );
   hl->addStretch( 1 );

   connect( m_pJuliaParameter, SIGNAL( dataChanged( ) ), SIGNAL( dataChanged( ) ) );
   connect( m_pAlgebraType, SIGNAL( activated( int ) ),
            SLOT( slotAlgebraTypeSelected( int ) ) );
   connect( m_pFunctionType, SIGNAL( activated( int ) ),
            SLOT( slotFunctionTypeSelected( int ) ) );
   connect( m_pExponents, SIGNAL( dataChanged( ) ), SIGNAL( dataChanged( ) ) );
   connect( m_pMaxIterations, SIGNAL( dataChanged( ) ), SIGNAL( dataChanged( ) ) );
   connect( m_pPrecision, SIGNAL( dataChanged( ) ), SIGNAL( dataChanged( ) ) );
   connect( m_pSliceNormal, SIGNAL( dataChanged( ) ), SIGNAL( dataChanged( ) ) );
   connect( m_pSliceDistance, SIGNAL( dataChanged( ) ), SIGNAL( dataChanged( ) ) );   
}

void PMJuliaFractalEdit::displayObject( PMObject* o )
{
   if( o->isA( "JuliaFractal" ) )
   {
      bool readOnly = o->isReadOnly( );
      m_pDisplayedObject = ( PMJuliaFractal* ) o;

      m_pJuliaParameter->setVector( m_pDisplayedObject->juliaParameter( ) );
      switch( m_pDisplayedObject->algebraType( ) )
      {
         case PMJuliaFractal::Quaternion:
            m_pAlgebraType->setCurrentItem( 0 );
            break;
         case PMJuliaFractal::Hypercomplex:
            m_pAlgebraType->setCurrentItem( 1 );
            break;
      }
      switch( m_pDisplayedObject->functionType( ) )
      {
         case PMJuliaFractal::FTsqr:
            m_pFunctionType->setCurrentItem( 0 );
            break;
         case PMJuliaFractal::FTcube:
            m_pFunctionType->setCurrentItem( 1 );
            break;
         case PMJuliaFractal::FTexp:
            m_pFunctionType->setCurrentItem( 2 );
            break;
         case PMJuliaFractal::FTreciprocal:
            m_pFunctionType->setCurrentItem( 3 );
            break;
         case PMJuliaFractal::FTsin:
            m_pFunctionType->setCurrentItem( 4 );
            break;
         case PMJuliaFractal::FTasin:
            m_pFunctionType->setCurrentItem( 5 );
            break;
         case PMJuliaFractal::FTsinh:
            m_pFunctionType->setCurrentItem( 6 );
            break;
         case PMJuliaFractal::FTasinh:
            m_pFunctionType->setCurrentItem( 7 );
            break;
         case PMJuliaFractal::FTcos:
            m_pFunctionType->setCurrentItem( 8 );
            break;
         case PMJuliaFractal::FTacos:
            m_pFunctionType->setCurrentItem( 9 );
            break;
         case PMJuliaFractal::FTcosh:
            m_pFunctionType->setCurrentItem( 10 );
            break;
         case PMJuliaFractal::FTacosh:
            m_pFunctionType->setCurrentItem( 11 );
            break;
         case PMJuliaFractal::FTtan:
            m_pFunctionType->setCurrentItem( 12 );
            break;
         case PMJuliaFractal::FTatan:
            m_pFunctionType->setCurrentItem( 13 );
            break;
         case PMJuliaFractal::FTtanh:
            m_pFunctionType->setCurrentItem( 14 );
            break;
         case PMJuliaFractal::FTatanh:
            m_pFunctionType->setCurrentItem( 15 );
            break;
         case PMJuliaFractal::FTlog:
            m_pFunctionType->setCurrentItem( 16 );
            break;
         case PMJuliaFractal::FTpwr:
            m_pFunctionType->setCurrentItem( 17 );
            break;
      }
      m_pExponents->setVector( m_pDisplayedObject->exponent( ) );
      if( m_pDisplayedObject->functionType( ) == PMJuliaFractal::FTpwr )
      {
         m_pExponents->show( );
         m_pExponentsLabel->show( );
      }
      else
      {
         m_pExponents->hide( );
         m_pExponentsLabel->hide( );
      }
      
      m_pMaxIterations->setValue( m_pDisplayedObject->maximumIterations( ) );
      m_pPrecision->setValue( m_pDisplayedObject->precision( ) );
      m_pSliceNormal->setVector( m_pDisplayedObject->sliceNormal( ) );
      m_pSliceDistance->setValue( m_pDisplayedObject->sliceDistance( ) );

      m_pJuliaParameter->setReadOnly( readOnly );
      m_pAlgebraType->setEnabled( !readOnly );
      m_pFunctionType->setEnabled( !readOnly );
      m_pExponents->setReadOnly( readOnly );
      m_pMaxIterations->setReadOnly( readOnly );
      m_pPrecision->setReadOnly( readOnly );
      m_pSliceNormal->setReadOnly( readOnly );
      m_pSliceDistance->setReadOnly( readOnly );
      
      Base::displayObject( o );
   }
   else
      kdError( PMArea ) << "PMJuliaFractalEdit: Can't display object\n";
}

void PMJuliaFractalEdit::saveContents( )
{
   if( m_pDisplayedObject )
   {
      Base::saveContents( );
      PMVector p( 4 );

      m_pDisplayedObject->setJuliaParameter( m_pJuliaParameter->vector( ) );
      switch( m_pAlgebraType->currentItem( ) )
      {
         case 0:
            m_pDisplayedObject->setAlgebraType( PMJuliaFractal::Quaternion );
            break;
         case 1:
            m_pDisplayedObject->setAlgebraType( PMJuliaFractal::Hypercomplex );
            break;
         default:
            m_pDisplayedObject->setAlgebraType( PMJuliaFractal::Quaternion );
            break;
      }
      switch( m_pFunctionType->currentItem( ) )
      {
         case 0:
            m_pDisplayedObject->setFunctionType( PMJuliaFractal::FTsqr );
            break;
         case 1:
            m_pDisplayedObject->setFunctionType( PMJuliaFractal::FTcube );
            break;
         case 2:
            m_pDisplayedObject->setFunctionType( PMJuliaFractal::FTexp );
            break;
         case 3:
            m_pDisplayedObject->setFunctionType( PMJuliaFractal::FTreciprocal );
            break;
         case 4:
            m_pDisplayedObject->setFunctionType( PMJuliaFractal::FTsin );
            break;
         case 5:
            m_pDisplayedObject->setFunctionType( PMJuliaFractal::FTasin );
            break;
         case 6:
            m_pDisplayedObject->setFunctionType( PMJuliaFractal::FTsinh );
            break;
         case 7:
            m_pDisplayedObject->setFunctionType( PMJuliaFractal::FTasinh );
            break;
         case 8:
            m_pDisplayedObject->setFunctionType( PMJuliaFractal::FTcos );
            break;
         case 9:
            m_pDisplayedObject->setFunctionType( PMJuliaFractal::FTacos );
            break;
         case 10:
            m_pDisplayedObject->setFunctionType( PMJuliaFractal::FTcosh );
            break;
         case 11:
            m_pDisplayedObject->setFunctionType( PMJuliaFractal::FTacosh );
            break;
         case 12:
            m_pDisplayedObject->setFunctionType( PMJuliaFractal::FTtan );
            break;
         case 13:
            m_pDisplayedObject->setFunctionType( PMJuliaFractal::FTatan );
            break;
         case 14:
            m_pDisplayedObject->setFunctionType( PMJuliaFractal::FTtanh );
            break;
         case 15:
            m_pDisplayedObject->setFunctionType( PMJuliaFractal::FTatanh );
            break;
         case 16:
            m_pDisplayedObject->setFunctionType( PMJuliaFractal::FTlog );
            break;
         case 17:
            m_pDisplayedObject->setFunctionType( PMJuliaFractal::FTpwr );
            break;
         default:
            m_pDisplayedObject->setFunctionType( PMJuliaFractal::FTsqr );
            break;
      }
      if( m_pExponents->isVisible( ) )
         m_pDisplayedObject->setExponent( m_pExponents->vector( ) );
      m_pDisplayedObject->setMaximumIterations( m_pMaxIterations->value( ) );
      m_pDisplayedObject->setPrecision( m_pPrecision->value( ) );
      m_pDisplayedObject->setSliceNormal( m_pSliceNormal->vector( ) );
      m_pDisplayedObject->setSliceDistance( m_pSliceDistance->value( ) );
   }
}

bool PMJuliaFractalEdit::isDataValid( )
{
   if( !m_pJuliaParameter->isDataValid( ) )
      return false;
   if( m_pExponents->isVisible( ) )
      if( !m_pExponents->isDataValid( ) )
         return false;
   if( !m_pMaxIterations->isDataValid( ) )
      return false;
   if( !m_pPrecision->isDataValid( ) )
      return false;
   if( !m_pSliceNormal->isDataValid( ) )
      return false;
   PMVector v = m_pSliceNormal->vector( );
   if( approxZero( v.abs( ) ) )
   {
      KMessageBox::error( this, i18n( "The slice normal vector may not be a null vector." ),
                          i18n( "Error" ) );
      return false;
   }
   if( approxZero( v[3] ) )
   {
      KMessageBox::error( this, i18n( "The 'k' component of the slice normal vector may not be zero." ),
                          i18n( "Error" ) );
      return false;
   }
   
   if( !m_pSliceDistance->isDataValid( ) )
      return false;

   if( m_pAlgebraType->currentItem( ) == 0 )
   {
      if( m_pFunctionType->currentItem( ) > 1 )
      {
         KMessageBox::error( this, i18n( "Only the functions 'sqr' and 'cube' "
                                         "are defined in the quaternion algebra." ),
                             i18n( "Error" ) );
         return false;
      }
   }
         
   
   return Base::isDataValid( );
}

void PMJuliaFractalEdit::slotAlgebraTypeSelected( int )
{
   emit dataChanged( );
}

void PMJuliaFractalEdit::slotFunctionTypeSelected( int index )
{
   if( index == 17 )
   {
      m_pExponents->show( );
      m_pExponentsLabel->show( );
   }
   else
   {
      m_pExponents->hide( );
      m_pExponentsLabel->hide( );
   }
   emit dataChanged( );
}

#include "pmjuliafractaledit.moc"
