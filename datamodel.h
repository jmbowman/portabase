/*
 * datamodel.h
 *
 * (c) 2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file datamodel.h
 * Header file for DataModel
 */

#ifndef DATAMODEL_H
#define DATAMODEL_H

#include <QAbstractTableModel>
#include <QObject>

class Database;
class View;

/**
 * Data model for the currently-displayed subset of the main data table in
 * a %PortaBase file.  Used to optimize access to that table from Qt views,
 * like the main data grid widget.
 */
class DataModel: public QAbstractTableModel
{
    Q_OBJECT
public:
    DataModel(QObject *parent=0);

    int rowCount(const QModelIndex &parent=QModelIndex()) const;
    int columnCount(const QModelIndex &parent=QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role=Qt::DisplayRole) const;
    void toggleSort(int column);
    Database *database();
    void setDatabase(Database *dbase);
    View *view();
    void setView(const QString &name, bool applyDefaults, bool midReset=false);
    void closeView();
    void saveViewSettings();
    void setRowsPerPage(int rowsPerPage);
    int page();
    void setPage(int p);
    void setSorting(const QString &name);
    void setFilter(const QString &name);
    void addRow(int rowId);
    void editRow(int rowId, int oldIndex=-1);
    void deleteRow(int rowId);
    void deleteAllRows();
    void toggleBoolean(const QModelIndex &index);
    void refresh();

signals:
    void paginationChanged(int currentPage, int totalPages);
    void viewLoaded(View *v);

protected:
    void beginResetModel();
    void endResetModel();

private:
    void processPaginationChange(bool midReset=false);
    void processRowRemoval(int oldIndex, int omit=-1);

private:
    Database *db; /**< The database currently in use */
    View *currentView; /**< The current data view */
    int rpp; /**< The current number of rows per page; may not have been saved to the View yet */
    int currentPage; /**< The currently displayed page (0 if not using pagination) */
};

#endif // DATAMODEL_H
