/*
 * dbeditor.h
 *
 * (c) 2002-2004,2008-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file dbeditor.h
 * Header file for DBEditor
 */

#ifndef DBEDITOR_H
#define DBEDITOR_H

#include <mk4.h>
#include <QMap>
#include <QStringList>
#include "pbdialog.h"

class CalcNode;
class ColumnEditor;
class Database;
class QLabel;
class QStackedWidget;
class QTreeWidget;

typedef QMap<QString,CalcNode*> NameCalcMap;
typedef QMap<QString,int> NameDecimalsMap;

/**
 * "Columns Editor" dialog for editing the format of a %PortaBase database.
 * Shows all of the currently defined columns, and shows buttons for adding,
 * editing, deleting, and reordering the database columns.
 */
class DBEditor: public PBDialog
{
    Q_OBJECT
public:
    DBEditor(QWidget *parent = 0);
    ~DBEditor();

    int edit(Database *subject);
    void applyChanges();
    QStringList columnNames();
    int *columnTypes();

protected:
    void resizeEvent(QResizeEvent *event);

private:
    void updateTable();
    QString getTypeString(int type);
    bool isValidName(const QString &colName);
    bool isValidDefault(int type, const QString &defaultVal);
    void selectRow(const QString &colName);
    void renameColumnRefs(const QString &oldName, const QString &newName);
    void deleteColumnRefs(const QString &name);

private slots:
    void addColumn();
    void editColumn();
    void deleteColumn();
    void moveUp();
    void moveDown();

private:
    ColumnEditor *columnEditor; /**< The subdialog for adding or editing columns */
    Database *db; /**< The database being edited */
    QStackedWidget *stack; /**< Main widget stack (data display and "no columns" label) */
    QTreeWidget *table; /**< The display of the column definition data */
    QLabel *noColumns; /**< "No columns" placeholder label */
    QStringList originalCols; /**< The list of columns that existed when the dialog was launched */
    QStringList deletedCols; /**< The list of columns that have been marked for deletion */
    QStringList renamedCols; /**< The current names of all the original columns (including ones already marked for deletion) */
    NameCalcMap calcMap; /**< Mapping from column names to calculation root nodes */
    NameDecimalsMap decimalsMap; /**< Mapping from column names to calculation display decimal places */
    c4_View info; /**< In-memory table of column definition data */
    c4_StringProp ceName; /**< The column name field */
    c4_IntProp ceType; /**< The column type field */
    c4_StringProp ceDefault; /**< The column default value field */
    c4_IntProp ceOldIndex; /**< Field for the original column position index */
    c4_IntProp ceNewIndex; /**< Field for the current column position index */
    bool resized; /**< True if the dialog has already been set to its initial size, false otherwise */
};

#endif
