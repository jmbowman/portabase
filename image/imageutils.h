/*
 * imageutils.h
 *
 * (c) 2003,2008-2009 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file imageutils.h
 * Header file for ImageUtils
 */

#ifndef IMAGE_UTILS_H
#define IMAGE_UTILS_H

#include <QByteArray>
#include <QImage>
#include <QString>

class Database;

/**
 * Utilities for loading and exporting images.
 */
class ImageUtils
{
public:
    ImageUtils();

    static QImage load(Database *db, int rowId, const QString &colName,
                       const QString &format);
    QImage load(const QString &path, bool *resized);
    QString getFormat() const;
    QString getErrorMessage() const;
    static QByteArray getImageData(QImage image, const QString &format,
                              const QString &path, bool changed);
    void setExportPaths(const QString &filePath);
    QString exportImage(Database *db, int rowId, const QString &columnName,
                        const QString &format);

private:
    QString format; /**< Image format; "JPEG" or "PNG" */
    QString error; /**< Error message (if any) for the last image load */
    QString imageAbsPath; /**< Absolute path of the directory to save exported images in */
    QString imageRelPath; /**< Version of imageAbsPath relative to the exported file */
};

#endif
