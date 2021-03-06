/**
 * Copyright (c) 1999 Matthias Elter (me@kde.org)
 * Copyright (c) 2001-2002 Igor Jansen (rm@kde.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KO_HSV_WIDGET_H
#define KO_HSV_WIDGET_H

#include "qwidget.h"
#include "kdualcolorbutton.h"

#include "koColor.h"

#include <koffice_export.h>

class KDualColorButton;
class KValueSelector;
class KoColorWheel;
class KoColorSlider;
class KoFrameButton;
class QGridLayout;
class QLabel;
class QSpinBox;

class KoHSVWidget
     : public QWidget
{
    Q_OBJECT
    typedef QWidget super;

public:
    KoHSVWidget(QWidget *parent = 0L, const char *name = 0);
    virtual ~KoHSVWidget() {}

public slots:
    /**
     * Set the current color to c. Do not emit the color changed signals
     */
    virtual void setFgColor(const QColor & c);
    virtual void setBgColor(const QColor & c);
    /// Sets the current mode (foreground/background) without emitting the signal
    virtual void setMode(KDualColorButton::DualColor);
signals:

    /**
     * Emitted when the current color is changed.
     */
    virtual void sigFgColorChanged(const QColor & c);
    virtual void sigBgColorChanged(const QColor & c);
    /// Emitted when the mode (foreground/background) was changed
    virtual void sigModeChanged(KDualColorButton::DualColor);

protected slots:

    virtual void slotHChanged(int h);
    virtual void slotSChanged(int s);
    virtual void slotVChanged(int v);
    virtual void slotWheelChanged(const KoColor& c);

    void slotFGColorSelected(const QColor& c);
    void slotBGColorSelected(const QColor& c);
    void currentChanged(KDualColorButton::DualColor);

private:
    void changedFgColor();
    void changedBgColor();

    void update(const KoColor & fgColor, const KoColor & bgColor);

    KoColorWheel *m_colorwheel;
    KValueSelector *m_VSelector;
    QLabel *mHLabel;
    QLabel *mSLabel;
    QLabel *mVLabel;
    QSpinBox *mHIn;
    QSpinBox *mSIn;
    QSpinBox *mVIn;
    KDualColorButton *m_ColorButton;

    KoColor m_fgColor;
    KoColor m_bgColor;

    bool m_autovalue;
};

#endif
