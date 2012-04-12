/*
 * Author:  Nadeem Hasan <nhasan@kde.org>
 * License: GPL V2
 */

#include <kapplication.h>
#include <kinputdialog.h>
#include <kdebug.h>

#include <qstring.h>
#include <qvalidator.h>

int main( int argc, char *argv[] )
{
  KApplication app( argc, argv, "kinputdialogtest" );

  bool ok;
  QString svalue;
  int ivalue;
  double dvalue;

  svalue = KInputDialog::getText( "_caption", "_label:", "_value", &ok );
  kdDebug() << "value1: " << svalue << ", ok: " << ok << endl;

  svalue = KInputDialog::getText( "_caption", "_label:", "_value", &ok, 0L, 0L,
    &QRegExpValidator( QRegExp( "[0-9]{3}\\-[0-9]{3}\\-[0-9]{4}" ), 0L ) );
  kdDebug() << "value2: " << svalue << ", ok: " << ok << endl;

  svalue = KInputDialog::getText( "_caption", "_label:", "_value", &ok, 0L, 0L,
    0L, "900.900.900.900" );
  kdDebug() << "value1: " << svalue << ", ok: " << ok << endl;

  ivalue = KInputDialog::getInteger( "_caption", "_label:", 64, 0, 255,
    16, 16, &ok );
  kdDebug() << "value3: " << ivalue << ", ok: " << ok << endl;

  ivalue = KInputDialog::getInteger( "_caption", "_label:", 100, 0, 255,
    10, 10, &ok );
  kdDebug() << "value4: " << ivalue << ", ok: " << ok << endl;

  dvalue = KInputDialog::getDouble( "_caption", "_label:", 10, 0, 100, 0.1,
    2, &ok );
  kdDebug() << "value5: " << dvalue << ", ok: " << ok << endl;

  dvalue = KInputDialog::getDouble( "_caption", "_label:", 10, 0, 100, 2, &ok );
  kdDebug() << "value6: " << dvalue << ", ok: " << ok << endl;

  QStringList list, slvalue;
  list << "Item 1" << "Item 2" << "Item 3" << "Item 4" << "Item 5";
  svalue = KInputDialog::getItem( "_caption", "_label:", list, 1, false, &ok );
  kdDebug() << "value7: " << svalue << ", ok: " << ok << endl;

  svalue = KInputDialog::getItem( "_caption", "_label:", list, 1, true, &ok );
  kdDebug() << "value8: " << svalue << ", ok: " << ok << endl;

  QStringList select;
  select << "Item 3";
  list << "Item 6" << "Item 7" << "Item 8" << "Item 9" << "Item 10";
  slvalue = KInputDialog::getItemList( "_caption", "_label:", list, select,
    false, &ok );
  kdDebug() << "value9: " << slvalue << ", ok: " << ok << endl;

  select << "Item 5";
  slvalue = KInputDialog::getItemList( "_caption", "_label:", list, select,
    true, &ok );
  kdDebug() << "value10: " << slvalue << ", ok: " << ok << endl;
}
