////////////////////////////////////////////////////////////////////////////////
//
// Namespace     : KFI::Print
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 14/05/2005
// Version       : $Revision$ $Date$
//
////////////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2005
////////////////////////////////////////////////////////////////////////////////

#include "KfiPrint.h"
#include "FcEngine.h"
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qsettings.h>
#include <qstringlist.h>
#include <kprinter.h>
#include <qapplication.h>
#include <qeventloop.h>
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

namespace KFI
{

namespace Print
{

static const int constMarginLineBefore=1;
static const int constMarginLineAfter=2;
static const int constMarginFont=4;

inline bool sufficientSpace(int y, int pageHeight, int size)
{
    return (y+constMarginFont+size)<pageHeight;
}

static bool sufficientSpace(int y, int titleFontHeight, const int *sizes, int pageHeight, int size)
{
    int required=titleFontHeight+constMarginLineBefore+constMarginLineAfter;

    for(unsigned int s=0; sizes[s]; ++s)
    {
        required+=sizes[s];
        if(sizes[s+1])
            required+=constMarginFont;
    }

    if(0==size)
        required+=(3*(constMarginFont+CFcEngine::constDefaultAlphaSize))+constMarginLineBefore+constMarginLineAfter;
    return (y+required)<pageHeight;
}

bool printable(const QString &mime)
{
    return "application/x-font-type1"==mime || "application/x-font-ttf"==mime || "application/x-font-otf"==mime ||
           "application/x-font-ttc"==mime || "application/x-font-ghostscript"==mime;
}

void printItems(const QStringList &items, int size, QWidget *parent, CFcEngine &engine)
{
#ifdef HAVE_LOCALE_H
    char *oldLocale=setlocale(LC_NUMERIC, "C"),
#endif

    KPrinter printer;

    printer.setFullPage(true);

    if(printer.setup(parent))
    {
        QPainter   painter;
        QFont      sans("sans", 12, QFont::Bold);
        QSettings  settings;
        bool       entryExists,
                   embedFonts,
                   set=false;
        QString    str(engine.getPreviewString());

        //
        // Cehck whether the user has enabled font embedding...
        embedFonts=settings.readBoolEntry("/qt/embedFonts", false, &entryExists);

        // ...if not, then turn on - we may have installed new fonts, without ghostscript being informed, etc.
        if(!entryExists || !embedFonts)
            settings.writeEntry("/qt/embedFonts", true);

        printer.setResolution(72);
        painter.begin(&printer);

        QPaintDeviceMetrics metrics(painter.device());
        int                 margin=(int)((2/2.54)*metrics.logicalDpiY()), // 2 cm margins
                            pageWidth=metrics.width()-(2*margin),
                            pageHeight=metrics.height()-(2*margin),
                            y=margin,
                            oneSize[2]={size, 0};
        const int           *sizes=oneSize;
        bool                firstFont(true);

        if(0==size)
            sizes=CFcEngine::constScalableSizes;

        painter.setClipping(true);
        painter.setClipRect(margin, margin, pageWidth, pageHeight);

        QStringList::ConstIterator it(items.begin()),
                                   end(items.end());

        for(; it!=end; ++it)
        {
            unsigned int s=0;

            painter.setFont(sans);
            QApplication::eventLoop()->processEvents(QEventLoop::ExcludeUserInput, 0);

            if(!firstFont && !sufficientSpace(y, painter.fontMetrics().height(), sizes, pageHeight, size))
            {
                printer.newPage();
                y=margin;
            }
            painter.setFont(sans);
            y+=painter.fontMetrics().height();
            painter.drawText(margin, y, *it);
            y+=constMarginLineBefore;
            painter.drawLine(margin, y, margin+pageWidth, y);
            y+=constMarginLineAfter;

            if(0==size)
            {
                y+=CFcEngine::constDefaultAlphaSize;
                painter.setFont(engine.getQFont(*it, CFcEngine::constDefaultAlphaSize));
                painter.drawText(margin, y, CFcEngine::getLowercaseLetters());
                y+=constMarginFont+CFcEngine::constDefaultAlphaSize;
                painter.drawText(margin, y, CFcEngine::getUppercaseLetters());
                y+=constMarginFont+CFcEngine::constDefaultAlphaSize;
                painter.drawText(margin, y, CFcEngine::getPunctuation());
                y+=constMarginFont+constMarginLineBefore;
                painter.drawLine(margin, y, margin+pageWidth, y);
                y+=constMarginLineAfter;
            }
            for(; sizes[s]; ++s)
            {
                y+=sizes[s];
                painter.setFont(engine.getQFont(*it, sizes[s]));
                if(sufficientSpace(y, pageHeight, sizes[s]))
                {
                    painter.drawText(margin, y, str);
                    if(sizes[s+1])
                        y+=constMarginFont;
                }
            }
            firstFont=false;
            y+=(s<1 || sizes[s-1]<25 ? 14 : 28);
        }

        painter.end();

        //
        // Did we change the users font settings? If so, reset to their previous values...
        if(set)
            if(entryExists)
                settings.writeEntry("/qt/embedFonts", false);
            else
                settings.removeEntry("/qt/embedFonts");
    }
#ifdef HAVE_LOCALE_H
    if(oldLocale)
        setlocale(LC_NUMERIC, oldLocale);
#endif
}

}

}
