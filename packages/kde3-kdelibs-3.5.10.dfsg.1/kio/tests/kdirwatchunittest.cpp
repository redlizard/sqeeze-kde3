 /*
  This file is or will be part of KDE desktop environment

        (c) 2006 Dirk Mueller <mueller@kde.org>

  It is licensed under GPL version 2.

  If it is part of KDE libraries than this file is licensed under
  LGPL version 2.
 */

#include <unistd.h>

#include <qfile.h>
#include <qdir.h>

#include <kdebug.h>

#include "kdirwatchunittest.h"

void KDirWatchTest::VERIFY_NOTHING()
{
    unsigned m_s[3];
    for(int i = 0; i < 3; ++i)
        m_s[i] = m_signals[i];

    for (int i = 0; i < 20; ++i) {
      usleep(100*1000);
      qApp->processEvents();
    }

    bool nothing_failed = true;
    for (int i = 0; i < 3; ++i)
        if (m_s[i] != m_signals[i])
            nothing_failed = false;

    KDirWatch::statistics();

    VERIFY (nothing_failed);
}

void KDirWatchTest::VERIFY_DIRTY(const QString& alert)
{
    unsigned m_s[3];
    for(int i = 0; i < 3; ++i)
        m_s[i] = m_signals[i];

    for (int i = 0; i < 20; ++i) {
      usleep(100*1000);
      qApp->processEvents();
    }

    VERIFY (m_s[sigCreated] == m_signals[sigCreated] &&
            m_s[sigDeleted] == m_signals[sigDeleted] && 
            m_s[sigDirty] + 1 == m_signals[sigDirty] && 
            m_lastSignal == alert);
}

void KDirWatchTest::VERIFY_CREATED(const QString& alert)
{
    unsigned m_s[3];
    for(int i = 0; i < 3; ++i)
        m_s[i] = m_signals[i];

    for (int i = 0; i < 20; ++i) {
      usleep(100*1000);
      qApp->processEvents();
    }

    VERIFY (m_s[sigDirty] == m_signals[sigDirty] &&
            m_s[sigDeleted] == m_signals[sigDeleted] && 
            m_s[sigCreated] + 1 == m_signals[sigCreated] &&
            m_lastSignal == alert);
}

void KDirWatchTest::VERIFY_DELETED(const QString& alert)
{
    unsigned m_s[3];
    for(int i = 0; i < 3; ++i)
        m_s[i] = m_signals[i];

    for (int i = 0; i < 20; ++i) {
      usleep(100*1000);
      qApp->processEvents();
    }

    VERIFY (m_s[sigDirty] == m_signals[sigDirty] &&
            m_s[sigCreated] == m_signals[sigCreated] && 
            m_s[sigDeleted] + 1 == m_signals[sigDeleted] &&
            m_lastSignal == alert);
}

void KDirWatchTest::remove_file (const QString& file)
{
  ::unlink (QFile::encodeName(file));
}

void KDirWatchTest::touch_file (const QString& file)
{
  QFile f(file);
  f.open(IO_WriteOnly);
}

void KDirWatchTest::rename_file(const QString& from, const QString& to)
{
  ::rename(QFile::encodeName(from), QFile::encodeName(to));
}

KUNITTEST_MODULE ( kunittest_kdirwatch, "KDirWatchTest" )
KUNITTEST_MODULE_REGISTER_TESTER (KDirWatchTest)

#define SLEEP() QApplication::processEvents();

void KDirWatchTest::allTests()
{
  for(int loop=0; loop<3; ++loop) {
    d = new KDirWatch;
    VERIFY (d != 0);

    QDir* dir = new QDir(m_workingDir);
    VERIFY (dir != 0);

    connect(d, SIGNAL (dirty( const QString &)), SLOT( slotDirty( const QString &)) );
    connect(d, SIGNAL (created( const QString &)), SLOT( slotCreated( const QString &)) );
    connect(d, SIGNAL (deleted( const QString &)), SLOT( slotDeleted( const QString &)) );

    VERIFY (dir->mkdir (m_workingDir));

    d->addDir (m_workingDir);
    VERIFY_NOTHING();
    dir->mkdir ("does");
    VERIFY_DIRTY (m_workingDir);
    touch_file (m_workingDir + "/file");
    VERIFY_DIRTY (m_workingDir);
    VERIFY_NOTHING ();
    remove_file (m_workingDir + "/file");
    d->addDir (m_workingDir + "/does/not/exist");
    d->removeDir (m_workingDir);
    VERIFY_NOTHING();

    dir->mkdir ("does/not");
    VERIFY_NOTHING();
    dir->mkdir ("does/not/exist");
    VERIFY_CREATED (m_workingDir + "/does/not/exist");

    dir->rmdir ("does/not/exist");
    VERIFY_DELETED (m_workingDir + "/does/not/exist");
    dir->rmdir ("does/not");
    VERIFY_NOTHING();
    dir->rmdir ("does");
    VERIFY_NOTHING();

    d->addFile(m_workingDir + "/a");
    touch_file(m_workingDir + "/a");
    VERIFY_CREATED (m_workingDir + "/a");

    rename_file (m_workingDir + "/a", m_workingDir + "/b");
    VERIFY_DELETED (m_workingDir + "/a");
    VERIFY_NOTHING();

    touch_file (m_workingDir + "/a");
    VERIFY_CREATED (m_workingDir + "/a");
    VERIFY_NOTHING();

    touch_file (m_workingDir + "/a");
    VERIFY_DIRTY (m_workingDir + "/a");

    remove_file (m_workingDir + "/b");
    VERIFY_NOTHING();

    d->removeFile(m_workingDir + "/a");

    remove_file(m_workingDir + "/a");

    VERIFY (dir->rmdir (m_workingDir));
    delete d;
  }
}

#include "kdirwatchunittest.moc"
