/***************************************************************************
                               sidebarextension.h
                             -------------------
    begin                : Sun July 20 16:00:00 CEST 2003
    copyright            : (C) 2003 Joseph Wenninger
    email                : jowenn@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef SIDEBAREXTENSION_H
#define SIDEBAREXTENSION_H

#include <kpanelextension.h>
#include <kurl.h>
#include <kparts/browserextension.h>

class QHBoxLayout;
class QVBox;

class SidebarExtension : public KPanelExtension
{
    Q_OBJECT

public:
    SidebarExtension( const QString& configFile,
                     Type t = Normal,
                     int actions = 0,
                     QWidget *parent = 0, const char *name = 0 );

    virtual ~SidebarExtension();

    QSize sizeHint( Position, QSize maxSize ) const;
    Position preferedPosition() const;

    virtual void positionChange( Position position );

protected:
    virtual void about();
    virtual void preferences();
    virtual bool eventFilter( QObject *o, QEvent *e );
protected slots:
    void openURLRequest( const KURL &, const KParts::URLArgs &);
    void needLayoutUpdate(bool);

private:
    int m_currentWidth;
    int m_x;
    QFrame *m_resizeHandle;
    bool m_resizing;
    int m_expandedSize;
    QHBoxLayout *m_layout;
    QVBox *m_sbWrapper;
};

#endif

