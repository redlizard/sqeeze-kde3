#ifndef __X_CONFIG_H__
#define __X_CONFIG_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : KFI::CXConfig
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 05/05/2001
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
// (C) Craig Drummond, 2001, 2002, 2003, 2004
///////////////////////////////////////////////////////////////////////////////

#include "Misc.h"
#include <qptrlist.h>
#include <qstring.h>
#include <qstringlist.h>
#include <time.h>

namespace KFI
{

class CFontEngine;

class CXConfig
{
    public:

    struct TPath
    {
        enum EType
        {
            DIR,
            FONT_SERVER,
            FONT_CONFIG
        };

        TPath(const QString &d, bool u=false, EType t=DIR, bool o=true)
           : dir(DIR==t ? Misc::dirSyntax(d) : d), unscaled(u), orig(o), type(t) {}

        static EType getType(const QString &d);

        QString dir;
        bool    unscaled,
                orig;          // Was dir in file when read?
        EType   type;
    };

    enum EType
    {
        XFS,
        X11
    };

    public:

    CXConfig(EType type, const QString &file);

    static bool configureDir(const QString &dir);

    bool  ok()                       { return itsOk; }
    bool  writable()                 { return itsWritable; }
    bool  readConfig();
    bool  writeConfig();
    bool  madeChanges();
    void  addPath(const QString &dir, bool unscaled=false);
    bool  inPath(TPath::EType type);
    bool  xfsInPath()                { return inPath(TPath::FONT_SERVER); }
    bool  fcInPath()                 { return inPath(TPath::FONT_CONFIG); }
    void  refreshPaths()             { refreshPaths(XFS==itsType); }
    void  restart();
    EType getType()                  { return itsType; }

    static void refreshPaths(bool xfs);

    private:

    bool processX11(bool read);
    bool processXfs(bool read);

    TPath * findPath(const QString &dir);

    private:

    EType           itsType;
    QPtrList<TPath> itsPaths;
    QString         itsFileName,
                    itsInsertPos;
    bool            itsOk,
                    itsWritable;
    time_t          itsTime;
};

}

#endif
