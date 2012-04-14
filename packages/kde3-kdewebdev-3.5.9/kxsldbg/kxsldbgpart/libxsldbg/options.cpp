
/***************************************************************************
                          options.c  -  provide the implementation for option
                                           related functions
                             -------------------
    begin                : Sat Nov 10 2001
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
#include "xsldbgthread.h"
#include "options.h"
#include "arraylist.h"
#include "xsldbgmsg.h"
#include "utils.h"
#include <kglobal.h>
#include <kstandarddirs.h> 
#include <qfileinfo.h>


/* keep track of our integer/boolean options */
static int intOptions[OPTIONS_LAST_INT_OPTIONID - OPTIONS_FIRST_INT_OPTIONID + 1];

/* make use that use of options are safe by only copying
   critical values from intVolitleOptions just before stylesheet is started
 */
int intVolitileOptions[OPTIONS_LAST_INT_OPTIONID - OPTIONS_FIRST_INT_OPTIONID + 1];

/* keep track of our string options */
static xmlChar *stringOptions[OPTIONS_LAST_STRING_OPTIONID -
                              OPTIONS_FIRST_STRING_OPTIONID + 1];

/* keep track of our parameters */
static arrayListPtr parameterList;

/* what are the expressions to be printed out when xsldbg stops */
static arrayListPtr watchExpressionList;


/* the names for our options
   Items that start with *_ are options that CANNOT be used by the user
  Once you set an option you need to give a run command to activate
  new settings */
const char *optionNames[] = {
    "xinclude",                 /* Use xinclude during xml parsing */
    "docbook",                  /* Use of docbook sgml parsing */
    "timing",                   /* Use of timing */
    "profile",                  /* Use of profiling */
    "valid",                    /* Enable file validation */
    "out",                      /* Enable output to stdout */
    "html",                     /* Enable the use of html parsing */
    "debug",                    /* Enable the use of xml tree debugging */
    "shell",                    /* Enable the use of debugger shell */
    "gdb",                      /* Run in gdb modem prints more messages */
    "preferhtml",               /* Prefer html output for search results */
    "autoencode",               /* Try to use the encoding from the stylesheet */
    "utf8input",                /* All input from "user" will be in UTF-8 */
    "stdout",                   /* Print all error messages to  stdout, 
				 * normally error messages go to stderr */
    "autorestart",		/* When finishing the debug of a XSLT script 
				   automaticly restart at the beginning */
    "verbose",                  /* Be verbose with messages */
    "repeat",                   /* The number of times to repeat */
    "*_trace_*",                /* Trace execution */
    "*_walkspeed_*",            /* How fast do we walk through code */
    "catalogs",                 /* do we use catalogs in SGML_CATALOG_FILES */
    "output",                   /* what is the output file name */
    "source",                   /* The stylesheet source to use */
    "docspath",                 /* Path of xsldbg's documentation */
    "catalognames",             /* The names of the catalogs to use when the catalogs option is active */
    "encoding",                 /* What encoding to use for standard output */
    "searchresultspath",        /* Where do we store the results of search */
    "data",                     /* The xml data file to use */
    NULL                        /* indicate end of list */
};



// find the translated help documentation directory
// langLookupDir code modified from langLookup function in kdebase/khelpcenter/view.cpp 

static QString langLookupDir( const QString &fname )
{
    QStringList search;

    // assemble the local search paths
    const QStringList localDoc = KGlobal::dirs()->resourceDirs("html");

    // look up the different languages
    for (int id=localDoc.count()-1; id >= 0; --id)
    {
        QStringList langs = KGlobal::locale()->languageList();
        langs.append( "en" );
        langs.remove( "C" );
        QStringList::ConstIterator lang;
        for (lang = langs.begin(); lang != langs.end(); ++lang)
            search.append(QString("%1%2/%3/%4").arg(localDoc[id]).arg(*lang).arg("xsldbg").arg(fname));
    }

    // try to locate the file
    QStringList::Iterator it;
    for (it = search.begin(); it != search.end(); ++it)
    {
	QString baseDir = (*it).left((*it).findRev('/')) ;
	QFileInfo info(baseDir + "/"+ fname);
	if (info.exists() && info.isFile() && info.isReadable())
	  return baseDir;
    }

    return QString::null;
}


/** 
 * optionsInit:
 *
 * Intialize the options module
 *
 * Returns 1 on success,
 *         0 otherwise
 */
int
optionsInit(void)
{
    int optionId;

    for (optionId = 0;
         optionId <= OPTIONS_LAST_INT_OPTIONID - OPTIONS_FIRST_INT_OPTIONID; optionId++) {
        intOptions[optionId] = 0;
        intVolitileOptions[optionId] = 0;
    }

    for (optionId = 0;
         optionId <= OPTIONS_LAST_STRING_OPTIONID - OPTIONS_FIRST_STRING_OPTIONID;
         optionId++) {
        stringOptions[optionId] = NULL;
    }

    /* init our parameter list */
    parameterList = arrayListNew(10, (freeItemFunc) optionsParamItemFree);

    /* setup the docs path */
    optionsSetStringOption(OPTIONS_DOCS_PATH, (const xmlChar*)(langLookupDir("xsldbghelp.xml").utf8().data()));

    optionsSetIntOption(OPTIONS_TRACE, TRACE_OFF);
    optionsSetIntOption(OPTIONS_WALK_SPEED, WALKSPEED_STOP);
    /* always try to use encoding if found */
    optionsSetIntOption(OPTIONS_AUTOENCODE, 1);
    /* start up with auto restart turned off */
    optionsSetIntOption(OPTIONS_AUTORESTART, 0); 	
    /* start up with gdb mode turned on */
    optionsSetIntOption(OPTIONS_GDB, 1); 	
    /* start up with output mode turned on */
    optionsSetIntOption(OPTIONS_OUT, 1); 	
    /* start up with validation turned on */
    optionsSetIntOption(OPTIONS_VALID, 1); 	
    /* start up with xinclude turned on */
    optionsSetIntOption(OPTIONS_XINCLUDE, 1); 	

    /* set output default as standard output. Must be changed if not using
     * xsldbg's command line. Or the tty command is used */
    optionsSetStringOption(OPTIONS_OUTPUT_FILE_NAME, NULL);

    /* init our list of expressions to watch which are only a list of 
     strings ie xmlChar*'s  */
    watchExpressionList = arrayListNew(10, (freeItemFunc) xmlFree);

    return (parameterList && watchExpressionList);
}


/**
 * optionsFree:
 *
 * Free memory used by the options module
 */
void
optionsFree(void)
{
    int string_option;

    for (string_option = OPTIONS_FIRST_STRING_OPTIONID;
         string_option <= OPTIONS_LAST_STRING_OPTIONID; string_option++) {
        optionsSetStringOption(OptionTypeEnum(string_option), NULL);
    }

    /* Free up memory used by parameters and watches*/
    arrayListFree(parameterList);
    arrayListFree(watchExpressionList);
    parameterList = NULL;
    watchExpressionList = NULL;
}


  /**
   * optionsGetOptionID:
   * @optionName : A valid option name see documentation for "setoption" 
   *        command and program usage documentation
   *
   * Find the option id for a given option name
   *
   * Returns The optionID for @optionName if successful, where  
   *             OPTIONS_FIRST_OPTIONID<= optionID <= OPTIONS_LAST_OPTIONID,
   *         otherwise returns -1
   */
int
optionsGetOptionID(xmlChar * optionName)
{
    int result = -1;
    int optID = lookupName(optionName, (xmlChar **) optionNames);

    if (optID >= 0) {
        result = optID + OPTIONS_FIRST_OPTIONID;
    }

    return result;
}


  /**
   * optionsGetOptionName:
   * @ID : A valid option ID
   *
   * Get the name text for an option
   *
   * Returns The name of option if @ID is valid, 
   *         NULL otherwise 
   */
const xmlChar *
optionsGetOptionName(OptionTypeEnum ID)
{
    const xmlChar *result = 0;
    if ( (ID >= OPTIONS_FIRST_OPTIONID) && (ID <= OPTIONS_LAST_OPTIONID)){
	/* An option ID is always valid at the moment */
	result = (const xmlChar *) optionNames[ID - OPTIONS_FIRST_OPTIONID];
    }

    return result;
}


/**
 * optionsSetIntOption:
 * @optionType: A valid boolean option
 * @value: 1 to enable, 0 otherwise
 *
 * Set the state of a boolean xsldbg option to @value
 *
 * Returns 1 on success,
 *         0 otherwise
 */
int
optionsSetIntOption(OptionTypeEnum optionType, int value)
{
    int type = optionType, result = 1;

    if ((type >= OPTIONS_FIRST_INT_OPTIONID) && (type <= OPTIONS_LAST_INT_OPTIONID)) {
        /* make sure that use of options are safe by only copying
         * critical values from intVolitleOptions just before 
         * stylesheet is started
         */
        intVolitileOptions[type - OPTIONS_FIRST_INT_OPTIONID] = value;

        /* the following types must be activated imediately */
        switch (type) {

            case OPTIONS_TRACE:
            case OPTIONS_WALK_SPEED:
            case OPTIONS_GDB:
                intOptions[type - OPTIONS_FIRST_INT_OPTIONID] = value;
                break;

            default:
                break;
        }
    } else {
	if ((type >= OPTIONS_FIRST_OPTIONID) && (type <= OPTIONS_LAST_OPTIONID)){
	     xsldbgGenericErrorFunc(i18n("Error: Option %1 is not a valid boolean/integer option.\n").arg(xsldbgText(optionNames[type - OPTIONS_FIRST_OPTIONID])));
	}else{
#ifdef WITH_XSLDBG_DEBUG_PROCESS
	    xsldbgGenericErrorFunc(QString("Error: Invalid arguments for the command %1.\n").arg("setoption"));
#endif
	}
        result = 0;
    }
    return result;
}


/**
 * optionsGetIntOption:
 * @optionType: A valid boolean option to query
 *
 * Return the state of a boolean option
 *
 * Returns The state of a boolean xsldbg option. 
 *         ie 1 for enabled , 0 for disabled
 */
int
optionsGetIntOption(OptionTypeEnum optionType)
{
    int type = optionType, result = 0;

    if ((type >= OPTIONS_FIRST_INT_OPTIONID) && (type <= OPTIONS_LAST_INT_OPTIONID)) {
        result = intOptions[type - OPTIONS_FIRST_INT_OPTIONID];
    } else {
	if ((type >= OPTIONS_FIRST_OPTIONID) && (type <= OPTIONS_LAST_OPTIONID)){
	    xsldbgGenericErrorFunc(i18n("Error: Option %1 is not a valid boolean/integer option.\n").arg(xsldbgText(optionNames[type - OPTIONS_FIRST_OPTIONID])));
	}else{
#ifdef WITH_XSLDBG_DEBUG_PROCESS
	    xsldbgGenericErrorFunc(QString("Error: Invalid arguments for the command %1.\n").arg("options"));
#endif
	}
    }
    return result;
}



/**
 * optionsSetStringOption:
 * @optionType: A valid string option
 * @value: The value to copy
 *
 * Set value for a string xsldbg option to @value. 
 * Any memory used currently by option @optionType will be freed
 *
 * Returns 1 on success,
 *         0 otherwise
 */
int
optionsSetStringOption(OptionTypeEnum optionType, const xmlChar * value)
{
    int type = optionType, result = 0;

    if ((type >= OPTIONS_FIRST_STRING_OPTIONID) &&
        (type <= OPTIONS_LAST_STRING_OPTIONID)) {
        int optionId = type - OPTIONS_FIRST_STRING_OPTIONID;

        if (stringOptions[optionId])
            xmlFree(stringOptions[optionId]);
        if (value)
            stringOptions[optionId] =
                (xmlChar *) xmlMemStrdup((char *) value);
        else                    /* we want to be able to provide a NULL value */
            stringOptions[optionId] = NULL;
        result = 1;
    } else{
	if ((type >= OPTIONS_FIRST_OPTIONID) && (type <= OPTIONS_LAST_OPTIONID)){
	    xsldbgGenericErrorFunc(i18n("Error: Option %1 is not a valid string xsldbg option.\n").arg(xsldbgText(optionNames[type - OPTIONS_LAST_OPTIONID])));
	}else{
#ifdef WITH_XSLDBG_DEBUG_PROCESS
	    xsldbgGenericErrorFunc(QString("Error: Invalid arguments for the command %1.\n").arg("setoption"));
#endif
	}
    }
    return result;
}


/**
 * optionsGetStringOption:
 * @optionType: A valid string option 
 *
 * Get value for a string xsldbg option of @optionType

 * Returns current option value which may be NULL
 */
const xmlChar *
optionsGetStringOption(OptionTypeEnum optionType)
{
    int type = optionType;
    xmlChar *result = NULL;

    if ((type >= OPTIONS_FIRST_STRING_OPTIONID) &&
        (type <= OPTIONS_LAST_STRING_OPTIONID)) {
        int optionId = type - OPTIONS_FIRST_STRING_OPTIONID;
        result = stringOptions[optionId];
    } else{
	if ((type >= OPTIONS_FIRST_OPTIONID) && (type <= OPTIONS_LAST_OPTIONID)){
	    xsldbgGenericErrorFunc(i18n("Error: Option %1 is not a valid string xsldbg option.\n").arg(xsldbgText(optionNames[type - OPTIONS_FIRST_OPTIONID])));
	}else{
#ifdef WITH_XSLDBG_DEBUG_PROCESS
	    xsldbgGenericErrorFunc(QString("Error: Invalid arguments for the command %1.\n").arg("options"));
#endif
	}
    }
    return result;
}


  /**
   * optionsCopyVolitleOptions:
   *
   * Copy volitile options to the working area for xsldbg to be used
   *   just after xsldbg starts its processing loop
   */
void
optionsCopyVolitleOptions(void)
{
    int optionId;

    for (optionId = 0;
         optionId <= OPTIONS_LAST_INT_OPTIONID - OPTIONS_FIRST_INT_OPTIONID; optionId++) {
        intOptions[optionId] = intVolitileOptions[optionId];
    }
}


/**
 * optionsParamItemNew:
 * @name: Is valid 
 * @value: Is valid 
 *
 * Create a new libxslt parameter item
 * Returns non-null if sucessful
 *         NULL otherwise
 */
parameterItemPtr
optionsParamItemNew(const xmlChar * name, const xmlChar * value)
{
    parameterItemPtr result = NULL;

    if (name) {
        result = (parameterItem *) xmlMalloc(sizeof(parameterItem));
        if (result) {
            result->name = (xmlChar *) xmlMemStrdup((char *) name);
            if (value)
                result->value = (xmlChar *) xmlMemStrdup((char *) value);
            else
                result->value = (xmlChar *) xmlMemStrdup("");
            result->intValue = -1;
        }
    }
    return result;
}


/**
 * optionsParamItemFree:
 * @item: Is valid
 *
 * Free memory used by libxslt parameter item @item
 */
void
optionsParamItemFree(parameterItemPtr item)
{
    if (item) {
        if (item->name)
            xmlFree(item->name);
        if (item->value)
            xmlFree(item->value);
	xmlFree(item);
    }
    
}


/**
 * optionsGetParamItemList:
 *
 * Return the list of libxlt parameters
 *
 * Returns The list of parameters to provide to libxslt when doing 
 *           stylesheet transformation if successful
 *        NULL otherwise
 */
arrayListPtr
optionsGetParamItemList(void)
{
    return parameterList;
}


/**
 * optionsPrintParam:
 * @paramId: 0 =< paramID < arrayListCount(getParamList())
 * 
 * Print parameter information
 *
 * Returns 1 on success,
 *         0 otherwise
 */
int
optionsPrintParam(int paramId)
{
    int result = 0;
    parameterItemPtr paramItem =
        (parameterItemPtr) arrayListGet(optionsGetParamItemList(),
                                        paramId);

    if (paramItem && paramItem->name && paramItem->value) {
        xsldbgGenericErrorFunc(i18n(" Parameter %1 %2=\"%3\"\n").arg(paramId).arg(xsldbgText(paramItem->name)).arg(xsldbgText(paramItem->value)));
        result = 1;
    }
    return result;
}


/**
 * optionsPrintParamList:
 *
 * Prints all items in parameter list
 *
 * Returns 1 on success,
 *         0 otherwise
 */
int
optionsPrintParamList(void)
{
    int result = 1;
    int paramIndex = 0;
    int itemCount = arrayListCount(optionsGetParamItemList());

    if (getThreadStatus() == XSLDBG_MSG_THREAD_RUN) {
        if (itemCount > 0) {
            while (result && (paramIndex < itemCount)) {
                result = optionsPrintParam(paramIndex++);
            }
        }
    } else {
        if (itemCount > 0) {
            xsltGenericError(xsltGenericErrorContext, "\n");
            while (result && (paramIndex < itemCount)) {
                result = optionsPrintParam(paramIndex++);
            }
        } else
            xsldbgGenericErrorFunc(i18n("\nNo parameters present.\n"));
    }
    return result;
}


  /**
   * optionsNode:
   * @optionType : Is valid, option to convert to a xmlNodePtr 
   *
   * Convert option into a xmlNodePtr
   *
   * Returns the option @optionType as a xmlNodePtr if successful,
   *          NULL otherwise
   */
xmlNodePtr
optionsNode(OptionTypeEnum optionType)
{
    xmlNodePtr node = NULL;
    char numberBuffer[10];
    int result = 1;

    numberBuffer[0] = '\0';
    if (optionType <= OPTIONS_VERBOSE) {
        /* integer option */
        node = xmlNewNode(NULL, (xmlChar *) "intoption");
        if (node) {
            snprintf(numberBuffer, sizeof(numberBuffer), "%d",
                     optionsGetIntOption(optionType));
            result = result && (xmlNewProp
                                (node, (xmlChar *) "name",
                                 (xmlChar *) optionNames[optionType -
                                                         OPTIONS_XINCLUDE])
                                != NULL);
            if (result)
                result = result && (xmlNewProp(node, (xmlChar *) "value",
                                               (xmlChar *) numberBuffer) !=
                                    NULL);
            if (!result) {
                xmlFreeNode(node);
                node = NULL;
            }

        }
    } else {
        /* string option */
        node = xmlNewNode(NULL, (xmlChar *) "stringoption");
        if (node) {
            result = result && (xmlNewProp
                                (node, (xmlChar *) "name",
                                 (xmlChar *) optionNames[optionType -
                                                         OPTIONS_XINCLUDE])
                                != NULL);
            if (result) {
                if (optionsGetStringOption(optionType))
                    result = result && (xmlNewProp
                                        (node, (xmlChar *) "value",
                                         optionsGetStringOption
                                         (optionType)) != NULL);
                else
                    result = result && (xmlNewProp
                                        (node, (xmlChar *) "value",
                                         (xmlChar *) "") != NULL);
            }

            if (!result) {
                xmlFreeNode(node);
                node = NULL;
            }

        }
    }
    return node;
}


  /**
   * optionsReadDoc:
   * @doc : Is valid xsldbg config document, in the format as indicated 
   *        by config.dtd
   *
   * Read options from document provided. 
   *
   * Returns 1 if able to read @doc and load options found,
   *         0 otherwise
   */
int
optionsReadDoc(xmlDocPtr doc)
{
    int result = 1;
    xmlChar *name, *value;
    int optID;
    xmlNodePtr node;

    /* very primative search for config node
     * we skip the DTD and then go straight to the first child of 
     * config node */
    if (doc && doc->children->next && doc->children->next->children) {
        /* find the first configuration option */
        node = doc->children->next->children;
        while (node && result) {
            if (node->type == XML_ELEMENT_NODE) {
                if (xmlStrCmp(node->name, "intoption") == 0) {
                    name = xmlGetProp(node, (xmlChar *) "name");
                    value = xmlGetProp(node, (xmlChar *) "value");
                    if (name && value && (atoi((char *) value) >= 0)) {
                        optID = lookupName(name, (xmlChar **) optionNames);
                        if (optID >= 0)
                            result =
                                optionsSetIntOption(OptionTypeEnum(optID + OPTIONS_XINCLUDE),
                                                    atoi((char *) value));
                    }
                    if (name)
                        xmlFree(name);
                    if (value)
                        xmlFree(value);
                } else if (xmlStrCmp(node->name, "stringoption") == 0) {
                    name = xmlGetProp(node, (xmlChar *) "name");
                    value = xmlGetProp(node, (xmlChar *) "value");
                    if (name && value) {
                        optID = lookupName(name, (xmlChar **) optionNames);
                        if (optID >= 0)
                            result =
                                optionsSetStringOption(OptionTypeEnum(optID + OPTIONS_XINCLUDE),
                                                       value);
                    }
                    if (name)
                        xmlFree(name);
                    if (value)
                        xmlFree(value);
                }
            }
            node = node->next;
        }
    }
    return result;
}



/**
 * optionsSavetoFile:
 * @fileName : Must be NON NULL be a local file that can be written to
 *
 * Save configuation to file specified
 *
 * Returns 1 if able to save options to @fileName,
 *         0 otherwise
 */
int
optionsSavetoFile(xmlChar * fileName)
{
    int result = 0;
    int optionIndex = 0;
    xmlDocPtr configDoc;
    xmlNodePtr rootNode;
    xmlNodePtr node = NULL;

    if (!fileName) {
#ifdef WITH_XSLDBG_DEBUG_PROCESS
        xsltGenericError(xsltGenericErrorContext,
                         "Error: NULL file name provided\n");
#endif
        return result;
    }

    configDoc = xmlNewDoc((xmlChar *) "1.0");
    rootNode = xmlNewNode(NULL, (xmlChar *) "config");

    if (configDoc && rootNode) {
        xmlCreateIntSubset(configDoc, (xmlChar *) "config", (xmlChar *)
                           "-//xsldbg//DTD config XML V1.0//EN",
                           (xmlChar *) "config.dtd");
        xmlAddChild((xmlNodePtr) configDoc, rootNode);

        /*now to do the work of adding configuration items */
        for (optionIndex = OPTIONS_XINCLUDE;
             optionIndex <= OPTIONS_DATA_FILE_NAME; optionIndex++) {
            result = 1;
            if (optionNames[optionIndex - OPTIONS_XINCLUDE][0] == '*')
                continue;       /* don't save non user options */

            node = optionsNode(OptionTypeEnum(optionIndex));
            if (node)
                xmlAddChild(rootNode, node);
            else {
                result = 0;
                break;
            }
        }

        if (result) {
            /* so far so good, now to serialize it to disk */
            if (!xmlSaveFormatFile((char *) fileName, configDoc, 1))
                result = 0;
        }

        xmlFreeDoc(configDoc);
    } else {

        if (configDoc)
            xmlFreeDoc(configDoc);

        if (rootNode)
            xmlFreeNode(rootNode);

    }

    return result;
}



  /**
   * optionsConfigState:
   * @value : Is valid
   *
   * Set/Get the state of configuration loading/saving
   *
   * Returns The current/new value of configuration flag. Where
   *         @value means:
   *           OPTIONS_CONFIG_READVALUE : No change return current 
   *               value of read configuration flag
   *           OPTIONS_CONFIG_WRITING  : Clear flag and return 
   *               OPTIONS_CONFIG_WRITING which mean configuration 
   *               file is being written
   *           OPTIONS_CONFIG_READING : Set flag and return 
   *               OPTIONS_CONFIG_READING, which means configuration
   *               file is being read
   *           OPTIONS_CONFIG_IDLE : We are neither reading or writing 
   *               configuration and return OPTIONS_CONFIG_IDLE
   */
int
optionsConfigState(OptionsConfigState value)
{
    int result = OPTIONS_CONFIG_IDLE;

    /* work around as some compiler refuse to switch on enums */
    int configValue = value;

    /* hold the current state of configuration reading/writing */
    static int configState = OPTIONS_CONFIG_IDLE;

    switch (configValue) {
        case OPTIONS_CONFIG_READVALUE:
            /* read the current value only */
            result = configState;
            break;

        case OPTIONS_CONFIG_READING:
        case OPTIONS_CONFIG_WRITING:
            /* update the value */
            result = configValue;
            configState = configValue;
            break;
    }

    return result;
}

  /**
   * optionsAddWatch:
   * @xPath : A valid xPath to evaluate in a context and 
   *          has not already been addded
   *
   * Add xPath to be evaluated and printed out each time the debugger stops
   *
   * Returns 1 if able to add xPath to watched
   *         0 otherwise
   */
  int optionsAddWatch(const xmlChar* xPath)
{
  int result = 0;
  if (!xPath || (xmlStrlen(xPath) == 0)){
#ifdef WITH_XSLDBG_DEBUG_PROCESS
           xsltGenericError(xsltGenericErrorContext,
			    "Error: Invalid XPath supplied\n");
#endif
  } else{
    if (optionsGetWatchID(xPath) == 0){
      xmlChar * nameCopy = xmlStrdup(xPath);
      if (nameCopy){
	arrayListAdd(watchExpressionList, nameCopy);
	result = 1;
      }
    }
  }

  return result;
}


  /** 
   * optionsGetWatchID:
   * @xPath : A valid watch expression that has already been added
   *
   * Finds the ID of watch expression previously added
   *
   * Returns 0 if not found, 
   *         otherwise returns the ID of watch expression
   */
  int optionsGetWatchID(const xmlChar* xPath)
{
  int result = 0, counter;
  xmlChar* watchExpression;
  if (!xPath){
#ifdef WITH_XSLDBG_DEBUG_PROCESS
           xsltGenericError(xsltGenericErrorContext,
		       "Error: Invalid xPath supplied\n");
#endif
  } else{
    for ( counter = 0; 
	  counter < arrayListCount(watchExpressionList); 
				   counter++){
      watchExpression = (xmlChar*)arrayListGet(watchExpressionList, counter);
      if (watchExpression){
	if (xmlStrEqual(xPath, watchExpression)){
	  result = counter + 1;
	  break;
	}
      }else{
	break;
      }
    }	    
  }
 
 return result;
}

  /**
   * optionsRemoveWatch:
   * @watchID : A valid watchID as indicated by last optionsPrintWatches
   * @showWarnings : Print more error messages if 1, 
   *                  print less if 0
   *
   * Remove the watch with given ID from our list of expressions to watch
   *
   * Returns 1 if able to remove to watch expression
   *         0 otherwise
   */
  int optionsRemoveWatch(int watchID)
{
  return arrayListDelete(watchExpressionList, watchID - 1);
}

  /**
   * optionsGetWatchList:
   * 
   * Return the current list of expressions to watch
   *
   * Return the current list of expressions to watch
   */
  arrayListPtr optionsGetWatchList()
{
  return watchExpressionList;
}

