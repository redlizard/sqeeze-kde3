/* This file is part of the KDE project
   Copyright (C) 2002, The Karbon Developers

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
 * Boston, MA 02110-1301, USA.
*/

#ifndef __EPSEXPORT_H__
#define __EPSEXPORT_H__


#include <KoFilter.h>

#include "vvisitor.h"

class QTextStream;
class VColor;
class VPath;
class VDocument;
class VFill;
class VGroup;
class VLayer;
class VSubpath;
class VStroke;
class VText;


class EpsExport : public KoFilter, private VVisitor
{
	Q_OBJECT

public:
	EpsExport( KoFilter* parent, const char* name, const QStringList& );
	virtual ~EpsExport() {}

	virtual KoFilter::ConversionStatus convert( const QCString& from, const QCString& to );

private:
	virtual void visitVPath( VPath& composite );
	virtual void visitVDocument( VDocument& document );
	virtual void visitVSubpath( VSubpath& path );
	virtual void visitVText( VText& text );
	virtual void visitVGroup( VGroup& group );
	virtual void visitVLayer( VLayer& layer );

	void getStroke( const VStroke& stroke );
	void getFill( const VFill& fill );
	void getColor( const VColor& color );

	bool isVisible( const VObject* object ) const;

	QTextStream* m_stream;

	uint m_psLevel;
	bool m_exportHidden;
};

#endif

