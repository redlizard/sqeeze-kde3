/*
 * Copyright (C) 2003 Stefan Westerfeld
 * 
 * This code is based on code from the KDE Libraries
 * Copyright (C) 2000 Timo Hummel <timo.hummel@sap.com>
 *                    Tom Braun <braunt@fh-konstanz.de>
 *
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
 */

/*
 * This file is used to catch signals which would normally
 * crash the application (like segmentation fault, floating
 * point exception and such).
 */

#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "crashhandler.h"
#include "audiosubsys.h"
#include "debug.h"
#include "config.h"

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>

#ifdef HAVE_REALTIME_SCHED
#include <sched.h>
#endif

using namespace Arts;

CrashHandler::HandlerType CrashHandler::_crashHandler = 0;
const char *CrashHandler::appName = 0;
const char *CrashHandler::appPath = 0;
const char *CrashHandler::appVersion = 0;
const char *CrashHandler::programName = 0;
const char *CrashHandler::bugAddress = 0;
const char *CrashHandler::crashApp = 0;

// This function sets the function which should be responsible for
// the application crash handling.
void
CrashHandler::setCrashHandler (HandlerType handler)
{
  if (!handler)
    handler = SIG_DFL;

  sigset_t mask;
  sigemptyset(&mask);

#ifdef SIGSEGV
  signal (SIGSEGV, handler);
  sigaddset(&mask, SIGSEGV);
#endif
#ifdef SIGFPE
  signal (SIGFPE, handler);
  sigaddset(&mask, SIGFPE);
#endif
#ifdef SIGILL
  signal (SIGILL, handler);
  sigaddset(&mask, SIGILL);
#endif
#ifdef SIGABRT
  signal (SIGABRT, handler);
  sigaddset(&mask, SIGABRT);
#endif

  sigprocmask(SIG_UNBLOCK, &mask, 0);

  _crashHandler = handler;
}

void
CrashHandler::defaultCrashHandler (int sig)
{
  // WABA: Do NOT use kdDebug() in this function because it is much too risky!
  // Handle possible recursions
  static int crashRecursionCounter = 0;
  crashRecursionCounter++; // Nothing before this, please !

  signal(SIGALRM, SIG_DFL);
  alarm(3); // Kill me... (in case we deadlock in malloc)

  if (crashRecursionCounter < 2) {
	Arts::AudioSubSystem *a = Arts::AudioSubSystem::the();
	if(a) a->emergencyCleanup();

#ifdef HAVE_REALTIME_SCHED
	struct sched_param sp;
	sp.sched_priority = 0;
	if (sched_setscheduler(0, SCHED_OTHER, &sp) != 0)
		fprintf(stderr, "CrashHandler: can't stop running as realtime process (%s)\n", strerror(errno));
#endif

    crashRecursionCounter++; //
  }

  // Close all remaining file descriptors
  struct rlimit rlp;
  getrlimit(RLIMIT_NOFILE, &rlp);
  for (int i = 0; i < (int)rlp.rlim_cur; i++)
    close(i);

  bool shuttingDown = false;

  // don't load drkonqi during shutdown
  if ( !shuttingDown )
  {
    if (crashRecursionCounter < 3)
    {
      if (appName)
      {
#ifndef NDEBUG
        fprintf(stderr, "CrashHandler: crashing... crashRecursionCounter = %d\n", crashRecursionCounter);
        fprintf(stderr, "CrashHandler: Application Name = %s path = %s pid = %d\n", appName ? appName : "<unknown>" , appPath ? appPath : "<unknown>", getpid());
#else
        fprintf(stderr, "CrashHandler: Application '%s' crashing...\n", appName ? appName : "<unknown>");
#endif

        pid_t pid = fork();

        if (pid <= 0) {
          // this code is leaking, but this should not hurt cause we will do a
          // exec() afterwards. exec() is supposed to clean up.
          char * argv[18]; // don't forget to update this
          int i = 0;

          // argument 0 has to be drkonqi
          argv[i++] = strdup(crashApp);

		  char *display = getenv("DISPLAY");
		  
		  if (display) {
          // start up on the correct display
          	argv[i++] = strdup("-display");
            argv[i++] = display;
		  }

          // we have already tested this
          argv[i++] = strdup("--appname");
          argv[i++] = strdup(appName);

          // only add apppath if it's not NULL
          if (appPath) {
            argv[i++] = strdup("--apppath");
            argv[i++] = strdup(appPath);
          }

          // signal number -- will never be NULL
          argv[i++] = strdup("--signal");
          argv[i++] = arts_strdup_printf("%d", sig);

          // pid number -- only include if this is the child
          // the debug stuff will be disabled if we was not able to fork
          if (pid == 0) {
            argv[i++] = strdup("--pid");
            argv[i++] = arts_strdup_printf("%d", getppid());
          }

		  if (appVersion) {
			argv[i++] = strdup("--appversion");
			argv[i++] = strdup(appVersion);
		  }

          if (programName) {
            argv[i++] = strdup("--programname");
            argv[i++] = strdup(programName);
          }

          if (bugAddress) {
            argv[i++] = strdup("--bugaddress");
            argv[i++] = strdup(bugAddress);
          }

          // NULL terminated list
          argv[i++] = NULL;

          setgid(getgid());
          if (getuid() != geteuid())
            setuid(getuid());
          if (getuid() != geteuid()) {
	    perror("setuid()");
            exit(255);
          }

          execvp(crashApp, argv);

          // we could clean up here
          // i = 0;
          // while (argv[i])
          //   free(argv[i++]);
        }
        else
        {

          alarm(0); // Seems we made it....

 		  fprintf(stderr, "ArtsCrash: ... waiting for child to exit\n");
          // wait for child to exit
          waitpid(pid, NULL, 0);
          _exit(253);
        }
      }
      else {
        fprintf(stderr, "Unknown appname\n");
      }
    }

    if (crashRecursionCounter < 4)
    {
      fprintf(stderr, "Unable to start %s\n", crashApp);
    }
  }

  _exit(255);
}
