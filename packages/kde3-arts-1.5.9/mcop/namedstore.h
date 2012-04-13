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

#ifndef ARTS_NAMEDSTORE_H
#define ARTS_NAMEDSTORE_H

#include <string>
#include <list>
#include <vector>
#include "stdio.h"

/*
 * BC - Status (2002-03-08): NamedStore
 *
 * None of these classes is considered part of the public API. Do NOT use it
 * in your apps. These are part of the implementation of object.cc and not
 * to be used elsewhere.
 */

namespace Arts {

/**
 * -- internal class --
 * 
 * this stores key-value pairs, where key is a string which is kept unique
 */
template<class T>
class NamedStore
{
private:
	class Element {
	public:
		T t;
		std::string name;

		Element(const T& t, const std::string& name) :t(t), name(name) { }
	};
	typedef std::list<Element> Element_list;
	Element_list elements;

public:
	bool get(const std::string& name, T& result)
	{
		typename Element_list::iterator i;
		for(i = elements.begin(); i != elements.end(); i++)
		{
			if(i->name == name)
			{
				result = i->t;
				return true;
			}
		}

		return false;
	}
	bool remove(const std::string& name)
	{
		typename Element_list::iterator i;
		for(i = elements.begin(); i != elements.end(); i++)
		{
			if(i->name == name)
			{
				elements.erase(i);
				return true;
			}
		}
		return false;
	}
	std::vector<std::string> *contents()
	{
		std::vector<std::string> *result = new std::vector<std::string>;

		typename Element_list::iterator i;
		for(i = elements.begin(); i != elements.end(); i++)
			result->push_back(i->name);

		return result;
	}
	std::string put(const std::string& name, const T& t)
	{
		std::string xname = name;
		int append = 1;

		for(;;)
		{
			typename Element_list::iterator i;
		
			i = elements.begin();
			while(i != elements.end() && i->name != xname)
				i++;

			if(i == elements.end())
			{
				elements.push_back(Element(t,xname));
				return xname;
			}

			char buffer[1024];
			sprintf(buffer,"%d",append++);
			xname = name + std::string(buffer);
		}
	}
};

}
#endif /* ARTS_NAMEDSTORE_H */
