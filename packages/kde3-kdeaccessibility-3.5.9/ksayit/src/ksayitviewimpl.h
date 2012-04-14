/***************************************************************************
                          ksayitviewimpl.h  -  description
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

#ifndef KSAYITVIEWIMPL_H
#define KSAYITVIEWIMPL_H

// QT includes
#include <qwidget.h>
#include <qstring.h>

// KDE includes

// App includes
#include <KSayItView.h>

/**
  *@author Robert Vogl
  */

class KSayItViewImpl : public KSayItView  {
   Q_OBJECT

public: 
    KSayItViewImpl(QWidget *parent=0, const char *name=0);
    ~KSayItViewImpl();

signals:
  void signalSetCaption(const QString &caption);
  void signalShowStatus(const QString &status);
  void signalEnableCopyCut(bool enable);
  
  /** Emitted when the text in the TextEditor view cahnges.
   * \param empty True if the TextEditor view is empty. 
   */
  void signalTextChanged(bool empty);

public slots:
  /** True if text is selected or false if text is deselected.
   */
  void slotCopyAvailable(bool available);
  
  /** Copy selected text to the clipboard.
   */
  void slotCopy();
  
  /** Copy selected text to the clipboard and delete it.
   */
  void slotCut();
  
  /** Paste text from the clipboard to the texteditor.
   */
  void slotPaste();
  
private slots:
  /** Called from the widget
   */
  void slotTextChanged();

public: // Methods
  /** Returns the text of the TextEdit-Widget
   */
  QString& getText();
  
  /** Set the content of the textEdit-Widget to text
   */
  void setText(const QString &text);
  
  /** Enables/disables the Textedit
   * \param enable true=enabled, false=diabled
   */
  void enableTextedit( bool enable );
  
  /** Deletes the entire text of the texteditor.
   */
  void textClear();

private: 
  QString t;

     
};

#endif
