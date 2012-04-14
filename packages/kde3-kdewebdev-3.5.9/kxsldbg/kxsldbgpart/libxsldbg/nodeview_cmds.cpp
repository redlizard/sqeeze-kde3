
/***************************************************************************
                          nodeview_cmds.c  - node viewing commands for xsldbg
                             -------------------
    begin                : Wed Nov 21 2001
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
#include <libxml/xpathInternals.h>
#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>
#include <ctype.h>              /* for isspace*/
#include "debugXSL.h"
#include "arraylist.h"
#include "breakpoint.h"
#include "xsldbgmsg.h"
#include "xsldbgthread.h"       /* for getThreadStatus */
#include "files.h"
#include "options.h"


/* -----------------------------------------
   Private function declarations for nodeview_cmds.c
 -------------------------------------------*/
static xmlChar nodeViewBuffer[500];
static int printVariableValue = 0;

/*
 * xslDbgShellPrintNames:
 * Print a name of variable found by scanning variable table
 * It is used by print_variable function.
 * @payload : not used
 * @data : not used
 * @name : the variable name 
 */
void *xslDbgShellPrintNames(void *payload,
                            void *data, xmlChar * name);

/**
 * xslDbgCatToFile:
 * @node : Is valid
 * @file : Is valid
 *
 * Send the results of cat command in @node to @file
 */
static void xslDbgCatToFile(xmlNodePtr node, FILE * file);


/**
 * printXPathObject:
 * @item :  XPath object to print
 * @xPath : The XPath used to find item
 *
 * Print an XPath object
 *
 * Returns 1 on success,
 *         0 otherwise
 */
static int printXPathObject(xmlXPathObjectPtr item, xmlChar* xPath);

/* ------------------------------------- 
    End private functions
---------------------------------------*/


/**
 * xslDbgShellPrintList: 
 * @ctxt: The current shell context
 * @arg: What xpath to display and in UTF-8
 * @dir: If 1 print in dir mode?, 
 *        otherwise ls mode
 *
 * Print list of nodes in either ls or dir format
 *
 * Returns 1 on success,
 *         0 otherwise
 */
int
xslDbgShellPrintList(xmlShellCtxtPtr ctxt, xmlChar * arg, int dir)
{
    xmlXPathObjectPtr list;
    int result = 0;

    if (!ctxt || !arg) {
#ifdef WITH_XSLDBG_DEBUG_PROCESS
        xsltGenericError(xsltGenericErrorContext,
                         "Error: NULL arguments provided\n");
#endif
        return result;
    }

    if (arg[0] == 0) {
        if (dir)
            xmlShellDir(ctxt, NULL, ctxt->node, NULL);
        else
            xmlShellList(ctxt, NULL, ctxt->node, NULL);
        result = 1;             /*assume that this worked */
    } else {
        ctxt->pctxt->node = ctxt->node;
        ctxt->pctxt->node = ctxt->node;
        if (!xmlXPathNsLookup(ctxt->pctxt, (xmlChar *) "xsl"))
            xmlXPathRegisterNs(ctxt->pctxt, (xmlChar *) "xsl",
                               XSLT_NAMESPACE);
        list = xmlXPathEval(arg, ctxt->pctxt);
        if (list != NULL) {
            switch (list->type) {
                case XPATH_NODESET:{
                        int indx;

                        for (indx = 0;
                             indx < list->nodesetval->nodeNr; indx++) {
                            if (dir)
                                xmlShellList(ctxt, NULL,
                                             list->nodesetval->
                                             nodeTab[indx], NULL);
                            else
                                xmlShellList(ctxt, NULL,
                                             list->nodesetval->
                                             nodeTab[indx], NULL);
                        }
                        result = 1;
                        break;
                    }
                default:
                    xmlShellPrintXPathError(list->type, (char *) arg);
            }
            xmlXPathFreeObject(list);
        } else {
            xsldbgGenericErrorFunc(i18n("Error: XPath %1 results in an empty Node Set.\n").arg(xsldbgText(arg)));
        }
        ctxt->pctxt->node = NULL;
    }
    return result;
}



/**
 * xslDbgCatToFile:
 * @node : Is valid
 * @file : Is valid
 *
 * Send the results of cat command in @node to @file
 */
void
xslDbgCatToFile(xmlNodePtr node, FILE * file)
{
    if (!node || !file)
      return;

    /* assume that HTML usage is enabled */
    if (node->doc->type == XML_HTML_DOCUMENT_NODE) {
        if (node->type == XML_HTML_DOCUMENT_NODE)
            htmlDocDump(file, (htmlDocPtr) node);
        else
            htmlNodeDumpFile(file, node->doc, node);
    } else if (node->type == XML_DOCUMENT_NODE) {
        /* turn off encoding for the moment and just dump UTF-8 
         * which will be converted by xsldbgGeneralErrorFunc */
        xmlDocPtr doc = (xmlDocPtr) node;
        const xmlChar *encoding = doc->encoding;

        if (encoding) {
            xsldbgGenericErrorFunc(i18n("Information: Temporarily setting document's encoding to UTF-8. Previously was %1.\n").arg(xsldbgText(encoding)));
        }
        doc->encoding = (xmlChar *) "UTF-8";
        xmlDocDump(file, (xmlDocPtr) node);
        doc->encoding = encoding;
    } else {
	xmlElemDump(file, node->doc, node);
    }
}


/**
 * printXPathObject:
 * @item :  XPath object to print
 * @xPath : The XPath used to find item
 *
 * Print an XPath object
 *
 * Returns 1 on success,
 *         0 otherwise
 */

static int
printXPathObject(xmlXPathObjectPtr item, xmlChar* xPath){
  int result = 0;
  if (item){ 
    switch (item->type) {
    case XPATH_BOOLEAN:
      xsltGenericError(xsltGenericErrorContext,
		       "= %s\n%s\n", xPath,
		       xmlBoolToText(item->boolval));
      result = 1;
      break;

    case XPATH_NUMBER:
      xsltGenericError(xsltGenericErrorContext,
		       "= %s\n%0g\n", xPath, item->floatval);
      result = 1;
      break;

      /*  case XPATH_NODESET:*/
    default:{
	/* We may need to convert this XPath to a string,
	   plus ensure that we print required the number of
	   lines of text */
	int indx;

	const char *fileName = filesTempFileName(0);
	FILE *file = NULL;

	if (!fileName)
	  break;
	file = fopen(fileName, "w+");
	if (!file) {
	   xsldbgGenericErrorFunc(i18n("Error: Unable to save temporary results to %1.\n").arg(xsldbgText(fileName)));
	  break;
	} else {
	  fprintf(file, "= %s\n", xPath);
	  switch(item->type){

	  case XPATH_NODESET:
	    if (item->nodesetval){
	      for (indx = 0;
		   indx < item->nodesetval->nodeNr; indx++){ 
				xslDbgCatToFile(item->nodesetval->
						nodeTab[indx], file);
	      }
	    } else {
	      xsldbgGenericErrorFunc(i18n("Error: XPath %1 results in an empty Node Set.\n").arg(xsldbgText(xPath)));
	    }
	    break;
			     
	  case XPATH_STRING:
	    if (item->stringval)
	      fprintf(file, "\'%s\'", item->stringval);
	    else
	      fprintf(file, "%s", i18n("NULL string value supplied.").utf8().data());
	    break;
			     
	  default:{
	      xmlXPathObjectPtr tempObj = 
		xmlXPathObjectCopy(item);
	      if (tempObj)
		tempObj = xmlXPathConvertString(tempObj);
	      if (tempObj && tempObj->stringval){
		fprintf(file, "%s", tempObj->stringval);
	      }else{
		fprintf(file, "%s", i18n("Unable to convert XPath to string.").utf8().data());	
	      }
	      if (tempObj)
		xmlXPathFreeObject(tempObj);
	    }
	    break;
	    fprintf(file,"\n");	    

	  } /* inner switch statement  */
	  if (getThreadStatus() == XSLDBG_MSG_THREAD_RUN) {
	    fclose(file);
	    file = NULL;
	    /* send the data to application */
	    notifyXsldbgApp(XSLDBG_MSG_FILEOUT, fileName);
	  } else {
	    int lineCount = 0, gdbModeEnabled = 0;

	    /* save the value of option to speed things up
	     * a bit */
	    gdbModeEnabled =
	      optionsGetIntOption(OPTIONS_GDB);
	    rewind(file);

	    /* when gdb mode is enable then only print the first
	     * GDB_LINES_TO_PRINT lines */
	    while (!feof(file)) {
	      if (fgets
		  ((char *) nodeViewBuffer, sizeof(nodeViewBuffer),
		   file))
		xsltGenericError
		  (xsltGenericErrorContext, "%s",
		   nodeViewBuffer);
	      if (gdbModeEnabled) {
		lineCount++;
		/* there is an overhead of two lines
		 * when print expression values */
		if (lineCount ==
		    GDB_LINES_TO_PRINT + 2) {
		  xsltGenericError
		    (xsltGenericErrorContext,
		     "...");
		  break;
		}
	      }
	    }
	    xsltGenericError
	      (xsltGenericErrorContext, "\n");
	  }
	  if (file)
	    fclose(file);
	  result = 1;
	  break;
	}
      }
    }
  }
  return result;
}


/** 
 * xslDbgShellCat:
 * @styleCtxt: the current stylesheet context
 * @ctxt: The current shell context
 * @arg: The xpath to print (in UTF-8)
 *
 * Print the result of an xpath expression. This can include variables
 *        if styleCtxt is not NULL
 *
 * Returns 1 on success,
 *         0 otherwise
 */

int
xslDbgShellCat(xsltTransformContextPtr styleCtxt, xmlShellCtxtPtr ctxt,
               xmlChar * arg)
{
    xmlXPathObjectPtr list;
    int result = 0;
    static const char * QUIET_STR = "-q";
    bool silenceCtxtErrors = false;

    if ((arg == NULL) || (xmlStrLen(arg) == 0))
        arg = (xmlChar *) ".";

    /* Do we quietly ingore style context errors */
    if (strncasecmp((char*)arg, QUIET_STR, strlen(QUIET_STR))== 0){
      silenceCtxtErrors = true;	
      arg = arg + strlen(QUIET_STR);
      while (isspace(*arg)){
	arg++;
      }
    }

    if (!styleCtxt || !ctxt || !ctxt->node) {
	if (!(!xsldbgReachedFirstTemplate && silenceCtxtErrors)) 
        xsldbgGenericErrorFunc(i18n("Warning: Unable to print expression. No stylesheet was properly loaded.\n"));
        return 0;
    }

    if ((arg == NULL) || (xmlStrLen(arg) == 0))
        arg = (xmlChar *) ".";

    ctxt->pctxt->node = ctxt->node;
    if (!styleCtxt) {
        list = xmlXPathEval((xmlChar *) arg, ctxt->pctxt);
    } else {
        xmlNodePtr savenode = styleCtxt->xpathCtxt->node;

        ctxt->pctxt->node = ctxt->node;
        styleCtxt->xpathCtxt->node = ctxt->node;
        if (!xmlXPathNsLookup(styleCtxt->xpathCtxt, (xmlChar *) "xsl"))
            xmlXPathRegisterNs(styleCtxt->xpathCtxt, (xmlChar *) "xsl",
                               XSLT_NAMESPACE);
        list = xmlXPathEval((xmlChar *) arg, styleCtxt->xpathCtxt);
        styleCtxt->xpathCtxt->node = savenode;
    }
    if (list != NULL) {
        result = printXPathObject(list, arg);
        xmlXPathFreeObject(list);
    } else {
        xsldbgGenericErrorFunc(i18n("Error: XPath %1 results in an empty Node Set.\n").arg(xsldbgText(arg)));
    }
    ctxt->pctxt->node = NULL;
    return result;
}

/* only used by xslDbgPrintNames and xslDbgPrintVariable cound number of variables */
static int varCount;

/*
 * xslDbgShellPrintNames:
 * Print a name of variable found by scanning variable table
 * It is used by print_variable function.
 * @payload : Global variable of type xsltStackElemPtr
 * @data : not used
 * @name : the variable name 
 */
void *
xslDbgShellPrintNames(void *payload,
                      void *data, xmlChar * name)
{
    Q_UNUSED(payload);
    Q_UNUSED(data);
    if (getThreadStatus() == XSLDBG_MSG_THREAD_RUN) {
        notifyListQueue(payload);
    } else if (payload && name) {
        xmlChar * fullQualifiedName = nodeViewBuffer;
        xsltStackElemPtr item = (xsltStackElemPtr)payload;
	if (item->nameURI == NULL){
	    snprintf((char*)fullQualifiedName, sizeof(nodeViewBuffer), "$%s", item->name);
	}else{
	    snprintf((char*)fullQualifiedName, sizeof(nodeViewBuffer), "$%s:%s",
		     item->nameURI, item->name);
	}
        if (printVariableValue == 0){
	    xsldbgGenericErrorFunc(i18n(" Global %1\n").arg(xsldbgText(fullQualifiedName)));
        }else{
	      if (item->computed == 1){
	         xsldbgGenericErrorFunc(i18n(" Global "));
		 printXPathObject(item->value, fullQualifiedName);
	      }else if (item->tree){
	         xsldbgGenericErrorFunc(i18n(" Global = %1\n").arg(xsldbgText(fullQualifiedName)));
		 xslDbgCatToFile(item->tree, stderr);
	      }else if (item->select){
	         xsldbgGenericErrorFunc(i18n(" Global = %1\n%2").arg(xsldbgText(fullQualifiedName)).arg(xsldbgText(item->select)));
	      }else{
		/* can't find a value give only a variable name an error message */
		xsldbgGenericErrorFunc(i18n(" Global = %1\n%2").arg(xsldbgText(fullQualifiedName)).arg(i18n("Warning: No value assigned to variable.\n")));
	      }
	    xsltGenericError(xsltGenericErrorContext, "\n\032\032\n");
        }
        varCount++;
    }
    return NULL;
}



/**
 * xslDbgShellPrintVariable:
 * @styleCtxt: The current stylesheet context 
 * @arg: The name of variable to look for '$' prefix is optional and in UTF-8
 * @type: A valid VariableTypeEnum
 *
 *  Print the value variable specified by args.
 *
 * Returns 1 on success,
 *         0 otherwise
 */
int
xslDbgShellPrintVariable(xsltTransformContextPtr styleCtxt, xmlChar * arg,
                         VariableTypeEnum type)
{
    int result = 0;
    /* command argument to include both name and its value */
    static const char * FULLNAME_STR = "-f";
    /* Quietly exit if an invalid stylesheet is provided */
    static const char * QUIET_STR = "-q";
    bool silenceCtxtErrors = false;

    if (!arg) {
#ifdef WITH_XSLDBG_DEBUG_PROCESS
        xsltGenericError(xsltGenericErrorContext,
                         "Error: NULL argument provided\n");
#endif
        return result;
    }

    varCount = 0;
    /* Do we quietly ingore style context errors */
    if (strncasecmp((char*)arg, QUIET_STR, strlen(QUIET_STR))== 0){
      silenceCtxtErrors = true;	
      arg = arg + strlen(QUIET_STR);
      while (isspace(*arg)){
	arg++;
      }
    }

    if (!styleCtxt) {
	if (!(!xsldbgReachedFirstTemplate && silenceCtxtErrors)) 
	    xsldbgGenericErrorFunc(i18n("Error: Debugger has no files loaded or libxslt has not reached a template.\nTry reloading files or taking more steps.\n"));
        return result;
    }

    /* Do we include the name as well as its value */
    if (strncasecmp((char*)arg, FULLNAME_STR, strlen(FULLNAME_STR))== 0){
      printVariableValue = 1;
      arg = arg + strlen(FULLNAME_STR);
      while (isspace(*arg)){
	arg++;
      }
    }
    if (arg[0] == 0) {
        /* list variables of type requested */
        if (type == DEBUG_GLOBAL_VAR) {
            if (styleCtxt->globalVars) {
                if (getThreadStatus() == XSLDBG_MSG_THREAD_RUN) {
                    notifyListStart(XSLDBG_MSG_GLOBALVAR_CHANGED);
                    /* list global variables */
                    xmlHashScan(styleCtxt->globalVars,
                                (xmlHashScanner) xslDbgShellPrintNames,
                                NULL);
                    notifyListSend();
                } else
                    /* list global variables */
                    xmlHashScan(styleCtxt->globalVars,
                                (xmlHashScanner) xslDbgShellPrintNames,
                                NULL);
                result = 1;
                /* ensure that the locals follow imediately after the 
                 * globals when in gdb mode */
                if (optionsGetIntOption(OPTIONS_GDB) == 0)
                    xsltGenericError(xsltGenericErrorContext, "\n");
            } else {
                if (getThreadStatus() != XSLDBG_MSG_THREAD_RUN) {
                    /* Don't show this message when running as a thread as it 
                     * is annoying */
                    xsldbgGenericErrorFunc(i18n("Error: Libxslt has not initialized variables yet; try stepping to a template.\n"));
                } else {
                    /* send an empty list */
                    notifyListStart(XSLDBG_MSG_GLOBALVAR_CHANGED);
                    notifyListSend();
                    result = 1;
                }
            }
        } else {
            /* list local variables */
            if (styleCtxt->varsBase) {
                xsltStackElemPtr item =
                    styleCtxt->varsTab[styleCtxt->varsBase];
                if (getThreadStatus() == XSLDBG_MSG_THREAD_RUN) {
                    notifyListStart(XSLDBG_MSG_LOCALVAR_CHANGED);
                    while (item) {
                        notifyListQueue(item);
                        item = item->next;
                    }
                    notifyListSend();
                } else {
		    xmlChar * fullQualifiedName = nodeViewBuffer;
                    while (item) {		      
                        if (item->name) {			     
			    if (item->nameURI == NULL){
				snprintf((char*)fullQualifiedName, sizeof(nodeViewBuffer), "$%s",
					 item->name);
			    }else{

				snprintf((char*)fullQualifiedName, sizeof(nodeViewBuffer), "$%s:%s",
					 item->nameURI, item->name);
			    }
			    if (printVariableValue == 0){
				xsldbgGenericErrorFunc(i18n(" Local %1").arg(xsldbgText(fullQualifiedName)));
			    }else{
			        if (item->computed == 1){
			           xsldbgGenericErrorFunc(i18n(" Local "));
			           printXPathObject(item->value, fullQualifiedName);
			        }else if (item->tree){
			           xsldbgGenericErrorFunc(i18n(" Local = %1\n").arg(xsldbgText(fullQualifiedName)));
			           xslDbgCatToFile(item->tree, stderr);
			        }else if (item->select){
				  xsldbgGenericErrorFunc(i18n(" Local = %1\n%2").arg(xsldbgText(fullQualifiedName)).arg(xsldbgText(item->select)));
				}else{
				  /* can't find a value give only a variable name and an error */
				  xsldbgGenericErrorFunc(i18n(" Local = %1\n%2").arg(xsldbgText(fullQualifiedName)).arg(i18n("Warning: No value assigned to variable.\n")));
				}
			    }
			    xsltGenericError(xsltGenericErrorContext, "\n\032\032\n");
			}
                        item = item->next;
		    }
                }
                result = 1;
                xsltGenericError(xsltGenericErrorContext, "\n");
            } else {
                if (getThreadStatus() != XSLDBG_MSG_THREAD_RUN) {
                    /* Don't show this message when running as a thread as it 
                     * is annoying */
                    xsldbgGenericErrorFunc(i18n("Error: Libxslt has not initialized variables yet; try stepping past the xsl:param elements in the template.\n"));
                } else {
                    /* send an empty list */
                    notifyListStart(XSLDBG_MSG_LOCALVAR_CHANGED);
                    notifyListSend();
                    result = 1;
                }
            }
        }
    } else {
        /* Display the value of variable */
        if (arg[0] == '$') {
            printXPathObject(xmlXPathEval(arg, styleCtxt->xpathCtxt), arg);
	    xsltGenericError(xsltGenericErrorContext, "\032\032\n");
        } else {
            xmlStrCpy(nodeViewBuffer, "$");
            xmlStrCat(nodeViewBuffer, arg);
	    printXPathObject(xmlXPathEval((xmlChar*)nodeViewBuffer,styleCtxt->xpathCtxt),
			     (xmlChar*)nodeViewBuffer);
	    xsltGenericError(xsltGenericErrorContext, "\032\032\n");
        }

    }

    printVariableValue = 0;
    return result;
}
