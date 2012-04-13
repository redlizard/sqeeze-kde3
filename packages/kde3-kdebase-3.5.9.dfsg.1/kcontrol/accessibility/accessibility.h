/****************************************************************************
  accessibility.h
  KDE Control Accessibility module to control Bell, Keyboard and ?Mouse?
  ------------------- 
  Copyright : (c) 2000 Matthias H�lzer-Kl�pfel
  -------------------
  Original Author: Matthias H�lzer-Kl�pfel
  Contributors: Jos� Pablo Ezequiel "Pupeno" Fern�ndez <pupeno@kde.org> 
  Current Maintainer: Jos� Pablo Ezequiel "Pupeno" Fern�ndez <pupeno@kde.org> 
 ****************************************************************************/

/****************************************************************************
 *                                                                          *
 *   This program is free software; you can redistribute it and/or modify   *
 *   it under the terms of the GNU General Public License as published by   *
 *   the Free Software Foundation; either version 2 of the License, or      *
 *   (at your option) any later version.                                    *
 *                                                                          *
 ****************************************************************************/

#ifndef _ACCESSIBILITY_H_
#define _ACCESSIBILITY_H_

#include <kdebug.h>

#include "accessibilityconfigwidget.h"

class AccessibilityConfig : public AccessibilityConfigWidget {
   Q_OBJECT

   public:
      /**
       * Constructor
       */   
      AccessibilityConfig(QWidget *parent = 0L, const char *name = 0L, const QStringList &foo = QStringList());

      /**
       * Destructor
       */            
      virtual ~AccessibilityConfig();
  
      /**
       * This method is invoked whenever the module should read its 
       * configuration (most of the times from a config file) and update the 
       * user interface. This happens when the user clicks the "Reset" button in 
       * the control center, to undo all of his changes and restore the currently 
       * valid settings. NOTE that this is not called after the modules is loaded,
       * so you probably want to call this method in the constructor.
       */
      void load();
      void load( bool useDefaults );

      /**
       * This function gets called when the user wants to save the settings in 
       * the user interface, updating the config files or wherever the 
       * configuration is stored. The method is called when the user clicks "Apply" 
       * or "Ok".
       */
      void save();

      /**
       * This function is called to set the settings in the module to sensible
       * default values. It gets called when hitting the "Default" button. The 
       * default values should probably be the same as the ones the application 
       * uses when started without a config file.
       */
      void defaults();

};

#endif // _ACCESSIBILITY_H_
