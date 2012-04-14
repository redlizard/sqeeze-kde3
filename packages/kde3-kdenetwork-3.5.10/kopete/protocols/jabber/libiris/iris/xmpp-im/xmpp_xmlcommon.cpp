/*
 * xmlcommon.cpp - helper functions for dealing with XML
 * Copyright (C) 2001, 2002  Justin Karneges
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include"xmpp_xmlcommon.h"

#include <qstring.h>
#include <qdom.h>
#include <qdatetime.h>
#include <qsize.h>
#include <qrect.h>
#include <qstringlist.h>
#include <qcolor.h>

#include"im.h"

bool stamp2TS(const QString &ts, QDateTime *d)
{
	if(ts.length() != 17)
		return false;

	int year  = ts.mid(0,4).toInt();
	int month = ts.mid(4,2).toInt();
	int day   = ts.mid(6,2).toInt();

	int hour  = ts.mid(9,2).toInt();
	int min   = ts.mid(12,2).toInt();
	int sec   = ts.mid(15,2).toInt();

	QDate xd;
	xd.setYMD(year, month, day);
	if(!xd.isValid())
		return false;

	QTime xt;
	xt.setHMS(hour, min, sec);
	if(!xt.isValid())
		return false;

	d->setDate(xd);
	d->setTime(xt);

	return true;
}

QString TS2stamp(const QDateTime &d)
{
	QString str;

	str.sprintf("%04d%02d%02dT%02d:%02d:%02d",
		d.date().year(),
		d.date().month(),
		d.date().day(),
		d.time().hour(),
		d.time().minute(),
		d.time().second());

	return str;
}

QDomElement textTag(QDomDocument *doc, const QString &name, const QString &content)
{
	QDomElement tag = doc->createElement(name);
	QDomText text = doc->createTextNode(content);
	tag.appendChild(text);

	return tag;
}

QString tagContent(const QDomElement &e)
{
	// look for some tag content
	for(QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
		QDomText i = n.toText();
		if(i.isNull())
			continue;
		return i.data();
	}

	return "";
}

QDomElement findSubTag(const QDomElement &e, const QString &name, bool *found)
{
	if(found)
		*found = false;

	for(QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
		QDomElement i = n.toElement();
		if(i.isNull())
			continue;
		if(i.tagName() == name) {
			if(found)
				*found = true;
			return i;
		}
	}

	QDomElement tmp;
	return tmp;
}

QDomElement createIQ(QDomDocument *doc, const QString &type, const QString &to, const QString &id)
{
	QDomElement iq = doc->createElement("iq");
	if(!type.isEmpty())
		iq.setAttribute("type", type);
	if(!to.isEmpty())
		iq.setAttribute("to", to);
	if(!id.isEmpty())
		iq.setAttribute("id", id);

	return iq;
}

QDomElement queryTag(const QDomElement &e)
{
	bool found;
	QDomElement q = findSubTag(e, "query", &found);
	return q;
}

QString queryNS(const QDomElement &e)
{
	bool found;
	QDomElement q = findSubTag(e, "query", &found);
	if(found)
		return q.attribute("xmlns");

	return "";
}

void getErrorFromElement(const QDomElement &e, int *code, QString *str)
{
	bool found;
	QDomElement tag = findSubTag(e, "error", &found);
	if(!found)
		return;

	if(code)
		*code = tag.attribute("code").toInt();
	if(str)
		*str = tagContent(tag);
}

//----------------------------------------------------------------------------
// XMLHelper
//----------------------------------------------------------------------------

namespace XMLHelper {

QDomElement emptyTag(QDomDocument *doc, const QString &name)
{
	QDomElement tag = doc->createElement(name);

	return tag;
}

bool hasSubTag(const QDomElement &e, const QString &name)
{
	bool found;
	findSubTag(e, name, &found);
	return found;
}

QString subTagText(const QDomElement &e, const QString &name)
{
	bool found;
	QDomElement i = findSubTag(e, name, &found);
	if ( found )
		return i.text();
	return QString::null;
}

QDomElement textTag(QDomDocument &doc, const QString &name, const QString &content)
{
	QDomElement tag = doc.createElement(name);
	QDomText text = doc.createTextNode(content);
	tag.appendChild(text);

	return tag;
}

QDomElement textTag(QDomDocument &doc, const QString &name, int content)
{
	QDomElement tag = doc.createElement(name);
	QDomText text = doc.createTextNode(QString::number(content));
	tag.appendChild(text);

	return tag;
}

QDomElement textTag(QDomDocument &doc, const QString &name, bool content)
{
	QDomElement tag = doc.createElement(name);
	QDomText text = doc.createTextNode(content ? "true" : "false");
	tag.appendChild(text);

	return tag;
}

QDomElement textTag(QDomDocument &doc, const QString &name, QSize &s)
{
	QString str;
	str.sprintf("%d,%d", s.width(), s.height());

	QDomElement tag = doc.createElement(name);
	QDomText text = doc.createTextNode(str);
	tag.appendChild(text);

	return tag;
}

QDomElement textTag(QDomDocument &doc, const QString &name, QRect &r)
{
	QString str;
	str.sprintf("%d,%d,%d,%d", r.x(), r.y(), r.width(), r.height());

	QDomElement tag = doc.createElement(name);
	QDomText text = doc.createTextNode(str);
	tag.appendChild(text);

	return tag;
}

QDomElement stringListToXml(QDomDocument &doc, const QString &name, const QStringList &l)
{
	QDomElement tag = doc.createElement(name);
	for(QStringList::ConstIterator it = l.begin(); it != l.end(); ++it)
		tag.appendChild(textTag(doc, "item", *it));

	return tag;
}

/*QString tagContent(const QDomElement &e)
{
	// look for some tag content
	for(QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
		QDomText i = n.toText();
		if(i.isNull())
			continue;
		return i.data();
	}

	return "";
}*/

/*QDomElement findSubTag(const QDomElement &e, const QString &name, bool *found)
{
	if(found)
		*found = FALSE;

	for(QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
		QDomElement i = n.toElement();
		if(i.isNull())
			continue;
		if(i.tagName() == name) {
			if(found)
				*found = TRUE;
			return i;
		}
	}

	QDomElement tmp;
	return tmp;
}*/

void readEntry(const QDomElement &e, const QString &name, QString *v)
{
	bool found = FALSE;
	QDomElement tag = findSubTag(e, name, &found);
	if(!found)
		return;
	*v = tagContent(tag);
}

void readNumEntry(const QDomElement &e, const QString &name, int *v)
{
	bool found = FALSE;
	QDomElement tag = findSubTag(e, name, &found);
	if(!found)
		return;
	*v = tagContent(tag).toInt();
}

void readBoolEntry(const QDomElement &e, const QString &name, bool *v)
{
	bool found = FALSE;
	QDomElement tag = findSubTag(e, name, &found);
	if(!found)
		return;
	*v = (tagContent(tag) == "true") ? TRUE: FALSE;
}

void readSizeEntry(const QDomElement &e, const QString &name, QSize *v)
{
	bool found = FALSE;
	QDomElement tag = findSubTag(e, name, &found);
	if(!found)
		return;
	QStringList list = QStringList::split(',', tagContent(tag));
	if(list.count() != 2)
		return;
	QSize s;
	s.setWidth(list[0].toInt());
	s.setHeight(list[1].toInt());
	*v = s;
}

void readRectEntry(const QDomElement &e, const QString &name, QRect *v)
{
	bool found = FALSE;
	QDomElement tag = findSubTag(e, name, &found);
	if(!found)
		return;
	QStringList list = QStringList::split(',', tagContent(tag));
	if(list.count() != 4)
		return;
	QRect r;
	r.setX(list[0].toInt());
	r.setY(list[1].toInt());
	r.setWidth(list[2].toInt());
	r.setHeight(list[3].toInt());
	*v = r;
}

void readColorEntry(const QDomElement &e, const QString &name, QColor *v)
{
	bool found = FALSE;
	QDomElement tag = findSubTag(e, name, &found);
	if(!found)
		return;
	QColor c;
	c.setNamedColor(tagContent(tag));
	if(c.isValid())
		*v = c;
}

void xmlToStringList(const QDomElement &e, const QString &name, QStringList *v)
{
	bool found = false;
	QDomElement tag = findSubTag(e, name, &found);
	if(!found)
		return;
	QStringList list;
	for(QDomNode n = tag.firstChild(); !n.isNull(); n = n.nextSibling()) {
		QDomElement i = n.toElement();
		if(i.isNull())
			continue;
		if(i.tagName() == "item")
			list += tagContent(i);
	}
	*v = list;
}

void setBoolAttribute(QDomElement e, const QString &name, bool b)
{
	e.setAttribute(name, b ? "true" : "false");
}

void readBoolAttribute(QDomElement e, const QString &name, bool *v)
{
	if(e.hasAttribute(name)) {
		QString s = e.attribute(name);
		*v = (s == "true") ? TRUE: FALSE;
	}
}

}

