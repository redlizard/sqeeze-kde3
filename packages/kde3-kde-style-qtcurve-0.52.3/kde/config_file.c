/*
  QtCurve (C) Craig Drummond, 2003 - 2007 Craig.Drummond@lycos.co.uk

  ----

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public
  License version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; see the file COPYING.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
 */

#include "common.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <sys/types.h>

#define QTC_MAX_FILENAME_LEN   1024
#define QTC_MAX_INPUT_LINE_LEN 256
#define QTC_FILE               "qtcurvestylerc"

#ifdef CONFIG_READ
static int c2h(char ch)
{
    return (ch>='0' && ch<='9') ? ch-'0' :
           (ch>='a' && ch<='f') ? 10+(ch-'a') :
           (ch>='A' && ch<='F') ? 10+(ch-'A') :
           0;
}

#define ATOH(str) ((c2h(*str)<<4)+c2h(*(str+1)))

static void setRgb(color *col, const char *str)
{
    if(str && strlen(str)>6)
    {
        int offset='#'==str[0] ? 1 : 0;
#ifdef __cplusplus
        col->setRgb(ATOH(&str[offset]), ATOH(&str[offset+2]), ATOH(&str[offset+4]));
#else
        col->red=ATOH(&str[offset])<<8;
        col->green=ATOH(&str[offset+2])<<8;
        col->blue=ATOH(&str[offset+4])<<8;
        col->pixel=0;
#endif
    }
    else
#ifdef __cplusplus
        col->setRgb(0, 0, 0);
#else
        col->red=col->green=col->blue=col->pixel=0;
#endif
}

static EDefBtnIndicator toInd(const char *str, EDefBtnIndicator def)
{
    if(str)
    {
        if(0==memcmp(str, "fontcolor", 9) || 0==memcmp(str, "border", 6))
            return IND_FONT_COLOR;
        if(0==memcmp(str, "none", 4))
            return IND_NONE;
        if(0==memcmp(str, "corner", 6))
            return IND_CORNER;
        if(0==memcmp(str, "colored", 7))
            return IND_COLORED;
    }

    return def;
}

static ELine toLine(const char *str, ELine def)
{
    if(str)
    {
        if(0==memcmp(str, "dashes", 6))
            return LINE_DASHES;
        if(0==memcmp(str, "none", 4))
            return LINE_NONE;
        if(0==memcmp(str, "sunken", 6))
            return LINE_SUNKEN;
        if(0==memcmp(str, "dots", 4))
            return LINE_DOTS;
    }
    return def;
}

static ETBarBorder toTBarBorder(const char *str, ETBarBorder def)
{
    if(str)
    {
        if(0==memcmp(str, "dark", 4))
            return 0==memcmp(&str[4], "-all", 4) ? TB_DARK_ALL : TB_DARK;
        if(0==memcmp(str, "none", 4))
            return TB_NONE;
        if(0==memcmp(str, "light", 5))
            return 0==memcmp(&str[5], "-all", 4) ? TB_LIGHT_ALL : TB_LIGHT;
    }
    return def;
}

static EMouseOver toMouseOver(const char *str, EMouseOver def)
{
    if(str)
    {
        if(0==memcmp(str, "true", 4) || 0==memcmp(str, "colored", 7))
            return MO_COLORED;
        if(0==memcmp(str, "plastik", 7))
            return MO_PLASTIK;
        if(0==memcmp(str, "false", 4) || 0==memcmp(str, "none", 4))
            return MO_NONE;
    }
    return def;
}

static EAppearance toAppearance(const char *str, EAppearance def)
{
    if(str)
    {
        if(0==memcmp(str, "flat", 4))
            return APPEARANCE_FLAT;
        if(0==memcmp(str, "raised", 6))
            return APPEARANCE_RAISED;
        if(0==memcmp(str, "gradient", 8) || 0==memcmp(str, "lightgradient", 13))
            return APPEARANCE_GRADIENT;
        if(0==memcmp(str, "glass", 5) || 0==memcmp(str, "shinyglass", 10))
            return APPEARANCE_SHINY_GLASS;
        if(0==memcmp(str, "dullglass", 9))
            return APPEARANCE_DULL_GLASS;
        if(0==memcmp(str, "inverted", 8))
            return APPEARANCE_INVERTED;
        if(0==memcmp(str, "bevelled", 8))
            return APPEARANCE_BEVELLED;
    }
    return def;
}

static EShade toShade(const char *str, bool allowDarken, EShade def)
{
    if(str)
    {
        /* true/false is from 0.25... */
        if(0==memcmp(str, "true", 4) || 0==memcmp(str, "selected", 8))
            return SHADE_BLEND_SELECTED;
        if(0==memcmp(str, "origselected", 12))
            return SHADE_SELECTED;
        if(allowDarken && 0==memcmp(str, "darken", 6))
            return SHADE_DARKEN;
        if(0==memcmp(str, "custom", 6))
            return SHADE_CUSTOM;
        if(0==memcmp(str, "none", 4))
            return SHADE_NONE;
    }

    return def;
}

/* Prior to 0.42 round was a bool - so need to read 'false' as 'none' */
static ERound toRound(const char *str, ERound def)
{
    if(str)
    {
        if(0==memcmp(str, "none", 4) || 0==memcmp(str, "false", 5))
            return ROUND_NONE;
        if(0==memcmp(str, "slight", 6))
            return ROUND_SLIGHT;
        if(0==memcmp(str, "full", 4))
            return ROUND_FULL;
    }

    return def;
}

static EScrollbar toScrollbar(const char *str, EScrollbar def)
{
    if(str)
    {
        if(0==memcmp(str, "kde", 3))
            return SCROLLBAR_KDE;
        if(0==memcmp(str, "windows", 7))
            return SCROLLBAR_WINDOWS;
        if(0==memcmp(str, "platinum", 8))
            return SCROLLBAR_PLATINUM;
        if(0==memcmp(str, "next", 4))
            return SCROLLBAR_NEXT;
        if(0==memcmp(str, "none", 4))
            return SCROLLBAR_NONE;
    }

    return def;
}

#ifndef QTC_CONFIG_DIALOG
static EShading toShading(const char * str, EShading def)
{
    if(str)
    {
        if(0==memcmp(str, "simple", 6))
            return SHADING_SIMPLE;
        if(0==memcmp(str, "hsl", 3))
            return SHADING_HSL;
        if(0==memcmp(str, "hsv", 3))
            return SHADING_HSV;
    }

    return def;
}
#endif

#endif

#ifdef CONFIG_WRITE
#include <kstandarddirs.h>
#endif

static const char * getHome()
{
    static const char *home=NULL;

    if(!home)
    {
        struct passwd *p=getpwuid(getuid());

        if(p)
            home=p->pw_dir;
        else
        {
            char *env=getenv("HOME");

            if(env)
                home=env;
        }

        if(!home)
            home="/tmp";
    }

    return home;
}

static const char *xdgConfigFolder()
{
    static char xdgDir[QTC_MAX_FILENAME_LEN]={'\0'};

    if(!xdgDir[0])
    {
        static const char *home=NULL;

#if 0
        char *env=getenv("XDG_CONFIG_HOME");

        /*
            Check the setting of XDG_CONFIG_HOME
            For some reason, sudo leaves the env vars set to those of the
            caller - so XDG_CONFIG_HOME would point to the users setting, and
            not roots.

            Therefore, check that home is first part of XDG_CONFIG_HOME
        */

        if(env && 0==getuid())
        {
            if(!home)
                home=getHome();
            if(home && home!=strstr(env, home))
                env=NULL;
        }
#else
        /*
           Hmm... for 'root' dont bother to check env var, just set to ~/.config
           - as problems would arise if "sudo kcmshell style", and then
           "sudo su" / "kcmshell style". The 1st would write to ~/.config, but
           if root has a XDG_ set then that would be used on the second :-(
        */
        char *env=0==getuid() ? NULL : getenv("XDG_CONFIG_HOME");
#endif

        if(!env)
        {
            if(!home)
                home=getHome();

            sprintf(xdgDir, "%s/.config", home);
        }
        else
            strcpy(xdgDir, env);

#ifdef CONFIG_WRITE
#ifdef __cplusplus
        KStandardDirs::makeDir(xdgDir);
#endif
#endif
    }

    return xdgDir;
}

#ifdef CONFIG_READ

#ifdef __cplusplus
#define QTC_IS_BLACK(A) (0==(A).red() && 0==(A).green() && 0==(A).blue())
#else
#define QTC_IS_BLACK(A) (0==(A).red && 0==(A).green && 0==(A).blue)
#endif

static void checkColor(EShade *s, color *c)
{
    if(SHADE_CUSTOM==*s && QTC_IS_BLACK(*c))
        *s=SHADE_NONE;
}

#ifdef __cplusplus

#if QT_VERSION >= 0x040000
#include <QMap>
#include <QFile>
#include <QTextStream>
#define QTC_LATIN1(A) A.toLatin1()
#else
#define QTC_LATIN1(A) A.latin1()

#include <qmap.h>
#include <qfile.h>
#include <qtextstream.h>
#endif

class QtCConfig
{
    public:

    QtCConfig(const QString &filename);

    bool            ok() const { return values.count()>0; }
    const QString & readEntry(const char *key, const QString &def=QString::null);

    private:

    QMap<QString, QString> values;
};

QtCConfig::QtCConfig(const QString &filename)
{
    QFile f(filename);

#if QT_VERSION >= 0x040000
    if(f.open(QIODevice::ReadOnly))
#else
    if(f.open(IO_ReadOnly))
#endif
    {
        QTextStream stream(&f);
        QString     line;

        while(!stream.atEnd())
        {
            line = stream.readLine();
#if QT_VERSION >= 0x040000
            int pos=line.indexOf('=');
#else
            int pos=line.find('=');
#endif
            if(-1!=pos)
                values[line.left(pos)]=line.mid(pos+1);
        }
        f.close();
    }
}

inline const QString & QtCConfig::readEntry(const char *key, const QString &def)
{
    return values.contains(key) ? values[key] : def;
}

inline QString readStringEntry(QtCConfig &cfg, const char *key)
{
    return cfg.readEntry(key);
}

static int readNumEntry(QtCConfig &cfg, const char *key, int def)
{
    const QString &val(readStringEntry(cfg, key));

    return val.isEmpty() ? def : val.toInt();
}

static bool readBoolEntry(QtCConfig &cfg, const char *key, bool def)
{
    const QString &val(readStringEntry(cfg, key));

    return val.isEmpty() ? def : (val=="true" ? true : false);
}

#if QT_VERSION >= 0x040000
#define QTC_LATIN1(A) A.toLatin1()
#else
#define QTC_LATIN1(A) A.latin1()
#endif

#define QTC_CFG_READ_COLOR(ENTRY) \
    { \
        QString sVal(cfg.readEntry(#ENTRY)); \
        if(sVal.isEmpty()) \
            opts->ENTRY=def->ENTRY; \
        else \
            setRgb(&(opts->ENTRY), QTC_LATIN1(sVal)); \
    }

#else

static char * lookupCfgHash(GHashTable **cfg, char *key, char *val)
{
    char *rv=NULL;

    if(!*cfg)
        *cfg=g_hash_table_new(g_str_hash, g_str_equal);
    else
        rv=(char *)g_hash_table_lookup(*cfg, key);

    if(!rv && val)
    {
        g_hash_table_insert(*cfg, g_strdup(key), g_strdup(val));
        rv=(char *)g_hash_table_lookup(*cfg, key);
    }

    return rv;
}

static GHashTable * loadConfig(const char *filename)
{
    FILE       *f=fopen(filename, "r");
    GHashTable *cfg=NULL;

    if(f)
    {
        char line[QTC_MAX_INPUT_LINE_LEN];

        while(NULL!=fgets(line, QTC_MAX_INPUT_LINE_LEN-1, f))
        {
            char *eq=strchr(line, '=');
            int  pos=eq ? eq-line : -1;

            if(pos>0)
            {
                char *endl=strchr(line, '\n');

                if(endl)
                    *endl='\0';

                line[pos]='\0';

                lookupCfgHash(&cfg, line, &line[pos+1]);
            }
        }

        fclose(f);
    }

    return cfg;
}

static void releaseConfig(GHashTable *cfg)
{
    g_hash_table_destroy(cfg);
}

static char * readStringEntry(GHashTable *cfg, char *key)
{
    return lookupCfgHash(&cfg, key, NULL);
}

static int readNumEntry(GHashTable *cfg, char *key, int def)
{
    char *str=readStringEntry(cfg, key);

    return str ? atoi(str) : def;
}

static gboolean readBoolEntry(GHashTable *cfg, char *key, gboolean def)
{
    char *str=readStringEntry(cfg, key);

    return str ? (0==memcmp(str, "true", 4) ? true : false) : def;
}

#define QTC_LATIN1(A) A

#define QTC_CFG_READ_COLOR(ENTRY) \
    { \
        const char *str=readStringEntry(cfg, #ENTRY); \
    \
        if(str) \
            setRgb(&(opts->ENTRY), str); \
        else \
            opts->ENTRY=def->ENTRY; \
    }
#endif

#define QTC_CFG_READ_BOOL(ENTRY) \
    opts->ENTRY=readBoolEntry(cfg, #ENTRY, def->ENTRY);

#define QTC_CFG_READ_ROUND(ENTRY) \
    opts->ENTRY=toRound(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def->ENTRY);

#define QTC_CFG_READ_DI(ENTRY) \
    opts->ENTRY=((double)(readNumEntry(cfg, #ENTRY, ((int)(def->ENTRY*100))-100)+100))/100.0;

#define QTC_CFG_READ_TB_BORDER(ENTRY) \
    opts->ENTRY=toTBarBorder(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def->ENTRY);

#define QTC_CFG_READ_MOUSE_OVER(ENTRY) \
    opts->ENTRY=toMouseOver(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def->ENTRY);

#define QTC_CFG_READ_APPEARANCE(ENTRY, DEF) \
    opts->ENTRY=toAppearance(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), DEF);

/*
#define QTC_CFG_READ_APPEARANCE(ENTRY) \
    opts->ENTRY=toAppearance(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def->ENTRY);
*/

#define QTC_CFG_READ_DEF_BTN(ENTRY) \
    opts->ENTRY=toInd(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def->ENTRY);

#define QTC_CFG_READ_LINE(ENTRY) \
    opts->ENTRY=toLine(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def->ENTRY);

#define QTC_CFG_READ_SHADE(ENTRY, AD) \
    opts->ENTRY=toShade(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), AD, def->ENTRY);

#define QTC_CFG_READ_SCROLLBAR(ENTRY) \
    opts->ENTRY=toScrollbar(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), def->ENTRY);

#define QTC_CFG_READ_SHADING(ENTRY, DEF) \
    ENTRY=toShading(QTC_LATIN1(readStringEntry(cfg, #ENTRY)), DEF);

#ifdef __cplusplus
static bool readConfig(const QString &file, Options *opts, Options *def)
#else
static bool readConfig(const char *file, Options *opts, Options *def)
#endif
{
#ifdef __cplusplus
    if(file.isEmpty())
    {
        const char *xdg=xdgConfigFolder();

        if(xdg)
        {
            QString filename(xdg);

            filename+="/"QTC_FILE;
            return readConfig(filename, opts, def);
        }
    }
#else
    if(!file)
    {
        const char *xdg=xdgConfigFolder();

        if(xdg)
        {
            char filename[QTC_MAX_FILENAME_LEN];

            sprintf(filename, "%s/"QTC_FILE, xdg);
            return readConfig(filename, opts, def);
        }
    }
#endif
    else
    {
#ifdef __cplusplus
        QtCConfig cfg(file);

        if(cfg.ok())
        {
#else
        GHashTable *cfg=loadConfig(file);

        if(cfg)
        {
#endif
            QTC_CFG_READ_ROUND(round)
            QTC_CFG_READ_DI(highlightFactor)
            QTC_CFG_READ_TB_BORDER(toolbarBorders)
            QTC_CFG_READ_APPEARANCE(appearance, def->appearance)
            QTC_CFG_READ_BOOL(fixParentlessDialogs)
            QTC_CFG_READ_BOOL(stripedProgress)
            QTC_CFG_READ_BOOL(animatedProgress)
            QTC_CFG_READ_BOOL(lighterPopupMenuBgnd)
            QTC_CFG_READ_BOOL(embolden)
            QTC_CFG_READ_DEF_BTN(defBtnIndicator)
            QTC_CFG_READ_LINE(sliderThumbs)
            QTC_CFG_READ_LINE(handles)
            QTC_CFG_READ_BOOL(highlightTab)
            QTC_CFG_READ_SHADE(shadeSliders, false)
            QTC_CFG_READ_SHADE(shadeMenubars, true)
            QTC_CFG_READ_SHADE(shadeCheckRadio, false)
            QTC_CFG_READ_APPEARANCE(menubarAppearance, def->menubarAppearance)
            QTC_CFG_READ_APPEARANCE(menuitemAppearance, opts->appearance)
            QTC_CFG_READ_APPEARANCE(toolbarAppearance, def->toolbarAppearance)
            QTC_CFG_READ_LINE(toolbarSeparators)
            QTC_CFG_READ_LINE(splitters)
            QTC_CFG_READ_BOOL(customMenuTextColor)
            QTC_CFG_READ_MOUSE_OVER(coloredMouseOver)
            QTC_CFG_READ_BOOL(menubarMouseOver)
            QTC_CFG_READ_BOOL(shadeMenubarOnlyWhenActive)
            QTC_CFG_READ_BOOL(thinnerMenuItems)
            QTC_CFG_READ_COLOR(customSlidersColor)
            QTC_CFG_READ_COLOR(customMenubarsColor)
            QTC_CFG_READ_COLOR(customMenuSelTextColor)
            QTC_CFG_READ_COLOR(customMenuNormTextColor)
            QTC_CFG_READ_COLOR(customCheckRadioColor)
            QTC_CFG_READ_SCROLLBAR(scrollbarType)
            QTC_CFG_READ_BOOL(shadowButtons)
            QTC_CFG_READ_APPEARANCE(lvAppearance, opts->appearance)
            QTC_CFG_READ_APPEARANCE(tabAppearance, opts->appearance)
            QTC_CFG_READ_APPEARANCE(sliderAppearance, opts->appearance)
            QTC_CFG_READ_APPEARANCE(progressAppearance, opts->appearance)
#ifndef QTC_PLAIN_FOCUS_ONLY
            QTC_CFG_READ_BOOL(stdFocus)
#endif
            QTC_CFG_READ_BOOL(lvLines)
            QTC_CFG_READ_BOOL(drawStatusBarFrames)
            QTC_CFG_READ_BOOL(fillSlider)
            QTC_CFG_READ_BOOL(roundMbTopOnly)
            QTC_CFG_READ_BOOL(borderMenuitems)
            QTC_CFG_READ_BOOL(gradientPbGroove)
            QTC_CFG_READ_BOOL(darkerBorders)
            QTC_CFG_READ_BOOL(vArrows)
            QTC_CFG_READ_BOOL(colorMenubarMouseOver)
#ifdef __cplusplus
            QTC_CFG_READ_BOOL(stdSidebarButtons)
            QTC_CFG_READ_BOOL(gtkScrollViews);
            QTC_CFG_READ_BOOL(gtkComboMenus);
#endif

#ifndef QTC_COMMON_ONLY_COVERTERS
            QTC_CFG_READ_SHADING(shading, SHADING_HSL);
#endif

#ifndef __cplusplus
            releaseConfig(cfg);
#endif
            if(SHADE_SELECTED==opts->shadeCheckRadio)
                opts->shadeCheckRadio=SHADE_BLEND_SELECTED;

            checkColor(&opts->shadeMenubars, &opts->customMenubarsColor);
            checkColor(&opts->shadeSliders, &opts->customSlidersColor);
            checkColor(&opts->shadeCheckRadio, &opts->customCheckRadioColor);

            if(APPEARANCE_BEVELLED==opts->toolbarAppearance)
                opts->toolbarAppearance=APPEARANCE_GRADIENT;
            else if(APPEARANCE_RAISED==opts->toolbarAppearance)
                opts->toolbarAppearance=APPEARANCE_FLAT;

            if(APPEARANCE_BEVELLED==opts->menubarAppearance)
                opts->menubarAppearance=APPEARANCE_GRADIENT;
            else if(APPEARANCE_RAISED==opts->menubarAppearance)
                opts->menubarAppearance=APPEARANCE_FLAT;

            if(APPEARANCE_BEVELLED==opts->sliderAppearance)
                opts->sliderAppearance=APPEARANCE_GRADIENT;

            if(APPEARANCE_BEVELLED==opts->tabAppearance)
                opts->tabAppearance=APPEARANCE_GRADIENT;

            if(opts->highlightFactor<((100.0+MIN_HIGHLIGHT_FACTOR)/100.0) ||
               opts->highlightFactor>((100.0+MAX_HIGHLIGHT_FACTOR)/100.0))
                opts->highlightFactor=DEFAULT_HIGHLIGHT_FACTOR;

            if(opts->animatedProgress && !opts->stripedProgress)
                opts->animatedProgress=false;

            if(SHADE_CUSTOM==opts->shadeMenubars || SHADE_BLEND_SELECTED==opts->shadeMenubars || !opts->borderMenuitems)
                opts->colorMenubarMouseOver=true;

            return true;
        }
    }

    return false;
}

static bool fileExists(const char *path)
{
    struct stat info;

    return 0==lstat(path, &info) && (info.st_mode&S_IFMT)==S_IFREG;
}

static const char * getSystemConfigFile()
{
    static const char * constFiles[]={ "/etc/qt4/"QTC_FILE, "/etc/qt3/"QTC_FILE, "/etc/qt/"QTC_FILE, NULL };

    int i;

    for(i=0; constFiles[i]; ++i)
        if(fileExists(constFiles[i]))
            return constFiles[i];
    return NULL;
}

static void defaultSettings(Options *opts)
{
    /* Set hard-coded defaults... */
    opts->contrast=7;
    opts->highlightFactor=DEFAULT_HIGHLIGHT_FACTOR;
    opts->round=ROUND_FULL;
    opts->lighterPopupMenuBgnd=true;
    opts->animatedProgress=true;
    opts->stripedProgress=true;
    opts->highlightTab=true;
    opts->embolden=false;
    opts->appearance=APPEARANCE_DULL_GLASS;
    opts->lvAppearance=APPEARANCE_BEVELLED;
    opts->tabAppearance=APPEARANCE_GRADIENT;
    opts->sliderAppearance=APPEARANCE_DULL_GLASS;
    opts->menubarAppearance=APPEARANCE_GRADIENT;
    opts->menuitemAppearance=APPEARANCE_DULL_GLASS;
    opts->toolbarAppearance=APPEARANCE_GRADIENT;
    opts->progressAppearance=APPEARANCE_DULL_GLASS;
    opts->defBtnIndicator=IND_COLORED;
    opts->sliderThumbs=LINE_DOTS;
    opts->handles=LINE_DOTS;
    opts->shadeSliders=SHADE_BLEND_SELECTED;
    opts->shadeMenubars=SHADE_DARKEN;
    opts->shadeCheckRadio=SHADE_NONE;
    opts->toolbarBorders=TB_NONE;
    opts->toolbarSeparators=LINE_DOTS;
    opts->splitters=LINE_DOTS;
    opts->fixParentlessDialogs=false;
    opts->customMenuTextColor=false;
    opts->coloredMouseOver=MO_PLASTIK;
    opts->menubarMouseOver=true;
    opts->shadeMenubarOnlyWhenActive=true;
    opts->thinnerMenuItems=false;
    opts->scrollbarType=SCROLLBAR_KDE;
    opts->shadowButtons=true;
#ifndef QTC_PLAIN_FOCUS_ONLY
    opts->stdFocus=true;
#endif
    opts->lvLines=false;
    opts->drawStatusBarFrames=false;
    opts->fillSlider=true;
    opts->roundMbTopOnly=true;
    opts->borderMenuitems=true;
    opts->gradientPbGroove=true;
    opts->darkerBorders=false;
    opts->vArrows=false;
    opts->colorMenubarMouseOver=false;
#ifdef __cplusplus
    opts->stdSidebarButtons=false;
    opts->gtkScrollViews=false;
    opts->gtkComboMenus=false;
    opts->customMenubarsColor.setRgb(0, 0, 0);
    opts->customSlidersColor.setRgb(0, 0, 0);
    opts->customMenuNormTextColor.setRgb(0, 0, 0);
    opts->customMenuSelTextColor.setRgb(0, 0, 0);
    opts->customCheckRadioColor.setRgb(0, 0, 0);
#else
    opts->customMenubarsColor.red=opts->customMenubarsColor.green=opts->customMenubarsColor.blue=0;
    opts->customSlidersColor.red=opts->customSlidersColor.green=opts->customSlidersColor.blue=0;
    opts->customMenuNormTextColor.red=opts->customMenuNormTextColor.green=opts->customMenuNormTextColor.blue=0;
    opts->customMenuSelTextColor.red=opts->customMenuSelTextColor.green=opts->customMenuSelTextColor.blue=0;
    opts->customCheckRadioColor.red=opts->customCheckRadioColor.green=opts->customCheckRadioColor.blue=0;
#endif

    /* Read system config file... */
    {
    static const char * systemFilename=NULL;

    if(!systemFilename)
        systemFilename=getSystemConfigFile();

    if(systemFilename)
        readConfig(systemFilename, opts, opts);
    }
}

#endif

#ifdef CONFIG_WRITE
static const char *toStr(EDefBtnIndicator ind)
{
    switch(ind)
    {
        case IND_NONE:
            return "none";
        case IND_FONT_COLOR:
            return "fontcolor";
        case IND_CORNER:
            return "corner";
        default:
            return "colored";
    }
}

static const char *toStr(ELine ind, bool none)
{
    switch(ind)
    {
        case LINE_DOTS:
            return "dots";
        case LINE_DASHES:
            return none ? "none" : "dashes";
        default:
            return "sunken";
    }
}

static const char *toStr(ETBarBorder ind)
{
    switch(ind)
    {
        case TB_DARK:
            return "dark";
        case TB_DARK_ALL:
            return "dark-all";
        case TB_LIGHT_ALL:
            return "light-all";
        case TB_NONE:
            return "none";
        default:
            return "light";
    }
}

static const char *toStr(EMouseOver mo)
{
    switch(mo)
    {
        case MO_COLORED:
            return "colored";
        case MO_NONE:
            return "none";
        default:
            return "plastik";
    }
}

static const char *toStr(EAppearance exp)
{
    switch(exp)
    {
        case APPEARANCE_FLAT:
            return "flat";
        case APPEARANCE_RAISED:
            return "raised";
        case APPEARANCE_GRADIENT:
            return "gradient";
        case APPEARANCE_DULL_GLASS:
            return "dullglass";
        case APPEARANCE_BEVELLED:
            return "bevelled";
        case APPEARANCE_INVERTED:
            return "inverted";
        default:
            return "shinyglass";
    }
}

static const char *toStr(EShade exp, bool dark, bool convertBlendSelToSel)
{
    switch(exp)
    {
        default:
        case SHADE_NONE:
            return "none";
        case SHADE_BLEND_SELECTED:
            return dark || !convertBlendSelToSel ? "selected" : "origselected";
        case SHADE_CUSTOM:
            return "custom";
        /* case SHADE_SELECTED */
        case SHADE_DARKEN:
            return dark ? "darken" : "origselected";
    }
}

static const char *toStr(ERound exp)
{
    switch(exp)
    {
        case ROUND_NONE:
            return "none";
        case ROUND_SLIGHT:
            return "slight";
        default:
        case ROUND_FULL:
            return "full";
    }
}

static const char *toStr(EScrollbar sb)
{
    switch(sb)
    {
        case SCROLLBAR_KDE:
            return "kde";
        default:
        case SCROLLBAR_WINDOWS:
            return "windows";
        case SCROLLBAR_PLATINUM:
            return "platinum";
        case SCROLLBAR_NEXT:
            return "next";
        case SCROLLBAR_NONE:
            return "none";
    }
}

inline const char * toStr(bool b) { return b ? "true" : "false"; }

static QString toStr(const QColor &col)
{
    QString colorStr;

    colorStr.sprintf("#%02X%02X%02X", col.red(), col.green(), col.blue());
    return colorStr;
}

#if QT_VERSION >= 0x040000
#define CFG config
#else
#define CFG (*cfg)
#endif

#define CFG_WRITE_ENTRY(ENTRY) \
    if (!exportingStyle && def.ENTRY==opts.ENTRY) \
        CFG.deleteEntry(#ENTRY); \
    else \
        CFG.writeEntry(#ENTRY, toStr(opts.ENTRY));

#define CFG_WRITE_ENTRY_FORCE(ENTRY) \
        CFG.writeEntry(#ENTRY, toStr(opts.ENTRY));

#define CFG_WRITE_ENTRY_B(ENTRY, B) \
    if (!exportingStyle && def.ENTRY==opts.ENTRY) \
        CFG.deleteEntry(#ENTRY); \
    else \
        CFG.writeEntry(#ENTRY, toStr(opts.ENTRY, B));

#define CFG_WRITE_ENTRY_SHADE(ENTRY, DARK, CONVERT_SHADE) \
    if (!exportingStyle && def.ENTRY==opts.ENTRY) \
        CFG.deleteEntry(#ENTRY); \
    else \
        CFG.writeEntry(#ENTRY, toStr(opts.ENTRY, DARK, CONVERT_SHADE));

#define CFG_WRITE_ENTRY_D(ENTRY) \
    if (!exportingStyle && def.ENTRY==opts.ENTRY) \
        CFG.deleteEntry(#ENTRY); \
    else \
        CFG.writeEntry(#ENTRY, ((int)(opts.ENTRY*100))-100);

bool static writeConfig(KConfig *cfg, const Options &opts, const Options &def, bool exportingStyle=false)
{
    if(!cfg)
    {
        const char *xdg=xdgConfigFolder();

        if(xdg)
        {
            char filename[QTC_MAX_FILENAME_LEN];

            sprintf(filename, "%s/"QTC_FILE, xdg);

#if QT_VERSION >= 0x040000
            KConfig defCfg(filename, KConfig::OnlyLocal);
#else
            KConfig defCfg(filename, false, false);
#endif

            return writeConfig(&defCfg, opts, def, exportingStyle);
        }
    }
    else
    {
#if QT_VERSION >= 0x040000
        KConfigGroup config(cfg, QTC_GROUP);
#else
        cfg->setGroup(QTC_GROUP);
#endif
        CFG_WRITE_ENTRY(round)
        CFG_WRITE_ENTRY_D(highlightFactor)
        CFG_WRITE_ENTRY(toolbarBorders)
        CFG_WRITE_ENTRY_FORCE(appearance)
        CFG_WRITE_ENTRY(fixParentlessDialogs)
        CFG_WRITE_ENTRY(stripedProgress)
        CFG_WRITE_ENTRY(animatedProgress)
        CFG_WRITE_ENTRY(lighterPopupMenuBgnd)
        CFG_WRITE_ENTRY(embolden)
        CFG_WRITE_ENTRY(defBtnIndicator)
        CFG_WRITE_ENTRY_B(sliderThumbs, true)
        CFG_WRITE_ENTRY_B(handles, false)
        CFG_WRITE_ENTRY(highlightTab)
        CFG_WRITE_ENTRY_SHADE(shadeSliders, false, false)
        CFG_WRITE_ENTRY_SHADE(shadeMenubars, true, false)
        CFG_WRITE_ENTRY_SHADE(shadeCheckRadio, false, true)
        CFG_WRITE_ENTRY_FORCE(menubarAppearance)
        CFG_WRITE_ENTRY_FORCE(menuitemAppearance)
        CFG_WRITE_ENTRY_FORCE(toolbarAppearance)
        CFG_WRITE_ENTRY_B(toolbarSeparators, true)
        CFG_WRITE_ENTRY_B(splitters, false)
        CFG_WRITE_ENTRY(customMenuTextColor)
        CFG_WRITE_ENTRY(coloredMouseOver)
        CFG_WRITE_ENTRY(menubarMouseOver)
        CFG_WRITE_ENTRY(shadeMenubarOnlyWhenActive)
        CFG_WRITE_ENTRY(thinnerMenuItems)
        CFG_WRITE_ENTRY(customSlidersColor)
        CFG_WRITE_ENTRY(customMenubarsColor)
        CFG_WRITE_ENTRY(customMenuSelTextColor)
        CFG_WRITE_ENTRY(customMenuNormTextColor)
        CFG_WRITE_ENTRY(customCheckRadioColor)
        CFG_WRITE_ENTRY(scrollbarType)
        CFG_WRITE_ENTRY(shadowButtons)
        CFG_WRITE_ENTRY_FORCE(lvAppearance)
        CFG_WRITE_ENTRY_FORCE(tabAppearance)
        CFG_WRITE_ENTRY_FORCE(sliderAppearance)
        CFG_WRITE_ENTRY_FORCE(progressAppearance)
#ifndef QTC_PLAIN_FOCUS_ONLY
        CFG_WRITE_ENTRY(stdFocus)
#endif
        CFG_WRITE_ENTRY(lvLines)
        CFG_WRITE_ENTRY(drawStatusBarFrames)
        CFG_WRITE_ENTRY(fillSlider)
        CFG_WRITE_ENTRY(roundMbTopOnly)
        CFG_WRITE_ENTRY(borderMenuitems)
        CFG_WRITE_ENTRY(gradientPbGroove)
        CFG_WRITE_ENTRY(darkerBorders)
        CFG_WRITE_ENTRY(vArrows)
#ifdef __cplusplus
        CFG_WRITE_ENTRY(stdSidebarButtons)
#endif
        cfg->sync();
        return true;
    }
    return false;
}
#endif

