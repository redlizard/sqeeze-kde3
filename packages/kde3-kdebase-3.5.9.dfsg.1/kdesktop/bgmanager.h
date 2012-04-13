/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdesktop.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 *
 * You can Freely distribute this program under the GNU General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#ifndef __BGManager_h_Included__
#define __BGManager_h_Included__

#include <qstring.h>
#include <qptrvector.h>

#include <KBackgroundIface.h>

class KConfig;
class QTimer;
class QPixmap;
class KPopupMenu;
class KWinModule;
class KPixmap;
class KVirtualBGRenderer;
class KPixmapServer;

/**
 * Internal struct for KBackgroundManager.
 */
struct KBackgroundCacheEntry
{
    int hash;
    int atime;
    int exp_from;
    KPixmap *pixmap;
};


/**
 * Background manager for KDE. This class is to be used in kdesktop. Usage is
 * very simple: instantiate this class once and the desktop background will
 * be painted automatically from now on.
 *
 * The background manager also has a DCOP interface to remotely control its
 * operation. See KBackgroundIface.h for details.
 */

class KBackgroundManager
    : public QObject,
      virtual public KBackgroundIface
{
    Q_OBJECT

public:
    KBackgroundManager(QWidget *desktop, KWinModule* kwinModule);
    ~KBackgroundManager();

    void configure();
    void setCommon(int);
    bool isCommon() { return m_bCommon; };
    void setExport(int);
    bool isExport() { return m_bExport; };
    void setCache(int, int);
    void setWallpaper(int desk, QString wallpaper, int mode);
    void setWallpaper(QString wallpaper, int mode);
    void setWallpaper(QString wallpaper);
    void changeWallpaper();
    QString currentWallpaper(int desk);
    void setColor(const QColor & c, bool isColorA = true);
    void setBackgroundEnabled(const bool enable);
    QStringList wallpaperList(int desk);
    QStringList wallpaperFiles(int desk);

signals:
    void initDone();

private slots:
    void slotTimeout();
    void slotImageDone(int desk);
    void slotChangeDesktop(int);
    void slotChangeNumberOfDesktops(int);
    void repaintBackground();
    void desktopResized();
    void clearRoot();
    void saveImages();
    
private:
    void applyCommon(bool common);
    void applyExport(bool _export);
    void applyCache(bool limit, int size);

    int realDesktop();
    int effectiveDesktop();
    int validateDesk(int desk);

    void renderBackground(int desk);
    void exportBackground(int pixmap, int desk);
    int pixmapSize(QPixmap *pm);
    int cacheSize();
    void removeCache(int desk);
    bool freeCache(int size);
    void addCache(KPixmap *pm, int hash, int desk);
    void setPixmap(KPixmap *pm, int hash, int desk);

    bool m_bExport, m_bCommon;
    bool m_bLimitCache, m_bInit;
    bool m_bBgInitDone;
    bool m_bEnabled;

    int m_CacheLimit;
    int m_Serial, m_Hash, m_Current;

    KConfig *m_pConfig;
    QWidget *m_pDesktop;
    QTimer *m_pTimer;

    QPtrVector<KVirtualBGRenderer> m_Renderer;
    QPtrVector<KBackgroundCacheEntry> m_Cache;

    KWinModule *m_pKwinmodule;
    KPixmapServer *m_pPixmapServer;
    
    unsigned long m_xrootpmap;
};

#endif // __BGManager_h_Included__
