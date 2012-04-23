/* 
 *
 * $Id: k3brichtextlabel.h 619556 2007-01-03 17:38:12Z trueg $
 * Copyright (C) 2005 Waldo Bastian <bastian@kde.org>
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

#ifndef K3BRICHTEXTLABEL_H
#define K3BRICHTEXTLABEL_H

#include <qlabel.h>

#include <k3b_export.h>

/**
 * @short A replacement for QLabel that supports richtext and proper layout management
 *
 * @author Waldo Bastian <bastian@kde.org>
 */

/*
 * QLabel
 */
class LIBK3B_EXPORT K3bRichTextLabel : public QLabel {
  Q_OBJECT

public:
  /**
   * Default constructor.
   */
  K3bRichTextLabel( QWidget *parent, const char *name = 0 );
  K3bRichTextLabel( const QString &text, QWidget *parent, const char *name = 0 );

  int defaultWidth() const { return m_defaultWidth; }
  void setDefaultWidth(int defaultWidth);

  virtual QSize minimumSizeHint() const;
  virtual QSize sizeHint() const;
  QSizePolicy sizePolicy() const;

public slots:
  void setText( const QString & );

protected:
  int m_defaultWidth;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class K3bRichTextLabelPrivate;
  K3bRichTextLabelPrivate *d;
};

#endif // K3BRICHTEXTLABEL_H
