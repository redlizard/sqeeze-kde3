//
// C++ Interface: ksayitfreeverblib
//
// Description: 
//
//
// Author: Robert Vogl <voglrobe@lapislazuli>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef KSAYITFREEVERBLIB_H
#define KSAYITFREEVERBLIB_H

// QT includes
#include <qobject.h>
#include <qstringlist.h>

// KDE includes
#include <ksimpleconfig.h>
#include <kapplication.h>
#include <klibloader.h>
#include <kinstance.h>

#include "ksayit_fxplugin.h"

class FreeverbPluginFactory : public KLibFactory
{
Q_OBJECT
public:
    // Consturctor
    FreeverbPluginFactory(QObject *parent=0, const char* name=0);
    
    // Destructor
    virtual ~FreeverbPluginFactory(){ delete p_instance; };
    
    QObject* createObject(QObject *parent=0, const char* name=0, 
            const char* className="QObject", 
            const QStringList &args=QStringList());        

private:
    static KInstance* p_instance;

};





class FreeverbPlugin : public FXPlugin
{
Q_OBJECT
public: 
    // Constructor
    FreeverbPlugin(QObject *parent=0, const char* name=0); //, KApplication *Appl=0);
    
    // Destructor
    ~FreeverbPlugin();    
    
    /** sets the Main application object
    */
    void setApplication(KApplication *Appl);
    
    /** returns the Name of the Plugin
    */
    QString getName_KS() const;
    
    /** returns a description of the plugin
    */
    QString getDescription_KS() const;
    
    /** shows the GUI to configure the plugin
    */
    bool showGUI_KS();
    
    /** activate the effect
    */
    long activate_KS(KArtsServer *server,
                             StereoEffectStack *fx_stack) const;
    
    /** deactivates the effect
    */
    bool deactivate_KS(StereoEffectStack *fx_stack,
                               long EffectID ) const;
                               

private:
    KApplication *m_Appl;
    KSimpleConfig *m_config;

};
    


#endif

