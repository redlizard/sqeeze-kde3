/*
    Copyright (C) 2003 Stefan Westerfeld
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
#ifndef TRADERCHECK_H
#define TRADERCHECK_H

#include "core.h"

namespace Arts {

/**
 * This is an internal class used by artscontrol to check the consistency of
 * the entries in the trader made through mcopclass/mcoptype files
 */
class TraderCheck {
protected:
	bool wroteHeader;
	std::string interfaceName;
	Arts::InterfaceRepoV2 interfaceRepo;

	void check(bool cond, const char *fmt, ...);
	bool haveProperty(Arts::TraderOffer& offer, const std::string& property);
	std::string getSingleProperty(Arts::TraderOffer& offer, const std::string& property);
	bool findFile(const std::vector<std::string> *path, const std::string& filename);
	void collectInterfaces(const std::string& interfaceName, std::map<std::string, int>& i);

public:
	void run();
};

}

#endif

