//
// C++ Implementation: kttsdplugin
//
// Description: 
//
//
// Author: Robert Vogl <voglrobe@lapislazuli>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
// #include <time.h> // nanosleep

// Qt includes
#include <qstring.h>
#include <qstringlist.h>

// KDE includes
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>

// App specific includes
#include "kttsdlib.h"
#include "kttsdlibtalker2.h"
#include "kttsdlibsetupimpl.h"


KTTSDLib::KTTSDLib(QObject *parent, const char *name, KApplication *Appl)
 : QObject(parent, name), m_Appl(Appl)
{
    KGlobal::locale()->insertCatalogue("libKTTSD");
    m_talker = new kttsdlibtalker2(static_cast<QObject*>(this), "kttsdlibtalker");
    connect(m_talker, SIGNAL(signalTextFinished(const uint)),
        this, SLOT(slotTextFinished(const uint)) );
    connect(m_talker, SIGNAL(signalTextStopped(const uint)),
        this, SLOT(slotTextStopped(const uint)) );
    connect(m_talker, SIGNAL(signalTextStarted(const uint)),
        this, SLOT(slotTextStarted(const uint)) );
    
    // reset list of currently processed jobs
    while ( !jobList.empty() ){
        jobList.pop();
    }
    
    // initialize the talker
    m_talker->KTTSD_init(m_Appl);
    
    // some presets
    // m_config = new KSimpleConfig("ksayit_kttsdpluginrc");
    // m_usersetupimpl = NULL;
}


KTTSDLib::~KTTSDLib()
{
    // delete m_config;
}


QString KTTSDLib::getName() const
{
    return "KDE KTTSD";
}


QString KTTSDLib::getDescription() const
{
    QString str;
    str =  i18n("<qt><big><u>Description:</u></big><br>");
    str += i18n("This plugin uses the KDE TTS Daemon for speech output.");
        
    return str;
}


int KTTSDLib::getActions()
{
    return ACTIONS::PLAY | ACTIONS::STOP | ACTIONS::PAUSE | ACTIONS::FFWD | ACTIONS::FREV;
}


int KTTSDLib::getStatus() const
{
    return TTS::AUDIOFILE;
}
    

const QWidget* KTTSDLib::getGUI(QFrame *frame)
{
    kdDebug(100200) << "KTTSDLib::getGUI()" << endl;
    
        return new KTTSDlibSetupImpl( frame, "kttsdlibsetup" );
}


void KTTSDLib::reloadConfiguration()
{
    kdDebug(100200) << "KTTSDLib::reloadConfiguration()" << endl;
        // N.A.
}


bool KTTSDLib::saveWasClicked() const
{
    kdDebug(100200) << "KTTSDLib::saveWasClicked()" << endl;
        // N.A.
    
    return true;
}
    

void KTTSDLib::setText(const QString &text)
{
    kdDebug(100200) << "KTTSDLib::setText()" << endl;
    
    uint jobNum = m_talker->KTTSD_setText(text, "");
    jobList.push(jobNum);    
}
    


void KTTSDLib::sayText()
{
    kdDebug(100200) << "KTTSDLib::sayText()" << endl;
     
    if( !jobList.empty() ){
        m_curJobNum = jobList.front();
        jobList.pop();
        kdDebug(100200) << "   Starting job No.: " << m_curJobNum << endl;
        m_talker->KTTSD_startText( m_curJobNum );
    }
}
    

void KTTSDLib::removeAllJobsFromList()
{
    kdDebug(100200) << "KTTSDLib::removeAllJobsFromList()" << endl;
    
    m_talker->KTTSD_removeText(m_curJobNum);    
    
    while( !jobList.empty() ){
        uint job = jobList.front();
        jobList.pop();
        kdDebug(100200) << "*** removing... " << job << endl;
        m_talker->KTTSD_removeText( job );
    }
}

void KTTSDLib::stop()
{
    kdDebug(100200) << "***** KTTSDLib::stop(" << m_curJobNum << ")" << endl;
    m_talker->KTTSD_stopText(m_curJobNum);
    removeAllJobsFromList();
    emit signalFinished();
}


void KTTSDLib::pause()
{
    kdDebug(100200) << "void KTTSDLib::pause(" << m_curJobNum << ")" << endl;
    
    m_talker->KTTSD_pauseText( m_curJobNum );
}
    
    
void KTTSDLib::resume()
{
    kdDebug(100200) << "void KTTSDLib::resume(" << m_curJobNum << ")" << endl;
    
    m_talker->KTTSD_resumeText( m_curJobNum );
}


void KTTSDLib::ffwd()
{
    kdDebug(100200) << "void KTTSDLib::ffwd(" << m_curJobNum << ")" << endl;
    m_talker->KTTSD_moveRelTextSentence(1, m_curJobNum);
}


void KTTSDLib::frev()
{
    kdDebug(100200) << "void KTTSDLib::frev(" << m_curJobNum << ")" << endl;
    m_talker->KTTSD_moveRelTextSentence(-1, m_curJobNum);
}


//////////////////////////////////////
// Signals from the talker
//////////////////////////////////////

void KTTSDLib::slotTextFinished(const uint job)
{
    kdDebug(100200) << "---- KTTSDLib::slotTextFinished(" << job << ")" << endl;
    
    // check if List is empty. If yes, send signalFinished().
    if ( jobList.empty() ){
        kdDebug(100200) << "    All jobs processed." << endl;
        emit signalFinished();
    } else {
        sayText();
    }
}
    
void KTTSDLib::slotTextStopped(const uint job)
{
    kdDebug(100200) << "---- KTTSDLib::slotTextStopped(" << job << ")" << endl;
    // removeAllJobsFromList();
    // emit signalFinished();
}


void KTTSDLib::slotTextStarted(const uint job)
{
    kdDebug(100200) << "---- KTTSDLib::slotTextStarted(" << job << ")" << endl;
    // m_curJobNum = job;    
}

#include "kttsdlib.moc"
