/* GSL - Generic Sound Layer
 * Copyright (C) 2002 Tim Janik
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */
#ifndef __GSL_FILE_HASH_H__
#define __GSL_FILE_HASH_H__

#include <gsl/gsldefs.h>
#include <gsl/gslcommon.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



/* --- typedefs & structures --- */
typedef struct {
  gchar   *file_name;
  GTime    mtime;
  GslLong  n_bytes;
  /*< private >*/
  GslLong  cpos;
  GslMutex mutex;
  gint     fd;
  guint    ocount;
} GslHFile;
typedef struct {
  GslHFile *hfile;
  GslLong   offset;
} GslRFile;


/* --- GslHFile API --- */
GslHFile* gsl_hfile_open	(const gchar	*file_name);
GslLong	  gsl_hfile_pread	(GslHFile	*hfile,
				 GslLong	 offset,
				 GslLong         n_bytes,
				 gpointer	 bytes);
void	  gsl_hfile_close	(GslHFile	*hfile);


/* --- GslRFile API --- */
GslRFile* gsl_rfile_open	(const gchar	*file_name);
gchar*    gsl_rfile_name	(GslRFile	*rfile);
GslLong	  gsl_rfile_pread	(GslRFile	*rfile,
				 GslLong	 offset,
				 GslLong         n_bytes,
				 gpointer	 bytes);
GslLong	  gsl_rfile_read	(GslRFile	*rfile,
				 GslLong         n_bytes,
				 gpointer	 bytes);
GslLong	  gsl_rfile_seek_set	(GslRFile	*rfile,
				 GslLong	 offset);
GslLong	  gsl_rfile_position	(GslRFile	*rfile);
GslLong	  gsl_rfile_length	(GslRFile	*rfile);
void	  gsl_rfile_close	(GslRFile	*rfile);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GSL_FILE_HASH_H__ */
