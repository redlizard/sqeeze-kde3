/*****************************************************************

Copyright (c) 1996-2001 the kicker authors. See file AUTHORS.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include "print_mnu.h"
#include <kiconloader.h>
#include <klocale.h>
#include <kglobal.h>
#include <kapplication.h>
#include <krun.h>
#include <kdeprint/kmmanager.h>
#include <qregexp.h>

K_EXPORT_KICKER_MENUEXT(kdeprint, PrintMenu)

#define ADD_PRINTER_ID		0
#define KDEPRINT_SETTINGS_ID	1
#define	CONFIG_SERVER_ID	2
#define	PRINT_MANAGER_ID	3
#define PRINT_BROWSER_ID	4
#define	KPRINTER_ID		5
#define PRINTER_LIST_ID		10

PrintMenu::PrintMenu(QWidget *parent, const char *name, const QStringList & /*args*/)
: KPanelMenu("", parent, name)
{
    static bool kdeprintIconsInitialized = false;
    if ( !kdeprintIconsInitialized ) {
        KGlobal::iconLoader()->addAppDir("kdeprint");
        kdeprintIconsInitialized = true;
    }
}

PrintMenu::~PrintMenu()
{
}

void PrintMenu::initialize()
{
    if (initialized()) clear();
    setInitialized(true);

    int ID = PRINTER_LIST_ID;
    // just to be sure the plugin is loaded -> icons are available
    KMManager::self();

    if ((KMManager::self()->printerOperationMask() & KMManager::PrinterCreation) && KMManager::self()->hasManagement())
        insertItem(SmallIconSet("wizard"), i18n("Add Printer..."), ADD_PRINTER_ID);
    insertItem(SmallIconSet("kdeprint_configmgr"), i18n("KDE Print Settings"), KDEPRINT_SETTINGS_ID);
    if (KMManager::self()->serverOperationMask() & KMManager::ServerConfigure)
        insertItem(SmallIconSet("kdeprint_configsrv"), i18n("Configure Server"), CONFIG_SERVER_ID);
    insertSeparator();
    insertItem(SmallIconSet("kcontrol"), i18n("Print Manager"), PRINT_MANAGER_ID);
    insertItem(SmallIconSet("konqueror"), i18n("Print Browser (Konqueror)"), PRINT_BROWSER_ID);
    insertSeparator();
    insertItem(SmallIconSet("fileprint"), i18n("Print File..."), KPRINTER_ID);

    // printer list
    QPtrList<KMPrinter>    *l = KMManager::self()->printerList();
    if (l && !l->isEmpty())
    {
        bool separatorInserted = false;
        QPtrListIterator<KMPrinter>    it(*l);
        for (; it.current(); ++it)
        {
            // no special, implicit or pure instances
            if (it.current()->isSpecial() || it.current()->isVirtual())
                continue;
            if (!separatorInserted)
            {
                // we insert a separator only when we find the first
                // printer
                insertSeparator();
                separatorInserted = true;
            }
            insertItem(SmallIconSet(it.current()->pixmap()),
                       it.current()->printerName(), ID++);
        }
    }
}

void PrintMenu::slotExec(int ID)
{
    switch (ID)
    {
        case ADD_PRINTER_ID:
            kapp->kdeinitExec("kaddprinterwizard");
            break;
        case KDEPRINT_SETTINGS_ID:
	    kapp->kdeinitExec("kaddprinterwizard", QStringList("--kdeconfig"));
            break;
	case CONFIG_SERVER_ID:
	    kapp->kdeinitExec("kaddprinterwizard", QStringList("--serverconfig"));
	    break;
        case PRINT_MANAGER_ID:
            KRun::runCommand("kcmshell kde-printers.desktop");
            break;
        case PRINT_BROWSER_ID:
            KRun::runCommand("kfmclient openProfile filemanagement print:/", "kfmclient", "konqueror");
            break;
	case KPRINTER_ID:
	    kapp->kdeinitExec("kprinter");
	    break;
        default:
            {
                // start kjobviewer
                QStringList args;
                args << "--show" << "-d" << text(ID).remove('&');
                kapp->kdeinitExec("kjobviewer", args);
            }
            break;
    }
}

void PrintMenu::reload()
{
	initialize();
}

#include "print_mnu.moc"
