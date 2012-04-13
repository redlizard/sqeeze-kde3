/* GSL Engine - Flow module operation engine
 * Copyright (C) 2001 Tim Janik
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#ifndef __GSL_ENGINE_MASTER_H__
#define __GSL_ENGINE_MASTER_H__

#include	<gsl/gslengine.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* --- internal (EngineThread) --- */
gboolean	_engine_master_prepare		(GslEngineLoop		*loop);
gboolean	_engine_master_check		(const GslEngineLoop	*loop);
void		_engine_master_dispatch_jobs	(void);
void		_engine_master_dispatch		(void);
void		_engine_master_thread		(gpointer		 data);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GSL_ENGINE_MASTER_H__ */
