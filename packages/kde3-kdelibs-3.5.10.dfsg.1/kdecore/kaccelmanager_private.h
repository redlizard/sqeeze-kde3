/*  This file is part of the KDE project
    Copyright (C) 2002 Matthias Hoelzer-Kluepfel <mhk@kde.org>

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


#ifndef __KACCELMANAGER_PRIVATE_H__
#define __KACCELMANAGER_PRIVATE_H__


#include <qstring.h>
#include <qmemarray.h>
#include <qvaluelist.h>
#include <qobject.h>

class QWidgetStack;

/**
 * A string class handling accelerators.
 *
 * This class contains a string and knowledge about accelerators.
 * It keeps a list weights, telling how valuable each character
 * would be as an accelerator.
 *
 * @author Matthias Hoelzer-Kluepfel <mhk@kde.org>
*/

class KAccelString
{
public:

  KAccelString() : m_pureText(), m_accel(-1) {}
  KAccelString(const QString &input, int initalWeight=-1);

  void calculateWeights(int initialWeight);

  const QString &pure() const { return m_pureText; }
  QString accelerated() const;

  int accel() const { return m_accel; }
  void setAccel(int accel) { m_accel = accel; }

  int originalAccel() const { return m_orig_accel; }
  QString originalText() const { return m_origText; }

  QChar accelerator() const;

  int maxWeight(int &index, const QString &used);

  bool operator == (const KAccelString &c) const { return m_pureText == c.m_pureText && m_accel == c.m_accel && m_orig_accel == c.m_orig_accel; }


private:

  int stripAccelerator(QString &input);

  void dump();

  QString        m_pureText,  m_origText;
  int            m_accel, m_orig_accel;
  QMemArray<int> m_weight;

};


typedef QValueList<KAccelString> KAccelStringList;


/**
 * This class encapsulates the algorithm finding the 'best'
 * distribution of accelerators in a hierarchy of widgets.
 *
 * @author Matthias Hoelzer-Kluepfel <mhk@kde.org>
*/

class KAccelManagerAlgorithm
{
public:

  /// Constants used in the algorithm
  enum {
    /// Default control weight
    DEFAULT_WEIGHT = 50,
    /// Additional weight for first character in string
    FIRST_CHARACTER_EXTRA_WEIGHT = 50,
    /// Additional weight for the beginning of a word
    WORD_BEGINNING_EXTRA_WEIGHT = 50,
    /// Additional weight for the dialog buttons (large, we basically never want these reassigned)
    DIALOG_BUTTON_EXTRA_WEIGHT = 300,
    /// Additional weight for a 'wanted' accelerator
    WANTED_ACCEL_EXTRA_WEIGHT = 150,
    /// Default weight for an 'action' widget (ie, pushbuttons)
    ACTION_ELEMENT_WEIGHT = 50,
    /// Default weight for group boxes (low priority)
    GROUP_BOX_WEIGHT = -2000,
    /// Default weight for menu titles
    MENU_TITLE_WEIGHT = 250,
    /// Additional weight for KDE standard accelerators
    STANDARD_ACCEL = 300
  };

  /// Method to call to find the best distribution of accelerators.
  static void findAccelerators(KAccelStringList &result, QString &used);

};


class QPopupMenu;


/**
 * This class manages a popup menu. It will notice if entries have been
 * added or changed, and will recalculate the accelerators accordingly.
 *
 * This is necessary for dynamic menus like for example in kicker.
 *
 * @author Matthias Hoelzer-Kluepfel <mhk@kde.org>
*/

class KPopupAccelManager : public QObject
{
  Q_OBJECT

public:

  static void manage(QPopupMenu *popup);


protected:

  KPopupAccelManager(QPopupMenu *popup);


private slots:

  void aboutToShow();


private:

  void calculateAccelerators();

  void findMenuEntries(KAccelStringList &list);
  void setMenuEntries(const KAccelStringList &list);

  QPopupMenu       *m_popup;
  KAccelStringList m_entries;
  int              m_count;

};


class QWidgetStackAccelManager : public QObject
{
  Q_OBJECT

public:

  static void manage(QWidgetStack *popup);


protected:

  QWidgetStackAccelManager(QWidgetStack *popup);


private slots:

  void aboutToShow(QWidget *);
    bool eventFilter ( QObject * watched, QEvent * e );

private:

  void calculateAccelerators();

  QWidgetStack     *m_stack;
  KAccelStringList m_entries;

};


#endif
