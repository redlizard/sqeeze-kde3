/* 
 *
 * $Id: k3bmsfedit.cpp 619556 2007-01-03 17:38:12Z trueg $
 * Copyright (C) 2003 Sebastian Trueg <trueg@k3b.org>
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



#include "k3bmsfedit.h"
#include "k3bglobals.h"

#include <qstringlist.h>
#include <qlineedit.h>
#include <qstyle.h>
#include <qfontmetrics.h>
#include <qapplication.h>



K3bMsfValidator::K3bMsfValidator( QObject* parent, const char* name )
  : QRegExpValidator( K3b::Msf::regExp(), parent, name )
{
}



K3bMsfEdit::K3bMsfEdit( QWidget* parent, const char* name )
  : QSpinBox( parent, name )
{
  setValidator( new K3bMsfValidator( this ) );
  setMinValue( 0 );
  // some very high value (10000 minutes)
  setMaxValue( 10000*60*75 );

  connect( this, SIGNAL(valueChanged(int)),
	   this, SLOT(slotValueChanged(int)) );
}


K3bMsfEdit::~K3bMsfEdit()
{}


QSize K3bMsfEdit::sizeHint() const
{
  // more or less copied from QSpinBox
  constPolish();
  QSize sz = editor()->sizeHint();
  int h = sz.height();
  QFontMetrics fm( font() );
  int w = fm.width( "00:00:00" );
  int wx = fm.width( ' ' )*2;
  int frame = style().pixelMetric( QStyle::PM_SpinBoxFrameWidth );
  return style().sizeFromContents(QStyle::CT_SpinBox, this,
				  QSize( w + wx + downRect().width() + frame*2,
					 h + frame*2).
				  expandedTo( QApplication::globalStrut() ));
}


QString K3bMsfEdit::mapValueToText( int value )
{
  return K3b::framesToString( value, true );
}


K3b::Msf K3bMsfEdit::msfValue() const
{
  return K3b::Msf(value());
}


void K3bMsfEdit::setMsfValue( const K3b::Msf& msf )
{
  setValue( msf.totalFrames() );
}


int K3bMsfEdit::mapTextToValue( bool* ok )
{
  return K3b::Msf::fromString( text(), ok ).lba();
}


void K3bMsfEdit::setText( const QString& str )
{
  bool ok;
  editor()->setText( str );
  setValue( mapTextToValue( &ok) );
}


void K3bMsfEdit::setFrameStyle( int style )
{
  editor()->setFrameStyle( style );
}

void K3bMsfEdit::setLineWidth( int v )
{
  editor()->setLineWidth( v );
}

void K3bMsfEdit::setValue( int v )
{
  int i = editor()->cursorPosition();
  QSpinBox::setValue( v );
  editor()->setCursorPosition( i );
}

void K3bMsfEdit::stepUp()
{
  setValue( value() + currentStepValue() );
}

void K3bMsfEdit::stepDown()
{
  setValue( value() - currentStepValue() );
}

int K3bMsfEdit::currentStepValue() const
{
  int val = 1;

  // look if we are currently editing minutes or seconds
  QString text = editor()->text();
  if( text.length() == 8 ) {
    text = text.mid( editor()->cursorPosition() );
    int num = text.contains( ':' );
    if( num == 1 )
      val = 75;
    else if( num == 2 )
      val = 60*75;
  }

  return val;
}


void K3bMsfEdit::slotValueChanged( int v )
{
  emit valueChanged( K3b::Msf(v) );
}

#include "k3bmsfedit.moc"
