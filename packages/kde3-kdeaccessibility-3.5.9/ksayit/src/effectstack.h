//
// C++ Interface: effectstack
//
// Description: 
//
//
// Author: Robert Vogl <voglrobe@lapislazuli>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef EFFECTSTACK_H
#define EFFECTSTACK_H

// KDE includes
#include <arts/kmedia2.h>
#include <kconfig.h>

#include <arts/kartsdispatcher.h>
#include <arts/kartsserver.h>
#include <arts/soundserver.h>


namespace Arts
{
    class SoundServerV2;
}
using namespace Arts;

// forward declarations
class FXPluginHandler;

/**
@author Robert Vogl
*/
class EffectStack{
public:
    EffectStack(FXPluginHandler *pluginhandler=0, KConfig *config=0);
    ~EffectStack();
    
    // KDE::PlayObject *playobj;
    KArtsDispatcher dispatcher;
    KArtsServer server;   
    StereoEffectStack FX_Stack;

public: //Methods
    bool loadEffects();
    bool unloadEffects();

private:
    FXPluginHandler *m_pluginhandler;
    KConfig *m_config;


};

#endif
