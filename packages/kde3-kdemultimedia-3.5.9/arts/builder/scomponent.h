    /*

    Copyright (C) 1999 Stefan Westerfeld
                       stefan@space.twc.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    */

#ifndef __SCOMPONENT_H__
#define __SCOMPONENT_H__

#include <qstring.h>
#include <list>

class ModulePort;
class QPainter;
class QPixmap;

class StructureCanvas {
public:
	virtual void redrawRect(int x, int y, int width, int height) = 0;
};

class StructureComponent {
protected:
	StructureCanvas *canvas;

	int _x, _y;
	bool _selected, _visible;

	virtual bool moveInternal(int x, int y) = 0;

public:
	StructureComponent(StructureCanvas *canvas);
	virtual ~StructureComponent();

	// type

	enum ComponentType { ctModule, ctPort };
	virtual ComponentType type() = 0;
	
	// TODO: connection & autorouter

	virtual ModulePort *portAt(int segment, int x, int y) = 0;
	virtual void dumpPorts(std::list<ModulePort *>& ports) = 0;

	// visibility
	bool visible();
	void show();
	void hide();
	void redraw();

	// drawing
	virtual bool drawNeedsBackground(int segment) = 0;
	virtual void drawSegment(QPainter *dest, int cellsize, int segment) = 0;
	virtual QPixmap *pixmap() = 0;
	virtual QString name() = 0;

	// space checking & positioning (dimensions in cells)
	bool move(int x, int y);
	int x() const;
	int y() const;
	virtual int width() const = 0;
	virtual int height() const = 0;

	// selection
	bool selected();
	void setSelected(bool newselection);

	// creation and destruction are handled via standard
	// constructors/destructors
};

#endif
