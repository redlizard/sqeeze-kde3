/***************************************************************************
                          kverbedit.h  -  description
                             -------------------
    begin                : Sat Dec 15 2001
    copyright            : (C) 2001 by Arnold Kraschinski
    email                : arnold.k67@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KVERBEDIT_H
#define KVERBEDIT_H

#include "qverbedit.h"
#include "kverbosdoc.h"

/**
  * In dieser Klasse geht es darum die Liste der Verben zu editieren, nicht
  * gew�nschte Eintragungen zu l�schen, weitere Eintragungen zu erg�nzen
  * �nderungen an den Eintragungen vorzunehmen.
  *
  *@author Arnold Kraschinski
  */

class KVerbEdit : public QVerbEdit  {
public:
  /** An die Dialogklasse wird direkt die Verbliste �bergeben, damit
   *  die Liste der Verben bearbeitet werden kann.
   */
  KVerbEdit(spanishVerbList& l, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0);
  ~KVerbEdit();
  /** kopiert die Liste in die ListView. */
  void listToView();
  /** Die Liste aus der Dialogklasse holen. */
  spanishVerbList getList() { return list; }
  /** returns if the list in the dialog has been changed */
  bool wasChanged() const { return changed; }

public slots:
  /** Wird durch den OK-Button aufgerufen und beendet den Dialog. �nderungen
   *  werden �bernommen.
   */
  virtual void slotDlgEnde();
  /** Wird durch den Cancel-Button aufgerufen und beendet den Dialog ohne
   *  �nderungen zu �bernehmen.
   */
  virtual void slotCancel();
  /** Ist ein Verb in der Listenansicht ausgew�hlt, dann wird der Dialog
   *  KErfassen mit diesem Verb gestartet, ansonsten leer. Es k�nnen neue
   *  Verben eingegeben werden oder alte ver�ndert werden.
   */
  virtual void slotEditVerb();
  /** Das in der Listenansicht ausgew�hlte Verb wird gel�scht. */
  virtual void slotDelete();

private:	
  /** Der Zeiger auf die Liste im Dokument.
   */
  spanishVerbList list;
  /** zeigt an, ob der momentane Inhalt der Felder des Dialoges schon
   *  in die Liste aufgenommen wurde, das ist wichtig, um beim Schlie�en
   *  keine Informationen zu verlieren.
   */
  bool changed;
};

#endif
