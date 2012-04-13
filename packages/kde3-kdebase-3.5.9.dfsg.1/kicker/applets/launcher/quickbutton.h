/*****************************************************************

Copyright (c) 2000 Bill Nagel

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef __quickbutton_h__
#define __quickbutton_h__

#include <qbutton.h>
#include <qpoint.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qcursor.h>

#include <kickertip.h>
#include <kicontheme.h>
#include <kmimetype.h>
#include <kpanelapplet.h>
#include <kservice.h>
#include <kurl.h>

class QPopupMenu;
class KAction;
class KToggleAction;

class QuickURL {
public:
    QuickURL(const QString &u);
    KURL kurl() const {return _kurl;};
    QString url() const {return _kurl.url();};
    QString menuId() const  {return _menuId;};
    QString genericName() const { return m_genericName; }
    QString name() const { return m_name; }
    KService::Ptr service() const {return _service;};
    void run() const;
    QPixmap pixmap(mode_t _mode = 0, KIcon::Group _group = KIcon::Desktop,
                   int _force_size = 0, int _state = 0, QString * _path = 0L) const;

private:
    KURL _kurl;
    QString _menuId;
    QString m_genericName;
    QString m_name;
    KService::Ptr _service;
};


class QuickButton: public QButton, public KickerTip::Client {
    Q_OBJECT

public:
    enum { DEFAULT_ICON_DIM = 16 };
    enum { ICON_MARGIN = 1 };
    QuickButton(const QString &u, KAction* configAction,
                QWidget *parent=0, const char *name=0);
    ~QuickButton();
    QString url() const;
    QString menuId() const;
    QPixmap icon() const{ return _icon;}
    bool sticky() { return m_sticky; }
    void setSticky(bool bSticky);
    void setPopupDirection(KPanelApplet::Direction d);

    void setDragging(bool drag);
    void setEnableDrag(bool enable);
    void setDynamicModeEnabled(bool enabled);
    void flash();

signals:
    void removeApp(QuickButton *);
    void executed(QString serviceStorageID);
    void stickyToggled(bool isSticky);

protected:
    void paintEvent(QPaintEvent *);
    void drawButton(QPainter *p);
    void drawButtonLabel(QPainter *p);
    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void resizeEvent(QResizeEvent *rsevent);
    void loadIcon();
    void updateKickerTip(KickerTip::Data &data);

protected slots:
    void settingsChanged(int);
    void iconChanged(int);
    void launch();
    void removeApp();
    void slotFlash();
    void slotStickyToggled(bool isSticky);

private:
    int m_flashCounter;
    QuickURL *_qurl;
    QPoint _dragPos;
    QPopupMenu *_popup;
    QPixmap _icon, _iconh;
    QCursor _oldCursor;
    bool _highlight, _changeCursorOverItem, _dragEnabled;
    int _iconDim;
    bool m_sticky;
    KToggleAction *m_stickyAction;
    int m_stickyId;
    KPanelApplet::Direction m_popupDirection;
};

#endif

