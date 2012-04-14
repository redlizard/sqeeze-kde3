//
// C++ Implementation: KSayItFXPlugin
//
// Description: 
//
//
// Author: Robert Vogl <voglrobe@lapislazuli>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

// QT includes
// #include <qtextcodec.h>
#include <qstringlist.h>

// KDE includes
#include <kdebug.h>
#include <klocale.h>
#include <kdemacros.h>
#include <arts/artsmodules.h>

// App specific includes
#include "ksayitfreeverblib.h"
#include "freeverbsetupimpl.h"


// The class factory
K_EXPORT_COMPONENT_FACTORY( libFreeverb_plugin, FreeverbPluginFactory )
/** replaced by macro
extern "C"
{
    void* init_libFreeverb_plugin()
    {
        return new FreeverbPluginFactory;    
    }
};
*/

KInstance *FreeverbPluginFactory::p_instance = 0L;


// Factory Constructor
FreeverbPluginFactory::FreeverbPluginFactory(QObject *parent, const char* name)
{
    p_instance = new KInstance("FreeverbPluginFactory");
}

    
QObject* FreeverbPluginFactory::createObject(QObject *parent, const char* name, 
            const char*, 
            const QStringList &)
{
    kdDebug(100200) << "FreeverbPluginFactory::createObject()" << endl;
    
    QObject* obj = new FreeverbPlugin( parent, name );
    emit objectCreated( obj );
    return obj;  
}        




// Plugin Constructor
FreeverbPlugin::FreeverbPlugin(QObject *parent, const char* name) //, KApplication *Appl)
 : FXPlugin(parent, name) //, m_Appl(Appl)
{
    m_config = new KSimpleConfig("ksayit_freeverbrc");
    
}

FreeverbPlugin::~FreeverbPlugin()
{
    delete m_config;
}

/** sets the Main application object
*/
void FreeverbPlugin::setApplication(KApplication *Appl)
{
    m_Appl = Appl;
}


/** returns the Name of the Plugin
*/
QString FreeverbPlugin::getName_KS() const
{
    return "Synth_FREEVERB";
}

/** returns a description of the plugin
*/
QString FreeverbPlugin::getDescription_KS() const
{
    return i18n("This is a freeverb effect.");
}

/** shows the GUI to configure the plugin
*/
bool FreeverbPlugin::showGUI_KS()
{     
    FreeverbSetupImpl *dlg = new FreeverbSetupImpl(0, "Freeverb", true, m_config);
    if ( !dlg ){
        delete dlg;
        return false;
    }
    dlg->exec();
    delete dlg;
     
    return true;
}

/** activate the effect
*/
long FreeverbPlugin::activate_KS(KArtsServer *server,
                            StereoEffectStack *fx_stack) const
{
    // kdDebug(100200) << "Aktiviere Synth_FREEVERB-Effekt" << endl;
    Synth_FREEVERB fv = DynamicCast( server->server().createObject("Arts::Synth_FREEVERB") );
    if ( !fv.isNull() ){
        // kdDebug(100200) << "Filter angelegt" << endl;
        // get filter parameter
        m_config->setGroup("Synth_FREEVERB");            
        fv.roomsize( m_config->readDoubleNumEntry("roomsize", 50.0)/100.0 );
        fv.damp    ( m_config->readDoubleNumEntry("damp", 50.0    )/100.0 );
        fv.wet     ( m_config->readDoubleNumEntry("wet", 50.0     )/100.0 );
        fv.dry     ( m_config->readDoubleNumEntry("dry", 50.0     )/100.0 );
        fv.width   ( m_config->readDoubleNumEntry("width", 50.0   )/100.0 );
        
        fv.start();
        return fx_stack->insertBottom( fv, "Freeverb" );
    }
    return 0;
}

/** deactivates the effect
*/
bool FreeverbPlugin::deactivate_KS(StereoEffectStack *fx_stack,
                            long EffectID ) const
{
    // kdDebug(100200) << "Deaktiviere Synth_FREEVERB-Effekt" << endl;
    if (EffectID!=0){
        fx_stack->remove(EffectID);
    }
    return true;
}
















#include "ksayitfreeverblib.moc"
