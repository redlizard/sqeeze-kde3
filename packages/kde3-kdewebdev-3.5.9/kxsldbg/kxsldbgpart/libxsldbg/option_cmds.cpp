
/***************************************************************************
                          option_cmds.c  -  implementation for option
                                                 related commands

                             -------------------
    begin                : Fri Feb 1 2001
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
#include "utils.h"
#include "options.h"
#include "xsldbgmsg.h"
#include "xsldbgthread.h"
#include "debugXSL.h"


/**
 * xslDbgShellSetOption:
 * @arg : Is valid, and in the format   <NAME> <VALUE>
 * 
 * Set the value of an option 
 *
 * Returns 1 on success,
 *         0 otherwise
 */
int
xslDbgShellSetOption(xmlChar * arg)
{
    int result = 0;

    if (!arg) {
#ifdef WITH_XSLDBG_DEBUG_PROCESS
        xsltGenericError(xsltGenericErrorContext,
                         "Error: NULL argument provided\n");
#endif
        return result;
    }

    if (xmlStrLen(arg) > 0) {
        xmlChar *opts[2];
        long optValue;
        long optID;


        if (splitString(arg, 2, opts) == 2) {
	    bool invertOption = false;
            optID = optionsGetOptionID(opts[0]);
	    if ((optID == -1) && (opts[0][0] == 'n') &&  (opts[0][1] == 'o')){
		optID = optionsGetOptionID(&opts[0][2]);
		if (optID != -1){
		    // invert the value the user provides
		    invertOption = true;     
		}
	    }

            if (optID >= OPTIONS_FIRST_INT_OPTIONID) {
                if (optID <= OPTIONS_LAST_INT_OPTIONID) {
                    /* handle setting integer option */
                    if ((xmlStrlen(opts[1]) == 0) ||
			!sscanf((char *) opts[1], "%ld", &optValue)) {
			 xsldbgGenericErrorFunc(i18n("Error: Unable to parse %1 as an option value.\n").arg(xsldbgText(opts[1])));
                    } else {
			if (invertOption)
			    optValue = !optValue;
                        result = optionsSetIntOption(OptionTypeEnum(optID), optValue);
                    }
                } else {
                    /* handle setting a string option */
                    result = optionsSetStringOption(OptionTypeEnum(optID), opts[1]);

                }
            } else {
		static xmlExternalEntityLoader xsldbgDefaultEntLoader = 0;
		bool invertOption = false;
		bool enableNet = false;

		if (!xsldbgDefaultEntLoader)
		    xsldbgDefaultEntLoader = xmlGetExternalEntityLoader(); 

		if (xmlStrEqual(opts[0], (const xmlChar *)"nonet" ))
		    invertOption = true;
		    
		if (xmlStrEqual(&opts[0][2*invertOption], (const xmlChar *)"net" )){
		    if (sscanf((char *) opts[1], "%ld", &optValue)) {
			if (invertOption)
			    optValue = !optValue;
			if (optValue)
			    enableNet = true;
			result = true;
			if (enableNet)
			    xmlSetExternalEntityLoader(xsldbgDefaultEntLoader);
			else
			    xmlSetExternalEntityLoader(xmlNoNetExternalEntityLoader);
		    }else{
			 xsldbgGenericErrorFunc(i18n("Error: Unable to parse %1 as an option value.\n").arg(xsldbgText(opts[1])));
		    }
		} else 
                xsldbgGenericErrorFunc(i18n("Error: Unknown option name %1.\n").arg(xsldbgText(opts[0])));
            }
        } else {
            xsldbgGenericErrorFunc(i18n("Error: Missing arguments for the command %1.\n").arg("setoption"));
        }
    } else {
	xsldbgGenericErrorFunc(i18n("Error: Missing arguments for the command %1.\n").arg("setoption"));
    }

    return result;
}



/**
 * xslDbgShellOptions:
 *
 * Prints out values for user options
 *
 * Returns 1 on success,
 *         0 otherwise
 */
int
xslDbgShellOptions(void)
{
    int result = 1;
    int optionIndex;
    const xmlChar *optionName, *optionValue;

    /* Print out the integer options and thier values */
    if (getThreadStatus() != XSLDBG_MSG_THREAD_RUN) {
        for (optionIndex = OPTIONS_XINCLUDE;
             optionIndex <= OPTIONS_VERBOSE; optionIndex++) {
            /* skip any non-user options */
            optionName = optionsGetOptionName(OptionTypeEnum(optionIndex));
            if (optionName && (optionName[0] != '*')) {
                xsldbgGenericErrorFunc(i18n("Option %1 = %2\n").arg(xsldbgText(optionName)).arg(optionsGetIntOption(OptionTypeEnum(optionIndex))));

            }
        }
        /* Print out the string options and thier values */
        for (optionIndex = OPTIONS_OUTPUT_FILE_NAME;
             optionIndex <= OPTIONS_DATA_FILE_NAME; optionIndex++) {
            optionName = optionsGetOptionName(OptionTypeEnum(optionIndex));
            if (optionName && (optionName[0] != '*')) {
                optionValue = optionsGetStringOption(OptionTypeEnum(optionIndex));
                if (optionValue) {
                    xsldbgGenericErrorFunc(i18n("Option %1 = \"%2\"\n").arg(xsldbgText(optionName)).arg((char*)optionValue));
                } else {
                    xsldbgGenericErrorFunc(i18n("Option %1 = \"\"\n").arg(xsldbgText(optionName)));

                }
            }

        }
	xsldbgGenericErrorFunc("\n");
    } else {
        /* we are now notifying the application of the value of options */
        parameterItemPtr paramItem;

        notifyListStart(XSLDBG_MSG_INTOPTION_CHANGE);
        /* send the integer options and their values */
        for (optionIndex = OPTIONS_XINCLUDE;
             optionIndex <= OPTIONS_VERBOSE; optionIndex++) {
            /* skip any non-user options */
            optionName = optionsGetOptionName(OptionTypeEnum(optionIndex));
            if (optionName && (optionName[0] != '*')) {
                paramItem = optionsParamItemNew(optionName, 0L);
                if (!paramItem) {
                    notifyListSend();   /* send what ever we've got so far */
                    return 0;   /* out of memory */
                }
                paramItem->intValue = optionsGetIntOption(OptionTypeEnum(optionIndex));
                notifyListQueue(paramItem);     /* this will be free later */
            }
        }

        notifyListSend();
        notifyListStart(XSLDBG_MSG_STRINGOPTION_CHANGE);
        /* Send the string options and thier values */
        for (optionIndex = OPTIONS_OUTPUT_FILE_NAME;
             optionIndex <= OPTIONS_DATA_FILE_NAME; optionIndex++) {
            optionName = optionsGetOptionName(OptionTypeEnum(optionIndex));
            if (optionName && (optionName[0] != '*')) {
                paramItem =
                    optionsParamItemNew(optionName,
                                        optionsGetStringOption
                                        (OptionTypeEnum(optionIndex)));
                if (!paramItem) {
                    notifyListSend();   /* send what ever we've got so far */
                    return 0;   /* out of memory */
                } else
                    notifyListQueue(paramItem); /* this will be freed later */
            }
        }
        notifyListSend();
    }

    return result;
}


  /**
   * xslDbgShellShowWatches:
   * @styleCtxt: the current stylesheet context
   * @ctxt: The current shell context
   * @showWarnings : If 1 then showWarning messages,
   *                 otherwise do not show warning messages
   *
   * Print the current watches and their values
   *
   * Returns 1 on success,
   *         0 otherwise
   */
  int xslDbgShellShowWatches(xsltTransformContextPtr styleCtxt, 
			       xmlShellCtxtPtr ctx,int showWarnings)
{
  int result = 0, counter;  
  xmlChar* watchExpression;  
  if ((showWarnings == 1) && (arrayListCount(optionsGetWatchList()) == 0)){
    xsldbgGenericErrorFunc(i18n("\tNo expression watches set.\n"));
  }
  for ( counter = 0; 
	counter < arrayListCount(optionsGetWatchList()); 
	counter++){
    watchExpression = (xmlChar*)arrayListGet(optionsGetWatchList(), counter);
    if (watchExpression){
      xsldbgGenericErrorFunc(i18n(" WatchExpression %1 ").arg(counter + 1));
      result = xslDbgShellCat(styleCtxt, ctx, watchExpression);
    }else
      break;
  }

  return result;
}


  /**
   * xslDbgShellAddWatch:
   * @arg : A valid xPath of expression to watch the value of
   *
   * Add expression to list of expressions to watch value of
   *
   * Returns 1 on success,
   *         0 otherwise   
   */
  int xslDbgShellAddWatch(xmlChar* arg)
{
  int result = 0;
  if (arg){
    trimString(arg);
    result = optionsAddWatch(arg);
    if (!result)
      xsldbgGenericErrorFunc(i18n("Error: Unable to add watch expression \"%1\". It already has been added or it cannot be watched.\n").arg(xsldbgText(arg)));
  }
  return result;
}

  /**
   * xslDbgShellDeleteWatch:
   * @arg : A watch ID to remove or "*" to remove all watches
   *
   * Delete a given watch ID from our list of expressions to watch
   *
   * Returns 1 on success,
   *         0 otherwise
   */
  int xslDbgShellDeleteWatch(xmlChar* arg)
{
  int result = 0;
  long watchID;
  if (arg){
    trimString(arg);
    if (arg[0] == '*') {
      arrayListEmpty(optionsGetWatchList());
    }else if ((xmlStrlen(arg) == 0) || 
	      !sscanf((char *) arg, "%ld", &watchID)) {
      xsldbgGenericErrorFunc(i18n("Error: Unable to parse %1 as a watchID.\n").arg(xsldbgText(arg)));
      return result;
    } else {
      result = optionsRemoveWatch(watchID);
      if (!result)
	xsldbgGenericErrorFunc(i18n("Error: Watch expression %1 does not exist.\n").arg(watchID));
    }
  }
  return result;
}



