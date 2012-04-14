
/***************************************************************************
                          xsldbg.c  -  description
                             -------------------
    begin                : Sun Sep 16 2001
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
 * Based on file xsltproc.c
 *
 * by  Daniel Veillard 
 *     daniel@veillard.com
 *
 *  xsltproc.c is part of libxslt
 *
 *
 */

/* Justin's port version - do not merge into core!!! */
#define RISCOS_PORT_VERSION "2.01"

#include <kurl.h>
#include <kdebug.h>
#include "xsldbg.h"
#include "debug.h"
#include "options.h"
#include "utils.h"
#include "files.h"
#include "breakpoint.h"
#include "debugXSL.h"

#include <libxml/xmlerror.h>
#include "xsldbgmsg.h"
#include "xsldbgthread.h"       /* for getThreadStatus */
#ifdef HAVE_READLINE
#  include <readline/readline.h>
#  ifdef HAVE_HISTORY
#     include <readline/history.h>
#   endif
#endif

/* need to setup catch of SIGINT */
#include <signal.h>

/* needed by printTemplateNames */
#include <libxslt/transform.h>

/* standard includes from xsltproc*/
#include <libxslt/xslt.h>
#include <libexslt/exslt.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif
#include <libxml/xmlmemory.h>
#include <libxml/debugXML.h>
#include <libxml/xmlerror.h>
#include <libxml/HTMLtree.h>
#include <libxml/xmlIO.h>
#ifdef LIBXML_DOCB_ENABLED
#include <libxml/DOCBparser.h>
#endif
#ifdef LIBXML_XINCLUDE_ENABLED
#include <libxml/xinclude.h>
#endif

#include <libxml/catalog.h>
#include <libxml/parserInternals.h>

#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libxslt/extensions.h>
#include <libexslt/exsltconfig.h>

#include <kcmdlineargs.h> 
#include <kglobal.h>
#include <qfile.h>

#ifdef WIN32
#ifdef _MSC_VER
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#define gettimeofday(p1,p2)
#define HAVE_TIME_H
#include <time.h>
#define HAVE_STDARG_H
#include <stdarg.h>
#endif /* _MS_VER */
#else /* WIN32 */
#if defined(HAVE_SYS_TIME_H)
#include <sys/time.h>
#elif defined(HAVE_TIME_H)
#include <time.h>
#endif


#endif /* WIN32 */

#ifndef HAVE_STAT
#  ifdef HAVE__STAT

/* MS C library seems to define stat and _stat. The definition
      *         is identical. Still, mapping them to each other causes a warning. */
#    ifndef _MSC_VER
#      define stat(x,y) _stat(x,y)
#    endif
#    define HAVE_STAT
#  endif
#endif

#ifdef __riscos

/* Global definition of our program name on invocation.
   This is required such that we can invoke ourselves for processing
   search or help messages where our executable does not exist on the
   current run path */
char *xsldbgCommand = NULL;
#endif

xmlSAXHandler mySAXhdlr;

FILE *errorFile = NULL; /* we'll set this just before starting debugger */

xmlParserInputPtr xmlNoNetExternalEntityLoader(const char *URL,
                                               const char *ID,
                                               xmlParserCtxtPtr ctxt);

/* -----------------------------------------
   Private function declarations for xsldbg.c
 -------------------------------------------*/

/**
 * xsldbgInit:
 * 
 * Returns 1 if able to allocate memory needed by xsldbg
 *         0 otherwise
 */
int xsldbgInit(void);


/**
 * xsldbgFree:
 *
 * Free memory used by xsldbg
 */
void xsldbgFree(void);


/**
 * printTemplates:
 * @style : valid as parsed my xsldbg
 * @doc :    "    "   "     "    "
 *  
 * print out list of template names
 */
void printTemplates(xsltStylesheetPtr style, xmlDocPtr doc);


/**
 * catchSigInt:
 * @value : not used
 *
 * Recover from a signal(SIGINT), exit if needed
 */
void catchSigInt(int value);


/**
 * catchSigTerm:
 * @value : not used
 *
 * Clean up and exit
 */
void
  catchSigTerm(int value);

/**
 * xsldbgGenericErrorFunc:
 * @ctx:  Is Valid
 * @msg:  Is valid
 * @...:  other parameters to use
 * 
 * Handles print output from xsldbg and passes it to the application if 
 *  running as a thread otherwise send to stderr
 */
void
  xsldbgGenericErrorFunc(void *ctx, const char *msg, ...);

xmlEntityPtr (*oldGetEntity)( void * user_data, const xmlChar * name);

static xmlEntityPtr xsldbgGetEntity( void * user_data, const xmlChar * name)
{
    xmlEntityPtr ent = NULL;
    if (oldGetEntity){
	ent =  (oldGetEntity)(user_data, name);
	if (ent)
	    filesEntityRef(ent, ent->children, ent->last);
    }
    return ent;
}

/* ------------------------------------- 
   End private functions
   ---------------------------------------*/


/*
 * Internal timing routines to remove the necessity to have unix-specific
 * function calls
 */

#if defined(HAVE_GETTIMEOFDAY)
static struct timeval begin, end;

/*
 * startTimer: call where you want to start timing
 */
static void
startTimer(void)
{
    gettimeofday(&begin, NULL);
}

/*
 * endTimer: call where you want to stop timing and to print out a
 *           message about the timing performed; format is a printf
 *           type argument
 */
static void
endTimer(const QString& message)
{
    long msec;

    gettimeofday(&end, NULL);
    msec = end.tv_sec - begin.tv_sec;
    msec *= 1000;
    msec += (end.tv_usec - begin.tv_usec) / 1000;

#ifndef HAVE_STDARG_H
#error "endTimer required stdarg functions"
#endif
    /* Display the time taken to complete this task */
    xsldbgGenericErrorFunc(i18n("%1 took %2 ms to complete.\n").arg(message).arg(msec));
}
#elif defined(HAVE_TIME_H)

/*
 * No gettimeofday function, so we have to make do with calling clock.
 * This is obviously less accurate, but there's little we can do about
 * that.
 */

clock_t begin, end;
static void
startTimer(void)
{
    begin = clock();
}
static void
endTimer(const QString& message)
{
    long msec;

    end = clock();
    msec = ((end - begin) * 1000) / CLOCKS_PER_SEC;

#ifndef HAVE_STDARG_H
#error "endTimer required stdarg functions"
#endif
    /* Display the time taken to complete this task */
    xsldbgGenericErrorFunc(i18n("%1 took %2 ms to complete.\n").arg(message).arg(msec));
}
#else

/*
 * We don't have a gettimeofday or time.h, so we just don't do timing
 */
static void
startTimer(void)
{
    /*
     * Do nothing
     */
}
static void
endTimer(const QString& message)
{
    /*
     * We can not do anything because we don't have a timing function
     */
#ifdef HAVE_STDARG_H
    /* Display the time taken to complete this task */
    xsldbgGenericErrorFunc(i18n("%1 took %2 ms to complete.\n"),arg(message).arg(msec));
#else
    /* We don't have gettimeofday, time or stdarg.h, what crazy world is
     * this ?!
     */
#endif
}
#endif

static void
xsltProcess(xmlDocPtr doc, xsltStylesheetPtr cur)
{

    xmlDocPtr res;
    const char *params[8 * 2 + 2];
    int bytesWritten = -1;
    int nbparams = 0;
    int paramIndex;
    parameterItemPtr paramItem;

    /* Copy the parameters accross for libxslt */
    for (paramIndex = 0;
         paramIndex < arrayListCount(optionsGetParamItemList());
         paramIndex++) {
        paramItem = (parameterItemPtr)arrayListGet(optionsGetParamItemList(), paramIndex);
        if (paramItem) {
            params[nbparams] = (char *) paramItem->name;
            params[nbparams + 1] = (char *) paramItem->value;
            nbparams += 2;
        }
    }

    params[nbparams] = NULL;

#ifdef LIBXML_XINCLUDE_ENABLED
    if (optionsGetIntOption(OPTIONS_XINCLUDE)) {
        if (optionsGetIntOption(OPTIONS_TIMING))
            startTimer();
        xmlXIncludeProcess(doc);
        if (optionsGetIntOption(OPTIONS_TIMING)) {
	    /* Display the time taken to do XInclude processing */
            endTimer(i18n("XInclude processing %1.").arg((const char*)optionsGetStringOption(OPTIONS_DATA_FILE_NAME)));
        }
    }
#endif
    if (optionsGetIntOption(OPTIONS_TIMING) ||
        optionsGetIntOption(OPTIONS_PROFILING))
        startTimer();
    if ((optionsGetStringOption(OPTIONS_OUTPUT_FILE_NAME) == NULL) ||
        optionsGetIntOption(OPTIONS_SHELL)) {
        if (optionsGetIntOption(OPTIONS_REPEAT)) {
            int j;

            for (j = 1; j < optionsGetIntOption(OPTIONS_REPEAT); j++) {
                res = xsltApplyStylesheet(cur, doc, params);
                xmlFreeDoc(res);
                doc = xsldbgLoadXmlData();
            }
        }
        if (optionsGetIntOption(OPTIONS_PROFILING)) {
            if (terminalIO != NULL)
                res = xsltProfileStylesheet(cur, doc, params, terminalIO);
            else if ((optionsGetStringOption(OPTIONS_OUTPUT_FILE_NAME) ==
                      NULL) || (getThreadStatus() != XSLDBG_MSG_THREAD_RUN)
                     || (filesTempFileName(1) == NULL))
                res = xsltProfileStylesheet(cur, doc, params, stderr);
            else {
                /* We now have to output to using notify using 
                 * temp file #1 */
                FILE *tempFile = fopen(filesTempFileName(1), "w");

                if (tempFile != NULL) {
                    res =
                        xsltProfileStylesheet(cur, doc, params, tempFile);
                    fclose(tempFile);
                    /* send the data to application */
                    notifyXsldbgApp(XSLDBG_MSG_FILEOUT,
                                    filesTempFileName(1));
                } else {
		     xsldbgGenericErrorFunc(i18n("Error: Unable to write temporary results to %1.\n").arg(filesTempFileName(1)));
                    res = xsltProfileStylesheet(cur, doc, params, stderr);
                }
            }
        } else {
            res = xsltApplyStylesheet(cur, doc, params);
        }
        if (optionsGetIntOption(OPTIONS_PROFILING)) {
            if (optionsGetIntOption(OPTIONS_REPEAT))
		/* Display how long it took to apply stylesheet */
                endTimer(i18n("Applying stylesheet %n time", "Applying stylesheet %n times", optionsGetIntOption(OPTIONS_REPEAT)));
            else
		/* Display how long it took to apply stylesheet */
                endTimer(i18n("Applying stylesheet"));
        }
        if (res == NULL) {
#ifdef WITH_XSLDBG_DEBUG_PROCESS
            xsltGenericError(xsltGenericErrorContext, "Error: Transformation did not complete writing to file %s\n",
                             optionsGetStringOption
                             (OPTIONS_OUTPUT_FILE_NAME));
#endif
            return;
        }
        if (!optionsGetIntOption(OPTIONS_OUT)) {
            xmlFreeDoc(res);
            return;
        }
#ifdef LIBXML_DEBUG_ENABLED
        if (optionsGetIntOption(OPTIONS_DEBUG)) {
	    if (xslDebugStatus != DEBUG_RUN_RESTART){
		if (terminalIO != NULL)
		    xmlDebugDumpDocument(terminalIO, res);
		else if ((optionsGetStringOption(OPTIONS_OUTPUT_FILE_NAME) ==
			    NULL) || (getThreadStatus() != XSLDBG_MSG_THREAD_RUN)
			|| (filesTempFileName(1) == NULL))
		    xmlDebugDumpDocument(stdout, res);
		else {
		    FILE *tempFile = fopen(filesTempFileName(1), "w");

		    if (tempFile) {
			bytesWritten = 0; // flag that we have writen at least zero bytes
			xmlDebugDumpDocument(tempFile, res);
			fclose(tempFile);
			/* send the data to application */
			notifyXsldbgApp(XSLDBG_MSG_FILEOUT,
				filesTempFileName(1));
		    } else {
			 xsldbgGenericErrorFunc(i18n("Error: Unable to write temporary results to %1.\n").arg(filesTempFileName(1)));
			xmlDebugDumpDocument(stdout, res);
		    }

		}
	    }
        } else {
#endif
	    if (xslDebugStatus != DEBUG_RUN_RESTART){
		if (cur->methodURI == NULL) {
		    if (optionsGetIntOption(OPTIONS_TIMING))
			startTimer();
		    if (xslDebugStatus != DEBUG_QUIT) {
			if (terminalIO != NULL)
			    bytesWritten = xsltSaveResultToFile(terminalIO, res, cur);
			else if (optionsGetStringOption
				(OPTIONS_OUTPUT_FILE_NAME) == NULL)
			    bytesWritten = xsltSaveResultToFile(stdout, res, cur);
			else{
			    bytesWritten = xsltSaveResultToFilename((const char *)
				    optionsGetStringOption
				    (OPTIONS_OUTPUT_FILE_NAME),
				    res, cur, 0);
		    }
		    }
		    if (optionsGetIntOption(OPTIONS_TIMING))
			/* Indicate how long it took to save to file */
			endTimer(i18n("Saving result"));
		} else {
		    if (xmlStrEqual(cur->method, (const xmlChar *) "xhtml")) {
			xsldbgGenericErrorFunc(i18n("Warning: Generating non-standard output XHTML.\n"));
			if (optionsGetIntOption(OPTIONS_TIMING))
			    startTimer();
			if (terminalIO != NULL)
			    bytesWritten = xsltSaveResultToFile(terminalIO, res, cur);
			else if (optionsGetStringOption
				(OPTIONS_OUTPUT_FILE_NAME) == NULL)
			    bytesWritten = xsltSaveResultToFile(stdout, res, cur);
			else
			    bytesWritten = xsltSaveResultToFilename((const char *)
				    optionsGetStringOption
				    (OPTIONS_OUTPUT_FILE_NAME),
				    res, cur, 0);
			if (optionsGetIntOption(OPTIONS_TIMING))
			    /* Indicate how long it took to save to file */
			    endTimer(i18n("Saving result"));
		    } else {
			xsldbgGenericErrorFunc(i18n("Warning: Unsupported, non-standard output method %1.\n").arg(xsldbgText(cur->method)));
		    }
		}
	    }
#ifdef LIBXML_DEBUG_ENABLED
        }
#endif

        xmlFreeDoc(res);
    } else {
        xsltTransformContextPtr userCtxt = xsltNewTransformContext(cur, doc);
        if (userCtxt){
        bytesWritten = xsltRunStylesheetUser(cur, doc, params,
                          (char *)optionsGetStringOption(OPTIONS_OUTPUT_FILE_NAME),
                          NULL, NULL, NULL, userCtxt);
        if (optionsGetIntOption(OPTIONS_TIMING))
            endTimer(i18n("Running stylesheet and saving result"));
        xsltFreeTransformContext(userCtxt);
        }else{
	    xsldbgGenericErrorFunc(i18n("Error: Out of memory.\n"));
        }
    }
    if ((xslDebugStatus != DEBUG_RUN_RESTART) && (bytesWritten == -1))
	xsldbgGenericErrorFunc(i18n("Error: Unable to save results of transformation to file %1.\n").arg(xsldbgText(optionsGetStringOption(OPTIONS_OUTPUT_FILE_NAME)))); 
}

int
xsldbgMain(int argc, char **argv)
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);
    int i=0, result = 1, noFilesFound = 0;
    xsltStylesheetPtr cur = NULL;
    xmlChar *expandedName;      /* contains file name with path expansion if any */

    /* in some cases we always want to bring up a command prompt */
    int showPrompt;

    /* the xml document we're processing */
    xmlDocPtr doc;
    
    KCmdLineArgs *args = 0;
    if (getThreadStatus() == XSLDBG_MSG_THREAD_NOTUSED)
	args = KCmdLineArgs::parsedArgs();

    errorFile = stderr;
    

    if (args){
	QString langChoice = args->getOption("lang");
	if (KGlobal::locale() && !langChoice.isEmpty() && result)
	    KGlobal::locale()->setLanguage(langChoice);
    }

#ifdef __riscos
    /* Remember our invocation command such that we may call ourselves */
    xsldbgCommand = argv[0];
#endif

    xmlInitMemory();


    LIBXML_TEST_VERSION xmlLineNumbersDefault(1);

    if (!xsldbgInit()) {
	   xsldbgGenericErrorFunc(i18n("Fatal error: Aborting debugger due to an unrecoverable error.\n"));
        xsldbgFree();
        xsltCleanupGlobals();
        xmlCleanupParser();
        xmlMemoryDump();
        return (1);
    }


    if (args){
	for (i = 0; i < args->count(); i++) {
	    if (!result)
		break;

	    if (args->arg(i)[0] != '-') {
		expandedName = filesExpandName((const xmlChar*)args->arg(i));
		if (!expandedName) {
		    result = 0;
		    break;
		}
		switch (noFilesFound) {
		    case 0:
			optionsSetStringOption(OPTIONS_SOURCE_FILE_NAME,
				expandedName);
			noFilesFound++;
			break;
		    case 1:
			optionsSetStringOption(OPTIONS_DATA_FILE_NAME,
				expandedName);
			noFilesFound++;
			break;

		    default:
			xsldbgGenericErrorFunc(i18n("Error: Too many file names supplied via command line.\n"));
			result = 0;
		}
		xmlFree(expandedName);
		continue;
	    }
	}  	


	 // Handle boolean options
	for (int optionID = OPTIONS_FIRST_BOOL_OPTIONID; optionID < OPTIONS_LAST_BOOL_OPTIONID; optionID++){
	    if (optionsGetOptionName(OptionTypeEnum(optionID))){
		if (args->isSet((char *)optionsGetOptionName(OptionTypeEnum(optionID))))
		    optionsSetIntOption(OptionTypeEnum
			    (optionID), 1);
		else
		    optionsSetIntOption(OptionTypeEnum(optionID), 0);
	    }
	}

	// No extra arguments go straight to the shell? 
	if (args->count() == 0)
	    result = optionsSetIntOption(OPTIONS_SHELL, 1);

	// Disable net access? 
	if (!args->isSet("net")){
	    char noNetCmd[] = {"nonet 1"}; 
	    xslDbgShellSetOption((xmlChar*)noNetCmd);
	}
    }
   
    if (getThreadStatus() != XSLDBG_MSG_THREAD_NOTUSED){
	result = optionsSetIntOption(OPTIONS_SHELL, 1);
    }
    /* copy the volitile options over to xsldbg */
    optionsCopyVolitleOptions();
    
    /*
     * shell interraction
     */
    if (!optionsGetIntOption(OPTIONS_SHELL)) {  /* excecute stylesheet (ie no debugging) */
        xslDebugStatus = DEBUG_NONE;
    } else {
        xslDebugStatus = DEBUG_STOP;
        xsltGenericError(xsltGenericErrorContext, "XSLDBG %s\n", VERSION);
    }

    if (optionsGetIntOption(OPTIONS_VALID))
	xmlLoadExtDtdDefaultValue = XML_DETECT_IDS | XML_COMPLETE_ATTRS;
    else
	xmlLoadExtDtdDefaultValue = 0;


    if (args){
	if (args->isSet("verbose") && result)
	    xsltSetGenericDebugFunc(stderr, NULL);

	QCString outFile = args->getOption("output");
	if (!outFile.isEmpty() && result)
	    result = xslDbgShellOutput((const xmlChar *)QFile::encodeName(outFile).data());

	QCString maxDepth = args->getOption("maxdepth");
	if (!maxDepth.isEmpty() && result){
	    bool OK;
	    int value = maxDepth.toInt(&OK);
	    if (OK && (value> 0))
		xsltMaxDepth = value;
	}

	if (args->isSet("repeat") && result){
	    if (optionsGetIntOption(OPTIONS_REPEAT) == 0)
		optionsSetIntOption(OPTIONS_REPEAT, 20);
	    else
		optionsSetIntOption(OPTIONS_REPEAT, 100);
	}

	QCStringList xslParams(args->getOptionList("param"));
	QCStringList::iterator it;
	QCString param, paramName, paramValue;
	int separatorIdx;
	bool paramOK;
	for ( it = xslParams.begin(); it != xslParams.end(); ++it){
	    param = (*it);
	    paramOK = true;
	    separatorIdx = param.find(':');
	    if (separatorIdx > 0){
		paramName = param.left((uint)separatorIdx);
		paramValue = param.mid((uint)separatorIdx + 1);
		if (!paramName.isEmpty() && !paramValue.isEmpty()){
		    if (arrayListCount(optionsGetParamItemList()) <= 31) {
			arrayListAdd(optionsGetParamItemList(), optionsParamItemNew((const xmlChar*)paramName.data(), (const xmlChar*)paramValue.data()));
		    }else{
			xsldbgGenericErrorFunc(i18n("Warning: Too many libxslt parameters provided via the command line option --param.\n"));
		    }
		}else{
		    paramOK = false;
		}
	    }else{
		paramOK = false;
	    }
	    if (!paramOK)
		xsldbgGenericErrorFunc(i18n("Error: Argument \"%1\" to --param is not in the format <name>:<value>.\n").arg(param.data()));
	}

	QCString cdPath = args->getOption("cd");
	if (!cdPath.isEmpty() && result)
	    result = changeDir((const xmlChar *)QFile::encodeName(cdPath).data());

    }

    if (!result) {
	KCmdLineArgs::usage(); 
        xsldbgFree();
        return (1);
    }


    /*
     * * Replace entities with their content.
     */
    xmlSubstituteEntitiesDefault(1);

    /*
     * * Register the EXSLT extensions and the test module
     */
    exsltRegisterAll();
    xsltRegisterTestModule();

    

    debugGotControl(0);
    while (xslDebugStatus != DEBUG_QUIT) {
	xsldbgReachedFirstTemplate = false;
	/* don't force xsldbg to show command prompt */
	showPrompt = 0;
	cur = NULL;
	doc = NULL;
	arrayListEmpty(filesEntityList());
	xsltSetXIncludeDefault(optionsGetIntOption(OPTIONS_XINCLUDE));

	/* copy the volitile options over to xsldbg */
	optionsCopyVolitleOptions();

	/* choose where error messages/xsldbg output get sent to */
	if (optionsGetIntOption(OPTIONS_STDOUT))
	    errorFile = stdout;
	else
	    errorFile = stderr;

	filesLoadCatalogs();

	if (optionsGetIntOption(OPTIONS_SHELL)) {
	    debugGotControl(0);
	    xsldbgGenericErrorFunc(i18n("\nStarting stylesheet\n\n"));
	    if (optionsGetIntOption(OPTIONS_TRACE) == TRACE_OFF)
		xslDebugStatus = DEBUG_STOP;    /* stop as soon as possible */
	}

	if ((optionsGetStringOption(OPTIONS_SOURCE_FILE_NAME) == NULL) ||
		(optionsGetStringOption(OPTIONS_DATA_FILE_NAME) == NULL)) {
	    /* at least on file name has not been set */
	    /*goto a xsldbg command prompt */
	    showPrompt = 1;
	    if (optionsGetStringOption(OPTIONS_SOURCE_FILE_NAME) == NULL)
		xsldbgGenericErrorFunc(i18n("Error: No XSLT source file supplied.\n"));

	    if (optionsGetStringOption(OPTIONS_DATA_FILE_NAME) == NULL) {
		xsldbgGenericErrorFunc(i18n("Error: No XML data file supplied.\n"));
	    }

	} else {
	    filesLoadXmlFile(NULL, FILES_SOURCEFILE_TYPE);
	    cur = filesGetStylesheet();
	    if ((cur == NULL) || (cur->errors != 0)) {
		/*goto a xsldbg command prompt */
		showPrompt = 1;
		if (xslDebugStatus == DEBUG_NONE) {
		    xslDebugStatus = DEBUG_QUIT;        /* panic !! */
		    result = 0;
		}
	    }
	}

	if (showPrompt == 0) {
	    filesLoadXmlFile(NULL, FILES_XMLFILE_TYPE);
	    doc = filesGetMainDoc();
	    if (doc == NULL) {
		if (xslDebugStatus == DEBUG_NONE) {
		    xslDebugStatus = DEBUG_QUIT;        /* panic !! */
		    result = 0;
		} else {
		    /*goto a xsldbg command prompt */
		    showPrompt = 1;
		}
	    } else {
		if (xslDebugStatus != DEBUG_QUIT) {
		    xsltProcess(doc, cur);
		    result = 1;
		}
	    }

	    if (optionsGetIntOption(OPTIONS_SHELL) && (showPrompt == 0)) {
		if ((xslDebugStatus != DEBUG_QUIT)
			&& !debugGotControl(-1)) {
		    xsldbgGenericErrorFunc(i18n("\nDebugger never received control.\n"));
		    /*goto a xsldbg command prompt */
		    showPrompt = 1;
		    xslDebugStatus = DEBUG_STOP;
		} else {
		    xsldbgGenericErrorFunc(i18n("\nFinished stylesheet\n\032\032\n"));
		    {
			/* handle trace execution */
			int trace = optionsGetIntOption(OPTIONS_TRACE);

			switch (trace) {
			    case TRACE_OFF:
				/* no trace of execution */
				break;

			    case TRACE_ON:
				/* tell xsldbg to stop tracing next time we get here */
				optionsSetIntOption(OPTIONS_TRACE,
					TRACE_RUNNING);
				xslDebugStatus = DEBUG_TRACE;
				break;

			    case TRACE_RUNNING:
				/* turn off tracing */
				xslDebugStatus = DEBUG_CONT;
				optionsSetIntOption(OPTIONS_TRACE,
					TRACE_OFF);
				break;
			}
		    }
		    if (!optionsGetIntOption(OPTIONS_AUTORESTART) && (xslDebugStatus != DEBUG_RUN_RESTART)){ 
			/* pass control to user they won't be able to do much
			   other than add breakpoints, quit, run, continue */
			debugXSLBreak((xmlNodePtr) cur->doc, (xmlNodePtr) doc,
				NULL, NULL);
		    }
		}
	    } else {
		/* request to execute stylesheet only  so we're done */
		xslDebugStatus = DEBUG_QUIT;
	    }
	} else {
	    /* Some sort of problem loading source file has occured. Quit? */
	    if (xslDebugStatus == DEBUG_NONE) {
		xslDebugStatus = DEBUG_QUIT;    /* Panic!! */
		result = 0;
	    } else {
		/*goto a xsldbg command prompt */
		showPrompt = 1;
	    }
	}

	if (showPrompt && optionsGetIntOption(OPTIONS_SHELL)) {
	    xmlDocPtr tempDoc = xmlNewDoc((xmlChar *) "1.0");
	    xmlNodePtr tempNode =
		xmlNewNode(NULL, (xmlChar *) "xsldbg_default_node");
	    if (!tempDoc || !tempNode) {
		xsldbgFree();
		return (1);
	    }
	    xmlAddChild((xmlNodePtr) tempDoc, tempNode);

	    xsldbgGenericErrorFunc(i18n("Going to the command shell; not all xsldbg commands will work as not all needed have been loaded.\n"));
	    xslDebugStatus = DEBUG_STOP;
	    if ((cur == NULL) && (doc == NULL)) {
		/*no doc's loaded */
		debugXSLBreak(tempNode, tempNode, NULL, NULL);
	    } else if ((cur != NULL) && (doc == NULL)) {
		/* stylesheet is loaded */
		debugXSLBreak((xmlNodePtr) cur->doc, tempNode, NULL, NULL);
	    } else if ((cur == NULL) && (doc != NULL)) {
		/* xml doc is loaded */
		debugXSLBreak(tempNode, (xmlNodePtr) doc, NULL, NULL);
	    } else {
		/* unexpected problem, both docs are loaded */
		debugXSLBreak((xmlNodePtr) cur->doc, (xmlNodePtr) doc,
			NULL, NULL);
	    }
	    xmlFreeDoc(tempDoc);
	} else if (showPrompt && !optionsGetIntOption(OPTIONS_SHELL)) {
	    xslDebugStatus = DEBUG_QUIT;
	    result = 0;         /* panic */
	}

	if (optionsGetIntOption(OPTIONS_SHELL)) {
	    /* force a refesh of both stlesheet and xml data */
	    filesFreeXmlFile(FILES_SOURCEFILE_TYPE);
	    filesFreeXmlFile(FILES_XMLFILE_TYPE);
	}
    }

    if (!result) {
	xsldbgGenericErrorFunc(i18n("Fatal error: Aborting debugger due to an unrecoverable error.\n"));
    }
    xsldbgFree();
    xsltCleanupGlobals();
    xmlCleanupParser();
    xmlMemoryDump();
    return !result;
}

/**
 * xsldbgLoadStylesheet:
 *
 * Load the stylesheet and return it 
 *
 * Returns The stylesheet after reloading it if successful
 *         NULL otherwise
 */
xsltStylesheetPtr
xsldbgLoadStylesheet()
{
    xsltStylesheetPtr cur = NULL;
    xmlDocPtr style;

    if (optionsGetIntOption(OPTIONS_TIMING))
        startTimer();
#if LIBXML_VERSION >= 2006000
    style = xmlSAXParseFile(&mySAXhdlr, (const char *) optionsGetStringOption(OPTIONS_SOURCE_FILE_NAME), 0);
#else
    style = xmlParseFile((const char *) optionsGetStringOption(OPTIONS_SOURCE_FILE_NAME));
#endif
    if (optionsGetIntOption(OPTIONS_TIMING))
        endTimer(i18n("Parsing stylesheet %1").arg((const char*)optionsGetStringOption(OPTIONS_SOURCE_FILE_NAME)));
    if (style == NULL) {
        xsldbgGenericErrorFunc(i18n("Error: Cannot parse file %1.\n").arg(xsldbgUrl(optionsGetStringOption(OPTIONS_SOURCE_FILE_NAME))));
        cur = NULL;
        if (!optionsGetIntOption(OPTIONS_SHELL)) {
            xsldbgGenericErrorFunc(i18n("Fatal error: Aborting debugger due to an unrecoverable error.\n"));
            xslDebugStatus = DEBUG_QUIT;
        } else {
            xsltGenericError(xsltGenericErrorContext, "\n");
            xslDebugStatus = DEBUG_STOP;
        }
    } else {
        cur = xsltLoadStylesheetPI(style);
        if (cur != NULL) {
            /* it is an embedded stylesheet */
            xsltProcess(style, cur);
            xsltFreeStylesheet(cur);
        } else {
            cur = xsltParseStylesheetDoc(style);
            if (cur != NULL) {
                if (cur->indent == 1)
                    xmlIndentTreeOutput = 1;
                else
                    xmlIndentTreeOutput = 0;
            } else {
                xmlFreeDoc(style);
            }
        }
    }
    return cur;
}



/**
 * xsldbgLoadXmlData:
 *
 * Load the xml data file and return it  
 *
 * Returns The data file after reloading it if successful
 *         NULL otherwise
 */
xmlDocPtr
xsldbgLoadXmlData(void)
{
    xmlDocPtr doc = NULL;
    doc = NULL;

    if (optionsGetIntOption(OPTIONS_TIMING))
        startTimer();
#ifdef LIBXML_HTML_ENABLED
    if (optionsGetIntOption(OPTIONS_HTML))
        doc = htmlParseFile((char *)
                            optionsGetStringOption(OPTIONS_DATA_FILE_NAME),
                            NULL);
    else
#endif
#ifdef LIBXML_DOCB_ENABLED
    if (optionsGetIntOption(OPTIONS_DOCBOOK))
        doc = docbParseFile((char *)
                            optionsGetStringOption(OPTIONS_DATA_FILE_NAME),
                            NULL);
    else
#endif
#if LIBXML_VERSION >= 2006000
        doc = xmlSAXParseFile(&xmlDefaultSAXHandler,
			     (char *) optionsGetStringOption(OPTIONS_DATA_FILE_NAME), 0);
#else
        doc = xmlParseFile((char *) optionsGetStringOption(OPTIONS_DATA_FILE_NAME));
#endif
    if (doc == NULL) {
        xsldbgGenericErrorFunc(i18n("Error: Unable to parse file %1.\n").arg(xsldbgUrl(optionsGetStringOption(OPTIONS_DATA_FILE_NAME))));
        if (!optionsGetIntOption(OPTIONS_SHELL)) {
            xsldbgGenericErrorFunc(i18n("Fatal error: Aborting debugger due to an unrecoverable error.\n"));
            xslDebugStatus = DEBUG_QUIT;
        } else {
            xsltGenericError(xsltGenericErrorContext, "\n");
            xslDebugStatus = DEBUG_STOP;
        }
    } else if (optionsGetIntOption(OPTIONS_TIMING))
        endTimer(QString("Parsing document %1").arg(xsldbgUrl(optionsGetStringOption(OPTIONS_DATA_FILE_NAME))).utf8().data());

    return doc;
}


/**
 * xsldbgLoadXmlTemporary:
 * @path: The name of temporary file to load 
 *
 * Load the temporary data file and return it 
 *
 * Returns The temporary file after reloading it if successful,
 *         NULL otherwise
 */
xmlDocPtr
xsldbgLoadXmlTemporary(const xmlChar * path)
{
    xmlDocPtr doc = NULL;
    doc = NULL;

    if (optionsGetIntOption(OPTIONS_TIMING))
        startTimer();
#ifdef LIBXML_HTML_ENABLED
    if (optionsGetIntOption(OPTIONS_HTML))
        doc = htmlParseFile((char *) path, NULL);
    else
#endif
#ifdef LIBXML_DOCB_ENABLED
    if (optionsGetIntOption(OPTIONS_DOCBOOK))
        doc = docbParseFile((char *) path, NULL);
    else
#endif
        doc = xmlSAXParseFile(&mySAXhdlr, (char *) path, 0);
    if (doc == NULL) {
        xsldbgGenericErrorFunc(i18n("Error: Unable to parse file %1.\n").arg(xsldbgUrl(path)));
    }

    if (optionsGetIntOption(OPTIONS_TIMING)
        && (xslDebugStatus != DEBUG_QUIT)) {
        endTimer(QString("Parsing document %1").arg(xsldbgUrl(path)));
    }
    return doc;
}

/**
 * printTemplates:
 * @style : valid as parsed my xsldbg
 * @doc :    "    "   "     "    "
 *  
 * print out list of template names
 */
void
printTemplates(xsltStylesheetPtr style, xmlDocPtr doc)
{
    xsltTransformContextPtr ctxt = xsltNewTransformContext(style, doc);

    if (ctxt) {
        /* don't be verbose when printing out template names */
        xslDbgShellPrintTemplateNames(ctxt, NULL, NULL, 0, 0);
    } else {
        xsldbgGenericErrorFunc(i18n("Error: Out of memory.\n"));
    }
}

#ifdef WIN32

/* For the windows world we capture the control event */
BOOL WINAPI
handler_routine(DWORD dwCtrlType)
{

    switch (dwCtrlType) {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
            catchSigInt(SIGINT);
            break;

        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT:
            xsldbgFree();
            exit(1);
            break;

        default:
            printf("Error: Unknown control event\n");
            break;
    }

    return (true);
}

#endif

#if LIBXML_VERSION >= 2006000
/* libxml/ handlers */
void xsldbgStructErrorHandler(void * userData, xmlErrorPtr error)
{
    if (error && error->message && (error->level >= 0) && (error->level <= 4)){
	if (getThreadStatus() != XSLDBG_MSG_THREAD_RUN){
	    static const char *msgPrefix[4 + 1] = {"", "warning :", "error:", "fatal:"};
	    if (error->file)
		xsltGenericError(xsltGenericErrorContext, "%s%s in file \"%s\" line %d", msgPrefix[error->level], error->message, error->file, error->line);
	    else
		xsltGenericError(xsltGenericErrorContext, "%s%s", msgPrefix[error->level], error->message);

	}else{
	    xsltGenericError(xsltGenericErrorContext,"Struct error handler");
	    notifyXsldbgApp(XSLDBG_MSG_ERROR_MESSAGE, error);
	}
    }
}

void xsldbgSAXErrorHandler(void * ctx, const char * msg, ...)
{
    if (ctx)
	xsldbgStructErrorHandler(0, ((xmlParserCtxtPtr)ctx)->lastError);
}

void xsldbgSAXWarningHandler(void * ctx, const char * msg, ...)
{
    if (ctx)
	xsldbgStructErrorHandler(0, ((xmlParserCtxtPtr)ctx)->lastError);
}

#endif

/**
 * catchSigInt:
 * @value : not used
 *
 * Recover from a signal(SIGINT), exit if needed
 */
void
catchSigInt(int value)
{
    Q_UNUSED(value);
    if ((xslDebugStatus == DEBUG_NONE) || (xsldbgStop == 1) || (xslDebugStatus == DEBUG_STOP)) {
        xsldbgFree();
        exit(1);
    }
#ifdef __riscos
    /* re-catch SIGINT - RISC OS resets the handler when the interupt occurs */
    signal(SIGINT, catchSigInt);
#endif

    if (xslDebugStatus != DEBUG_STOP) {
        /* stop running/walking imediately !! */
        xsldbgStop = 1;
    }
}


/**
 * catchSigTerm:
 * @value : not used
 *
 * Clean up and exit
 */
void
catchSigTerm(int value)
{
    Q_UNUSED(value);
    xsldbgFree();
    exit(1);
}



typedef void (*sighandler_t) (int);
static sighandler_t oldHandler;

static int initialized = 0;

/**
 * xsldbgInit:
 * 
 * Returns 1 if able to allocate memory needed by xsldbg
 *         0 otherwise
 */
int
xsldbgInit()
{
    int result = 0;
    int xmlVer = 0;

    if (!initialized) {
        sscanf(xmlParserVersion, "%d", &xmlVer);
        xsldbgHasLineNumberFix = (xmlVer >= 20508);
        if (!debugInit()) {
#ifdef WITH_XSLDBG_DEBUG_PROCESS
            xsltGenericError(xsltGenericErrorContext,
                             "Fatal error: Init of debug module failed\n");
#endif
            return result;
        }
        if (!filesInit()) {
#ifdef WITH_XSLDBG_DEBUG_PROCESS
            xsltGenericError(xsltGenericErrorContext,
                             "Fatal error: Init of files module failed\n");
#endif
            return result;
        }

        if (!optionsInit()) {
#ifdef WITH_XSLDBG_DEBUG_PROCESS
            xsltGenericError(xsltGenericErrorContext,
                             "Fatal error: Init of options module failed\n");
#endif
            return result;
        }

        if (!searchInit()) {
#ifdef WITH_XSLDBG_DEBUG_PROCESS
            xsltGenericError(xsltGenericErrorContext,
                             "Fatal error: Init of search module failed\n");
#endif
            return result;
        }

	

        /* set up the parser */
        xmlInitParser();
#if 0
#if LIBXML_VERSION >= 2006000
     xmlSetGenericErrorFunc(NULL,  NULL);
     xmlSetStructuredErrorFunc(NULL , (xmlStructuredErrorFunc)xsldbgStructErrorHandler);
#else
        xmlSetGenericErrorFunc(0, xsldbgGenericErrorFunc);
        xsltSetGenericErrorFunc(0, xsldbgGenericErrorFunc);
#endif    
#else
        xmlSetGenericErrorFunc(0, xsldbgGenericErrorFunc);
        xsltSetGenericErrorFunc(0, xsldbgGenericErrorFunc);
#endif

	/*
	 * disable CDATA from being built in the document tree
	 */
	xmlDefaultSAXHandlerInit();
	xmlDefaultSAXHandler.cdataBlock = NULL;
if (xsldbgHasLineNumberFix){
	oldGetEntity = xmlDefaultSAXHandler.getEntity;
	xmlDefaultSAXHandler.getEntity = xsldbgGetEntity;
}

        if (getThreadStatus() != XSLDBG_MSG_THREAD_NOTUSED) {
            initialized = 1;
            return 1;           /* this is all we need to do when running as a thread */
        }
#ifndef WIN32
        /* catch SIGINT */
        oldHandler = signal(SIGINT, catchSigInt);
#else
        if (SetConsoleCtrlHandler(handler_routine, true) != true)
            return result;
#endif

#ifndef WIN32
        /* catch SIGTIN tty input available fro child */
        signal(SIGTERM, catchSigTerm);
#endif
        initialized = 1;
    }
    return 1;
}

/**
 * xsldbgFree:
 *
 * Free memory used by xsldbg
 */
void
xsldbgFree()
{
    debugFree();
    filesFree();
    optionsFree();
    searchFree();
#ifndef WIN32
    if (oldHandler != SIG_ERR)
        signal(SIGINT, oldHandler);
#else
    SetConsoleCtrlHandler(handler_routine, false);
#endif
    initialized = 0;

#ifdef HAVE_READLINE
    /*  rl_free_line_state ();
      rl_cleanup_after_signal(); */
#   ifdef HAVE_HISTORY
       clear_history();
#   endif    
#endif

}


char msgBuffer[4000];

/**
 * xsldbgGenericErrorFunc:
 * @ctx:  Is Valid
 * @msg:  Is valid
 * @...:  other parameters to use
 * 
 * Handles print output from xsldbg and passes it to the application if 
 *  running as a thread otherwise send to stderr
 */
void
xsldbgGenericErrorFunc(void *ctx, const char *msg, ...)
{
    va_list args;
    Q_UNUSED(ctx);

    va_start(args, msg);
    if (getThreadStatus() == XSLDBG_MSG_THREAD_RUN) {
        vsnprintf(msgBuffer, sizeof(msgBuffer), msg, args);

        notifyTextXsldbgApp(XSLDBG_MSG_TEXTOUT, msgBuffer);
    } else {
        xmlChar *encodeResult = NULL;

        vsnprintf(msgBuffer, sizeof(msgBuffer), msg, args);
        encodeResult = filesEncode((xmlChar *) msgBuffer);
        if (encodeResult) {
            fprintf(errorFile, "%s", encodeResult);
            xmlFree(encodeResult);
        } else
            fprintf(errorFile, "%s", msgBuffer);
    }
    va_end(args);
}

void xsldbgGenericErrorFunc(QString const &text)
{
    xsldbgGenericErrorFunc(0, "%s", text.utf8().data());
}


QString xsldbgUrl(const char *utf8fUrl)
{
    QString tempUrl(utf8fUrl);
    QString fixedURI;
    KURL url(tempUrl);

    // May be provided with a URL that only has been encoded and has no protocol prefix ie a local file
    if ( !tempUrl.startsWith("file:/")  &&  !tempUrl.startsWith("http:/") && !tempUrl.startsWith("ftp:/"))
	fixedURI = KURL::decode_string(tempUrl);
    else
	fixedURI = url.prettyURL();

    return fixedURI;
}

QString xsldbgUrl(const xmlChar *utf8Url)
{
    return xsldbgUrl((const char*)(utf8Url));
}

QString xsldbgText(const char *utf8Text)
{
    return QString::fromUtf8(utf8Text);
    
}

QString xsldbgText(const xmlChar *utf8Text)
{
    return QString::fromUtf8((const char *)utf8Text);
}

