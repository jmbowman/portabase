/*
 * view.cpp
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <mk4.h>
#include <qmessagebox.h>
#include <qstringlist.h>
#include "database.h"
#include "datatypes.h"
#include "filter.h"
#include "portabase.h"
#include "view.h"

View::View(Database *parent, c4_View baseview, QStringList colNames, 
    int *types, int *widths, QStringList colIds, int rpp) : Id("_id"),
    sortColumn(-1), ascending(TRUE), sortName("")
{
    db = parent;
    dbview = baseview;
    columns = colNames;
    dataTypes = types;
    colWidths = widths;
    ids = colIds;
    numCols = columns.count();
    rowsPerPage = rpp;
    sort(db->currentSorting());
}

View::~View()
{
    delete[] dataTypes;
    delete[] colWidths;
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
        if (type == INTEGER || type == BOOLEAN) {
            c4_IntProp prop(ids[i]);
            int value = prop (row);
            results.append(QString::number(value));
        }
        else if (type == FLOAT) {
            c4_FloatProp prop(ids[i]);
            double value = prop (row);
            results.append(QString::number(value));
        }
        else if (type == STRING || type == NOTE) {
            c4_StringProp prop(ids[i]);
            results.append(QString::fromUtf8(prop (row)));
        }
        else if (type == DATE) {
            c4_IntProp prop(ids[i]);
            int value = prop (row);
            results.append(db->dateToString(value));
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
    for (int i = 0; i < count; i++) {
        ids[i] = Id (sorted[count - i - 1]);
    }
    // do deletions after obtaining all IDs so we don't need to worry about
    // the view changing
    for (int i = 0; i < count; i++) {
        db->deleteRow(ids[i]);
    }
    delete[] ids;
}

QStringList View::getStatistics(int colIndex)
{
    QStringList lines;
    int count = getRowCount();
    if (count == 0) {
        lines.append(PortaBase::tr("No data to summarize"));
        return lines;
    }
    int type = dataTypes[colIndex];
    if (type == INTEGER) {
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
        lines.append(PortaBase::tr("Total") + ": " + QString::number(total));
        lines.append(PortaBase::tr("Average") + ": " + QString::number(mean));
        lines.append(PortaBase::tr("Minimum") + ": " + QString::number(min));
        lines.append(PortaBase::tr("Maximum") + ": " + QString::number(max));
    }
    else if (type == FLOAT) {
        c4_FloatProp prop(ids[colIndex]);
        double value = prop (dbview[0]);
        double total = value;
        double min = value;
        double max = value;
        for (int i = 1; i < count; i++) {
            double value = prop (dbview[i]);
            total += value;
            min = QMIN(min, value);
            max = QMAX(max, value);
        }
        double mean = total / count;
        lines.append(PortaBase::tr("Total") + ": " + QString::number(total));
        lines.append(PortaBase::tr("Average") + ": " + QString::number(mean));
        lines.append(PortaBase::tr("Minimum") + ": " + QString::number(min));
        lines.append(PortaBase::tr("Maximum") + ": " + QString::number(max));
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
        lines.append(PortaBase::tr("Checked") + ": "
                     + QString::number(checked));
        lines.append(PortaBase::tr("Unchecked") + ": "
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
        lines.append(PortaBase::tr("Earliest") + ": " + db->dateToString(min));
        lines.append(PortaBase::tr("Latest") + ": " + db->dateToString(max));
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
        lines.append(PortaBase::tr("Total length") + ": "
                     + QString::number(total) + " "
                     + PortaBase::tr("characters"));
        lines.append(PortaBase::tr("Average length") + ": "
                     + QString::number(mean) + " "
                     + PortaBase::tr("characters"));
        lines.append(PortaBase::tr("Minimum length") + ": "
                     + QString::number(min) + " "
                     + PortaBase::tr("characters"));
        lines.append(PortaBase::tr("Maximum length") + ": "
                     + QString::number(max) + " "
                     + PortaBase::tr("characters"));
    }
    return lines;
}
