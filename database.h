/*
 * database.h
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef DATABASE_H
#define DATABASE_H

#include <mk4.h>
#include <qpixmap.h>
#include "datatypes.h"

#define FILE_VERSION 2

class QString;
class QStringList;
class View;

class Database
{
public:
    Database(QString path, bool *ok);
    ~Database();

    QString currentView();
    View *getView(QString name);
    QStringList listViews();
    void addView(QString name, QStringList names);
    void deleteView(QString name);
    void setViewColWidths(int *widths);
    void setViewRowsPerPage(int rpp);

    QStringList listColumns();
    int getIndex(QString column);
    void setIndex(QString column, int index);
    int getType(QString column);
    QString getDefault(QString column);
    void setDefault(QString column, QString value);
    QString isValidValue(int type, QString value);
    void addColumn(int index, QString name, int type, QString defaultVal);
    void renameColumn(QString oldName, QString newName);
    void deleteColumn(QString name);
    void renameView(QString oldName, QString newName);
    QStringList listViewColumns(QString viewName);
    void addViewColumn(QString viewName, QString columnName);
    void deleteViewColumn(QString viewName, QString columnName);
    void setViewColumnSequence(QString viewName, QStringList colNames);
    void updateDataFormat();
    QStringList getRow(int rowId);

    QString currentSorting();
    QStringList listSortings();
    bool getSortingInfo(QString sortingName, QStringList *allCols,
                        QStringList *descCols);
    void addSorting(QString name, QStringList allCols, QStringList descCols);
    void deleteSorting(QString name);
    void deleteSortingColumn(QString sortName, QString columnName);
    c4_View sortData(QString column, bool ascending);
    c4_View sortData(QString sortingName);

    QString addRow(QStringList values);
    void updateRow(int rowId, QStringList values);
    void deleteRow(int id);
    void commit();
    QPixmap getCheckBoxPixmap(int checked);
    QString importFromCSV(QString filename);
    void exportToCSV(QString filename);

private:
    int *listTypes();
    c4_View createEmptyView(QStringList colNames);
    QString formatString();

private:
    QPixmap checkedPixmap;
    QPixmap uncheckedPixmap;
    c4_Storage *file;
    c4_View columns;
    c4_View views;
    c4_View viewColumns;
    c4_View sorts;
    c4_View sortColumns;
    c4_View global;
    c4_View data;
    View *curView;
    int maxId;
    c4_IntProp Id;
    // "_columns" view
    c4_IntProp cIndex;
    c4_StringProp cName;
    c4_IntProp cType;
    c4_StringProp cDefault;
    // "_views" view
    c4_StringProp vName;
    c4_IntProp vRpp;
    // "_viewcolumns" view
    c4_StringProp vcView;
    c4_IntProp vcIndex;
    c4_StringProp vcName;
    c4_IntProp vcWidth;
    // "_sorts" view
    c4_StringProp sName;
    // "_sortcolumns" view
    c4_StringProp scSort;
    c4_IntProp scIndex;
    c4_StringProp scName;
    c4_IntProp scDesc;
    // "_global" view
    c4_IntProp gVersion;
    c4_StringProp gView;
    c4_StringProp gSort;
};

#endif
