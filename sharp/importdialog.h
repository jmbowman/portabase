/*
 * importdialog.h
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef IMPORT_DIALOG_H
#define IMPORT_DIALOG_H

#include <qdialog.h>
#include <qstringlist.h>

// possible data sources
#define NO_SOURCE 0
#define CSV_FILE 1
#define MOBILEDB_FILE 2
#define XML_FILE 3
#define OPTION_LIST 4

class Database;
class QComboBox;
class QFileInfo;
class SlFileSelector;

class ImportDialog: public QDialog
{
    Q_OBJECT
public:
    ImportDialog(int sourceType, Database *subject, QWidget *parent = 0,
                 const char *name = 0, WFlags f = 0);
    ~ImportDialog();
    int exec();
    QStringList getOptions();
    QString getPath();

private slots:
    void import(const QFileInfo &);

private:
    bool import(const QString &file);

private:
    Database *db;
    QComboBox *encodings;
    SlFileSelector *selector;
    int source;
    bool importDone;
    QStringList options;
    QString path;
};

#endif
