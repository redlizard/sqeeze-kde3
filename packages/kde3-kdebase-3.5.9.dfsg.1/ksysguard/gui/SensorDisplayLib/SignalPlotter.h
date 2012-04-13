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

#ifndef KSG_SIGNALPLOTTER_H
#define KSG_SIGNALPLOTTER_H

#include <qptrlist.h>
#include <qstring.h>
#include <qvaluelist.h>
#include <qwidget.h>

#define GRAPH_POLYGON     0
#define	GRAPH_ORIGINAL    1

class QColor;

class SignalPlotter : public QWidget
{
  Q_OBJECT

  public:
    SignalPlotter( QWidget *parent = 0, const char *name = 0 );
    ~SignalPlotter();

    bool addBeam( const QColor &color );
    void addSample( const QValueList<double> &samples );

    void removeBeam( uint pos );

    void changeRange( int beam, double min, double max );

    QValueList<QColor> &beamColors();

    void setTitle( const QString &title );
    QString title() const;

    void setUseAutoRange( bool value );
    bool useAutoRange() const;

    void setMinValue( double min );
    double minValue() const;

    void setMaxValue( double max );
    double maxValue() const;

    void setGraphStyle( uint style );
    uint graphStyle() const;

    void setHorizontalScale( uint scale );
    int horizontalScale() const;

    void setShowVerticalLines( bool value );
    bool showVerticalLines() const;

    void setVerticalLinesColor( const QColor &color );
    QColor verticalLinesColor() const;

    void setVerticalLinesDistance( int distance );
    int verticalLinesDistance() const;

    void setVerticalLinesScroll( bool value );
    bool verticalLinesScroll() const;

    void setShowHorizontalLines( bool value );
    bool showHorizontalLines() const;

    void setHorizontalLinesColor( const QColor &color );
    QColor horizontalLinesColor() const;

    void setHorizontalLinesCount( int count );
    int horizontalLinesCount() const;

    void setShowLabels( bool value );
    bool showLabels() const;

    void setShowTopBar( bool value );
    bool showTopBar() const;

    void setFontSize( int size );
    int fontSize() const;

    void setBackgroundColor( const QColor &color );
    QColor backgroundColor() const;
    void reorderBeams( const QValueList<int>& newOrder );

    void setThinFrame(bool set) { mShowThinFrame = set; }

  protected:
    void updateDataBuffers();

    virtual void resizeEvent( QResizeEvent* );
    virtual void paintEvent( QPaintEvent* );

  private:
    double mMinValue;
    double mMaxValue;
    bool mUseAutoRange;
    bool mShowThinFrame;

    uint mGraphStyle;

    bool mShowVerticalLines;
    QColor mVerticalLinesColor;
    uint mVerticalLinesDistance;
    bool mVerticalLinesScroll;
    uint mVerticalLinesOffset;
    uint mHorizontalScale;

    bool mShowHorizontalLines;
    QColor mHorizontalLinesColor;
    uint mHorizontalLinesCount;

    bool mShowLabels;
    bool mShowTopBar;
    uint mFontSize;

    QColor mBackgroundColor;

    QPtrList<double> mBeamData;
    QValueList<QColor> mBeamColor;

    unsigned int mSamples;

    QString mTitle;
};

#endif
