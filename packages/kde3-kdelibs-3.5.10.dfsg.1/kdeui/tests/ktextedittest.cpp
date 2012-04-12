/* This file is part of the KDE libraries
    Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kapplication.h>
#include <ktextedit.h>

#include <qfile.h>

int main( int argc, char **argv )
{
    KApplication app( argc, argv, "ktextedittest" );
    KTextEdit *edit = new KTextEdit();
    edit->setReadOnly( true );
    edit->setCheckSpellingEnabled( true );

    QFile file( "ktextedittest.cpp" );
    if ( file.open( IO_ReadOnly ) )
    {
        edit->setText( file.readAll() );
        file.close();
    }

    edit->resize( 600, 600 );
    edit->show();
    edit->setReadOnly( false ); // also enables spell checking
    return app.exec();
}
