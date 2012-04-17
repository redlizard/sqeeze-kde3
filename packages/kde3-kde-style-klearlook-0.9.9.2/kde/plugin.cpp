/* $Id: plugin.cpp,v 1.4 2005/11/27 21:30:50 jck Exp $
*
* Qt Plugin interface for the Klearlook style
*

Klearlook (C) Joerg C. Koenig, 2005 jck@gmx.org

----

Based upon QtCurve (C) Craig Drummond, 2003 Craig.Drummond@lycos.co.uk
    Bernhard Rosenkränzer <bero@r?dh?t.com>
    Preston Brown <pbrown@r?dh?t.com>
    Than Ngo <than@r?dh?t.com>

Released under the GNU General Public License (GPL) v2.

----

*/
#include <qstyleplugin.h>
#include "klearlook.h"
#include "config.h"

class KlearlookStylePlugin : public QStylePlugin {
    public:
        KlearlookStylePlugin();

        QStringList keys() const;
        QStyle *create( const QString & );
};

KlearlookStylePlugin::KlearlookStylePlugin()
        : QStylePlugin() {}

QStringList KlearlookStylePlugin::keys() const {
    QStringList list;
    list << "Klearlook";

#ifndef USE_SINGLE_STYLE

    list << "Klearlook V2";
    list << "Klearlook V3";
    list << "Klearlook V5";
#endif

    return list;
}

QStyle * KlearlookStylePlugin::create( const QString &s ) {
#ifdef USE_SINGLE_STYLE
    if ( s.lower() == "klearlook" )
        return new KlearlookStyle;
#else

    if ( s.lower() == "klearlook" )
        return new KlearlookStyle( false );
    else if ( s.lower() == "klearlook v2" )
        return new KlearlookStyle( true, true, true );
    else if ( s.lower() == "klearlook v3" )
        return new KlearlookStyle( true, true, true, true, GROOVE_SUNKEN, GROOVE_SUNKEN, true, false, true, false, IND_CORNER,
                                   TB_LIGHT, LV_EXP_ARR, LV_LINES_SOLID, false, true );
    else if ( s.lower() == "klearlook v5" )
        return new KlearlookStyle( true, true, true, true, GROOVE_RAISED, GROOVE_RAISED, true, false, true, false, IND_CORNER,
                                   TB_DARK, LV_EXP_ARR, LV_LINES_SOLID, false, true );
#endif

    return 0;
}

Q_EXPORT_PLUGIN( KlearlookStylePlugin )
