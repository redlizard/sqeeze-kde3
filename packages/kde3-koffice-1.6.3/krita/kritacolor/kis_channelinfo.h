/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_CHANNELINFO_H_
#define KIS_CHANNELINFO_H_

#include <qcolor.h>
#include "qstring.h"
#include "ksharedptr.h"

/** 
 * This class gives some basic information about a channel,
 * that is, one of the components that makes up a particular 
 * pixel.
 */
class KisChannelInfo : public KShared {
public:
    enum enumChannelType {
        COLOR, // The channel represents a color
        ALPHA, // The channel represents the opacity of a pixel
        SUBSTANCE, // The channel represents a real-world substance like pigments or medium
        SUBSTRATE // The channel represents a real-world painting substrate like a canvas
    };
    
    enum enumChannelValueType {
        UINT8,
        UINT16,
        FLOAT16,
        FLOAT32,
        INT8,
        INT16,
        OTHER // Use this if the channel is neither an integer or a float
    };
    enum enumChannelFlags {
        FLAG_COLOR = 1,
        FLAG_ALPHA = (1 << 1),
        FLAG_SUBSTANCE = (1 << 2),
        FLAG_SUBSTRATE = (1 << 3),
        FLAG_COLOR_AND_ALPHA = FLAG_ALPHA | FLAG_COLOR // HACK to be able to use convolution of color and alpha at the same time
    };

public:
    KisChannelInfo() { };
    /**
     * @param name The i18n'ed name of this channel ("Red")
     * @param abbrev A one or two letter abbreviation of the name of this channel ("R")
     * @param npos the position of the first byte of this channel value in the pixel
     * @param channelType the type of this channel (color, alpha, etc)
     * @param channelValueType the datatype of this channel
     * @param size the size in bytes of this channel 
     * @param color a color to visually represent this channel by in the gui
     */
    KisChannelInfo( const QString & name, const QString & abbrev, Q_INT32 npos, enumChannelType channelType, enumChannelValueType channelValueType, Q_INT32 size = 1, QColor color = QColor(0,0,0))
        : m_name (name), m_abbrev(abbrev), m_pos (npos), m_channelType(channelType), m_channelValueType(channelValueType), m_size(size), m_color(color) { };
public:
    /**
     * User-friendly name for this channel for presentation purposes in the gui
     */
    inline QString name() const { return m_name; };

    /**
     * Return the single-letter abbreviation for this channel
     */
    inline QString abbrev() const { return m_abbrev; };
    /** 
     * returns the position of the first byte of the channel in the pixel
     */
    inline Q_INT32 pos() const { return m_pos; };
    
    /**
     * returns the number of bytes this channel takes
     */
    inline Q_INT32 size() const { return m_size; };

    /**
     * returns the type of the channel
     */
    inline enumChannelType channelType() const { return m_channelType; };
    /**
     * return the type of the value of the channel (float, uint8 or uint16)
     */
    inline enumChannelValueType channelValueType() const { return m_channelValueType; };
    /**
     * This is a color that can be used to represent this channel in histograms and so.
     * By default this is black, so keep in mind that many channels might look the same
     */
    inline QColor color() const { return m_color; }

private:

    QString m_name;
    QString m_abbrev;
    Q_INT32 m_pos;
    enumChannelType m_channelType;
    enumChannelValueType m_channelValueType;
    Q_INT32 m_size;
    QColor m_color;

};

#endif // KIS_CHANNELINFO_H_
