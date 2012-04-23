/* 
 *
 * $Id: k3blistview.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef K3BLISTVIEW_H
#define K3BLISTVIEW_H


#include <klistview.h>
#include "k3b_export.h"
#include <qptrvector.h>
#include <qptrlist.h>
#include <qstringlist.h>
#include <kpixmap.h>

class QPainter;
class QPushButton;
class QIconSet;
class QResizeEvent;
class QComboBox;
class QSpinBox;
class QLineEdit;
class QEvent;
class QValidator;
class K3bMsfEdit;

class K3bListView;


class LIBK3B_EXPORT K3bListViewItem : public KListViewItem
{
 public:
  K3bListViewItem(QListView *parent);
  K3bListViewItem(QListViewItem *parent);
  K3bListViewItem(QListView *parent, QListViewItem *after);
  K3bListViewItem(QListViewItem *parent, QListViewItem *after);

  K3bListViewItem(QListView *parent,
		  const QString&, const QString& = QString::null,
		  const QString& = QString::null, const QString& = QString::null,
		  const QString& = QString::null, const QString& = QString::null,
		  const QString& = QString::null, const QString& = QString::null);

  K3bListViewItem(QListViewItem *parent,
		  const QString&, const QString& = QString::null,
		  const QString& = QString::null, const QString& = QString::null,
		  const QString& = QString::null, const QString& = QString::null,
		  const QString& = QString::null, const QString& = QString::null);

  K3bListViewItem(QListView *parent, QListViewItem *after,
		  const QString&, const QString& = QString::null,
		  const QString& = QString::null, const QString& = QString::null,
		  const QString& = QString::null, const QString& = QString::null,
		  const QString& = QString::null, const QString& = QString::null);

  K3bListViewItem(QListViewItem *parent, QListViewItem *after,
		  const QString&, const QString& = QString::null,
		  const QString& = QString::null, const QString& = QString::null,
		  const QString& = QString::null, const QString& = QString::null,
		  const QString& = QString::null, const QString& = QString::null);

  virtual ~K3bListViewItem();

  /**
   * reimplemented from KListViewItem
   */
  void setup();

  virtual int width( const QFontMetrics& fm, const QListView* lv, int c ) const;

  void setEditor( int col, int type, const QStringList& = QStringList() );
  void setButton( int col, bool );

  void setValidator( int col, QValidator* v );
  QValidator* validator( int col ) const;

  int editorType( int col ) const;
  bool needButton( int col ) const;
  const QStringList& comboStrings( int col ) const;

  enum EditorType { NONE, COMBO, LINE, SPIN, MSF };

  void setFont( int col, const QFont& f );
  void setBackgroundColor( int col, const QColor& );
  void setForegroundColor( int col, const QColor& );

  void setDisplayProgressBar( int col, bool );
  void setProgress( int, int );
  void setTotalSteps( int col, int steps );

  /**
   * The margin left and right of the cell
   */
  void setMarginHorizontal( int col, int margin );

  /**
   * The top and button margin of the cell
   */
  void setMarginVertical( int margin );

  int marginHorizontal( int col ) const;
  int marginVertical() const;

  /**
   * Do not reimplement this but paintK3bCell to use the margin and background stuff.
   */
  virtual void paintCell( QPainter* p, const QColorGroup& cg, int col, int width, int align );

  virtual void paintK3bCell( QPainter* p, const QColorGroup& cg, int col, int width, int align );

 private:
  void paintProgressBar( QPainter* p, const QColorGroup& cgh, int col, int width );

  class ColumnInfo;
  mutable ColumnInfo* m_columns;

  ColumnInfo* getColumnInfo( int ) const;
  void init();

  int m_vMargin;
};


class LIBK3B_EXPORT K3bCheckListViewItem : public K3bListViewItem
{
 public:
  K3bCheckListViewItem(QListView *parent);
  K3bCheckListViewItem(QListViewItem *parent);
  K3bCheckListViewItem(QListView *parent, QListViewItem *after);
  K3bCheckListViewItem(QListViewItem *parent, QListViewItem *after);

  virtual bool isChecked() const;
  virtual void setChecked( bool checked );

 protected:
  virtual void paintK3bCell( QPainter* p, const QColorGroup& cg, int col, int width, int align );

 private:
  bool m_checked;
};



class LIBK3B_EXPORT K3bListView : public KListView
{
  friend class K3bListViewItem;

  Q_OBJECT

 public:
  K3bListView (QWidget *parent = 0, const char *name = 0);
  virtual ~K3bListView();

  virtual void setCurrentItem( QListViewItem* );

  K3bListViewItem* currentlyEditedItem() const { return m_currentEditItem; }

  QWidget* editor( K3bListViewItem::EditorType ) const;

  enum BgPixPosition {
    TOP_LEFT,
    CENTER 
  };

  /**
   * This will set a background pixmap which is not tiled.
   * @param pos position on the viewport.
   */
  void setK3bBackgroundPixmap( const QPixmap&, int pos = CENTER );

  /**
   * Create a faded pixmap showing the items.
   */
  KPixmap createDragPixmap( const QPtrList<QListViewItem>& items );

  /**
   * Searches for the first item above @p i which is one level higher.
   * For 1st level items this will always be the listview's root item.
   */
  static QListViewItem* parentItem( QListViewItem* i );

 signals:
  void editorButtonClicked( K3bListViewItem*, int );

 public slots:
  void setNoItemText( const QString& );
  //  void setNoItemPixmap( const QPixmap& );
  void setNoItemVerticalMargin( int i ) { m_noItemVMargin = i; }
  void setNoItemHorizontalMargin( int i ) { m_noItemHMargin = i; }
  void setDoubleClickForEdit( bool b ) { m_doubleClickForEdit = b; }
  void hideEditor();
  void editItem( K3bListViewItem*, int );

  virtual void clear();

 private slots:
  void updateEditorSize();
  virtual void slotEditorLineEditReturnPressed();
  virtual void slotEditorComboBoxActivated( const QString& );
  virtual void slotEditorSpinBoxValueChanged( int );
  virtual void slotEditorMsfEditValueChanged( int );
  virtual void slotEditorButtonClicked();

 protected slots:
  void showEditor( K3bListViewItem*, int col );
  void placeEditor( K3bListViewItem*, int col );

  /**
   * This is called whenever one of the editor's contents changes
   * the default implementation just returnes true
   *
   * FIXME: should be called something like mayRename
   */
  virtual bool renameItem( K3bListViewItem*, int, const QString& );

  /**
   * default impl just emits signal
   * editorButtonClicked(...)
   */
  virtual void slotEditorButtonClicked( K3bListViewItem*, int );

 protected:
  /**
   * calls KListView::drawContentsOffset
   * and paints a the noItemText if no item is in the list
   */
  virtual void drawContentsOffset ( QPainter * p, int ox, int oy, int cx, int cy, int cw, int ch );
  virtual void resizeEvent( QResizeEvent* );
  virtual void paintEmptyArea( QPainter*, const QRect& rect );

  /**
   * Reimplemented for internal reasons.
   *
   * Further reimplementations should call this function or else some features may not work correctly.
   *
   * The API is unaffected.
   */
  virtual void viewportResizeEvent( QResizeEvent* );

  /**
   * Reimplemented for internal reasons.
   * Further reimplementations should call this function or else
   * some features may not work correctly.
   *
   * The API is unaffected.
   */
  virtual void viewportPaintEvent(QPaintEvent*);

  virtual bool eventFilter( QObject*, QEvent* );

  K3bListViewItem* currentEditItem() const { return m_currentEditItem; }
  int currentEditColumn() const { return m_currentEditColumn; }

 private:
  QWidget* prepareEditor( K3bListViewItem* item, int col );
  void prepareButton( K3bListViewItem* item, int col );
  bool doRename();

  QString m_noItemText;
  //  QPixmap m_noItemPixmap;
  int m_noItemVMargin;
  int m_noItemHMargin;

  K3bListViewItem* m_currentEditItem;
  int m_currentEditColumn;

  bool m_doubleClickForEdit;
  QListViewItem* m_lastClickedItem;

  QPushButton* m_editorButton;
  QComboBox* m_editorComboBox;
  QSpinBox* m_editorSpinBox;
  QLineEdit* m_editorLineEdit;
  K3bMsfEdit* m_editorMsfEdit;

  QPixmap m_backgroundPixmap;
  int m_backgroundPixmapPosition;

  class Private;
  Private* d;
};


#endif
