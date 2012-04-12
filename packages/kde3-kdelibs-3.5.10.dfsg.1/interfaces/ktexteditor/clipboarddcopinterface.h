#ifndef Clipboard_DCOP_INTERFACE_H
#define Clipboard_DCOP_INTERFACE_H

#include <dcopobject.h>
#include <dcopref.h>
#include <qstringlist.h>
#include <qcstring.h>

namespace KTextEditor
{
	class ClipboardInterface;
	/**
	This is the main interface to the ClipboardInterface of KTextEdit.
	This will provide a consistant dcop interface to all KDE applications that use it.
	@short DCOP interface to ClipboardInterface.
	@author Ian Reinhart Geiser <geiseri@kde.org>
	*/
	class KTEXTEDITOR_EXPORT ClipboardDCOPInterface : virtual public DCOPObject
	{
	K_DCOP

	public:
		/**
		Construct a new interface object for the text editor.
		@param Parent the parent ClipboardInterface object
		that will provide us with the functions for the interface.
		@param name the QObject's name
		*/
		ClipboardDCOPInterface( ClipboardInterface *Parent, const char *name );
		/**
		Destructor
		Cleans up the object.
		*/
		virtual ~ClipboardDCOPInterface();
	k_dcop:
	/**
	* copies selected text
	*/
	void copy ( );
	/**
	* cuts selected text
	*/
	void cut ( );
	/**
	* pastes selected text
	*/
	void paste ( );
	private:
		ClipboardInterface *m_parent;
	};
}
#endif


