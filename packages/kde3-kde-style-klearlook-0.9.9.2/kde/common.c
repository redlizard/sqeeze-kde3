/*
Klearlook (C) Craig Drummond, 2003 Craig.Drummond@lycos.co.uk

----

Released under the GNU General Public License (GPL) v2.

----

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

#include <string.h>
#include "common.h"

#define gdouble double

EDefBtnIndicator qtc_to_ind( const char *str ) {
    if ( 0 == memcmp( str, "fontcolour", 10 ) )
        return IND_FONT_COLOUR;
    if ( 0 == memcmp( str, "border", 6 ) )
        return IND_BORDER;
    if ( 0 == memcmp( str, "none", 4 ) )
        return IND_NONE;
    return IND_CORNER;
}

EGroove qtc_to_groove( const char *str ) {
    if ( 0 == memcmp( str, "raised", 6 ) )
        return GROOVE_RAISED;
    if ( 0 == memcmp( str, "none", 4 ) )
        return GROOVE_NONE;
    return GROOVE_SUNKEN;
}

ETBarBorder qtc_to_tbar_border( const char *str ) {
    if ( 0 == memcmp( str, "dark", 4 ) )
        return TB_DARK;
    if ( 0 == memcmp( str, "none", 4 ) )
        return TB_NONE;
    if ( 0 == memcmp( str, "light", 5 ) )
        return TB_LIGHT;
    return TB_LIGHT;
}

ELvExpander qtc_to_lv_expander( const char *str ) {
    return 0 == memcmp( str, "arrow", 5 ) ? LV_EXP_ARR : LV_EXP_PM;
}

ELvLines qtc_to_lv_lines( const char *str ) {
    if ( 0 == memcmp( str, "none", 4 ) )
        return LV_LINES_NONE;
    if ( 0 == memcmp( str, "dotted", 6 ) )
        return LV_LINES_DOTTED;
    return LV_LINES_SOLID;
}

EProfile qtc_to_profile( const char *str ) {
    if ( 0 == memcmp( str, "flat", 4 ) )
        return PROFILE_FLAT;
    if ( 0 == memcmp( str, "raised", 6 ) )
        return PROFILE_RAISED;
    return PROFILE_SUNKEN;
}

EAppearance qtc_to_appearance( const char *str ) {
    if ( 0 == memcmp( str, "flat", 4 ) )
        return APPEARANCE_FLAT;
    if ( 0 == memcmp( str, "gradient", 8 ) )
        return APPEARANCE_GRADIENT;
    return APPEARANCE_LIGHT_GRADIENT;
}

