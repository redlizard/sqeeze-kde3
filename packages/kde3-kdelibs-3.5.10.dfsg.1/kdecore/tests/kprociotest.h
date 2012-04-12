//
//  DUMMY -- A dummy class with a slot to demonstrate KProcess signals
//
//  version 0.2, Aug 2nd 1997
//
//  (C) Christian Czezatke
//  e9025461@student.tuwien.ac.at
//


#ifndef __DUMMY_H__
#define __DUMMY_H__

#include <stdio.h>
#include <qobject.h>
#include "kprocio.h"

class Dummy : public QObject
{
 Q_OBJECT

 public slots:
   void printMessage(KProcess *proc)
   {
     printf("Process %d exited!\n", (int)proc->getPid()); 
   } 
 
   void gotOutput(KProcIO*proc)
   {
    QString line;
    while(true) {
       int result = proc->readln(line);
       if (result == -1) return;
       printf("OUTPUT>> [%d] '%s'\n", result, line.latin1());
    }
   }

};

#endif


