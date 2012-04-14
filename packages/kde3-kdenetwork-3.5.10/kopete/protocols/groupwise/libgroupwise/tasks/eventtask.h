/*
    Kopete Groupwise Protocol
    eventtask.h - Ancestor of all Event Handling tasks

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Iris, Copyright (C) 2003  Justin Karneges

    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef GW_EVENTTASK_H
#define GW_EVENTTASK_H

#include <qvaluelist.h>

#include "eventtransfer.h"
#include "task.h"

class Transfer;

class EventTask : public Task
{
Q_OBJECT
	public:
		EventTask( Task *parent );
	protected:
		bool forMe( Transfer * transfer, EventTransfer *& event ) const;
		void registerEvent( GroupWise::Event e );
	private:
		QValueList<int> m_eventCodes;
};

#endif
