
/***************************************************************************
                          xsldbg.cpp  - send message to console or KXSLDbg
                             -------------------
    begin                : Mon April 26 2004
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

#include "xsldbg.h"

#include <libxslt/xsltutils.h>

#ifdef HAVE_READLINE
#include <readline/readline.h>
#ifdef HAVE_HISTORY
#include <readline/history.h>
#endif
#endif

#include "xsldbgmsg.h"
#include "xsldbgio.h"
#include "options.h"

static int (*notifyXsldbgAppFuncPtr) (XsldbgMessageEnum type, const void *data) = 0;

static int (*notifyStateXsldbgAppFuncPtr)(XsldbgMessageEnum type, int commandId,
                             XsldbgCommandStateEnum commandState,
                             const char *text) = 0;

static int (*notifyTextXsldbgAppFuncPtr)(XsldbgMessageEnum type, const char *text) = 0;
static xmlChar * (*xslDbgShellReadlineFuncPtr)(xmlChar * prompt) = 0;


#ifdef __cplusplus
extern "C" {
#endif

void xsldbgSetAppFunc(int (*notifyXsldbgAppFunc) (XsldbgMessageEnum type, const void *data))
{
    notifyXsldbgAppFuncPtr = notifyXsldbgAppFunc;
}

void xsldbgSetAppStateFunc(int (*notifyStateXsldbgAppFunc)(XsldbgMessageEnum type, int commandId,
                             XsldbgCommandStateEnum commandState,
                             const char *text))
{
    notifyStateXsldbgAppFuncPtr = notifyStateXsldbgAppFunc;
}


void xsldbgSetTextFunc(int (*notifyTextXsldbgAppFunc)(XsldbgMessageEnum type, const char *text))
{
    notifyTextXsldbgAppFuncPtr = notifyTextXsldbgAppFunc;
}

void xsldbgSetReadlineFunc(xmlChar * (*xslDbgShellReadlineFunc)(xmlChar * prompt))
{
    xslDbgShellReadlineFuncPtr = xslDbgShellReadlineFunc;
}

int notifyXsldbgApp(XsldbgMessageEnum type, const void *data)
{
    if (!notifyXsldbgAppFuncPtr)
	return 1;
    else
	return (notifyXsldbgAppFuncPtr)(type, data);
}

int notifyStateXsldbgApp(XsldbgMessageEnum type, int commandId,
			   XsldbgCommandStateEnum commandState, const char *text)
{
    if (!notifyStateXsldbgAppFuncPtr)
	return 1;
    else
	return (notifyStateXsldbgApp)(type, commandId, commandState, text);
}

int notifyTextXsldbgApp(XsldbgMessageEnum type, const char *text)
{
    if (!notifyTextXsldbgAppFuncPtr)
	return 1;
    else
	return (notifyTextXsldbgAppFuncPtr)(type, text);
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
xslDbgShellReadline(xmlChar * prompt)
{

    if (!xslDbgShellReadlineFuncPtr)
	return 0;
    else
	return (xslDbgShellReadlineFuncPtr)(prompt);
}

#ifdef __cplusplus
}
#endif
