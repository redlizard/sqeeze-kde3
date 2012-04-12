#ifndef Print_DCOP_INTERFACE_H
#define Print_DCOP_INTERFACE_H

#include <dcopobject.h>
#include <dcopref.h>
#include <qstringlist.h>
#include <qcstring.h>
//#include "editdcopinterface.moc"
namespace KTextEditor
{
	class PrintInterface;
	/**
	This is the main interface to the PrintInterface of KTextEdit.
	This will provide a consistant dcop interface to all KDE applications that use it.
	@short DCOP interface to PrintInterface.
	@author Ian Reinhart Geiser <geiseri@kde.org>
	*/
	class KTEXTEDITOR_EXPORT PrintDCOPInterface : virtual public DCOPObject
	{
	K_DCOP

	public:
		/**
		Construct a new interface object for the text editor.
		@param Parent the parent PrintInterface object
		that will provide us with the functions for the interface.
		@param name the QObject's name
		*/
		PrintDCOPInterface( PrintInterface *Parent, const char *name );
		/**
		Destructor
		Cleans up the object.
		*/
		virtual ~PrintDCOPInterface();
	k_dcop:
		uint printInterfaceNumber () ;
		bool printDialog ();
		bool print ();
	
	private:
		PrintInterface *m_parent;
	};
}
#endif


