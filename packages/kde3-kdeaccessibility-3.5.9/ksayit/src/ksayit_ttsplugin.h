//
// C++ Interface: ksayit_ttsplugin
//
// Description: 
//
//
// Author: Robert Vogl <voglrobe@lapislazuli>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef KSAYIT_TTSPLUGIN
#define KSAYIT_TTSPLUGIN

// QT includes
#include <qobject.h>
#include <qwidget.h>
#include <qframe.h>
#include <qstring.h>


// KDE includes
#include <kapplication.h>

// App specific includes


#include "kttsdlib.h"

/** TTSPlugin is an abstract class to integrate various TTS systems as plugins
 *  (Shared Objects) into KSayIt.
 *  If you would like to implement a plugin, simply make a class
 *  inherited from TTSPlugin, include 'ksayit_ttsplugin.h' and reimplement all the
 *  pure virtual functions provided herein.
 *  In addition you must implement two class factories:\n
 *  \p createPlugin() returns a pointer to an instance of your class.
 *  The Plugin Handler of KSayIt calls this function with a parameter pointing
 *  to the main application instance, as delivered by \p KApplication::kApplication().
 *  This pointer can be used for any reason i.e. to install a Qt translator.\n
 *  An instance of your class should be deleted by use of \p destroyPlugin().\n
 *  Example:
 \code
    extern "C"
    {
        TTSPlugin* createPlugin(KApplication *Appl)
        {
            return new MyNewPlugin(Appl);
        }
        
        void destroyPlugin(TTSPlugin* p)
        {
            delete p;
        }
    };
 \endcode
 *  KSayIt expects two Qt-signals from the plugin:\n
 *  \p signalPluginFinished() must be emitted when the plugin is finished,
 *  i.e. a task to speak text has been finished.\n
 *  \p signalPluginFailed() is optional and shall emitted if the processing
 *  of the plugin has been failed by any reason.
 \author Robert Vogl
 */

class TTSPlugin : public QObject
{
protected:
    TTSPlugin(QObject *parent, const char *name) : QObject(parent, name){};
    
public:
    /** Returns the name of the plugin. This name is the unique identifier
     *  for the plugin. A expressive name is recommended because this name
     *  may be shown to the user, i.e. in a configuration dialog.
     *  The PluginHandler internally references to each TTS plugin by this name.\n
     *  Has to be reimplemented by the plugin implementation.
     */
    virtual QString getName_KS() const = 0;
    
    /** Returns the description of the plugin.\n
     *  Has to be reimplemented by the plugin implementation.
     */
    virtual  QString getDescription_KS() const = 0;
    
    /** Returns the supported control actions of the plugin.
     *  It is represented as an OR'ed value of enum \pACTIONS. 
     */
    virtual int getActions_KS() = 0;
     
    /** Returnes a pointer to the GUI widget to configure the plugin.\n
     *  Will be deleted by the PluginHandler.\n
     *  \param frame A pointer to the QFrame object in which the dialog will
     *  be embedded.
     */
    virtual const QWidget* getGUI_KS(QFrame *frame) = 0;
       
    /** Let the plugin (re)load its configuration
     */
    virtual void reloadConfiguration_KS() = 0;
     
    /** Called from the Plugin Handler if the "OK" Button of the main
     *  configuartion dialog was clicked.\n
     *  Returns false if something went wrong otherwise true.
     */
    virtual bool saveWasClicked_KS() const = 0;
    
    /** This method speeches the given Text.\n
     *  If the used TTS system outputs to a sound file
     *  (i.e. a .wav file) it may return its path and filename to KSayIt. 
     *  In this case, KSayIt uses a built-in audio player to play back
     *  the file via aRts.\n
     *  If this plugin provides its own audio output mechanisms, then return
     *  \p QString::null.\n
     *  The TTS processing shall be implemented non-blocking, i.e. this function has
     *  to return a valid string as soon as possible, before the typically time
     *  consuming TTS processing starts. The synchronization with KSayIt shall
     *  be performed by the status flags (see \p getStatus_KS()).
     *  \param text The text to speach. 
     */
    virtual QString sayText_KS(const QString &text) = 0;
    
    /** Returns an OR'ed value of status bits of the plugin.\n
     *  Currently only \p TTS::AUDIOFILE is defined.\n This
     *  bit indicates that the plugin is configured to output
     *  to an audiofile.\n
     *  If configurable, \p TTS::AUDIOFILE must toggle synchronously with
     *  the setup widget.
     *  This flag must have a valid value during the overall lifetime
     *  of the plugin object. 
     */
    virtual int getStatus_KS() const = 0;
    
    /** Processes the stop event from KSayIt.
     */
    virtual void stop_KS() = 0;
    
    /** Processes the pause event from KSayIt.
     */
    virtual void pause_KS() = 0;
    
    /** Processes resume event after pause from KSayIt.
     */
    virtual void resume_KS() = 0;
    
    /** May be used as a fast forward function, e.g.
     *  jump to the next sentence, paragraph or whatever.
     */
    virtual void ffwd_KS() = 0;
    
    /** May be used as a rewind function, e.g.
     *  jumb back to the beginning of the current paragraph,
     *  repeat previous sentence or whatever.
     */
    virtual void frev_KS() = 0;

};

// Types of the class factories
typedef TTSPlugin* (*create_ttspi)(KApplication *Appl);
typedef void (*destroy_ttspi)(TTSPlugin *p);




#endif


