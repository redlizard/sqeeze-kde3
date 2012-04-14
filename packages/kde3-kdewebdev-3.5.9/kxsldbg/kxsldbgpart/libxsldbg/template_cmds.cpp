
/***************************************************************************
                          template_cmds.c  - template commands for xsldbg
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
#include "debugXSL.h"
#include "files.h"
#include "utils.h"
#include "xsldbgmsg.h"
#include "xsldbgthread.h"       /* for getThreadStatus */

static int printCounter;        /* Dangerous name think of a better one */

/* -----------------------------------------
   Private function declarations for files.c
 -------------------------------------------*/

/**
 * xslDbgShellPrintStylesheetsHelper:
 * @payload :valid xxsltStylesheetPtr
 * @data :  not used
 * name : not used
 *
 * Print out the stylesheet name from the stylesheet given to
 *   us via walkStylesheets
 */
void
  xslDbgShellPrintStylesheetsHelper(void *payload,
                                    void *data,
                                    xmlChar * name);


/**
 * xslDbgShellPrintStylesheetsHelper2:
 * @payload :valid xmlNodePtr of included stylesheet
 * @data :  not used
 * name : not used
 *
 * Print out the stylesheet name from the stylesheet given to
 *   us via walkIncludes
 */
void
  xslDbgShellPrintStylesheetsHelper2(void *payload,
                                     void *data,
                                     xmlChar * name);


/**
 * printTemplateHelper:
 * @templ: Is valid
 * @verbose: Either 1 or 0
 * @templateCount: Is valid
 * @count: Is valid
 * @templateName: template name to print and in UTF-8, may be NULL
 *
 * This display the templates in the same order as they are in the 
 *   stylesheet. If verbose is 1 then print more information
 *   For each template found @templateCount is increased
 *   For each printed template @printCount is increased
 */
void printTemplateHelper(xsltTemplatePtr templ, int verbose,
                         int *templateCount, int *count,
                         xmlChar * templateName);

/* ------------------------------------- 
    End private functions
---------------------------------------*/



/**
 * printTemplateHelper:
 * @templ: Is valid
 * @verbose: Either 1 or 0
 * @templateCount: Is valid
 * @count: Is valid
 * @templateName: template name to print, may be NULL
 *
 * This display the templates in the same order as they are in the 
 *   stylesheet. If verbose is 1 then print more information
 *   For each template found @templateCount is increased
 *   For each printed template @printCount is increased
 */
void
printTemplateHelper(xsltTemplatePtr templ, int verbose,
                    int *templateCount, int *count, xmlChar * templateName)
{
  xmlChar *name, *defaultUrl = (xmlChar *) "<n/a>";
    const xmlChar *url;

      if (!templ) 
        return;

      *templateCount = *templateCount + 1;
      printTemplateHelper(templ->next, verbose,
			  templateCount, count, templateName);
      if (templ->elem && templ->elem->doc && templ->elem->doc->URL) {
	url = templ->elem->doc->URL;
      } else {
	url = defaultUrl;
      }

      if (templ->match)
	name = xmlStrdup(templ->match);
      else
	name = fullQName(templ->nameURI, templ->name);

      if (name) {
	if (templateName &&
	    (xmlStrcmp(templateName, name) != 0)) {
	  /*  search for template name supplied failed */
	  /* empty */
	} else {
	  xmlChar *modeTemp = NULL;
	  *count = *count + 1;
	  if (getThreadStatus() == XSLDBG_MSG_THREAD_RUN) {
	    notifyListQueue(templ);
	  } else {
	    modeTemp = fullQName(templ->modeURI, templ->mode);
	    if (verbose)
	      xsldbgGenericErrorFunc(i18n(" template: \"%1\" mode: \"%2\" in file \"%3\" at line %4\n").arg(xsldbgText(name)).arg(xsldbgText(modeTemp)).arg(xsldbgUrl(url)).arg(xmlGetLineNo(templ->elem)));
	    else
	       xsldbgGenericErrorFunc(QString("\"%s\" ").arg(xsldbgText(name)));
	    if (modeTemp)
	      xmlFree(modeTemp);
	  }
	}
	       
	xmlFree(name);
	  
      }
}


/** 
 * xslDbgShellPrintTemplateNames:
 * @styleCtxt: Is valid 
 * @ctxt: Not used
 * @arg: Not used
 * @verbose: If 1 then print extra messages about templates found,
 *            otherwise print normal messages only 
 * @allFiles: If 1 then look for all templates in stylsheets found in 
 *                 @styleCtxt
 *             otherwise look in the stylesheet found by 
 *                 debugXSLBreak function
 *
 * Print out the list of template names found that match critieria
 *
 * Returns 1 on success,
 *         0 otherwise
 */
int
xslDbgShellPrintTemplateNames(xsltTransformContextPtr styleCtxt,
                              xmlShellCtxtPtr ctxt,
                              xmlChar * arg, int verbose, int allFiles)
{
    Q_UNUSED(ctxt);
    int templateCount = 0, printedTemplateCount = 0;
    int result = 0;
    xsltStylesheetPtr curStyle;
    xsltTemplatePtr templ;

    if (xmlStrLen(arg) == 0) {
        arg = NULL;
    } else {
        allFiles = 1;           /* make sure we find it if we can */
    }

    if (!styleCtxt) {
        xsldbgGenericErrorFunc(i18n("Error: Stylesheet is not valid.\n"));
        return result;
    }

    if (allFiles)
        curStyle = styleCtxt->style;
    else {
        /* try to find files in the current stylesheet */
        /* root copy is set to the stylesheet found by debugXSLBreak */
        if (debugXSLGetTemplate())
            curStyle = debugXSLGetTemplate()->style;
        else
            curStyle = NULL;
    }

    if (getThreadStatus() == XSLDBG_MSG_THREAD_RUN) {
        notifyListStart(XSLDBG_MSG_TEMPLATE_CHANGED);
        while (curStyle) {
            templ = curStyle->templates;
            /* print them out in the order their in the file */
            printTemplateHelper(templ, verbose, &templateCount,
                                &printedTemplateCount, arg);
            if (curStyle->next)
                curStyle = curStyle->next;
            else
                curStyle = curStyle->imports;
        }
        notifyListSend();
    } else {
        xsltGenericError(xsltGenericErrorContext, "\n");
        while (curStyle) {
            templ = curStyle->templates;
            /* print them out in the order their in the file */
            printTemplateHelper(templ, verbose, &templateCount,
                                &printedTemplateCount, arg);
            xsltGenericError(xsltGenericErrorContext, "\n");
            if (curStyle->next)
                curStyle = curStyle->next;
            else
                curStyle = curStyle->imports;
        }
        if (templateCount == 0) {
            xsldbgGenericErrorFunc(i18n("\tNo XSLT templates found.\n"));
        } else {
	    xsldbgGenericErrorFunc(i18n("\tTotal of %n XSLT template found", "\tTotal of %n XSLT templates found", templateCount) + QString("\n"));
	    xsldbgGenericErrorFunc(i18n("\tTotal of %n XSLT template printed", "\tTotal of %n XSLT templates printed", printedTemplateCount) + QString("\n"));
        }
    }

    result = 1;
    return result;
}


/**
 * xslDbgShellPrintStylesheetsHelper:
 * @payload :valid xsltStylesheetPtr
 * @data :  not used
 * name : not used
 *
 * Print out the stylesheet name from the stylesheet given to
 *   us via walkStylesheets
 */
void
xslDbgShellPrintStylesheetsHelper(void *payload,
                                  void *data,
                                  xmlChar * name)
{
    Q_UNUSED(data);
    Q_UNUSED(name);
    xsltStylesheetPtr style = (xsltStylesheetPtr) payload;

    if (style && style->doc && style->doc->URL) {
        if (getThreadStatus() == XSLDBG_MSG_THREAD_RUN)
            notifyListQueue(payload);
        else
	    /* display the URL of stylesheet  */
	    xsldbgGenericErrorFunc(i18n(" Stylesheet %1\n").arg(xsldbgUrl(style->doc->URL)));
        printCounter++;
    }
}


/**
 * xslDbgShellPrintStylesheetsHelper2:
 * @payload :valid xmlNodePtr of included stylesheet
 * @data :  not used
 * name : not used
 *
 * Print out the stylesheet name from the stylesheet given to
 *   us via walkIncludes
 */
void
xslDbgShellPrintStylesheetsHelper2(void *payload,
                                   void *data,
                                   xmlChar * name)
{
    Q_UNUSED(data);
    Q_UNUSED(name);
    xmlNodePtr node = (xmlNodePtr) payload;

    if (node && node->doc && node->doc->URL) {
        if (getThreadStatus() == XSLDBG_MSG_THREAD_RUN)
            notifyListQueue(payload);
        else
	    /* display the URL of stylesheet  */
	     xsldbgGenericErrorFunc(i18n(" Stylesheet %1\n").arg(xsldbgUrl(node->doc->URL)));
        printCounter++;
    }
}


/**
 * xslDbgShellPrintStyleSheets:
 * @arg: The stylesheets of interests and in UTF-8, is NULL for all stylsheets
 *
 * Print stylesheets that can be found in loaded stylsheet
 *
 * Returns 1 on success,
 *         0 otherwise
 */
int
xslDbgShellPrintStyleSheets(xmlChar * arg)
{
    Q_UNUSED(arg);
    printCounter = 0;
    if (getThreadStatus() == XSLDBG_MSG_THREAD_RUN) {
        notifyListStart(XSLDBG_MSG_SOURCE_CHANGED);
        walkStylesheets((xmlHashScanner) xslDbgShellPrintStylesheetsHelper,
                        NULL, filesGetStylesheet());
        notifyListSend();
        notifyListStart(XSLDBG_MSG_INCLUDED_SOURCE_CHANGED);
        walkIncludes((xmlHashScanner) xslDbgShellPrintStylesheetsHelper2,
                     NULL, filesGetStylesheet());
        notifyListSend();
    } else {
        walkStylesheets((xmlHashScanner) xslDbgShellPrintStylesheetsHelper,
                        NULL, filesGetStylesheet());
        walkIncludes((xmlHashScanner) xslDbgShellPrintStylesheetsHelper2,
                     NULL, filesGetStylesheet());
        if (printCounter != 0)
	    xsldbgGenericErrorFunc(i18n("\tTotal of %n XSLT stylesheet found.", "\tTotal of %n XSLT stylesheets found.", printCounter) + QString("\n"));
        else
            /* strange but possible */
            xsldbgGenericErrorFunc(i18n("\tNo XSLT stylesheets found.\n"));
    }
    return 1;                   /* always succeed */
}
