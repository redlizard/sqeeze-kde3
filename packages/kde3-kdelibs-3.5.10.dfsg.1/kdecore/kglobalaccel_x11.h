/* This file is part of the KDE libraries
    Copyright (C) 2001,2002 Ellis Whitehead <ellis@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef _KGLOBALACCEL_X11_H
#define _KGLOBALACCEL_X11_H

#include <qmap.h>
#include <qwidget.h>

#include "kaccelbase.h"
#include "kkeyserver_x11.h"
#include "kshortcut.h"

/**
 * @internal
 */
class KGlobalAccelPrivate : public QWidget, public KAccelBase
{
	friend class KGlobalAccel;
	Q_OBJECT
 public:
	KGlobalAccelPrivate();
	virtual ~KGlobalAccelPrivate();

	virtual void setEnabled( bool bEnabled );

	virtual bool emitSignal( Signal signal );
	virtual bool connectKey( KAccelAction& action, const KKeyServer::Key& key );
	virtual bool connectKey( const KKeyServer::Key& key );
	virtual bool disconnectKey( KAccelAction& action, const KKeyServer::Key& key );
	virtual bool disconnectKey( const KKeyServer::Key& key );

 protected:
	/**
	 * @internal
	 * Represents a key code and modifier combination.
	 */
	class CodeMod
	{
	 public:
	  /**
	   * The key code of the CodeMod.
	   */
		uchar code;
	  /**
	   * The modifier flags of the CodeMod.
	   */
		uint mod;
	
		/**
		 * Compares two CodeMods. 
		 */
		bool operator < ( const CodeMod& b ) const
		{
			if( code < b.code ) return true;
			if( code == b.code && mod < b.mod ) return true;
			return false;
		}
	};
	typedef QMap<CodeMod, KAccelAction*> CodeModMap;

	CodeModMap m_rgCodeModToAction;

	/**
	 * @param bGrab Set to true to grab key, false to ungrab key.
	 */
	bool grabKey( const KKeyServer::Key&, bool bGrab, KAccelAction* );

	/**
	 * Filters X11 events ev for key bindings in the accelerator dictionary.
	 * If a match is found the activated activated is emitted and the function
	 * returns true. Return false if the event is not processed.
	 *
	 * This is public for compatibility only. You do not need to call it.
	 */
	virtual bool x11Event( XEvent* );
	void x11MappingNotify();
	bool x11KeyPress( const XEvent *pEvent );
	void activate( KAccelAction* pAction, const KKeySequence& seq );
	virtual bool isEnabledInternal() const;
	static void blockShortcuts( bool block );
	void disableBlocking( bool disable );
        void suspend( bool s );

 protected slots:
	void slotActivated( int iAction );
 private:
	bool m_blocked;
	bool m_blockingDisabled;
        bool m_suspended;
};

#endif // _KGLOBALACCEL_X11_H
