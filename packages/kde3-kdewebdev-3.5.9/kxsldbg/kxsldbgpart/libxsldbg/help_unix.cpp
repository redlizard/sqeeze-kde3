
/***************************************************************************
                          help.c  -  help system for *nix platform
                             -------------------
    begin                : Tue Jan 29 2002
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

/*
 * Uses docs/xsldoc.xsl docs/xsldoc.xml and xslproc to generate text
 */

#include "xsldbg.h"
#include "options.h"
#include "utils.h"
#include "debugXSL.h"
#include "help.h"
#include "files.h"
#include <stdlib.h>
#include <kglobal.h>
#include <kstandarddirs.h> 

/**
 * helpTop:
 * @args : Is valid command or empty string
 *
 * Display help about the command in @args
 *
 * This is a platform specific interface
 *
 * Returns 1 on success,
 *         0 otherwise
 */
int
helpTop(const xmlChar * args)
{

    //Extra phrases to support translation of help display see kdewebdev/doc/xsldbg/xsldbghelp.xml and kdewebdev/kxsldbg/xsldbghelp.xsl
    static const char* xsldbghelp_translations[] =
    {
	I18N_NOOP("xsldbg version"),
	I18N_NOOP("Help document version"),
	I18N_NOOP("Help not found for command")
    };

    QString xsldbgVerTxt(i18n("xsldbg version"));
    QString helpDocVerTxt(i18n("Help document version"));
    QString helpErrorTxt(i18n("Help not found for command"));
    

    char buff[500], helpParam[100];
    const QString appDataDir = KGlobal::dirs()->findResourceDir("data", "xsldbg/xsldbghelp.xsl");

    const char *docsDirPath =
        (const char *) optionsGetStringOption(OPTIONS_DOCS_PATH);
    int result = 0;

    if (xmlStrLen(args) > 0) {
        snprintf(helpParam, 100, "--param help:%c'%s'%c", QUOTECHAR, args,
                 QUOTECHAR);
    } else
        xmlStrCpy(helpParam, "");
    if (docsDirPath && filesTempFileName(0)) {
        snprintf((char *) buff, sizeof(buff), "%s %s"
                 " --param xsldbg_version:%c'%s'%c "
                 " --param xsldbgVerTxt:%c'%s'%c "
                 " --param helpDocVerTxt:%c'%s'%c "
                 " --param helpErrorTxt:%c'%s'%c "
                 " --output %s "
                 " --cd %s "
                 "%sxsldbg%cxsldbghelp.xsl %s%cxsldbghelp.xml",
                 XSLDBG_BIN, helpParam,
                 QUOTECHAR, VERSION, QUOTECHAR,
                 QUOTECHAR, xsldbgVerTxt.utf8().data(), QUOTECHAR,
                 QUOTECHAR, helpDocVerTxt.utf8().data(), QUOTECHAR,
                 QUOTECHAR, helpErrorTxt.utf8().data(), QUOTECHAR,
                 filesTempFileName(0),
		 docsDirPath,
                 appDataDir.utf8().data(), PATHCHAR, docsDirPath, PATHCHAR);
        if (xslDbgShellExecute((xmlChar *) buff, optionsGetIntOption(OPTIONS_VERBOSE)) == 0) {
            if (docsDirPath)
                xsldbgGenericErrorFunc(i18n("Error: Unable to display help. Help files not found in %1 or xsldbg not found in path.\n").arg(docsDirPath)); /* FIXME: Comments not correct - the command is that invoked  */
            else
                xsldbgGenericErrorFunc(i18n("Error: Unable to find xsldbg or help files.\n"));
        } else {
            if (filesMoreFile((xmlChar*)filesTempFileName(0), NULL) == 1) {
                result = 1;
            } else {
                xsldbgGenericErrorFunc(i18n("Error: Unable to print help file.\n"));
            }
        }

    } else {
        xsldbgGenericErrorFunc(i18n("Error: No path to documentation; aborting help.\n"));
#ifdef WITH_XSLDBG_DEBUG_PROCESS
#ifdef USE_DOCS_MACRO
        xsltGenericError(xsltGenericErrorContext,"MACRO has been defined look at Makefile.am\n");
#else
        xsltGenericError(xsltGenericErrorContext,
                         "Error: Environment variable %s is not set to the directory of xsldbg documentation.\n",
                         XSLDBG_DOCS_DIR_VARIABLE);
#endif
#endif
    }
    return result;
}

