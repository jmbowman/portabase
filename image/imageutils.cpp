/*
 * imageutils.cpp
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qbuffer.h>
#include <qcstring.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <stdlib.h>
#include <string.h>
#include "imageeditor.h"
#include "imagereader.h"
#include "imageutils.h"
#include "../database.h"

ImageUtils::ImageUtils()
{

}

ImageUtils::~ImageUtils()
{

}

QImage ImageUtils::load(Database *db, int rowId, const QString &colName,
                        const QString &format)
{
    int size = 0;
    char *data = db->getBinaryField(rowId, colName, &size);
    QByteArray array;
    array.setRawData(data, (uint)size);
    QBuffer buffer(array);
    QImageIO iio(&buffer, format);
    buffer.open(IO_ReadOnly);
    QImage image;
    if (iio.read()) {
        image = iio.image();
    }
    buffer.close();
    array.resetRawData(data, (uint)size);
    free(data);
    return image;
}

QImage ImageUtils::load(const QString &path, bool *resized)
{
    ImageReader reader(path);
    format = reader.format();
    if (format != "JPEG" && format != "PNG") {
        error = ImageEditor::tr("Unsupported image format");
        return QImage();
    }
    int width = reader.imageWidth();
    int height = reader.imageHeight();
    // Scale the image down so as to not swamp the Zaurus's memory when loaded
    // (only works for JPEGs, big PNG images will be rejected)
    int scaleDenom = 1;
    *resized = FALSE;
    while (width * height > 800 * 600) {
        scaleDenom *= 2;
        // I doubt if many people have 6400 x 4800 images, but it's probably
        // only a matter of time before digital cameras get there...
        if (scaleDenom > 8 || format != "JPEG") {
            error = ImageEditor::tr("Image is too large to import");
            return QImage();
        }
        width /= 2;
        height /= 2;
        *resized = TRUE;
    }
    return reader.getImage(width, height);
}

QString ImageUtils::getFormat()
{
    return format;
}

QString ImageUtils::getErrorMessage()
{
    return error;
}

char *ImageUtils::getImageData(QImage image, const QString &format,
                               const QString &path, bool changed, int *size)
{
    if (!changed) {
        QFile file(path);
        *size = file.size();
        char *data = (char*)malloc(*size);
        file.open(IO_ReadOnly);
        int position = 0;
        while (!file.atEnd()) {
            position += file.readBlock(data + position, 1024);
        }
        file.close();
        return data;
    }
    else {
        QByteArray array;
        QBuffer buffer(array);
        QImageIO iio(&buffer, format);
        iio.setImage(image);
        buffer.open(IO_WriteOnly);
        iio.write();
        buffer.close();
        *size = array.size();
        char *data = (char*)malloc(*size);
        memcpy(data, array.data(), *size);
        return data;
    }
}

void ImageUtils::setExportPaths(const QString &filePath)
{
    QFileInfo info(filePath);
#if !defined(Q_WS_QWS) || defined(SHARP)
    // Put image files in the same directory as the CSV/XML file
    jpegAbsPath = info.dirPath(TRUE) + "/";
    jpegRelPath = "";
    pngAbsPath = jpegAbsPath;
    pngRelPath = "";
#else
    // Put image files in the appropriate mime type directories
    QDir baseDir = info.dir(TRUE);
    // currently in Documents/text/x-csv or Documents/text/xml
    baseDir.cdUp();
    baseDir.cdUp();
    Qstring basePath = baseDir.absPath();
    jpegAbsPath = basePath + "/image/jpeg/";
    jpegRelPath = "../../image/jpeg/";
    pngAbsPath = basePath + "/image/png/";
    pngRelPath = "../../image/png/";
#endif
}

QString ImageUtils::exportImage(Database *db, int rowId,
                                const QString &columnName,
                                const QString &format)
{
    if (format == "") {
        return "";
    }
    QString suffix;
    QString absBase;
    QString relBase;
    if (format == "JPEG") {
        suffix = ".jpg";
        absBase = jpegAbsPath;
        relBase = jpegRelPath;
    }
    else {
        suffix = ".png";
        absBase = pngAbsPath;
        relBase = pngRelPath;
    }
    QString nameRoot = columnName + QString("_%1").arg(rowId);
    QString filename = nameRoot + suffix;
    int counter = 0;
    while (QFile::exists(absBase + filename)) {
        filename = nameRoot + QString("_%1").arg(counter) + suffix;
        counter++;
    }
    int size;
    char *data = db->getBinaryField(rowId, columnName, &size);
    if (size == 0) {
        return "";
    }
    QFile file(absBase + filename);
    file.open(IO_WriteOnly);
    int position = 0;
    int blockSize;
    while (position < size) {
        blockSize = QMIN(1024, size - position);
        position += file.writeBlock(data + position, blockSize);
    }
    file.close();
    free(data);
    return relBase + filename;
}
