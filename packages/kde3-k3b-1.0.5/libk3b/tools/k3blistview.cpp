/*
 *
 * $Id: k3blistview.cpp 768493 2008-01-30 08:44:05Z trueg $
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



#include "k3blistview.h"

#include "k3bmsfedit.h"

#include <qstringlist.h>
#include <qfontmetrics.h>
#include <qpainter.h>
#include <qheader.h>
#include <qrect.h>
#include <qpushbutton.h>
#include <qiconset.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qevent.h>
#include <qvalidator.h>
#include <qfont.h>
#include <qpalette.h>
#include <qstyle.h>
#include <qapplication.h>
#include <qprogressbar.h>
#include <qimage.h>

#include <kpixmapeffect.h>

#include <limits.h>



// ///////////////////////////////////////////////
//
// K3BLISTVIEWITEM
//
// ///////////////////////////////////////////////


class K3bListViewItem::ColumnInfo
{
public:
  ColumnInfo()
    : showProgress(false),
      progressValue(0),
      totalProgressSteps(100),
      margin(0),
      validator(0) {
    editorType = NONE;
    button = false;
    comboEditable = false;
    next = 0;
    fontSet = false;
    backgroundColorSet = false;
    foregroundColorSet = false;
  }

  ~ColumnInfo() {
    if( next )
      delete next;
  }

  bool button;
  int editorType;
  QStringList comboItems;
  bool comboEditable;
  bool fontSet;
  bool backgroundColorSet;
  bool foregroundColorSet;
  QFont font;
  QColor backgroundColor;
  QColor foregroundColor;
  ColumnInfo* next;

  bool showProgress;
  int progressValue;
  int totalProgressSteps;
  int margin;

  QValidator* validator;
};



K3bListViewItem::K3bListViewItem(QListView *parent)
  : KListViewItem( parent )
{
  init();
}

K3bListViewItem::K3bListViewItem(QListViewItem *parent)
  : KListViewItem( parent )
{
  init();
}

K3bListViewItem::K3bListViewItem(QListView *parent, QListViewItem *after)
  : KListViewItem( parent, after )
{
  init();
}

K3bListViewItem::K3bListViewItem(QListViewItem *parent, QListViewItem *after)
  : KListViewItem( parent, after )
{
  init();
}


K3bListViewItem::K3bListViewItem(QListView *parent,
				 const QString& s1, const QString& s2,
				 const QString& s3, const QString& s4,
				 const QString& s5, const QString& s6,
				 const QString& s7, const QString& s8)
  : KListViewItem( parent, s1, s2, s3, s4, s5, s6, s7, s8 )
{
  init();
}


K3bListViewItem::K3bListViewItem(QListViewItem *parent,
				 const QString& s1, const QString& s2,
				 const QString& s3, const QString& s4,
				 const QString& s5, const QString& s6,
				 const QString& s7, const QString& s8)
  : KListViewItem( parent, s1, s2, s3, s4, s5, s6, s7, s8 )
{
  init();
}


K3bListViewItem::K3bListViewItem(QListView *parent, QListViewItem *after,
				 const QString& s1, const QString& s2,
				 const QString& s3, const QString& s4,
				 const QString& s5, const QString& s6,
				 const QString& s7, const QString& s8)
  : KListViewItem( parent, after, s1, s2, s3, s4, s5, s6, s7, s8 )
{
  init();
}


K3bListViewItem::K3bListViewItem(QListViewItem *parent, QListViewItem *after,
				 const QString& s1, const QString& s2,
				 const QString& s3, const QString& s4,
				 const QString& s5, const QString& s6,
				 const QString& s7, const QString& s8)
  : KListViewItem( parent, after, s1, s2, s3, s4, s5, s6, s7, s8 )
{
  init();
}


K3bListViewItem::~K3bListViewItem()
{
  if( K3bListView* lv = dynamic_cast<K3bListView*>(listView()) )
    if( lv->currentlyEditedItem() == this )
      lv->hideEditor();

  if( m_columns )
    delete m_columns;
}


void K3bListViewItem::init()
{
  m_columns = 0;
  m_vMargin = 0;
}


int K3bListViewItem::width( const QFontMetrics& fm, const QListView* lv, int c ) const
{
  return KListViewItem::width( fm, lv, c ) + getColumnInfo(c)->margin*2;
}


void K3bListViewItem::setEditor( int column, int editor, const QStringList& cs )
{
  ColumnInfo* colInfo = getColumnInfo(column);

  colInfo->editorType = editor;
  if( !cs.isEmpty() )
    colInfo->comboItems = cs;
}


void K3bListViewItem::setValidator( int column, QValidator* v )
{
  getColumnInfo(column)->validator = v;
}


QValidator* K3bListViewItem::validator( int col ) const
{
  return getColumnInfo(col)->validator;
}


void K3bListViewItem::setButton( int column, bool on )
{
  ColumnInfo* colInfo = getColumnInfo(column);

  colInfo->button = on;
}


K3bListViewItem::ColumnInfo* K3bListViewItem::getColumnInfo( int col ) const
{
  if( !m_columns )
    m_columns = new ColumnInfo();

  ColumnInfo* info = m_columns;
  int i = 0;
  while( i < col ) {
    if( !info->next )
      info->next = new ColumnInfo();
    info = info->next;
    ++i;
  }

  return info;
}


int K3bListViewItem::editorType( int col ) const
{
  ColumnInfo* info = getColumnInfo( col );
  return info->editorType;
}


bool K3bListViewItem::needButton( int col ) const
{
  ColumnInfo* info = getColumnInfo( col );
  return info->button;
}


const QStringList& K3bListViewItem::comboStrings( int col ) const
{
  ColumnInfo* info = getColumnInfo( col );
  return info->comboItems;
}


void K3bListViewItem::setFont( int col, const QFont& f )
{
  ColumnInfo* info = getColumnInfo( col );
  info->fontSet = true;
  info->font = f;
}


void K3bListViewItem::setBackgroundColor( int col, const QColor& c )
{
  ColumnInfo* info = getColumnInfo( col );
  info->backgroundColorSet = true;
  info->backgroundColor = c;
  repaint();
}


void K3bListViewItem::setForegroundColor( int col, const QColor& c )
{
 ColumnInfo* info = getColumnInfo( col );
 info->foregroundColorSet = true;
 info->foregroundColor = c;
 repaint();
}


void K3bListViewItem::setDisplayProgressBar( int col, bool displ )
{
  ColumnInfo* info = getColumnInfo( col );
  info->showProgress = displ;
}


void K3bListViewItem::setProgress( int col, int p )
{
  ColumnInfo* info = getColumnInfo( col );
  if( !info->showProgress )
    setDisplayProgressBar( col, true );
  if( info->progressValue != p ) {
    info->progressValue = p;
    repaint();
  }
}


void K3bListViewItem::setTotalSteps( int col, int steps )
{
  ColumnInfo* info = getColumnInfo( col );
  info->totalProgressSteps = steps;

  repaint();
}


void K3bListViewItem::setMarginHorizontal( int col, int margin )
{
  ColumnInfo* info = getColumnInfo( col );
  info->margin = margin;

  repaint();
}


void K3bListViewItem::setMarginVertical( int margin )
{
  m_vMargin = margin;
  repaint();
}


int K3bListViewItem::marginHorizontal( int col ) const
{
  return getColumnInfo( col )->margin;
}


int K3bListViewItem::marginVertical() const
{
  return m_vMargin;
}


void K3bListViewItem::setup()
{
  KListViewItem::setup();

  setHeight( height() + 2*m_vMargin );
}


void K3bListViewItem::paintCell( QPainter* p, const QColorGroup& cg, int col, int width, int align )
{
  ColumnInfo* info = getColumnInfo( col );

  p->save();

  QFont oldFont( p->font() );
  QFont newFont = info->fontSet ? info->font : oldFont;
  p->setFont( newFont );
  QColorGroup cgh(cg);
  if( info->foregroundColorSet )
    cgh.setColor( QColorGroup::Text, info->foregroundColor );
  if( info->backgroundColorSet )
    cgh.setColor( QColorGroup::Base, info->backgroundColor );

  // in case this is the selected row has a margin we need to repaint the selection bar
  if( isSelected() &&
      (col == 0 || listView()->allColumnsShowFocus()) &&
      info->margin > 0 ) {

    p->fillRect( 0, 0, info->margin, height(),
		 cgh.brush( QColorGroup::Highlight ) );
    p->fillRect( width-info->margin, 0, info->margin, height(),
		 cgh.brush( QColorGroup::Highlight ) );
  }
  else { // in case we use the KListView alternate color stuff
    p->fillRect( 0, 0, info->margin, height(),
		 cgh.brush( QColorGroup::Base ) );
    p->fillRect( width-info->margin, 0, info->margin, height(),
		 cgh.brush( QColorGroup::Base ) );
  }

  // FIXME: the margin (we can only translate horizontally since height() is used for painting)
  p->translate( info->margin, 0 );

  if( info->showProgress ) {
    paintProgressBar( p, cgh, col, width-2*info->margin );
  }
  else {
    paintK3bCell( p, cgh, col, width-2*info->margin, align );
  }

  p->restore();
}


void K3bListViewItem::paintK3bCell( QPainter* p, const QColorGroup& cg, int col, int width, int align )
{
  QListViewItem::paintCell( p, cg, col, width, align );
}


void K3bListViewItem::paintProgressBar( QPainter* p, const QColorGroup& cgh, int col, int width )
{
  ColumnInfo* info = getColumnInfo( col );

  QStyle::SFlags flags = QStyle::Style_Default;
  if( listView()->isEnabled() )
    flags |= QStyle::Style_Enabled;
  if( listView()->hasFocus() )
    flags |= QStyle::Style_HasFocus;

  // FIXME: the QPainter is translated so 0, m_vMargin is the upper left of our paint rect
  QRect r( 0, m_vMargin, width, height()-2*m_vMargin );

  // create the double buffer pixmap
  static QPixmap *doubleBuffer = 0;
  if( !doubleBuffer )
    doubleBuffer = new QPixmap;
  doubleBuffer->resize( width, height() );

  QPainter dbPainter( doubleBuffer );

  // clear the background (we cannot use paintEmptyArea since it's protected in QListView)
  if( K3bListView* lv = dynamic_cast<K3bListView*>(listView()) )
    lv->paintEmptyArea( &dbPainter, r );
  else
    dbPainter.fillRect( 0, 0, width, height(),
			cgh.brush( QPalette::backgroundRoleFromMode(listView()->viewport()->backgroundMode()) ) );

  // we want a little additional margin
  r.setLeft( r.left()+1 );
  r.setWidth( r.width()-2 );
  r.setTop( r.top()+1 );
  r.setHeight( r.height()-2 );

  // this might be a stupid hack but most styles do not reimplement drawPrimitive PE_ProgressBarChunk
  // so this way the user is happy....
  static QProgressBar* s_dummyProgressBar = 0;
  if( !s_dummyProgressBar ) {
    s_dummyProgressBar = new QProgressBar();
  }

  s_dummyProgressBar->setTotalSteps( info->totalProgressSteps );
  s_dummyProgressBar->setProgress( info->progressValue );

  // some styles use the widget's geometry
  s_dummyProgressBar->setGeometry( r );

  listView()->style().drawControl(QStyle::CE_ProgressBarContents, &dbPainter, s_dummyProgressBar, r, cgh, flags );
  listView()->style().drawControl(QStyle::CE_ProgressBarLabel, &dbPainter, s_dummyProgressBar, r, cgh, flags );

  // now we really paint the progress in the listview
  p->drawPixmap( 0, 0, *doubleBuffer );
}







K3bCheckListViewItem::K3bCheckListViewItem(QListView *parent)
  : K3bListViewItem( parent ),
    m_checked(false)
{
}


K3bCheckListViewItem::K3bCheckListViewItem(QListViewItem *parent)
  : K3bListViewItem( parent ),
    m_checked(false)
{
}


K3bCheckListViewItem::K3bCheckListViewItem(QListView *parent, QListViewItem *after)
  : K3bListViewItem( parent, after ),
    m_checked(false)
{
}


K3bCheckListViewItem::K3bCheckListViewItem(QListViewItem *parent, QListViewItem *after)
  : K3bListViewItem( parent, after ),
    m_checked(false)
{
}


bool K3bCheckListViewItem::isChecked() const
{
  return m_checked;
}


void K3bCheckListViewItem::setChecked( bool checked )
{
  m_checked = checked;
  repaint();
}


void K3bCheckListViewItem::paintK3bCell( QPainter* p, const QColorGroup& cg, int col, int width, int align )
{
  K3bListViewItem::paintK3bCell( p, cg, col, width, align );

  if( col == 0 ) {
    if( m_checked ) {
      QRect r( 0, marginVertical(), width, /*listView()->style().pixelMetric( QStyle::PM_CheckListButtonSize )*/height()-2*marginVertical() );

      QStyle::SFlags flags = QStyle::Style_Default;
      if( listView()->isEnabled() )
	flags |= QStyle::Style_Enabled;
      if( listView()->hasFocus() )
	flags |= QStyle::Style_HasFocus;
      if( isChecked() )
	flags |= QStyle::Style_On;
      else
	flags |= QStyle::Style_Off;

      listView()->style().drawPrimitive( QStyle::PE_CheckMark, p, r, cg, flags );
    }
  }
}






// ///////////////////////////////////////////////
//
// K3BLISTVIEW
//
// ///////////////////////////////////////////////


class K3bListView::Private
{
public:
  QLineEdit* spinBoxLineEdit;
  QLineEdit* msfEditLineEdit;
};


K3bListView::K3bListView( QWidget* parent, const char* name )
  : KListView( parent, name ),
    m_noItemVMargin( 20 ),
    m_noItemHMargin( 20 )
{
  d = new Private;

  connect( header(), SIGNAL( sizeChange( int, int, int ) ),
	   this, SLOT( updateEditorSize() ) );

  m_editorButton = 0;
  m_editorComboBox = 0;
  m_editorSpinBox = 0;
  m_editorLineEdit = 0;
  m_editorMsfEdit = 0;
  m_currentEditItem = 0;
  m_currentEditColumn = 0;
  m_doubleClickForEdit = true;
  m_lastClickedItem = 0;
}

K3bListView::~K3bListView()
{
  delete d;
}


QWidget* K3bListView::editor( K3bListViewItem::EditorType t ) const
{
  switch( t ) {
  case K3bListViewItem::COMBO:
    return m_editorComboBox;
  case K3bListViewItem::LINE:
    return m_editorLineEdit;
  case K3bListViewItem::SPIN:
    return m_editorSpinBox;
  case K3bListViewItem::MSF:
    return m_editorMsfEdit;
  default:
    return 0;
  }
}


void K3bListView::clear()
{
  hideEditor();
  KListView::clear();
}


void K3bListView::editItem( K3bListViewItem* item, int col )
{
  if( item == 0 )
    hideEditor();
  else if( item->isEnabled() ) {
    showEditor( item, col );
  }
}


void K3bListView::hideEditor()
{
  m_lastClickedItem = 0;
  m_currentEditItem = 0;
  m_currentEditColumn = 0;

  if( m_editorSpinBox )
    m_editorSpinBox->hide();
  if( m_editorLineEdit )
    m_editorLineEdit->hide();
  if( m_editorComboBox )
    m_editorComboBox->hide();
  if( m_editorButton )
    m_editorButton->hide();
  if( m_editorMsfEdit )
    m_editorMsfEdit->hide();
}

void K3bListView::showEditor( K3bListViewItem* item, int col )
{
  if( !item )
    return;

  if( item->needButton( col ) || item->editorType(col) != K3bListViewItem::NONE ) {
    m_currentEditColumn = col;
    m_currentEditItem = item;
  }

  placeEditor( item, col );
  if( item->needButton( col ) ) {
    m_editorButton->show();
  }
  switch( item->editorType(col) ) {
  case K3bListViewItem::COMBO:
    m_editorComboBox->show();
    m_editorComboBox->setFocus();
    m_editorComboBox->setValidator( item->validator(col) );
    break;
  case K3bListViewItem::LINE:
    m_editorLineEdit->show();
    m_editorLineEdit->setFocus();
    m_editorLineEdit->setValidator( item->validator(col) );
    break;
  case K3bListViewItem::SPIN:
    m_editorSpinBox->show();
    m_editorSpinBox->setFocus();
    break;
  case K3bListViewItem::MSF:
    m_editorMsfEdit->show();
    m_editorMsfEdit->setFocus();
    break;
  default:
    break;
  }
}


void K3bListView::placeEditor( K3bListViewItem* item, int col )
{
  ensureItemVisible( item );
  QRect r = itemRect( item );

  r.setX( contentsToViewport( QPoint(header()->sectionPos( col ), 0) ).x() );
  r.setWidth( header()->sectionSize( col ) - 1 );

  // check if the column is fully visible
  if( visibleWidth() < r.right() )
    r.setRight(visibleWidth());

  r = QRect( viewportToContents( r.topLeft() ), r.size() );

  if( item->pixmap( col ) ) {
    r.setX( r.x() + item->pixmap(col)->width() );
  }

  // the tree-stuff is painted in the first column
  if( col == 0 ) {
    r.setX( r.x() + item->depth() * treeStepSize() );
    if( rootIsDecorated() )
      r.setX( r.x() + treeStepSize() );
  }

  if( item->needButton(col) ) {
    prepareButton( item, col );
    m_editorButton->setFixedHeight( r.height() );
    // for now we make a square button
    m_editorButton->setFixedWidth( m_editorButton->height() );
    r.setWidth( r.width() - m_editorButton->width() );
    moveChild( m_editorButton, r.right(), r.y() );
  }

  if( QWidget* editor = prepareEditor( item, col ) ) {
    editor->resize( r.size() );
    //    editor->resize( QSize( r.width(), editor->minimumSizeHint().height() ) );
    moveChild( editor, r.x(), r.y() );
  }
}


void K3bListView::prepareButton( K3bListViewItem*, int )
{
  if( !m_editorButton ) {
    m_editorButton = new QPushButton( viewport() );
    connect( m_editorButton, SIGNAL(clicked()),
	     this, SLOT(slotEditorButtonClicked()) );
  }

  // TODO: do some useful things
  m_editorButton->setText( "..." );
}


QWidget* K3bListView::prepareEditor( K3bListViewItem* item, int col )
{
  switch( item->editorType(col) ) {
  case K3bListViewItem::COMBO:
    if( !m_editorComboBox ) {
      m_editorComboBox = new QComboBox( viewport() );
      connect( m_editorComboBox, SIGNAL(activated(const QString&)),
	       this, SLOT(slotEditorComboBoxActivated(const QString&)) );
      m_editorComboBox->installEventFilter( this );
    }
    m_editorComboBox->clear();
    if( item->comboStrings( col ).isEmpty() ) {
      m_editorComboBox->insertItem( item->text( col ) );
    }
    else {
      m_editorComboBox->insertStringList( item->comboStrings(col) );
      int current = item->comboStrings(col).findIndex( item->text(col) );
      if( current != -1 )
	m_editorComboBox->setCurrentItem( current );
    }
    return m_editorComboBox;

  case K3bListViewItem::LINE: {
    if( !m_editorLineEdit ) {
      m_editorLineEdit = new QLineEdit( viewport() );
      m_editorLineEdit->setFrameStyle( QFrame::Box | QFrame::Plain );
      m_editorLineEdit->setLineWidth(1);
      m_editorLineEdit->installEventFilter( this );
    }

    QString txt = item->text( col );
    m_editorLineEdit->setText( txt );

    // select the edit text (handle extensions while doing so)
    int pos = txt.findRev( '.' );
    if( pos > 0 )
      m_editorLineEdit->setSelection( 0, pos );
    else
      m_editorLineEdit->setSelection( 0, txt.length() );

    return m_editorLineEdit;
  }

  //
  // A QSpinBox (and thus also a K3bMsfEdit) uses a QLineEdit), thus
  // we have to use this QLineEdit as the actual object to dead with
  //

  case K3bListViewItem::SPIN:
    if( !m_editorSpinBox ) {
      m_editorSpinBox = new QSpinBox( viewport() );
      d->spinBoxLineEdit = static_cast<QLineEdit*>( m_editorSpinBox->child( 0, "QLineEdit" ) );
      connect( m_editorSpinBox, SIGNAL(valueChanged(int)),
	       this, SLOT(slotEditorSpinBoxValueChanged(int)) );
      //      m_editorSpinBox->installEventFilter( this );
      d->spinBoxLineEdit->installEventFilter( this );
    }
    // set the range
    m_editorSpinBox->setValue( item->text(col).toInt() );
    return m_editorSpinBox;

  case K3bListViewItem::MSF:
    if( !m_editorMsfEdit ) {
      m_editorMsfEdit = new K3bMsfEdit( viewport() );
      d->msfEditLineEdit = static_cast<QLineEdit*>( m_editorMsfEdit->child( 0, "QLineEdit" ) );
      connect( m_editorMsfEdit, SIGNAL(valueChanged(int)),
	       this, SLOT(slotEditorMsfEditValueChanged(int)) );
      //      m_editorMsfEdit->installEventFilter( this );
      d->msfEditLineEdit->installEventFilter( this );
    }
    m_editorMsfEdit->setText( item->text(col) );
    return m_editorMsfEdit;

  default:
    return 0;
  }
}

void K3bListView::setCurrentItem( QListViewItem* i )
{
  if( !i || i == currentItem() )
    return;

  // I cannot remember why I did this here exactly. However, it resets the
  // m_lastClickedItem and thus invalidates the editing.
//   doRename();
//   hideEditor();
//   m_currentEditItem = 0;
  KListView::setCurrentItem( i );
}


void K3bListView::setNoItemText( const QString& text )
{
  m_noItemText = text;
  triggerUpdate();
}


void K3bListView::viewportPaintEvent( QPaintEvent* e )
{
  KListView::viewportPaintEvent( e );
}


// FIXME: move this to viewportPaintEvent
void K3bListView::drawContentsOffset( QPainter * p, int ox, int oy, int cx, int cy, int cw, int ch )
{
  KListView::drawContentsOffset( p, ox, oy, cx, cy, cw, ch );

  if( childCount() == 0 && !m_noItemText.isEmpty()) {

    p->setPen( Qt::darkGray );

    QStringList lines = QStringList::split( "\n", m_noItemText );
    int xpos = m_noItemHMargin;
    int ypos = m_noItemVMargin + p->fontMetrics().height();

    QStringList::Iterator end ( lines.end() );
    for( QStringList::Iterator str = lines.begin(); str != end; ++str ) {
      p->drawText( xpos, ypos, *str );
      ypos += p->fontMetrics().lineSpacing();
    }
  }
}

void K3bListView::paintEmptyArea( QPainter* p, const QRect& rect )
{
  KListView::paintEmptyArea( p, rect );

//   if( childCount() == 0 && !m_noItemText.isEmpty()) {

//     QPainter pp( viewport() );
//     pp.fillRect( viewport()->rect(), viewport()->paletteBackgroundColor() );
//     pp.end();

//     p->setPen( Qt::darkGray );

//     QStringList lines = QStringList::split( "\n", m_noItemText );
//     int xpos = m_noItemHMargin;
//     int ypos = m_noItemVMargin + p->fontMetrics().height();

//     for( QStringList::Iterator str = lines.begin(); str != lines.end(); str++ ) {
//       p->drawText( xpos, ypos, *str );
//       ypos += p->fontMetrics().lineSpacing();
//  }
//   }
}

void K3bListView::resizeEvent( QResizeEvent* e )
{
  KListView::resizeEvent( e );
  updateEditorSize();
}


void K3bListView::updateEditorSize()
{
  if( m_currentEditItem )
    placeEditor( m_currentEditItem, m_currentEditColumn );
}


void K3bListView::slotEditorLineEditReturnPressed()
{
  if( doRename() ) {
    // edit the next line
    // TODO: add config for this
    if( K3bListViewItem* nextItem = dynamic_cast<K3bListViewItem*>( m_currentEditItem->nextSibling() ) )
      editItem( nextItem, currentEditColumn() );
    else {
      hideEditor();

      // keep the focus here
      viewport()->setFocus();
    }
  }
}


void K3bListView::slotEditorComboBoxActivated( const QString& )
{
  doRename();
//   if( renameItem( m_currentEditItem, m_currentEditColumn, str ) ) {
//     m_currentEditItem->setText( m_currentEditColumn, str );
//     emit itemRenamed( m_currentEditItem, str, m_currentEditColumn );
//   }
//   else {
//     for( int i = 0; i < m_editorComboBox->count(); ++i ) {
//       if( m_editorComboBox->text(i) == m_currentEditItem->text(m_currentEditColumn) ) {
// 	m_editorComboBox->setCurrentItem( i );
// 	break;
//       }
//     }
//   }
}


void K3bListView::slotEditorSpinBoxValueChanged( int )
{
//   if( renameItem( m_currentEditItem, m_currentEditColumn, QString::number(value) ) ) {
//     m_currentEditItem->setText( m_currentEditColumn, QString::number(value) );
//     emit itemRenamed( m_currentEditItem, QString::number(value), m_currentEditColumn );
//   }
//   else
//     m_editorSpinBox->setValue( m_currentEditItem->text( m_currentEditColumn ).toInt() );
}


void K3bListView::slotEditorMsfEditValueChanged( int )
{
  // FIXME: do we always need to update the value. Isn't it enough to do it at the end?
//   if( renameItem( m_currentEditItem, m_currentEditColumn, QString::number(value) ) ) {
//     m_currentEditItem->setText( m_currentEditColumn, QString::number(value) );
//     emit itemRenamed( m_currentEditItem, QString::number(value), m_currentEditColumn );
//   }
//   else
//     m_editorMsfEdit->setText( m_currentEditItem->text( m_currentEditColumn ) );
}


bool K3bListView::doRename()
{
  if( m_currentEditItem ) {
    QString newValue;
    switch( m_currentEditItem->editorType( m_currentEditColumn ) ) {
    case K3bListViewItem::COMBO:
      newValue = m_editorComboBox->currentText();
      break;
    case K3bListViewItem::LINE:
      newValue = m_editorLineEdit->text();
      break;
    case K3bListViewItem::SPIN:
      newValue = QString::number(m_editorSpinBox->value());
      break;
    case K3bListViewItem::MSF:
      newValue = QString::number(m_editorMsfEdit->value());
      break;
    }

    if( renameItem( m_currentEditItem, m_currentEditColumn, newValue ) ) {
      m_currentEditItem->setText( m_currentEditColumn, newValue );
      emit itemRenamed( m_currentEditItem, newValue, m_currentEditColumn );
      return true;
    }
    else {
      switch( m_currentEditItem->editorType( m_currentEditColumn ) ) {
      case K3bListViewItem::COMBO:
	for( int i = 0; i < m_editorComboBox->count(); ++i ) {
	  if( m_editorComboBox->text(i) == m_currentEditItem->text(m_currentEditColumn) ) {
	    m_editorComboBox->setCurrentItem( i );
	    break;
	  }
	}
	break;
      case K3bListViewItem::LINE:
	m_editorLineEdit->setText( m_currentEditItem->text( m_currentEditColumn ) );
	break;
      case K3bListViewItem::SPIN:
	m_editorSpinBox->setValue( m_currentEditItem->text( m_currentEditColumn ).toInt() );
	break;
      case K3bListViewItem::MSF:
	m_editorMsfEdit->setText( m_currentEditItem->text( m_currentEditColumn ) );
	break;
      }
    }
  }


  return false;
}


void K3bListView::slotEditorButtonClicked()
{
  slotEditorButtonClicked( m_currentEditItem, m_currentEditColumn );
}


bool K3bListView::renameItem( K3bListViewItem* item, int col, const QString& text )
{
  Q_UNUSED(item);
  Q_UNUSED(col);
  Q_UNUSED(text);
  return true;
}


void K3bListView::slotEditorButtonClicked( K3bListViewItem* item, int col )
{
  emit editorButtonClicked( item, col );
}


bool K3bListView::eventFilter( QObject* o, QEvent* e )
{
  if( e->type() == QEvent::KeyPress ) {
     QKeyEvent* ke = static_cast<QKeyEvent*>(e);
     if( ke->key() == Key_Tab ) {
       if( o == m_editorLineEdit ||
	   o == d->msfEditLineEdit ||
	   o == d->spinBoxLineEdit ) {
	 K3bListViewItem* lastEditItem = m_currentEditItem;

	 doRename();

	 if( lastEditItem ) {
	   // can we rename one of the other columns?
	   int col = currentEditColumn()+1;
	   while( col < columns() && lastEditItem->editorType( col ) == K3bListViewItem::NONE )
	     ++col;
	   if( col < columns() )
	     editItem( lastEditItem, col );
	   else {
	     hideEditor();

	     // keep the focus here
	     viewport()->setFocus();

	     // search for the next editable item
	     while( K3bListViewItem* nextItem =
		    dynamic_cast<K3bListViewItem*>( lastEditItem->nextSibling() ) ) {
	       // edit first column
	       col = 0;
	       while( col < columns() && nextItem->editorType( col ) == K3bListViewItem::NONE )
		 ++col;
	       if( col < columns() ) {
		 editItem( nextItem, col );
		 break;
	       }

	       lastEditItem = nextItem;
	     }
	   }
	 }

	 return true;
       }
     }
     if( ke->key() == Key_Return ||
         ke->key() == Key_Enter ) {
       if( o == m_editorLineEdit ||
	   o == d->msfEditLineEdit ||
	   o == d->spinBoxLineEdit ) {
	 K3bListViewItem* lastEditItem = m_currentEditItem;
	 doRename();

	 if( K3bListViewItem* nextItem =
	     dynamic_cast<K3bListViewItem*>( lastEditItem->nextSibling() ) )
	   editItem( nextItem, currentEditColumn() );
	 else {
	   hideEditor();

	   // keep the focus here
	   viewport()->setFocus();
	 }

	 return true;
       }
     }
     else if( ke->key() == Key_Escape ) {
       if( o == m_editorLineEdit ||
	   o == d->msfEditLineEdit ||
	   o == d->spinBoxLineEdit ) {
	 hideEditor();

	 // keep the focus here
	 viewport()->setFocus();

	 return true;
       }
     }
  }

  else if( e->type() == QEvent::MouseButtonPress && o == viewport() ) {

    // first let's grab the focus
    viewport()->setFocus();

    QMouseEvent* me = static_cast<QMouseEvent*>( e );
    QListViewItem* item = itemAt( me->pos() );
    int col = header()->sectionAt( me->pos().x() );
    if( K3bCheckListViewItem* ci = dynamic_cast<K3bCheckListViewItem*>( item ) ) {
      if( col == 0 ) {
	// FIXME: improve this click area!
	ci->setChecked( !ci->isChecked() );
	return true;
      }
    }

    if( me->button() == QMouseEvent::LeftButton ) {
      if( item != m_currentEditItem || m_currentEditColumn != col ) {
	doRename();
	if( K3bListViewItem* k3bItem = dynamic_cast<K3bListViewItem*>(item) ) {
	  if( me->pos().x() > item->depth()*treeStepSize() &&
	      item->isEnabled() &&
	      (m_lastClickedItem == item || !m_doubleClickForEdit) )
	    showEditor( k3bItem, col );
	  else {
	    hideEditor();

	    // keep the focus here
	    viewport()->setFocus();
	  }
	}
	else {
	  hideEditor();

	  // keep the focus here
	  viewport()->setFocus();
	}

	// do not count clicks in the item tree for editing
	if( item && me->pos().x() > item->depth()*treeStepSize() )
	  m_lastClickedItem = item;
      }
    }
  }

  else if( e->type() == QEvent::FocusOut ) {
    if( o == m_editorLineEdit ||
	o == d->msfEditLineEdit ||
	o == d->spinBoxLineEdit ||
	o == m_editorComboBox ) {
      // make sure we did not lose the focus to one of the edit widgets' children
      if( !qApp->focusWidget() || qApp->focusWidget()->parentWidget() != o ) {
	doRename();
	hideEditor();
      }
    }
  }

  return KListView::eventFilter( o, e );
}


void K3bListView::setK3bBackgroundPixmap( const QPixmap& pix, int pos )
{
  m_backgroundPixmap = pix;
  m_backgroundPixmapPosition = pos;
}


void K3bListView::viewportResizeEvent( QResizeEvent* e )
{
  if( !m_backgroundPixmap.isNull() ) {

    QSize size = viewport()->size().expandedTo( QSize( contentsWidth(), contentsHeight() ) );

    QPixmap bgPix( size );

    // FIXME: let the user specify the color
    bgPix.fill( colorGroup().base() );

    if( bgPix.width() < m_backgroundPixmap.width() ||
	bgPix.height() < m_backgroundPixmap.height() ) {
      QPixmap newBgPix( m_backgroundPixmap.convertToImage().scale( bgPix.size(), QImage::ScaleMin ) );
      if( m_backgroundPixmapPosition == TOP_LEFT )
	bitBlt( &bgPix, 0, 0,
		&newBgPix, 0, 0,
		newBgPix.width(), newBgPix.height() );
      else {
	int dx = bgPix.width() / 2 - m_backgroundPixmap.width() /2;
	int dy = bgPix.height() / 2 - m_backgroundPixmap.height() /2;
	bitBlt( &bgPix, dx, dy, &newBgPix, 0, 0,
		newBgPix.width(), newBgPix.height() );
      }
    }
    else {
      if( m_backgroundPixmapPosition == TOP_LEFT )
	bitBlt( &bgPix, 0, 0,
		&m_backgroundPixmap, 0, 0,
		m_backgroundPixmap.width(), m_backgroundPixmap.height() );
      else {
	int dx = bgPix.width() / 2 - m_backgroundPixmap.width() /2;
	int dy = bgPix.height() / 2 - m_backgroundPixmap.height() /2;
	bitBlt( &bgPix, dx, dy, &m_backgroundPixmap, 0, 0,
		m_backgroundPixmap.width(), m_backgroundPixmap.height() );
      }
    }

    viewport()->setPaletteBackgroundPixmap( bgPix );
  }

  KListView::viewportResizeEvent( e );
}


QListViewItem* K3bListView::parentItem( QListViewItem* item )
{
  if( !item )
    return 0;
  if( item->parent() )
    return item->parent();
  else
    return K3bListView::parentItem( item->itemAbove() );
}


KPixmap K3bListView::createDragPixmap( const QPtrList<QListViewItem>& items )
{
  //
  // Create drag pixmap.
  // If there are too many items fade the pixmap using the mask
  // always fade invisible items
  //
  int width = header()->width();
  int height = 0;
  for( QPtrListIterator<QListViewItem> it( items ); *it; ++it ) {
    QRect r = itemRect( *it );

    if( r.isValid() ) {
      height += ( *it )->height();
    }
  }

  // now we should have a range top->bottom which includes all visible items

  // there are two situations in which we fade the pixmap on the top or the bottom:
  // 1. there are invisible items above (below) the visible
  // 2. the range is way too big

  // FIXME: how do we determine if there are invisible items outside our range?

  KPixmap pix;
  pix.resize( width, height );
  pix.fill( Qt::white );
  //  QBitmap mask( width, bottom-top );

  // now paint all the visible items into the pixmap
  // FIXME: only paint the visible items
  QPainter p( &pix );
  for( QListViewItemIterator it( this ); *it; ++it ) {
    QListViewItem* item = *it;

    // FIXME: items on other than the top level have a smaller first column
    //        the same goes for all items if root is decorated
    bool alreadyDrawing = false;
    QRect r = itemRect( item );
    if( r.isValid() ) {
      if( items.containsRef( item ) ) {
	// paint all columns
	int x = 0;
	for( int i = 0; i < columns(); ++i ) {
	  item->paintCell( &p, colorGroup(), i, columnWidth( i ), columnAlignment( i ) );
	  p.translate( columnWidth( i ), 0 );
	  x += columnWidth( i );
	}

	p.translate( -x, item->height() );

	alreadyDrawing = true;
      }
      else if( alreadyDrawing )
	p.translate( 0, item->height() );

      if( p.worldMatrix().dy() >= pix.height() )
	break;
    }
  }

  // make it a little lighter
  KPixmapEffect::fade( pix, 0.3, Qt::white );

  // FIXME: fade the pixmap at the right side if the items are longer than width

  return pix;
}

#include "k3blistview.moc"
