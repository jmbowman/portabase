/*
 * importdialog.h
 *
 * (c) 2003-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef IMPORT_DIALOG_H
#define IMPORT_DIALOG_H

#include <qstringlist.h>

// possible data sources
#define NO_SOURCE 0
#define CSV_FILE 1
#define MOBILEDB_FILE 2
#define XML_FILE 3
#define OPTION_LIST 4
#define IMAGE_FILE 5

#include <qobject.h>

class Database;
class QWidget;

class ImportDialog : QObject
{
    Q_OBJECT
public:
    ImportDialog(int sourceType, Database *subject, QWidget *parent = 0);
    ~ImportDialog();

    bool exec();
    QStringList getOptions();
    QString getPath();

private:
    Database *db;
    QWidget *parentWidget;
    int source;
    QStringList options;
    QString path;
};

#endif
