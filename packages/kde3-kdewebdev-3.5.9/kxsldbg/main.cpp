#include "kxsldbg.h"
#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kdebug.h>

static const char *description =
    I18N_NOOP("A KDE KPart Application for xsldbg, an XSLT debugger");

static const char *version = VERSION;
static const KCmdLineOptions options[] =
{
    { "+XSLSource", 	I18N_NOOP("XSL script to run"), 0},
    { "+XMLData",       I18N_NOOP("XML data to be transformed"), 0},
    { "+OutputFile",    I18N_NOOP("File to save results to"), 0},
    KCmdLineLastOption // End of options.
};

int main(int argc, char **argv)
{
    KAboutData about("kxsldbg", I18N_NOOP("KXSLDbg"), version, description, KAboutData::License_GPL, "(C) 2003 Keith Isdale", 0, 0, "k_isdale@tpg.com.au");
    about.addAuthor( "Keith Isdale", 0, "k_isdale@tpg.com.au" );
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions( options );
    KApplication app;

    // see if we are starting with session management
    if (app.isRestored())
        RESTORE(KXsldbg)
    else
    {
        KXsldbg *widget = new KXsldbg;
        widget->show();
    }

    return app.exec();
}
