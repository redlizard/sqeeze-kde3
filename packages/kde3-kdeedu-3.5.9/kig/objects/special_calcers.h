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

#ifndef KIG_OBJECTS_SPECIAL_CALCERS_H
#define KIG_OBJECTS_SPECIAL_CALCERS_H

class MeasureTransportCalcer
  : public ObjectCalcer
{
  ObjectCalcer* mcircle;
  ObjectCalcer* mpoint;
  ObjectCalcer* msegment;
  ObjectImp* mimp;
public:
  MeasureTransportCalcer(ObjectCalcer* circle, ObjectCalcer* point, ObjectCalcer* segment );
  ~MeasureTransportCalcer();

  std::vector<ObjectCalcer*> parents() const;
  void calc( const KigDocument& );
  const ObjectImpType* impRequirement(
    ObjectCalcer* o, const std::vector<ObjectCalcer*>& os ) const;
};

#endif
