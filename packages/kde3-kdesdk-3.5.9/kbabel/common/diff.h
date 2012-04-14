/* **************************************************************************
  This file is part of KBabel

  Copyright (C) 2001 by Matthias Kiefer <kiefer@kde.org>
  charDiff algorithm by Wolfram Diestel <wolfram@steloj.de>
  wordDiff algorithm by Nick Shaforostoff <shafff@ukr.net>
  (based on Markus Stengel's GPL implementation of LCS-Delta algorithm as it is described in "Introduction to Algorithms", MIT Press, 2001, Second Edition, written by Thomas H. Cormen et. al. It uses dynamic programming to solve the Longest Common Subsequence (LCS) problem. - http://www.markusstengel.de/text/en/i_4_1_5_3.html)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

  In addition, as a special exception, the copyright holders give
  permission to link the code of this program with any edition of
  the Qt library by Trolltech AS, Norway (or with modified versions
  of Qt that use the same license as Qt), and distribute linked
  combinations including the two.  You must obey the GNU General
  Public License in all respects for all of the code used other than
  Qt. If you modify this file, you may extend this exception to
  your version of the file, but you are not obligated to do so.  If
  you do not wish to do so, delete this exception statement from
  your version.

************************************************************************** */
#ifndef DIFF_H
#define DIFF_H

#include <qvaluevector.h>
#include <qstringlist.h>

typedef enum
{
    NOTHING       = 0,
    ARROW_UP      = 1,
    ARROW_LEFT    = 2,
    ARROW_UP_LEFT = 3,
    FINAL         = 4
} LCSMarker;


/**
 * Word-by-word diff algorithm
 *
 * @short Word-by-word diff algorithm
 * @author Nick Shaforostoff <shafff@ukr.net>
 */
    QString wordDiff(const QString& oldString, const QString& newString);


/**
     * This class is for keeping "global" params of recursive function
     *
     * @short Class for keeping "global" params of recursive function
     * @author Nick Shaforostoff <shafff@ukr.net>
 */
    class LCSprinter
{
    public:
        LCSprinter(const QStringList &s_1, const QStringList &s_2, QValueVector<LCSMarker>* b_, const uint nT_, uint index);
        void printLCS(uint index);
        inline QString getString();

        ~LCSprinter() {};
    private:
        QStringList s1, s2, resultString;
        uint nT;//for use 1d vector as 2d
        QValueVector<LCSMarker> *b;
        QStringList::iterator it1, it2;
};


inline QString LCSprinter::getString()
{
    return resultString.join("").replace(QChar('\n'), ""); //w/o replace we'd get whole line colored
}

#endif // DIFF_H

