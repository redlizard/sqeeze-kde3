/* kaspopup.cpp
**
** Copyright (C) 2001-2004 Richard Moore <rich@kde.org>
** Contributor: Mosfet
**     All rights reserved.
**
** KasBar is dual-licensed: you can choose the GPL or the BSD license.
** Short forms of both licenses are included below.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
** OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
** SUCH DAMAGE.
*/

/*
** Bug reports and questions can be sent to kde-devel@kde.org
*/
#include <qapplication.h>

#include "kasitem.h"
#include "kasbar.h"

#include "kaspopup.h"
#include "kaspopup.moc"

KasPopup::KasPopup( KasItem *item, const char *name )
    : QHBox( 0, name, WStyle_Customize | WStyle_StaysOnTop | WStyle_Tool | WStyle_NoBorder | WX11BypassWM ),
      item_( item ),
      kasbar_( item->kasbar() )
{
}

KasPopup::~KasPopup()
{
}

void KasPopup::positionSelf()
{
   move( calcPosition( item_, width(), height() ) );
}

void KasPopup::show()
{
   emit aboutToShow();
   positionSelf();
   QWidget::show();
   emit shown();
}

QPoint KasPopup::calcPosition( KasItem *item, int w, int h )
{
   KasBar *kasbar = item->kasbar();
   QPoint pos = kasbar->itemPos( item );

   if ( ( pos.x() < 0 ) && ( pos.y() < 0 ) )
      return QPoint();

   pos = kasbar->mapToGlobal( pos );
   int x = pos.x();
   int y = pos.y();

   if ( kasbar->orientation() == Horizontal ) {
      if ( y < ( qApp->desktop()->height() / 2 ) )
	 y = y + kasbar->itemExtent();
      else
	 y = y - h;

      if ( (x + w) > qApp->desktop()->width() )
         x = x - w + kasbar->itemExtent();
   }
   else {
      if ( x < ( qApp->desktop()->width() / 2 ) )
	 x = x + kasbar->itemExtent();
      else
	 x = x - w;

      if ( (y + h) > qApp->desktop()->height() )
         y = y - h + kasbar->itemExtent();
   }

   return QPoint( x, y );
}



