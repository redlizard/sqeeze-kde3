#ifndef %{APPNAME}_MENU_APPLET_H
#define %{APPNAME}_MENU_APPLET_H

#include <qpe/menuappletinterface.h>

#include <qobject.h>

class %{APPNAME} : public QObject, public MenuAppletInterface
{

    Q_OBJECT

public:
    %{APPNAME} ( );
    virtual ~%{APPNAME} ( );

    QRESULT queryInterface( const QUuid&, QUnknownInterface** );
    Q_REFCOUNT

    virtual int position() const;
    
    virtual QString name ( ) const;
    virtual QIconSet icon ( ) const;
    virtual QString text ( ) const;
    /* virtual QString tr( const char* ) const;
    virtual QString tr( const char*, const char* ) const;
    */
    virtual QPopupMenu *popup ( QWidget *parent ) const;
    virtual void activated ( );
};

#endif
