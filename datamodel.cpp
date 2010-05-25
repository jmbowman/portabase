/*
 * datamodel.cpp
 *
 * (c) 2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file datamodel.cpp
 * Source file for DataModel
 */

#include <math.h>
#include "database.h"
#include "datamodel.h"
#include "view.h"

/**
 * Constructor.
 *
 * @param parent This object's parent object
 */
DataModel::DataModel(QObject *parent): QAbstractTableModel(parent), db(0),
    currentView(0), rpp(10), currentPage(1)
{

}

/**
 * Get the database currently in use.
 *
 * @return The current database (0 if none)
 */
Database *DataModel::database()
{
    return db;
}

/**
 * Set the database from which data is to be shown.  Called when a file is
 * created, opened, or significantly altered.
 *
 * @param dbase The database now in use.
 */
void DataModel::setDatabase(Database *dbase)
{
    beginResetModel();
    closeView();
    db = dbase;
    if (currentPage > 0) {
        currentPage = 1;
    }
    setView(db->currentView(), false, true);
    endResetModel();
}

/**
 * Get the view currently in use.
 *
 * @return The current view (0 if none)
 */
View *DataModel::view()
{
    return currentView;
}

/**
 * Set the view currently in use.
 *
 * @param name The name of the new view to use
 * @param applyDefaults True if the view's default filter and sorting are to
 *                      be used, false to retain the current settings
 * @param midReset True if this is part of a larger model reset operation
 */
void DataModel::setView(const QString &name, bool applyDefaults, bool midReset)
{
    if (!midReset) {
        beginResetModel();
    }
    closeView();
    currentView = db->getView(name, applyDefaults);
    currentView->prepareData();
    rpp = currentView->getRowsPerPage();
    processPaginationChange(true);
    if (!midReset) {
        endResetModel();
    }
    emit viewLoaded(currentView);
}

/**
 * Save any settings made to the current view, then remove this object's
 * reference to it.
 */
void DataModel::closeView()
{
    if (currentView) {
        saveViewSettings();
        currentView = 0;
    }
    // don't signal a reset; that'll happen when the next view gets opened
}

/**
 * Store in the database any changes made to column widths or the maximum
 * number of records per page.
 */
void DataModel::saveViewSettings()
{
    if (currentView) {
        currentView->saveColWidths();
        currentView->setRowsPerPage(rpp);
    }
}

/**
 * Make the model aware of a change in the number of rows to display on each
 * page.
 *
 * @param rpp The new number of rows per page
 */
void DataModel::setRowsPerPage(int rowsPerPage)
{
    if (rpp == rowsPerPage) {
        return;
    }
    rpp = rowsPerPage;
    processPaginationChange();
}

/**
 * Get the page number of the data to be shown
 *
 * @return The current page number (0 if not using pages)
 */
int DataModel::page()
{
    return currentPage;
}

/**
 * Set the page of data to be shown.
 *
 * @param page The new page number
 */
void DataModel::setPage(int p)
{
    if (p == currentPage) {
        return;
    }
    currentPage = p;
    processPaginationChange();
}

/**
 * Make updates to the database view and signal any listening widgets
 * appropriately when a pagination change occurs.  This could be optimized
 * better, but since the main point of pagination is to show few enough rows
 * to avoid displaying a vertical scrollbar, there probably wouldn't be
 * enough performance gain to be worth it.
 *
 * @param midReset True if this is part of a larger model reset operation
 */
void DataModel::processPaginationChange(bool midReset)
{
    if (currentView == 0) {
        // the table and buttons shouldn't even be visible
        return;
    }
    int totalRows = currentView->totalRowCount();
    int firstRow = 0;
    int pageRows = totalRows;
    if (currentPage > 0) { // pagination in use
        int totalPages = (int)(ceil((double)totalRows/(double)rpp));
        if (currentPage > totalPages) {
            currentPage = (totalPages == 0) ? 1 : totalPages;
        }
        emit paginationChanged(currentPage, totalPages);
        firstRow = (currentPage - 1) * rpp;
        pageRows = qMin(rpp, totalRows - firstRow);
    }
    if (!midReset) {
        beginResetModel();
    }
    currentView->setPagination(firstRow, pageRows);
    if (!midReset) {
        endResetModel();
    }
}

/**
 * Set the sorting to use and update the displayed data accordingly.
 *
 * @param name The name of the sorting to use
 */
void DataModel::setSorting(const QString &name)
{
    beginResetModel();
    currentView->sort(name);
    currentView->prepareData();
    endResetModel();
}

/**
 * Update the sorting parameters to indicate the next sorting direction on
 * the specified field.  The order is descending if this field was already
 * being sorted in ascending order, ascending otherwise.
 *
 * @param colIndex The position index of the field to sort on
 */
void DataModel::toggleSort(int colIndex)
{
    beginResetModel();
    currentView->toggleSort(colIndex);
    currentView->prepareData();
    endResetModel();
}

/**
 * Set the filter to use and update the data display accordingly.
 *
 * @param name The name of the filter to use
 */
void DataModel::setFilter(const QString &name)
{
    beginResetModel();
    db->getFilter(name);
    currentView->prepareData();
    processPaginationChange(true);
    endResetModel();
}

/**
 * Force a full refresh of the view data and the display grid.
 */
void DataModel::refresh()
{
    beginResetModel();
    currentView->prepareData();
    processPaginationChange(true);
    endResetModel();
}

/**
 * Update the model to reflect the addition of a new record to the database.
 * If the new row doesn't pass the current filter, there's no need to update
 * the display.
 *
 * @param rowId The ID of the row that was added
 */
void DataModel::addRow(int rowId)
{
    int oldCount = currentView->totalRowCount();
    currentView->prepareData();
    int newCount = currentView->totalRowCount();
    if (newCount > oldCount) {
        int index = currentView->getIndex(rowId);
        int pageStart = qMax(0, (currentPage - 1) * rpp);
        int pageEnd = newCount - 1; // if not paged
        if (currentPage > 0 && pageStart + rpp > newCount) {
            pageEnd = pageStart + rpp - 1; // limit since it is paged
            // anything get pushed to the next page?
            if (index <= pageEnd && newCount > pageEnd + 1) {
                beginRemoveRows(QModelIndex(), pageEnd, pageEnd);
                currentView->setPagination(pageStart + 1, rpp - 1);
                endRemoveRows();
            }
        }
        if (index < pageStart) {
            // was inserted into a previous page
            // include the row from the last page that got pushed here
            beginInsertRows(QModelIndex(), 0, 0);
            currentView->setPagination(pageStart, qMin(rpp, newCount - pageStart));
            endInsertRows();
        }
        else if (index <= pageEnd) {
            // was inserted into this page
            int virtualIndex = index - pageStart;
            beginInsertRows(QModelIndex(), virtualIndex, virtualIndex);
            currentView->setPagination(pageStart, pageEnd - pageStart + 1);
            endInsertRows();
        }
        // no need to signal the view for this, just updating the page buttons
        processPaginationChange(true);
    }

}

/**
 * Update the model to reflect a particular record being edited.
 *
 * @param rowId The ID of the edited row
 * @param oldIndex The index within the view of the row before it was edited
 */
void DataModel::editRow(int rowId, int oldIndex)
{
    currentView->prepareData();
    int newIndex = currentView->getIndex(rowId);
    int pageStart = qMax(0, (currentPage - 1) * rpp);
    int pageEnd = currentView->totalRowCount() - 1; // if not paged
    if (currentPage > 0) {
        pageEnd = pageStart + rpp - 1; // limit if paged
    }
    if (newIndex == oldIndex) {
        // nothing moved, just update that row if it's on the page
        if (newIndex >= pageStart && newIndex <= pageEnd) {
            QModelIndex start = createIndex(newIndex, 0);
            QModelIndex end = createIndex(newIndex, currentView->columnCount() - 1);
            emit dataChanged(start, end);
        }
        return;
    }
    // remove it from its old location
    processRowRemoval(oldIndex, newIndex);
    if (newIndex < pageStart) {
        // row got moved to an earlier page, add the previous row to this page
        beginInsertRows(QModelIndex(), 0, 0);
        currentView->setPagination(pageStart, pageEnd - pageStart + 1);
        endInsertRows();
    }
    else if (newIndex <= pageEnd) {
        // got moved elsewhere on the page; add it in
        int virtualIndex = newIndex - pageStart;
        beginInsertRows(QModelIndex(), virtualIndex, virtualIndex);
        currentView->setPagination(pageStart, pageEnd - pageStart + 1);
        endInsertRows();
    }
}

/**
 * Delete the specified record from the database and update the model
 * accordingly.
 *
 * @param rowId the ID of the deleted row
 */
void DataModel::deleteRow(int rowId)
{
    int oldIndex = currentView->getIndex(rowId);
    db->deleteRow(rowId);
    currentView->prepareData();
    processRowRemoval(oldIndex);
}

/**
 * Do the grunge work of removing a row from the model.  Used by deleteRow(),
 * but also by editRow() if the row got resorted or failed the filter.
 *
 * @param oldIndex The previous position of the row being removed (in the
 *                 database view, not the model)
 * @param omit The index of a new row to temporarily ignore, -1 if none
 */
void DataModel::processRowRemoval(int oldIndex, int omit)
{
    int pageStart = qMax(0, (currentPage - 1) * rpp);
    int totalRows = currentView->totalRowCount();
    int pageEnd = totalRows - 1; // if not paged
    if (currentPage > 0) {
        pageEnd = pageStart + rpp - 1; // limit if paged
    }
    int tempPageEnd = qMin(pageEnd, totalRows - 1); // tentative actual end index
    if (currentPage > 0 && oldIndex <= pageEnd && totalRows >= pageEnd + 1) {
        // the number of rows in the page will drop by one briefly
        tempPageEnd--;
    }
    if (oldIndex < pageStart) {
        // removed from a previous page; the first row from this one drops back
        beginRemoveRows(QModelIndex(), 0, 0);
        currentView->setPagination(pageStart, tempPageEnd - pageStart + 1, omit);
        endRemoveRows();
    }
    else if (oldIndex <= pageEnd) {
        // it was on this page, remove it
        int virtualIndex = oldIndex - pageStart;
        beginRemoveRows(QModelIndex(), virtualIndex, virtualIndex);
        currentView->setPagination(pageStart, tempPageEnd - pageStart + 1, omit);
        endRemoveRows();
    }
    if (totalRows > pageEnd && (omit == -1 || omit > pageEnd)) {
        // move forward a row from the next page
        beginInsertRows(QModelIndex(), rpp - 1, rpp - 1);
        currentView->setPagination(pageStart, pageEnd - pageStart + 1);
        endInsertRows();
    }
    if (currentPage > 0) {
        // update the pagination
        int rowsOnLastPage = totalRows % rpp;
        if (rowsOnLastPage == 0) {
            // lost a page; were we on it?
            processPaginationChange(currentPage <= totalRows / rpp);
        }
    }
}

/**
 * Delete all rows included in the current filter.  Do this in both the
 * database and the model.
 */
void DataModel::deleteAllRows()
{
    int totalRows = currentView->totalRowCount();
    if (totalRows == 0) {
        // nothing to delete
        return;
    }
    beginRemoveRows(QModelIndex(), 0, totalRows - 1);
    currentView->deleteAllRows();
    currentView->prepareData();
    processPaginationChange(true);
    endRemoveRows();
}

/**
 * Toggle the value of the boolean field at the specified location.
 *
 * @param index The location of the field to be toggled
 */
void DataModel::toggleBoolean(const QModelIndex &index)
{
    QString colName = headerData(index.column(), Qt::Horizontal).toString();
    int pageStart = qMax(0, (currentPage - 1) * rpp);
    int rowIndex = pageStart + index.row();
    int rowId = currentView->getId(rowIndex);
    db->toggleBoolean(rowId, colName);
    editRow(rowId, rowIndex);
}

/**
 * Get the number of rows in the currently displayed data set.
 *
 * @parent The parent item in the data tree; should always be an invalid index
 * @return The number of rows in the subset
 */
int DataModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || !currentView) {
        return 0;
    }
    return currentView->rowCount(parent);
}

/**
 * Get the number of columns in the currently displayed data set.
 *
 * @parent The parent item in the data tree; should always be an invalid index
 * @return The number of columns in the subset
 */
int DataModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid() || !currentView) {
        return 0;
    }
    return currentView->columnCount(parent);
}

/**
 * Get the data stored under the given role for the item referred to by the
 * index.
 *
 * @param index The location of the desired data in the table
 * @param role The role the retrieved data will be used in
 * @return The desired data value
 */
QVariant DataModel::data(const QModelIndex &index, int role) const
{
    if (currentView) {
        return currentView->data(index, role);
    }
    return QVariant();
}

/**
 * Get the data for the given role and section in the header with the
 * specified orientation.  For horizontal headers, the section number
 * corresponds to the column number. Similarly, for vertical headers, the
 * section number corresponds to the row number.
 *
 * @param section The column or row number
 * @param orientation The orientation of the header whose data is wanted
 * @param role The role of the header data to get
 * @return The desired header data
 */
QVariant DataModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (currentView) {
        return currentView->headerData(section, orientation, role);
    }
    return QVariant();
}

/**
 * Begin a model reset operation.  Calls QAbstractItemModel::beginResetModel()
 * on Qt 4.6 and above, does nothing on earlier versions where the cruder
 * all-in-one reset() method has to be used.
 */
void DataModel::beginResetModel()
{
#if QT_VERSION >= 0x040600
    QAbstractItemModel::beginResetModel();
#endif
}

/**
 * Complete a model reset operation.  Calls QAbstractItemModel::endResetModel()
 * on Qt 4.6 and above, calls the cruder all-in-one reset() on earlier
 * versions.
 */
void DataModel::endResetModel()
{
#if QT_VERSION >= 0x040600
    QAbstractItemModel::endResetModel();
#else
    reset();
#endif
}
