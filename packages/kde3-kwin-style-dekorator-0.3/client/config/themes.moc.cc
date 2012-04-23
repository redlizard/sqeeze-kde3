/****************************************************************************
** IconThemesConfig meta object code from reading C++ file 'themes.h'
**
** Created: Wed Dec 7 20:20:14 2005
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.5   edited Sep 2 14:41 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "themes.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *IconThemesConfig::className() const
{
    return "IconThemesConfig";
}

QMetaObject *IconThemesConfig::metaObj = 0;
static QMetaObjectCleanUp cleanUp_IconThemesConfig( "IconThemesConfig", &IconThemesConfig::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString IconThemesConfig::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "IconThemesConfig", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString IconThemesConfig::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "IconThemesConfig", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* IconThemesConfig::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = KCModule::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "item", &static_QUType_ptr, "QListViewItem", QUParameter::In }
    };
    static const QUMethod slot_0 = {"themeSelected", 1, param_slot_0 };
    static const QUMethod slot_1 = {"installNewTheme", 0, 0 };
    static const QUMethod slot_2 = {"removeSelectedTheme", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "themeSelected(QListViewItem*)", &slot_0, QMetaData::Protected },
	{ "installNewTheme()", &slot_1, QMetaData::Protected },
	{ "removeSelectedTheme()", &slot_2, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"IconThemesConfig", parentObject,
	slot_tbl, 3,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_IconThemesConfig.setMetaObject( metaObj );
    return metaObj;
}

void* IconThemesConfig::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "IconThemesConfig" ) )
	return this;
    return KCModule::qt_cast( clname );
}

bool IconThemesConfig::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: themeSelected((QListViewItem*)static_QUType_ptr.get(_o+1)); break;
    case 1: installNewTheme(); break;
    case 2: removeSelectedTheme(); break;
    default:
	return KCModule::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool IconThemesConfig::qt_emit( int _id, QUObject* _o )
{
    return KCModule::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool IconThemesConfig::qt_property( int id, int f, QVariant* v)
{
    return KCModule::qt_property( id, f, v);
}

bool IconThemesConfig::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
