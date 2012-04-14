/**
 * kunittest.cpp
 *
 * Copyright (C)  2004  Zack Rusin <zack@kde.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "kunittest.h"

#include "tester.h"
#include "chatnavtests.h"

#include <qapplication.h>
#include <qtimer.h>

#include <iostream>
using namespace std;

void KUnitTest::registerTests()
{
      ADD_TEST( ChatNavTests );
//    ADD_TEST( SampleTest );
//    ADD_TEST( OnePassTest );
//    ADD_TEST( TwoPassTest );
//    ADD_TEST( MultiFailTest );
//    ADD_TEST( ExpectedFailureTest );
//    ADD_TEST( UnexpectedPassTest );
//    ADD_TEST( OnlyUnexpectedPassTest );
//    ADD_TEST( SkipLogTest );
//    ADD_TEST( SkipWithFailTest );
}

KUnitTest::KUnitTest()
{
    QTimer::singleShot( 0, this, SLOT(checkRun()) );

    m_tests.setAutoDelete( TRUE );
//    m_qtests.setAutoDelete( TRUE );

    registerTests();
}

void KUnitTest::checkRun()
{
//    if ( m_qtests.isEmpty() )
//        qApp->exit();
}

int KUnitTest::runTests()
{
    int globalSteps = 0;
    int globalPasses = 0;
    int globalFails = 0;
    int globalXFails = 0;
    int globalXPasses = 0;
    int globalSkipped = 0;

    cout << "# Running normal tests... #" << endl << endl;
    QAsciiDictIterator<Tester> it( m_tests );

    for( ; it.current(); ++it ) {
        Tester* test = it.current();
        test->allTests();
	cout << it.currentKey() << " - ";
	int numPass = test->testsFinished() - ( test->errorList().count() + test->xfailList().count() + test->skipList().count() );
	int numFail = test->errorList().count() + test->xfailList().count();
	int numXFail = test->xfailList().count();
	int numXPass = test->xpassList().count();
	int numSkip = test->skipList().count();

	globalSteps += test->testsFinished();
	globalPasses += numPass;
	globalFails += numFail;
	globalXFails += numXFail;
	globalXPasses += numXPass;
	globalSkipped += numSkip;

	cout << numPass << " test" << ( ( 1 == numPass )?"":"s") << " passed";
	if ( 0 < test->xpassList().count() ) {
	    cout << " (" << numXPass << " unexpected pass" << ( ( 1 == numXPass )?"":"es") << ")";
	}
	cout << ", " << numFail << " test" << ( ( 1 == numFail )?"":"s") << " failed";
	if ( 0 < numXFail  ) {
	    cout << " (" << numXFail << " expected failure" << ( ( 1 == numXFail )?"":"s") << ")";
	}
	if ( 0 < numSkip ) {
	    cout << "; also " << numSkip << " skipped";
	}	
	cout  << endl;

	if ( 0 < numXPass  ) {
	  cout << "    Unexpected pass" << ( ( 1 == numXPass )?"":"es") << ":" << endl;
	  QStringList list = test->xpassList();
	  for ( QStringList::Iterator itr = list.begin(); itr != list.end(); ++itr ) {
	    cout << "\t" << (*itr).latin1() << endl;
	  }
	}
	if ( 0 < (numFail - numXFail) ) {
	  cout << "    Unexpected failure" << ( ( 1 == numFail )?"":"s") << ":" << endl;
	  QStringList list = test->errorList();
	  for ( QStringList::Iterator itr = list.begin(); itr != list.end(); ++itr ) {
	    cout << "\t" << (*itr).latin1() << endl;
	  }
	}
	if ( 0 < numXFail ) {
	  cout << "    Expected failure" << ( ( 1 == numXFail)?"":"s") << ":" << endl;
	  QStringList list = test->xfailList();
	  for ( QStringList::Iterator itr = list.begin(); itr != list.end(); ++itr ) {
	    cout << "\t" << (*itr).latin1() << endl;
	  }
	}
	if ( 0 < numSkip ) {
	    cout << "    Skipped test" << ( ( 1 == numSkip )?"":"s") << ":" << endl;
	    QStringList list = test->skipList();
	    for ( QStringList::Iterator itr = list.begin(); itr != list.end(); ++itr ) {
		cout << "\t" << (*itr).latin1() << endl;
	    }
	}
	cout << endl;
    }

    cout << "# Done with normal tests:" << endl;
    cout << "  Total test cases: " << m_tests.count() << endl;
    cout << "  Total test steps                                 : " << globalSteps << endl;
    cout << "    Total passed test steps (including unexpected) : " << globalPasses << endl;
    cout << "      Total unexpected passed test steps           :  " << globalXPasses << endl;
    cout << "    Total failed test steps (including expected)   :  " << globalFails << endl;
    cout << "      Total expected failed test steps             :  " << globalXFails << endl;
    cout << "    Total skipped test steps                       :  " << globalSkipped << endl;

    return m_tests.count();
}

//void KUnitTest::addTester( QTester *test )
//{
//    m_qtests.insert( test, test );
//    connect( test, SIGNAL(destroyed(QObject*)),
//             SLOT(qtesterDone(QObject* )) );
//}

void KUnitTest::qtesterDone( QObject *obj )
{
//    m_qtests.remove( obj );
//    if ( m_qtests.isEmpty() )
//        qApp->quit();
}

#include "kunittest.moc"
