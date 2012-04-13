    /*

    Copyright (C) 2001-2002 Stefan Westerfeld
                            stefan@space.twc.de

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
   
    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    */

#ifndef GSLARTSTHREADS_H
#define GSLARTSTHREADS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* --- typedefs & structures --- */

typedef struct _GError {
  const char *message;
} GError;

typedef struct _GThread {
  void *data;
} GThread;

typedef enum
{
  G_THREAD_PRIORITY_LOW,
  G_THREAD_PRIORITY_NORMAL,
  G_THREAD_PRIORITY_HIGH,
  G_THREAD_PRIORITY_URGENT
} GThreadPriority;

#define g_mutex_lock		gsl_arts_mutex_lock
#define g_mutex_trylock		gsl_arts_mutex_trylock
#define g_mutex_unlock		gsl_arts_mutex_unlock
#define g_mutex_new		gsl_arts_mutex_new
#define g_mutex_free		gsl_arts_mutex_free

#define g_cond_new		gsl_arts_cond_new
#define g_cond_free		gsl_arts_cond_free
#define g_cond_signal		gsl_arts_cond_signal
#define g_cond_broadcast	gsl_arts_cond_broadcast
#define g_cond_wait		gsl_arts_cond_wait
#define g_cond_timed_wait	gsl_arts_cond_timed_wait

#define g_thread_create_full	gsl_arts_thread_create_full
#define g_thread_self		gsl_arts_thread_self

#define	g_thread_init		gsl_arts_thread_init

#define g_error_free(x)         g_assert_not_reached()

/* --- prototypes --- */

gpointer  gsl_arts_mutex_new ();
void	  gsl_arts_mutex_free (gpointer mutex);
void	  gsl_arts_mutex_lock (gpointer mutex);
gboolean  gsl_arts_mutex_trylock (gpointer mutex);
void	  gsl_arts_mutex_unlock (gpointer mutex);

gpointer  gsl_arts_cond_new ();
void	  gsl_arts_cond_free (gpointer cond);
void	  gsl_arts_cond_signal (gpointer cond);
void	  gsl_arts_cond_broadcast (gpointer cond);
void	  gsl_arts_cond_wait (gpointer cond, gpointer mutex);
void	  gsl_arts_cond_timed_wait (gpointer cond, gpointer mutex, GTimeVal *abstime);

GThread* gsl_arts_thread_create_full(gpointer (*func)(gpointer data),
                                  gpointer               data,
                                  gulong                 stack_size,
                                  gboolean               joinable,
                                  gboolean               bound,
                                  GThreadPriority        priority,
                                  GError               **error);

gpointer  gsl_arts_thread_self ();

void      gsl_arts_thread_init (gpointer arg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* vim:set ts=8 sts=2 sw=2: */

#endif // GSLARTSTHREADS_H

