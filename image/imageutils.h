/*
 * imageutils.h
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef IMAGE_UTILS_H
#define IMAGE_UTILS_H

#include <qimage.h>
#include <qstring.h>

class Database;

class ImageUtils
{
public:
    ImageUtils();
    ~ImageUtils();

    static QImage load(Database *db, int rowId, const QString &colName,
                       const QString &format);
    QImage load(const QString &path, bool *resized);
    QString getFormat();
    QString getErrorMessage();
    static char *getImageData(QImage image, const QString &format,
                              const QString &path, bool changed, int *size);
    void setExportPaths(const QString &filePath);
    QString exportImage(Database *db, int rowId, const QString &columnName,
                        const QString &format);

private:
    QString format;
    QString error;
    QString jpegAbsPath;
    QString jpegRelPath;
    QString pngAbsPath;
    QString pngRelPath;
};

#endif
