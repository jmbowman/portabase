/*
 * resource.cpp
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "qpeapplication.h"
#include "resource.h"
#include <qfile.h>
#include <qpixmapcache.h>

QPixmap Resource::loadPixmap(const QString &pix)
{
    QPixmap pm;
    QString key = "QPE_" + pix;
    if (!QPixmapCache::find(key, pm)) {
	pm.convertFromImage(loadImage(pix));
	QPixmapCache::insert(key, pm);
    }
    return pm;
}

QString Resource::findPixmap(const QString &pix)
{
    QString picsPath = QPEApplication::iconDir();

    if (QFile(picsPath + pix + ".png").exists()) {
	return picsPath + pix + ".png";
    }
    else if (QFile(picsPath + pix + ".xpm").exists()) {
	return picsPath + pix + ".xpm";
    }
    else if (QFile(picsPath + pix).exists()) {
	return picsPath + pix;
    }
    return QString();
}

QImage Resource::loadImage(const QString &name)
{
    return QImage(findPixmap(name));
}
