/*
   Copyright (c) 2002 Malte Starostik <malte@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
 
   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

// $Id: ksimmail.h 490596 2005-12-22 13:05:45Z scripty $

#ifndef __ksimmail_h__
#define __ksimmail_h__

#include <pluginmodule.h>

class MailPlugin : public KSim::PluginObject
{
public:
	MailPlugin( const char* name );
	~MailPlugin();

	virtual KSim::PluginView* createView( const char* );
	virtual KSim::PluginPage* createConfigPage( const char* );

	virtual void showAbout();
};

class MailView : public KSim::PluginView
{
	Q_OBJECT
public:
	MailView( KSim::PluginObject* parent, const char* name );
	~MailView();

	virtual void reparseConfig();

private slots:
	void updateDisplay();
};

class MailLabel : public KSim::Label
{
	Q_OBJECT
public:
	MailLabel( QWidget* parent );
	virtual ~MailLabel();

	virtual void configureObject( bool );

protected:
	virtual void paintEvent( QPaintEvent* );

private slots:
	void animation();

private:
	QPixmap frame( const QPixmap& source, int number ) const;

private:
	QPixmap m_envelope;
	QPixmap m_penguin;
	int m_frames;
	int m_delay;
};

class MailConfig : public KSim::PluginPage
{
	Q_OBJECT
public:
	MailConfig( KSim::PluginObject* parent, const char* name );
	~MailConfig();

	virtual void saveConfig();
	virtual void readConfig();
};

#endif

// vim: ts=4 sw=4 noet
