/*
 *
 * $Id: k3bpatternparser.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef K3BPATTERNPARSER_H
#define K3BPATTERNPARSER_H

#include <qstring.h>

#include <k3bcddbquery.h>


/**
  *@author Sebastian Trueg
  */
class K3bPatternParser
{
 public:
  static QString parsePattern( const K3bCddbResultEntry& entry,
                               unsigned int trackNumber,
                               const QString& pattern,
                               bool replace = false,
                               const QString& replaceString = "_" );

 private:
  enum {
    TITLE   = 't',
    ARTIST  = 'a',
    NUMBER  = 'n',
    COMMENT = 'c',
    YEAR    = 'y',
    GENRE   = 'g',
    ALBUMTITLE   = 'T',
    ALBUMARTIST  = 'A',
    ALBUMCOMMENT = 'C',
    DATE = 'd'
  };
};

#endif
