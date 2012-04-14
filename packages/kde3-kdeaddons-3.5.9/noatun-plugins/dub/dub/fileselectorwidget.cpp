/****************************************************************************
 *   Copyright (C) 2001 by Hugo Varotto										*
 *   hugo@varotto-usa.com													*
 *																			*
 *	 Based on Kate's fileselector widget	by									*
 *          Matt Newell														*
 *			(C) 2001 by Matt Newell											*
 *			newellm@proaxis.com												*
 *																			*
 *   This program is free software; you can redistribute it and/or modify		*
 *   it under the terms of the GNU General Public License as published by		*
 *   the Free Software Foundation; either version 2 of the License, or			*
 *   (at your option) any later version										*
 *																			*
 ***************************************************************************/

#include <qlayout.h>
#include <qpushbutton.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qstrlist.h>
#include <qtooltip.h>

#include <kiconloader.h>
#include <kurlcombobox.h>
#include <kurlcompletion.h>
#include <kprotocolinfo.h>
#include <kdiroperator.h>
#include <kconfig.h>
#include <klocale.h>
#include <kcombobox.h>

#include <kdebug.h>

//#include "fileselector_part.h"
#include "fileselectorwidget.h"


FileSelectorWidget::FileSelectorWidget(QWidget *parent)
    : QWidget(parent, "file selector widget")
{

	// widgets and layout

	QVBoxLayout* lo = new QVBoxLayout(this);

	QHBox *hlow = new QHBox (this);
	lo->addWidget(hlow);

	home = new QPushButton( hlow );
	home->setPixmap(SmallIcon("gohome"));
	QToolTip::add(home, i18n("Home folder"));
	up = new QPushButton( /*i18n("&Up"),*/ hlow );
	up->setPixmap(SmallIcon("up"));
	QToolTip::add(up, i18n("Up one level"));
	back = new QPushButton( /*i18n("&Back"),*/ hlow );
	back->setPixmap(SmallIcon("back"));
	QToolTip::add(back, i18n("Previous folder"));
	forward = new QPushButton( /*i18n("&Next"),*/ hlow );
	forward->setPixmap(SmallIcon("forward"));
	QToolTip::add(forward, i18n("Next folder"));

	// HACK
	QWidget* spacer = new QWidget(hlow);
	hlow->setStretchFactor(spacer, 1);
	hlow->setMaximumHeight(up->height());

	cmbPath = new KURLComboBox( KURLComboBox::Directories, true, this, "path combo" );
	cmbPath->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ));
	KURLCompletion* cmpl = new KURLCompletion();
	cmbPath->setCompletionObject( cmpl );
	lo->addWidget(cmbPath);

	dir = new KDirOperator(QString::null, this, "operator");
	dir->setView(KFile::Detail);
	lo->addWidget(dir);
	lo->setStretchFactor(dir, 2);

	QHBox* filterBox = new QHBox(this);
	filterIcon = new QLabel(filterBox);
	filterIcon->setPixmap( BarIcon("filter") );
	filter = new KHistoryCombo(filterBox, "filter");
	filter->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ));
	filterBox->setStretchFactor(filter, 2);
	lo->addWidget(filterBox);

	// slots and signals

	connect( filter, SIGNAL( activated(const QString&) ), SLOT( slotFilterChange(const QString&) ) );
	connect( filter, SIGNAL( returnPressed(const QString&) ),filter, SLOT( addToHistory(const QString&) ) );

	connect( home, SIGNAL( clicked() ), dir, SLOT( home() ) );
	connect( up, SIGNAL( clicked() ), dir, SLOT( cdUp() ) );
	connect( back, SIGNAL( clicked() ), dir, SLOT( back() ) );
	connect( forward, SIGNAL( clicked() ), dir, SLOT( forward() ) );

	connect( cmbPath, SIGNAL( urlActivated( const KURL&  )),
				this,  SLOT( cmbPathActivated( const KURL& ) ));
	connect( cmbPath, SIGNAL( returnPressed( const QString&  )),
				this,  SLOT( cmbPathReturnPressed( const QString& ) ));
	connect(dir, SIGNAL(urlEntered(const KURL&)),
				this, SLOT(dirUrlEntered(const KURL&)) );

	connect(dir, SIGNAL(finishedLoading()),
				this, SLOT(dirFinishedLoading()) );


    connect(dir, SIGNAL(fileHighlighted(const KFileItem *)),
                SLOT(fileHighlighted(const KFileItem *)));
    connect(dir, SIGNAL(fileSelected(const KFileItem *)),
                SLOT(fileSelected(const KFileItem *)));

    kdDebug(90010) << "connected stuff!" << endl;
}


FileSelectorWidget::~FileSelectorWidget()
{}

KURL FileSelectorWidget::currentDirectory()
{
  return dirLister()->url();
}

void FileSelectorWidget::slotFilterChange( const QString & nf )
{
  dir->setNameFilter( nf );
  dir->rereadDir();
}

void FileSelectorWidget::cmbPathActivated( const KURL& u )
{
   dir->setURL( u, true );
}

void FileSelectorWidget::cmbPathReturnPressed( const QString& u )
{
   dir->setFocus();
   dir->setURL( KURL(u), true );
}


void FileSelectorWidget::dirUrlEntered( const KURL& u )
{
   cmbPath->removeURL( u );
   QStringList urls = cmbPath->urls();
   urls.prepend( u.url() );
   while ( urls.count() >= (uint)cmbPath->maxItems() )
      urls.remove( urls.last() );
   cmbPath->setURLs( urls );
}


void FileSelectorWidget::dirFinishedLoading()
{
   // HACK - enable the nav buttons
   // have to wait for diroperator...
   up->setEnabled( dir->actionCollection()->action( "up" )->isEnabled() );
   back->setEnabled( dir->actionCollection()->action( "back" )->isEnabled() );
   forward->setEnabled( dir->actionCollection()->action( "forward" )->isEnabled() );
   home->setEnabled( dir->actionCollection()->action( "home" )->isEnabled() );
}


void FileSelectorWidget::focusInEvent(QFocusEvent*)
{
   dir->setFocus();
}

void FileSelectorWidget::setDir( KURL u )
{
  dir->setURL(u, true);
}

void FileSelectorWidget::fileHighlighted(const KFileItem *) {
  kdDebug(90010) << "file highlighted!" << endl;
}

void FileSelectorWidget::fileSelected(const KFileItem * ) {
  kdDebug(90010) << "file selected!" << endl;
}
#include "fileselectorwidget.moc"

