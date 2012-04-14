/*
    videodevicepool.h  -  Kopete Multiple Video Device handler Class

    Copyright (c) 2005-2006 by Cláudio da Silveira Pinheiro   <taupter@gmail.com>

    Kopete    (c) 2002-2003      by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "videodevicemodelpool.h"

namespace Kopete {

namespace AV {

VideoDeviceModelPool::VideoDeviceModelPool()
{
}


VideoDeviceModelPool::~VideoDeviceModelPool()
{
}

void VideoDeviceModelPool::clear()
{
	m_devicemodel.clear();
}

size_t VideoDeviceModelPool::size()
{
	return m_devicemodel.size();
}

size_t VideoDeviceModelPool::addModel( QString newmodel )
{
	VideoDeviceModel newdevicemodel;
	newdevicemodel.model=newmodel;
	newdevicemodel.count=0;

	if(m_devicemodel.size())
	{
		for ( size_t loop = 0 ; loop < m_devicemodel.size(); loop++)
		if (newmodel == m_devicemodel[loop].model)
		{
			kdDebug() << k_funcinfo << "Model " << newmodel << " already exists." << endl;
			m_devicemodel[loop].count++;
			return m_devicemodel[loop].count;
		}
	}
	m_devicemodel.push_back(newdevicemodel);
	m_devicemodel[m_devicemodel.size()-1].model = newmodel;
	m_devicemodel[m_devicemodel.size()-1].count = 0;
	return 0;
}


}

}
