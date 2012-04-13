/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kcmdisplay.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 *
 * You can Freely distribute this program under the GNU General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#ifndef __Bgnd_h_Included__
#define __Bgnd_h_Included__

#include <kcmodule.h>

class KConfig;
class BGDialog;

/**
 * The Desktop/Background tab in kcontrol.
 */
class KBackground: public KCModule
{
    Q_OBJECT

public:
    KBackground(QWidget *parent, const char *name, const QStringList &);
    ~KBackground();

    virtual void load();
    virtual void load( bool useDefaults );
    virtual void save();
    virtual void defaults();

private:
    BGDialog      *m_base;
    KConfig       *m_pConfig;
};


#endif // __Bgnd_h_Included__
