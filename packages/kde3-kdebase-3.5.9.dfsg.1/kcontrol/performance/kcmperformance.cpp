/*
 *  Copyright (c) 2003 Lubos Lunak <l.lunak@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <qlayout.h>
#include <qtabwidget.h>

#include <klocale.h>
#include <kdialog.h>

#include "kcmperformance.h"
#include "konqueror.h"
#include "system.h"

extern "C"
{
    KDE_EXPORT KCModule* create_performance( QWidget* parent_P, const char* name_P )
    {
    return new KCMPerformance::Config( parent_P, name_P );
    }

    KDE_EXPORT KCModule* create_konqueror( QWidget* parent_P, const char* name_P )
    {
    return new KCMPerformance::KonquerorConfig( parent_P, name_P );
    }
}

namespace KCMPerformance
{

Config::Config( QWidget* parent_P, const char* )
    : KCModule( parent_P, "kcmperformance" )
    {
    setQuickHelp( i18n( "<h1>KDE Performance</h1>"
        " You can configure settings that improve KDE performance here." ));
    
    QVBoxLayout *topLayout = new QVBoxLayout( this );
    QTabWidget* tabs = new QTabWidget( this );
    konqueror_widget = new Konqueror( tabs );
    konqueror_widget->layout()->setMargin( KDialog::marginHint() );
    connect( konqueror_widget, SIGNAL( changed()), SLOT( changed()));
    tabs->addTab( konqueror_widget, i18n( "Konqueror" ));
    system_widget = new SystemWidget( tabs );
    system_widget->layout()->setMargin( KDialog::marginHint() );
    connect( system_widget, SIGNAL( changed()), SLOT( changed()));
    tabs->addTab( system_widget, i18n( "System" ));
    topLayout->add( tabs );
    load();
    }

void Config::load()
    {
    load( false );
	 }

void Config::load( bool useDefaults)
    {
    konqueror_widget->load( useDefaults );
    system_widget->load( useDefaults );
	 emit changed( useDefaults );
    }

void Config::save()
    {
    konqueror_widget->save();
    system_widget->save();
    }

void Config::defaults()
    {
		 load( true );
    }

KonquerorConfig::KonquerorConfig( QWidget* parent_P, const char* )
    : KCModule( parent_P, "kcmperformance" )
    {
    setQuickHelp( i18n( "<h1>Konqueror Performance</h1>"
        " You can configure several settings that improve Konqueror performance here."
        " These include options for reusing already running instances"
        " and for keeping instances preloaded." ));

    QVBoxLayout *topLayout = new QVBoxLayout( this );
    widget = new Konqueror( this );
    connect( widget, SIGNAL( changed()), SLOT( changed()));
    topLayout->add( widget );
    load();
    }

void KonquerorConfig::load()
    {
		 load( false );
	 }

void KonquerorConfig::load(bool useDefaults)
    {
    widget->load( useDefaults );
	 emit changed( useDefaults );
    }

void KonquerorConfig::save()
    {
    widget->save();
    }

void KonquerorConfig::defaults()
    {
		 load( true );
    }

} // namespace

#include "kcmperformance.moc"
