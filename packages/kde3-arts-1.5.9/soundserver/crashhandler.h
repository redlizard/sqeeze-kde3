/*
 * Copyright (C) 2003 Stefan Westerfeld
 * 
 * This code is based on code from the KDE Libraries
 * Copyright (C) 2000 Timo Hummel <timo.hummel@sap.com>
 *                    Tom Braun <braunt@fh-konstanz.de>
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#ifndef __ARTS_CRASHHANDLER_H
#define __ARTS_CRASHHANDLER_H

#include <string>

namespace Arts {

/**
 * This class handles segmentation-faults.
 * By default it displays a  message-box saying the application crashed.
 * This default can be overridden by setting a custom crash handler with
 * setCrashHandler().
 */
class CrashHandler
{
 private: // ;o)
  static const char *appName;
  static const char *appPath;
  static const char *appVersion;
  static const char *programName;
  static const char *bugAddress;
  static const char *crashApp;

 public:
  /**
   * The default crash handler.
   * @param signal the signal number
   */
  static void defaultCrashHandler (int signal);

  /**
   * This function type is a pointer to a crash handler function.
   * The function's argument is the number of the signal.
   */
  typedef void (*HandlerType)(int);

  /**
   * Install a function to be called in case a SIGSEGV is caught.
   * @param handler HandlerType handler can be one of
   * @li null in which case signal-catching is disabled
   *  (by calling signal(SIGSEGV, SIG_DFL))
   * @li if handler is omitted the default crash handler is installed.
   * @li an user defined function in the form:
   * static (if in a class) void myCrashHandler(int);
   * @param handler the crash handler
   */

  static void setCrashHandler (HandlerType handler = defaultCrashHandler);

  /**
   * Returns the installed crash handler.
   * @return the crash handler
   */
  static HandlerType crashHandler() { return _crashHandler; }

  /**
   * Sets the application @p path which should be passed to
   * Dr. Konqi, our nice crash display application.
   * @param path the application path.
   */
  static void setApplicationPath (const std::string& path) { appPath = strdup(path.c_str()); }

  /**
   * Sets the application name @p name which should be passed to
   * Dr. Konqi, our nice crash display application.
   * @param name the name of the application, as shown in Dr. Konqi
   */
  static void setApplicationName (const std::string& name) { appName = strdup(name.c_str()); }

  /**
   * Sets the application version @p name which should be passed to
   * Dr. Konqi, our nice crash display application.
   * @param name the name of the application, as shown in Dr. Konqi
   */
  static void setApplicationVersion (const std::string& name) { appVersion = strdup(name.c_str()); }

  /**
   * Sets the program name @p name which should be passed to
   * Dr. Konqi, our nice crash display application.
   * @param name the name of the application, as shown in Dr. Konqi
   */
  static void setProgramName (const std::string& name) { programName = strdup(name.c_str()); }

  /**
   * Sets the bug adress name @p name which should be passed to
   * Dr. Konqi, our nice crash display application.
   * @param name the name of the application, as shown in Dr. Konqi
   */
  static void setBugAddress (const std::string& name) { bugAddress = strdup(name.c_str()); }

  /**
   * Sets the crash application @p app which will be called to
   * handle the crash. To use Dr. Konqi (KDE), pass "drkonqi" here.
   * @param app the name of the application that handles the crash
   */
  static void setCrashApp (const std::string& app) { crashApp = strdup(app.c_str()); }

 protected:
  /**
   * Pointer to the crash handler.
   */
  static HandlerType _crashHandler;
};

}

#endif

