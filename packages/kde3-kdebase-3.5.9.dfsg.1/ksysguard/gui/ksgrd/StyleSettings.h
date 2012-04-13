/*
    KSysGuard, the KDE System Guard
   
    Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

    KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>.
    Please do not commit any changes without consulting me first. Thanks!

*/

#ifndef KSG_STYLESETTINGS_H
#define KSG_STYLESETTINGS_H

#include <kdialogbase.h>

#include <qcolor.h>

class KColorButton;

class QListBoxItem;
class QPushButton;

class StyleSettings : public KDialogBase
{
  Q_OBJECT

  public:
    StyleSettings( QWidget *parent = 0, const char *name = 0 );
    ~StyleSettings();

    void setFirstForegroundColor( const QColor &color );
    QColor firstForegroundColor() const;

    void setSecondForegroundColor( const QColor &color );
    QColor secondForegroundColor() const;

    void setAlarmColor( const QColor &color );
    QColor alarmColor() const;

    void setBackgroundColor( const QColor &color );
    QColor backgroundColor() const;

    void setFontSize( uint size );
    uint fontSize() const;

    void setSensorColors( const QValueList<QColor> &list );
    QValueList<QColor> sensorColors();

  private slots:
    void editSensorColor();
    void selectionChanged( QListBoxItem* );

  private:
    KColorButton *mFirstForegroundColor;
    KColorButton *mSecondForegroundColor;
    KColorButton *mAlarmColor;
    KColorButton *mBackgroundColor;

    QSpinBox *mFontSize;

    QListBox *mColorListBox;
    QPushButton *mEditColorButton;
};

#endif
