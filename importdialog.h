/*
 * importdialog.h
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef IMPORTDIALOG_H
#define IMPORT_DIALOG_H

#include <qdialog.h>

class Database;
class DocLnk;
class FileSelector;

class ImportDialog: public QDialog
{
    Q_OBJECT
public:
    ImportDialog(Database *subject, QWidget *parent = 0, const char *name = 0,
                 WFlags f = 0);
    ~ImportDialog();

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void import(const DocLnk &);

private:
    Database *db;
    FileSelector *csvSelector;
};

#endif
