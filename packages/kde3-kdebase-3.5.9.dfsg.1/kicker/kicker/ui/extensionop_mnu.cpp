/*****************************************************************

Copyright (c) 1996-2000 the kicker authors. See file AUTHORS.

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

#include <klocale.h>
#include <kiconloader.h>
#include <kpanelextension.h>
#include <kstdguiitem.h>

#include "kicker.h"
#include "extensionop_mnu.h"

PanelExtensionOpMenu::PanelExtensionOpMenu(const QString& extension, int actions, QWidget *parent, const char *name)
  : QPopupMenu(parent, name)
{
    if (!Kicker::the()->isImmutable())
    {
        insertItem(SmallIcon("remove"), i18n("&Remove"), Remove);
    }

    if (actions & KPanelExtension::ReportBug)
    {
        insertSeparator();
        insertItem(i18n("Report &Bug..."), ReportBug);
    }

    if (actions & KPanelExtension::Help
        || actions & KPanelExtension::About)
	insertSeparator();

    if (actions & KPanelExtension::About)
    {
        insertItem(i18n("&About"), About);
    }

    if (actions & KPanelExtension::Help)
    {
        insertItem(SmallIcon("help"), KStdGuiItem::help().text(), Help);
    }

    if (!Kicker::the()->isImmutable() && (actions & KPanelExtension::Preferences)) {
	insertSeparator();
	insertItem(SmallIcon("configure"), i18n("&Configure %1...").arg(extension), Preferences);
    }

    adjustSize();
}
