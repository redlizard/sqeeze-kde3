/* This file is part of the KDE libraries
   Copyright (C) 2004-2005 Anders Lund <anders@alweb.dk>
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>
   Copyright (C) 2001-2004 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>

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

#ifndef __KATE_SPELL_H__
#define __KATE_SPELL_H__

#include "katecursor.h"

class KateView;

class KAction;
class KSpell;

class KateSpell : public QObject
{
  Q_OBJECT

  public:
    KateSpell( KateView* );
    ~KateSpell();

    void createActions( KActionCollection* );

    void updateActions ();

  // spellcheck from cursor, selection
  private slots:
    void spellcheckFromCursor();

    // defined here in anticipation of pr view selections ;)
    void spellcheckSelection();

    void spellcheck();

      /**
      * Spellcheck a defined portion of the text.
      *
      * @param from Where to start the check
      * @param to Where to end. If this is (0,0), it will be set to the end of the document.
      */
    void spellcheck( const KateTextCursor &from, const KateTextCursor &to=KateTextCursor() );

    void ready(KSpell *);
    void misspelling( const QString&, const QStringList&, unsigned int );
    void corrected  ( const QString&, const QString&, unsigned int);
    void spellResult( const QString& );
    void spellCleanDone();

    void locatePosition( uint pos, uint& line, uint& col );

  private:
    KateView *m_view;
    KAction *m_spellcheckSelection;

    KSpell *m_kspell;

    // define the part of the text to check
    KateTextCursor m_spellStart, m_spellEnd;

    // keep track of where we are.
    KateTextCursor m_spellPosCursor;
    uint m_spellLastPos;
};

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
