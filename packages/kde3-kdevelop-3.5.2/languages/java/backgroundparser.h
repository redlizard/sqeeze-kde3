/***************************************************************************
 *   Copyright (C) 2002 by Roberto Raggi                                   *
 *   roberto@kdevelop.org                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef BACKGROUNDPARSER_H
#define BACKGROUNDPARSER_H

#include "driver.h"
#include "JavaAST.hpp"

#include <qthread.h>
#include <qwaitcondition.h>
#include <qmutex.h>
#include <qmap.h>
#include <kdebug.h>

class JavaSupportPart;
class TranslationUnitAST;
class SynchronizedFileList;

class Unit
{
public:
    Unit() {}
    ~Unit() {}

    QString fileName;
    QValueList<Problem> problems;
    RefJavaAST translationUnit;

protected:
    Unit( const Unit& source );
    void operator = ( const Unit& source );
};

class BackgroundParser: public QThread
{
public:
    BackgroundParser( JavaSupportPart*, QWaitCondition* consumed );
    virtual ~BackgroundParser();

    QMutex& mutex() { return m_mutex; }
    void lock() { m_mutex.lock(); }
    void unlock() { m_mutex.unlock(); }

    QWaitCondition& canParse() { return m_canParse; }
    QWaitCondition& isEmpty() { return m_isEmpty; }

    bool filesInQueue();

    void addFile( const QString& fileName, bool readFromDisk=false );
    void removeFile( const QString& fileName );
    void removeAllFiles();

    RefJavaAST translationUnit( const QString& fileName );
    QValueList<Problem> problems( const QString& fileName );

    void close();

    virtual void run();

protected:
    Unit* findUnit( const QString& fileName );
    Unit* parseFile( const QString& fileName, bool readFromDisk );

private:
    class KDevDriver* m_driver;
    QString m_currentFile;
    QWaitCondition m_canParse;
    QWaitCondition m_isEmpty;
    QWaitCondition* m_consumed;
    QMutex m_mutex;
    SynchronizedFileList* m_fileList;
    JavaSupportPart* m_javaSupport;
    bool m_close;
    QMap<QString, Unit*> m_unitDict;
};

#endif
