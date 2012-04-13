    /*

    Copyright (C) 2000 Stefan Westerfeld
                       stefan@space.twc.de

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    */

#ifndef MCOP_LOOPBACKCONNECTION_H
#define MCOP_LOOPBACKCONNECTION_H

#include "connection.h"

#include "arts_export.h"

/*
 * BC - Status (2002-03-08): LoopbackConnection
 *
 * Not part of the public API. Do NOT use it in your apps. No binary
 * compatibility guaranteed.
 */

namespace Arts {

class ARTS_EXPORT LoopbackConnection : public Connection
{
public:
	LoopbackConnection(const std::string& serverID);

	void qSendBuffer(Buffer *buffer);
	void drop();
	bool broken();
};

}

#endif /* MCOP_LOOPBACKCONNECTION_H */
