// win.h --
// $Id: win.h 403187 2005-04-04 20:17:07Z osterfeld $
// This is part of Metakit, the homepage is http://www.equi4.com/metakit/

/** @file
 * Configuration header for Windows builds
 */

#if defined (_MSDOS)
#define q4_DOS 1
#endif

#if defined (_WINDOWS)
#define q4_WIN 1
#endif

#if defined (_WIN32)
#define q4_WIN32 1
#endif

#if defined (_WIN32_WCE)	// check for Win CE
#define q4_WINCE 1
#define q4_WIN32 1
#endif

#if q4_WIN32                    // WIN32 implies WIN
#undef q4_WIN
#define q4_WIN 1
#endif

#if q4_WIN                      // WIN implies not DOS, even for Win3
#undef q4_DOS
#endif
