// Copyright (C)  2004  Dominique Devriese <devriese@kde.org>

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301, USA.

#include "filters-common.h"

#include <vector>

#include <qcstring.h>
#include <qstring.h>

#include "../objects/object_calcer.h"
#include "../objects/object_factory.h"

ObjectTypeCalcer* filtersConstructTextObject(
  const Coordinate& c, ObjectCalcer* o,
  const QCString& arg, const KigDocument& doc, bool needframe )
{
  const ObjectFactory* fact = ObjectFactory::instance();
  ObjectCalcer* propo = fact->propertyObjectCalcer( o, arg );
  propo->calc( doc );
  std::vector<ObjectCalcer*> args;
  args.push_back( propo );
  return fact->labelCalcer( QString::fromLatin1( "%1" ), c, needframe,
                            args, doc );
}
