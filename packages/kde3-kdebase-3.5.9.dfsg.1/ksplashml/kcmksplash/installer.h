/***************************************************************************
 *   Copyright Ravikiran Rajagopal 2003                                    *
 *   ravi@ee.eng.ohio-state.edu                                            *
 *   Copyright (c) 1998 Stefan Taferner <taferner@kde.org>                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License (version 2) as   *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 ***************************************************************************/
#ifndef SPLASHINSTALLER_H
#define SPLASHINSTALLER_H

#include <qmap.h>
#include <qpoint.h>

#include <klistbox.h>
#include <kurl.h>

class QLabel;
class QTextEdit;
class QPushButton;
class ThemeListBox;

class SplashInstaller : public QWidget
{
  Q_OBJECT
public:
  SplashInstaller(QWidget *parent=0, const char *aName=0, bool aInit=FALSE);
  ~SplashInstaller();

  virtual void load();
  virtual void load( bool useDefaults );
  virtual void save();
  virtual void defaults();

signals:
  void changed( bool state );

protected slots:
  virtual void slotAdd();
  virtual void slotRemove();
  virtual void slotTest();
  virtual void slotSetTheme(int);
  void slotFilesDropped(const KURL::List &urls);

protected:
  /** Scan Themes directory for available theme packages */
  virtual void readThemesList();
  /** add a theme to the list, returns the list index */
  int addTheme(const QString &path, const QString &name);
  void addNewTheme(const KURL &srcURL);
  int findTheme( const QString &theme );

private:
  bool mGui;
  ThemeListBox *mThemesList;
  QPushButton *mBtnAdd, *mBtnRemove, *mBtnTest;
  QTextEdit *mText;
  QLabel *mPreview;
};

class ThemeListBox: public KListBox
{
  Q_OBJECT
public:
  ThemeListBox(QWidget *parent);
  QMap<QString, QString> text2path;

signals:
  void filesDropped(const KURL::List &urls);

protected:
  void dragEnterEvent(QDragEnterEvent* event);
  void dropEvent(QDropEvent* event);
  void mouseMoveEvent(QMouseEvent *e);

protected slots:
  void slotMouseButtonPressed(int button, QListBoxItem *item, const QPoint &p);

private:
  QString mDragFile;
  QPoint mOldPos;

};

#endif
