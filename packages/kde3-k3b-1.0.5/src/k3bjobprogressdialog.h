/* 
 *
 * $Id: k3bjobprogressdialog.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef _K3B_JOB_PROGRESSDIALOG_H_
#define _K3B_JOB_PROGRESSDIALOG_H_

#include <kdialog.h>

#include "k3bdebuggingoutputfile.h"

#include <k3bjobhandler.h>

#include <qdatetime.h>
#include <qfile.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class KListView;
class QFrame;
class QGroupBox;
class QLabel;
class QListViewItem;
class KProgress;
class QPushButton;
class QTimer;
class K3bJob;
class KCutLabel;
class QCloseEvent;
class QGridLayout;
class QKeyEvent;
class K3bJobProgressOSD;
class K3bThemedLabel;


class K3bJobProgressDialog : public KDialog, public K3bJobHandler
{
  Q_OBJECT

 public:
  K3bJobProgressDialog( QWidget* parent = 0, 
			const char* name = 0, 
			bool showSubProgress = true, 
			bool modal = FALSE, 
			WFlags fl = 0 );
  virtual ~K3bJobProgressDialog();

  virtual void setJob( K3bJob* job );
  void setExtraInfo( QWidget *extra );

  /**
   * reimplemented for internal reasons
   */
  void show();

  /**
   * reimplemented for internal reasons
   */
  void hide();

  /**
   * This will show the dialog and then start the given job or
   * if job == 0 the job set with setJob
   * Use instead of exec()
   */
  int startJob( K3bJob* job = 0 );

  QSize sizeHint() const;

  /**
   * @reimplemented from K3bJobHandler
   */
  int waitForMedia( K3bDevice::Device*,
		    int mediaState = K3bDevice::STATE_EMPTY,
		    int mediaType = K3bDevice::MEDIA_WRITABLE_CD,
		    const QString& message = QString::null );
  
  /**
   * @reimplemented from K3bJobHandler
   */
  bool questionYesNo( const QString& text,
		      const QString& caption = QString::null,
		      const QString& yesText = QString::null,
		      const QString& noText = QString::null );

  /**
   * reimplemented from K3bJobHandler
   */
  void blockingInformation( const QString& text,
			    const QString& caption = QString::null );
  
 protected slots:
  virtual void slotProcessedSize( int processed, int size );
  virtual void slotProcessedSubSize( int processed, int size );
  virtual void slotInfoMessage( const QString& infoString, int type );
  virtual void slotDebuggingOutput( const QString&, const QString& );
  virtual void slotNewSubTask(const QString& name);
  virtual void slotNewTask(const QString& name);
  virtual void slotFinished(bool);
  virtual void slotCanceled();
  virtual void slotStarted();


  void slotCancelButtonPressed();
  void slotUpdateTime();
  void slotShowDebuggingOutput();

  void slotProgress( int );

  virtual void slotThemeChanged();

 protected:
  void closeEvent( QCloseEvent* );
  void keyPressEvent( QKeyEvent* e );

  void setupGUI();
  void setupConnections();
	
  K3bThemedLabel* m_labelJob;
  K3bThemedLabel* m_labelJobDetails;
  KListView* m_viewInfo;
  K3bThemedLabel* m_labelTask;
  K3bThemedLabel* m_labelElapsedTime;
  KCutLabel* m_labelSubTask;
  QLabel* m_labelSubProcessedSize;
  KProgress* m_progressSubPercent;
  QLabel* m_labelProcessedSize;
  KProgress* m_progressPercent;
  QFrame* m_frameExtraInfo;
  QPushButton* m_buttonCancel;
  QPushButton* m_buttonClose;
  QPushButton* m_buttonShowDebug;
  K3bThemedLabel* m_pixLabel;

  QGridLayout* m_frameExtraInfoLayout;

 private:
  class Private;
  Private* d;

  K3bJob* m_job;
  QTimer* m_timer;
  QTime m_startTime;
  QTime m_lastProgressUpdateTime;

  K3bDebuggingOutputFile m_logFile;

  QMap<QString, QStringList> m_debugOutputMap;

  bool m_bCanceled;

  QString m_plainCaption;

  bool in_loop;

  K3bJobProgressOSD* m_osd;
};


#endif
