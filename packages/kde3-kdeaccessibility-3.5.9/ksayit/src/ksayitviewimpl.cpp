/***************************************************************************
                          ksayitviewimpl.cpp  -  description
                             -------------------
    begin                : Son Aug 10 2003
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
#include <qradiobutton.h>
#include <qtextstream.h>
#include <qstring.h>

// KDE includes
#include <kdebug.h>
#include <klocale.h>
#include <ktextedit.h>
#include <kmessagebox.h>

// App specific includes
#include "ksayitviewimpl.h"

KSayItViewImpl::KSayItViewImpl(QWidget *parent, const char *name ) : KSayItView(parent,name) {

  // some presets

}
KSayItViewImpl::~KSayItViewImpl(){
}

QString& KSayItViewImpl::getText(){
  t = TextEdit->text();
  return t;
}


void KSayItViewImpl::enableTextedit( bool enable )
{
    // if enable==true, we are in Edit Mode => RTF-Mode off.
    if ( enable ){
        TextEdit->setTextFormat( Qt::PlainText);
    } else {
        TextEdit->setTextFormat( Qt::RichText);
    }    
    TextEdit->setReadOnly( !enable );
}


void KSayItViewImpl::slotTextChanged()
{
  int length = TextEdit->length();     
  if ( length > 2 ){
    emit signalTextChanged(false);
  } else {
    emit signalTextChanged(true);
  }
}

void KSayItViewImpl::slotCopyAvailable(bool available)
{
  // enable/disable copy/cut-action in the menubar
  if (available)
    emit signalEnableCopyCut(true);
  else
    emit signalEnableCopyCut(false);
}

void KSayItViewImpl::slotCopy()
{
  // copy selected text to the clipboard
  TextEdit->copy();
}


void KSayItViewImpl::slotCut()
{
  // copy selected text to the clipboard and delete it
  TextEdit->cut();
}


void KSayItViewImpl::slotPaste()
{
  // paste text from the clipboard to the texteditor 
  TextEdit->paste();
}

void KSayItViewImpl::textClear()
{
  // deletes the entire text of the texteditor
  TextEdit->clear();
}

void KSayItViewImpl::setText(const QString &text)
{
  // set text to text
  TextEdit->setText( text );
}



#include "ksayitviewimpl.moc"
