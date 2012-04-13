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

#ifndef KSG_DANCINGBARS_H
#define KSG_DANCINGBARS_H

#include <SensorDisplay.h>
#include <qbitarray.h>

class KIntNumInput;

class QGroupBox;
class QLineEdit;
class QListViewItem;

class BarGraph;
class DancingBarsSettings;

class DancingBars : public KSGRD::SensorDisplay
{
  Q_OBJECT

  public:
    DancingBars( QWidget *parent = 0, const char *name = 0,
                 const QString &title = QString::null, int min = 0,
                 int max = 100, bool noFrame = false, bool isApplet = false );
    virtual ~DancingBars();

    void configureSettings();

    bool addSensor( const QString &hostName, const QString &name,
                    const QString &type, const QString &title );
    bool removeSensor( uint pos );

    void updateSamples( const QMemArray<double> &samples );

    virtual QSize sizeHint();

    virtual void answerReceived( int id, const QString &answer );

    bool restoreSettings( QDomElement& );
    bool saveSettings( QDomDocument&, QDomElement&, bool save = true );

    virtual bool hasSettingsDialog() const;

  public slots:
    void applySettings();
    virtual void applyStyle();

  protected:
    virtual void resizeEvent( QResizeEvent* );

  private:
    uint mBars;

    BarGraph* mPlotter;

    DancingBarsSettings* mSettingsDialog;

    /**
      The sample buffer and the flags are needed to store the incoming
      samples for each beam until all samples of the period have been
      received. The flags variable is used to ensure that all samples have
      been received.
     */
    QMemArray<double> mSampleBuffer;
    QBitArray mFlags;
};

#endif
