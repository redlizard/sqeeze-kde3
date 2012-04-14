/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLOPERATIONLIST_H
#define UMLOPERATIONLIST_H

#include <qptrlist.h>

// forward declaration
class UMLOperation;

typedef QPtrList<UMLOperation> UMLOperationList;
typedef QPtrListIterator<UMLOperation> UMLOperationListIt;

#endif
