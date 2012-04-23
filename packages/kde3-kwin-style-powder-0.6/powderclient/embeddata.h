
#ifndef EMBEDDATA_H
#define EMBEDDATA_H

#include <qimage.h>
#include <qdict.h>
#include <qmime.h>
#include <qdragobject.h>

// MXLS: I dunno what I'm doin'!
// QImage uic_findImage(const QString& name);

class MimeSourceFactory_KWinPowder : public QMimeSourceFactory
{
public:
    MimeSourceFactory_KWinPowder() {}
    ~MimeSourceFactory_KWinPowder() {}
    const QMimeSource* data( const QString& abs_name ) const {
	const QMimeSource* d = QMimeSourceFactory::data( abs_name );
	if ( d || abs_name.isNull() ) return d;
	// QImage img = uic_findImage( abs_name );
	// if ( !img.isNull() )
	//    ((QMimeSourceFactory*)this)->setImage( abs_name, img );
	return QMimeSourceFactory::data( abs_name );
    };
};

static QMimeSourceFactory* factory = 0;

void qInitImages_KWinPowder();

void qCleanupImages_KWinPowder();

class StaticInitImages_KWinPowder
{
public:
    StaticInitImages_KWinPowder() { qInitImages_KWinPowder(); }
#if defined(Q_OS_SCO) || defined(Q_OS_UNIXWARE)
    ~StaticInitImages_KWinPowder() { }
#else
    ~StaticInitImages_KWinPowder() { qCleanupImages_KWinPowder(); }
#endif
};

static StaticInitImages_KWinPowder staticImages;
#endif
