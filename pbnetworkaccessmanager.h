/*
 * pbnetworkaccessmanager.h
 *
 * (c) 2012 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file
 * Header file for PBNetworkAccessManager
 */

#ifndef PB_NETWWRK_ACCESS_MANAGER_H
#define PB_NETWORK_ACCESS_MANAGER_H

#include <QNetworkAccessManager>
#include <QNetworkRequest>

class Database;
class QIODevice;
class QNetworkReply;

/**
 * Subclass of QNetworkAccessManager that allows image data to be retrieved
 * by QWebView widgets directly from database fields.  Used by RowViewer on
 * Maemo Fremantle (where QTextEdit has proven somewhat buggy for HTML
 * display).
 */
class PBNetworkAccessManager: public QNetworkAccessManager
{
public:
    PBNetworkAccessManager(Database *dbase);

protected:
    QNetworkReply *createRequest(QNetworkAccessManager::Operation op,
                                 const QNetworkRequest &req,
                                 QIODevice *outgoingData = 0);

private:
    Database *db; /**< The database containing the images */
};
#endif
