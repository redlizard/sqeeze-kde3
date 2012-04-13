#include <qpainter.h>
#include <qbitmap.h>
#include <qdatetime.h>
#include <qdrawutil.h>
#include <qlcdnumber.h>
#include <qtimer.h>

#include <kdatepicker.h>
#include <kglobal.h>
#include <kwin.h>
#include <kiconloader.h>
#include <kpixmap.h>
#include <kpixmapeffect.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kpopupmenu.h>

#include <taskmanager.h>

#include "kaspopup.h"
#include "kastasker.h"

#include "kasclockitem.h"
#include "kasclockitem.moc"

class LCD : public QLCDNumber
{
public:
    LCD( QWidget *parent, const char *name=0 )
	: QLCDNumber(parent,name) {}
    ~LCD() {}

    void draw( QPainter *p ) { drawContents(p); }
};

KasClockItem::KasClockItem( KasBar *parent )
    : KasItem( parent )
{
    setCustomPopup( true );

    QTimer *t = new QTimer( this );
    connect( t, SIGNAL( timeout() ), SLOT( updateTime() ) );
    t->start( 1000 );

    lcd = new LCD( parent );
    lcd->hide();

    lcd->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    lcd->setBackgroundMode( NoBackground );
    lcd->setFrameStyle( QFrame::NoFrame );
    lcd->setSegmentStyle( QLCDNumber::Flat );
    lcd->setNumDigits( 5 );
    lcd->setAutoMask( true );
    updateTime();

    connect( this, SIGNAL(leftButtonClicked(QMouseEvent *)), SLOT(togglePopup()) );
    connect( this, SIGNAL(rightButtonClicked(QMouseEvent *)), SLOT(showMenuAt(QMouseEvent *) ) );
}

KasClockItem::~KasClockItem()
{
    delete lcd;
}

KasPopup *KasClockItem::createPopup()
{
    KasPopup *pop = new KasPopup( this );
    setPopup( pop );

    (void) new KDatePicker( pop );
    pop->adjustSize();

    return pop;
}

void KasClockItem::updateTime()
{
    setText( KGlobal::locale()->formatDate( QDate::currentDate(), true /* shortFormat */ ) );
    lcd->display( KGlobal::locale()->formatTime( QTime::currentTime(), false /* includeSecs */, false /* isDuration */) );
    
    update();
}

void KasClockItem::paint( QPainter *p )
{
    KasItem::paint( p );

    lcd->setGeometry( QRect( 0, 0, extent(), extent()-15 ) );

    p->save();
    p->translate( 3, 15 );
    lcd->setPaletteForegroundColor( kasbar()->colorGroup().mid() );
    lcd->draw( p );
    p->restore();

    p->save();
    p->translate( 1, 13 );
    lcd->setPaletteForegroundColor( resources()->activePenColor() );
    lcd->draw( p );
    p->restore();
}

void KasClockItem::showMenuAt( QMouseEvent *ev )
{
    hidePopup();
    showMenuAt( ev->globalPos() );
}

void KasClockItem::showMenuAt( QPoint p )
{
    mouseLeave();
    kasbar()->updateMouseOver();

    KasTasker *bar = dynamic_cast<KasTasker *> (KasItem::kasbar());
    if ( !bar )
	return;

    KPopupMenu *menu = bar->contextMenu();
    menu->exec( p );
}
