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
#include <qpe/timestring.h>
#include <qpixmap.h>
#include "datatypes.h"

#define FILE_VERSION 4

class Condition;
class Filter;
class QDate;
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
    QString getColId(QString column);
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
    c4_View getData();
    c4_View sortData(c4_View filteredData, QString column, bool ascending);
    c4_View sortData(c4_View filteredData, QString sortingName);

    QString currentFilter();
    QStringList listFilters();
    Filter *getFilter(QString name);
    void addFilter(Filter *filter);
    void deleteFilter(QString name);
    void deleteFilterColumn(QString filterName, QString columnName);
    int getConditionCount(QString filterName);
    Condition *getCondition(QString filterName, int index);

    QString addRow(QStringList values, int *rowId = 0);
    void updateRow(int rowId, QStringList values);
    void deleteRow(int id);
    void commit();
    QPixmap getCheckBoxPixmap(int checked);
    QString dateToString(int date);
    QString dateToString(QDate &date);
    QString importFromCSV(QString filename);
    void exportToCSV(QString filename);

private:
    int *listTypes();
    c4_View createEmptyView(QStringList colNames);
    QString makeColId(int id, int type);
    QString formatString(bool old = FALSE);
    void updateDataColumnFormat();
    void updateEncoding();
    void updateEncoding(c4_View &view);

private:
    QPixmap checkedPixmap;
    QPixmap uncheckedPixmap;
    DateFormat::Order dateOrder;
    QString dateSeparator;
    c4_Storage *file;
    c4_View columns;
    c4_View views;
    c4_View viewColumns;
    c4_View sorts;
    c4_View sortColumns;
    c4_View filters;
    c4_View filterConditions;
    c4_View global;
    c4_View data;
    View *curView;
    Filter *curFilter;
    int maxId;
    c4_IntProp Id;
    // "_columns" view
    c4_IntProp cIndex;
    c4_StringProp cName;
    c4_IntProp cType;
    c4_StringProp cDefault;
    c4_IntProp cId;
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
    // "_filters" view
    c4_StringProp fName;
    // "_filterconditions" view
    c4_StringProp fcFilter;
    c4_IntProp fcIndex;
    c4_StringProp fcColumn;
    c4_IntProp fcOperator;
    c4_StringProp fcConstant;
    c4_IntProp fcCase;
    // "_global" view
    c4_IntProp gVersion;
    c4_StringProp gView;
    c4_StringProp gSort;
    c4_StringProp gFilter;
};

#endif
