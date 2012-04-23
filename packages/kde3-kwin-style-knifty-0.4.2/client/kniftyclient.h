/***************************************************************************
*   Copyright (C) 2003 by Sandro Giessl                                   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/

#ifndef KNIFTYCLIENT_H
#define KNIFTYCLIENT_H

#include "knifty.h"
#include <qlayout.h>
#include <qstring.h>

class QSpacerItem;
class QVBoxLayout;

namespace KNifty {

class KniftyButton;

class KniftyClient : public KDecoration
{
    Q_OBJECT
public:
    KniftyClient (KDecorationBridge *b, KDecorationFactory *f);
    ~KniftyClient();

    virtual void init(void);

    QPixmap getTitleBarTile(bool active) const
    {
        return active ? *aTitleBarTile : *iTitleBarTile;
    }
protected:
    virtual void resizeEvent(QResizeEvent *e);
    virtual void paintEvent(QPaintEvent *e);
    virtual void showEvent(QShowEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *e);

    virtual void doShape();
    virtual void borders (int &l, int &r, int &t, int &b) const;
    virtual void resize(const QSize& size);
    virtual void captionChange ();
    virtual void maximizeChange ();
    virtual void shadeChange ();
    virtual bool isTool() const;

    virtual void desktopChange();
    virtual void iconChange();
    virtual void activeChange();
    virtual QSize minimumSize() const;
    virtual Position mousePosition(const QPoint &point) const;

private slots:
    void maxButtonPressed();
    void menuButtonPressed();
    void menuButtonReleased();
private:
    bool eventFilter (QObject *obj, QEvent *e);
    void _resetLayout();
    void addButtons(QBoxLayout* layout, const QString& buttons, int buttonSize = 18);

    QVBoxLayout *mainLayout_;
    QSpacerItem *topSpacer_,
                *titleSpacer_,
                *leftTitleSpacer_, *rightTitleSpacer_,
                *decoSpacer_,
                *leftSpacer_, *rightSpacer_,
                *bottomSpacer_;

    QPixmap *aCaptionBuffer, *iCaptionBuffer;
    void update_captionBuffer();

    QPixmap *aTitleBarTile, *iTitleBarTile;
    bool pixmaps_created;
    void create_pixmaps();
    void delete_pixmaps();

    KniftyButton *m_button[ButtonTypeCount];

    bool captionBufferDirty;
    bool closing;

    // settings...
    int   s_titleHeight;
    QFont s_titleFont;
};

}

#endif // KNIFTCLIENT_H
