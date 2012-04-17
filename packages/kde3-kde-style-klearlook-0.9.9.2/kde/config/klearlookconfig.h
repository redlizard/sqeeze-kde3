/* $Id: klearlookconfig.h,v 1.6 2005/12/13 10:55:38 jck Exp $
*/

#ifndef __KLEARLOOKCONFIG_H__
#define __KLEARLOOKCONFIG_H__

/*
 
  Klearlook (C) Joerg C. Koenig, 2005 jck@gmx.org
 
----
 
  Based upon QtCurve (C) Craig Drummond, 2003 Craig.Drummond@lycos.co.uk
      Bernhard Rosenkränzer <bero@r?dh?t.com>
      Preston Brown <pbrown@r?dh?t.com>
      Than Ngo <than@r?dh?t.com>
 
  Released under the GNU General Public License (GPL) v2.
 
  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files(the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:
 
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
 
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.
*/


#include <klearlookconfigbase.h>
#include "../common.h"

class KlearlookConfig : public KlearlookConfigBase {
        Q_OBJECT

    public:

        KlearlookConfig( QWidget *parent );
        ~KlearlookConfig();

    signals:

        void changed( bool );

    public slots:

        void save();
        void defaults();

    private slots:

        void borderToggled();
        void roundToggled();
        void updateChanged();
        void emboldenToggled();
        void dbiChanged();
        void lvChanged();

    private:
        int popupmenuHighlightLevel;
        bool border,
        icons,
        darkMenubar,
        round,
        gradientPM,
        vArrow,
        embolden,
        crLabelHighlight,
        lvDark,
        borderSplitter;
        ETBarBorder toolbarBorders;
        EDefBtnIndicator defBtnIndicator;
        EGroove sliderThumbs,
        handles;
        ELvExpander lvExpander;
        ELvLines lvLines;
        EProfile pm;
        EAppearance appearance;
};

#endif
