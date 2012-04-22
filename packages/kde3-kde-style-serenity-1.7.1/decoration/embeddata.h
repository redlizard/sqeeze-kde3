
#ifndef EMBEDDATA_H
#define EMBEDDATA_H

#include <qimage.h>
#include <qdict.h>
#include <qmime.h>
#include <qdragobject.h>

// MXLS: I dunno what I'm doin'!
// QImage uic_findImage(const QString& name);

class MimeSourceFactory_KWinSerenity : public QMimeSourceFactory
{
	public:
		MimeSourceFactory_KWinSerenity() {}
		~MimeSourceFactory_KWinSerenity() {}
		const QMimeSource* data( const QString& abs_name ) const 
		{
			const QMimeSource* d = QMimeSourceFactory::data( abs_name );
			if ( d || abs_name.isNull() ) return d;
			// QImage img = uic_findImage( abs_name );
			// if ( !img.isNull() )
			//    ((QMimeSourceFactory*)this)->setImage( abs_name, img );
			return QMimeSourceFactory::data( abs_name );
		};
};

// MXLS: No, it isn't unused!
static QMimeSourceFactory* factory = 0;

void qInitImages_KWinSerenity();

void qCleanupImages_KWinSerenity();

class StaticInitImages_KWinSerenity
{
	public:
		StaticInitImages_KWinSerenity() { qInitImages_KWinSerenity(); }
#if defined(Q_OS_SCO) || defined(Q_OS_UNIXWARE)
		~StaticInitImages_KWinSerenity() { }
#else
		~StaticInitImages_KWinSerenity() { qCleanupImages_KWinSerenity(); }
#endif
};

static StaticInitImages_KWinSerenity staticImages;
#endif
