/*
 * view.cpp
 *
 * (c) 2002-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <mk4.h>
#include <qfile.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include "csvutils.h"
#include "database.h"
#include "datatypes.h"
#include "filter.h"
#include "view.h"
#include "image/imageutils.h"

View::View(QString name, Database *parent, c4_View baseview, QStringList colNames, int *types, int *widths, QStringList colIds, QStringList stringColIds, int rpp)
  : QObject(), viewName(name), Id("_id"), sortColumn(-1), ascending(TRUE), sortName("")
{
    db = parent;
    dbview = baseview;
    columns = colNames;
    dataTypes = types;
    colWidths = widths;
    ids = colIds;
    scIds = stringColIds;
    numCols = columns.count();
    rowsPerPage = rpp;
    sort(db->currentSorting());
}

View::~View()
{
    delete[] dataTypes;
    delete[] colWidths;
}

QString View::getName()
{
    return viewName;
}

QStringList View::getColNames()
{
    return columns;
}

int *View::getColTypes()
{
    return dataTypes;
}

int View::getColWidth(int index)
{
    return colWidths[index];
}

void View::setColWidth(int index, int width)
{
    colWidths[index] = width;
}

void View::saveColWidths()
{
    db->setViewColWidths(colWidths);
}

int View::getRowsPerPage() {
    return rowsPerPage;
}

void View::setRowsPerPage(int rpp) {
    rowsPerPage = rpp;
    db->setViewRowsPerPage(rpp);
}

int View::getRowCount()
{
    return dbview.GetSize();
}

QStringList View::getRow(int index)
{
    QStringList results;
    c4_RowRef row = dbview[index];
    for (int i = 0; i < numCols; i++) {
        int type = dataTypes[i];
        if (type == INTEGER || type == BOOLEAN || type == SEQUENCE) {
            c4_IntProp prop(ids[i]);
            int value = prop (row);
            results.append(QString::number(value));
        }
        else if (type == FLOAT || type == CALC || type == IMAGE) {
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
            results.append(db->dateToString(value));
        }
        else if (type == TIME) {
            c4_IntProp prop(ids[i]);
            int value = prop (row);
            results.append(db->timeToString(value));
        }
    }
    return results;
}

QString View::getNote(int rowId, int colIndex)
{
    int index = dbview.Find(Id [rowId]);
    c4_StringProp prop(ids[colIndex]);
    return QString::fromUtf8(prop (dbview[index]));
}

QString View::getImageFormat(int rowId, int colIndex)
{
    int index = dbview.Find(Id [rowId]);
    c4_StringProp prop(scIds[colIndex]);
    return QString::fromUtf8(prop (dbview[index]));
}

QImage View::getImage(int rowId, int colIndex)
{
    QString colName = columns[colIndex];
    QString format = getImageFormat(rowId, colIndex);
    return ImageUtils::load(db, rowId, colName, format);
}

void View::sort(int colIndex)
{
    if (colIndex != sortColumn) {
        ascending = TRUE;
        sortColumn = colIndex;
    }
    else {
        ascending = !ascending;
    }
}

void View::sort(QString sortingName)
{
    sortColumn = -1;
    sortName = sortingName;
}

void View::prepareData()
{
    dbview = db->getData();
    Filter *filter = db->getFilter(db->currentFilter());
    dbview = filter->apply(dbview);
    if (sortColumn != -1) {
        dbview = db->sortData(dbview, columns[sortColumn], ascending);
    }
    else {
        // if sortName is "", just returns the unsorted data
        dbview = db->sortData(dbview, sortName);
    }
}

int View::getId(int index)
{
    return Id (dbview[index]);
}

void View::deleteAllRows()
{
    // delete by descending ID so the IDs don't change with each deletion
    c4_View sorted = dbview.SortOn(Id);
    int count = dbview.GetSize();
    int *ids = new int[count];
    int i;
    for (i = 0; i < count; i++) {
        ids[i] = Id (sorted[count - i - 1]);
    }
    // do deletions after obtaining all IDs so we don't need to worry about
    // the view changing
    for (i = 0; i < count; i++) {
        db->deleteRow(ids[i]);
    }
    delete[] ids;
}

void View::exportToCSV(QString filename)
{
    QFile f(filename);
    f.open(IO_WriteOnly);
    QTextStream output(&f);
    output.setEncoding(QTextStream::UnicodeUTF8);
    CSVUtils csv;
    int size = dbview.GetSize();
    ImageUtils utils;
    utils.setExportPaths(filename);
    for (int i = 0; i < size; i++) {
        QStringList row = db->getRow(Id (dbview[i]), &utils);
        output << csv.encodeRow(row);
    }
    f.close();
}

void View::exportToXML(QString filename)
{
    c4_View fullView = db->getData();
    if (sortColumn != -1) {
        fullView = db->sortData(fullView, columns[sortColumn], ascending);
    }
    else {
        // if sortName is "", just returns the unsorted data
        fullView = db->sortData(fullView, sortName);
    }
    Filter *filter = db->getFilter(db->currentFilter());
    c4_View currView = filter->apply(fullView);
    db->exportToXML(filename, fullView, currView, columns);
}

QStringList View::getStatistics(int colIndex)
{
    QStringList lines;
    int count = getRowCount();
    if (count == 0) {
        lines.append(tr("No data to summarize"));
        return lines;
    }
    int type = dataTypes[colIndex];
    if (type == INTEGER || type == SEQUENCE) {
        c4_IntProp prop(ids[colIndex]);
        int value = prop (dbview[0]);
        int total = value;
        int min = value;
        int max = value;
        for (int i = 1; i < count; i++) {
            int value = prop (dbview[i]);
            total += value;
            min = QMIN(min, value);
            max = QMAX(max, value);
        }
        float mean = total / (float)count;
        lines.append(tr("Total") + ": " + QString::number(total));
        lines.append(tr("Average") + ": " + QString::number(mean));
        lines.append(tr("Minimum") + ": " + QString::number(min));
        lines.append(tr("Maximum") + ": " + QString::number(max));
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
        for (int i = 1; i < count; i++) {
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
        double mean = total / count;
        lines.append(tr("Total") + ": " + QString::number(total));
        lines.append(tr("Average") + ": " + QString::number(mean));
        lines.append(tr("Minimum") + ": " + minString);
        lines.append(tr("Maximum") + ": " + maxString);
    }
    else if (type == BOOLEAN) {
        c4_IntProp prop(ids[colIndex]);
        int checked = 0;
        for (int i = 0; i < count; i++) {
            int value = prop (dbview[i]);
            if (value == 1) {
                checked++;
            }
        }
        lines.append(tr("Checked") + ": " + QString::number(checked));
        lines.append(tr("Unchecked") + ": "
                     + QString::number(count - checked));
    }
    else if (type == DATE) {
        c4_IntProp prop(ids[colIndex]);
        int value = prop (dbview[0]);
        int min = value;
        int max = value;
        for (int i = 1; i < count; i++) {
            int value = prop (dbview[i]);
            min = QMIN(min, value);
            max = QMAX(max, value);
        }
        lines.append(tr("Earliest") + ": " + db->dateToString(min));
        lines.append(tr("Latest") + ": " + db->dateToString(max));
    }
    else if (type == TIME) {
        c4_IntProp prop(ids[colIndex]);
        int value = prop (dbview[0]);
        int min = value;
        int max = value;
        for (int i = 1; i < count; i++) {
            int value = prop (dbview[i]);
            min = QMIN(min, value);
            max = QMAX(max, value);
        }
        lines.append(tr("Earliest") + ": " + db->timeToString(min));
        lines.append(tr("Latest") + ": " + db->timeToString(max));
    }
    else if (type == STRING || type == NOTE) {
        c4_StringProp prop(ids[colIndex]);
        QString value = QString::fromUtf8(prop (dbview[0]));
        int length = value.length();
        int total = length;
        int min = length;
        int max = length;
        for (int i = 1; i < count; i++) {
            QString svalue = QString::fromUtf8(prop (dbview[i]));
            length = svalue.length();
            total += length;
            min = QMIN(min, length);
            max = QMAX(max, length);
        }
        float mean = total / (float)count;
        lines.append(tr("Total length") + ": " + QString::number(total) + " "
                     + tr("characters"));
        lines.append(tr("Average length") + ": " + QString::number(mean) + " "
                     + tr("characters"));
        lines.append(tr("Minimum length") + ": " + QString::number(min) + " "
                     + tr("characters"));
        lines.append(tr("Maximum length") + ": " + QString::number(max) + " "
                     + tr("characters"));
    }
    else if (type == IMAGE) {
        c4_StringProp stringProp(scIds[colIndex]);
        int missing = 0;
        for (int i = 0; i < count; i++) {
            QString value(stringProp (dbview[i]));
            if (value == "") {
                missing++;
            }
        }
        lines.append(tr("Image available") + ": "
                     + QString::number(count - missing));
        lines.append(tr("No image") + ": " + QString::number(missing));
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
        for (i = 0; i < count; i++) {
            int index = options.findIndex(QString::fromUtf8(prop (dbview[i])));
            tallies[index] = tallies[index] + 1;
        }
        for (i = 0; i < optionCount; i++) {
            lines.append(options[i] + ": " + QString::number(tallies[i]));
        }
        delete[] tallies;
    }
    return lines;
}

void View::copyStateFrom(View *otherView)
{
    sortColumn = otherView->sortColumn;
    ascending = otherView->ascending;
    sortName = otherView->sortName;
    prepareData();
}
