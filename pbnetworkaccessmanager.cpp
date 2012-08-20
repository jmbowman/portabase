/*
 * pbnetworkaccessmanager.cpp
 *
 * (c) 2012 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file
 * Source file for PBNetworkAccessManager
 */

#include <QNetworkReply>
#include "image/imagereply.h"
#include "pbnetworkaccessmanager.h"

/**
 * Constructor.
 *
 * @param dbase The database from which images will be retrieved.
 */
PBNetworkAccessManager::PBNetworkAccessManager(Database *dbase)
    : QNetworkAccessManager(), db(dbase)
{

}

/**
 * Returns a new QNetworkReply object to handle the request. Get requests
 * for "img:/..." URLs are handled here, all others are handled by the
 * superclass.
 *
 * @param op The type of request operation being performed
 * @param req The request for an asset
 * @param outgoingData Data for post and put operations
 * @return The reply to the request
 */
QNetworkReply *PBNetworkAccessManager::createRequest(QNetworkAccessManager::Operation op,
                                                     const QNetworkRequest &req,
                                                     QIODevice *outgoingData)
{
    QString scheme = req.url().scheme();
    if ((scheme != QLatin1String("img") && scheme != QLatin1String("qrc"))
        || op != QNetworkAccessManager::GetOperation) {
        return QNetworkAccessManager::createRequest(op, req, outgoingData);
    }
    return new ImageReply(req, db);
}
