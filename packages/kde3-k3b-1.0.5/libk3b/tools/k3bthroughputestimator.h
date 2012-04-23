/* 
 *
 * $Id: k3bthroughputestimator.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef _K3B_THROUGHPUT_ESTIMATOR_H_
#define _K3B_THROUGHPUT_ESTIMATOR_H_

#include <qobject.h>


/**
 * Little helper class that allows an estimation of the current writing
 * speed. Just init with @p reset() then always call @p dataWritten with
 * the already written data in KB. The class will emit throughput signals
 * whenever the throughput changes.
 */
class K3bThroughputEstimator : public QObject
{
  Q_OBJECT

 public:
  K3bThroughputEstimator( QObject* parent = 0, const char* name = 0 );
  ~K3bThroughputEstimator();

  int average() const;

 signals:
  /**
   * kb/s if differs from previous
   */
  void throughput( int );

 public slots:
  void reset();

  /**
   * @param data written kb
   */
  void dataWritten( unsigned long data );

 private:
  class Private;
  Private* d;
};

#endif
