/* This file is part of the KDE libraries
 * Copyright (c) 1998 Stefan Taferner
 *                       2001/2003 thierry lorthiois (lorthioist@wanadoo.fr)
 * With the help of WMF documentation by Caolan Mc Namara

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#ifndef _KOWMFREADPRIVATE_H_
#define _KOWMFREADPRIVATE_H_

#include <qpen.h>
#include <qcolor.h>
#include <qfont.h>
#include <qrect.h>
#include <qregion.h>
#include <qbuffer.h>
#include <qstring.h>

#include "kowmfstruct.h"
#include "kowmfstack.h"

class KoWmfRead;

/**
 * KoWmfReadPrivate allows to read WMF files
 *
 */

class KoWmfReadPrivate
{
public:
    KoWmfReadPrivate();
    virtual ~KoWmfReadPrivate();

    /**
     * Load WMF file. Returns true on success.
     */
    bool load( const QByteArray& array );

    /**
     * Plays a metafile in @p readKwmf and returns true on success.
     * To draw on a device you have to inherit the class ReadKwmf
     */
    bool play( KoWmfRead* readKwmf );


    /* Metafile painter methods */

    /// set window origin
    void setWindowOrg( Q_UINT32 size, QDataStream& stream );
    /// set window extents
    void setWindowExt( Q_UINT32 size, QDataStream& stream );
    /// scale window extents
    void ScaleWindowExt( Q_UINT32, QDataStream& stream );
    /// offset in window origin
    void OffsetWindowOrg( Q_UINT32, QDataStream& stream );

    /****************** Drawing *******************/
    /// draw line to coord
    void lineTo( Q_UINT32 size, QDataStream& stream );
    /// move pen to coord
    void moveTo( Q_UINT32 size, QDataStream& stream );
    /// draw ellipse
    void ellipse( Q_UINT32 size, QDataStream& stream );
    /// draw polygon
    void polygon( Q_UINT32 size, QDataStream& stream );
    /// draw a list of polygons
    void polyPolygon( Q_UINT32 size, QDataStream& stream );
    /// draw series of lines
    void polyline( Q_UINT32 size, QDataStream& stream );
    /// draw a rectangle
    void rectangle( Q_UINT32 size, QDataStream& stream );
    /// draw round rectangle
    void roundRect( Q_UINT32 size, QDataStream& stream );
    /// draw arc
    void arc( Q_UINT32 size, QDataStream& stream );
    /// draw chord
    void chord( Q_UINT32 size, QDataStream& stream );
    /// draw pie
    void pie( Q_UINT32 size, QDataStream& stream );
    /// set polygon fill mode
    void setPolyFillMode( Q_UINT32 size, QDataStream& stream );
    /// set background pen color
    void setBkColor( Q_UINT32 size, QDataStream& stream );
    /// set background pen mode
    void setBkMode( Q_UINT32 size, QDataStream& stream );
    /// set a pixel
    void setPixel( Q_UINT32 size, QDataStream& stream );
    /// set raster operation mode
    void setRop( Q_UINT32 size, QDataStream& stream );
    /// save device context */
    void saveDC( Q_UINT32 size, QDataStream& stream );
    /// restore device context
    void restoreDC( Q_UINT32 size, QDataStream& stream );
    ///  clipping region is the intersection of this region and the original region
    void intersectClipRect( Q_UINT32 size, QDataStream& stream );
    /// delete a clipping rectangle of the original region
    void excludeClipRect( Q_UINT32 size, QDataStream& stream );

    /****************** Text *******************/
    /// set text color
    void setTextColor( Q_UINT32 size, QDataStream& stream );
    /// set text alignment
    void setTextAlign( Q_UINT32 size, QDataStream& stream );
    /// draw text
    void textOut( Q_UINT32 size, QDataStream& stream );
    void extTextOut( Q_UINT32 size, QDataStream& stream );

    /****************** Bitmap *******************/
    void SetStretchBltMode( Q_UINT32, QDataStream& stream );
    /// copies a DIB into a dest location
    void dibBitBlt( Q_UINT32 size, QDataStream& stream );
    /// stretches a DIB into a dest location
    void dibStretchBlt( Q_UINT32 size, QDataStream& stream );
    void stretchDib( Q_UINT32 size, QDataStream& stream );
    /// create a pattern brush
    void dibCreatePatternBrush( Q_UINT32 size, QDataStream& stream );

    /****************** Object handle *******************/
    /// activate object handle
    void selectObject( Q_UINT32 size, QDataStream& stream );
    /// free object handle
    void deleteObject( Q_UINT32 size, QDataStream& stream );
    /// create an empty object in the object list
    void createEmptyObject();
    /// create a logical brush
    void createBrushIndirect( Q_UINT32 size, QDataStream& stream );
    /// create a logical pen
    void createPenIndirect( Q_UINT32 size, QDataStream& stream );
    /// create a logical font
    void createFontIndirect( Q_UINT32 size, QDataStream& stream );

    /****************** misc *******************/
    /// end of meta file
    void end( Q_UINT32, QDataStream& stream );

    /** Calculate header checksum */
    static Q_UINT16 calcCheckSum( WmfPlaceableHeader* );

    // function unimplemented
    void notyet( Q_UINT32, QDataStream& stream );
    void region( Q_UINT32, QDataStream& stream );
    void palette( Q_UINT32, QDataStream& stream );
    void escape( Q_UINT32, QDataStream& stream );
    void setRelAbs( Q_UINT32, QDataStream& stream );
    void setMapMode( Q_UINT32, QDataStream& stream );
    void extFloodFill( Q_UINT32, QDataStream& stream );
    void startDoc( Q_UINT32, QDataStream& stream );
    void startPage( Q_UINT32, QDataStream& stream );
    void endDoc( Q_UINT32, QDataStream& stream );
    void endPage( Q_UINT32, QDataStream& stream );
    void resetDC( Q_UINT32, QDataStream& stream );
    void bitBlt( Q_UINT32, QDataStream& stream );
    void setDibToDev( Q_UINT32, QDataStream& stream );
    void createBrush( Q_UINT32, QDataStream& stream );
    void createPatternBrush( Q_UINT32, QDataStream& stream );
    void createBitmap( Q_UINT32, QDataStream& stream );
    void createBitmapIndirect( Q_UINT32, QDataStream& stream );
    void createPalette( Q_UINT32, QDataStream& stream );
    void createRegion( Q_UINT32, QDataStream& stream );

private:
    //-----------------------------------------------------------------------------
    // Utilities and conversion Wmf -> Qt

    /** Handle win-object-handles */
    bool addHandle( KoWmfHandle*  );
    void deleteHandle( int );

    /** Convert QINT16 points into QPointArray */
    void pointArray( QDataStream& stream, QPointArray& pa );

    /** Convertion between windows color and QColor */
    QColor qtColor( Q_UINT32 color ) const
    { return QColor( color & 0xFF, (color>>8) & 0xFF, (color>>16) & 0xFF ); }

    /** Convert (x1,y1) and (x2, y2) positions in angle and angleLength */
    void xyToAngle( int xStart, int yStart, int xEnd, int yEnd, int& angle, int& aLength );

    /** Convert windows rasterOp in QT rasterOp */
    Qt::RasterOp winToQtRaster( Q_UINT16 param ) const;
    Qt::RasterOp winToQtRaster( Q_UINT32 param ) const;

    /** Converts DIB to BMP */
    bool dibToBmp( QImage& bmp, QDataStream& stream, Q_UINT32 size);


public:
    // state of the WMF
    bool mValid;
    bool mStandard;
    bool mPlaceable;
    bool mEnhanced;

    /// bounding rectangle
    QRect  mBBox;     // placeable file : this is the header
                      // standard file : this is the value in setWindowOrg and setWindowExt
    /// number of points per inch for the default size
    int mDpi;

    /// number of functions to draw (==0 for all)
    int mNbrFunc;

private:
    // the output
    KoWmfRead *mReadWmf;

    // current coordinate != mBBox
    QRect  mWindow;
    // current state of the drawing
    QColor  mTextColor;
    Q_UINT16  mTextAlign;
    int     mTextRotation;
    bool    mWinding;

    // memory allocation for WMF file
    QBuffer*  mBuffer;
    int    mOffsetFirstRecord;

    // stack of object handle
    KoWmfHandle**  mObjHandleTab;
    // number of object on the stack
    int    mNbrObject;
    bool   mStackOverflow;
};

/**
 *  static data
 */
    static const struct KoWmfFunc {
        void ( KoWmfReadPrivate::*method )( Q_UINT32, QDataStream& );
    } koWmfFunc[] = {
      { &KoWmfReadPrivate::end }, // 0
      { &KoWmfReadPrivate::setBkColor }, // 1
      { &KoWmfReadPrivate::setBkMode }, // 2
      { &KoWmfReadPrivate::setMapMode }, // 3
      { &KoWmfReadPrivate::setRop }, // 4
      { &KoWmfReadPrivate::setRelAbs }, // 5
      { &KoWmfReadPrivate::setPolyFillMode }, // 6
      { &KoWmfReadPrivate::SetStretchBltMode }, // 7
      { &KoWmfReadPrivate::notyet }, // 8
      { &KoWmfReadPrivate::setTextColor }, // 9
      { &KoWmfReadPrivate::ScaleWindowExt }, // 10
      { &KoWmfReadPrivate::setWindowOrg }, // 11
      { &KoWmfReadPrivate::setWindowExt }, // 12
      { &KoWmfReadPrivate::notyet }, // 13
      { &KoWmfReadPrivate::notyet }, // 14
      { &KoWmfReadPrivate::OffsetWindowOrg }, // 15
      { &KoWmfReadPrivate::notyet }, // 16
      { &KoWmfReadPrivate::notyet }, // 17
      { &KoWmfReadPrivate::notyet }, // 18
      { &KoWmfReadPrivate::lineTo }, // 19
      { &KoWmfReadPrivate::moveTo }, // 20
      { &KoWmfReadPrivate::excludeClipRect }, // 21
      { &KoWmfReadPrivate::intersectClipRect }, // 22
      { &KoWmfReadPrivate::arc }, // 23
      { &KoWmfReadPrivate::ellipse }, // 24
      { &KoWmfReadPrivate::notyet }, // 25
      { &KoWmfReadPrivate::pie }, // 26
      { &KoWmfReadPrivate::rectangle }, // 27
      { &KoWmfReadPrivate::roundRect }, // 28
      { &KoWmfReadPrivate::notyet }, // 29
      { &KoWmfReadPrivate::saveDC }, // 30
      { &KoWmfReadPrivate::setPixel }, // 31
      { &KoWmfReadPrivate::notyet }, // 32
      { &KoWmfReadPrivate::textOut }, // 33
      { &KoWmfReadPrivate::bitBlt }, // 34
      { &KoWmfReadPrivate::notyet }, // 35
      { &KoWmfReadPrivate::polygon }, // 36
      { &KoWmfReadPrivate::polyline }, // 37
      { &KoWmfReadPrivate::escape }, // 38
      { &KoWmfReadPrivate::restoreDC }, // 39
      { &KoWmfReadPrivate::region }, // 40
      { &KoWmfReadPrivate::region }, // 41
      { &KoWmfReadPrivate::region }, // 42
      { &KoWmfReadPrivate::region }, // 43
      { &KoWmfReadPrivate::region }, // 44
      { &KoWmfReadPrivate::selectObject }, // 45
      { &KoWmfReadPrivate::setTextAlign }, // 46
      { 0 }, // 47
      { &KoWmfReadPrivate::chord }, // 48
      { &KoWmfReadPrivate::notyet }, // 49
      { &KoWmfReadPrivate::extTextOut }, // 50
      { &KoWmfReadPrivate::setDibToDev }, // 51
      { &KoWmfReadPrivate::palette }, // 52
      { &KoWmfReadPrivate::palette }, // 53
      { &KoWmfReadPrivate::palette }, // 54
      { &KoWmfReadPrivate::palette }, // 55
      { &KoWmfReadPrivate::polyPolygon }, // 56
      { &KoWmfReadPrivate::palette }, // 57
      { 0 }, // 58
      { 0 }, // 59
      { 0 }, // 60
      { 0 }, // 61
      { 0 }, // 62
      { 0 }, // 63
      { &KoWmfReadPrivate::dibBitBlt }, // 64
      { &KoWmfReadPrivate::dibStretchBlt }, // 65
      { &KoWmfReadPrivate::dibCreatePatternBrush }, // 66
      { &KoWmfReadPrivate::stretchDib }, // 67
      { 0 }, // 68
      { 0 }, // 69
      { 0 }, // 70
      { 0 }, // 71
      { &KoWmfReadPrivate::extFloodFill }, // 72
      { 0 }, // 73
      { 0 }, // 74
      { 0 }, // 75
      { &KoWmfReadPrivate::resetDC }, // 76
      { &KoWmfReadPrivate::startDoc }, // 77
      { 0 }, // 78
      { &KoWmfReadPrivate::startPage }, // 79
      { &KoWmfReadPrivate::endPage }, // 80
      { 0 }, // 81
      { 0 }, // 82
      { 0 }, // 83
      { 0 }, // 84
      { 0 }, // 85
      { 0 }, // 86
      { 0 }, // 87
      { 0 }, // 88
      { 0 }, // 89
      { 0 }, // 90
      { 0 }, // 91
      { 0 }, // 92
      { 0 }, // 93
      { &KoWmfReadPrivate::endDoc }, // 94
      { 0 }, // 95
      // 0x5F last function until 0xF0
      { &KoWmfReadPrivate::deleteObject }, // 96
      { 0 }, // 97
      { 0 }, // 98
      { 0 }, // 99
      { 0 }, // 100
      { 0 }, // 101
      { 0 }, // 102
      { &KoWmfReadPrivate::createPalette }, // 103
      { &KoWmfReadPrivate::createBrush }, // 104
      { &KoWmfReadPrivate::createPatternBrush }, // 105
      { &KoWmfReadPrivate::createPenIndirect }, // 106
      { &KoWmfReadPrivate::createFontIndirect }, // 107
      { &KoWmfReadPrivate::createBrushIndirect }, //108
      { &KoWmfReadPrivate::createBitmapIndirect }, //109
      { &KoWmfReadPrivate::createBitmap }, // 110
      { &KoWmfReadPrivate::createRegion } // 111
    };


#endif

