/***************************************************************************
 *   Copyright (C) 2003 by Julian Rockey                                   *
 *   linux@jrockey.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/



#ifndef __KDEVPART_FILECREATE_H__
#define __KDEVPART_FILECREATE_H__


#include <qguardedptr.h>
#include <qptrlist.h>

#include <qwidget.h>
#include <kdevplugin.h>
#include <kdevcreatefile.h>

#include "filecreate_typechooser.h"

class KPopupMenu;

namespace FileCreate {
  class Widget;
  class FileType;
}

class KDialogBase;
class FCConfigWidget;
class ConfigWidgetProxy;

using namespace FileCreate;

class FileCreatePart : public KDevCreateFile
{
  Q_OBJECT

  friend class FCConfigWidget;
public:
  FileCreatePart(QObject *parent, const char *name, const QStringList &);
  virtual ~FileCreatePart();

  /**
   * Returns the list of available file types.
   */
  QPtrList<FileType> getFileTypes() const { return m_filetypes; }

  /**
   * Call this method to create a new file, within or without the project. Supply as
   * much information as you know. Leave what you don't know as QString::null.
   * The user will be prompted as necessary for the missing information, and the
   * file created, and added to the project as necessary.
   */
  virtual KDevCreateFile::CreatedFile createNewFile(QString ext = QString::null,
                     QString dir = QString::null,
                     QString name = QString::null,
                     QString subtype = QString::null);

  /**
   * Finds the file type object for a given extension and optionally subtype.
   * You can omit the subtype and specify the extension as ext-subtype if you wish.
   */
  FileType * getType(const QString & ext, const QString subtype = QString::null);
  /**
   * Finds the file type object for a given extension and optionally subtype.
   * You can omit the subtype and specify the extension as ext-subtype if you wish.
   * Returns only enabled type (i.e. used in the project).
   */
  FileType * getEnabledType(const QString & ext, const QString subtype = QString::null);

public slots:

  void slotProjectOpened();
  void slotProjectClosed();
  void slotInitialize();
  void slotGlobalInitialize();

  /**
   * Called when a file type is selected from the docked widget.
   */
  void slotFiletypeSelected(const FileType *);

  /**
   * Called when a new file is required - for example, from the "New.." menu action.
   */
  void slotNewFile();

  /**
   * Called from KToolBarPopupMenu to request a new file action
   * @param pFileType is acutally a pointer to FileType
   */
  void slotNewFilePopup(int pFileType);

protected slots:
  void slotNoteFiletype(const FileType * filetype);
  void slotAboutToShowNewPopupMenu();
  void insertConfigWidget( const KDialogBase * dlg, QWidget * page, unsigned int );

private:
  /**
   * If a file has been successfully created, open it
   */
  void openCreatedFile(const KDevCreateFile::CreatedFile & createdFile);

  /**
   * Reads in file type definitions from a config DOM and adds them
   * to the file type list. If enable is true, sets them all to
   * enabled=true by default.
   */
  int readTypes(const QDomDocument & dom, QPtrList<FileType> &m_filetypes, bool enable);

  /**
   * Add a file type
   */
  void addFileType(const QString & filename);

  /**
   * Finds the global XML file that has the same version as KDevelop
   */
  QString findGlobalXMLFile() const;

  /**
   * List of file types from which the user can choose
   */
  QPtrList<FileType> m_filetypes;

  /**
   * The file type selected by the new file dialog, if appropriate.
   */
  const FileType * m_filedialogFiletype;

  ConfigWidgetProxy * _configProxy;

  KPopupMenu* m_newPopupMenu;
  QPtrList<KPopupMenu>* m_subPopups;
};


#endif
