/*
 * view.h
 *
 * (c) 2002-2004,2009-2011 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file view.h
 * Header file for View
 */

#ifndef VIEW_H
#define VIEW_H

#include <mk4.h>
#include <QImage>
#include <QModelIndex>
#include <QObject>
#include <QStringList>
#include "datatypes.h"

class Database;

/**
 * Representation of a view in the database (a data table using a subset of
 * the available data fields, shown in a particular order).
 */
class View : public QObject
{
    Q_OBJECT
public:
    View(const QString &name, Database *parent, c4_View baseview,
         const QStringList &colNames, int *types, int *widths,
         const QStringList &colIds, const QStringList &stringColIds, int rpp);
    ~View();

    QString getName();
    QStringList getColNames();
    int *getColTypes();
    int getColWidth(int index);
    void setColWidth(int index, int width);
    void saveColWidths();
    int getRowsPerPage();
    void setRowsPerPage(int rpp);
    void setPagination(int firstRow, int rowCount, int omit=-1);
    QStringList getRow(int index);
    int getId(int index);
    int getIndex(int rowId);
    void toggleSort(int colIndex);
    void sort(const QString &sortingName);
    void prepareData();
    QStringList getStatistics(int colIndex);
    QString getImageFormat(int rowId, int colIndex);
    QImage getImage(int rowId, int colIndex);
    void deleteAllRows();
    void exportToCSV(const QString &filename);
    void exportToHTML(const QString &filename);
    void exportToXML(const QString &filename);
    void copyStateFrom(View *otherView);
    int totalRowCount();

    // used by DataModel; QAbstractTableModel interface
    int rowCount(const QModelIndex &parent=QModelIndex()) const;
    int columnCount(const QModelIndex &parent=QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const;
    QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const;

private:
    QString viewName; /**< The name of this view */
    Database *db; /**< The database this view belongs to */
    c4_View dbview; /**< The underlying Metakit data view, with filtering and sorting applied */
    c4_IntProp Id; /**< Metakit record ID field identifier */
    QStringList columns; /**< Ordered list of names for fields included in the view */
    int *dataTypes; /**< Ordered array of field data types */
    int *colWidths; /**< Ordered array of field display widths (in pixels) */
    QList<QByteArray> ids; /**< Ordered list of Metakit data field identifiers */
    QList<QByteArray> scIds; /**< Ordered list of alternate string form field identifiers */
    int rowsPerPage; /**< The maximum number of records to show per page of data */
    int sortColumn; /**< The index of the single column being sorted on, -1 if none */
    Qt::SortOrder sortOrder; /**< Order of single-column sorting */
    QString sortName; /**< The name of the sorting currently in use, if any */
    int rowCnt; /**< The number of rows currently in the view, cached for performance */
    int colCnt; /**< The number of columns currently in the view, cached for performance */
    int pageRowCnt; /**< The number of rows in the current display page, cached for performance */
    int pageFirstRow; /**< The index of the first row in the current display page, cached for performance */
    int omitRow; /**< Index of a row to temporarily ignore for model requests, -1 if none */
};

#endif
