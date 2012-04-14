//-*-C++-*-
/*
**************************************************************************
                                 description
                             --------------------
    copyright            : (C) 2001 by Andreas Zehender
    email                : zehender@kde.org
**************************************************************************

**************************************************************************
*                                                                        *
*  This program is free software; you can redistribute it and/or modify  *
*  it under the terms of the GNU General Public License as published by  *
*  the Free Software Foundation; either version 2 of the License, or     *
*  (at your option) any later version.                                   *
*                                                                        *
**************************************************************************/


#ifndef PMTEXTUREMAPEDIT_H
#define PMTEXTUREMAPEDIT_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "pmtexturebaseedit.h"
#include <qptrlist.h>

class PMTextureMapBase;
class PMFloatEdit;
class QWidget;
class QVBoxLayout;
class QLabel;

/**
 * Dialog edit class for @ref PMTextureMapBase.
 */
class PMTextureMapEdit : public PMTextureBaseEdit
{
   Q_OBJECT
   typedef PMTextureBaseEdit Base;
public:
   /**
    * Creates a PMTextureMapEdit with parent and name
    */
   PMTextureMapEdit( QWidget* parent, const char* name = 0 );

   /** */
   virtual void displayObject( PMObject* o );

   /** */
   virtual bool isDataValid( );
protected:
   /** */
   virtual void createTopWidgets( );
   /** */
   virtual void saveContents( );
   
private:
   PMTextureMapBase* m_pDisplayedObject;
   QPtrList<PMFloatEdit> m_edits;
   QVBoxLayout* m_pEditLayout;
   QLabel* m_pNoChildLabel;
   QLabel* m_pPureLinkLabel;
   int m_numValues;
};


#endif
