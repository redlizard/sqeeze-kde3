/* This file is part of the KDE libraries
   Copyright (C) 1997 David Sweet <dsweet@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kspell.h"
#include <kapplication.h>
#include <kdebug.h>

#include <qstring.h>
#include <qlabel.h>
#include <qtextcodec.h>

int main(int argc, char **argv)
{
    KApplication *app = new KApplication(argc, argv, "KSpellTest");

    // test ASCII
    //QString text( "I have noo idee of how to wride englisch or englisch" );

    // test Latin-3 (need dictionary "esperanto" and "Latin3" in config)
    //QCString ctext( "sed jen: e�o�an�' �iu�a�de; e�u�un�o �iun fendredon - kaj diman�on...");
    //QTextCodec *codec = QTextCodec::codecForName("ISO 8859-3");

    // test UTF-8 (need dictionary "esperanto" and "UTF-8" in config)  
   QCString ctext( "sed jen: e\304\245oŝanĝo ĉiuĵaŭde e\304\245uŝunĝo ĝiun fendredon kaj dimanĝon");
   QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    
    QString text = codec->toUnicode(ctext.data());

    KSpell::modalCheck( text );

    kdDebug() << "Returned " << text << endl;

    return 0;

}







