/*
    Copyright (C) 2002 Nikolas Zimmermann <wildfox@kde.org>
    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    aint with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KSVGIconPainter_H
#define KSVGIconPainter_H

#include <libart_lgpl/art_render.h>
#include <libart_lgpl/art_render_gradient.h>

class QImage;
class QColor;
class QWMatrix;
class QDomElement;
class QPointArray;

class KDECORE_EXPORT KSVGIconPainter
{
public:
	KSVGIconPainter(int width, int height);
	~KSVGIconPainter();

	void setDrawWidth(int dwidth);
	void setDrawHeight(int dheight);

	QImage *image();

	QWMatrix *worldMatrix();

	void finish();

	void setUseFill(bool fill);
	void setUseStroke(bool stroke);

	void setStrokeWidth(double width);
	void setStrokeMiterLimit(const QString &miter);
	void setCapStyle(const QString &cap);
	void setJoinStyle(const QString &join);
	void setStrokeColor(const QString &stroke);
	void setFillColor(const QString &fill);
	void setFillRule(const QString &fillRule);
	void setOpacity(const QString &opacity);
	void setFillOpacity(const QString &fillOpacity);
	void setStrokeOpacity(const QString &strokeOpacity);
	void setStrokeDashOffset(const QString &dashOffset);
	void setStrokeDashArray(const QString &dashes);

	void setWorldMatrix(QWMatrix *worldMatrix);
	void setClippingRect(int x, int y, int w, int h);

	void drawRectangle(double x, double y, double w, double h, double rx, double ry);
	void drawEllipse(double cx, double cy, double rx, double ry);
	void drawLine(double x1, double y1, double x2, double y2);
	void drawPolyline(QPointArray polyArray, int points = -1);
	void drawPolygon(QPointArray polyArray);
	void drawPath(const QString &data, bool fill);
	void drawImage(double x, double y, QImage &image);

	QColor parseColor(const QString &param);
	Q_UINT32 toArtColor(const QColor &color);
	Q_UINT32 parseOpacity(const QString &data);

	double toPixel(const QString &s, bool hmode);
	double dpi();

	ArtGradientLinear *linearGradient(const QString &id);
	void addLinearGradient(const QString &id, ArtGradientLinear *gradient);

	QDomElement linearGradientElement(ArtGradientLinear *linear);
	void addLinearGradientElement(ArtGradientLinear *gradient, QDomElement element);

	ArtGradientRadial *radialGradient(const QString &id);
	void addRadialGradient(const QString &id, ArtGradientRadial *gradient);

	QDomElement radialGradientElement(ArtGradientRadial *radial);
	void addRadialGradientElement(ArtGradientRadial *gradient, QDomElement element);

	QWMatrix parseTransform(const QString &transform);

private:
	struct Private;
	Private *d;
};

#endif
