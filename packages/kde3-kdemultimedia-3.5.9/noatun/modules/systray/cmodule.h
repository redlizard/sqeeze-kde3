// cmodule.h
//
// Copyright (C) 2001 Neil Stevens <multivac@fcmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// THE AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
// AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// Except as contained in this notice, the name(s) of the author(s) shall not be
// used in advertising or otherwise to promote the sale, use or other dealings
// in this Software without prior written authorization from the author(s).

#ifndef CMODULE_H
#define CMODULE_H

#include <qmap.h>
#include <noatun/pref.h>

class YHConfigWidget;

class YHModule : public CModule
{
Q_OBJECT
	public:
		YHModule(QObject *_parent);
		virtual void save();
		virtual void reopen();

	signals:
		void saved();

	private:
		YHConfigWidget *mWidget;
		QMap<int, int> mActionMap;

	private slots:
		void slotUsePopupToggled(bool on);
		void slotModifierActivated(int index);
		void slotMwheelClicked(int index);
};

#endif
