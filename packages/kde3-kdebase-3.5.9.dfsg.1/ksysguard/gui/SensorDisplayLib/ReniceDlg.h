/*
    KSysGuard, the KDE System Guard

	Copyright (c) 1999 Chris Schlaeger <cs@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

	KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>. Please do
	not commit any changes without consulting me first. Thanks!

*/

#ifndef _ReniceDlg_h_
#define _ReniceDlg_h_

#include <kdialogbase.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlcdnumber.h>
#include <qpushbutton.h>
#include <qslider.h>

class KIntNumInput;

/**
 * This class creates and handles a simple dialog to change the scheduling
 * priority of a process.
 */
class ReniceDlg : public KDialogBase
{
	Q_OBJECT

public:
	ReniceDlg(QWidget* parent, const char* name, int currentPPrio, int pid);

public slots:
	void slotOk();
	void slotCancel();

private:
	QBoxLayout* vLay;
	QBoxLayout* butLay;
	QBoxLayout* sldLay;

	QLabel* message;
	KIntNumInput* input;
};

#endif
