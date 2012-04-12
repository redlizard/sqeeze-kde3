#include <config.h>

#include <kglobal.h>
#include <stdio.h>
#include <kapplication.h>
#include <stdlib.h>
#include <kdebug.h>
#include <assert.h>
#include <kcmdlineargs.h>

static bool check(const QString& txt, QString a, QString b)
{
  if (a.isEmpty())
     a = QString::null;
  if (b.isEmpty())
     b = QString::null;
  if (a == b) {
    kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "ok" << endl;
  }
  else {
    kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "KO !" << endl;
    exit(1);
  }
  return true;
}

void testkasciistricmp()
{
  assert( kasciistricmp( "test", "test" ) == 0 );
  assert( kasciistricmp( "test", "Test" ) == 0 );
  assert( kasciistricmp( "TeSt", "tEst" ) == 0 );

  assert( kasciistricmp( 0, 0 ) == 0 );
  assert( kasciistricmp( "", "" ) == 0 );
  assert( kasciistricmp( 0, "" ) < 0 );
  assert( kasciistricmp( "", 0 ) > 0 );

  assert( kasciistricmp( "", "foo" ) < 0 );
  assert( kasciistricmp( "foo", "" ) > 0 );

  assert( kasciistricmp( "test", "testtest" ) < 0 );
  assert( kasciistricmp( "testtest", "test" ) > 0 );

  assert( kasciistricmp( "a", "b" ) < 0 );
  assert( kasciistricmp( "b", "a" ) > 0 );
  assert( kasciistricmp( "A", "b" ) < 0 );
  assert( kasciistricmp( "b", "A" ) > 0 );
  assert( kasciistricmp( "a", "B" ) < 0 );
  assert( kasciistricmp( "B", "a" ) > 0 );
  assert( kasciistricmp( "A", "B" ) < 0 );
  assert( kasciistricmp( "B", "A" ) > 0 );
}

int main(int argc, char *argv[])
{
  KApplication::disableAutoDcopRegistration();
  KCmdLineArgs::init( argc, argv, "kglobaltest", 0, 0, 0, 0 );
  KApplication app( false, false );

  testkasciistricmp();

  printf("\nTest OK !\n");
}

