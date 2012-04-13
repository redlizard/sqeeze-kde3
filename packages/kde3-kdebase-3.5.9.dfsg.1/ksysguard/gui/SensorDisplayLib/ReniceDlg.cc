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

#include <klocale.h>
#include <knuminput.h>

#include "ReniceDlg.moc"

ReniceDlg::ReniceDlg(QWidget* parent, const char* name, int currentPPrio,
					 int pid)
	: KDialogBase( parent, name, true, i18n("Renice Process"),
                       KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true )
{
	QWidget *page = new QWidget( this );
	setMainWidget(page);
	vLay = new QVBoxLayout(page, 20, -1, "ReniceLayout");

	QString msg;
	msg = i18n("You are about to change the scheduling priority of\n"
			   "process %1. Be aware that only the Superuser (root)\n"
			   "can decrease the nice level of a process. The lower\n"
			   "the number is the higher the priority.\n\n"
			   "Please enter the desired nice level:").arg(pid);
	message = new QLabel(msg, page);
	message->setMinimumSize(message->sizeHint());
	vLay->addWidget(message);

	/*
	 * Create a slider with an LCD display to the right using a horizontal
	 * layout. The slider and the LCD are kept in sync through signals
	 */
	input = new KIntNumInput(currentPPrio, page, 10);
	input->setRange(-20, 19);
	vLay->addWidget(input);
}

void ReniceDlg::slotOk()
{
    done(input->value());
}

void ReniceDlg::slotCancel()
{
    done(40);
}
