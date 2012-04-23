/* 
 *
 * $Id: k3bjobprogressosd.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef _K3B_JOB_PROGRESS_OSD_H_
#define _K3B_JOB_PROGRESS_OSD_H_

#include <qwidget.h>
#include <qpixmap.h>

class QPaintEvent;
class QMouseEvent;
class KConfigBase;

/**
 * An OSD displaying a text and a progress bar.
 *
 * Insprired by Amarok's OSD (I also took a bit of their code. :)
 */
class K3bJobProgressOSD : public QWidget
{
  Q_OBJECT

 public:
  K3bJobProgressOSD( QWidget* parent = 0, const char* name = 0 );
  ~K3bJobProgressOSD();

  int screen() const { return m_screen; }
  const QPoint& position() const { return m_position; }

  void readSettings( KConfigBase* );
  void saveSettings( KConfigBase* );

 public slots:
  void setScreen( int );
  void setText( const QString& );
  void setProgress( int );

  /**
   * The position refers to one of the corners of the widget
   * regarding on the value of the x and y coordinate.
   * If for example the x coordinate is bigger than half the screen
   * width it refers to the left edge of the widget.
   */
  void setPosition( const QPoint& );

  void show();

 protected:
  void paintEvent( QPaintEvent* );
  void mousePressEvent( QMouseEvent* );
  void mouseReleaseEvent( QMouseEvent* );
  void mouseMoveEvent( QMouseEvent* );
  void renderOSD();
  void reposition( QSize size = QSize() );

 protected slots:
  void refresh();

 private:
  /**
   * Ensure that the position is inside m_screen 
   */
  QPoint fixupPosition( const QPoint& p );
  static const int s_outerMargin = 15;

  QPixmap m_osdBuffer;
  bool m_dirty;
  QString m_text;
  int m_progress;
  bool m_dragging;
  QPoint m_dragOffset;
  int m_screen;
  QPoint m_position;
};

#endif
