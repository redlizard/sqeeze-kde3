/*
    From WebMaker - KDE HTML Editor
    Copyright (C) 1998, 1999 Alexei Dets <dets@services.ru>
    Rewritten for Quanta Plus: (C) 2002 Andras Mantia <amantia@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#ifndef _QEXTFILEINFO_H_
#define _QEXTFILEINFO_H_

#include <kio/global.h>
#include <kio/job.h>
#include <kurl.h>
#include <kfileitem.h>

#include <qobject.h>
#include <qptrlist.h>
#include <qdict.h>
#include <qregexp.h>

class QExtFileInfo:public QObject
{
 Q_OBJECT
public:
  QExtFileInfo() {};
  ~QExtFileInfo() {};

  /**
   * Works like QDir::canonicalPath, but it does not return an empty
   * string if the path does not exists, but tries to find if there
   * is a part of the path which exists and resolv the symlinks for
   * that part. The ending slash is kept in the result.
   * Example:
   * /home/user/foo points to /mnt/foo
   * /home/user/foo/foo2 does not exists
   * QExtFileInfo::canonicalPath("/home/user/foo/foo2/") will return
   * /mnt/foo/foo2/ .
   * @param path the path to resolve
   * @return the canonical path (symlinks resolved)
   */
  static QString canonicalPath(const QString& path);

  /**
   * Similar to QDir::homeDirPath(), but returns a resolved path.
   *
   */
  static QString homeDirPath();

  /** Returns the relative url of urlToConvert to baseURL. */
  static KURL toRelative(const KURL& urlToConvert, const KURL& baseURL, bool resolveSymlinks = true);
  /** Convert relative url to absolute, based on baseURL. */
  static KURL toAbsolute(const KURL& urlToConvert, const KURL& baseURL);
  /** Returns a recursive list of files under path, matching the specified file mask. */
  static KURL::List allFiles( const KURL& path, const QString &mask, QWidget *window);
  /** Returns a recursive list of files under path, matching the specified file mask.
      The returned urls are relative to path.
  */
  static KURL::List allFilesRelative( const KURL& path, const QString &mask, QWidget *window, bool resolveSymlinks = true);
  /** Returns a recursive list of files under path, matching the specified file mask.
      The returned list contains detailed information about each url.
      The user should delete the KFileItems and clear the dict
      after they are not needed.
  */
  static QDict<KFileItem> allFilesDetailed(const KURL& path, const QString &mask, QWidget *window);
  /** Creates a dir if don't exists. */
  static bool createDir(const KURL & path, QWidget *window);
  /** Returns the parent directory of dir. */
  static KURL cdUp(const KURL &dir);
  /** Returns the filename from a path string. */
  static QString shortName(const QString &fname );
  /** Returns the path to the url. */
  static KURL path(const KURL &);
  /** Returns the user's home directory as an url. */
  static KURL home();
  /** A slightly better working alternative of KIO::NetAccess::exists().
      Checks for the existance of the url. readingOnly is true if we check if
      the url is readable, and false if we check if it is writable.*/
  static bool exists(const KURL& url, bool readingOnly, QWidget *window);
  /** Synchronous copy, like NetAccess::file_copy in KDE 3.2, just that it doesn't show a progress dialog */
  static bool copy( const KURL& src, const KURL& dest, int permissions=-1,
                    bool overwrite=false, bool resume=false, QWidget* window = 0L );
  /** Reenters the event loop. You must call qApp->exit_loop() to exit it. */
  void enter_loop();

private:
  /** Internal methods called by the above ones. They start their own event loop and
      exit when the even loop is exited */
  bool internalExists(const KURL& url, bool readingOnly, QWidget *window);
  bool internalCopy(const KURL& src, const KURL& target, int permissions,
                    bool overwrite, bool resume, QWidget* window);
  KURL::List allFilesInternal(const KURL& startURL, const QString& mask, QWidget *window);
  QDict<KFileItem> allFilesDetailedInternal(const KURL& startURL, const QString& mask, QWidget *window);
  KURL::List allLocalFiles(const QString& startPath, const QString& mask);

  bool bJobOK;
  static QString lastErrorMsg;
  KIO::UDSEntry m_entry;
  KURL::List dirListItems;
  QDict<KFileItem> detailedDirListItems;
  QPtrList<QRegExp> lstFilters;
  int m_listJobCount;
  QString m_listStartURL;

private slots:
  void slotListResult(KIO::Job *job);
  void slotResult(KIO::Job * job);
  void slotNewEntries(KIO::Job *job, const KIO::UDSEntryList& udsList);
  void slotNewDetailedEntries(KIO::Job *job, const KIO::UDSEntryList& udsList);
public slots:
  /** Timeout occurred while waiting for some network function to return. */
  void slotTimeout();
};


#endif
