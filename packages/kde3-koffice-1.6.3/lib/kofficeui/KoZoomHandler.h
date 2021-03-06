/* This file is part of the KDE project
   Copyright (C) 2001-2005 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef kozoomhandler_h
#define kozoomhandler_h

#include <KoRect.h>
#include <koffice_export.h>
#include <KoZoomMode.h>

/**
 * This class handles the zooming and DPI stuff (conversions between pt values and pixels).
 * An instance of KoZoomHandler operates at a given zoom (see setZoomAndResolution() and setZoom())
 * so there is usually one instance of KoZoomHandler per view.
 */
class KOFFICEUI_EXPORT KoZoomHandler
{
public:
    KoZoomHandler();
    virtual ~KoZoomHandler() {}

    /**
     * Change the zoom factor to @p z (e.g. 150 for 150%)
     * and/or change the resolution, given in DPI.
     * This is done on startup, when zooming, and when printing.
     * The same call combines both so that all the updating done behind
     * the scenes is done only once, even if both zoom and DPI must be changed.
     */
    virtual void setZoomAndResolution( int zoom, int dpiX, int dpiY );

    /**
     * @return the conversion factor between pt and pixel, that
     * takes care of the zoom and the DPI setting.
     * Use zoomIt(pt) instead, though.
     */
    double zoomedResolutionX() const { return m_zoomedResolutionX; }
    double zoomedResolutionY() const { return m_zoomedResolutionY; }

    double resolutionX() const { return m_resolutionX; }
    double resolutionY() const { return m_resolutionY; }

    /**
     * Zoom factor for X. Equivalent to zoomedResolutionX()/resolutionX()
     */
    double zoomFactorX() const { return m_zoomedResolutionX / m_resolutionX; }
    /**
     * Zoom factor for Y. Equivalent to zoomedResolutionY()/resolutionY()
     */
    double zoomFactorY() const { return m_zoomedResolutionY / m_resolutionY; }


    /**
     * Set a resolution for X and Y, when no zoom applies (e.g. when painting an
     * embedded document. This will set the zoom to 100, and it will set
     * zoomedResolution[XY] to the resolution[XY] parameters
     * Helper method, equivalent to setZoomAndResolution(100,...).
     */
    void setResolution( double resolutionX, double resolutionY );

    /**
     * Set the zoomed resolution for X and Y.
     * Compared to the setZoom... methods, this allows to set a different
     * zoom factor for X and for Y.
     */
    virtual void setZoomedResolution( double zoomedResolutionX, double zoomedResolutionY );

    /**
     * Change the zoom level, keeping the resolution unchanged.
     * @param zoom the zoom factor (e.g. 100 for 100%)
     */
    void setZoom( int zoom );
    /**
     * Change the zoom mode
     * @param zoomMode the zoom mode.
     */
    void setZoomMode( KoZoomMode::Mode zoomMode ) { m_zoomMode = zoomMode; }
    /**
     * @return the global zoom factor (e.g. 100 for 100%).
     * Only use this to display to the user, don't use in calculations
     */
    int zoom() const { return m_zoom; }
    /**
     * @return the global zoom mode (e.g. KoZoomMode::ZOOM_WIDTH).
     * use this to determine how to zoom
     */
    KoZoomMode::Mode zoomMode() const { return m_zoomMode; }
    
    // Input: pt. Output: pixels. Resolution and zoom are applied.
    int zoomItX( double z ) const {
        return qRound( m_zoomedResolutionX * z );
    }
    int zoomItY( double z ) const {
        return qRound( m_zoomedResolutionY * z );
    }

    QPoint zoomPoint( const KoPoint & p ) const {
        return QPoint( zoomItX( p.x() ), zoomItY( p.y() ) );
    }
    QRect zoomRect( const KoRect & r ) const {
        QRect _r;
        _r.setCoords( zoomItX( r.left() ),  zoomItY( r.top() ),
                      zoomItX( r.right() ), zoomItY( r.bottom() ) );
        return _r;
    }
    /**
     * Returns the size in pixels for a input size in points.
     *
     * This function can return a size with 1 pixel to less, depending
     * on the reference point and the width and/or the zoom level.
     * It's save to use if the starting point is (0/0).
     * You can use it if you don't know the starting point yet
     * (like when inserting a picture), but then please take
     * care of it afterwards, when you know the reference point.
     */
    QSize zoomSize( const KoSize & s ) const {
        return QSize( zoomItX( s.width() ), zoomItY( s.height() ) );
    }

    // Input: pixels. Output: pt.
    double unzoomItX( int x ) const {
        return static_cast<double>( x ) / m_zoomedResolutionX;
    }
    double unzoomItY( int y ) const {
        return static_cast<double>( y ) / m_zoomedResolutionY;
    }
    KoPoint unzoomPoint( const QPoint & p ) const {
        return KoPoint( unzoomItX( p.x() ), unzoomItY( p.y() ) );
    }
    KoRect unzoomRect( const QRect & r ) const {
        KoRect _r;
        _r.setCoords( unzoomItX( r.left() ),  unzoomItY( r.top() ),
                      unzoomItX( r.right() ), unzoomItY( r.bottom() ) );
        return _r;
    }


protected:
    int m_zoom;
    KoZoomMode::Mode m_zoomMode;
    
    double m_resolutionX;
    double m_resolutionY;
    double m_zoomedResolutionX;
    double m_zoomedResolutionY;
};

#endif
