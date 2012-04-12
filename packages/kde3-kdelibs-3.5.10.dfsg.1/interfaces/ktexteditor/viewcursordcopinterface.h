#ifndef ViewCursor_DCOP_INTERFACE_H
#define ViewCursor_DCOP_INTERFACE_H

#include <dcopobject.h>
#include <dcopref.h>
#include <qstringlist.h>
#include <qcstring.h>
//#include "editdcopinterface.moc"
namespace KTextEditor
{
	class ViewCursorInterface;
	/**
	This is the main interface to the ViewCursorInterface of KTextEdit.
	This will provide a consistant dcop interface to all KDE applications that use it.
	@short DCOP interface to ViewCursorInterface.
	@author Ian Reinhart Geiser <geiseri@kde.org>
	*/
	class KTEXTEDITOR_EXPORT ViewCursorDCOPInterface : virtual public DCOPObject
	{
	K_DCOP

	public:
		/**
		Construct a new interface object for the text editor.
		@param Parent the parent ViewCursorInterface object
		that will provide us with the functions for the interface.
		@param name the QObject's name
		*/
		ViewCursorDCOPInterface( ViewCursorInterface *Parent, const char *name );
		/**
		Destructor
		Cleans up the object.
		*/
		virtual ~ViewCursorDCOPInterface();
	k_dcop:

		uint viewCursorInterfaceNumber ();
		/**
		* Get the current cursor coordinates in pixels.
		*/
		class QPoint cursorCoordinates ();

		/**
		* Get the cursor position
		*/
		void cursorPosition (uint line, uint col);

		/**
		* Get the cursor position, calculated with 1 character per tab
		*/
		void cursorPositionReal (uint line, uint col);

		/**
		* Set the cursor position
		*/
		bool setCursorPosition (uint line, uint col);

		/**
		* Set the cursor position, use 1 character per tab
		*/
		bool setCursorPositionReal (uint line, uint col);

		uint cursorLine ();
		uint cursorColumn ();
		uint cursorColumnReal ();
		void cursorPositionChanged ();

	private:
		ViewCursorInterface *m_parent;
	};
}
#endif


