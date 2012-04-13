#include "gsl/gslglib.h"
#include "gsl/gsldefs.h"

#include <locale.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define	GLIB_SIZEOF_INTMAX	(GSL_SIZEOF_STD_INTMAX_T ? GSL_SIZEOF_STD_INTMAX_T : 8 /* educated guess */)

gpointer g_malloc         (gulong        n_bytes) { void*p = malloc(n_bytes); GSL_ASSERT(p!=0); return p; }
gpointer g_malloc0        (gulong        n_bytes) { return memset(g_malloc(n_bytes),0,n_bytes); }
gpointer g_realloc        (gpointer      mem,
			   gulong        n_bytes) { void*p = realloc(mem,n_bytes); GSL_ASSERT(p!=0); return p; }
void     g_free           (gpointer      mem) { if (mem) free(mem); }
void g_usleep (unsigned long usec) { usleep (usec); }
char* g_strerror(int e) { return strerror (e); }


gpointer g_memdup (gconstpointer mem,  guint  byte_size) { gpointer new_mem; if (mem)  {    new_mem = g_malloc (byte_size);  memcpy (new_mem, mem, byte_size);  } else  new_mem = NULL;  return new_mem; }
gchar* g_strdup (const gchar *str) {   gchar *new_str;   if (str)   {    new_str = g_new (char, strlen (str) + 1);     strcpy (new_str, str);    }   else   new_str = NULL;   return new_str; }
gchar* g_strndup (const gchar *str,   gsize        n) {   gchar *new_str;   if (str)    {      new_str = g_new (gchar, n + 1);       strncpy (new_str, str, n);       new_str[n] = '\0';     }   else     new_str = NULL;   return new_str; }
gchar*g_strconcat (const gchar *string1, ...){  gsize   l;  va_list args;  gchar   *s;  gchar   *concat;  gchar   *ptr;  g_return_val_if_fail (string1 != NULL, NULL);  l = 1 + strlen (string1);  va_start (args, string1);  s = va_arg (args, gchar*);  while (s)    {      l += strlen (s);      s = va_arg (args, gchar*);    }  va_end (args);  concat = g_new (gchar, l);  ptr = concat;  ptr = g_stpcpy (ptr, string1);  va_start (args, string1);  s = va_arg (args, gchar*);  while (s)    {      ptr = g_stpcpy (ptr, s);       s = va_arg (args, gchar*);     }   va_end (args);  return concat; }

gchar*
g_strdup_printf (const gchar *format,
		 ...)
{
  gchar *buffer;
  va_list args;

  va_start (args, format);
  buffer = g_strdup_vprintf (format, args);
  va_end (args);

  return buffer;
}


gchar*
g_strdup_vprintf (const gchar *format,
		  va_list      args1)
{
  gchar *buffer;
#ifdef HAVE_VASPRINTF
  if (vasprintf (&buffer, format, args1) < 0)
    buffer = NULL;
#else
  va_list args2;

  G_VA_COPY (args2, args1);

  buffer = g_new (gchar, g_printf_string_upper_bound (format, args1));

  vsprintf (buffer, format, args2);
  va_end (args2);
#endif
  return buffer;
}
gchar *
g_stpcpy (gchar       *dest,
	  const gchar *src)
{
#ifdef GLIB_HAVE_STPCPY
  g_return_val_if_fail (dest != NULL, NULL);
  g_return_val_if_fail (src != NULL, NULL);
  return stpcpy (dest, src);
#else
  register gchar *d = dest;
  register const gchar *s = src;

  g_return_val_if_fail (dest != NULL, NULL);
  g_return_val_if_fail (src != NULL, NULL);
  do
    *d++ = *s;
  while (*s++ != '\0');

  return d - 1;
#endif
}

gchar *
g_strescape (const gchar *source,
	     const gchar *exceptions)
{
  const guchar *p;
  gchar *dest;
  gchar *q;
  guchar excmap[256];
  
  g_return_val_if_fail (source != NULL, NULL);

  p = (guchar *) source;
  /* Each source byte needs maximally four destination chars (\777) */
  q = dest = g_malloc (strlen (source) * 4 + 1);

  memset (excmap, 0, 256);
  if (exceptions)
    {
      guchar *e = (guchar *) exceptions;

      while (*e)
	{
	  excmap[*e] = 1;
	  e++;
	}
    }

  while (*p)
    {
      if (excmap[*p])
	*q++ = *p;
      else
	{
	  switch (*p)
	    {
	    case '\b':
	      *q++ = '\\';
	      *q++ = 'b';
	      break;
	    case '\f':
	      *q++ = '\\';
	      *q++ = 'f';
	      break;
	    case '\n':
	      *q++ = '\\';
	      *q++ = 'n';
	      break;
	    case '\r':
	      *q++ = '\\';
	      *q++ = 'r';
	      break;
	    case '\t':
	      *q++ = '\\';
	      *q++ = 't';
	      break;
	    case '\\':
	      *q++ = '\\';
	      *q++ = '\\';
	      break;
	    case '"':
	      *q++ = '\\';
	      *q++ = '"';
	      break;
	    default:
	      if ((*p < ' ') || (*p >= 0177))
		{
		  *q++ = '\\';
		  *q++ = '0' + (((*p) >> 6) & 07);
		  *q++ = '0' + (((*p) >> 3) & 07);
		  *q++ = '0' + ((*p) & 07);
		}
	      else
		*q++ = *p;
	      break;
	    }
	}
      p++;
    }
  *q = 0;
  return dest;
}




guint g_direct_hash (gconstpointer v) {   return GPOINTER_TO_UINT (v); }
gboolean g_direct_equal (gconstpointer v1, gconstpointer v2) {   return v1 == v2; }
gboolean g_str_equal (gconstpointer v1,   gconstpointer v2) { const gchar *string1 = v1; const gchar *string2 = v2; return strcmp (string1, string2) == 0; }
guint g_str_hash (gconstpointer key) {   const char *p = key;   guint h = *p;   if (h)     for (p += 1; *p != '\0'; p++)       h = (h << 5) - h + *p;   return h; }


void
gsl_g_log (const gchar*msg,const char *format, va_list ap)
{
  if (msg) printf ("\n%s",msg);
  vprintf(format, ap);
  if (msg) printf ("\n");
}

void
gsl_g_print_fd (int fd, const char *format, va_list ap)
{
  g_return_if_fail (fd == 1 || fd == 2);
  if (fd == 1)
    vprintf (format, ap);
  else
    vfprintf (stderr, format, ap);
}

gchar*
gsl_g_convert (const gchar  *str,
	       gsize        len,            /* gssize */
	       const gchar  *to_codeset,
	       const gchar  *from_codeset,
	       gsize        *bytes_read,
	       gsize        *bytes_written,
	       void         **error)        /* GError */
{
  g_error ("g_convert not implemented");
  
  /* not reached: */
  return 0;
}


/* --- GScanner --- */

#ifdef HAVE_SNPRINTF
/* FIXME: might want to do the configure test in gsl instead of arts */
#define g_snprintf snprintf
#else
#define g_snprintf gsl_g_snprintf
static void
gsl_g_snprintf (gchar        *out_buffer,
		size_t        size,
		const gchar  *format,
		...)
{
  gchar *buffer;
  va_list args;
  
  /* print string into large enough buffer */
  va_start (args, format);
  buffer = g_strdup_printf (format, args);
  va_end (args);
  
  /* copy the first size bytes into out_buffer */
  strncpy (out_buffer, buffer, size);
  out_buffer[ size - 1 ] = '\0';
  g_free (buffer);
}
#endif


struct _GString
{
  gchar *str;
  guint len;
};

static GString*
g_string_new(const gchar* s)
{
  GString *gstr=g_new0(GString,1);
  gstr->str= g_strdup(s);
  gstr->len=strlen(s);
  return gstr;
}
static GString*
g_string_append_c(GString*gstr, int ch)
{
  gstr->str=g_renew(gchar,gstr->str,++gstr->len + 1);
  gstr->str[gstr->len-1]=ch;
  gstr->str[gstr->len]=0;
  return gstr;
}
static void
g_string_free(GString*gstr, int freeme)
{
  if(freeme)g_free(gstr->str);g_free(gstr);
}


/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * GScanner: Flexible lexical scanner for general purpose.
 * Copyright (C) 1997, 1998 Tim Janik
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GLib Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GLib Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GLib at ftp://ftp.gtk.org/pub/gtk/. 
 */

/* 
 * MT safe
 */


#include	<stdlib.h>
#include	<stdarg.h>
#include	<string.h>
#include	<stdio.h>
#include	<unistd.h>
#include	<errno.h>

/* --- defines --- */
#define	to_lower(c)				( \
	(guchar) (							\
	  ( (((guchar)(c))>='A' && ((guchar)(c))<='Z') * ('a'-'A') ) |	\
	  ( (((guchar)(c))>=192 && ((guchar)(c))<=214) * (224-192) ) |	\
	  ( (((guchar)(c))>=216 && ((guchar)(c))<=222) * (248-216) ) |	\
	  ((guchar)(c))							\
	)								\
)
#define	READ_BUFFER_SIZE	(4000)


/* --- typedefs --- */
typedef	struct	_GScannerKey	GScannerKey;

struct	_GScannerKey
{
  guint		 scope_id;
  gchar		*symbol;
  gpointer	 value;
};



/* --- variables --- */
#define g_scanner_config_template _cplusplus_wont_have_a_problem_with_this_foog_scanner_config_template
static GScannerConfig g_scanner_config_template =
{
  (
   " \t\r\n"
   )			/* cset_skip_characters */,
  (
   G_CSET_a_2_z
   "_"
   G_CSET_A_2_Z
   )			/* cset_identifier_first */,
  (
   G_CSET_a_2_z
   "_"
   G_CSET_A_2_Z
   G_CSET_DIGITS
   G_CSET_LATINS
   G_CSET_LATINC
   )			/* cset_identifier_nth */,
  ( "#\n" )		/* cpair_comment_single */,
  
  FALSE			/* case_sensitive */,
  
  TRUE			/* skip_comment_multi */,
  TRUE			/* skip_comment_single */,
  TRUE			/* scan_comment_multi */,
  TRUE			/* scan_identifier */,
  FALSE			/* scan_identifier_1char */,
  FALSE			/* scan_identifier_NULL */,
  TRUE			/* scan_symbols */,
  FALSE			/* scan_binary */,
  TRUE			/* scan_octal */,
  TRUE			/* scan_float */,
  TRUE			/* scan_hex */,
  FALSE			/* scan_hex_dollar */,
  TRUE			/* scan_string_sq */,
  TRUE			/* scan_string_dq */,
  TRUE			/* numbers_2_int */,
  FALSE			/* int_2_float */,
  FALSE			/* identifier_2_string */,
  TRUE			/* char_2_token */,
  FALSE			/* symbol_2_token */,
  FALSE			/* scope_0_fallback */,
};


/* --- prototypes --- */
static inline
GScannerKey*	g_scanner_lookup_internal (GScanner	*scanner,
					   guint	 scope_id,
					   const gchar	*symbol);
static gboolean	g_scanner_key_equal	  (gconstpointer v1,
					   gconstpointer v2);
static guint	g_scanner_key_hash	  (gconstpointer v);
static void	g_scanner_get_token_ll	  (GScanner	*scanner,
					   GTokenType	*token_p,
					   GTokenValue	*value_p,
					   guint	*line_p,
					   guint	*position_p);
static void	g_scanner_get_token_i	  (GScanner	*scanner,
					   GTokenType	*token_p,
					   GTokenValue	*value_p,
					   guint	*line_p,
					   guint	*position_p);

static guchar	g_scanner_peek_next_char  (GScanner	*scanner);
static guchar	g_scanner_get_char	  (GScanner	*scanner,
					   guint	*line_p,
					   guint	*position_p);
static void	g_scanner_msg_handler	  (GScanner	*scanner,
					   gchar	*message,
					   gint		 is_error);


/* --- functions --- */
static inline gint
g_scanner_char_2_num (guchar	c,
		      guchar	base)
{
  if (c >= '0' && c <= '9')
    c -= '0';
  else if (c >= 'A' && c <= 'Z')
    c -= 'A' - 10;
  else if (c >= 'a' && c <= 'z')
    c -= 'a' - 10;
  else
    return -1;
  
  if (c < base)
    return c;
  
  return -1;
}

GScanner*
g_scanner_new (const GScannerConfig *config_templ)
{
  GScanner *scanner;
  
  if (!config_templ)
    config_templ = &g_scanner_config_template;
  
  scanner = g_new0 (GScanner, 1);
  
  scanner->user_data = NULL;
  scanner->max_parse_errors = 0;
  scanner->parse_errors	= 0;
  scanner->input_name = NULL;
  /* g_datalist_init (&scanner->qdata); */
  
  scanner->config = g_new0 (GScannerConfig, 1);
  
  scanner->config->case_sensitive	 = config_templ->case_sensitive;
  scanner->config->cset_skip_characters	 = config_templ->cset_skip_characters;
  if (!scanner->config->cset_skip_characters)
    scanner->config->cset_skip_characters = "";
  scanner->config->cset_identifier_first = config_templ->cset_identifier_first;
  scanner->config->cset_identifier_nth	 = config_templ->cset_identifier_nth;
  scanner->config->cpair_comment_single	 = config_templ->cpair_comment_single;
  scanner->config->skip_comment_multi	 = config_templ->skip_comment_multi;
  scanner->config->skip_comment_single	 = config_templ->skip_comment_single;
  scanner->config->scan_comment_multi	 = config_templ->scan_comment_multi;
  scanner->config->scan_identifier	 = config_templ->scan_identifier;
  scanner->config->scan_identifier_1char = config_templ->scan_identifier_1char;
  scanner->config->scan_identifier_NULL	 = config_templ->scan_identifier_NULL;
  scanner->config->scan_symbols		 = config_templ->scan_symbols;
  scanner->config->scan_binary		 = config_templ->scan_binary;
  scanner->config->scan_octal		 = config_templ->scan_octal;
  scanner->config->scan_float		 = config_templ->scan_float;
  scanner->config->scan_hex		 = config_templ->scan_hex;
  scanner->config->scan_hex_dollar	 = config_templ->scan_hex_dollar;
  scanner->config->scan_string_sq	 = config_templ->scan_string_sq;
  scanner->config->scan_string_dq	 = config_templ->scan_string_dq;
  scanner->config->numbers_2_int	 = config_templ->numbers_2_int;
  scanner->config->int_2_float		 = config_templ->int_2_float;
  scanner->config->identifier_2_string	 = config_templ->identifier_2_string;
  scanner->config->char_2_token		 = config_templ->char_2_token;
  scanner->config->symbol_2_token	 = config_templ->symbol_2_token;
  scanner->config->scope_0_fallback	 = config_templ->scope_0_fallback;
  
  scanner->token = G_TOKEN_NONE;
  scanner->value.v_int = 0;
  scanner->line = 1;
  scanner->position = 0;
  
  scanner->next_token = G_TOKEN_NONE;
  scanner->next_value.v_int = 0;
  scanner->next_line = 1;
  scanner->next_position = 0;
  
  scanner->symbol_table = g_hash_table_new (g_scanner_key_hash, g_scanner_key_equal);
  scanner->input_fd = -1;
  scanner->text = NULL;
  scanner->text_end = NULL;
  scanner->buffer = NULL;
  scanner->scope_id = 0;
  
  scanner->msg_handler = g_scanner_msg_handler;
  
  return scanner;
}

static inline void
g_scanner_free_value (GTokenType     *token_p,
		      GTokenValue     *value_p)
{
  switch (*token_p)
    {
    case G_TOKEN_STRING:
    case G_TOKEN_IDENTIFIER:
    case G_TOKEN_IDENTIFIER_NULL:
    case G_TOKEN_COMMENT_SINGLE:
    case G_TOKEN_COMMENT_MULTI:
      g_free (value_p->v_string);
      break;
      
    default:
      break;
    }
  
  *token_p = G_TOKEN_NONE;
}

static void
g_scanner_destroy_symbol_table_entry (gpointer _key,
				      gpointer _value,
				      gpointer _data)
{
  GScannerKey *key = _key;
  
  g_free (key->symbol);
  g_free (key);
}

void
g_scanner_destroy (GScanner	*scanner)
{
  g_return_if_fail (scanner != NULL);
  
  /* g_datalist_clear (&scanner->qdata); */
  g_hash_table_foreach (scanner->symbol_table, 
			g_scanner_destroy_symbol_table_entry, NULL);
  g_hash_table_destroy (scanner->symbol_table);
  g_scanner_free_value (&scanner->token, &scanner->value);
  g_scanner_free_value (&scanner->next_token, &scanner->next_value);
  g_free (scanner->config);
  g_free (scanner->buffer);
  g_free (scanner);
}

static void
g_scanner_msg_handler (GScanner		*scanner,
		       gchar		*message,
		       gint		 is_error)
{
  g_return_if_fail (scanner != NULL);
  
  fprintf (stderr, "%s:%d: ", scanner->input_name, scanner->line);
  if (is_error)
    fprintf (stderr, "error: ");
  fprintf (stderr, "%s\n", message);
}

void
g_scanner_error (GScanner	*scanner,
		 const gchar	*format,
		 ...)
{
  g_return_if_fail (scanner != NULL);
  g_return_if_fail (format != NULL);
  
  scanner->parse_errors++;
  
  if (scanner->msg_handler)
    {
      va_list args;
      gchar *string;
      
      va_start (args, format);
      string = g_strdup_vprintf (format, args);
      va_end (args);
      
      scanner->msg_handler (scanner, string, TRUE);
      
      g_free (string);
    }
}

void
g_scanner_warn (GScanner       *scanner,
		const gchar    *format,
		...)
{
  g_return_if_fail (scanner != NULL);
  g_return_if_fail (format != NULL);
  
  if (scanner->msg_handler)
    {
      va_list args;
      gchar *string;
      
      va_start (args, format);
      string = g_strdup_vprintf (format, args);
      va_end (args);
      
      scanner->msg_handler (scanner, string, FALSE);
      
      g_free (string);
    }
}

static gboolean
g_scanner_key_equal (gconstpointer v1,
		     gconstpointer v2)
{
  const GScannerKey *key1 = v1;
  const GScannerKey *key2 = v2;
  
  return (key1->scope_id == key2->scope_id) && (strcmp (key1->symbol, key2->symbol) == 0);
}

static guint
g_scanner_key_hash (gconstpointer v)
{
  const GScannerKey *key = v;
  gchar *c;
  guint h;
  
  h = key->scope_id;
  for (c = key->symbol; *c; c++)
    h = (h << 5) - h + *c;
  
  return h;
}

static inline GScannerKey*
g_scanner_lookup_internal (GScanner	*scanner,
			   guint	 scope_id,
			   const gchar	*symbol)
{
  GScannerKey	*key_p;
  GScannerKey key;
  
  key.scope_id = scope_id;
  
  if (!scanner->config->case_sensitive)
    {
      gchar *d;
      const gchar *c;
      
      key.symbol = g_new (gchar, strlen (symbol) + 1);
      for (d = key.symbol, c = symbol; *c; c++, d++)
	*d = to_lower (*c);
      *d = 0;
      key_p = g_hash_table_lookup (scanner->symbol_table, &key);
      g_free (key.symbol);
    }
  else
    {
      key.symbol = (gchar*) symbol;
      key_p = g_hash_table_lookup (scanner->symbol_table, &key);
    }
  
  return key_p;
}

void
g_scanner_scope_add_symbol (GScanner	*scanner,
			    guint	 scope_id,
			    const gchar	*symbol,
			    gpointer	 value)
{
  GScannerKey	*key;
  
  g_return_if_fail (scanner != NULL);
  g_return_if_fail (symbol != NULL);
  
  key = g_scanner_lookup_internal (scanner, scope_id, symbol);
  
  if (!key)
    {
      key = g_new (GScannerKey, 1);
      key->scope_id = scope_id;
      key->symbol = g_strdup (symbol);
      key->value = value;
      if (!scanner->config->case_sensitive)
	{
	  gchar *c;
	  
	  c = key->symbol;
	  while (*c != 0)
	    {
	      *c = to_lower (*c);
	      c++;
	    }
	}
      g_hash_table_insert (scanner->symbol_table, key, key);
    }
  else
    key->value = value;
}

void
g_scanner_scope_remove_symbol (GScanner	   *scanner,
			       guint	    scope_id,
			       const gchar *symbol)
{
  GScannerKey	*key;
  
  g_return_if_fail (scanner != NULL);
  g_return_if_fail (symbol != NULL);
  
  key = g_scanner_lookup_internal (scanner, scope_id, symbol);
  
  if (key)
    {
      g_hash_table_remove (scanner->symbol_table, key);
      g_free (key->symbol);
      g_free (key);
    }
}

gpointer
g_scanner_lookup_symbol (GScanner	*scanner,
			 const gchar	*symbol)
{
  GScannerKey	*key;
  guint scope_id;
  
  g_return_val_if_fail (scanner != NULL, NULL);
  
  if (!symbol)
    return NULL;
  
  scope_id = scanner->scope_id;
  key = g_scanner_lookup_internal (scanner, scope_id, symbol);
  if (!key && scope_id && scanner->config->scope_0_fallback)
    key = g_scanner_lookup_internal (scanner, 0, symbol);
  
  if (key)
    return key->value;
  else
    return NULL;
}

gpointer
g_scanner_scope_lookup_symbol (GScanner	      *scanner,
			       guint	       scope_id,
			       const gchar    *symbol)
{
  GScannerKey	*key;
  
  g_return_val_if_fail (scanner != NULL, NULL);
  
  if (!symbol)
    return NULL;
  
  key = g_scanner_lookup_internal (scanner, scope_id, symbol);
  
  if (key)
    return key->value;
  else
    return NULL;
}

guint
g_scanner_set_scope (GScanner	    *scanner,
		     guint	     scope_id)
{
  guint old_scope_id;
  
  g_return_val_if_fail (scanner != NULL, 0);
  
  old_scope_id = scanner->scope_id;
  scanner->scope_id = scope_id;
  
  return old_scope_id;
}

static void
g_scanner_foreach_internal (gpointer  _key,
			    gpointer  _value,
			    gpointer  _user_data)
{
  GScannerKey *key;
  gpointer *d;
  GHFunc func;
  gpointer user_data;
  guint *scope_id;
  
  d = _user_data;
  func = (GHFunc) d[0];
  user_data = d[1];
  scope_id = d[2];
  key = _value;
  
  if (key->scope_id == *scope_id)
    func (key->symbol, key->value, user_data);
}

void
g_scanner_scope_foreach_symbol (GScanner       *scanner,
				guint		scope_id,
				GHFunc		func,
				gpointer	user_data)
{
  gpointer d[3];
  
  g_return_if_fail (scanner != NULL);
  
  d[0] = (gpointer) func;
  d[1] = user_data;
  d[2] = &scope_id;
  
  g_hash_table_foreach (scanner->symbol_table, g_scanner_foreach_internal, d);
}

GTokenType
g_scanner_peek_next_token (GScanner	*scanner)
{
  g_return_val_if_fail (scanner != NULL, G_TOKEN_EOF);
  
  if (scanner->next_token == G_TOKEN_NONE)
    {
      scanner->next_line = scanner->line;
      scanner->next_position = scanner->position;
      g_scanner_get_token_i (scanner,
			     &scanner->next_token,
			     &scanner->next_value,
			     &scanner->next_line,
			     &scanner->next_position);
    }
  
  return scanner->next_token;
}

GTokenType
g_scanner_get_next_token (GScanner	*scanner)
{
  g_return_val_if_fail (scanner != NULL, G_TOKEN_EOF);
  
  if (scanner->next_token != G_TOKEN_NONE)
    {
      g_scanner_free_value (&scanner->token, &scanner->value);
      
      scanner->token = scanner->next_token;
      scanner->value = scanner->next_value;
      scanner->line = scanner->next_line;
      scanner->position = scanner->next_position;
      scanner->next_token = G_TOKEN_NONE;
    }
  else
    g_scanner_get_token_i (scanner,
			   &scanner->token,
			   &scanner->value,
			   &scanner->line,
			   &scanner->position);
  
  return scanner->token;
}

GTokenType
g_scanner_cur_token (GScanner *scanner)
{
  g_return_val_if_fail (scanner != NULL, G_TOKEN_EOF);
  
  return scanner->token;
}

GTokenValue
g_scanner_cur_value (GScanner *scanner)
{
  GTokenValue v;
  
  v.v_int = 0;
  
  g_return_val_if_fail (scanner != NULL, v);

  /* MSC isn't capable of handling return scanner->value; ? */

  v = scanner->value;

  return v;
}

guint
g_scanner_cur_line (GScanner *scanner)
{
  g_return_val_if_fail (scanner != NULL, 0);
  
  return scanner->line;
}

guint
g_scanner_cur_position (GScanner *scanner)
{
  g_return_val_if_fail (scanner != NULL, 0);
  
  return scanner->position;
}

gboolean
g_scanner_eof (GScanner	*scanner)
{
  g_return_val_if_fail (scanner != NULL, TRUE);
  
  return scanner->token == G_TOKEN_EOF;
}

void
g_scanner_input_file (GScanner *scanner,
		      gint	input_fd)
{
  g_return_if_fail (scanner != NULL);
  g_return_if_fail (input_fd >= 0);

  if (scanner->input_fd >= 0)
    g_scanner_sync_file_offset (scanner);

  scanner->token = G_TOKEN_NONE;
  scanner->value.v_int = 0;
  scanner->line = 1;
  scanner->position = 0;
  scanner->next_token = G_TOKEN_NONE;

  scanner->input_fd = input_fd;
  scanner->text = NULL;
  scanner->text_end = NULL;

  if (!scanner->buffer)
    scanner->buffer = g_new (gchar, READ_BUFFER_SIZE + 1);
}

void
g_scanner_input_text (GScanner	  *scanner,
		      const gchar *text,
		      guint	   text_len)
{
  g_return_if_fail (scanner != NULL);
  if (text_len)
    g_return_if_fail (text != NULL);
  else
    text = NULL;

  if (scanner->input_fd >= 0)
    g_scanner_sync_file_offset (scanner);

  scanner->token = G_TOKEN_NONE;
  scanner->value.v_int = 0;
  scanner->line = 1;
  scanner->position = 0;
  scanner->next_token = G_TOKEN_NONE;

  scanner->input_fd = -1;
  scanner->text = text;
  scanner->text_end = text + text_len;

  if (scanner->buffer)
    {
      g_free (scanner->buffer);
      scanner->buffer = NULL;
    }
}

static guchar
g_scanner_peek_next_char (GScanner *scanner)
{
  if (scanner->text < scanner->text_end)
    {
      return *scanner->text;
    }
  else if (scanner->input_fd >= 0)
    {
      gint count;
      gchar *buffer;

      buffer = scanner->buffer;
      do
	{
	  count = read (scanner->input_fd, buffer, READ_BUFFER_SIZE);
	}
      while (count == -1 && (errno == EINTR || errno == EAGAIN));

      if (count < 1)
	{
	  scanner->input_fd = -1;

	  return 0;
	}
      else
	{
	  scanner->text = buffer;
	  scanner->text_end = buffer + count;

	  return *buffer;
	}
    }
  else
    return 0;
}

void
g_scanner_sync_file_offset (GScanner *scanner)
{
  g_return_if_fail (scanner != NULL);

  /* for file input, rewind the filedescriptor to the current
   * buffer position and blow the file read ahead buffer. useful for
   * third party uses of our filedescriptor, which hooks onto the current
   * scanning position.
   */

  if (scanner->input_fd >= 0 && scanner->text_end > scanner->text)
    {
      gint buffered;

      buffered = scanner->text_end - scanner->text;
      if (lseek (scanner->input_fd, - buffered, SEEK_CUR) >= 0)
	{
	  /* we succeeded, blow our buffer's contents now */
	  scanner->text = NULL;
	  scanner->text_end = NULL;
	}
      else
	errno = 0;
    }
}

static guchar
g_scanner_get_char (GScanner	*scanner,
		    guint	*line_p,
		    guint	*position_p)
{
  guchar fchar;

  if (scanner->text < scanner->text_end)
    fchar = *(scanner->text++);
  else if (scanner->input_fd >= 0)
    {
      gint count;
      gchar *buffer;

      buffer = scanner->buffer;
      do
	{
	  count = read (scanner->input_fd, buffer, READ_BUFFER_SIZE);
	}
      while (count == -1 && (errno == EINTR || errno == EAGAIN));

      if (count < 1)
	{
	  scanner->input_fd = -1;
	  fchar = 0;
	}
      else
	{
	  scanner->text = buffer + 1;
	  scanner->text_end = buffer + count;
	  fchar = *buffer;
	  if (!fchar)
	    {
	      g_scanner_sync_file_offset (scanner);
	      scanner->text_end = scanner->text;
	      scanner->input_fd = -1;
	    }
	}
    }
  else
    fchar = 0;
  
  if (fchar == '\n')
    {
      (*position_p) = 0;
      (*line_p)++;
    }
  else if (fchar)
    {
      (*position_p)++;
    }
  
  return fchar;
}

void
g_scanner_unexp_token (GScanner		*scanner,
		       GTokenType	 expected_token,
		       const gchar	*identifier_spec,
		       const gchar	*symbol_spec,
		       const gchar	*symbol_name,
		       const gchar	*message,
		       gint		 is_error)
{
  gchar	*token_string;
  guint	token_string_len;
  gchar	*expected_string;
  guint	expected_string_len;
  const gchar *message_prefix;
  gboolean print_unexp;
  void (*msg_handler)	(GScanner*, const gchar*, ...);
  
  g_return_if_fail (scanner != NULL);
  
  if (is_error)
    msg_handler = g_scanner_error;
  else
    msg_handler = g_scanner_warn;
  
  if (!identifier_spec)
    identifier_spec = "identifier";
  if (!symbol_spec)
    symbol_spec = "symbol";
  
  token_string_len = 56;
  token_string = g_new (gchar, token_string_len + 1);
  expected_string_len = 64;
  expected_string = g_new (gchar, expected_string_len + 1);
  print_unexp = TRUE;
  
  switch (scanner->token)
    {
    case G_TOKEN_EOF:
      g_snprintf (token_string, token_string_len, "end of file");
      break;
      
    default:
      if (scanner->token >= 1 && scanner->token <= 255)
	{
	  if ((scanner->token >= ' ' && scanner->token <= '~') ||
	      strchr (scanner->config->cset_identifier_first, scanner->token) ||
	      strchr (scanner->config->cset_identifier_nth, scanner->token))
	    g_snprintf (token_string, expected_string_len, "character `%c'", scanner->token);
	  else
	    g_snprintf (token_string, expected_string_len, "character `\\%o'", scanner->token);
	  break;
	}
      else if (!scanner->config->symbol_2_token)
	{
	  g_snprintf (token_string, token_string_len, "(unknown) token <%d>", scanner->token);
	  break;
	}
      /* fall through */
    case G_TOKEN_SYMBOL:
      if (expected_token == G_TOKEN_SYMBOL ||
	  (scanner->config->symbol_2_token &&
	   expected_token > G_TOKEN_LAST))
	print_unexp = FALSE;
      if (symbol_name)
	g_snprintf (token_string,
		    token_string_len,
		    "%s%s `%s'",
		    print_unexp ? "" : "invalid ",
		    symbol_spec,
		    symbol_name);
      else
	g_snprintf (token_string,
		    token_string_len,
		    "%s%s",
		    print_unexp ? "" : "invalid ",
		    symbol_spec);
      break;
      
    case G_TOKEN_ERROR:
      print_unexp = FALSE;
      expected_token = G_TOKEN_NONE;
      switch (scanner->value.v_error)
	{
	case G_ERR_UNEXP_EOF:
	  g_snprintf (token_string, token_string_len, "scanner: unexpected end of file");
	  break;
	  
	case G_ERR_UNEXP_EOF_IN_STRING:
	  g_snprintf (token_string, token_string_len, "scanner: unterminated string constant");
	  break;
	  
	case G_ERR_UNEXP_EOF_IN_COMMENT:
	  g_snprintf (token_string, token_string_len, "scanner: unterminated comment");
	  break;
	  
	case G_ERR_NON_DIGIT_IN_CONST:
	  g_snprintf (token_string, token_string_len, "scanner: non digit in constant");
	  break;
	  
	case G_ERR_FLOAT_RADIX:
	  g_snprintf (token_string, token_string_len, "scanner: invalid radix for floating constant");
	  break;
	  
	case G_ERR_FLOAT_MALFORMED:
	  g_snprintf (token_string, token_string_len, "scanner: malformed floating constant");
	  break;
	  
	case G_ERR_DIGIT_RADIX:
	  g_snprintf (token_string, token_string_len, "scanner: digit is beyond radix");
	  break;
	  
	case G_ERR_UNKNOWN:
	default:
	  g_snprintf (token_string, token_string_len, "scanner: unknown error");
	  break;
	}
      break;
      
    case G_TOKEN_CHAR:
      g_snprintf (token_string, token_string_len, "character `%c'", scanner->value.v_char);
      break;
      
    case G_TOKEN_IDENTIFIER:
    case G_TOKEN_IDENTIFIER_NULL:
      if (expected_token == G_TOKEN_IDENTIFIER ||
	  expected_token == G_TOKEN_IDENTIFIER_NULL)
	print_unexp = FALSE;
      g_snprintf (token_string,
		  token_string_len,
		  "%s%s `%s'",
		  print_unexp ? "" : "invalid ",
		  identifier_spec,
		  scanner->token == G_TOKEN_IDENTIFIER ? scanner->value.v_string : "null");
      break;
      
    case G_TOKEN_BINARY:
    case G_TOKEN_OCTAL:
    case G_TOKEN_INT:
    case G_TOKEN_HEX:
      g_snprintf (token_string, token_string_len, "number `%ld'", scanner->value.v_int);
      break;
      
    case G_TOKEN_FLOAT:
      g_snprintf (token_string, token_string_len, "number `%.3f'", scanner->value.v_float);
      break;
      
    case G_TOKEN_STRING:
      if (expected_token == G_TOKEN_STRING)
	print_unexp = FALSE;
      g_snprintf (token_string,
		  token_string_len,
		  "%s%sstring constant \"%s\"",
		  print_unexp ? "" : "invalid ",
		  scanner->value.v_string[0] == 0 ? "empty " : "",
		  scanner->value.v_string);
      token_string[token_string_len - 2] = '"';
      token_string[token_string_len - 1] = 0;
      break;
      
    case G_TOKEN_COMMENT_SINGLE:
    case G_TOKEN_COMMENT_MULTI:
      g_snprintf (token_string, token_string_len, "comment");
      break;
      
    case G_TOKEN_NONE:
      /* somehow the user's parsing code is screwed, there isn't much
       * we can do about it.
       * Note, a common case to trigger this is
       * g_scanner_peek_next_token(); g_scanner_unexp_token();
       * without an intermediate g_scanner_get_next_token().
       */
      g_assert_not_reached ();
      break;
    }
  
  
  switch (expected_token)
    {
      gboolean need_valid;
      
    default:
      if (expected_token >= 1 && expected_token <= 255)
	{
	  if ((expected_token >= ' ' && expected_token <= '~') ||
	      strchr (scanner->config->cset_identifier_first, expected_token) ||
	      strchr (scanner->config->cset_identifier_nth, expected_token))
	    g_snprintf (expected_string, expected_string_len, "character `%c'", expected_token);
	  else
	    g_snprintf (expected_string, expected_string_len, "character `\\%o'", expected_token);
	  break;
	}
      else if (!scanner->config->symbol_2_token)
	{
	  g_snprintf (expected_string, expected_string_len, "(unknown) token <%d>", expected_token);
	  break;
	}
      /* fall through */
    case G_TOKEN_SYMBOL:
      need_valid = (scanner->token == G_TOKEN_SYMBOL ||
		    (scanner->config->symbol_2_token &&
		     scanner->token > G_TOKEN_LAST));
      g_snprintf (expected_string,
		  expected_string_len,
		  "%s%s",
		  need_valid ? "valid " : "",
		  symbol_spec);
      /* FIXME: should we attempt to lookup the symbol_name for symbol_2_token? */
      break;
      
    case G_TOKEN_INT:
      g_snprintf (expected_string, expected_string_len, "%snumber (integer)",
		  scanner->token == G_TOKEN_INT ? "valid " : "");
      break;
      
    case G_TOKEN_FLOAT:
      g_snprintf (expected_string, expected_string_len, "%snumber (float)",
		  scanner->token == G_TOKEN_FLOAT ? "valid " : "");
      break;
      
    case G_TOKEN_STRING:
      g_snprintf (expected_string,
		  expected_string_len,
		  "%sstring constant",
		  scanner->token == G_TOKEN_STRING ? "valid " : "");
      break;
      
    case G_TOKEN_IDENTIFIER:
    case G_TOKEN_IDENTIFIER_NULL:
      g_snprintf (expected_string,
		  expected_string_len,
		  "%s%s",
		  (scanner->token == G_TOKEN_IDENTIFIER_NULL ||
		   scanner->token == G_TOKEN_IDENTIFIER ? "valid " : ""),
		  identifier_spec);
      break;
      
    case G_TOKEN_EOF:
      g_snprintf (expected_string, expected_string_len, "end of file");
      break;

    case G_TOKEN_NONE:
      break;
    }
  
  if (message && message[0] != 0)
    message_prefix = " - ";
  else
    {
      message_prefix = "";
      message = "";
    }
  
  if (expected_token != G_TOKEN_NONE)
    {
      if (print_unexp)
	msg_handler (scanner,
		     "unexpected %s, expected %s%s%s",
		     token_string,
		     expected_string,
		     message_prefix,
		     message);
      else
	msg_handler (scanner,
		     "%s, expected %s%s%s",
		     token_string,
		     expected_string,
		     message_prefix,
		     message);
    }
  else
    {
      if (print_unexp)
	msg_handler (scanner,
		     "unexpected %s%s%s",
		     token_string,
		     message_prefix,
		     message);
      else
	msg_handler (scanner,
		     "%s%s%s",
		     token_string,
		     message_prefix,
		     message);
    }
  
  g_free (token_string);
  g_free (expected_string);
}

static void
g_scanner_get_token_i (GScanner	*scanner,
		       GTokenType	*token_p,
		       GTokenValue	*value_p,
		       guint		*line_p,
		       guint		*position_p)
{
  do
    {
      g_scanner_free_value (token_p, value_p);
      g_scanner_get_token_ll (scanner, token_p, value_p, line_p, position_p);
    }
  while (((*token_p > 0 && *token_p < 256) &&
	  strchr (scanner->config->cset_skip_characters, *token_p)) ||
	 (*token_p == G_TOKEN_CHAR &&
	  strchr (scanner->config->cset_skip_characters, value_p->v_char)) ||
	 (*token_p == G_TOKEN_COMMENT_MULTI &&
	  scanner->config->skip_comment_multi) ||
	 (*token_p == G_TOKEN_COMMENT_SINGLE &&
	  scanner->config->skip_comment_single));
  
  switch (*token_p)
    {
    case G_TOKEN_IDENTIFIER:
      if (scanner->config->identifier_2_string)
	*token_p = G_TOKEN_STRING;
      break;
      
    case G_TOKEN_SYMBOL:
      if (scanner->config->symbol_2_token)
	*token_p = (GTokenType) value_p->v_symbol;
      break;
      
    case G_TOKEN_BINARY:
    case G_TOKEN_OCTAL:
    case G_TOKEN_HEX:
      if (scanner->config->numbers_2_int)
	*token_p = G_TOKEN_INT;
      break;
      
    default:
      break;
    }
  
  if (*token_p == G_TOKEN_INT &&
      scanner->config->int_2_float)
    {
      *token_p = G_TOKEN_FLOAT;
      value_p->v_float = value_p->v_int;
    }
  
  errno = 0;
}

static void
g_scanner_get_token_ll	(GScanner	*scanner,
			 GTokenType	*token_p,
			 GTokenValue	*value_p,
			 guint		*line_p,
			 guint		*position_p)
{
  GScannerConfig *config;
  GTokenType	   token;
  gboolean	   in_comment_multi;
  gboolean	   in_comment_single;
  gboolean	   in_string_sq;
  gboolean	   in_string_dq;
  GString	  *gstring;
  GTokenValue	   value;
  guchar	   ch;
  
  config = scanner->config;
  (*value_p).v_int = 0;
  
  if ((scanner->text >= scanner->text_end && scanner->input_fd < 0) ||
      scanner->token == G_TOKEN_EOF)
    {
      *token_p = G_TOKEN_EOF;
      return;
    }
  
  in_comment_multi = FALSE;
  in_comment_single = FALSE;
  in_string_sq = FALSE;
  in_string_dq = FALSE;
  gstring = NULL;
  
  do /* while (ch != 0) */
    {
      gboolean dotted_float = FALSE;
      
      ch = g_scanner_get_char (scanner, line_p, position_p);
      
      value.v_int = 0;
      token = G_TOKEN_NONE;
      
      /* this is *evil*, but needed ;(
       * we first check for identifier first character, because	 it
       * might interfere with other key chars like slashes or numbers
       */
      if (config->scan_identifier &&
	  ch && strchr (config->cset_identifier_first, ch))
	goto identifier_precedence;
      
      switch (ch)
	{
	case 0:
	  token = G_TOKEN_EOF;
	  (*position_p)++;
	  /* ch = 0; */
	  break;
	  
	case '/':
	  if (!config->scan_comment_multi ||
	      g_scanner_peek_next_char (scanner) != '*')
	    goto default_case;
	  g_scanner_get_char (scanner, line_p, position_p);
	  token = G_TOKEN_COMMENT_MULTI;
	  in_comment_multi = TRUE;
	  gstring = g_string_new ("");
	  while ((ch = g_scanner_get_char (scanner, line_p, position_p)) != 0)
	    {
	      if (ch == '*' && g_scanner_peek_next_char (scanner) == '/')
		{
		  g_scanner_get_char (scanner, line_p, position_p);
		  in_comment_multi = FALSE;
		  break;
		}
	      else
		g_string_append_c (gstring, ch);
	    }
	  ch = 0;
	  break;
	  
	case '\'':
	  if (!config->scan_string_sq)
	    goto default_case;
	  token = G_TOKEN_STRING;
	  in_string_sq = TRUE;
	  gstring = g_string_new ("");
	  while ((ch = g_scanner_get_char (scanner, line_p, position_p)) != 0)
	    {
	      if (ch == '\'')
		{
		  in_string_sq = FALSE;
		  break;
		}
	      else
		gstring = g_string_append_c (gstring, ch);
	    }
	  ch = 0;
	  break;
	  
	case '"':
	  if (!config->scan_string_dq)
	    goto default_case;
	  token = G_TOKEN_STRING;
	  in_string_dq = TRUE;
	  gstring = g_string_new ("");
	  while ((ch = g_scanner_get_char (scanner, line_p, position_p)) != 0)
	    {
	      if (ch == '"')
		{
		  in_string_dq = FALSE;
		  break;
		}
	      else
		{
		  if (ch == '\\')
		    {
		      ch = g_scanner_get_char (scanner, line_p, position_p);
		      switch (ch)
			{
			  guint	i;
			  guint	fchar;
			  
			case 0:
			  break;
			  
			case '\\':
			  gstring = g_string_append_c (gstring, '\\');
			  break;
			  
			case 'n':
			  gstring = g_string_append_c (gstring, '\n');
			  break;
			  
			case 't':
			  gstring = g_string_append_c (gstring, '\t');
			  break;
			  
			case 'r':
			  gstring = g_string_append_c (gstring, '\r');
			  break;
			  
			case 'b':
			  gstring = g_string_append_c (gstring, '\b');
			  break;
			  
			case 'f':
			  gstring = g_string_append_c (gstring, '\f');
			  break;
			  
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			  i = ch - '0';
			  fchar = g_scanner_peek_next_char (scanner);
			  if (fchar >= '0' && fchar <= '7')
			    {
			      ch = g_scanner_get_char (scanner, line_p, position_p);
			      i = i * 8 + ch - '0';
			      fchar = g_scanner_peek_next_char (scanner);
			      if (fchar >= '0' && fchar <= '7')
				{
				  ch = g_scanner_get_char (scanner, line_p, position_p);
				  i = i * 8 + ch - '0';
				}
			    }
			  gstring = g_string_append_c (gstring, i);
			  break;
			  
			default:
			  gstring = g_string_append_c (gstring, ch);
			  break;
			}
		    }
		  else
		    gstring = g_string_append_c (gstring, ch);
		}
	    }
	  ch = 0;
	  break;
	  
	case '.':
	  if (!config->scan_float)
	    goto default_case;
	  token = G_TOKEN_FLOAT;
	  dotted_float = TRUE;
	  ch = g_scanner_get_char (scanner, line_p, position_p);
	  goto number_parsing;
	  
	case '$':
	  if (!config->scan_hex_dollar)
	    goto default_case;
	  token = G_TOKEN_HEX;
	  ch = g_scanner_get_char (scanner, line_p, position_p);
	  goto number_parsing;
	  
	case '0':
	  if (config->scan_octal)
	    token = G_TOKEN_OCTAL;
	  else
	    token = G_TOKEN_INT;
	  ch = g_scanner_peek_next_char (scanner);
	  if (config->scan_hex && (ch == 'x' || ch == 'X'))
	    {
	      token = G_TOKEN_HEX;
	      g_scanner_get_char (scanner, line_p, position_p);
	      ch = g_scanner_get_char (scanner, line_p, position_p);
	      if (ch == 0)
		{
		  token = G_TOKEN_ERROR;
		  value.v_error = G_ERR_UNEXP_EOF;
		  (*position_p)++;
		  break;
		}
	      if (g_scanner_char_2_num (ch, 16) < 0)
		{
		  token = G_TOKEN_ERROR;
		  value.v_error = G_ERR_DIGIT_RADIX;
		  ch = 0;
		  break;
		}
	    }
	  else if (config->scan_binary && (ch == 'b' || ch == 'B'))
	    {
	      token = G_TOKEN_BINARY;
	      g_scanner_get_char (scanner, line_p, position_p);
	      ch = g_scanner_get_char (scanner, line_p, position_p);
	      if (ch == 0)
		{
		  token = G_TOKEN_ERROR;
		  value.v_error = G_ERR_UNEXP_EOF;
		  (*position_p)++;
		  break;
		}
	      if (g_scanner_char_2_num (ch, 10) < 0)
		{
		  token = G_TOKEN_ERROR;
		  value.v_error = G_ERR_NON_DIGIT_IN_CONST;
		  ch = 0;
		  break;
		}
	    }
	  else
	    ch = '0';
	  /* fall through */
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	number_parsing:
	{
          gboolean in_number = TRUE;
	  gchar *endptr;
	  
	  if (token == G_TOKEN_NONE)
	    token = G_TOKEN_INT;
	  
	  gstring = g_string_new (dotted_float ? "0." : "");
	  gstring = g_string_append_c (gstring, ch);
	  
	  do /* while (in_number) */
	    {
	      gboolean is_E;
	      
	      is_E = token == G_TOKEN_FLOAT && (ch == 'e' || ch == 'E');
	      
	      ch = g_scanner_peek_next_char (scanner);
	      
	      if (g_scanner_char_2_num (ch, 36) >= 0 ||
		  (config->scan_float && ch == '.') ||
		  (is_E && (ch == '+' || ch == '-')))
		{
		  ch = g_scanner_get_char (scanner, line_p, position_p);
		  
		  switch (ch)
		    {
		    case '.':
		      if (token != G_TOKEN_INT && token != G_TOKEN_OCTAL)
			{
			  value.v_error = token == G_TOKEN_FLOAT ? G_ERR_FLOAT_MALFORMED : G_ERR_FLOAT_RADIX;
			  token = G_TOKEN_ERROR;
			  in_number = FALSE;
			}
		      else
			{
			  token = G_TOKEN_FLOAT;
			  gstring = g_string_append_c (gstring, ch);
			}
		      break;
		      
		    case '0':
		    case '1':
		    case '2':
		    case '3':
		    case '4':
		    case '5':
		    case '6':
		    case '7':
		    case '8':
		    case '9':
		      gstring = g_string_append_c (gstring, ch);
		      break;
		      
		    case '-':
		    case '+':
		      if (token != G_TOKEN_FLOAT)
			{
			  token = G_TOKEN_ERROR;
			  value.v_error = G_ERR_NON_DIGIT_IN_CONST;
			  in_number = FALSE;
			}
		      else
			gstring = g_string_append_c (gstring, ch);
		      break;
		      
		    case 'e':
		    case 'E':
		      if ((token != G_TOKEN_HEX && !config->scan_float) ||
			  (token != G_TOKEN_HEX &&
			   token != G_TOKEN_OCTAL &&
			   token != G_TOKEN_FLOAT &&
			   token != G_TOKEN_INT))
			{
			  token = G_TOKEN_ERROR;
			  value.v_error = G_ERR_NON_DIGIT_IN_CONST;
			  in_number = FALSE;
			}
		      else
			{
			  if (token != G_TOKEN_HEX)
			    token = G_TOKEN_FLOAT;
			  gstring = g_string_append_c (gstring, ch);
			}
		      break;
		      
		    default:
		      if (token != G_TOKEN_HEX)
			{
			  token = G_TOKEN_ERROR;
			  value.v_error = G_ERR_NON_DIGIT_IN_CONST;
			  in_number = FALSE;
			}
		      else
			gstring = g_string_append_c (gstring, ch);
		      break;
		    }
		}
	      else
		in_number = FALSE;
	    }
	  while (in_number);
	  
	  endptr = NULL;
	  switch (token)
	    {
	    case G_TOKEN_BINARY:
	      value.v_binary = strtol (gstring->str, &endptr, 2);
	      break;
	      
	    case G_TOKEN_OCTAL:
	      value.v_octal = strtol (gstring->str, &endptr, 8);
	      break;
	      
	    case G_TOKEN_INT:
	      value.v_int = strtol (gstring->str, &endptr, 10);
	      break;
	      
	    case G_TOKEN_FLOAT:
	      value.v_float = g_strtod (gstring->str, &endptr);
	      break;
	      
	    case G_TOKEN_HEX:
	      value.v_hex = strtol (gstring->str, &endptr, 16);
	      break;
	      
	    default:
	      break;
	    }
	  if (endptr && *endptr)
	    {
	      token = G_TOKEN_ERROR;
	      if (*endptr == 'e' || *endptr == 'E')
		value.v_error = G_ERR_NON_DIGIT_IN_CONST;
	      else
		value.v_error = G_ERR_DIGIT_RADIX;
	    }
	  g_string_free (gstring, TRUE);
	  gstring = NULL;
	  ch = 0;
	} /* number_parsing:... */
	break;
	
	default:
	default_case:
	{
	  if (config->cpair_comment_single &&
	      ch == config->cpair_comment_single[0])
	    {
	      token = G_TOKEN_COMMENT_SINGLE;
	      in_comment_single = TRUE;
	      gstring = g_string_new ("");
	      ch = g_scanner_get_char (scanner, line_p, position_p);
	      while (ch != 0)
		{
		  if (ch == config->cpair_comment_single[1])
		    {
		      in_comment_single = FALSE;
		      ch = 0;
		      break;
		    }
		  
		  gstring = g_string_append_c (gstring, ch);
		  ch = g_scanner_get_char (scanner, line_p, position_p);
		}
	    }
	  else if (config->scan_identifier && ch &&
		   strchr (config->cset_identifier_first, ch))
	    {
	    identifier_precedence:
	      
	      if (config->cset_identifier_nth && ch &&
		  strchr (config->cset_identifier_nth,
			  g_scanner_peek_next_char (scanner)))
		{
		  token = G_TOKEN_IDENTIFIER;
		  gstring = g_string_new ("");
		  gstring = g_string_append_c (gstring, ch);
		  do
		    {
		      ch = g_scanner_get_char (scanner, line_p, position_p);
		      gstring = g_string_append_c (gstring, ch);
		      ch = g_scanner_peek_next_char (scanner);
		    }
		  while (ch && strchr (config->cset_identifier_nth, ch));
		  ch = 0;
		}
	      else if (config->scan_identifier_1char)
		{
		  token = G_TOKEN_IDENTIFIER;
		  value.v_identifier = g_new0 (gchar, 2);
		  value.v_identifier[0] = ch;
		  ch = 0;
		}
	    }
	  if (ch)
	    {
	      if (config->char_2_token)
		token = ch;
	      else
		{
		  token = G_TOKEN_CHAR;
		  value.v_char = ch;
		}
	      ch = 0;
	    }
	} /* default_case:... */
	break;
	}
      g_assert (ch == 0 && token != G_TOKEN_NONE); /* paranoid */
    }
  while (ch != 0);
  
  if (in_comment_multi || in_comment_single ||
      in_string_sq || in_string_dq)
    {
      token = G_TOKEN_ERROR;
      if (gstring)
	{
	  g_string_free (gstring, TRUE);
	  gstring = NULL;
	}
      (*position_p)++;
      if (in_comment_multi || in_comment_single)
	value.v_error = G_ERR_UNEXP_EOF_IN_COMMENT;
      else /* (in_string_sq || in_string_dq) */
	value.v_error = G_ERR_UNEXP_EOF_IN_STRING;
    }
  
  if (gstring)
    {
      value.v_string = gstring->str;
      g_string_free (gstring, FALSE);
      gstring = NULL;
    }
  
  if (token == G_TOKEN_IDENTIFIER)
    {
      if (config->scan_symbols)
	{
	  GScannerKey *key;
	  guint scope_id;
	  
	  scope_id = scanner->scope_id;
	  key = g_scanner_lookup_internal (scanner, scope_id, value.v_identifier);
	  if (!key && scope_id && scanner->config->scope_0_fallback)
	    key = g_scanner_lookup_internal (scanner, 0, value.v_identifier);
	  
	  if (key)
	    {
	      g_free (value.v_identifier);
	      token = G_TOKEN_SYMBOL;
	      value.v_symbol = key->value;
	    }
	}
      
      if (token == G_TOKEN_IDENTIFIER &&
	  config->scan_identifier_NULL &&
	  strlen (value.v_identifier) == 4)
	{
	  const gchar *null_upper = "NULL";
	  const gchar *null_lower = "null";
	  
	  if (scanner->config->case_sensitive)
	    {
	      if (value.v_identifier[0] == null_upper[0] &&
		  value.v_identifier[1] == null_upper[1] &&
		  value.v_identifier[2] == null_upper[2] &&
		  value.v_identifier[3] == null_upper[3])
		token = G_TOKEN_IDENTIFIER_NULL;
	    }
	  else
	    {
	      if ((value.v_identifier[0] == null_upper[0] ||
		   value.v_identifier[0] == null_lower[0]) &&
		  (value.v_identifier[1] == null_upper[1] ||
		   value.v_identifier[1] == null_lower[1]) &&
		  (value.v_identifier[2] == null_upper[2] ||
		   value.v_identifier[2] == null_lower[2]) &&
		  (value.v_identifier[3] == null_upper[3] ||
		   value.v_identifier[3] == null_lower[3]))
		token = G_TOKEN_IDENTIFIER_NULL;
	    }
	}
    }
  
  *token_p = token;
  *value_p = value;
}




gdouble
g_strtod (const gchar *nptr,
	  gchar **endptr)
{
  gchar *fail_pos_1;
  gchar *fail_pos_2;
  gdouble val_1;
  gdouble val_2 = 0;

  g_return_val_if_fail (nptr != NULL, 0);

  fail_pos_1 = NULL;
  fail_pos_2 = NULL;

  val_1 = strtod (nptr, &fail_pos_1);

  if (fail_pos_1 && fail_pos_1[0] != 0)
    {
      gchar *old_locale;

      old_locale = g_strdup (setlocale (LC_NUMERIC, NULL));
      setlocale (LC_NUMERIC, "C");
      val_2 = strtod (nptr, &fail_pos_2);
      setlocale (LC_NUMERIC, old_locale);
      g_free (old_locale);
    }

  if (!fail_pos_1 || fail_pos_1[0] == 0 || fail_pos_1 >= fail_pos_2)
    {
      if (endptr)
	*endptr = fail_pos_1;
      return val_1;
    }
  else
    {
      if (endptr)
	*endptr = fail_pos_2;
      return val_2;
    }
}


#ifndef MB_LEN_MAX
#  define MB_LEN_MAX 8
#endif

typedef struct
{
  guint min_width;
  guint precision;
  gboolean alternate_format, zero_padding, adjust_left, locale_grouping;
  gboolean add_space, add_sign, possible_sign, seen_precision;
  gboolean mod_half, mod_long, mod_extra_long;
} PrintfArgSpec;


static gsize
printf_string_upper_bound (const gchar *format,
			   gboolean     may_warn,
			   va_list      args)
{
  static  gboolean honour_longs = sizeof(long) > 4 || sizeof(void*) > 4;
  gsize len = 1;

  if (!format)
    return len;

  while (*format)
    {
      register gchar c = *format++;

      if (c != '%')
	len += 1;
      else /* (c == '%') */
	{
	  PrintfArgSpec spec = { 0, };
	  gboolean seen_l = FALSE, conv_done = FALSE;
	  gsize conv_len = 0;
	  const gchar *spec_start = format;

	  do
	    {
	      c = *format++;
	      switch (c)
		{
		  GDoubleIEEE754 u_double;
		  guint v_uint;
		  gint v_int;
		  const gchar *v_string;

		  /* beware of positional parameters
		   */
		case '$':
		  if (may_warn)
		    g_warning (G_STRLOC ": unable to handle positional parameters (%%n$)");
		  len += 1024; /* try adding some safety padding */
		  break;

		  /* parse flags
		   */
		case '#':
		  spec.alternate_format = TRUE;
		  break;
		case '0':
		  spec.zero_padding = TRUE;
		  break;
		case '-':
		  spec.adjust_left = TRUE;
		  break;
		case ' ':
		  spec.add_space = TRUE;
		  break;
		case '+':
		  spec.add_sign = TRUE;
		  break;
		case '\'':
		  spec.locale_grouping = TRUE;
		  break;

		  /* parse output size specifications
		   */
		case '.':
		  spec.seen_precision = TRUE;
		  break;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		  v_uint = c - '0';
		  c = *format;
		  while (c >= '0' && c <= '9')
		    {
		      format++;
		      v_uint = v_uint * 10 + c - '0';
		      c = *format;
		    }
		  if (spec.seen_precision)
		    spec.precision = MAX (spec.precision, v_uint);
		  else
		    spec.min_width = MAX (spec.min_width, v_uint);
		  break;
		case '*':
		  v_int = va_arg (args, int);
		  if (spec.seen_precision)
		    {
		      /* forget about negative precision */
		      if (v_int >= 0)
			spec.precision = MAX (spec.precision, v_int);
		    }
		  else
		    {
		      if (v_int < 0)
			{
			  v_int = - v_int;
			  spec.adjust_left = TRUE;
			}
		      spec.min_width = MAX (spec.min_width, v_int);
		    }
		  break;

		  /* parse type modifiers
		   */
		case 'h':
		  spec.mod_half = TRUE;
		  break;
		case 'l':
		  if (!seen_l)
		    {
		      spec.mod_long = TRUE;
		      seen_l = TRUE;
		      break;
		    }
		  /* else, fall through */
		case 'L':
		case 'q':
		  spec.mod_long = TRUE;
		  spec.mod_extra_long = TRUE;
		  break;
		case 'z':
		case 'Z':
		  if (sizeof(size_t))
		    {
		      spec.mod_long = TRUE;
		      spec.mod_extra_long = TRUE;
		    }
		  break;
		case 't':
		  if (sizeof(ptrdiff_t) > 4)
		    {
		      spec.mod_long = TRUE;
		      spec.mod_extra_long = TRUE;
		    }
		  break;
		case 'j':
		  if (GLIB_SIZEOF_INTMAX > 4)
		    {
		      spec.mod_long = TRUE;
		      spec.mod_extra_long = TRUE;
		    }
		  break;

		  /* parse output conversions
		   */
		case '%':
		  conv_len += 1;
		  break;
		case 'O':
		case 'D':
		case 'I':
		case 'U':
		  /* some C libraries feature long variants for these as well? */
		  spec.mod_long = TRUE;
		  /* fall through */
		case 'o':
		  conv_len += 2;
		  /* fall through */
		case 'd':
		case 'i':
		  conv_len += 1; /* sign */
		  /* fall through */
		case 'u':
		  conv_len += 4;
		  /* fall through */
		case 'x':
		case 'X':
		  spec.possible_sign = TRUE;
		  conv_len += 10;
		  if (spec.mod_long && honour_longs)
		    conv_len *= 2;
		  if (spec.mod_extra_long)
		    conv_len *= 2;
		  if (spec.mod_extra_long)
		    {
		      (void) va_arg (args, gint64);
		    }
		  else if (spec.mod_long)
		    (void) va_arg (args, long);
		  else
		    (void) va_arg (args, int);
		  break;
		case 'A':
		case 'a':
		  /*          0x */
		  conv_len += 2;
		  /* fall through */
		case 'g':
		case 'G':
		case 'e':
		case 'E':
		case 'f':
		  spec.possible_sign = TRUE;
		  /*          n   .   dddddddddddddddddddddddd   E   +-  eeee */
		  conv_len += 1 + 1 + MAX (24, spec.precision) + 1 + 1 + 4;
                  if (may_warn && spec.mod_extra_long)
		    g_warning (G_STRLOC ": unable to handle long double, collecting double only");
#ifdef HAVE_LONG_DOUBLE
#error need to implement special handling for long double
#endif
		  u_double.v_double = va_arg (args, double);
		  /* %f can expand up to all significant digits before '.' (308) */
		  if (c == 'f' &&
		      u_double.mpn.biased_exponent > 0 && u_double.mpn.biased_exponent < 2047)
		    {
		      gint exp = u_double.mpn.biased_exponent;

		      exp -= G_IEEE754_DOUBLE_BIAS;
		      exp = exp * G_LOG_2_BASE_10 + 1;
		      conv_len += ABS (exp);	/* exp can be <0 */
		    }
		  /* some printf() implementations require extra padding for rounding */
		  conv_len += 2;
		  /* we can't really handle locale specific grouping here */
		  if (spec.locale_grouping)
		    conv_len *= 2;
		  break;
		case 'C':
		  spec.mod_long = TRUE;
                  /* fall through */
		case 'c':
		  conv_len += spec.mod_long ? MB_LEN_MAX : 1;
		  (void) va_arg (args, int);
		  break;
		case 'S':
		  spec.mod_long = TRUE;
		  /* fall through */
		case 's':
		  v_string = va_arg (args, char*);
		  if (!v_string)
		    conv_len += 8; /* hold "(null)" */
		  else if (spec.seen_precision)
		    conv_len += spec.precision;
		  else
		    conv_len += strlen (v_string);
		  conv_done = TRUE;
		  if (spec.mod_long)
		    {
		      if (may_warn)
			g_warning (G_STRLOC": unable to handle wide char strings");
		      len += 1024; /* try adding some safety padding */
		    }
		  break;
		case 'P': /* do we actually need this? */
		  /* fall through */
		case 'p':
		  spec.alternate_format = TRUE;
		  conv_len += 10;
		  if (honour_longs)
		    conv_len *= 2;
		  /* fall through */
		case 'n':
		  conv_done = TRUE;
		  (void) va_arg (args, void*);
		  break;
		case 'm':
		  /* there's not much we can do to be clever */
		  v_string = g_strerror (errno);
		  v_uint = v_string ? strlen (v_string) : 0;
		  conv_len += MAX (256, v_uint);
		  break;

		  /* handle invalid cases
		   */
		case '\000':
		  /* no conversion specification, bad bad */
		  conv_len += format - spec_start;
		  break;
		default:
		  if (may_warn)
		    g_warning (G_STRLOC": unable to handle `%c' while parsing format",
			       c);
		  break;
		}
	      conv_done |= conv_len > 0;
	    }
	  while (!conv_done);
	  /* handle width specifications */
	  conv_len = MAX (conv_len, MAX (spec.precision, spec.min_width));
	  /* handle flags */
	  conv_len += spec.alternate_format ? 2 : 0;
	  conv_len += (spec.add_space || spec.add_sign || spec.possible_sign);
	  /* finally done */
	  len += conv_len;
	} /* else (c == '%') */
    } /* while (*format) */

  return len;
}



gsize
g_printf_string_upper_bound (const gchar *format,
			     va_list      args)
{
  return printf_string_upper_bound (format, TRUE, args);
}

gchar*
g_get_current_dir (void)
{
  gchar *buffer = NULL;
  gchar *dir = NULL;
  static gulong max_len = 0;

  if (max_len == 0) 
    max_len = (G_PATH_LENGTH == -1) ? 2048 : G_PATH_LENGTH;
  
  /* We don't use getcwd(3) on SUNOS, because, it does a popen("pwd")
   * and, if that wasn't bad enough, hangs in doing so.
   */
#if	(defined (sun) && !defined (__SVR4)) || !defined(HAVE_GETCWD)
  buffer = g_new (gchar, max_len + 1);
  *buffer = 0;
  dir = getwd (buffer);
#else	/* !sun || !HAVE_GETCWD */
  while (max_len < G_MAXULONG / 2)
    {
      buffer = g_new (gchar, max_len + 1);
      *buffer = 0;
      dir = getcwd (buffer, max_len);

      if (dir || errno != ERANGE)
	break;

      g_free (buffer);
      max_len *= 2;
    }
#endif	/* !sun || !HAVE_GETCWD */
  
  if (!dir || !*buffer)
    {
      /* hm, should we g_error() out here?
       * this can happen if e.g. "./" has mode \0000
       */
      buffer[0] = G_DIR_SEPARATOR;
      buffer[1] = 0;
    }

  dir = g_strdup (buffer);
  g_free (buffer);
  
  return dir;
}

gboolean
g_path_is_absolute (const gchar *file_name)
{
  g_return_val_if_fail (file_name != NULL, FALSE);
  
  if (file_name[0] == G_DIR_SEPARATOR
#ifdef G_OS_WIN32
      || file_name[0] == '/'
#endif
				     )
    return TRUE;

#ifdef G_OS_WIN32
  /* Recognize drive letter on native Windows */
  if (g_ascii_isalpha (file_name[0]) && file_name[1] == ':' && (file_name[2] == G_DIR_SEPARATOR || file_name[2] == '/'))
    return TRUE;
#endif /* G_OS_WIN32 */

  return FALSE;
}

/* vim:set ts=8 sw=2 sts=2: */
