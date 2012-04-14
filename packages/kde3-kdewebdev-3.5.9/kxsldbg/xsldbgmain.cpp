
/***************************************************************************
                          main.c  - main fiule for xsldbg
                             -------------------
    begin                : Sat Dec 22 2001
    copyright            : (C) 2001 by Keith Isdale
    email                : k_isdale@tpg.com.au
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kxsldbgpart/libxsldbg/xsldbg.h"

#include <libxslt/xsltutils.h>

#ifdef HAVE_READLINE
#include <readline/readline.h>
#ifdef HAVE_HISTORY
#include <readline/history.h>
#endif
#endif

#include <string.h>
#include "kxsldbgpart/libxsldbg/xsldbgmsg.h"
#include "kxsldbgpart/libxsldbg/xsldbgio.h"
#include "kxsldbgpart/libxsldbg/xsldbg.h"
#include "kxsldbgpart/libxsldbg/options.h"

#include <libxslt/xsltutils.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>
/* Forward delare private functions */
static int notifyXsldbgAppSimple(XsldbgMessageEnum type, const void *data);
static int notifyStateXsldbgAppSimple(XsldbgMessageEnum type, int commandId,
			   XsldbgCommandStateEnum commandState, const char *text);
static int notifyTextXsldbgAppSimple(XsldbgMessageEnum type, const char *text);
static xmlChar * xslDbgShellReadlineSimple(xmlChar * prompt);


static const char *description = I18N_NOOP("A KDE console application for xsldbg, an XSLT debugger");

static const char *version = VERSION;

static const KCmdLineOptions options[] =
{
    { "shell",		I18N_NOOP("Start a shell"), 0},
    { "cd <path>",	I18N_NOOP("Path to change into before loading files"), 0},
    { "param <name>:<value>",I18N_NOOP("Add a parameter named <name> and value <value> to XSL environment"), 0},
    { "lang <LANG>", 	I18N_NOOP("Use ISO 639 language code specified; for example en_US"), 0},    { "output file", 	I18N_NOOP("Save to a given file. See output command documentation"), 0},
    { "version",	I18N_NOOP("Show the version of libxml and libxslt used"), 0},
    { "verbose", 	I18N_NOOP("Show logs of what is happening"), 0},
    { "timing", 	I18N_NOOP("Display the time used"), 0},
    { "repeat", 	I18N_NOOP("Run the transformation 20 times"), 0},
#ifdef LIBXML_DEBUG_ENABLED
    { "debug", 		I18N_NOOP("Dump the tree of the result instead"), 0},
#endif
    { "novalid", 	I18N_NOOP("Disable the DTD loading phase"), 0},
    { "noout", 		I18N_NOOP("Disable the output of the result"), 0},
    { "maxdepth val", 	I18N_NOOP("Increase the maximum depth"), 0},

#ifdef LIBXML_HTML_ENABLED
    { "html", 		I18N_NOOP("The input document is(are) an HTML file(s)"), 0},
#endif

#ifdef LIBXML_DOCB_ENABLED
    { "docbook", 	I18N_NOOP("The input document is SGML docbook"), 0},
#endif

    { "nonet", 		I18N_NOOP("Disable the fetching DTDs or entities over network"), 0},

#ifdef LIBXML_CATALOG_ENABLED
    { "catalogs", 	I18N_NOOP("Use the catalogs from $SGML_CATALOG_FILES"), 0},
#endif

#ifdef LIBXML_XINCLUDE_ENABLED
    { "noxinclude", 	I18N_NOOP("Disable XInclude processing on document input"), 0},
#endif

    { "profile", 	I18N_NOOP("Print profiling informations"), 0},
    { "nogdb", 		I18N_NOOP("Do not run gdb compatability mode and print less information"), 0},
    { "autoencode", 	I18N_NOOP("Detect and use encodings in the stylesheet"), 0},
    { "utf8input", 	I18N_NOOP("Treat command line input as encoded in UTF-8"), 0},
    { "preferhtml", 	I18N_NOOP("Use HTML output when generating search reports"), 0},
    { "stdout", 	I18N_NOOP("Print all error messages to stdout, normally error messages go to stderr"), 0},
    { "noautorestart", 	I18N_NOOP("Disable the automatic restarting of execution when current processing pass is complete"), 0},
    { "+XSLSource", 	I18N_NOOP("XSL script to run"), 0},
    { "+XMLData",       I18N_NOOP("XML data to be transformed"), 0},
    KCmdLineLastOption // End of options.
};

class XsldbgApp : public KApplication
{
 public:
    XsldbgApp()
	:KApplication(false, false)
    {
	xsldbgSetAppFunc(notifyXsldbgAppSimple);
	xsldbgSetAppStateFunc(notifyStateXsldbgAppSimple);
	xsldbgSetTextFunc(notifyTextXsldbgAppSimple);
	xsldbgSetReadlineFunc(xslDbgShellReadlineSimple);
    }

    int exec(){
	return xsldbgMain(0, 0);
    }

};

int main(int argc, char **argv)
{
    KLocale::setMainCatalogue("kxsldbg"); // Translations come from KXSLDbg's catalog

    QString xsldbgRunTimeInfo(i18n("Using libxml %1, libxslt %2 and libexslt %3\n").arg(xmlParserVersion).arg(xsltEngineVersion).arg(exsltLibraryVersion));
    QString libxmlCompileTimeInfo(i18n("xsldbg was compiled against libxml %1, libxslt %2 and libexslt %3\n").arg(LIBXML_VERSION).arg(LIBXSLT_VERSION).arg(LIBEXSLT_VERSION));
    QString libxsltCompileTimeInfo(i18n("libxslt %1 was compiled against libxml %2\n").arg(xsltLibxsltVersion).arg(xsltLibxmlVersion));
    QString libexsltCompileTimeInfo(i18n("libexslt %1 was compiled against libxml %2\n").arg(exsltLibexsltVersion).arg(exsltLibxmlVersion));
    QString freeFormText = xsldbgRunTimeInfo + libxmlCompileTimeInfo + libxsltCompileTimeInfo + libexsltCompileTimeInfo;

    KAboutData about("xsldbg", I18N_NOOP("Xsldbg"), version, description, KAboutData::License_GPL, "(C) 2003 Keith Isdale", freeFormText, 0, "k_isdale@tpg.com.au");
    about.addAuthor( "Keith Isdale", 0, "k_isdale@tpg.com.au" );
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions( options );
    XsldbgApp app;

    return app.exec();;
}

/* Private implmentation of messaging functions */
int notifyXsldbgAppSimple(XsldbgMessageEnum type, const void *data)
{
  Q_UNUSED(type);
  Q_UNUSED(data);
  return 1;
}

int notifyStateXsldbgAppSimple(XsldbgMessageEnum type, int commandId,
			   XsldbgCommandStateEnum commandState, const char *text)
{
  Q_UNUSED(type);
  Q_UNUSED(commandId);
  Q_UNUSED(commandState);
  Q_UNUSED(text);
  return 1;
}

int notifyTextXsldbgAppSimple(XsldbgMessageEnum type, const char *text)
{
  Q_UNUSED(type);
  Q_UNUSED(text);
  return 1;
}


/* use this function instead of the one that was in debugXSL.c */
/**
 * xslShellReadline:
 * @prompt:  the prompt value
 *
 * Read a string
 *
 * Returns a copy of the text inputed or NULL if EOF in stdin found.
 *    The caller is expected to free the returned string.
 */
xmlChar *
xslDbgShellReadlineSimple(xmlChar * prompt)
{

  static char last_read[DEBUG_BUFFER_SIZE] = { '\0' };

#ifdef HAVE_READLINE

      xmlChar *line_read;

      if (optionsGetIntOption(OPTIONS_STDOUT) == 0){
	/* Get a line from the user. */
	line_read = (xmlChar *) readline((char *) prompt);

	/* If the line has any text in it, save it on the history. */
	if (line_read && *line_read) {
	  char *temp = (char*)line_read;
	  add_history((char *) line_read);
	  strncpy((char*)last_read, (char*)line_read, DEBUG_BUFFER_SIZE - 1);
	  /* we must ensure that the data is free properly */
	  line_read = xmlStrdup((xmlChar*)line_read);
	  free(temp);
	} else {
	  free(line_read);
	  /* if only <Enter>is pressed then try last saved command line */
	  line_read = xmlStrdup((xmlChar*)last_read);
	}
      }else{
	/* readline library will/may  echo its output which is not wanted
	   when running in gdb mode.*/
	char line_buffer[DEBUG_BUFFER_SIZE];

	if (prompt != NULL)
	  xsltGenericError(xsltGenericErrorContext, "%s", prompt);
	if (!fgets(line_buffer, sizeof(line_buffer) - 1, stdin)){
	  line_read = NULL;
	}else{
	  line_buffer[DEBUG_BUFFER_SIZE - 1] = 0;
	  if ((strlen(line_buffer) == 0) || (line_buffer[0] == '\n')){
	    line_read = xmlStrdup((xmlChar*)last_read);
	  }else{
	    add_history((char *) line_buffer);
	    line_read = xmlStrdup((xmlChar*)line_buffer);
	    strncpy((char*)last_read, (char*)line_read, sizeof(last_read) - 1);	  	  }
	}

      }
      return (line_read);

#else
      char line_read[DEBUG_BUFFER_SIZE];

      if (prompt != NULL)
        xsltGenericError(xsltGenericErrorContext, "%s", prompt);
	  fflush(stderr);
      if (!fgets(line_read, DEBUG_BUFFER_SIZE - 1, stdin))
        return (NULL);
      line_read[DEBUG_BUFFER_SIZE - 1] = 0;
      /* if only <Enter>is pressed then try last saved command line */
      if ((strlen(line_read) == 0) || (line_read[0] == '\n')) {
	  strncpy(line_read, last_read, sizeof(line_read) - 1);
      } else {
	  strcpy(last_read, line_read);
      }
      return xmlStrdup((xmlChar*)line_read);
#endif

    }
/*
void xsldbgThreadCleanup(void);
*/
/* thread has died so cleanup after it not called directly but via
 notifyXsldbgApp*/
/*
void
xsldbgThreadCleanup(void)
{

}
*/


