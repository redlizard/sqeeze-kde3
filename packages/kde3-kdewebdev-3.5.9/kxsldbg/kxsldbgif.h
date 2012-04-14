#ifndef KXSLDBGIF_H
#define KXSLDBGIF_H

#include <dcopobject.h>

class KXsldbgIf : virtual public DCOPObject 
{
    K_DCOP
    k_dcop:

    virtual void newCursorPosition(const QString & file, int lineNumber, int columnNumber=0) = 0;
    virtual void newDebuggerPosition(const QString & file, int lineNumber) = 0;

};

#endif

