/*
 * importdialog.h
 *
 * (c) 2003-2004,2008-2009 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file importdialog.h
 * Header file for ImportDialog
 */

#ifndef IMPORT_DIALOG_H
#define IMPORT_DIALOG_H

#include <QObject>
#include <QStringList>

class Database;
class QWidget;

/**
 * Wrapper for the file open dialog which imports data from the selected
 * file.
 */
class ImportDialog : QObject
{
    Q_OBJECT
public:
    /** Enumeration of possible types of files to import from */
    enum DataSource {
        NoSource = 0,
        CSV = 1,
        MobileDB = 2,
        XML = 3,
        OptionList = 4,
        Image = 5
    };
    ImportDialog(DataSource sourceType, QWidget *parent = 0);

    bool exec();
    bool import(Database *db);
    QStringList getOptions();
    QString getPath();

private:
    QWidget *parentWidget; /**< This dialog's parent widget */
    DataSource source; /**< The type of file being imported from */
    QString encoding; /**< The text encoding to use for a CSV or enum options file */
    QStringList options; /**< List of imported enumeration options, if any */
    QString path; /**< The absolute path of the file that was imported */
};

#endif
