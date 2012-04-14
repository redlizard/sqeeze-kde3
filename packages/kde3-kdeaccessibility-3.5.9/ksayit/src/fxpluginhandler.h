//
// C++ Interface: fxpluginhandler
//
// Description: 
//
//
// Author: Robert Vogl <voglrobe@lapislazuli>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef FXPLUGINHANDLER_H
#define FXPLUGINHANDLER_H

// QT includes
#include <qobject.h>
#include <qlibrary.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>
// #include <qvaluelist.h>

// KDE includes
#include <kconfig.h>
#include <arts/kartsserver.h>
#include <arts/artsflow.h>

using namespace Arts;

// App specific includes
#include "ksayit_fxplugin.h"

/**
@author Robert Vogl
*/
/** Internal structure of the effect plugin
 */
typedef struct {
    QString name; // KService::Ptr->name()
    QString library; // KService::Ptr->library()
    QString description;
    long EffectID; // !=0, if effect is on the effect stack.
    FXPlugin *p; // factory->create(...)
} fx_struct;


class FXPluginHandler : public QObject
{
Q_OBJECT
public:
    FXPluginHandler(QObject *parent = 0, const char *name = 0, KConfig *config=0);
    ~FXPluginHandler();

public: // Methods  
    /** Loads the activated plugins.
     */
    void readConfiguration();

    /** Loads any available plugin from the directories containing KSayIt plugins,
     *  fills the plugin_struct with the corresponding data and unloads the plugin.
     */
    void searchPlugins();
    
    /** Shows the GUI to setup the effect named name
    \param pname The name of the effect to configure.
     */ 
    void showEffectGUI(const QString &pname);
    
    /** Activates the named effect
    \param pname Name of the effect to activate.
    \param server A pointer to the soundserver instance.
    \param fx_stack A pointer to the effect stack of the soundserver.
     */
    void activateEffect(const QString &pname,
                        KArtsServer *server,
                        StereoEffectStack *fx_stack);

    /** Removes all effects from the soundservers effect stack.
    \param fx_stack A pointer to the effect stack of the soundserver.
     */
    void deactivateEffects(StereoEffectStack *fx_stack);
    
    /** Returns a list of all available plugins.
    \param pluginlist A reference to a QStringList that contains the available plugins.
     */
    void getPlugins(QStringList &pluginlist); 

private: // Attributes
    KConfig *m_config;
    QMap<QString, fx_struct> m_mapPluginList; // holds all plugins found on the system
    // QMap<QString, fx_struct> m_mapActivePlugins; // holds the active effects
    QStringList m_lstActivePlugins;
          

};



#endif
