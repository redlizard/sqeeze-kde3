/* 
 *
 * $Id: k3bintvalidator.h 619556 2007-01-03 17:38:12Z trueg $
 * Copyright (C) 2004 Sebastian Trueg <trueg@k3b.org>
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


#ifndef _K3B_INT_VALIDATOR_H_
#define _K3B_INT_VALIDATOR_H_

#include <qvalidator.h>
#include "k3b_export.h"
class QWidget;
class QString;

/**
 * QValidator for integers.
 *
 * It differs from QIntValidator and KIntValidator in the fact that
 * it also accepts hex numbers prefixed with 0x.
 */
class LIBK3B_EXPORT K3bIntValidator : public QValidator
{
 public:
  /**
   * Constuctor.  Also sets the base value.
   */
  K3bIntValidator ( QWidget * parent, const char * name = 0 );

  /**
   * Constructor.  Also sets the minimum, maximum, and numeric base values.
   */
  K3bIntValidator ( int bottom, int top, QWidget * parent, const char * name = 0 );

  /**
   * Destructs the validator.
   */
  virtual ~K3bIntValidator ();

  /**
   * Validates the text, and return the result.  Does not modify the parameters.
   */
  virtual State validate ( QString &, int & ) const;

  /**
   * Fixes the text if possible, providing a valid string.  The parameter may be modified.
   */
  virtual void fixup ( QString & ) const;

  /**
   * Sets the minimum and maximum values allowed.
   */
  virtual void setRange ( int bottom, int top );

  /**
   * Returns the current minimum value allowed.
   */
  virtual int bottom () const;

  /**
   * Returns the current maximum value allowed.
   */
  virtual int top () const;

  /**
   * If the string starts with 0x it's assumed to be a hex number.
   */
  static int toInt( const QString&, bool* ok = 0 );

 private:
  int m_min;
  int m_max;
};

#endif
