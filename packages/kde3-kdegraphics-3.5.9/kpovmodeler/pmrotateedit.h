//-*-C++-*-
/*
**************************************************************************
                                 description
                             --------------------
    copyright            : (C) 2000-2001 by Andreas Zehender
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


#ifndef PMROTATEEDIT_H
#define PMROTATEEDIT_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "pmdialogeditbase.h"

class PMRotate;
class PMVectorEdit;

/**
 * Dialog edit class for @ref PMRotate
 */
class PMRotateEdit : public PMDialogEditBase
{
   Q_OBJECT
   typedef PMDialogEditBase Base;
public:
   /**
    * Creates a PMRotateEdit with parent and name
    */
   PMRotateEdit( QWidget* parent, const char* name = 0 );

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
   PMRotate* m_pDisplayedObject;
   PMVectorEdit* m_pVector;
};


#endif
