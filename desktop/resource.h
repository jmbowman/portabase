/*
 * resource.h
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef PIXMAPLOADER_H
#define PIXMAPLOADER_H

#include <qimage.h>
#include <qiconset.h>

class Resource
{
public:
    Resource() {}

    static QImage loadImage(const QString &name);
    static QPixmap loadPixmap(const QString &name);
    static QString findPixmap(const QString &name);
    static QIconSet loadIconSet(const QString &name);
};

// Inline for compatibility with SHARP ROMs
inline QIconSet Resource::loadIconSet(const QString &pix) 
{
    QImage img = loadImage(pix);
    QPixmap pm;
    pm.convertFromImage(img);
    QIconSet is(pm);
    /*QIconSet::Size size = pm.width() <= 22 ? QIconSet::Small : QIconSet::Large;

    QPixmap dpm = loadPixmap(pix + "_disabled");

#ifndef QT_NO_DEPTH_32	// have alpha-blended pixmaps
    if (dpm.isNull()) {
	QImage dimg(img.width(), img.height(), 32);
	for (int y = 0; y < img.height(); y++) {
	    for (int x = 0; x < img.width(); x++) {
		QRgb p = img.pixel(x, y);
		uint a = (p & 0xff000000) / 3;
		p = (p & 0x00ffffff) | (a & 0xff000000);
		dimg.setPixel(x, y, p);
	    }
	}

	dimg.setAlphaBuffer(TRUE);
	dpm.convertFromImage(dimg);
    }
#endif

    if (!dpm.isNull()) {
	is.setPixmap(dpm, size, QIconSet::Disabled);
	}*/
    return is;
}

#endif
