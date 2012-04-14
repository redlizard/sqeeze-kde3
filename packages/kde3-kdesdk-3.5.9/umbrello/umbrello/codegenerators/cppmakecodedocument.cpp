
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

/*  This code generated by:
 *      Author : thomas
 *      Date   : Tue Jun 24 2003
 */

#include "cppcodegenerator.h"

#include <qregexp.h>

const char * CPPMakefileCodeDocument::DOCUMENT_ID_VALUE = "Makefile_DOC";

// Constructors/Destructors
//

CPPMakefileCodeDocument::CPPMakefileCodeDocument ( )
{
    setFileName("Makefile"); // default name
    setFileExtension("");
    setID(DOCUMENT_ID_VALUE); // default id tag for this type of document
}

CPPMakefileCodeDocument::~CPPMakefileCodeDocument ( ) { }

//
// Methods
//

// Other methods
//

// we add in our code blocks that describe how to generate
// the project here...
void CPPMakefileCodeDocument::updateContent( ) {
    // FIX : fill in content
}

/**
 * @return      QString
 */
QString CPPMakefileCodeDocument::toString ( ) {
    return "# cpp make build document";
}

// We overwritten by CPP language implementation to get lowercase path
QString CPPMakefileCodeDocument::getPath ( )
{

    QString path = getPackage();

    // Replace all white spaces with blanks
    path.simplifyWhiteSpace();

    // Replace all blanks with underscore
    path.replace(QRegExp(" "), "_");

    path.replace(QRegExp("\\."),"/");
    path.replace(QRegExp("::"),"/");

    path.lower();

    return path;

}


#include "cppmakecodedocument.moc"
