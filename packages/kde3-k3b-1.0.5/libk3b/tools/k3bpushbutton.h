/* 
 *
 * $Id: k3bpushbutton.h 619556 2007-01-03 17:38:12Z trueg $
 * Copyright (C) 2004-2007 Sebastian Trueg <trueg@k3b.org>
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

#ifndef _K3B_PUSH_BUTTON_H_
#define _K3B_PUSH_BUTTON_H_


#include <kpushbutton.h>
#include "k3b_export.h"

/**
 * A pushbutton with delayed popu pmenu support just like the KToolBarButton
 */
class LIBK3B_EXPORT K3bPushButton : public KPushButton
{
 Q_OBJECT

 public:
  /**
   * Default constructor.
   */
  K3bPushButton( QWidget* parent = 0, const char* name = 0 );

  /**
   * Constructor, that sets the button-text to @p text
   */
  K3bPushButton( const QString& text, QWidget* parent = 0, const char* name = 0 );

  /**
   * Constructor, that sets an icon and the button-text to @p text
   */
  K3bPushButton( const QIconSet& icon, const QString& text,
		 QWidget* parent = 0, const char* name = 0 );

  /**
   * Constructor that takes a KGuiItem for the text, the icon, the tooltip
   * and the what's this help
   */
  K3bPushButton( const KGuiItem& item, QWidget* parent = 0, const char* name = 0 );

  /**
   * Destructs the button.
   */
  ~K3bPushButton();

  /**
   * The popup menu will show if the button is pressed down for about half a second
   * or if the mouse is moved while pressed just like the KToolBarButton.
   */
  void setDelayedPopupMenu( QPopupMenu* );

 protected:
  virtual bool eventFilter( QObject*, QEvent* );

 private slots:
  void slotDelayedPopup();

 private:
  class Private;
  Private* d;
};

#endif
