//
// C++ Implementation: kttsdlibsetupimpl
//
// Description:
//
//
// Author: Robert Vogl <voglrobe@lapislazuli>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <stdio.h>

// Qt includes
#include <qpushbutton.h>

// KDE includes
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

// App specific includes
#include "kttsdlibsetupimpl.h"

KTTSDlibSetupImpl::KTTSDlibSetupImpl(QWidget *parent, const char *name)
 : KTTSDlibSetup(parent, name)
{
}


KTTSDlibSetupImpl::~KTTSDlibSetupImpl()
{
}


void KTTSDlibSetupImpl::slotLaunchControlcenter()
{
    kdDebug(100200) << "KTTSDlibSetupImpl::slotLaunchControlCenter()" << endl;

    // check if controllcenter module for KTTSD exists
    FILE *fp;
    char cmdresult[20];

    // if ( (fp = popen("kcmshell --list | grep kcmkttsmgr", "r")) != NULL){
    if ( (fp = popen("kcmshell --list | grep kcmkttsd", "r")) != NULL){
        fgets(cmdresult, 18, fp);
        pclose(fp);
    }
    if ( !QCString(cmdresult).contains("kcmkttsd") ){
            QString error = i18n("Control Center Module for KTTSD not found.");
            KMessageBox::sorry(this, error, i18n("Problem"));
            return;
    }

    // invoke the Control Center Module
    KProcess *kcmproc = new KProcess();
    connect(kcmproc, SIGNAL(processExited(KProcess*)),
                    this, SLOT(slotKCMProcessExited(KProcess*)) );
    (*kcmproc) << "kcmshell";
    (*kcmproc) << "kcmkttsd";
    kcmproc->start(KProcess::NotifyOnExit);

    kcm_Button->setEnabled(false);
}


void KTTSDlibSetupImpl::slotKCMProcessExited(KProcess *p)
{
    kdDebug(100200) << "slotKCMProcessExited()" << endl;
    kcm_Button->setEnabled(true);
}



#include "kttsdlibsetupimpl.moc"
