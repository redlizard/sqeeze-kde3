/* 
 *
 * $Id: k3btitlelabel.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef _K3B_TITLE_LABEL_H_
#define _K3B_TITLE_LABEL_H_

#include <qframe.h>
#include "k3b_export.h"
class QPainter;
class QResizeEvent;


class LIBK3B_EXPORT K3bTitleLabel : public QFrame
{
  Q_OBJECT

 public:
  K3bTitleLabel( QWidget* parent = 0, const char* name = 0 );
  ~K3bTitleLabel();

  QSize sizeHint() const;
  QSize minimumSizeHint() const;

 public slots:
   /**
    * default: 2
    */
  void setMargin( int );

  void setTitle( const QString& title, const QString& subTitle = QString::null );
  void setSubTitle( const QString& subTitle );

  /**
   * The title label only supports alignments left, hcenter, and right
   *
   * Default alignment is left.
   */
  // FIXME: honor right-to-left languages
  void setAlignment( int align );

 protected:
  void resizeEvent( QResizeEvent* );
  void drawContents( QPainter* p );

 private:
  void updatePositioning();

  class ToolTip;
  ToolTip* m_toolTip;

  class Private;
  Private* d;
};

#endif
