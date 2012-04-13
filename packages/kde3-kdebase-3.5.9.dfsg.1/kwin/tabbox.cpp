/*****************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 1999, 2000 Matthias Ettrich <ettrich@kde.org>
Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>

You can Freely distribute this program under the GNU General Public
License. See the file "COPYING" for the exact licensing terms.
******************************************************************/

//#define QT_CLEAN_NAMESPACE
#include "tabbox.h"
#include "workspace.h"
#include "client.h"
#include <qpainter.h>
#include <qlabel.h>
#include <qdrawutil.h>
#include <qstyle.h>
#include <kglobal.h>
#include <fixx11h.h>
#include <kconfig.h>
#include <klocale.h>
#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qcursor.h>
#include <kstringhandler.h>
#include <stdarg.h>
#include <kdebug.h>
#include <kglobalaccel.h>
#include <kkeynative.h>
#include <kglobalsettings.h>
#include <kiconeffect.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>

// specify externals before namespace

extern Time qt_x_time;

namespace KWinInternal
{

extern QPixmap* kwin_get_menu_pix_hack();

TabBox::TabBox( Workspace *ws, const char *name )
    : QFrame( 0, name, Qt::WNoAutoErase ), current_client( NULL ), wspace(ws)
    {
    setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    setLineWidth(2);
    setMargin(2);

    showMiniIcon = false;

    no_tasks = i18n("*** No Windows ***");
    m = DesktopMode; // init variables
    reconfigure();
    reset();
    connect(&delayedShowTimer, SIGNAL(timeout()), this, SLOT(show()));
    
    XSetWindowAttributes attr;
    attr.override_redirect = 1;
    outline_left = XCreateWindow( qt_xdisplay(), qt_xrootwin(), 0, 0, 1, 1, 0,
        CopyFromParent, CopyFromParent, CopyFromParent, CWOverrideRedirect, &attr );
    outline_right = XCreateWindow( qt_xdisplay(), qt_xrootwin(), 0, 0, 1, 1, 0,
        CopyFromParent, CopyFromParent, CopyFromParent, CWOverrideRedirect, &attr );
    outline_top = XCreateWindow( qt_xdisplay(), qt_xrootwin(), 0, 0, 1, 1, 0,
        CopyFromParent, CopyFromParent, CopyFromParent, CWOverrideRedirect, &attr );
    outline_bottom = XCreateWindow( qt_xdisplay(), qt_xrootwin(), 0, 0, 1, 1, 0,
        CopyFromParent, CopyFromParent, CopyFromParent, CWOverrideRedirect, &attr );
    }

TabBox::~TabBox()
    {
    XDestroyWindow( qt_xdisplay(), outline_left );
    XDestroyWindow( qt_xdisplay(), outline_right );
    XDestroyWindow( qt_xdisplay(), outline_top );
    XDestroyWindow( qt_xdisplay(), outline_bottom );
    }


/*!
  Sets the current mode to \a mode, either DesktopListMode or WindowsMode

  \sa mode()
 */
void TabBox::setMode( Mode mode )
    {
    m = mode;
    }


/*!
  Create list of clients on specified desktop, starting with client c
*/
void TabBox::createClientList(ClientList &list, int desktop /*-1 = all*/, Client *c, bool chain)
    {
    ClientList::size_type idx = 0;

    list.clear();

    Client* start = c;

    if ( chain )
        c = workspace()->nextFocusChainClient(c);
    else
        c = workspace()->stackingOrder().first();

    Client* stop = c;

    while ( c )
        {
        if ( ((desktop == -1) || c->isOnDesktop(desktop))
             && c->wantsTabFocus() )
            {
            if ( start == c )
                {
                list.remove( c );
                list.prepend( c );
                }
            else
                { // don't add windows that have modal dialogs
                Client* modal = c->findModal();
                if( modal == NULL || modal == c )
                    list += c;
                else if( !list.contains( modal ))
                    list += modal;
                else
                    {
                    // nothing
                    }
                }
            }

        if ( chain )
          c = workspace()->nextFocusChainClient( c );
        else
          {
          if ( idx >= (workspace()->stackingOrder().size()-1) )
            c = 0;
          else
            c = workspace()->stackingOrder()[++idx];
          }

        if ( c == stop )
            break;
        }
    }


/*!
  Resets the tab box to display the active client in WindowsMode, or the
  current desktop in DesktopListMode
 */
void TabBox::reset()
    {
    int w, h, cw = 0, wmax = 0;

    QRect r = KGlobalSettings::desktopGeometry(QCursor::pos());

    // calculate height of 1 line
    // fontheight + 1 pixel above + 1 pixel below, or 32x32 icon + 2 pixel above + below
    lineHeight = QMAX(fontMetrics().height() + 2, 32 + 4);

    if ( mode() == WindowsMode )
        {
        setCurrentClient( workspace()->activeClient());

        // get all clients to show
        createClientList(clients, options_traverse_all ? -1 : workspace()->currentDesktop(), current_client, true);

        // calculate maximum caption width
        cw = fontMetrics().width(no_tasks)+20;
        for (ClientList::ConstIterator it = clients.begin(); it != clients.end(); ++it)
          {
          cw = fontMetrics().width( (*it)->caption() );
          if ( cw > wmax ) wmax = cw;
          }

        // calculate height for the popup
        if ( clients.count() == 0 )  // height for the "not tasks" text
          {
          QFont f = font();
          f.setBold( TRUE );
          f.setPointSize( 14 );

          h = QFontMetrics(f).height()*4;
          }
        else
          {
          showMiniIcon = false;
          h = clients.count() * lineHeight;

          if ( h > (r.height()-(2*frameWidth())) )  // if too high, use mini icons
            {
            showMiniIcon = true;
            // fontheight + 1 pixel above + 1 pixel below, or 16x16 icon + 1 pixel above + below
            lineHeight = QMAX(fontMetrics().height() + 2, 16 + 2);

            h = clients.count() * lineHeight;

            if ( h > (r.height()-(2*frameWidth())) ) // if still too high, remove some clients
              {
                // how many clients to remove
                int howMany = (h - (r.height()-(2*frameWidth())))/lineHeight;
                for (; howMany; howMany--)
                  clients.remove(clients.last());

                h = clients.count() * lineHeight;
              }
            }
          }
        }
    else
        { // DesktopListMode
        showMiniIcon = false;
        desk = workspace()->currentDesktop();

        for ( int i = 1; i <= workspace()->numberOfDesktops(); i++ )
          {
          cw = fontMetrics().width( workspace()->desktopName(i) );
          if ( cw > wmax ) wmax = cw;
          }

        // calculate height for the popup (max. 16 desktops always fit in a 800x600 screen)
        h = workspace()->numberOfDesktops() * lineHeight;
        }

    // height, width for the popup
    h += 2 * frameWidth();
    w = 2*frameWidth() + 5*2 + ( showMiniIcon ? 16 : 32 ) + 8 + wmax; // 5*2=margins, ()=icon, 8=space between icon+text
    w = kClamp( w, r.width()/3 , r.width() * 4 / 5 );

    setGeometry( (r.width()-w)/2 + r.x(),
                 (r.height()-h)/2+ r.y(),
                 w, h );
    }


/*!
  Shows the next or previous item, depending on \a next
 */
void TabBox::nextPrev( bool next)
    {
    if ( mode() == WindowsMode )
        {
        Client* firstClient = NULL;
        Client* client = current_client;
        do
            {
            if ( next )
                client = workspace()->nextFocusChainClient(client);
            else
                client = workspace()->previousFocusChainClient(client);
            if (!firstClient)
                {
		// When we see our first client for the second time,
		// it's time to stop.
                firstClient = client;
                }
            else if (client == firstClient)
                {
		// No candidates found.
                client = 0;
                break;
                }
            } while ( client && !clients.contains( client ));
        setCurrentClient( client );
        }
    else if( mode() == DesktopMode )
        {
        if ( next )
            desk = workspace()->nextDesktopFocusChain( desk );
        else
            desk = workspace()->previousDesktopFocusChain( desk );
        }
    else
        { // DesktopListMode
        if ( next )
            {
            desk++;
            if ( desk > workspace()->numberOfDesktops() )
                desk = 1;
            }
        else
            {
            desk--;
            if ( desk < 1 )
                desk = workspace()->numberOfDesktops();
            }
        }

    update();
    }



/*!
  Returns the currently displayed client ( only works in WindowsMode ).
  Returns 0 if no client is displayed.
 */
Client* TabBox::currentClient()
    {
    if ( mode() != WindowsMode )
        return 0;
    if (!workspace()->hasClient( current_client ))
        return 0;
    return current_client;
    }

void TabBox::setCurrentClient( Client* c )
    {
    if( current_client != c )
        {
        current_client = c;
        updateOutline();
        }
    }

/*!
  Returns the currently displayed virtual desktop ( only works in
  DesktopListMode )
  Returns -1 if no desktop is displayed.
 */
int TabBox::currentDesktop()
    {
    if ( mode() == DesktopListMode || mode() == DesktopMode )
        return desk;
    else
        return -1;
    }


/*!
  Reimplemented to raise the tab box as well
 */
void TabBox::showEvent( QShowEvent* )
    {
    updateOutline();
    XRaiseWindow( qt_xdisplay(), outline_left );
    XRaiseWindow( qt_xdisplay(), outline_right );
    XRaiseWindow( qt_xdisplay(), outline_top );
    XRaiseWindow( qt_xdisplay(), outline_bottom );
    raise();
    }


/*!
  hide the icon box if necessary
 */
void TabBox::hideEvent( QHideEvent* )
    {
    XUnmapWindow( qt_xdisplay(), outline_left );
    XUnmapWindow( qt_xdisplay(), outline_right );
    XUnmapWindow( qt_xdisplay(), outline_top );
    XUnmapWindow( qt_xdisplay(), outline_bottom );
    }

/*!
  Paints the tab box
 */
void TabBox::drawContents( QPainter * )
    {
    QRect r(contentsRect());
    QPixmap pix(r.size());  // do double buffering to avoid flickers
    pix.fill(this, 0, 0);

    QPainter p;
    p.begin(&pix, this);

    QPixmap* menu_pix = kwin_get_menu_pix_hack();

    int iconWidth = showMiniIcon ? 16 : 32;
    int x = 0;
    int y = 0;

    if ( mode () == WindowsMode )
        {
        if ( !currentClient() )
            {
            QFont f = font();
            f.setBold( TRUE );
            f.setPointSize( 14 );

            p.setFont(f);
            p.drawText( r, AlignCenter, no_tasks);
            }
        else
            {
            for (ClientList::ConstIterator it = clients.begin(); it != clients.end(); ++it)
              {
              if ( workspace()->hasClient( *it ) )  // safety
                  {
                  // draw highlight background
                  if ( (*it) == current_client )
                    p.fillRect(x, y, r.width(), lineHeight, colorGroup().highlight());

                  // draw icon
                  QPixmap icon;
                  if ( showMiniIcon )
                    {
                    if ( !(*it)->miniIcon().isNull() )
                      icon = (*it)->miniIcon();
                    }
                  else
                    if ( !(*it)->icon().isNull() )
                      icon = (*it)->icon();
                    else if ( menu_pix )
                      icon = *menu_pix;
                
                  if( !icon.isNull())
                    {
                    if( (*it)->isMinimized())
                        KIconEffect::semiTransparent( icon );
                    p.drawPixmap( x+5, y + (lineHeight - iconWidth)/2, icon );
                    }

                  // generate text to display
                  QString s;

                  if ( !(*it)->isOnDesktop(workspace()->currentDesktop()) )
                    s = workspace()->desktopName((*it)->desktop()) + ": ";

                  if ( (*it)->isMinimized() )
                    s += QString("(") + (*it)->caption() + ")";
                  else
                    s += (*it)->caption();

                  s = KStringHandler::cPixelSqueeze(s, fontMetrics(), r.width() - 5 - iconWidth - 8);

                  // draw text
                  if ( (*it) == current_client )
                    p.setPen(colorGroup().highlightedText());
                  else if( (*it)->isMinimized())
                    {
                    QColor c1 = colorGroup().text();
                    QColor c2 = colorGroup().background();
                    // from kicker's TaskContainer::blendColors()
                    int r1, g1, b1;
                    int r2, g2, b2;

                    c1.rgb( &r1, &g1, &b1 );
                    c2.rgb( &r2, &g2, &b2 );

                    r1 += (int) ( .5 * ( r2 - r1 ) );
                    g1 += (int) ( .5 * ( g2 - g1 ) );
                    b1 += (int) ( .5 * ( b2 - b1 ) );

                    p.setPen(QColor( r1, g1, b1 ));
                    }
                  else
                    p.setPen(colorGroup().text());

                  p.drawText(x+5 + iconWidth + 8, y, r.width() - 5 - iconWidth - 8, lineHeight,
                              Qt::AlignLeft | Qt::AlignVCenter | Qt::SingleLine, s);

                  y += lineHeight;
                  }
              if ( y >= r.height() ) break;
              }
            }
        }
    else
        { // DesktopMode || DesktopListMode
        int iconHeight = iconWidth;

        // get widest desktop name/number
        QFont f(font());
        f.setBold(true);
        f.setPixelSize(iconHeight - 4);  // pixel, not point because I need to know the pixels
        QFontMetrics fm(f);

        int wmax = 0;
        for ( int i = 1; i <= workspace()->numberOfDesktops(); i++ )
            {
            wmax = QMAX(wmax, fontMetrics().width(workspace()->desktopName(i)));

            // calculate max width of desktop-number text
            QString num = QString::number(i);
            iconWidth = QMAX(iconWidth - 4, fm.boundingRect(num).width()) + 4;
            }

        // In DesktopMode, start at the current desktop
        // In DesktopListMode, start at desktop #1
        int iDesktop = (mode() == DesktopMode) ? workspace()->currentDesktop() : 1;
        for ( int i = 1; i <= workspace()->numberOfDesktops(); i++ )
            {
            // draw highlight background
            if ( iDesktop == desk )  // current desktop
              p.fillRect(x, y, r.width(), lineHeight, colorGroup().highlight());

            p.save();

            // draw "icon" (here: number of desktop)
            p.fillRect(x+5, y+2, iconWidth, iconHeight, colorGroup().base());
            p.setPen(colorGroup().text());
            p.drawRect(x+5, y+2, iconWidth, iconHeight);

            // draw desktop-number
            p.setFont(f);
            QString num = QString::number(iDesktop);
            p.drawText(x+5, y+2, iconWidth, iconHeight, Qt::AlignCenter, num);

            p.restore();

            // draw desktop name text
            if ( iDesktop == desk )
              p.setPen(colorGroup().highlightedText());
            else
              p.setPen(colorGroup().text());

            p.drawText(x+5 + iconWidth + 8, y, r.width() - 5 - iconWidth - 8, lineHeight,
                       Qt::AlignLeft | Qt::AlignVCenter | Qt::SingleLine,
                       workspace()->desktopName(iDesktop));

            // show mini icons from that desktop aligned to each other
            int x1 = x + 5 + iconWidth + 8 + wmax + 5;

            ClientList list;
            createClientList(list, iDesktop, 0, false);
            // clients are in reversed stacking order
            for (ClientList::ConstIterator it = list.fromLast(); it != list.end(); --it)
              {
              if ( !(*it)->miniIcon().isNull() )
                {
                if ( x1+18 >= x+r.width() )  // only show full icons
                  break;

                p.drawPixmap( x1, y + (lineHeight - 16)/2, (*it)->miniIcon() );
                x1 += 18;
                }
              }

            // next desktop
            y += lineHeight;
            if ( y >= r.height() ) break;

            if( mode() == DesktopMode )
                iDesktop = workspace()->nextDesktopFocusChain( iDesktop );
            else
                iDesktop++;
            }
        }
    p.end();
    bitBlt(this, r.x(), r.y(), &pix);
    }

void TabBox::updateOutline()
    {
    Client* c = currentClient();
    if( !options->tabboxOutline || c == NULL || this->isHidden() || !c->isShown( true ) || !c->isOnCurrentDesktop())
        {
        XUnmapWindow( qt_xdisplay(), outline_left );
        XUnmapWindow( qt_xdisplay(), outline_right );
        XUnmapWindow( qt_xdisplay(), outline_top );
        XUnmapWindow( qt_xdisplay(), outline_bottom );
        return;
        }
    // left/right parts are between top/bottom, they don't reach as far as the corners
    XMoveResizeWindow( qt_xdisplay(), outline_left, c->x(), c->y() + 5, 5, c->height() - 10 );
    XMoveResizeWindow( qt_xdisplay(), outline_right, c->x() + c->width() - 5, c->y() + 5, 5, c->height() - 10 );
    XMoveResizeWindow( qt_xdisplay(), outline_top, c->x(), c->y(), c->width(), 5 );
    XMoveResizeWindow( qt_xdisplay(), outline_bottom, c->x(), c->y() + c->height() - 5, c->width(), 5 );
    {
    QPixmap pix( 5, c->height() - 10 );
    QPainter p( &pix );
    p.setPen( white );
    p.drawLine( 0, 0, 0, pix.height() - 1 );
    p.drawLine( 4, 0, 4, pix.height() - 1 );
    p.setPen( gray );
    p.drawLine( 1, 0, 1, pix.height() - 1 );
    p.drawLine( 3, 0, 3, pix.height() - 1 );
    p.setPen( black );
    p.drawLine( 2, 0, 2, pix.height() - 1 );
    p.end();
    XSetWindowBackgroundPixmap( qt_xdisplay(), outline_left, pix.handle());
    XSetWindowBackgroundPixmap( qt_xdisplay(), outline_right, pix.handle());
    }
    {
    QPixmap pix( c->width(), 5 );
    QPainter p( &pix );
    p.setPen( white );
    p.drawLine( 0, 0, pix.width() - 1 - 0, 0 );
    p.drawLine( 4, 4, pix.width() - 1 - 4, 4 );
    p.drawLine( 0, 0, 0, 4 );
    p.drawLine( pix.width() - 1 - 0, 0, pix.width() - 1 - 0, 4 );
    p.setPen( gray );
    p.drawLine( 1, 1, pix.width() - 1 - 1, 1 );
    p.drawLine( 3, 3, pix.width() - 1 - 3, 3 );
    p.drawLine( 1, 1, 1, 4 );
    p.drawLine( 3, 3, 3, 4 );
    p.drawLine( pix.width() - 1 - 1, 1, pix.width() - 1 - 1, 4 );
    p.drawLine( pix.width() - 1 - 3, 3, pix.width() - 1 - 3, 4 );
    p.setPen( black );
    p.drawLine( 2, 2, pix.width() - 1 - 2, 2 );
    p.drawLine( 2, 2, 2, 4 );
    p.drawLine( pix.width() - 1 - 2, 2, pix.width() - 1 - 2, 4 );
    p.end();
    XSetWindowBackgroundPixmap( qt_xdisplay(), outline_top, pix.handle());
    }
    {
    QPixmap pix( c->width(), 5 );
    QPainter p( &pix );
    p.setPen( white );
    p.drawLine( 4, 0, pix.width() - 1 - 4, 0 );
    p.drawLine( 0, 4, pix.width() - 1 - 0, 4 );
    p.drawLine( 0, 4, 0, 0 );
    p.drawLine( pix.width() - 1 - 0, 4, pix.width() - 1 - 0, 0 );
    p.setPen( gray );
    p.drawLine( 3, 1, pix.width() - 1 - 3, 1 );
    p.drawLine( 1, 3, pix.width() - 1 - 1, 3 );
    p.drawLine( 3, 1, 3, 0 );
    p.drawLine( 1, 3, 1, 0 );
    p.drawLine( pix.width() - 1 - 3, 1, pix.width() - 1 - 3, 0 );
    p.drawLine( pix.width() - 1 - 1, 3, pix.width() - 1 - 1, 0 );
    p.setPen( black );
    p.drawLine( 2, 2, pix.width() - 1 - 2, 2 );
    p.drawLine( 2, 0, 2, 2 );
    p.drawLine( pix.width() - 1 - 2, 0, pix.width() - 1 - 2, 2 );
    p.end();
    XSetWindowBackgroundPixmap( qt_xdisplay(), outline_bottom, pix.handle());
    }
    XClearWindow( qt_xdisplay(), outline_left );
    XClearWindow( qt_xdisplay(), outline_right );
    XClearWindow( qt_xdisplay(), outline_top );
    XClearWindow( qt_xdisplay(), outline_bottom );
    XMapWindow( qt_xdisplay(), outline_left );
    XMapWindow( qt_xdisplay(), outline_right );
    XMapWindow( qt_xdisplay(), outline_top );
    XMapWindow( qt_xdisplay(), outline_bottom );
    }

void TabBox::hide()
    {
    delayedShowTimer.stop();
    QWidget::hide();
    QApplication::syncX();
    XEvent otherEvent;
    while (XCheckTypedEvent (qt_xdisplay(), EnterNotify, &otherEvent ) )
        ;
    }


void TabBox::reconfigure()
    {
    KConfig * c(KGlobal::config());
    c->setGroup("TabBox");
    options_traverse_all = c->readBoolEntry("TraverseAll", false );
    }

/*!
  Rikkus: please document!   (Matthias)

  Ok, here's the docs :)

  You call delayedShow() instead of show() directly.

  If the 'ShowDelay' setting is false, show() is simply called.

  Otherwise, we start a timer for the delay given in the settings and only
  do a show() when it times out.

  This means that you can alt-tab between windows and you don't see the
  tab box immediately. Not only does this make alt-tabbing faster, it gives
  less 'flicker' to the eyes. You don't need to see the tab box if you're
  just quickly switching between 2 or 3 windows. It seems to work quite
  nicely.
 */
void TabBox::delayedShow()
    {
    KConfig * c(KGlobal::config());
    c->setGroup("TabBox");
    bool delay = c->readBoolEntry("ShowDelay", true);

    if (!delay)
        {
        show();
        return;
        }

    int delayTime = c->readNumEntry("DelayTime", 90);
    delayedShowTimer.start(delayTime, true);
    }


void TabBox::handleMouseEvent( XEvent* e )
    {
    XAllowEvents( qt_xdisplay(), AsyncPointer, qt_x_time );
    if( e->type != ButtonPress )
        return;
    QPoint pos( e->xbutton.x_root, e->xbutton.y_root );
    if( !geometry().contains( pos ))
        {
        workspace()->closeTabBox();  // click outside closes tab
        return;
        }
    pos.rx() -= x(); // pos is now inside tabbox
    pos.ry() -= y();
    int num = (pos.y()-frameWidth()) / lineHeight;

    if( mode() == WindowsMode )
        {
        for( ClientList::ConstIterator it = clients.begin();
             it != clients.end();
             ++it)
            {
            if( workspace()->hasClient( *it ) && (num == 0) ) // safety
                {
                setCurrentClient( *it );
                break;
                }
            num--;
            }
        }
    else
        {
        int iDesktop = (mode() == DesktopMode) ? workspace()->currentDesktop() : 1;
        for( int i = 1;
             i <= workspace()->numberOfDesktops();
             ++i )
            {
            if( num == 0 )
                {
                desk = iDesktop;
                break;
                }
            num--;
            if( mode() == DesktopMode )
                iDesktop = workspace()->nextDesktopFocusChain( iDesktop );
            else
                iDesktop++;
            }
        }
    update();
    }

//*******************************
// Workspace
//*******************************


/*!
  Handles alt-tab / control-tab
 */

static
bool areKeySymXsDepressed( bool bAll, const uint keySyms[], int nKeySyms )
    {
    char keymap[32];

    kdDebug(125) << "areKeySymXsDepressed: " << (bAll ? "all of " : "any of ") << nKeySyms << endl;

    XQueryKeymap( qt_xdisplay(), keymap );

    for( int iKeySym = 0; iKeySym < nKeySyms; iKeySym++ )
        {
        uint keySymX = keySyms[ iKeySym ];
        uchar keyCodeX = XKeysymToKeycode( qt_xdisplay(), keySymX );
        int i = keyCodeX / 8;
        char mask = 1 << (keyCodeX - (i * 8));

        kdDebug(125) << iKeySym << ": keySymX=0x" << QString::number( keySymX, 16 )
                << " i=" << i << " mask=0x" << QString::number( mask, 16 )
                << " keymap[i]=0x" << QString::number( keymap[i], 16 ) << endl;

                // Abort if bad index value,
        if( i < 0 || i >= 32 )
                return false;

                // If ALL keys passed need to be depressed,
        if( bAll )
            {
            if( (keymap[i] & mask) == 0 )
                    return false;
            }
        else
            {
                        // If we are looking for ANY key press, and this key is depressed,
            if( keymap[i] & mask )
                    return true;
            }
        }

        // If we were looking for ANY key press, then none was found, return false,
        // If we were looking for ALL key presses, then all were found, return true.
    return bAll;
    }

static bool areModKeysDepressed( const KKeySequence& seq )
    {
    uint rgKeySyms[10];
    int nKeySyms = 0;
    if( seq.isNull())
	return false;
    int mod = seq.key(seq.count()-1).modFlags();

    if ( mod & KKey::SHIFT )
        {
        rgKeySyms[nKeySyms++] = XK_Shift_L;
        rgKeySyms[nKeySyms++] = XK_Shift_R;
        }
    if ( mod & KKey::CTRL )
        {
        rgKeySyms[nKeySyms++] = XK_Control_L;
        rgKeySyms[nKeySyms++] = XK_Control_R;
        }
    if( mod & KKey::ALT )
        {
        rgKeySyms[nKeySyms++] = XK_Alt_L;
        rgKeySyms[nKeySyms++] = XK_Alt_R;
        }
    if( mod & KKey::WIN )
        {
        // It would take some code to determine whether the Win key
        // is associated with Super or Meta, so check for both.
        // See bug #140023 for details.
        rgKeySyms[nKeySyms++] = XK_Super_L;
        rgKeySyms[nKeySyms++] = XK_Super_R;
        rgKeySyms[nKeySyms++] = XK_Meta_L;
        rgKeySyms[nKeySyms++] = XK_Meta_R;
        }

    return areKeySymXsDepressed( false, rgKeySyms, nKeySyms );
    }

static bool areModKeysDepressed( const KShortcut& cut )
    {
    for( unsigned int i = 0;
	 i < cut.count();
	 ++i )
	{
	if( areModKeysDepressed( cut.seq( i )))
	    return true;
	}
    return false;
    }

void Workspace::slotWalkThroughWindows()
    {
    if ( root != qt_xrootwin() )
        return;
    if ( tab_grab || control_grab )
        return;
    if ( options->altTabStyle == Options::CDE || !options->focusPolicyIsReasonable())
        {
        //XUngrabKeyboard(qt_xdisplay(), qt_x_time); // need that because of accelerator raw mode
        // CDE style raise / lower
        CDEWalkThroughWindows( true );
        }
    else
        {
        if ( areModKeysDepressed( cutWalkThroughWindows ) )
            {
            if ( startKDEWalkThroughWindows() )
                KDEWalkThroughWindows( true );
            }
        else
            // if the shortcut has no modifiers, don't show the tabbox,
            // don't grab, but simply go to the next window
            KDEOneStepThroughWindows( true );
        }
    }

void Workspace::slotWalkBackThroughWindows()
    {
    if ( root != qt_xrootwin() )
        return;
    if( tab_grab || control_grab )
        return;
    if ( options->altTabStyle == Options::CDE || !options->focusPolicyIsReasonable())
        {
        // CDE style raise / lower
        CDEWalkThroughWindows( false );
        }
    else
        {
        if ( areModKeysDepressed( cutWalkThroughWindowsReverse ) )
            {
            if ( startKDEWalkThroughWindows() )
                KDEWalkThroughWindows( false );
            }
        else
            {
            KDEOneStepThroughWindows( false );
            }
        }
    }

void Workspace::slotWalkThroughDesktops()
    {
    if ( root != qt_xrootwin() )
        return;
    if( tab_grab || control_grab )
        return;
    if ( areModKeysDepressed( cutWalkThroughDesktops ) )
        {
        if ( startWalkThroughDesktops() )
            walkThroughDesktops( true );
        }
    else
        {
        oneStepThroughDesktops( true );
        }
    }

void Workspace::slotWalkBackThroughDesktops()
    {
    if ( root != qt_xrootwin() )
        return;
    if( tab_grab || control_grab )
        return;
    if ( areModKeysDepressed( cutWalkThroughDesktopsReverse ) )
        {
        if ( startWalkThroughDesktops() )
            walkThroughDesktops( false );
        }
    else
        {
        oneStepThroughDesktops( false );
        }
    }

void Workspace::slotWalkThroughDesktopList()
    {
    if ( root != qt_xrootwin() )
        return;
    if( tab_grab || control_grab )
        return;
    if ( areModKeysDepressed( cutWalkThroughDesktopList ) )
        {
        if ( startWalkThroughDesktopList() )
            walkThroughDesktops( true );
        }
    else
        {
        oneStepThroughDesktopList( true );
        }
    }

void Workspace::slotWalkBackThroughDesktopList()
    {
    if ( root != qt_xrootwin() )
        return;
    if( tab_grab || control_grab )
        return;
    if ( areModKeysDepressed( cutWalkThroughDesktopListReverse ) )
        {
        if ( startWalkThroughDesktopList() )
            walkThroughDesktops( false );
        }
    else
        {
        oneStepThroughDesktopList( false );
        }
    }

bool Workspace::startKDEWalkThroughWindows()
    {
    if( !establishTabBoxGrab())
        return false;
    tab_grab        = TRUE;
    keys->suspend( true );
    disable_shortcuts_keys->suspend( true );
    client_keys->suspend( true );
    tab_box->setMode( TabBox::WindowsMode );
    tab_box->reset();
    return TRUE;
    }

bool Workspace::startWalkThroughDesktops( int mode )
    {
    if( !establishTabBoxGrab())
        return false;
    control_grab = TRUE;
    keys->suspend( true );
    disable_shortcuts_keys->suspend( true );
    client_keys->suspend( true );
    tab_box->setMode( (TabBox::Mode) mode );
    tab_box->reset();
    return TRUE;
    }

bool Workspace::startWalkThroughDesktops()
    {
    return startWalkThroughDesktops( TabBox::DesktopMode );
    }

bool Workspace::startWalkThroughDesktopList()
    {
    return startWalkThroughDesktops( TabBox::DesktopListMode );
    }

void Workspace::KDEWalkThroughWindows( bool forward )
    {
    tab_box->nextPrev( forward );
    tab_box->delayedShow();
    }

void Workspace::walkThroughDesktops( bool forward )
    {
    tab_box->nextPrev( forward );
    tab_box->delayedShow();
    }

void Workspace::CDEWalkThroughWindows( bool forward )
    {
    Client* c = NULL;
// this function find the first suitable client for unreasonable focus
// policies - the topmost one, with some exceptions (can't be keepabove/below,
// otherwise it gets stuck on them)
    Q_ASSERT( block_stacking_updates == 0 );
    for( ClientList::ConstIterator it = stacking_order.fromLast();
         it != stacking_order.end();
         --it )
        {
        if ( (*it)->isOnCurrentDesktop() && !(*it)->isSpecialWindow()
            && (*it)->isShown( false ) && (*it)->wantsTabFocus()
            && !(*it)->keepAbove() && !(*it)->keepBelow())
            {
            c = *it;
            break;
            }
        }
    Client* nc = c;
    bool options_traverse_all;
        {
        KConfigGroupSaver saver( KGlobal::config(), "TabBox" );
        options_traverse_all = KGlobal::config()->readBoolEntry("TraverseAll", false );
        }

    Client* firstClient = 0;
    do
        {
        nc = forward ? nextStaticClient(nc) : previousStaticClient(nc);
        if (!firstClient)
            {
            // When we see our first client for the second time,
            // it's time to stop.
            firstClient = nc;
            }
        else if (nc == firstClient)
            {
            // No candidates found.
            nc = 0;
            break;
            }
        } while (nc && nc != c &&
            (( !options_traverse_all && !nc->isOnDesktop(currentDesktop())) ||
             nc->isMinimized() || !nc->wantsTabFocus() || nc->keepAbove() || nc->keepBelow() ) );
    if (nc)
        {
        if (c && c != nc)
            lowerClient( c );
        if ( options->focusPolicyIsReasonable() )
            {
            activateClient( nc );
            if( nc->isShade() && options->shadeHover )
                nc->setShade( ShadeActivated );
            }
        else
            {
            if( !nc->isOnDesktop( currentDesktop()))
                setCurrentDesktop( nc->desktop());
            raiseClient( nc );
            }
        }
    }

void Workspace::KDEOneStepThroughWindows( bool forward )
    {
    tab_box->setMode( TabBox::WindowsMode );
    tab_box->reset();
    tab_box->nextPrev( forward );
    if( Client* c = tab_box->currentClient() )
        {
        activateClient( c );
        if( c->isShade() && options->shadeHover )
            c->setShade( ShadeActivated );
        }
    }

void Workspace::oneStepThroughDesktops( bool forward, int mode )
    {
    tab_box->setMode( (TabBox::Mode) mode );
    tab_box->reset();
    tab_box->nextPrev( forward );
    if ( tab_box->currentDesktop() != -1 )
        setCurrentDesktop( tab_box->currentDesktop() );
    }

void Workspace::oneStepThroughDesktops( bool forward )
    {
    oneStepThroughDesktops( forward, TabBox::DesktopMode );
    }

void Workspace::oneStepThroughDesktopList( bool forward )
    {
    oneStepThroughDesktops( forward, TabBox::DesktopListMode );
    }

/*!
  Handles holding alt-tab / control-tab
 */
void Workspace::tabBoxKeyPress( const KKeyNative& keyX )
    {
    bool forward = false;
    bool backward = false;

    if (tab_grab)
        {
        forward = cutWalkThroughWindows.contains( keyX );
        backward = cutWalkThroughWindowsReverse.contains( keyX );
        if (forward || backward)
            {
            kdDebug(125) << "== " << cutWalkThroughWindows.toStringInternal()
                << " or " << cutWalkThroughWindowsReverse.toStringInternal() << endl;
            KDEWalkThroughWindows( forward );
            }
        }
    else if (control_grab)
        {
        forward = cutWalkThroughDesktops.contains( keyX ) ||
                  cutWalkThroughDesktopList.contains( keyX );
        backward = cutWalkThroughDesktopsReverse.contains( keyX ) ||
                   cutWalkThroughDesktopListReverse.contains( keyX );
        if (forward || backward)
            walkThroughDesktops(forward);
        }

    if (control_grab || tab_grab)
        {
        uint keyQt = keyX.keyCodeQt();
        if ( ((keyQt & 0xffff) == Qt::Key_Escape)
            && !(forward || backward) )
            { // if Escape is part of the shortcut, don't cancel
            closeTabBox();
            }
        }
    }

void Workspace::closeTabBox()
    {
    removeTabBoxGrab();
    tab_box->hide();
    keys->suspend( false );
    disable_shortcuts_keys->suspend( false );
    client_keys->suspend( false );
    tab_grab = FALSE;
    control_grab = FALSE;
    }

/*!
  Handles alt-tab / control-tab releasing
 */
void Workspace::tabBoxKeyRelease( const XKeyEvent& ev )
    {
    unsigned int mk = ev.state &
        (KKeyNative::modX(KKey::SHIFT) |
         KKeyNative::modX(KKey::CTRL) |
         KKeyNative::modX(KKey::ALT) |
         KKeyNative::modX(KKey::WIN));
    // ev.state is state before the key release, so just checking mk being 0 isn't enough
    // using XQueryPointer() also doesn't seem to work well, so the check that all
    // modifiers are released: only one modifier is active and the currently released
    // key is this modifier - if yes, release the grab
    int mod_index = -1;
    for( int i = ShiftMapIndex;
         i <= Mod5MapIndex;
         ++i )
        if(( mk & ( 1 << i )) != 0 )
        {
        if( mod_index >= 0 )
            return;
        mod_index = i;
        }
    bool release = false;
    if( mod_index == -1 )
        release = true;
    else
        {
        XModifierKeymap* xmk = XGetModifierMapping(qt_xdisplay());
        for (int i=0; i<xmk->max_keypermod; i++)
            if (xmk->modifiermap[xmk->max_keypermod * mod_index + i]
                == ev.keycode)
                release = true;
        XFreeModifiermap(xmk);
        }
    if( !release )
         return;
    if (tab_grab)
        {
        removeTabBoxGrab();
        tab_box->hide();
        keys->suspend( false );
        disable_shortcuts_keys->suspend( false );
        client_keys->suspend( false );
        tab_grab = false;
        if( Client* c = tab_box->currentClient())
            {
            activateClient( c );
            if( c->isShade() && options->shadeHover )
                c->setShade( ShadeActivated );
            }
        }
    if (control_grab)
        {
        removeTabBoxGrab();
        tab_box->hide();
        keys->suspend( false );
        disable_shortcuts_keys->suspend( false );
        client_keys->suspend( false );
        control_grab = False;
        if ( tab_box->currentDesktop() != -1 )
            {
            setCurrentDesktop( tab_box->currentDesktop() );
            }
        }
    }


int Workspace::nextDesktopFocusChain( int iDesktop ) const
    {
    int i = desktop_focus_chain.find( iDesktop );
    if( i >= 0 && i+1 < (int)desktop_focus_chain.size() )
            return desktop_focus_chain[i+1];
    else if( desktop_focus_chain.size() > 0 )
            return desktop_focus_chain[ 0 ];
    else
            return 1;
    }

int Workspace::previousDesktopFocusChain( int iDesktop ) const
    {
    int i = desktop_focus_chain.find( iDesktop );
    if( i-1 >= 0 )
            return desktop_focus_chain[i-1];
    else if( desktop_focus_chain.size() > 0 )
            return desktop_focus_chain[desktop_focus_chain.size()-1];
    else
            return numberOfDesktops();
    }

/*!
  auxiliary functions to travers all clients according the focus
  order. Useful for kwms Alt-tab feature.
*/
Client* Workspace::nextFocusChainClient( Client* c ) const
    {
    if ( global_focus_chain.isEmpty() )
        return 0;
    ClientList::ConstIterator it = global_focus_chain.find( c );
    if ( it == global_focus_chain.end() )
        return global_focus_chain.last();
    if ( it == global_focus_chain.begin() )
        return global_focus_chain.last();
    --it;
    return *it;
    }

/*!
  auxiliary functions to travers all clients according the focus
  order. Useful for kwms Alt-tab feature.
*/
Client* Workspace::previousFocusChainClient( Client* c ) const
    {
    if ( global_focus_chain.isEmpty() )
        return 0;
    ClientList::ConstIterator it = global_focus_chain.find( c );
    if ( it == global_focus_chain.end() )
        return global_focus_chain.first();
    ++it;
    if ( it == global_focus_chain.end() )
        return global_focus_chain.first();
    return *it;
    }

/*!
  auxiliary functions to travers all clients according the static
  order. Useful for the CDE-style Alt-tab feature.
*/
Client* Workspace::nextStaticClient( Client* c ) const
    {
    if ( !c || clients.isEmpty() )
        return 0;
    ClientList::ConstIterator it = clients.find( c );
    if ( it == clients.end() )
        return clients.first();
    ++it;
    if ( it == clients.end() )
        return clients.first();
    return *it;
    }
/*!
  auxiliary functions to travers all clients according the static
  order. Useful for the CDE-style Alt-tab feature.
*/
Client* Workspace::previousStaticClient( Client* c ) const
    {
    if ( !c || clients.isEmpty() )
        return 0;
    ClientList::ConstIterator it = clients.find( c );
    if ( it == clients.end() )
        return clients.last();
    if ( it == clients.begin() )
        return clients.last();
    --it;
    return *it;
    }

bool Workspace::establishTabBoxGrab()
    {
    if( XGrabKeyboard( qt_xdisplay(), root, FALSE,
        GrabModeAsync, GrabModeAsync, qt_x_time) != GrabSuccess )
        return false;
    // Don't try to establish a global mouse grab using XGrabPointer, as that would prevent
    // using Alt+Tab while DND (#44972). However force passive grabs on all windows
    // in order to catch MouseRelease events and close the tabbox (#67416).
    // All clients already have passive grabs in their wrapper windows, so check only
    // the active client, which may not have it.
    assert( !forced_global_mouse_grab );
    forced_global_mouse_grab = true;
    if( active_client != NULL )
        active_client->updateMouseGrab();
    return true;
    }

void Workspace::removeTabBoxGrab()
    {
    XUngrabKeyboard(qt_xdisplay(), qt_x_time);
    assert( forced_global_mouse_grab );
    forced_global_mouse_grab = false;
    if( active_client != NULL )
        active_client->updateMouseGrab();
    }

} // namespace

#include "tabbox.moc"
