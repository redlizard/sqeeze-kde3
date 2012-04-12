#include <stdio.h>
#include <kapplication.h>
#include <stdlib.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kcharsets.h>
#include <qtextcodec.h>
#include <krfcdate.h>

bool check(QString txt, time_t a, time_t b)
{
  if (a == b) {
    kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "ok" << endl;
  }
  else {
    kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "KO !" << endl;
    exit(1);
  }
  return true;
}

int main(int argc, char *argv[])
{
  KApplication app(argc,argv,"kurltest",false,false);

  time_t a;
  time_t b;
  
  // From http://www.w3.org/TR/NOTE-datetime
  b = KRFCDate::parseDateISO8601("1994-11-05T13:15:30Z");
  a = KRFCDate::parseDateISO8601("1994-11-05T08:15:30-05:00");
  check( "1994-11-05T08:15:30-05:00", a, b);

  a = KRFCDate::parseDateISO8601("1994-11-05T18:15:30+05:00");
  check( "1994-11-05T18:15:30+05:00", a, b);

  a = KRFCDate::parseDate("Thu Nov 5 1994 18:15:30 GMT+0500");
  check( "Thu Nov 5 1994 18:15:30 GMT+0500", a, b);

  a = KRFCDate::parseDate("Thu Nov 5 1994 18:15:30 GMT+05:00");
  check( "Thu Nov 5 1994 18:15:30 GMT+05:00", a, b);

  a = KRFCDate::parseDate("Wednesday, 05-Nov-94 13:15:30 GMT");
  check( "Wednesday, 05-Nov-94 13:15:30 GMT", a, b);

  a = KRFCDate::parseDate("Wed, 05-Nov-1994 13:15:30 GMT");
  check( "Wed, 05-Nov-1994 13:15:30 GMT", a, b);

  a = KRFCDate::parseDate("Wed, 05-November-1994 13:15:30 GMT");
  check( "Wed, 05-November-1994 13:15:30 GMT", a, b);

  b = KRFCDate::parseDateISO8601("1994-01-01T12:00:00");
  a = KRFCDate::parseDateISO8601("1994");
  check( "1994", a, b );

  a = KRFCDate::parseDateISO8601("1994-01");
  check( "1994-01", a, b );

  a = KRFCDate::parseDateISO8601("1994-01-01");
  check( "1994-01-01", a, b );

  b = 0;

  // pass RFC date to ISO parser
  a = KRFCDate::parseDateISO8601("Thu, 01 Jan 2004 19:48:21 GMT");
  check("pass RFC date \"Thu, 01 Jan 2004 19:48:21 GMT\" to parseDateISO8601", a, b);

  // pass ISO date to RFC parser
  a = KRFCDate::parseDate("1994-01-01T12:00:00");
  check("pass ISO date \"1994-01-01T12:00:00\" to parseDate()",  a, b);

  // empty/null strings

  check("parseDateISO8601(QString())", KRFCDate::parseDateISO8601(QString()), b);
  check("parseDateISO8601(\"\")", KRFCDate::parseDateISO8601(""), b);
  check("parseDate(QString())", KRFCDate::parseDate(QString()), b);
  check("parseDate(\"\")", KRFCDate::parseDate(""), b);
  
  printf("\nTest OK !\n");
}

