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
#include "portabase.h"
#include "view.h"

View::View(Database *parent, c4_View baseview, QStringList colNames, 
    int *types, int *widths, int rpp) : Id("_id"), sortColumn(-1),
    ascending(TRUE)
{
    db = parent;
    dbview = baseview;
    columns = colNames;
    dataTypes = types;
    colWidths = widths;
    numCols = columns.count();
    rowsPerPage = rpp;
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
        QString name = columns[i];
        int type = dataTypes[i];
        if (type == INTEGER || type == BOOLEAN) {
            c4_IntProp prop(name);
            int value = prop (row);
            results.append(QString::number(value));
        }
        else if (type == FLOAT) {
            c4_FloatProp prop(name);
            double value = prop (row);
            results.append(QString::number(value));
        }
        else if (type == STRING || type == NOTE) {
            c4_StringProp prop(name);
            QString value(prop (row));
            results.append(value);
        }
    }
    return results;
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

void View::prepareData()
{
    if (sortColumn != -1) {
        dbview = db->sortData(columns[sortColumn], ascending);
    }
}

int View::getId(int index)
{
    return Id (dbview[index]);
}

QStringList View::getStatistics(int colIndex)
{
    QStringList lines;
    int count = getRowCount();
    if (count == 0) {
        lines.append(PortaBase::tr("No data to summarize"));
        return lines;
    }
    QString name = columns[colIndex];
    int type = dataTypes[colIndex];
    if (type == INTEGER) {
        c4_IntProp prop(name);
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
        c4_FloatProp prop(name);
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
        c4_IntProp prop(name);
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
    else if (type == STRING || type == NOTE) {
        c4_StringProp prop(name);
        QString value(prop (dbview[0]));
        int length = value.length();
        int total = length;
        int min = length;
        int max = length;
        for (int i = 1; i < count; i++) {
            QString svalue(prop (dbview[i]));
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
