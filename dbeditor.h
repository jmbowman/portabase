/*
 * dbeditor.h
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef DBEDITOR_H
#define DBEDITOR_H

#include <mk4.h>
#include <qdialog.h>

class ColumnEditor;
class Database;
class QListView;

class DBEditor: public QDialog
{
    Q_OBJECT
public:
    DBEditor(QWidget *parent = 0, const char *name = 0, WFlags f = 0);
    ~DBEditor();

    int edit(Database *subject);
    void applyChanges();

private:
    void updateTable();
    QString getTypeString(int type);
    bool isValidName(QString colName);
    bool isValidDefault(int type, QString defaultVal);
    void selectRow(QString colName);

private slots:
    void addColumn();
    void editColumn();
    void deleteColumn();
    void moveUp();
    void moveDown();

private:
    ColumnEditor *columnEditor;
    Database *db;
    QListView *table;
    QStringList originalCols;
    QStringList deletedCols;
    QStringList renamedCols;
    c4_View info;
    c4_StringProp ceName;
    c4_IntProp ceType;
    c4_StringProp ceDefault;
    c4_IntProp ceOldIndex;
    c4_IntProp ceNewIndex;
};

#endif
