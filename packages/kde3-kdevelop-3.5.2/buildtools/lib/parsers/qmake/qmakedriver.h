/***************************************************************************
 *   Copyright (C) 2005 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef QMAKEQMAKEDRIVER_H
#define QMAKEQMAKEDRIVER_H

class QString;
class KURL;

namespace QMake {

/**
@file qmakedriver.h
Driver for a qmake parser.
*/

class ProjectAST;

/**
Driver.
Use methods of this class to lauch parsing and build the AST.
*/
class Driver{
public:
    /**Parses the file @p fileName and stores the resulting ProjectAST root
    into @p ast. @p ast should not be initialized before. Driver will
    initialize it on its own.
    @return The result of parsing. Result is 0 on success and <> 0 on failure.
    */
    static int parseFile(const char *fileName, ProjectAST **ast, int debug);
    static int parseFile(QString fileName, ProjectAST **ast, int debug);
    static int parseFile(KURL fileName, ProjectAST **ast, int debug);
    static int parseString(const char* string, ProjectAST **ast, int debug);

};

}

#endif
