/*
 * view.cpp
 *
 * (c) 2002-2004,2009-2013,2016-2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file
 * Source file for View
 */

#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QTextStream>
#include "csvutils.h"
#include "database.h"
#include "datatypes.h"
#include "factory.h"
#include "filter.h"
#include "formatting.h"
#include "view.h"
#include "image/imageutils.h"

/**
 * Constructor.
 *
 * @param name The name of this view
 * @param parent The database this view belongs to
 * @param baseview The main Metakit data table
 * @param colNames Ordered list of names of fields to include in the view
 * @param types Ordered array of field data types
 * @param widths Ordered array of field display widths (in device-independent pixels)
 * @param colIds Ordered list of Metakit main data table field identifiers
 * @param stringColIds Ordered list of Metakit main data table alternate
 *                     string representation field identifiers
 * @param rpp Maximum number of records to display per page of data
 */
View::View(const QString &name, Database *parent, c4_View baseview,
  const QStringList &colNames, int *types, int *widths,
  const QStringList &colIds, const QStringList &stringColIds, int rpp)
  : QObject(), viewName(name), Id("_id"), sortColumn(""),
  sortOrder(Qt::AscendingOrder), sortName(""), omitRow(-1)
{
    db = parent;
    dbview = baseview;
    columns = colNames;
    rowCnt = dbview.GetSize();
    colCnt = columns.count();
    dataTypes = types;
    colWidths = widths;
    int i;
    int count = colIds.count();
    for (i = 0; i < count; i++) {
        ids << colIds[i].toUtf8();
    }
    count = stringColIds.count();
    for (i = 0; i < count; i++) {
        scIds << stringColIds[i].toUtf8();
    }
    rowsPerPage = rpp;
    pageFirstRow = 0;
    pageRowCnt = rowCnt;
    sort(db->currentSorting());
}

/**
 * Destructor.
 */
View::~View()
{
    delete[] dataTypes;
    delete[] colWidths;
}

/**
 * Get the name of this view.
 *
 * @return The view's name
 */
QString View::getName()
{
    return viewName;
}

/**
 * Get the names of fields included in this view, listed in display order.
 *
 * @return A list of field names
 */
QStringList View::getColNames()
{
    return columns;
}

/**
 * Get the data types of fields included in this view, listed in display
 * order.
 *
 * @return An array of field data type codes
 */
int *View::getColTypes()
{
    return dataTypes;
}

/**
 * Get the width in pixels of the field at the specified display index.
 *
 * @param index The position index of a field in this view
 * @return The width of the display column for the specified field
 */
int View::getColWidth(int index)
{
    return colWidths[index];
}

/**
 * Set the display width of the specified field.
 *
 * @param index The position index of a field in this view
 * @param width The width in pixels to use for the field
 */
void View::setColWidth(int index, int width)
{
    colWidths[index] = width;
}

/**
 * Save any changes made to field display widths so that they will be used
 * again the next time the database is opened.
 */
void View::saveColWidths()
{
    db->setViewColWidths(colWidths);
}

/**
 * Get the number of records to show on each data display page.
 *
 * @return The number of records to show per page
 */
int View::getRowsPerPage()
{
    return rowsPerPage;
}

/**
 * Set the number of records to show on each data display page.
 *
 * @param rpp The number of records to show per page
 */
void View::setRowsPerPage(int rpp)
{
    rowsPerPage = rpp;
    db->setViewRowsPerPage(rpp);
}

/**
 * Set data on the subset of the view to be returned for the currently
 * displayed page.
 *
 * @param firstRow The index of the first row to include on the page
 * @param rowCount The number of rows on the current page
 * @param omit Index of a row to pretend doesn't exist, -1 if none
 */
void View::setPagination(int firstRow, int rowCount, int omit)
{
    pageFirstRow = firstRow;
    pageRowCnt = rowCount;
    omitRow = omit;
}

/**
 * Get the total number of records matching the currently applied filter.
 *
 * @return The number of records passing the current filter
 */
int View::totalRowCount()
{
    return rowCnt;
}

/**
 * Get the number of records on the currently-displayed page.
 *
 * @param parent The parent item in the data tree; should always be an invalid index
 * @return The number of records on the page
 */
int View::rowCount(const QModelIndex &) const
{
    return pageRowCnt;
}

/**
 * Get the total number of columns included in this view.
 *
 * @param parent The parent item in the data tree; should always be an invalid index
 * @return The number of columns in this view
 */
int View::columnCount(const QModelIndex &) const
{
    return colCnt;
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
QVariant View::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical || section >= colCnt) {
        return QVariant();
    }
    else if (role == Qt::DisplayRole) {
        return columns[section];
    }
    else if (role == Qt::DecorationRole && dataTypes[section] == NOTE) {
        return Factory::icon("note");
    }
    return QVariant();
}

/**
 * Get the data stored under the given role for the item referred to by the
 * index.  The index is with respect to the currently displayed page.
 *
 * @param index The location of the desired data in the view
 * @param role The role the retrieved data will be used in
 * @return The desired data value
 */
QVariant View::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    int rowIndex = pageFirstRow + index.row();
    if (omitRow != -1 && omitRow <= rowIndex) {
        rowIndex++;
    }
    int colIndex = index.column();
    if (rowIndex >= pageFirstRow + pageRowCnt || colIndex >= colCnt) {
        return QVariant();
    }
    int type = dataTypes[colIndex];
    if (role == Qt::CheckStateRole) {
        if (type == BOOLEAN) {
            c4_RowRef row = dbview[rowIndex];
            c4_IntProp prop(ids[colIndex]);
            int value = prop (row);
            return (int)(value ? Qt::Checked : Qt::Unchecked);
        }
    }
    if (role == Qt::DecorationRole) {
        if (type == IMAGE) {
            c4_RowRef row = dbview[rowIndex];
            c4_StringProp prop(scIds[colIndex]);
            if (!QString::fromUtf8(prop (row)).isEmpty()) {
                return Factory::icon("image");
            }
            else {
                return QVariant();
            }
        }
        else {
            return QVariant();
        }
    }
    else if (role == Qt::DisplayRole) {
        c4_RowRef row = dbview[rowIndex];
        if (type == INTEGER || type == SEQUENCE) {
            c4_IntProp prop(ids[colIndex]);
            int value = prop (row);
            return QLocale::system().toString(value);
        }
        else if (type == FLOAT || type == CALC) {
            // want the string version here
            c4_StringProp prop(scIds[colIndex]);
            return Formatting::toLocalDouble(QString::fromUtf8(prop (row)));
        }
        else if (type == STRING || type == NOTE || type >= FIRST_ENUM) {
            c4_StringProp prop(ids[colIndex]);
            return QString::fromUtf8(prop (row)).replace("\n", " ");
        }
        else if (type == DATE) {
            c4_IntProp prop(ids[colIndex]);
            int value = prop (row);
            return Formatting::dateToString(value);
        }
        else if (type == TIME) {
            c4_IntProp prop(ids[colIndex]);
            int value = prop (row);
            return Formatting::timeToString(value);
        }
        else if (type == BOOLEAN || type == IMAGE) {
            return "";
        }
        else {
            return QVariant();
        }
    }
    else if (role == Qt::EditRole) {
        if (type == NOTE) {
            c4_RowRef row = dbview[rowIndex];
            c4_StringProp prop(ids[colIndex]);
            return QString::fromUtf8(prop (row));
        }
        return QVariant();
    }
    else if (role == Qt::TextAlignmentRole) {
        if (type == INTEGER || type == FLOAT || type == CALC || type == SEQUENCE) {
            return (int)(Qt::AlignRight | Qt::AlignVCenter);
        }
        return (int)(Qt::AlignLeft | Qt::AlignVCenter);
    }
    else {
        return QVariant();
    }
}

/**
 * Get the data for the specified record.
 *
 * @param index The index of the desired record in the current Metakit view
 */
QStringList View::getRow(int index)
{
    QStringList results;
    c4_RowRef row = dbview[index];
    int numCols = columns.count();
    for (int i = 0; i < numCols; i++) {
        int type = dataTypes[i];
        if (type == INTEGER || type == BOOLEAN || type == SEQUENCE) {
            c4_IntProp prop(ids[i]);
            int value = prop (row);
            results.append(QLocale::system().toString(value));
        }
        else if (type == FLOAT || type == CALC) {
            // want the string version here
            c4_StringProp prop(scIds[i]);
            QString standardForm = QString::fromUtf8(prop (row));
            results.append(Formatting::toLocalDouble(standardForm));
        }
        else if (type == IMAGE) {
            // want the string version here
            c4_StringProp prop(scIds[i]);
            results.append(QString::fromUtf8(prop (row)));
        }
        else if (type == STRING || type == NOTE || type >= FIRST_ENUM) {
            c4_StringProp prop(ids[i]);
            results.append(QString::fromUtf8(prop (row)));
        }
        else if (type == DATE) {
            c4_IntProp prop(ids[i]);
            int value = prop (row);
            results.append(Formatting::dateToString(value));
        }
        else if (type == TIME) {
            c4_IntProp prop(ids[i]);
            int value = prop (row);
            results.append(Formatting::timeToString(value));
        }
    }
    return results;
}

/**
 * Get the image format identifier ("JPEG" or "PNG") of the specified image
 * field.
 *
 * @param rowId The ID of the record to fetch data from
 * @param colIndex The position index of the field to fetch data from
 * @return The format of the specified field's image ("" if none)
 */
QString View::getImageFormat(int rowId, int colIndex)
{
    int index = dbview.Find(Id [rowId]);
    c4_StringProp prop(scIds[colIndex]);
    return QString::fromUtf8(prop (dbview[index]));
}

/**
 * Get the image stored in the specified field.
 *
 * @param rowId The ID of the record to fetch data from
 * @param colIndex The position index of the field to fetch data from
 * @return The requested image
 */
QImage View::getImage(int rowId, int colIndex)
{
    QString colName = columns[colIndex];
    QString format = getImageFormat(rowId, colIndex);
    return ImageUtils::load(db, rowId, colName, format);
}

/**
 * Update the sorting parameters to indicate the next sorting direction on
 * the specified field.  The order is descending if this field was already
 * being sorted in ascending order, ascending otherwise.
 *
 * @param colIndex The position index of the field to sort on
 */
void View::toggleSort(int colIndex)
{
    if (columns[colIndex] != sortColumn) {
        sortOrder = Qt::AscendingOrder;
        sortColumn = columns[colIndex];
    }
    else {
        if (sortOrder == Qt::AscendingOrder) {
            sortOrder = Qt::DescendingOrder;
        }
        else {
            sortOrder = Qt::AscendingOrder;
        }
    }
}

/**
 * Update the sorting parameters to indicate use of the specified sorting.
 *
 * @param sortingName The name of the sorting to apply
 */
void View::sort(const QString &sortingName)
{
    sortColumn = "";
    sortName = sortingName;
}

/**
 * Update the Metakit view in use to reflect the current filtering and
 * sorting parameters.
 */
void View::prepareData()
{
    dbview = db->getData();
    Filter *filter = db->getFilter(db->currentFilter());
    dbview = filter->apply(dbview);
    if (!sortColumn.isEmpty()) {
        dbview = db->sortData(dbview, sortColumn, sortOrder);
    }
    else {
        // if sortName is "", just returns the unsorted data
        dbview = db->sortData(dbview, sortName);
    }
    rowCnt = dbview.GetSize();
}

/**
 * Get the ID of the record at the specified position index (with filtering
 * and sorting in effect).
 *
 * @param index The position index of the record in question
 * @return The specified record's ID
 */
int View::getId(int index)
{
    return Id (dbview[index]);
}

/**
 * Get the position index (with filtering and sorting in effect) of the row
 * with the specified ID.
 *
 * @param rowId The ID of the row to locate
 * @return The row's position index (or -1 if it didn't pass the filter)
 */
int View::getIndex(int rowId)
{
    return dbview.Find(Id [rowId]);
}

/**
 * Delete from the database all records that match the currently applied
 * filter.
 */
void View::deleteAllRows()
{
    int count = dbview.GetSize();
    int *ids = new int[count];
    int i;
    for (i = 0; i < count; i++) {
        ids[i] = Id (dbview[count - i - 1]);
    }
    // do deletions after obtaining all IDs so we don't need to worry about
    // the view changing
    for (i = 0; i < count; i++) {
        db->deleteRow(ids[i], false);
    }
    db->compressRowIds();
    delete[] ids;
}

/**
 * Export the information in the current view and filter to an HTML file at
 * the specified path.  The records are listed in the current sorting order.
 * Unlike the HTML generated for printing, this method's output is intended
 * to be a standalone page viewable in normal web browsers, with easily
 * overridden display properties.
 *
 * @param filename The HTML file to create or overwrite
 */
void View::exportToHTML(const QString &filename)
{
    // Open the output template
    QFile templateFile(":/templates/export.html");
    if (!templateFile.open(QFile::ReadOnly)) {
        return;
    }
    QTextStream stream(&templateFile);
    QString result = stream.readAll();
    templateFile.close();

    // populate the title and row colors
    result = result.arg(QFileInfo(filename).completeBaseName());
    result = result.arg(Factory::evenRowColor.name());
    result = result.arg(Factory::oddRowColor.name());

    // populate the column headers
    QStringList lines;
    int colCount = columns.count();
    int i, j;
    QString headerPattern("<th>%1</th>\n");
    for (i = 0; i < colCount; i++) {
        lines.append(headerPattern.arg(Formatting::toHtmlEscaped(columns[i])));
    }
    QFile f(filename);
    if (!f.open(QFile::WriteOnly)) {
        return;
    }
    QTextStream output(&f);
    output.setCodec("UTF-8");
    output << result.arg(lines.join(""));

    // populate the data
    QStringList data;
    int type;
    QStringList rowStarts;
    rowStarts << "<tr class=\"even\">\n" << "<tr class=\"odd\">\n";
    QString rowEnd("</tr>\n");
    QString leftPattern = "<td>%1</td>\n";
    QString rightPattern = "<td class=\"r\">%1</td>\n";
    QStringList divs;
    divs << "<td><div class=\"n\">&#9744;</div></td>\n";
    divs << "<td><div class=\"y\">&#9745;</div></td>\n";
    divs << "<td><div class=\"i\">&#9997;</div></td>\n";
    QString newline("\n");
    QString br("<br>");
    QString value;
    for (i = 0; i < rowCnt; i++) {
        output << rowStarts[i % 2];
        data = getRow(i);
        for (j = 0; j < colCount; j++) {
            type = dataTypes[j];
            if (type == INTEGER || type == FLOAT || type == CALC
                || type == SEQUENCE) {
                output << rightPattern.arg(data[j]);
            }
            else if (type == BOOLEAN) {
                output << divs[data[j].toInt()];
            }
            else if (type == IMAGE && !data[j].isEmpty()) {
                output << divs[2];
            }
            else if (type == NOTE || type == STRING) {
                value = Formatting::toHtmlEscaped(data[j]).replace(newline, br);
                output << leftPattern.arg(value);
            }
            else {
                output << leftPattern.arg(data[j]);
            }
        }
        output << rowEnd;
    }
    output << "</tbody>\n</table>\n</body>\n</html>\n";
    f.close();
}

/**
 * Export the full PortaBase file content to an XML file.  The records are
 * listed in the current sorting order, and the ones which do not match the
 * current filter are marked with an 'h="y"' attribute (an abbreviation for
 * 'hidden="yes"').
 *
 * @param filename The XML file to create or overwrite
 */
void View::exportToXML(const QString &filename)
{
    c4_View fullView = db->getData();
    if (!sortColumn.isEmpty()) {
        fullView = db->sortData(fullView, sortColumn, sortOrder);
    }
    else {
        // if sortName is "", just returns the unsorted data
        fullView = db->sortData(fullView, sortName);
    }
    Filter *filter = db->getFilter(db->currentFilter());
    c4_View currView = filter->apply(fullView);
    db->exportToXML(filename, fullView, currView, columns);
}

/**
 * Get some summary statistics on the specified field in a human-readable
 * format.  The exact content of the summary depends on the data type of the
 * field.  The summary is with respect to the current filter, not the entire
 * database (unless an all-inclusive filter is selected, of course).
 *
 * @param colIndex The position index of the field to get statistics for
 * @return A list of individual "Name: value" statistics formatted as rows
 *         of an HTML table
 */
QStringList View::getStatistics(int colIndex)
{
    QStringList lines;
    if (rowCnt == 0) {
        lines.append(QString("<tr><td>%1</td></tr>")
                     .arg(tr("No data to summarize")));
        return lines;
    }
    int type = dataTypes[colIndex];
    QLocale locale = QLocale::system();
    QString pattern("<tr><td align=\"right\"><font color=\"#0000ff\">%1</font>: </td><td>%2</td></tr>");
    if (type == INTEGER || type == SEQUENCE) {
        c4_IntProp prop(ids[colIndex]);
        int value = prop (dbview[0]);
        int total = value;
        int min = value;
        int max = value;
        for (int i = 1; i < rowCnt; i++) {
            int value = prop (dbview[i]);
            total += value;
            min = qMin(min, value);
            max = qMax(max, value);
        }
        float mean = total / (float)rowCnt;
        lines.append(pattern.arg(tr("Total")).arg(locale.toString(total)));
        lines.append(pattern.arg(tr("Average")).arg(locale.toString(mean)));
        lines.append(pattern.arg(tr("Minimum")).arg(locale.toString(min)));
        lines.append(pattern.arg(tr("Maximum")).arg(locale.toString(max)));
    }
    else if (type == FLOAT || type == CALC) {
        c4_FloatProp prop(ids[colIndex]);
        c4_StringProp stringProp(scIds[colIndex]);
        double value = prop (dbview[0]);
        double total = value;
        double min = value;
        double max = value;
        QString minString = QString::fromUtf8(stringProp (dbview[0]));
        QString maxString = minString;
        for (int i = 1; i < rowCnt; i++) {
            double value = prop (dbview[i]);
            total += value;
            if (value < min) {
                min = value;
                minString = QString::fromUtf8(stringProp (dbview[i]));
            }
            if (value > max) {
                max = value;
                maxString = QString::fromUtf8(stringProp (dbview[i]));
            }
        }
        double mean = total / rowCnt;
        lines.append(pattern.arg(tr("Total"))
                            .arg(locale.toString(total, 'f', 2)));
        lines.append(pattern.arg(tr("Average"))
                            .arg(locale.toString(mean, 'f', 2)));
        lines.append(pattern.arg(tr("Minimum"))
                            .arg(Formatting::toLocalDouble(minString)));
        lines.append(pattern.arg(tr("Maximum"))
                            .arg(Formatting::toLocalDouble(maxString)));
    }
    else if (type == BOOLEAN) {
        c4_IntProp prop(ids[colIndex]);
        int checked = 0;
        for (int i = 0; i < rowCnt; i++) {
            int value = prop (dbview[i]);
            if (value == 1) {
                checked++;
            }
        }
        lines.append(pattern.arg(tr("Checked"))
                            .arg(locale.toString(checked)));
        lines.append(pattern.arg(tr("Unchecked"))
                            .arg(locale.toString(rowCnt - checked)));
    }
    else if (type == DATE) {
        c4_IntProp prop(ids[colIndex]);
        int value = prop (dbview[0]);
        int min = value;
        int max = value;
        for (int i = 1; i < rowCnt; i++) {
            int value = prop (dbview[i]);
            if (value != 17520914) {
                min = (min == 17520914) ? value : qMin(min, value);
                max = qMax(max, value);
            }
        }
        lines.append(pattern.arg(tr("Earliest"))
                            .arg(Formatting::dateToString(min)));
        lines.append(pattern.arg(tr("Latest"))
                            .arg(Formatting::dateToString(max)));
    }
    else if (type == TIME) {
        c4_IntProp prop(ids[colIndex]);
        int value = prop (dbview[0]);
        int min = value;
        int max = value;
        for (int i = 1; i < rowCnt; i++) {
            int value = prop (dbview[i]);
            if (value != -1) {
                min = (min == -1) ? value : qMin(min, value);
                max = qMax(max, value);
            }
        }
        lines.append(pattern.arg(tr("Earliest"))
                            .arg(Formatting::timeToString(min)));
        lines.append(pattern.arg(tr("Latest"))
                            .arg(Formatting::timeToString(max)));
    }
    else if (type == STRING || type == NOTE) {
        c4_StringProp prop(ids[colIndex]);
        QString value = QString::fromUtf8(prop (dbview[0]));
        int length = value.length();
        int total = length;
        int min = length;
        int max = length;
        for (int i = 1; i < rowCnt; i++) {
            QString svalue = QString::fromUtf8(prop (dbview[i]));
            length = svalue.length();
            total += length;
            min = qMin(min, length);
            max = qMax(max, length);
        }
        int mean = total / rowCnt;
        lines.append(pattern.arg(tr("Total length"))
                            .arg(tr("%Ln character(s)", "", total)));
        lines.append(pattern.arg(tr("Average length"))
                            .arg(tr("%Ln character(s)", "", mean)));
        lines.append(pattern.arg(tr("Minimum length"))
                            .arg(tr("%Ln character(s)", "", min)));
        lines.append(pattern.arg(tr("Maximum length"))
                            .arg(tr("%Ln character(s)", "", max)));
    }
    else if (type == IMAGE) {
        c4_StringProp stringProp(scIds[colIndex]);
        int missing = 0;
        for (int i = 0; i < rowCnt; i++) {
            QString value(stringProp (dbview[i]));
            if (value.isEmpty()) {
                missing++;
            }
        }
        lines.append(pattern.arg(tr("Image available"))
                            .arg(locale.toString(rowCnt - missing)));
        lines.append(pattern.arg(tr("No image"))
                            .arg(locale.toString(missing)));
    }
    else if (type >= FIRST_ENUM) {
        c4_StringProp prop(ids[colIndex]);
        QStringList options = db->listEnumOptions(type);
        int optionCount = options.count();
        int *tallies = new int[optionCount];
        int i;
        for (i = 0; i < optionCount; i++) {
            tallies[i] = 0;
        }
        for (i = 0; i < rowCnt; i++) {
            int index = options.indexOf(QString::fromUtf8(prop (dbview[i])));
            tallies[index] = tallies[index] + 1;
        }
        for (i = 0; i < optionCount; i++) {
            lines.append(pattern.arg(options[i])
                                .arg(locale.toString(tallies[i])));
        }
        delete[] tallies;
    }
    return lines;
}

/**
 * Copy the sorting parameters from the specified view, and apply both those
 * and its current filter.
 *
 * @param otherView The view whose properties are to be mimicked
 */
void View::copyStateFrom(View *otherView)
{
    sortColumn = otherView->sortColumn;
    sortOrder = otherView->sortOrder;
    sortName = otherView->sortName;
    prepareData();
}
