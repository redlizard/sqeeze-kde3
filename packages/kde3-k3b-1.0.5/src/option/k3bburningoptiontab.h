/*
 *
 * $Id: k3bburningoptiontab.h 619556 2007-01-03 17:38:12Z trueg $
 * Copyright (C) 2003 Sebastian Trueg <trueg@k3b.org>
 *
 * This file is part of the K3b project.
 * Copyright (C) 1998-2007 Sebastian Trueg <trueg@k3b.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */


#ifndef K3B_BURNING_OPTION_TAB_H
#define K3B_BURNING_OPTION_TAB_H

#include <qwidget.h>

class QCheckBox;
class QLabel;
class QGroupBox;
class QComboBox;
class QString;
class QSpinBox;
class KIntNumInput;
class QRadioButton;



class K3bBurningOptionTab : public QWidget
{
Q_OBJECT

 public:
  K3bBurningOptionTab( QWidget* parent = 0, const char* name = 0 );
  ~K3bBurningOptionTab();

  void saveSettings();
  void readSettings();

 private slots:
  void slotSetDefaultBufferSizes( bool );

 private:
  void setupGui();

  QCheckBox*    m_checkBurnfree;
  QCheckBox*    m_checkEject;
  QCheckBox*    m_checkAutoErasingRewritable;
  QCheckBox*    m_checkOverburn;
  QCheckBox*    m_checkManualWritingBufferSize;
  KIntNumInput* m_editWritingBufferSize;
  QCheckBox*    m_checkAllowWritingAppSelection;
  QCheckBox*    m_checkForceUnsafeOperations;
};


#endif
