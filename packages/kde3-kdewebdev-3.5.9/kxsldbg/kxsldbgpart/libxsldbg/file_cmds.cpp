
/***************************************************************************
                          file_cmds.c  -  define file command related functions
                             -------------------
    begin                : Sat Jan 19 2002
    copyright            : (C) 2002 by Keith Isdale
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

#include <stdio.h>
#include <libxml/tree.h>
#include <libxml/catalog.h>

#include "xsldbg.h"
#include "debugXSL.h"
#include "files.h"
#include "options.h"
#include "utils.h"
#include "xsldbgthread.h"

static char buffer[500];

/**
 * xslDbgEntities:
 * 
 * Print list of entites found 
 *
 * Returns 1 on sucess,
 *         0 otherwise
 */
int
xslDbgEntities(void)
{
    int result = 0;

    if (filesEntityList()) {
        int entityIndex;
        entityInfoPtr entInfo;

        if (getThreadStatus() == XSLDBG_MSG_THREAD_RUN) {
            /* notify that we are starting new list of entity names */
            notifyListStart(XSLDBG_MSG_ENTITIY_CHANGED);
            for (entityIndex = 0;
                 entityIndex < arrayListCount(filesEntityList());
                 entityIndex++) {
                entInfo = (entityInfoPtr) arrayListGet(filesEntityList(),
                                                       entityIndex);
                if (entInfo)
                    notifyListQueue(entInfo);

            }
            notifyListSend();
            result = 1;
        } else {
            for (entityIndex = 0;
                 entityIndex < arrayListCount(filesEntityList());
                 entityIndex++) {
                entInfo = (entityInfoPtr) arrayListGet(filesEntityList(),
                                                       entityIndex);
                if (entInfo) {
		    /* display identifier of an XML entity */
                    xsldbgGenericErrorFunc(i18n("Entity %1 ").arg(xsldbgText(entInfo->SystemID)));
                    if (entInfo->PublicID)
			xsldbgGenericErrorFunc(xsldbgText(entInfo->PublicID));
		    xsldbgGenericErrorFunc("\n");
                }
            }
            if (arrayListCount(filesEntityList()) == 0) {
                xsldbgGenericErrorFunc(i18n("No external General Parsed entities present.\n"));
            } else {
		xsldbgGenericErrorFunc(i18n("\tTotal of %n entity found.", "\tTotal of %n entities found.", arrayListCount(filesEntityList())) + QString("\n"));
            }

            result = 1;
        }
    }
    return result;
}


/**
 * xslDbgSystem:
 * @arg : Is valid
 * 
 * Print what a system file @arg maps to via the current xml catalog
 *
 * Returns 1 on sucess,
 *         0 otherwise
 */
int
xslDbgSystem(const xmlChar * arg)
{
    int result = 0;
    xmlChar *name;

    if (!arg || (xmlStrlen(arg) == 0)) {
#ifdef WITH_XSLDBG_DEBUG_PROCESS
        xsltGenericError(xsltGenericErrorContext,
                         "Error: NULL argument provided\n");
#endif
        return result;
    }

    name = xmlCatalogResolveSystem(arg);
    if (getThreadStatus() == XSLDBG_MSG_THREAD_RUN) {
        if (name) {
            notifyXsldbgApp(XSLDBG_MSG_RESOLVE_CHANGE, name);
            result = 1;
            xmlFree(name);
        } else {
            notifyXsldbgApp(XSLDBG_MSG_RESOLVE_CHANGE, "");
            xsldbgGenericErrorFunc(i18n("SystemID \"%1\" was not found in current catalog.\n").arg(xsldbgText(arg)));
        }
    } else {
        if (name) {
            xsldbgGenericErrorFunc(i18n("SystemID \"%1\" maps to: \"%2\"\n").arg(xsldbgText(arg)).arg(xsldbgText(name)));
            xmlFree(name);
            result = 1;
        } else {
            xsldbgGenericErrorFunc(i18n("SystemID \"%1\" was not found in current catalog.\n").arg(xsldbgText(arg)));
        }
    }

    return result;
}


/**
 * xslDbgPublic:
 * @arg : Is valid
 * 
 * Print what a public ID @arg maps to via the current xml catalog
 *
 * Returns 1 on sucess,
 *         0 otherwise
 */
int
xslDbgPublic(const xmlChar * arg)
{
    int result = 0;
    xmlChar *name;

    if (!arg || (xmlStrlen(arg) == 0)) {
#ifdef WITH_XSLDBG_DEBUG_PROCESS
        xsltGenericError(xsltGenericErrorContext,
                         "Error: NULL argument provided\n");
#endif
        return result;
    }

    name = xmlCatalogResolvePublic(arg);
    if (getThreadStatus() == XSLDBG_MSG_THREAD_RUN) {
        if (name) {
            notifyXsldbgApp(XSLDBG_MSG_RESOLVE_CHANGE, name);
            result = 1;
            xmlFree(name);
        } else {
            notifyXsldbgApp(XSLDBG_MSG_RESOLVE_CHANGE, "");
	    xsldbgGenericErrorFunc(i18n("PublicID \"%1\" was not found in current catalog.\n").arg(xsldbgText(arg)));
        }
    } else {
        if (name) {
            xsldbgGenericErrorFunc(i18n("PublicID \"%1\" maps to: \"%2\"\n").arg(xsldbgText(arg)).arg(xsldbgText(name)));
            xmlFree(name);
            result = 1;
        } else {
	    xsldbgGenericErrorFunc(i18n("PublicID \"%1\" was not found in current catalog.\n").arg(xsldbgText(arg)));
        }
        xsltGenericError(xsltGenericErrorContext, "%s", buffer);
    }
    return result;
}


/**
 * xslDbgEncoding:
 * @arg: Is valid encoding supported by libxml2
 *
 * Set current encoding to use for output to standard output
 *
 * Returns 1 on sucess,
 *         0 otherwise
 */
int
xslDbgEncoding(xmlChar * arg)
{
    int result = 0;
    xmlChar *opts[2];

    if (!arg) {
#ifdef WITH_XSLDBG_DEBUG_PROCESS
        xsltGenericError(xsltGenericErrorContext,
                         "Error: NULL argument provided\n");
#endif
        return result;
    }

    if (splitString(arg, 1, opts) == 1) {
        if (filesSetEncoding((char *) opts[0])) {
            optionsSetStringOption(OPTIONS_ENCODING, opts[0]);
            result = 1;
        }
    } else
        xsldbgGenericErrorFunc(i18n("Error: Missing arguments for the command %1.\n").arg("encoding"));
    return result;
}


/**
 * xslDbgShellOutput:
 * @arg : Is valid, either a local file name which will be expanded 
 *        if needed, or a "file://" protocol URI
 *
 * Set the output file name to use
 *
 * Returns 1 on success, 
 *         0 otherwise
 */
int xslDbgShellOutput(const xmlChar *arg)
{
  int result = 0;
  if (arg && (xmlStrLen(arg) > 0)){
     if (!xmlStrnCmp(arg, "file://", 7)){
      /* convert URI to local file name */
      xmlChar *outputFileName = filesURItoFileName(arg);
      if (outputFileName){
	optionsSetStringOption(OPTIONS_OUTPUT_FILE_NAME,
				     outputFileName);
	notifyXsldbgApp(XSLDBG_MSG_FILE_CHANGED, 0L);
	xmlFree(outputFileName);
	result = 1;
      }
    } else if (xmlStrEqual(arg, (xmlChar*)"-")) {
      optionsSetStringOption(OPTIONS_OUTPUT_FILE_NAME,
			     NULL);
      notifyXsldbgApp(XSLDBG_MSG_FILE_CHANGED, 0L);
      result = 1;
    } else if (!xmlStrnCmp(arg, "ftp://", 6) || !xmlStrnCmp(arg, "http://", 7)){
	xsldbgGenericErrorFunc(i18n("Error: Invalid arguments for the command %1.\n").arg("output"));
	return 0;
    } else {
      /* assume that we were provided a local file name
       * that may need expanding 
       */
      xmlChar *expandedName = filesExpandName(arg);
     
      // The output file must not be the same as our SOURCE or DATA file
      if (expandedName && 
	(!xmlStrEqual(optionsGetStringOption(OPTIONS_SOURCE_FILE_NAME), expandedName)) &&
	(!xmlStrEqual(optionsGetStringOption(OPTIONS_DATA_FILE_NAME), expandedName)) ){
	   optionsSetStringOption(OPTIONS_OUTPUT_FILE_NAME, expandedName);
	   notifyXsldbgApp(XSLDBG_MSG_FILE_CHANGED, 0L);
	   xmlFree(expandedName);
	   result = 1;
      }else{
	   xsldbgGenericErrorFunc(i18n("Error: Invalid arguments for the command %1.\n").arg("output"));
      }
    }
   } else {
    xsldbgGenericErrorFunc(i18n("Error: Missing arguments for the command %1.\n").arg("output"));
  }

  return result;
}

