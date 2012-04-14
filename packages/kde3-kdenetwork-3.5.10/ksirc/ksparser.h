/* This file is part of the KDE project
   Copyright (C) 2001 Simon Hausmann <hausmann@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the Artistic License.
*/
#ifndef __ksparser_h__
#define __ksparser_h__

#include <qstring.h>
#include <qmap.h>
#include <qcolor.h>
#include <qvaluestack.h>

// Helper class to parse IRC colour/style codes and convert them to
// richtext. The parser maintains an internal stack of the styles
// applied because the IRC message could contain sequences as
// (bold)Hello (red)World(endbold)! (blue)blue text
// which needs to be converted to
// <b>Hello </b><font color="red"><b>World</b>! </font><font color="blue">blue text</font>
// to get correctly nested tags. (malte)
class KSParser
{
public:
    QString parse(const QString &);
    bool beeped() const { return m_beeped; }

private:
    QString pushTag(const QString &, const QString & = QString::null);
    QString popTag(const QString &);
    QString toggleTag(const QString &, const QString & = QString::null);
    QString popAll();
    QColor ircColor(int);

private:
    QValueStack<QString> m_tags;
    QMap<QString, QString> m_attributes;
    bool m_beeped;
};

#endif


