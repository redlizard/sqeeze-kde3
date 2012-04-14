/* GMODULE - GLIB wrapper code for dynamic module loading
 * Copyright (C) 1998 Tim Janik
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GLib Team and others 1997-1999.  See the AUTHORS
 * file for a list of people on the GLib Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GLib at ftp://ftp.gtk.org/pub/gtk/. 
 */

#ifndef __GMODULE_H__
#define __GMODULE_H__

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* exporting and importing functions, this is special cased
 * to feature Windows dll stubs.
 */
#define	LOCAL_HACK_G_MODULE_IMPORT		extern
#ifdef NATIVE_WIN32
#  define	LOCAL_HACK_G_MODULE_EXPORT		__declspec(dllexport)
#else /* !NATIVE_WIN32 */
#  define	LOCAL_HACK_G_MODULE_EXPORT
#endif /* !NATIVE_WIN32 */

typedef enum
{
  LOCAL_HACK_G_MODULE_BIND_LAZY	= 1 << 0,
  LOCAL_HACK_G_MODULE_BIND_MASK	= 0x01
} LocalHackGModuleFlags;

typedef	struct _LocalHackGModule			 LocalHackGModule;
typedef const gchar* (*LocalHackGModuleCheckInit) (LocalHackGModule	*module);
typedef void	     (*LocalHackGModuleUnload)	 (LocalHackGModule	*module);

/* return TRUE if dynamic module loading is supported */
gboolean	local_hack_g_module_supported	   (void);

/* open a module `file_name' and return handle, which is NULL on error */
LocalHackGModule*	local_hack_g_module_open		   (const gchar		*file_name,
					    LocalHackGModuleFlags	 flags);

/* close a previously opened module, returns TRUE on success */
gboolean	local_hack_g_module_close		   (LocalHackGModule		*module);

/* make a module resident so local_hack_g_module_close on it will be ignored */
void		local_hack_g_module_make_resident	   (LocalHackGModule		*module);

/* query the last module error as a string */
gchar*		local_hack_g_module_error		   (void);

/* retrive a symbol pointer from `module', returns TRUE on success */
gboolean	local_hack_g_module_symbol		   (LocalHackGModule		*module,
					    const gchar		*symbol_name,
					    gpointer		*symbol);

/* retrive the file name from an existing module */
gchar*		local_hack_g_module_name		   (LocalHackGModule		*module);


/* Build the actual file name containing a module. `directory' is the
 * directory where the module file is supposed to be, or NULL or empty
 * in which case it should either be in the current directory or, on
 * some operating systems, in some standard place, for instance on the
 * PATH. Hence, to be absoultely sure to get the correct module,
 * always pass in a directory. The file name consists of the directory,
 * if supplied, and `module_name' suitably decorated accoring to
 * the operating system's conventions (for instance lib*.so or *.dll).
 *
 * No checks are made that the file exists, or is of correct type.
 */
gchar*		local_hack_g_module_build_path	  (const gchar		*directory,
					   const gchar		*module_name);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GMODULE_H__ */
