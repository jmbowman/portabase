/*
 * view.h
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef VIEW_H
#define VIEW_H

#include <mk4.h>
#include <qimage.h>
#include "datatypes.h"

class c4_IntProp;
class c4_View;
class Database;
class QString;
class QStringList;

class View
{
public:
    View(QString name, Database *parent, c4_View baseview,
         QStringList colNames, int *types, int *widths, QStringList colIds,
         QStringList stringColIds, int rpp);
    ~View();

    QString getName();
    QStringList getColNames();
    int *getColTypes();
    int getColWidth(int index);
    void setColWidth(int index, int width);
    void saveColWidths();
    int getRowsPerPage();
    void setRowsPerPage(int rpp);
    int getRowCount();
    QStringList getRow(int index);
    int getId(int index);
    void sort(int colIndex);
    void sort(QString sortingName);
    void prepareData();
    QStringList getStatistics(int colIndex);
    QString getNote(int rowId, int colIndex);
    QString getImageFormat(int rowId, int colIndex);
    QImage getImage(int rowId, int colIndex);
    void deleteAllRows();
    void exportToCSV(QString filename);
    void exportToXML(QString filename);
    void copyStateFrom(View *otherView);

private:
    QString viewName;
    Database *db;
    c4_View dbview;
    c4_IntProp Id;
    QStringList columns;
    int *dataTypes;
    int *colWidths;
    QStringList ids;
    QStringList scIds;
    int numCols;
    int rowsPerPage;
    int sortColumn;
    bool ascending;
    QString sortName;
};

#endif
