#include "driver.h"
#include <kapplication.h>
#include <iostream>
#include <dcopclient.h>
#include <kcmdlineargs.h>
#include <qtimer.h>
#include <qtimer.h>

using namespace std;

#ifdef _MSC_VER
#define __PRETTY_FUNCTION__ __FUNCTION__
#endif

Driver::Driver(const char* app)
	:Test_stub( app, "TestInterface" ),
	 DCOPStub( app, "TestInterface" ), // DCOPStub is *virtual* inherited
	count( 0 )
{

}

QTextStream output(  fopen( "driver.returns", "w" ) , IO_WriteOnly );	
#include <iostream>
void Driver::test()
{
	// This is written like this to allow the potentially ASYNC calls to be syncronized
	// Just sleeping would mean that no errors were reported until much later
	// I could improve it, so that we don't sleep after a synchronous call, but I will
	// leave it for later
	
	std::cerr  << __PRETTY_FUNCTION__ << " count: " << count << '\n';
	
	Driver* object = this;
	switch ( count ) {
#include "driver.generated"
		default:
			exit( 0 );
	}

	++count;
	QTimer::singleShot( 100, this, SLOT( test() ) );
}

#include "driver.moc"

#ifdef Q_OS_WIN
# define main kdemain
#endif

int main(int argc, char** argv)
{
	if ( argc < 2 ) { qWarning("Usage: driver <appid>"); return 1; }
	const char* appname = strdup( argv[ 1 ] );
	argv[ 1 ] = 0; // sue me
	KCmdLineArgs::init( argc, argv, "TestAppDriver", "Tests the dcop familly of tools + libraries", "1.0" ); // FIXME
	KApplication app;
	app.dcopClient()->attach(  );
	app.dcopClient()->registerAs( "TestAppDriver" );
	Driver * object = new Driver( appname );
	QTimer::singleShot( 10, object, SLOT( test() ) );
	return app.exec();
}
	
	
