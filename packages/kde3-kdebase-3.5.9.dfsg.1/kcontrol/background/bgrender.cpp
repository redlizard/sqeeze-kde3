/* vi: ts=8 sts=4 sw=4
 * kate: space-indent on; tab-width 8; indent-width 4; indent-mode cstyle;
 *
 * This file is part of the KDE project, module kdesktop.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 *
 * You can Freely distribute this program under the GNU Library General
 * Public License. See the file "COPYING.LIB" for the exact licensing terms.
 */

#include <config.h>

#include <time.h>
#include <stdlib.h>
#include <utime.h>

#include <qtimer.h>
#include <qpainter.h>
#include <qimage.h>
#include <qfileinfo.h>
#include <qdir.h>

#include <dcopclient.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kimageeffect.h>
#include <kprocess.h>
#include <kpixmapio.h>
#include <ktempfile.h>
#include <kcursor.h>
#include <kmimetype.h>
#include <kfilemetainfo.h>

#ifdef HAVE_LIBART
#include <ksvgiconengine.h>
#endif

#include "bgdefaults.h"
#include "bghash.h"
#include "bgrender.h"

#include <X11/Xlib.h>

#include <config.h>

/**** KBackgroundRenderer ****/


KBackgroundRenderer::KBackgroundRenderer(int desk, int screen, bool drawBackgroundPerScreen, KConfig *config)
    : KBackgroundSettings(desk, screen, drawBackgroundPerScreen, config)
{
    m_State = 0;
    m_isBusyCursor = false;
    m_enableBusyCursor = false;
    m_pDirs = KGlobal::dirs();
    m_rSize = m_Size = drawBackgroundPerScreen ?
            QApplication::desktop()->screenGeometry(screen).size() : QApplication::desktop()->size();
    m_pProc = 0L;
    m_Tempfile = 0L;
    m_bPreview = false;
    m_Cached = false;
    m_TilingEnabled = false;

    m_pTimer = new QTimer(this);
    connect(m_pTimer, SIGNAL(timeout()), SLOT(render()));
}


KBackgroundRenderer::~KBackgroundRenderer()
{
    cleanup();
    delete m_Tempfile;
    m_Tempfile = 0;
}


void KBackgroundRenderer::setSize(const QSize &size)
{
    m_rSize = m_Size = size;
}

/*
 * Re-configure because the desktop has been resized.
 */
void KBackgroundRenderer::desktopResized()
{
    m_State = 0;
    m_rSize = drawBackgroundPerScreen() ?
            QApplication::desktop()->screenGeometry(screen()).size() : QApplication::desktop()->size();
    if( !m_bPreview )
        m_Size = m_rSize;
}


void KBackgroundRenderer::tile(QImage& dest, QRect rect, const QImage& src)
{
    rect &= dest.rect();

    int x, y;
    int h = rect.height(), w = rect.width();
    int offx = rect.x(), offy = rect.y();
    int sw = src.width(), sh = src.height();

    for (y=offy; y<offy+h; y++)
	for (x=offx; x<offx+w; x++)
	    dest.setPixel(x, y, src.pixel(x%sw, y%sh));
}


/*
 * Build a command line to run the program.
 */

QString KBackgroundRenderer::buildCommand()
{
    QString num;
    int pos = 0;

    QString cmd;
    if (m_bPreview)
        cmd = previewCommand();
    else
        cmd = command();

    if (cmd.isEmpty())
	return QString();

    while ((pos = cmd.find('%', pos)) != -1) {

        if (pos == (int) (cmd.length() - 1))
            break;

        switch (cmd.at(pos+1).latin1()) {
        case 'f':
            createTempFile();
            cmd.replace(pos, 2, KShellProcess::quote(m_Tempfile->name()));
            pos += m_Tempfile->name().length() - 2;
            break;

        case 'x':
            num.setNum(m_Size.width());
            cmd.replace(pos, 2, num);
            pos += num.length() - 2;
            break;

        case 'y':
            num.setNum(m_Size.height());
            cmd.replace(pos, 2, num);
            pos += num.length() - 2;
            break;

        case '%':
            cmd.replace(pos, 2, "%");
            pos--;
            break;
        default:
            ++pos; // avoid infinite loop
            break;
        }

    }
    return cmd;
}


/*
 * Create a background tile. If the background mode is `Program',
 * this is asynchronous.
 */
int KBackgroundRenderer::doBackground(bool quit)
{
    if (m_State & BackgroundDone)
        return Done;
    int bgmode = backgroundMode();

    if (!enabled())
      bgmode= Flat;

    if (quit) {
	if (bgmode == Program && m_pProc)
	    m_pProc->kill();
        return Done;
    }

    int retval = Done;
    QString file;

    static unsigned int tileWidth = 0;
    static unsigned int tileHeight = 0;
    if( tileWidth == 0 )
        {
        int tile_val = QPixmap::defaultDepth() >= 24 ? 1 : 2;
    // some dithering may be needed even with bpb==15/16, so don't use tileWidth==1
    // for them
    // with tileWidth>2, repainting the desktop causes nasty effect (XFree86 4.1.0 )
        if( XQueryBestTile( qt_xdisplay(), qt_xrootwin(), tile_val, tile_val,
            &tileWidth, &tileHeight ) != Success )
            tileWidth = tileHeight = tile_val; // some defaults
    }
    switch (bgmode) {

    case Flat:
        // this can be tiled correctly without problems
	m_Background.create( tileWidth, tileHeight, 32);
        m_Background.fill(colorA().rgb());
        break;

    case Pattern:
    {
        if (pattern().isEmpty())
            break;
        file = m_pDirs->findResource("dtop_pattern", pattern());
        if (file.isEmpty())
            break;

	m_Background.load(file);
	if (m_Background.isNull())
	    break;
	int w = m_Background.width();
	int h = m_Background.height();
	if ((w > m_Size.width()) || (h > m_Size.height())) {
	    w = QMIN(w, m_Size.width());
	    h = QMIN(h, m_Size.height());
	    m_Background = m_Background.copy(0, 0, w, h);
	}
	KImageEffect::flatten(m_Background, colorA(), colorB(), 0);
	break;
    }
    case Program:
        if (m_State & BackgroundStarted)
            break;
        m_State |= BackgroundStarted;
        createTempFile();

	file = buildCommand();
	if (file.isEmpty())
	    break;

        delete m_pProc;
        m_pProc = new KShellProcess;
        *m_pProc << file;
        connect(m_pProc, SIGNAL(processExited(KProcess *)),
                SLOT(slotBackgroundDone(KProcess *)));
        m_pProc->start(KShellProcess::NotifyOnExit);
        retval = Wait;
        break;

    case HorizontalGradient:
    {
	QSize size = m_Size;
        // on <16bpp displays the gradient sucks when tiled because of dithering
        if( canTile())
	    size.setHeight( tileHeight );
	m_Background = KImageEffect::gradient(size, colorA(), colorB(),
		KImageEffect::HorizontalGradient, 0);
        break;
    }
    case VerticalGradient:
    {
	QSize size = m_Size;
        // on <16bpp displays the gradient sucks when tiled because of dithering
        if( canTile())
	    size.setWidth( tileWidth );
        m_Background = KImageEffect::gradient(size, colorA(), colorB(),
		KImageEffect::VerticalGradient, 0);
        break;
    }
    case PyramidGradient:
        m_Background = KImageEffect::gradient(m_Size, colorA(), colorB(),
		KImageEffect::PyramidGradient, 0);
        break;

    case PipeCrossGradient:
        m_Background = KImageEffect::gradient(m_Size, colorA(), colorB(),
		KImageEffect::PipeCrossGradient, 0);
        break;

    case EllipticGradient:
        m_Background = KImageEffect::gradient(m_Size, colorA(), colorB(),
		KImageEffect::EllipticGradient, 0);
        break;
    }

    if (retval == Done)
        m_State |= BackgroundDone;

    return retval;
}


int KBackgroundRenderer::doWallpaper(bool quit)
{
    if (m_State & WallpaperDone)
        return Done;

    if (quit)
        // currently no asynch. wallpapers
        return Done;

    int wpmode= enabled()?wallpaperMode():NoWallpaper;

    m_Wallpaper = QImage();
    if (wpmode != NoWallpaper) {
wp_load:
	if (currentWallpaper().isEmpty()) {
	    wpmode = NoWallpaper;
	    goto wp_out;
	}
	QString file = m_pDirs->findResource("wallpaper", currentWallpaper());
	if (file.isEmpty()) {
	    wpmode = NoWallpaper;
	    goto wp_out;
	}

        // _Don't_ use KMimeType, as it relies on ksycoca which we really
        // don't want in krootimage (kdm context).
        //if ( KMimeType::findByPath( file )->is( "image/svg+xml" ) ) {
        if (file.endsWith(".svg") || file.endsWith(".svgz")) {
#ifdef HAVE_LIBART
	    // Special stuff for SVG icons
	    KSVGIconEngine* svgEngine = new KSVGIconEngine();

	    //FIXME
	    //ksvgiconloader doesn't seem to let us find out the
	    //ratio of width to height so for the most part we just
	    //assume it's a square
	    int svgWidth;
	    int svgHeight;
	    switch (wpmode)
	    {
	        case Centred:
	        case CentredAutoFit:
		    svgHeight = (int)(m_Size.height() * 0.8);
		    svgWidth = svgHeight;
	            break;
	        case Tiled:
	        case CenterTiled:
		    svgHeight = (int)(m_Size.height() * 0.5);
		    svgWidth = svgHeight;
	            break;
	        case Scaled:
		    svgHeight = m_Size.height();
		    svgWidth = m_Size.width();
	            break;
	        case CentredMaxpect:
		case ScaleAndCrop:
	        case TiledMaxpect:
		    svgHeight = m_Size.height();
		    svgWidth = svgHeight;
	            break;
	        case NoWallpaper:
	        default:
	            kdWarning() << k_funcinfo << "unknown diagram type" << endl;
		    svgHeight = m_Size.height();
		    svgWidth = svgHeight;
		    break;
	    }
	    //FIXME hack due to strangeness with
	    //background control modules
	    if ( svgHeight < 200 ) {
		svgHeight *= 6;
	        svgWidth *= 6;
	    }

	    if (svgEngine->load(svgWidth, svgHeight, file )) {
		QImage *image = svgEngine->image();
		m_Wallpaper = *image;
		delete image;
	    } else {
		kdWarning() << "failed to load SVG file " << file << endl;
	    }

	    delete svgEngine;
#else //not libart
	    kdWarning() << k_funcinfo
			<< "tried to load SVG file but libart not installed" << endl;
#endif
	} else {
	    m_Wallpaper.load(file);
	}
	if (m_Wallpaper.isNull()) {
            if (discardCurrentWallpaper())
               goto wp_load;
	    wpmode = NoWallpaper;
	    goto wp_out;
	}
	m_Wallpaper = m_Wallpaper.convertDepth(32, DiffuseAlphaDither);

	// If we're previewing, scale the wallpaper down to make the preview
	// look more like the real desktop.
	if (m_bPreview) {
	    int xs = m_Wallpaper.width() * m_Size.width() / m_rSize.width();
	    int ys = m_Wallpaper.height() * m_Size.height() / m_rSize.height();
	    if ((xs < 1) || (ys < 1))
	    {
	       xs = ys = 1;
	    }
	    if( m_Wallpaper.size() != QSize( xs, ys ))
		m_Wallpaper = m_Wallpaper.smoothScale(xs, ys);
	}

	// HACK: Use KFileMetaInfo only when we're attached to DCOP.
	// KFileMetaInfo needs ksycoca and so on, but this code is
	// used also in krootimage (which in turn is used by kdm).
	if( kapp->dcopClient()->isAttached()) {
	    KFileMetaInfo metaInfo(file);
	    if (metaInfo.isValid() && metaInfo.item("Orientation").isValid()) {
		switch (metaInfo.item("Orientation").string().toInt()) {
		    case 2:
			// Flipped horizontally
			m_Wallpaper = m_Wallpaper.mirror(true, false);
			break;
		    case 3:
			// Rotated 180 degrees
			m_Wallpaper = KImageEffect::rotate(m_Wallpaper, KImageEffect::Rotate180);
			break;
		    case 4:
			// Flipped vertically
			m_Wallpaper = m_Wallpaper.mirror(false, true);
			break;
		    case 5:
			// Rotated 90 degrees & flipped horizontally
			m_Wallpaper = KImageEffect::rotate(m_Wallpaper, KImageEffect::Rotate90).mirror(true, false);
			break;
		    case 6:
			// Rotated 90 degrees
			m_Wallpaper = KImageEffect::rotate(m_Wallpaper, KImageEffect::Rotate90);
			break;
		    case 7:
			// Rotated 90 degrees & flipped vertically
			m_Wallpaper = KImageEffect::rotate(m_Wallpaper, KImageEffect::Rotate90).mirror(false, true);
			break;
		    case 8:
			// Rotated 270 degrees
			m_Wallpaper = KImageEffect::rotate(m_Wallpaper, KImageEffect::Rotate270);
			break;
		    case 1:
		    default:
			// Normal or invalid orientation
			break;
		}
	    }
	}
    }
wp_out:

    if (m_Background.isNull()) {
	m_Background.create(8, 8, 32);
	m_Background.fill(colorA().rgb());
    }

    int retval = Done;

    int w = m_Size.width();	// desktop width/height
    int h = m_Size.height();

    int ww = m_Wallpaper.width();	// wallpaper width/height
    int wh = m_Wallpaper.height();

    m_WallpaperRect = QRect();	// to be filled destination rectangle; may exceed desktop!

    switch (wpmode)
    {
	case NoWallpaper:
	    break;
	case Centred:
	    m_WallpaperRect.setRect((w - ww) / 2, (h - wh) / 2, ww, wh);
	    break;
	case Tiled:
	    m_WallpaperRect.setRect(0, 0, w, h);
	    break;
	case CenterTiled:
	    m_WallpaperRect.setCoords(-ww + ((w - ww) / 2) % ww, -wh + ((h - wh) / 2) % wh, w-1, h-1);
	    break;
	case Scaled:
	    ww = w;
	    wh = h;
	    if( m_WallpaperRect.size() != QSize( w, h ))
		m_Wallpaper = m_Wallpaper.smoothScale( w, h );
	    m_WallpaperRect.setRect(0, 0, w, h);
	    break;
        case CentredAutoFit:
            if( ww <= w && wh <= h ) {
    	        m_WallpaperRect.setRect((w - ww) / 2, (h - wh) / 2, ww, wh); // like Centred
	        break;
            }
            // fall through
	case CentredMaxpect:
            {
              double sx = (double) w / ww;
              double sy = (double) h / wh;
              if (sx > sy) {
                  ww = (int)(sy * ww);
                  wh = h;
              } else {
                  wh = (int)(sx * wh);
                  ww = w;
              }
	      if( m_WallpaperRect.size() != QSize( ww, wh ))
                  m_Wallpaper = m_Wallpaper.smoothScale(ww, wh);
	      m_WallpaperRect.setRect((w - ww) / 2, (h - wh) / 2, ww, wh);
	      break;
            }
	case TiledMaxpect:
            {
              double sx = (double) w / ww;
              double sy = (double) h / wh;
              if (sx > sy) {
                  ww = (int)(sy * ww);
                  wh = h;
              } else {
                  wh = (int)(sx * wh);
                  ww = w;
              }
              if( m_WallpaperRect.size() != QSize( ww, wh ))
                  m_Wallpaper = m_Wallpaper.smoothScale(ww, wh);
	      m_WallpaperRect.setRect(0, 0, w, h);
	      break;
            }
	 case ScaleAndCrop:
            {
              double sx = (double) w / ww;
              double sy = (double) h / wh;
              if (sx > sy) {
	      	  //Case 1: x needs bigger scaling. Lets increase x and leave part of y offscreen
                  ww = w;
		  wh=(int)(sx * wh);
              } else {
	          //Case 2: y needs bigger scaling. Lets increase y and leave part of x offscreen
                  wh = h;
                  ww = (int)(sy*ww);
              }
              if( m_WallpaperRect.size() != QSize( ww, wh ))
                  m_Wallpaper = m_Wallpaper.smoothScale(ww, wh);
	      m_WallpaperRect.setRect((w - ww) / 2, (h - wh) / 2,w, h);
	      break;
            }
    }

    wallpaperBlend();

    if (retval == Done)
        m_State |= WallpaperDone;

    return retval;
}

bool KBackgroundRenderer::canTile() const
{
    return m_TilingEnabled && optimize();
}

extern bool qt_use_xrender; // in Qt ( qapplication_x11.cpp )

void KBackgroundRenderer::wallpaperBlend()
{
    if( !enabled() || wallpaperMode() == NoWallpaper
        || (blendMode() == NoBlending && ( qt_use_xrender || !m_Wallpaper.hasAlphaBuffer()))) {
        fastWallpaperBlend();
    }
    else {
        fullWallpaperBlend();
    }
}

// works only for NoBlending and no alpha in wallpaper
// but is much faster than QImage fidling
void KBackgroundRenderer::fastWallpaperBlend()
{
    m_Image = QImage();
    // copy background to m_pPixmap
    if( !enabled() || (wallpaperMode() == NoWallpaper && canTile())) {
        // if there's no wallpaper, no need to tile the pixmap to the size of desktop, as X does
        // that automatically and using a smaller pixmap should save some memory
        m_Pixmap.convertFromImage( m_Background );
        return;
    }
    else if( wallpaperMode() == Tiled && !m_Wallpaper.hasAlphaBuffer() && canTile() && !m_bPreview ) {
    // tiles will be tiled by X automatically
        if( useShm()) {
            KPixmapIO io;
            m_Pixmap = io.convertToPixmap( m_Wallpaper );
        }
        else
            m_Pixmap.convertFromImage( m_Wallpaper );
        return;
    }
    else if( m_WallpaperRect.contains( QRect( QPoint( 0, 0 ), m_Size ))
        && !m_Wallpaper.hasAlphaBuffer()) // wallpaper covers all and no blending
        m_Pixmap = QPixmap( m_Size );
    else if (m_Background.size() == m_Size)
        m_Pixmap.convertFromImage( m_Background );
    else {
        m_Pixmap = QPixmap( m_Size );
        QPainter p( &m_Pixmap );
        QPixmap pm;
        pm.convertFromImage( m_Background );
        p.drawTiledPixmap( 0, 0, m_Size.width(), m_Size.height(), pm );
    }

    // paint/alpha-blend wallpaper to destination rectangle of m_pPixmap
    if (m_WallpaperRect.isValid()) {
        QPixmap wp_pixmap;
        if( useShm() && !m_Wallpaper.hasAlphaBuffer()) {
            KPixmapIO io;
            wp_pixmap = io.convertToPixmap( m_Wallpaper );
        }
        else
            wp_pixmap.convertFromImage( m_Wallpaper );
        int ww = m_Wallpaper.width();
        int wh = m_Wallpaper.height();
        for (int y = m_WallpaperRect.top(); y < m_WallpaperRect.bottom(); y += wh) {
	    for (int x = m_WallpaperRect.left(); x < m_WallpaperRect.right(); x += ww) {
		bitBlt( &m_Pixmap, x, y, &wp_pixmap, 0, 0, ww, wh );
	    }
	}
    }
}


void KBackgroundRenderer::fullWallpaperBlend()
{
    m_Pixmap = QPixmap();
    int w = m_Size.width();	// desktop width/height
    int h = m_Size.height();
    // copy background to m_pImage
    if (m_Background.size() == m_Size) {
	m_Image = m_Background.copy();

	if (m_Image.depth() < 32)
	    m_Image = m_Image.convertDepth(32, DiffuseAlphaDither);

    } else {
	m_Image.create(w, h, 32);
	tile(m_Image, QRect(0, 0, w, h), m_Background);
    }

    // blend wallpaper to destination rectangle of m_pImage
    if (m_WallpaperRect.isValid())
    {
        int blendFactor = 100;
        if (blendMode() == FlatBlending)
            blendFactor = (blendBalance()+200)/4;
        int ww = m_Wallpaper.width();
        int wh = m_Wallpaper.height();
        for (int y = m_WallpaperRect.top(); y < m_WallpaperRect.bottom(); y += wh) {
	    for (int x = m_WallpaperRect.left(); x < m_WallpaperRect.right(); x += ww) {
		blend(m_Image, QRect(x, y, ww, wh), m_Wallpaper,
			QPoint(-QMIN(x, 0), -QMIN(y, 0)), blendFactor);
	    }
	}
    }


    // blend whole desktop
    if ( wallpaperMode() != NoWallpaper) {
      int bal = blendBalance();

      switch( blendMode() ) {
      case HorizontalBlending:
	KImageEffect::blend( m_Image, m_Background,
			     KImageEffect::HorizontalGradient,
			     bal, 100 );
	break;

      case VerticalBlending:
	KImageEffect::blend( m_Image, m_Background,
			     KImageEffect::VerticalGradient,
			     100, bal );
	break;

      case PyramidBlending:
	KImageEffect::blend( m_Image, m_Background,
			     KImageEffect::PyramidGradient,
			     bal, bal );
	break;

      case PipeCrossBlending:
	KImageEffect::blend( m_Image, m_Background,
			     KImageEffect::PipeCrossGradient,
			     bal, bal );
	break;

      case EllipticBlending:
	KImageEffect::blend( m_Image, m_Background,
			     KImageEffect::EllipticGradient,
			     bal, bal );
	break;

      case IntensityBlending:
	KImageEffect::modulate( m_Image, m_Background, reverseBlending(),
		    KImageEffect::Intensity, bal, KImageEffect::All );
	break;

      case SaturateBlending:
	KImageEffect::modulate( m_Image, m_Background, reverseBlending(),
		    KImageEffect::Saturation, bal, KImageEffect::Gray );
	break;

      case ContrastBlending:
	KImageEffect::modulate( m_Image, m_Background, reverseBlending(),
		    KImageEffect::Contrast, bal, KImageEffect::All );
	break;

      case HueShiftBlending:
	KImageEffect::modulate( m_Image, m_Background, reverseBlending(),
		    KImageEffect::HueShift, bal, KImageEffect::Gray );
	break;

      case FlatBlending:
        // Already handled
	break;
      }
    }
}

/* Alpha blend an area from <src> with offset <soffs> to rectangle <dr> of <dst>
 * Default offset is QPoint(0, 0).
 * blendfactor = [0, 100%]
 */
void KBackgroundRenderer::blend(QImage& dst, QRect dr, const QImage& src, QPoint soffs, int blendFactor)
{
    int x, y, a;
    dr &= dst.rect();

    for (y = 0; y < dr.height(); y++) {
	if (dst.scanLine(dr.y() + y) && src.scanLine(soffs.y() + y)) {
	    QRgb *b, *d;
	    for (x = 0; x < dr.width(); x++) {
		b = reinterpret_cast<QRgb*>(dst.scanLine(dr.y() + y)
			+ (dr.x() + x) * sizeof(QRgb));
                d = reinterpret_cast<QRgb*>(src.scanLine(soffs.y() + y)
			+ (soffs.x() + x) * sizeof(QRgb));
                a = (qAlpha(*d) * blendFactor) / 100;
                *b = qRgb(qRed(*b) - (((qRed(*b) - qRed(*d)) * a) >> 8),
                          qGreen(*b) - (((qGreen(*b) - qGreen(*d)) * a) >> 8),
                          qBlue(*b) - (((qBlue(*b) - qBlue(*d)) * a) >> 8));
            }
        }
    }
}



void KBackgroundRenderer::slotBackgroundDone(KProcess *process)
{
    Q_ASSERT(process == m_pProc);
    m_State |= BackgroundDone;

    if (m_pProc->normalExit() && !m_pProc->exitStatus()) {
        m_Background.load(m_Tempfile->name());
        m_State |= BackgroundDone;
    }

    m_Tempfile->unlink();
    delete m_Tempfile; m_Tempfile = 0;
    m_pTimer->start(0, true);
    setBusyCursor(false);
}



/*
 * Starts the rendering process.
 */
void KBackgroundRenderer::start(bool enableBusyCursor)
{
    m_enableBusyCursor = enableBusyCursor;
    setBusyCursor(true);

    m_Cached = false;

    m_State = Rendering;
    m_pTimer->start(0, true);
}


/*
 * This slot is connected to a timer event. It is called repeatedly until
 * the rendering is done.
 */
void KBackgroundRenderer::render()
{
    setBusyCursor(true);
    if (!(m_State & Rendering))
        return;

    if( !(m_State & InitCheck)) {
        QString f = cacheFileName();
        if( useCacheFile()) {
            QString w = m_pDirs->findResource("wallpaper", currentWallpaper());
            QFileInfo wi( w );
            QFileInfo fi( f );
            if( wi.lastModified().isValid() && fi.lastModified().isValid()
                && wi.lastModified() < fi.lastModified()) {
                QImage im;
                if( im.load( f, "PNG" )) {
                    m_Image = im;
                    m_Pixmap = QPixmap( m_Size );
                    m_Pixmap.convertFromImage( m_Image );
                    m_Cached = true;
                    m_State |= InitCheck | BackgroundDone | WallpaperDone;
                }
            }
        }
        m_pTimer->start(0, true);
        m_State |= InitCheck;
        return;
    }

    int ret;

    if (!(m_State & BackgroundDone)) {
        ret = doBackground();
        if (ret != Wait)
	    m_pTimer->start(0, true);
	return;
    }

    // No async wallpaper
    doWallpaper();

    done();
    setBusyCursor(false);
}


/*
 * Rendering is finished.
 */
void KBackgroundRenderer::done()
{
    setBusyCursor(false);
    m_State |= AllDone;
    emit imageDone(desk(), screen());
    if(backgroundMode() == Program && m_pProc &&
       m_pProc->normalExit() && m_pProc->exitStatus()) {
         emit programFailure(desk(), m_pProc->exitStatus());
     } else if(backgroundMode() == Program && m_pProc &&
       !m_pProc->normalExit()) {
         emit programFailure(desk(), -1);
     } else if(backgroundMode() == Program) {
         emit programSuccess(desk());
     }

}

/*
 * This function toggles a busy cursor on and off, for use in rendering.
 * It is useful because of the ASYNC nature of the rendering - it is hard
 * to make sure we don't set the busy cursor twice, but only restore
 * once.
 */
void KBackgroundRenderer::setBusyCursor(bool isBusy) {
   if(m_isBusyCursor == isBusy)
      return;
   if (isBusy && !m_enableBusyCursor)
      return;
   m_isBusyCursor = isBusy;
   if(isBusy)
      QApplication::setOverrideCursor( KCursor::workingCursor() );
   else
      QApplication::restoreOverrideCursor();
}

/*
 * Stop the rendering.
 */
void KBackgroundRenderer::stop()
{
    if (!(m_State & Rendering))
	return;

    doBackground(true);
    doWallpaper(true);
    m_State = 0;
}


/*
 * Cleanup after rendering.
 */
void KBackgroundRenderer::cleanup()
{
    setBusyCursor(false);
    m_Background = QImage();
    m_Image = QImage();
    m_Pixmap = QPixmap();
    m_Wallpaper = QImage();
    delete m_pProc; m_pProc = 0L;
    m_State = 0;
    m_WallpaperRect = QRect();
    m_Cached = false;
}


void KBackgroundRenderer::setPreview(const QSize &size)
{
    if (size.isNull())
        m_bPreview = false;
    else {
        m_bPreview = true;
        m_Size = size;
    }
}


QPixmap KBackgroundRenderer::pixmap()
{
    if (m_State & AllDone) {
        if( m_Pixmap.isNull())
            m_Pixmap.convertFromImage( m_Image );
        return m_Pixmap;
    }
    return QPixmap();
}

QImage KBackgroundRenderer::image()
{
    if (m_State & AllDone) {
        if( m_Image.isNull())
            fullWallpaperBlend(); // create from m_Pixmap
        return m_Image;
    }
    return QImage();
}


void KBackgroundRenderer::load(int desk, int screen, bool drawBackgroundPerScreen, bool reparseConfig)
{
    if (m_State & Rendering)
        stop();

    cleanup();
    m_bPreview = false;
    m_Size = m_rSize;

    KBackgroundSettings::load(desk, screen, drawBackgroundPerScreen, reparseConfig);
}

void KBackgroundRenderer::createTempFile()
{
   if( !m_Tempfile )
     m_Tempfile = new KTempFile();
}

QString KBackgroundRenderer::cacheFileName()
{
    QString f = fingerprint();
    f.replace ( ':', '_' ); // avoid characters that shouldn't be in filenames
    f.replace ( '/', '#' );
    f = locateLocal( "cache", QString( "background/%1x%2_%3.png" )
        .arg( m_Size.width()).arg( m_Size.height()).arg( f ));
    return f;
}

bool KBackgroundRenderer::useCacheFile() const
{
    if( !enabled())
        return false;
    if( backgroundMode() == Program )
        return false; // don't cache these at all
    if( wallpaperMode() == NoWallpaper )
        return false; // generating only background patterns should be always faster
    QString file = currentWallpaper();
    if( file.endsWith(".svg") || file.endsWith(".svgz"))
        return true; // cache these, they can be bloody slow
    switch( backgroundMode())
        {
        case NoWallpaper:
        case Centred:
        case Tiled:
        case CenterTiled:
            return false; // these don't need scaling
        case CentredMaxpect:
        case TiledMaxpect:
        case Scaled:
        case CentredAutoFit:
        case ScaleAndCrop:
        default:
            return true;
        }
}

void KBackgroundRenderer::saveCacheFile()
{
    if( !( m_State & AllDone ))
        return;
    if( !useCacheFile())
        return;
    if( m_Image.isNull())
        fullWallpaperBlend(); // generate from m_Pixmap
    QString f = cacheFileName();
    if( KStandardDirs::exists( f ) || m_Cached )
        utime( QFile::encodeName( f ), NULL );
    else {
        m_Image.save( f, "PNG" );
        // remove old entries from the cache
        QDir dir( locateLocal( "cache", "background/" ));
        if( const QFileInfoList* list = dir.entryInfoList( "*.png", QDir::Files, QDir::Time | QDir::Reversed )) {
            int size = 0;
            for( QFileInfoListIterator it( *list );
                 QFileInfo* info = it.current();
                 ++it )
                size += info->size();
            for( QFileInfoListIterator it( *list );
                 QFileInfo* info = it.current();
                 ++it ) {
                if( size < 8 * 1024 * 1024 )
                    break;
                // keep everything newer than 10 minutes if the total size is less than 50M (just in case)
                if( size < 50 * 1024 * 1024
                    && ( time_t ) info->lastModified().toTime_t() >= time( NULL ) - 10 * 60 )
                    break;
                size -= info->size();
                QFile::remove( info->absFilePath());
            }
        }
    }
}

//BEGIN class KVirtualBGRenderer
KVirtualBGRenderer::KVirtualBGRenderer( int desk, KConfig *config )
{
    m_pPixmap = 0l;
    m_desk = desk;
    m_numRenderers = 0;
    m_scaleX = 1;
    m_scaleY = 1;
    
    // The following code is borrowed from KBackgroundSettings::KBackgroundSettings
    if (!config) {
        int screen_number = 0;
        if (qt_xdisplay())
            screen_number = DefaultScreen(qt_xdisplay());
        QCString configname;
        if (screen_number == 0)
            configname = "kdesktoprc";
        else
            configname.sprintf("kdesktop-screen-%drc", screen_number);

        m_pConfig = new KConfig(configname, false, false);
        m_bDeleteConfig = true;
    } else {
        m_pConfig = config;
        m_bDeleteConfig = false;
    }
    
    initRenderers();
    m_size = QApplication::desktop()->size();
}

KVirtualBGRenderer::~KVirtualBGRenderer()
{
    for (unsigned i=0; i<m_numRenderers; ++i)
        delete m_renderer[i];
   
    delete m_pPixmap;
    
    if (m_bDeleteConfig)
        delete m_pConfig;
}

    
KBackgroundRenderer * KVirtualBGRenderer::renderer(unsigned screen)
{
    return m_renderer[screen];
}


QPixmap KVirtualBGRenderer::pixmap()
{
    if (m_numRenderers == 1)
        return m_renderer[0]->pixmap();
    
    return *m_pPixmap;
}


bool KVirtualBGRenderer::needProgramUpdate()
{
    for (unsigned i=0; i<m_numRenderers; ++i)
    {
        if ( m_renderer[i]->backgroundMode() == KBackgroundSettings::Program &&
             m_renderer[i]->KBackgroundProgram::needUpdate() )
            return true;
    }
    return false;
}


void KVirtualBGRenderer::programUpdate()
{
    for (unsigned i=0; i<m_numRenderers; ++i)
    {
        if ( m_renderer[i]->backgroundMode() == KBackgroundSettings::Program &&
             m_renderer[i]->KBackgroundProgram::needUpdate() )
        {
            m_renderer[i]->KBackgroundProgram::update();
        }
    }
}


bool KVirtualBGRenderer::needWallpaperChange()
{
    for (unsigned i=0; i<m_numRenderers; ++i)
    {
        if ( m_renderer[i]->needWallpaperChange() )
            return true;
    }
    return false;
}


void KVirtualBGRenderer::changeWallpaper()
{
    for (unsigned i=0; i<m_numRenderers; ++i)
    {
        m_renderer[i]->changeWallpaper();
    }
}


int KVirtualBGRenderer::hash()
{
    QString fp;
    for (unsigned i=0; i<m_numRenderers; ++i)
    {
        fp += m_renderer[i]->fingerprint();
    }
    //kdDebug() << k_funcinfo << " fp=\""<<fp<<"\" h="<<QHash(fp)<<endl;
    return QHash(fp);
}


bool KVirtualBGRenderer::isActive()
{
    for (unsigned i=0; i<m_numRenderers; ++i)
    {
        if ( m_renderer[i]->isActive() )
            return true;
    }
    return false;
}


void KVirtualBGRenderer::setEnabled(bool enable)
{
    for (unsigned i=0; i<m_numRenderers; ++i)
        m_renderer[i]->setEnabled(enable);
}


void KVirtualBGRenderer::desktopResized()
{
    m_size = QApplication::desktop()->size();
    
    if (m_pPixmap)
    {
        delete m_pPixmap;
        m_pPixmap = new QPixmap(m_size);
        m_pPixmap->fill(Qt::black);
    }
    
    for (unsigned i=0; i<m_numRenderers; ++i)
        m_renderer[i]->desktopResized();
}


void KVirtualBGRenderer::setPreview(const QSize & size)
{
    if (m_size == size)
        return;
    
    m_size = size;
    
    if (m_pPixmap)
        m_pPixmap->resize(m_size);
    
    // Scaling factors
    m_scaleX = float(m_size.width()) / float(QApplication::desktop()->size().width());
    m_scaleY = float(m_size.height()) / float(QApplication::desktop()->size().height());
    
    // Scale renderers appropriately
    for (unsigned i=0; i<m_renderer.size(); ++i)
    {
        QSize unscaledRendererSize = renderSize(i);
        
        m_renderer[i]->setPreview( QSize(
                int(unscaledRendererSize.width() * m_scaleX),
                int(unscaledRendererSize.height() * m_scaleY) ) );
    }
}


QSize KVirtualBGRenderer::renderSize(int screen)
{
    return m_bDrawBackgroundPerScreen ?
            QApplication::desktop()->screenGeometry(screen).size() : QApplication::desktop()->size();
}


void KVirtualBGRenderer::initRenderers()
{
    m_pConfig->setGroup("Background Common");
    m_bDrawBackgroundPerScreen = m_pConfig->readBoolEntry( QString("DrawBackgroundPerScreen_%1").arg(m_desk), _defDrawBackgroundPerScreen );
    
    m_bCommonScreen = m_pConfig->readBoolEntry("CommonScreen", _defCommonScreen);
    
    m_numRenderers = m_bDrawBackgroundPerScreen ? QApplication::desktop()->numScreens() : 1;
    
    m_bFinished.resize(m_numRenderers);
    m_bFinished.fill(false);
    
    if (m_numRenderers == m_renderer.size())
        return;
    
    for (unsigned i=0; i<m_renderer.size(); ++i)
        delete m_renderer[i];
    
    m_renderer.resize(m_numRenderers);
    for (unsigned i=0; i<m_numRenderers; ++i)
    {
        int eScreen = m_bCommonScreen ? 0 : i;
        KBackgroundRenderer * r = new KBackgroundRenderer( m_desk, eScreen, m_bDrawBackgroundPerScreen, m_pConfig );
        m_renderer.insert( i, r );
        r->setSize(renderSize(i));
        connect( r, SIGNAL(imageDone(int,int)), this, SLOT(screenDone(int,int)) );
    }
}


void KVirtualBGRenderer::load(int desk, bool reparseConfig)
{
    m_desk = desk;
    
    m_pConfig->setGroup("Background Common");
    m_bCommonScreen = m_pConfig->readBoolEntry("CommonScreen", _defCommonScreen);
    
    initRenderers();
    
    for (unsigned i=0; i<m_numRenderers; ++i)
    {
        unsigned eScreen = m_bCommonScreen ? 0 : i;
        m_renderer[i]->load(desk, eScreen, m_bDrawBackgroundPerScreen, reparseConfig);
    }
}


void KVirtualBGRenderer::screenDone(int _desk, int _screen)
{
    Q_UNUSED(_desk);
    Q_UNUSED(_screen);
    
    const KBackgroundRenderer * sender = dynamic_cast<const KBackgroundRenderer*>(this->sender());
    int screen = m_renderer.find(sender);
    if (screen == -1)
        //??
        return;
    
    m_bFinished[screen] = true;
    
    
    if (m_pPixmap)
    {
        // There's more than one renderer, so we are drawing each output to our own pixmap
        
        QRect overallGeometry;
        for (int i=0; i < QApplication::desktop()->numScreens(); ++i)
            overallGeometry |= QApplication::desktop()->screenGeometry(i);
        
        QPoint drawPos = QApplication::desktop()->screenGeometry(screen).topLeft() - overallGeometry.topLeft();
        drawPos.setX( int(drawPos.x() * m_scaleX) );
        drawPos.setY( int(drawPos.y() * m_scaleY) );
        
        QPixmap source = m_renderer[screen]->pixmap();
        QSize renderSize = this->renderSize(screen);
        renderSize.setWidth( int(renderSize.width() * m_scaleX) );
        renderSize.setHeight( int(renderSize.height() * m_scaleY) );
        
        QPainter p(m_pPixmap);
        
        if (renderSize == source.size())
            p.drawPixmap( drawPos, source );
        
        else
            p.drawTiledPixmap( drawPos.x(), drawPos.y(), renderSize.width(), renderSize.height(), source );
        
        p.end();
    }
    
    for (unsigned i=0; i<m_bFinished.size(); ++i)
    {
        if (!m_bFinished[i])
            return;
    }
    
    emit imageDone(m_desk);
}


void KVirtualBGRenderer::start()
{
    if (m_pPixmap)
    {
        delete m_pPixmap;
        m_pPixmap = 0l;
    }
    
    if (m_numRenderers > 1)
    {
        m_pPixmap = new QPixmap(m_size);
        // If are screen sizes do not properly tile the overall virtual screen
        // size, then we want the untiled parts to be black for use in desktop
        // previews, etc
        m_pPixmap->fill(Qt::black);
    }
    
    m_bFinished.fill(false);
    for (unsigned i=0; i<m_numRenderers; ++i)
        m_renderer[i]->start();
}


void KVirtualBGRenderer::stop()
{
    for (unsigned i=0; i<m_numRenderers; ++i)
        m_renderer[i]->stop();
}


void KVirtualBGRenderer::cleanup()
{
    m_bFinished.fill(false);
    
    for (unsigned i=0; i<m_numRenderers; ++i)
        m_renderer[i]->cleanup();
    
    delete m_pPixmap;
    m_pPixmap = 0l;
}

void KVirtualBGRenderer::saveCacheFile()
{
    for (unsigned i=0; i<m_numRenderers; ++i)
        m_renderer[i]->saveCacheFile();
}

void KVirtualBGRenderer::enableTiling( bool enable )
{
    for (unsigned i=0; i<m_numRenderers; ++i)
        m_renderer[i]->enableTiling( enable );
}

//END class KVirtualBGRenderer


#include "bgrender.moc"
