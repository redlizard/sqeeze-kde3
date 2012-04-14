/*
    Kopete Yahoo Protocol
    Add, remove or move a buddy to the Contactlist

    Copyright (c) 2005 André Duffeck <andre.duffeck@kdemail.net>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef MODIFYBUDDYTASK_H
#define MODIFYBUDDYTASK_H

#include "task.h"

class QString;

/**
@author André Duffeck
*/
class ModifyBuddyTask : public Task
{
public:
	enum Type { AddBuddy, RemoveBuddy, MoveBuddy };
	ModifyBuddyTask(Task *parent);
	~ModifyBuddyTask();
	
	virtual void onGo();
	
	void setType( Type type );
	void setMessage( const QString &text );
	void setTarget( const QString &target );
	void setGroup( const QString &group );
	void setOldGroup( const QString &group );
private:
	void addBuddy();
	void removeBuddy();
	void moveBuddy();

	QString m_message;
	QString m_target;
	QString m_group;
	QString m_oldGroup;
	Type m_type;
};

#endif
