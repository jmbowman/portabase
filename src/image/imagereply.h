/*
 * imagereply.h
 *
 * (c) 2012 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file imagereply.h
 * Header file for ImageReply
 */

#ifndef IMAGE_REPLY_H
#define IMAGE_REPLY_H

#include <QNetworkReply>
#include <QNetworkRequest>

class Database;

/**
 * Reply for a request from a QWebView for image data from a database field.
 */
class ImageReply : public QNetworkReply
{
    Q_OBJECT
public:
    ImageReply(const QNetworkRequest &req, Database *db);

    void abort();
    qint64 bytesAvailable() const;
    bool isSequential() const;

protected:
    qint64 readData(char *data, qint64 maxSize);

private:
    QByteArray content; /**< The image data from the database */
    qint64 offset; /**< The current byte offset in the image data */
};
#endif
