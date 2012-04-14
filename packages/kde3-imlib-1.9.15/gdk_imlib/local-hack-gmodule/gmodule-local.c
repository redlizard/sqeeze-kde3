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

/* 
 * MT safe
 */

#include	"gmodule-local.h"
#include	"gmoduleconf.h"
#include	<errno.h>
#include	<string.h>


/* We maintain a list of modules, so we can reference count them.
 * That's needed because some platforms don't support refernce counts on
 * modules e.g. the shl_* implementation of HP-UX
 * (http://www.stat.umn.edu/~luke/xls/projects/dlbasics/dlbasics.html).
 * Also, the module for the program itself is kept seperatedly for
 * faster access and because it has special semantics.
 */


/* --- structures --- */
struct _LocalHackGModule
{
  gchar	*file_name;
  gpointer handle;
  guint ref_count : 31;
  guint is_resident : 1;
  LocalHackGModuleUnload unload;
  LocalHackGModule *next;
};


/* --- prototypes --- */
static gpointer		_local_hack_g_module_open		(const gchar	*file_name,
						 gboolean	 bind_lazy);
static void		_local_hack_g_module_close		(gpointer	 handle,
						 gboolean	 is_unref);
static gpointer		_local_hack_g_module_self		(void);
static gpointer		_local_hack_g_module_symbol	(gpointer	 handle,
						 const gchar	*symbol_name);
static gchar*		_local_hack_g_module_build_path	(const gchar	*directory,
						 const gchar	*module_name);
static inline void	local_hack_g_module_set_error	(const gchar	*error);
static inline LocalHackGModule*	local_hack_g_module_find_by_handle (gpointer	 handle);
static inline LocalHackGModule*	local_hack_g_module_find_by_name	(const gchar	*name);


/* --- variables --- */
G_LOCK_DEFINE_STATIC (LocalHackGModule);
static LocalHackGModule	     *modules = NULL;
static LocalHackGModule	     *main_module = NULL;
static GStaticPrivate module_error_private = G_STATIC_PRIVATE_INIT;


/* --- inline functions --- */
static inline LocalHackGModule*
local_hack_g_module_find_by_handle (gpointer handle)
{
  LocalHackGModule *module;
  LocalHackGModule *retval = NULL;
  
  G_LOCK (LocalHackGModule);
  if (main_module && main_module->handle == handle)
    retval = main_module;
  else
    for (module = modules; module; module = module->next)
      if (handle == module->handle)
	{
	  retval = module;
	  break;
	}
  G_UNLOCK (LocalHackGModule);

  return retval;
}

static inline LocalHackGModule*
local_hack_g_module_find_by_name (const gchar *name)
{
  LocalHackGModule *module;
  LocalHackGModule *retval = NULL;
  
  G_LOCK (LocalHackGModule);
  for (module = modules; module; module = module->next)
    if (strcmp (name, module->file_name) == 0)
	{
	  retval = module;
	  break;
	}
  G_UNLOCK (LocalHackGModule);

  return retval;
}

static inline void
local_hack_g_module_set_error (const gchar *error)
{
  g_static_private_set (&module_error_private, g_strdup (error), g_free);
  errno = 0;
}


/* --- include platform specifc code --- */
#define	SUPPORT_OR_RETURN(rv)	{ local_hack_g_module_set_error (NULL); }
#if	(LOCAL_HACK_G_MODULE_IMPL == LOCAL_HACK_G_MODULE_IMPL_DL)
#include "gmodule-dl.c"
#elif	(LOCAL_HACK_G_MODULE_IMPL == LOCAL_HACK_G_MODULE_IMPL_DLD)
#include "gmodule-dld.c"
#elif	(LOCAL_HACK_G_MODULE_IMPL == LOCAL_HACK_G_MODULE_IMPL_WIN32)
#include "gmodule-win32.c"
#else
#undef	SUPPORT_OR_RETURN
#define	SUPPORT_OR_RETURN(rv)	{ local_hack_g_module_set_error ("dynamic modules are " \
                                              "not supported by this system"); return rv; }
static gpointer
_local_hack_g_module_open (const gchar	*file_name,
		gboolean	 bind_lazy)
{
  return NULL;
}
static void
_local_hack_g_module_close	(gpointer	 handle,
		 gboolean	 is_unref)
{
}
static gpointer
_local_hack_g_module_self (void)
{
  return NULL;
}
static gpointer
_local_hack_g_module_symbol (gpointer	 handle,
		  const gchar	*symbol_name)
{
  return NULL;
}
static gchar*
_local_hack_g_module_build_path (const gchar *directory,
		      const gchar *module_name)
{
  return NULL;
}
#endif	/* no implementation */

#if defined (NATIVE_WIN32) && defined (__LCC__)
int __stdcall 
LibMain (void         *hinstDll,
	 unsigned long dwReason,
	 void         *reserved)
{
  return 1;
}
#endif /* NATIVE_WIN32 && __LCC__ */


/* --- functions --- */
gboolean
local_hack_g_module_supported (void)
{
  SUPPORT_OR_RETURN (FALSE);
  
  return TRUE;
}

LocalHackGModule*
local_hack_g_module_open (const gchar    *file_name,
	       LocalHackGModuleFlags    flags)
{
  LocalHackGModule *module;
  gpointer handle;
  
  SUPPORT_OR_RETURN (NULL);
  
  if (!file_name)
    {      
      G_LOCK (LocalHackGModule);
      if (!main_module)
	{
	  handle = _local_hack_g_module_self ();
	  if (handle)
	    {
	      main_module = g_new (LocalHackGModule, 1);
	      main_module->file_name = NULL;
	      main_module->handle = handle;
	      main_module->ref_count = 1;
	      main_module->is_resident = TRUE;
	      main_module->unload = NULL;
	      main_module->next = NULL;
	    }
	}
      G_UNLOCK (LocalHackGModule);

      return main_module;
    }
  
  /* we first search the module list by name */
  module = local_hack_g_module_find_by_name (file_name);
  if (module)
    {
      module->ref_count++;
      
      return module;
    }
  
  /* open the module */
  handle = _local_hack_g_module_open (file_name, (flags & LOCAL_HACK_G_MODULE_BIND_LAZY) != 0);
  if (handle)
    {
      gchar *saved_error;
      LocalHackGModuleCheckInit check_init;
      const gchar *check_failed = NULL;
      
      /* search the module list by handle, since file names are not unique */
      module = local_hack_g_module_find_by_handle (handle);
      if (module)
	{
	  _local_hack_g_module_close (module->handle, TRUE);
	  module->ref_count++;
	  local_hack_g_module_set_error (NULL);
	  
	  return module;
	}
      
      saved_error = g_strdup (local_hack_g_module_error ());
      local_hack_g_module_set_error (NULL);
      
      module = g_new (LocalHackGModule, 1);
      module->file_name = g_strdup (file_name);
      module->handle = handle;
      module->ref_count = 1;
      module->is_resident = FALSE;
      module->unload = NULL;
      G_LOCK (LocalHackGModule);
      module->next = modules;
      modules = module;
      G_UNLOCK (LocalHackGModule);
      
      /* check initialization */
      if (local_hack_g_module_symbol (module, "local_hack_g_module_check_init", (gpointer) &check_init))
	check_failed = check_init (module);
      
      /* we don't call unload() if the initialization check failed. */
      if (!check_failed)
	local_hack_g_module_symbol (module, "local_hack_g_module_unload", (gpointer) &module->unload);
      
      if (check_failed)
	{
	  gchar *error;

	  error = g_strconcat ("LocalHackGModule initialization check failed: ", check_failed, NULL);
	  local_hack_g_module_close (module);
	  module = NULL;
	  local_hack_g_module_set_error (error);
	  g_free (error);
	}
      else
	local_hack_g_module_set_error (saved_error);

      g_free (saved_error);
    }
  
  return module;
}

gboolean
local_hack_g_module_close (LocalHackGModule	       *module)
{
  SUPPORT_OR_RETURN (FALSE);
  
  g_return_val_if_fail (module != NULL, FALSE);
  g_return_val_if_fail (module->ref_count > 0, FALSE);
  
  module->ref_count--;
  
  if (!module->ref_count && !module->is_resident && module->unload)
    {
      LocalHackGModuleUnload unload;

      unload = module->unload;
      module->unload = NULL;
      unload (module);
    }

  if (!module->ref_count && !module->is_resident)
    {
      LocalHackGModule *last;
      LocalHackGModule *node;
      
      last = NULL;
      
      G_LOCK (LocalHackGModule);
      node = modules;
      while (node)
	{
	  if (node == module)
	    {
	      if (last)
		last->next = node->next;
	      else
		modules = node->next;
	      break;
	    }
	  last = node;
	  node = last->next;
	}
      module->next = NULL;
      G_UNLOCK (LocalHackGModule);
      
      _local_hack_g_module_close (module->handle, FALSE);
      g_free (module->file_name);
      
      g_free (module);
    }
  
  return local_hack_g_module_error() == NULL;
}

void
local_hack_g_module_make_resident (LocalHackGModule *module)
{
  g_return_if_fail (module != NULL);

  module->is_resident = TRUE;
}

gchar*
local_hack_g_module_error (void)
{
  return g_static_private_get (&module_error_private);
}

gboolean
local_hack_g_module_symbol (LocalHackGModule	*module,
		 const gchar	*symbol_name,
		 gpointer	*symbol)
{
  gchar *module_error;

  if (symbol)
    *symbol = NULL;
  SUPPORT_OR_RETURN (FALSE);
  
  g_return_val_if_fail (module != NULL, FALSE);
  g_return_val_if_fail (symbol_name != NULL, FALSE);
  g_return_val_if_fail (symbol != NULL, FALSE);
  
#ifdef	LOCAL_HACK_G_MODULE_NEED_USCORE
  {
    gchar *name;

    name = g_strconcat ("_", symbol_name, NULL);
    *symbol = _local_hack_g_module_symbol (module->handle, name);
    g_free (name);
  }
#else	/* !LOCAL_HACK_G_MODULE_NEED_USCORE */
  *symbol = _local_hack_g_module_symbol (module->handle, symbol_name);
#endif	/* !LOCAL_HACK_G_MODULE_NEED_USCORE */
  
  module_error = local_hack_g_module_error ();
  if (module_error)
    {
      gchar *error;

      error = g_strconcat ("`", symbol_name, "': ", module_error, NULL);
      local_hack_g_module_set_error (error);
      g_free (error);
      *symbol = NULL;

      return FALSE;
    }
  
  return TRUE;
}

gchar*
local_hack_g_module_name (LocalHackGModule *module)
{
  g_return_val_if_fail (module != NULL, NULL);
  
  if (module == main_module)
    return "main";
  
  return module->file_name;
}

gchar*
local_hack_g_module_build_path (const gchar *directory,
		     const gchar *module_name)
{
  g_return_val_if_fail (module_name != NULL, NULL);
  
  return _local_hack_g_module_build_path (directory, module_name);
}
