/* 
 *
 * $Id: k3bstringutils.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef _K3B_STRING_UTILS_H_
#define _K3B_STRING_UTILS_H_

#include <qstring.h>

class QFontMetrics;

namespace K3b
{
  /**
   * Cuts the text at the end.
   * Example: "some long text" -> "some lo..."
   */
  QString cutToWidth( const QFontMetrics&, const QString&, int );

  /**
   * squeezes the text.
   * Example: "some long text" -> "some...ext"
   */
  QString squeezeTextToWidth( const QFontMetrics& fm, const QString& fullText, int cutWidth );
}

#endif
