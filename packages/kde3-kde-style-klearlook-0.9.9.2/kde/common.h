/* $Id: common.h,v 1.6 2006/04/23 10:40:15 jck Exp $
*/
#ifndef __COMMON_H__
#define __COMMON_H__

/*
 
  Klearlook (C) Joerg C. Koenig, 2005 jck@gmx.org
 
----
 
  Based upon QtCurve (C) Craig Drummond, 2003 Craig.Drummond@lycos.co.uk
      Bernhard Rosenkränzer <bero@r?dh?t.com>
      Preston Brown <pbrown@r?dh?t.com>
      Than Ngo <than@r?dh?t.com>
 
  Released under the GNU General Public License (GPL) v2.
 
  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files(the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:
 
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
 
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.
*/

#define QTC_CHECK_SIZE   13
#define QTC_RADIO_SIZE   13
#define QTC_MIN_BTN_SIZE 10
#define QTC_LV_SIZE(X)   LV_EXP_ARR==(X) ? 7 : 5

#define NUM_SHADES 7

/* Progressbar and selected menu items... */
#define SHADE_GRADIENT_TOP         0.92
#define SHADE_GRADIENT_BOTTOM      1.66
#define SHADE_GRADIENT_LIGHT       1.62
#define SHADE_GRADIENT_DARK        1.05

/* 3d effect - i.e. buttons, etc */
#define QTC_SHADES \
    double shades[2][11][NUM_SHADES]=\
    { \
        { \
          { 1.01, 1.03, 0.868, 0.780, 0.752, 0.74, 1.8 }, \
          { 1.02, 1.03,  0.870, 0.781, 0.753, 0.70, 1.8 }, \
          { 1.03, 1.03, 0.872, 0.782, 0.754, 0.68, 1.8 }, \
          { 1.04, 1.04, 0.875, 0.783, 0.755, 0.64, 1.8 }, \
          { 1.05, 1.04,  0.878, 0.784, 0.756, 0.62, 1.8 }, \
          { 1.06, 1.05, 0.881, 0.785, 0.758, 0.58, 1.8 }, \
          { 1.07, 1.05, 0.884, 0.786, 0.760, 0.54, 1.8 }, \
          { 1.09, 1.06,  0.887, 0.787, 0.762, 0.50, 1.8 }, /* default */ \
          { 1.11, 1.06, 0.890, 0.788, 0.764, 0.45, 1.8 }, \
          { 1.13, 1.07, 0.893, 0.789, 0.766, 0.40, 1.8 }, \
          { 1.15, 1.07,  0.896, 0.790, 0.768, 0.35, 1.8 } \
        }, \
        { \
          { 1.08, 1.03, 0.868, 0.780, 0.800, 0.74, 1.8 }, \
          { 1.09, 1.03,  0.870, 0.781, 0.810, 0.72, 1.8 }, \
          { 1.10, 1.03, 0.872, 0.782, 0.820, 0.70, 1.8 }, \
          { 1.11, 1.04, 0.875, 0.783, 0.840, 0.68, 1.8 }, \
          { 1.12, 1.04,  0.878, 0.784, 0.860, 0.66, 1.8 }, \
          { 1.13, 1.05, 0.881, 0.785, 0.880, 0.64, 1.8 }, \
          { 1.14, 1.05, 0.884, 0.786, 0.900, 0.62, 1.8 }, \
          { 1.15, 1.06,  0.887, 0.787, 0.920, 0.60, 1.8 }, /* default */ \
          { 1.17, 1.06, 0.890, 0.788, 0.764, 0.45, 1.8 }, \
          { 1.19, 1.07, 0.893, 0.789, 0.766, 0.40, 1.8 }, \
          { 1.21, 1.07,  0.896, 0.790, 0.768, 0.35, 1.8 } \
        } \
    };

#define QTC_SHADE(a, c, s) \
    (c>10 || c<0 || s>=NUM_SHADES || s<0 ? 1.0 : shades[APPEARANCE_LIGHT_GRADIENT!=(a) ? 0 : 1][c][s])

/* Shades used when gradient effect is selected */
#define SHADE_BEVEL_GRAD_LIGHT(A)     (APPEARANCE_LIGHT_GRADIENT!=(A) ? 1.0 : 1.0 )
#define SHADE_BEVEL_GRAD_DARK(A)      (APPEARANCE_LIGHT_GRADIENT!=(A) ? 0.75 : 0.75 )
#define SHADE_BEVEL_GRAD_SEL_LIGHT(A) (APPEARANCE_LIGHT_GRADIENT!=(A) ? 1.1 : 1.05 )
#define SHADE_BEVEL_GRAD_SEL_DARK(A)  (APPEARANCE_LIGHT_GRADIENT!=(A) ? 0.95 : 0.95 )

#define SHADE_BEVEL_BUTTON_GRAD_LIGHT(A) 1.00
#define SHADE_BEVEL_BUTTON_GRAD_DARK(A)  0.89

#define SHADE_BEVEL_MENU_GRAD_LIGHT(A) (APPEARANCE_LIGHT_GRADIENT!=(A) ? 0.97 : 0.97 )
#define SHADE_BEVEL_MENU_GRAD_DARK(A)  (APPEARANCE_LIGHT_GRADIENT!=(A) ? 0.93 : 0.93 )

#define SHADE_TAB_LIGHT(A)            (APPEARANCE_LIGHT_GRADIENT!=(A) ? 1.1 : 1.0 )
#define SHADE_TAB_DARK(A)             (APPEARANCE_LIGHT_GRADIENT!=(A) ? 1.0 : 1.0 )
#define SHADE_TAB_SEL_LIGHT(A)        (APPEARANCE_LIGHT_GRADIENT!=(A) ? 1.0 : 1.0 )
#define SHADE_TAB_SEL_DARK(A)         (APPEARANCE_LIGHT_GRADIENT!=(A) ? 1.0 : 1.0 )

#define SHADE_BOTTOM_TAB_LIGHT(A)     (APPEARANCE_LIGHT_GRADIENT!=(A) ? 1.0 : 1.0 )
#define SHADE_BOTTOM_TAB_DARK(A)      (APPEARANCE_LIGHT_GRADIENT!=(A) ? 0.95 : 1.0 )
#define SHADE_BOTTOM_TAB_SEL_LIGHT(A) (APPEARANCE_LIGHT_GRADIENT!=(A) ? 1.0 : 1.0 )
#define SHADE_BOTTOM_TAB_SEL_DARK(A)  (APPEARANCE_LIGHT_GRADIENT!=(A) ? 1.0 : 1.0 )

typedef enum
{
    IND_BORDER,
    IND_CORNER,
    IND_FONT_COLOUR,
    IND_NONE
} EDefBtnIndicator;

typedef enum
{
    GROOVE_RAISED,
    GROOVE_SUNKEN,
    GROOVE_NONE
} EGroove;

typedef enum
{
    TB_NONE,
    TB_LIGHT,
    TB_DARK
} ETBarBorder;

typedef enum
{
    LV_EXP_PM,
    LV_EXP_ARR
} ELvExpander;

typedef enum
{
    LV_LINES_NONE,
    LV_LINES_DOTTED,
    LV_LINES_SOLID
} ELvLines;

typedef enum
{
    PROFILE_FLAT,
    PROFILE_RAISED,
    PROFILE_SUNKEN
} EProfile;

typedef enum
{
    APPEARANCE_FLAT,
    APPEARANCE_GRADIENT,
    APPEARANCE_LIGHT_GRADIENT
} EAppearance;

#define DEF_IND_STR        "corner"
#define DEF_HANDLE_STR     "sunken"
#define DEF_TB_BORDER      "light"
#define DEF_APPEARANCE_STR "lightgradient"
#define DEF_PROFILE_STR    "raised"

#ifdef __cplusplus
extern "C" {
#endif

    extern EDefBtnIndicator qtc_to_ind( const char * str );
    extern EGroove qtc_to_groove( const char * str );
    extern ETBarBorder qtc_to_tbar_border( const char * str );
    extern ELvExpander qtc_to_lv_expander( const char * str );
    extern ELvLines qtc_to_lv_lines( const char * str );
    extern EProfile qtc_to_profile( const char * str );
    extern EAppearance qtc_to_appearance( const char * str );
#ifdef __cplusplus

}
#endif

#endif
