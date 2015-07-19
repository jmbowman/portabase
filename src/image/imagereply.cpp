/*
 * imagereply.cpp
 *
 * (c) 2012 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file imagereply.cpp
 * Source file for ImageReply
 */

#include <QFile>
#include <QNetworkAccessManager>
#include <QTimer>
#include "database.h"
#include "imagereply.h"

/**
 * Constructor.
 *
 * @param req The request containing the URL for the image
 * @param db The database to get the image data from
 */
ImageReply::ImageReply(const QNetworkRequest &req, Database *db)
    : QNetworkReply(), offset(0)
{
    setOpenMode(QIODevice::ReadOnly);
    setOperation(QNetworkAccessManager::GetOperation);
    setRequest(req);
    QString scheme = req.url().scheme();
    QString format;
    if (scheme == QLatin1String("img")) {
        QStringList parts = req.url().host().split('_');
        int rowId = parts[0].toInt();
        int colIndex = parts[1].toInt();
        format = parts[2];
        QString colName = db->listColumns()[colIndex];
        content = db->getBinaryField(rowId, colName);
    }
    else {
        // work around QTBUG-13805, still not fixed in Fremantle, where
        // loading qrc resources prompts for a network connection
        QString fileName = QLatin1Char(':') + req.url().path();
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly)) {
            content = file.readAll();
            file.close();
        }
        // cheating here, only worried about two particular PNG files
        format = QString("png");
    }
    setHeader(QNetworkRequest::ContentTypeHeader, QString("image/%1").arg(format.toLower()));
    setHeader(QNetworkRequest::ContentLengthHeader, content.size());
    QTimer::singleShot(0, this, SIGNAL(readyRead()));
    QTimer::singleShot(0, this, SIGNAL(finished()));
}

/**
 * Aborts the operation immediately.
 */
void ImageReply::abort()
{
}

/**
 * Returns the number of bytes that are available for reading.
 *
 * @return The number of bytes remaining
 */
qint64 ImageReply::bytesAvailable() const
{
    return content.size() - offset;
}

/**
 * Returns true if this device is sequential; otherwise returns false.
 *
 * @return False; could support random access, but currently no need to
 */
bool ImageReply::isSequential() const
{
    return true;
}

/**
 * Reads bytes from the image data into "data".
 *
 * @param data The buffer to read data into
 * @param maxSize The maximum number of bytes to read
 * @return The number of bytes read, or -1 if an error occurred
 */
qint64 ImageReply::readData(char *data, qint64 maxSize)
{
    if (offset < content.size()) {
        qint64 count = qMin(maxSize, content.size() - offset);
        memcpy(data, content.constData() + offset, count);
        offset += count;
        return count;
    }
    else {
        return -1;
    }
}
