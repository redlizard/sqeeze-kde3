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

#include "ast.h"
#include "driver.h"

#include <qthread.h>
#include <qwaitcondition.h>
#include <qmutex.h>
#include <qmap.h>
#include <kdebug.h>

class CppSupportPart;
class TranslationUnitAST;
class SynchronizedFileList;
class BackgroundKDevDriver;
class Unit
{
public:
	Unit() : translationUnit( 0 )
	{}
	~Unit()
	{
		translationUnit = 0;
	}

	QString fileName;
	QValueList<Problem> problems;
	ParsedFilePointer translationUnit;

protected:
	Unit( const Unit& source );
	void operator = ( const Unit& source );
};

class BackgroundParser: public QThread
{
public:
	BackgroundParser( CppSupportPart*, QWaitCondition* consumed );
	virtual ~BackgroundParser();

	QMutex& mutex()
	{
		return m_mutex;
	}
	void lock ()
	{
		m_mutex.lock();
	}
	void unlock()
	{
		m_mutex.unlock();
	}

	QWaitCondition& canParse()
	{
		return m_canParse;
	}
	QWaitCondition& isEmpty()
	{
		return m_isEmpty;
	}

	bool filesInQueue();

    int countInQueue( const QString& file ) const;

    void addFile( const QString& fileName, bool readFromDisk = false );
    void addFileFront( const QString& fileName, bool readFromDisk = false );
    void removeFile( const QString& fileName );
	void removeAllFiles();

    bool hasTranslationUnit( const QString& fileName );
	ParsedFilePointer translationUnit( const QString& fileName );
	QValueList<Problem> problems( const QString& fileName , bool readFromDisk = false, bool forceParse = false );
	void updateParserConfiguration();

    ///Should be run on a regular basis(every X minutes). It reduces some caches etc. BackgroundParser must be locked before.
    void saveMemory();
  
	void close();

	virtual void run();

protected:
    friend class BackgroundKDevDriver;
    void fileParsed( ParsedFile& fileName );
	Unit* findUnit( const QString& fileName );
	void parseFile( const QString& fileName, bool readFromDisk, bool lock = false )
		;

private:
	class KDevDriver* m_driver;
	QString m_currentFile;
	QWaitCondition m_canParse;
	QWaitCondition m_isEmpty;
	QWaitCondition* m_consumed;
	QMutex m_mutex;
	SynchronizedFileList* m_fileList;
	CppSupportPart* m_cppSupport;
	bool m_close;
	QMap<QString, Unit*> m_unitDict;
    bool m_saveMemory; //used to prevent blocking

    //State of parseFile(..):
    bool m_readFromDisk;
};

#endif 
// kate: indent-mode csands; tab-width 4;

