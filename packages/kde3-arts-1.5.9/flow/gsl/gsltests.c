/* GSL - Generic Sound Layer
 * Copyright (C) 2001-2002 Tim Janik and Stefan Westerfeld
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include <gsl/gslmath.h>
#include <gsl/gslcommon.h>
#include <gsl/gslmath.h>
#include <gsl/gslfilter.h>
#include <gsl/gslloader.h>
#include <stdlib.h>
#include <string.h>

#define PREC    "15"


static void	usage (void)	G_GNUC_NORETURN;


static guint         shift_argc = 0;
static const gchar **shift_argv = NULL;

static const gchar*
shift (void)
{
  const gchar *arg;

  if (shift_argc > 1)
    {
      shift_argc--;
      arg = shift_argv++[1];
      if (!arg)
	arg = "";
    }
  else
    arg = NULL;
  return arg;
}
static const gchar*
pshift (void)
{
  const gchar *arg = shift ();

  return arg ? arg : "";
}

int
main (int   argc,
      char *argv[])
{
  const gchar *arg;

  /* iir filter parameters */
  enum { FILTER_GNUPLOT, FILTER_SCAN } filter_mode = FILTER_GNUPLOT;
  const gchar *filter_label = 0;
  gdouble *a, *b;
  guint order = 0;

  shift_argc = argc;
  shift_argv = (gchar **)argv;

  if (!g_thread_supported ())
      g_thread_init (NULL);
  gsl_init (NULL, NULL);

  arg = shift ();
  if (!arg)
    usage ();

 restart:
  a = b = 0;

  if (strcmp (arg, "wave-scan") == 0)
    {
      const gchar *file = pshift ();

      while (file)
	{
	  GslWaveFileInfo *fi;
	  GslErrorType error;

	  fi = gsl_wave_file_info_load (file, &error);
	  if (fi)
	    {
	      guint i;

	      g_print ("Loader \"%s\" found %u waves in \"%s\":\n", fi->loader->name, fi->n_waves, file);
	      for (i = 0; i < fi->n_waves; i++)
		g_print ("%u) %s\n", i + 1, fi->waves[i].name);
	      gsl_wave_file_info_unref (fi);
	    }
	  else
	    g_print ("Failed to scan \"%s\": %s\n", file, gsl_strerror (error));
	  file = pshift ();
	  if (!file[0])
	    break;
	}
    }
  else if (strcmp (arg, "file-test") == 0)
    {
      const gchar *file = pshift ();

      g_print ("file test for \"%s\":\n", file);
      g_print ("  is readable   : %s\n", gsl_strerror (gsl_check_file (file, "r")));
      g_print ("  is writable   : %s\n", gsl_strerror (gsl_check_file (file, "w")));
      g_print ("  is executable : %s\n", gsl_strerror (gsl_check_file (file, "x")));
      g_print ("  is file       : %s\n", gsl_strerror (gsl_check_file (file, "f")));
      g_print ("  is directory  : %s\n", gsl_strerror (gsl_check_file (file, "d")));
      g_print ("  is link       : %s\n", gsl_strerror (gsl_check_file (file, "l")));
    }
  else if (strcmp (arg, "rf") == 0)
    {
      double x, y, z;
      x = atof (pshift ());
      y = atof (pshift ());
      z = atof (pshift ());

      g_print ("rf(%f, %f, %f) = %."PREC"f\n", x, y, z, gsl_ellip_rf (x, y, z));
    }
  else if (strcmp (arg, "F") == 0)
    {
      double phi, ak;
      phi = atof (pshift ());
      ak = atof (pshift ());

      g_print ("F(%f, %f) = %."PREC"f\n", phi, ak, gsl_ellip_F (phi, ak));
    }
  else if (strcmp (arg, "sn") == 0)
    {
      double u, emmc;
      u = atof (pshift ());
      emmc = atof (pshift ());

      g_print ("sn(%f, %f) = %."PREC"f\n", u, emmc, gsl_ellip_sn (u, emmc));
    }
  else if (strcmp (arg, "snc") == 0)
    {
      GslComplex u, emmc;
      u.re = atof (pshift ());
      u.im = atof (pshift ());
      emmc.re = atof (pshift ());
      emmc.im = atof (pshift ());

      g_print ("snc(%s, %s) = %s\n",
	       gsl_complex_str (u),
	       gsl_complex_str (emmc),
	       gsl_complex_str (gsl_complex_ellip_sn (u, emmc)));
    }
  else if (strcmp (arg, "sci_snc") == 0)
    {
      GslComplex u, k2;
      u.re = atof (pshift ());
      u.im = atof (pshift ());
      k2.re = atof (pshift ());
      k2.im = atof (pshift ());

      g_print ("sci_snc(%s, %s) = %s\n",
	       gsl_complex_str (u),
	       gsl_complex_str (k2),
	       gsl_complex_str (gsl_complex_ellip_sn (u, gsl_complex_sub (gsl_complex (1.0, 0), k2))));
    }
  else if (strcmp (arg, "asn") == 0)
    {
      double y, emmc;
      y = atof (pshift ());
      emmc = atof (pshift ());

      g_print ("asn(%f, %f) = %."PREC"f\n", y, emmc, gsl_ellip_asn (y, emmc));
    }
  else if (strcmp (arg, "asnc") == 0)
    {
      GslComplex y, emmc;
      y.re = atof (pshift ());
      y.im = atof (pshift ());
      emmc.re = atof (pshift ());
      emmc.im = atof (pshift ());

      g_print ("asnc(%s, %s) = %s\n",
	       gsl_complex_str (y), gsl_complex_str (emmc),
	       gsl_complex_str (gsl_complex_ellip_asn (y, emmc)));
      g_print ("asn(%f, %f = %."PREC"f\n",
	       y.re, emmc.re, gsl_ellip_asn (y.re, emmc.re));
    }
  else if (strcmp (arg, "sci_sn") == 0)
    {
      double u, k2;
      u = atof (pshift ());
      k2 = atof (pshift ());
      g_print ("sci_sn(%f, %f) = %."PREC"f\n", u, k2, gsl_ellip_sn (u, 1.0 - k2));
    }
  else if (strcmp (arg, "sci_asn") == 0)
    {
      double y, k2;
      y = atof (pshift ());
      k2 = atof (pshift ());
      g_print ("sci_asn(%f, %f) = %."PREC"f\n", y, k2, gsl_ellip_asn (y, 1.0 - k2));
    }
  else if (strcmp (arg, "sci_asnc") == 0)
    {
      GslComplex y, k2;
      y.re = atof (pshift ());
      y.im = atof (pshift ());
      k2.re = atof (pshift ());
      k2.im = atof (pshift ());
      g_print ("sci_asnc(%s, %s) = %s\n",
	       gsl_complex_str (y), gsl_complex_str (k2),
	       gsl_complex_str (gsl_complex_ellip_asn (y, gsl_complex_sub (gsl_complex (1.0, 0), k2))));
      g_print ("asn(%f, %f = %."PREC"f\n",
	       y.re, k2.re, gsl_ellip_asn (y.re, 1.0 - k2.re));
    }
  else if (strcmp (arg, "sin") == 0)
    {
      GslComplex phi;
      phi.re = atof (pshift ());
      phi.im = atof (pshift ());
      g_print ("sin(%s) = %s\n",
	       gsl_complex_str (phi),
	       gsl_complex_str (gsl_complex_sin (phi)));
    }
  else if (strcmp (arg, "cos") == 0)
    {
      GslComplex phi;
      phi.re = atof (pshift ());
      phi.im = atof (pshift ());
      g_print ("cos(%s) = %s\n",
	       gsl_complex_str (phi),
	       gsl_complex_str (gsl_complex_cos (phi)));
    }
  else if (strcmp (arg, "tan") == 0)
    {
      GslComplex phi;
      phi.re = atof (pshift ());
      phi.im = atof (pshift ());
      g_print ("tan(%s) = %s\n",
	       gsl_complex_str (phi),
	       gsl_complex_str (gsl_complex_tan (phi)));
    }
  else if (strcmp (arg, "sinh") == 0)
    {
      GslComplex phi;
      phi.re = atof (pshift ());
      phi.im = atof (pshift ());
      g_print ("sinh(%s) = %s\n",
	       gsl_complex_str (phi),
	       gsl_complex_str (gsl_complex_sinh (phi)));
    }
  else if (strcmp (arg, "cosh") == 0)
    {
      GslComplex phi;
      phi.re = atof (pshift ());
      phi.im = atof (pshift ());
      g_print ("cosh(%s) = %s\n",
	       gsl_complex_str (phi),
	       gsl_complex_str (gsl_complex_cosh (phi)));
    }
  else if (strcmp (arg, "tanh") == 0)
    {
      GslComplex phi;
      phi.re = atof (pshift ());
      phi.im = atof (pshift ());
      g_print ("tanh(%s) = %s\n",
	       gsl_complex_str (phi),
	       gsl_complex_str (gsl_complex_tanh (phi)));
    }
  else if (strcmp (arg, "midi2freq") == 0)
    {
      gint note;
      note = atol (pshift ());
      note = CLAMP (note, 0, 128);
      g_print ("midi2freq(%u) = %f\n",
	       note,
	       gsl_temp_freq (gsl_get_config ()->kammer_freq,
			      note - gsl_get_config ()->midi_kammer_note));
    }
  else if (strcmp (arg, "blp") == 0)
    {
      double f, e;
      order = atoi (pshift ()); order = MAX (order, 1);
      f = atof (pshift ());
      e = atof (pshift ());
      f *= GSL_PI / 2.;
      a = g_new (gdouble, order + 1);
      b = g_new (gdouble, order + 1);
      gsl_filter_butter_lp (order, f, e, a, b);
      g_print ("# Lowpass Butterworth filter order=%u freq=%f epsilon(s^2)=%f norm0=%f:\n",
	       order, f, e,
	       gsl_poly_eval (order, a, 1) / gsl_poly_eval (order, b, 1));
      filter_label = "BL";
    }
  else if (strcmp (arg, "bhp") == 0)
    {
      double f, e;
      order = atoi (pshift ()); order = MAX (order, 1);
      f = atof (pshift ());
      e = atof (pshift ());
      f *= GSL_PI / 2.;
      a = g_new (gdouble, order + 1);
      b = g_new (gdouble, order + 1);

      gsl_filter_butter_hp (order, f, e, a, b);
      g_print ("# Highpass Butterworth filter order=%u freq=%f epsilon(s^2)=%f norm0=%f:\n",
	       order, f, e,
	       gsl_poly_eval (order, a, 1) / gsl_poly_eval (order, b, 1));
      filter_label = "BH";
    }
  else if (strcmp (arg, "bbp") == 0)
    {
      double f1, f2, e;
      order = atoi (pshift ()); order = MAX (order, 1);
      f1 = atof (pshift ());
      f2 = atof (pshift ());
      e = atof (pshift ());
      f1 *= GSL_PI / 2.;
      f2 *= GSL_PI / 2.;
      a = g_new (gdouble, order + 1);
      b = g_new (gdouble, order + 1);

      gsl_filter_butter_bp (order, f1, f2, e, a, b);
      g_print ("# Bandpass Butterworth filter order=%u freq1=%f freq2=%f epsilon(s^2)=%f norm0=%f:\n",
	       order, f1, f2, e,
	       gsl_poly_eval (order, a, 1) / gsl_poly_eval (order, b, 1));
      filter_label = "BP";
    }
  else if (strcmp (arg, "bbs") == 0)
    {
      double f1, f2, e;
      order = atoi (pshift ()); order = MAX (order, 1);
      f1 = atof (pshift ());
      f2 = atof (pshift ());
      e = atof (pshift ());
      f1 *= GSL_PI / 2.;
      f2 *= GSL_PI / 2.;
      a = g_new (gdouble, order + 1);
      b = g_new (gdouble, order + 1);

      gsl_filter_butter_bs (order, f1, f2, e, a, b);
      g_print ("# Bandstop Butterworth filter order=%u freq1=%f freq2=%f epsilon(s^2)=%f norm0=%f:\n",
	       order, f1, f2, e,
	       gsl_poly_eval (order, a, 1) / gsl_poly_eval (order, b, 1));
      filter_label = "BS";
    }
  else if (strcmp (arg, "t1l") == 0)
    {
      double f, e;
      order = atoi (pshift ()); order = MAX (order, 1);
      f = atof (pshift ());
      e = atof (pshift ());
      f *= GSL_PI / 2.;
      a = g_new (gdouble, order + 1);
      b = g_new (gdouble, order + 1);

      gsl_filter_tscheb1_lp (order, f, e, a, b);
      g_print ("# Lowpass Tschebyscheff Type1 order=%u freq=%f epsilon(s^2)=%f norm0=%f:\n",
	       order, f, e,
	       gsl_poly_eval (order, a, 1) / gsl_poly_eval (order, b, 1));
      filter_label = "T1L";
    }
  else if (strcmp (arg, "t1h") == 0)
    {
      double f, e;
      order = atoi (pshift ()); order = MAX (order, 1);
      f = atof (pshift ());
      e = atof (pshift ());
      f *= GSL_PI / 2.;

      a = g_new (gdouble, order + 1);
      b = g_new (gdouble, order + 1);

      gsl_filter_tscheb1_hp (order, f, e, a, b);
      g_print ("# Highpass Tschebyscheff Type1 order=%u freq=%f epsilon(s^2)=%f norm0=%f:\n",
	       order, f, e,
	       gsl_poly_eval (order, a, 1) / gsl_poly_eval (order, b, 1));
      filter_label = "T1H";
    }
  else if (strcmp (arg, "t1s") == 0)
    {
      double fc, fr, e;
      order = atoi (pshift ()); order = MAX (order, 1);
      fc = atof (pshift ());
      fr = atof (pshift ());
      e = atof (pshift ());
      fc *= GSL_PI / 2.;
      fr *= GSL_PI / 2.;

      a = g_new (gdouble, order + 1);
      b = g_new (gdouble, order + 1);

      gsl_filter_tscheb1_bs (order, fc, fr, e, a, b);
      g_print ("# Bandstop Tschebyscheff Type1 order=%u freq_c=%f freq_r=%f epsilon(s^2)=%f norm=%f:\n",
	       order, fc, fr, e,
	       gsl_poly_eval (order, a, 1) / gsl_poly_eval (order, b, 1));
      filter_label = "T1S";
    }
  else if (strcmp (arg, "t1p") == 0)
    {
      double fc, fr, e;
      order = atoi (pshift ()); order = MAX (order, 1);
      fc = atof (pshift ());
      fr = atof (pshift ());
      e = atof (pshift ());
      fc *= GSL_PI / 2.;
      fr *= GSL_PI / 2.;

      a = g_new (gdouble, order + 1);
      b = g_new (gdouble, order + 1);

      gsl_filter_tscheb1_bp (order, fc, fr, e, a, b);
      g_print ("# Bandpass Tschebyscheff Type1 order=%u freq_c=%f freq_r=%f epsilon(s^2)=%f norm=%f:\n",
	       order, fc, fr, e,
	       gsl_poly_eval (order, a, 1) / gsl_poly_eval (order, b, 1));
      filter_label = "T1P";
    }
  else if (strcmp (arg, "t2l") == 0)
    {
      double f, st, e;
      order = atoi (pshift ()); order = MAX (order, 1);
      f = atof (pshift ());
      st = atof (pshift ());
      e = atof (pshift ());
      f *= GSL_PI / 2.;

      a = g_new (gdouble, order + 1);
      b = g_new (gdouble, order + 1);

      gsl_filter_tscheb2_lp (order, f, st, e, a, b);
      g_print ("# Lowpass Tschebyscheff Type2 order=%u freq=%f steepness=%f (%f) epsilon(s^2)=%f norm=%f:\n",
	       order, f, st, f * (1.+st), e,
	       gsl_poly_eval (order, a, 1) / gsl_poly_eval (order, b, 1));
      filter_label = "T2L";
    }
  else if (strcmp (arg, "t2h") == 0)
    {
      double f, st, e;
      order = atoi (pshift ()); order = MAX (order, 1);
      f = atof (pshift ());
      st = atof (pshift ());
      e = atof (pshift ());
      f *= GSL_PI / 2.;

      a = g_new (gdouble, order + 1);
      b = g_new (gdouble, order + 1);

      gsl_filter_tscheb2_hp (order, f, st, e, a, b);
      g_print ("# Highpass Tschebyscheff Type2 order=%u freq=%f steepness=%f (%f, %f) epsilon(s^2)=%f norm=%f:\n",
	       order, f, st, GSL_PI - f, (GSL_PI - f) * (1.+st), e,
	       gsl_poly_eval (order, a, 1) / gsl_poly_eval (order, b, 1));
      filter_label = "T2H";
    }
  else if (strcmp (arg, "t2p") == 0)
    {
      double f1, f2, st, e;
      order = atoi (pshift ()); order = MAX (order, 1);
      f1 = atof (pshift ());
      f2 = atof (pshift ());
      st = atof (pshift ());
      e = atof (pshift ());
      f1 *= GSL_PI / 2.;
      f2 *= GSL_PI / 2.;

      a = g_new (gdouble, order + 1);
      b = g_new (gdouble, order + 1);

      gsl_filter_tscheb2_bp (order, f1, f2, st, e, a, b);
      g_print ("# Bandpass Tschebyscheff Type2 order=%u freq1=%f freq2=%f steepness=%f epsilon(s^2)=%f norm=%f:\n",
	       order, f1, f2, st, e,
	       gsl_poly_eval (order, a, 1) / gsl_poly_eval (order, b, 1));
      filter_label = "T2P";
    }
  else if (strcmp (arg, "t2s") == 0)
    {
      double f1, f2, st, e;
      order = atoi (pshift ()); order = MAX (order, 1);
      f1 = atof (pshift ());
      f2 = atof (pshift ());
      st = atof (pshift ());
      e = atof (pshift ());
      f1 *= GSL_PI / 2.;
      f2 *= GSL_PI / 2.;

      a = g_new (gdouble, order + 1);
      b = g_new (gdouble, order + 1);

      gsl_filter_tscheb2_bs (order, f1, f2, st, e, a, b);
      g_print ("# Bandstop Tschebyscheff Type2 order=%u freq1=%f freq2=%f steepness=%f epsilon(s^2)=%f norm=%f:\n",
	       order, f1, f2, st, e,
	       gsl_poly_eval (order, a, 1) / gsl_poly_eval (order, b, 1));
      filter_label = "T2S";
    }
  else if (strcmp (arg, "scan") == 0)
    {
      filter_mode = FILTER_SCAN;
    }
  else if (strcmp (arg, "fir") == 0)
    {
      unsigned int iorder = atoi (pshift ());
      unsigned int n_points = 0;

      double *freq = g_newa (double, argc / 2 + 1);
      double *value = g_newa (double, argc / 2 + 1);
      double *a = g_newa (double, iorder);
      const char *f, *v;

      do
	{
	  f = pshift ();
	  v = pshift ();

	  if (f[0] && v[0])
	    {
	      freq[n_points] = atof (f) * GSL_PI;
	      value[n_points] = atof (v);
	      n_points++;
	    }
	}
      while (f[0] && v[0]);

      gsl_filter_fir_approx (iorder, a, n_points, freq, value);
      g_print ("FIR%u(z)=%s\n", iorder, gsl_poly_str (iorder, a, "z"));
    }
  else if (strncmp (arg, "poly", 4) == 0)
    {
      guint order;
      arg = arg + 4;
      order = 2;
      {
	double a[100] = { 1, 2, 1 }, b[100] = { 1, -3./2., 0.5 };
	g_print ("# Test order=%u  norm=%f:\n",
		 order,
		 gsl_poly_eval (order, a, 1) / gsl_poly_eval (order, b, 1));
	g_print ("H%u(z)=%s/%s\n", order,
		 gsl_poly_str (order, a, "z"),
		 gsl_poly_str (order, b, "z"));
	if (*arg)
	  {
	    GslComplex root, roots[100];
	    guint i;

	    if (*arg == 'r')
	      {
		g_print ("#roots:\n");
		gsl_poly_complex_roots (order, a, roots);
		for (i = 0; i < order; i++)
		  {
		    root = gsl_complex_div (gsl_complex (1, 0), roots[i]);
		    g_print ("%+.14f %+.14f # %.14f\n", root.re, root.im, gsl_complex_abs (root));
		  }
	      }
	    if (*arg == 'p')
	      {
		g_print ("#poles:\n");
		gsl_poly_complex_roots (order, b, roots);
		for (i = 0; i < order; i++)
		  {
		    root = gsl_complex_div (gsl_complex (1, 0), roots[i]);
		    g_print ("%+.14f %+.14f # %.14f\n", root.re, root.im, gsl_complex_abs (root));
		  }
	      }
	  }
      }
    }
  else
    usage ();

  if (a && b)
    {
      gdouble freq;

      if (filter_mode == FILTER_SCAN)
	{
	  freq = 0.001;
	  while (freq < 3.14)
	    {
	      g_print ("%f %.20f\n", freq, gsl_filter_sine_scan (order, a, b, freq, MAX((int)(1000.0/freq),10000)));
	      freq = MIN (freq * 1.1, freq + 0.01);
	    }
	}
      else if (filter_mode == FILTER_GNUPLOT)
	{
	  g_print ("%s%u(z)=%s/%s\n", filter_label, order,
		   gsl_poly_str (order, a, "z"),
		   gsl_poly_str (order, b, "z"));
	}
      else
	g_error ("unknown filter_mode");
      g_free (a);
      g_free (b);
    }

  arg = shift ();
  if (arg)
    goto restart;

  return 0;
}

static void
usage (void)
{
  g_print ("usage: gsltests {test} [args...]\n");
  g_print ("tests:\n");
  g_print ("  wave-scan <file>          scan a wave file for waves\n");
  g_print ("  file-test <file>          test file properties\n");
  g_print ("  rf <x> <y> <z>            Carlson's elliptic integral of the first kind\n");
  g_print ("  F <phi> <ak>              Legendre elliptic integral of the 1st kind\n");
  g_print ("  sn <u> <emmc>             Jacobian elliptic function sn()\n");
  g_print ("  asn <y> <emmc>            elliptic integral, inverse sn()\n");
  g_print ("  sin <phi.re> <phi.im>     complex sine\n");
  g_print ("  cos <phi.re> <phi.im>     complex cosine\n");
  g_print ("  tan <phi.re> <phi.im>     complex tangent\n");
  g_print ("  sinh <phi.re> <phi.im>    complex hyperbolic sine\n");
  g_print ("  cosh <phi.re> <phi.im>    complex hyperbolic cosine\n");
  g_print ("  tanh <phi.re> <phi.im>    complex hyperbolic tangent\n");
  g_print ("  midi2freq <midinote>      convert midinote into oscilaltor frequency\n");
  g_print ("  snc <u.re> <u.im> <emmc.re> <emmc.im>     sn() for complex numbers\n");
  g_print ("  asnc <y.re> <y.im> <emmc.re> <emmc.im>    asn() for complex numbers\n");
  g_print ("  sci_sn <u> <k2>                           scilab version of sn()\n");
  g_print ("  sci_asn <y> <k2>                          scilab version of asn()\n");
  g_print ("  sci_snc <u.re> <u.im> <k2.re> <k2.im>     scilab version of snc()\n");
  g_print ("  sci_asnc <y.re> <y.im> <k2.re> <k2.im>    scilab version of asnc()\n");
  g_print ("  blp <order> <freq> <epsilon>              butterworth lowpass filter\n");
  g_print ("  bhp <order> <freq> <epsilon>              butterworth higpass filter\n");
  g_print ("  bbp <order> <freqc> <freqr> <epsilon>     butterworth bandpass filter\n");
  g_print ("  t1l <order> <freq> <epsilon>              type1 tschebyscheff lowpass filter\n");
  g_print ("  t1h <order> <freq> <epsilon>              type1 tschebyscheff highpass filter\n");
  g_print ("  t1s <order> <freqc> <freqr> <epsilon>     type1 tschebyscheff bandstop filter\n");
  g_print ("  t1p <order> <freqc> <freqr> <epsilon>     type1 tschebyscheff bandpass filter\n");
  g_print ("  t2l <order> <freqc> <steepn> <epsilon>    type2 tschebyscheff lowpass filter\n");
  g_print ("  t2h <order> <freqc> <steepn> <epsilon>    type2 tschebyscheff highpass filter\n");
  g_print ("  fir <order> <freq1> <value1> ...          fir approximation\n");
  g_print ("  scan blp <order> <freq> <epsilon>         scan butterworth lowpass filter\n");
  g_print ("  poly | polyr | polyp                      polynom test (+roots or +poles)\n");
  exit (1);
}



/* vim:set ts=8 sts=2 sw=2: */
