/*
 * database.cpp
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qpe/resource.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include "csvutils.h"
#include "database.h"
#include "portabase.h"
#include "view.h"

Database::Database(QString path, bool *ok) : curView(0), Id("_id"), cIndex("_cindex"), cName("_cname"), cType("_ctype"), cDefault("_cdefault"), vName("_vname"), vRpp("_vrpp"), vcView("_vcview"), vcIndex("_vcindex"), vcName("_vcname"), vcWidth("_vcwidth"), vcSortIndex("_vcsortindex"), vcAscending("_vcascending"), vcFilter("_vcfilter"), gVersion("_gversion"), gView("_gview")
{
    checkedPixmap = Resource::loadPixmap("portabase/checked");
    uncheckedPixmap = Resource::loadPixmap("portabase/unchecked");

    file = new c4_Storage(path, TRUE);
    global = file->GetAs("_global[_gversion:I,_gview:S]");
    if (global.GetSize() == 0) {
        global.Add(gVersion [FILE_VERSION] + gView ["_all"]);
        *ok = TRUE;
    }
    else if (gVersion (global[0]) > FILE_VERSION) {
        *ok = FALSE;
    }
    else {
        *ok = TRUE;
    }
    if (*ok) {
        columns = file->GetAs("_columns[_cindex:I,_cname:S,_ctype:I,_cdefault:S]");
        views = file->GetAs("_views[_vname:S,_vrpp:I]");
        viewColumns = file->GetAs("_viewcolumns[_vcview:S,_vcindex:I,_vcname:S,_vcwidth:I,_vcsortindex:I,_vcascending:I,_vcfilter:S]");
        data = file->GetAs(formatString());
        maxId = data.GetSize() - 1;
        *ok = TRUE;
    }
}

Database::~Database()
{
    delete file;
    if (curView) {
        delete curView;
    }
}

QString Database::currentView()
{
    QString viewName(gView(global[0]));
    return viewName;
}

View *Database::getView(QString name)
{
    int index = views.Find(vName [name]);
    int rpp = vRpp (views[index]);
    c4_View cols = viewColumns.Select(vcView [name]);
    cols = cols.SortOn(vcIndex);
    int size = cols.GetSize();
    QStringList names;
    int *types = new int[size];
    int *widths = new int[size];
    for (int i = 0; i < size; i++) {
        QString name(vcName (cols[i]));
        names.append(name);
        int colIndex = columns.Find(cName [name]);
        types[i] = cType (columns[colIndex]);
        widths[i] = vcWidth (cols[i]);
    }
    if (curView) {
        delete curView;
    }
    gView(global[0]) = name;
    curView = new View(this, data, names, types, widths, rpp);
    return curView;
}

QStringList Database::listViews()
{
    c4_View sorted = views.SortOn(vName);
    int size = sorted.GetSize();
    QStringList list;
    for (int i = 0; i < size; i++) {
        QString name(vName (sorted[i]));
        list.append(name);
    }
    return list;
}

void Database::addView(QString name, QStringList names)
{
    c4_Row row;
    vName (row) = name;
    vRpp (row) = 13;
    views.Add(row);
    int count = names.count();
    for (int i = 0; i < count; i++) {
        c4_Row colRow;
        vcView (colRow) = name;
        vcIndex (colRow) = i;
        vcName (colRow) = names[i];
        vcWidth (colRow) = 60;
        vcSortIndex (colRow) = 0;
        vcAscending (colRow) = 0;
        vcFilter (colRow) = "";
        viewColumns.Add(colRow);
    }
}

void Database::deleteView(QString name)
{
    int index = views.Find(vName [name]);
    views.RemoveAt(index);
    // delete the view's columns
    int nextIndex = viewColumns.Find(vcView [name]);
    while (nextIndex != -1) {
        viewColumns.RemoveAt(nextIndex);
        nextIndex = viewColumns.Find(vcView [name]);
    }
}

void Database::renameView(QString oldName, QString newName)
{
    if (newName == oldName) {
        return;
    }
    int index = views.Find(vName [oldName]);
    vName (views[index]) = newName;
    // rename references to the view in its columns
    int nextIndex = viewColumns.Find(vcView [oldName]);
    while (nextIndex != -1) {
        vcView (viewColumns[nextIndex]) = newName;
        nextIndex = viewColumns.Find(vcView [oldName]);
    }
}

void Database::setViewColWidths(int *widths)
{
    QString viewName(gView(global[0]));
    int i = 0;
    int colIndex = viewColumns.Find(vcView [viewName] + vcIndex [i]);
    while (colIndex != -1) {
        vcWidth (viewColumns[colIndex]) = widths[i];
        i++;
        colIndex = viewColumns.Find(vcView [viewName] + vcIndex [i]);
    }
}

void Database::setViewRowsPerPage(int rpp)
{
    QString viewName(gView(global[0]));
    int index = views.Find(vName [viewName]);
    vRpp (views[index]) = rpp;
}

QStringList Database::listColumns()
{
    c4_View sorted = columns.SortOn(cIndex);
    int size = sorted.GetSize();
    QStringList list;
    for (int i = 0; i < size; i++) {
        QString name(cName (sorted[i]));
        list.append(name);
    }
    return list;
}

int *Database::listTypes()
{
    c4_View sorted = columns.SortOn(cIndex);
    int size = sorted.GetSize();
    int *list = new int[size];
    for (int i = 0; i < size; i++) {
        list[i] = cType (sorted[i]);
    }
    return list;
}

int Database::getIndex(QString column)
{
    int index = columns.Find(cName [column]);
    return cIndex (columns[index]);
}

void Database::setIndex(QString column, int index)
{
    int i = columns.Find(cName [column]);
    cIndex (columns[i]) = index;
}

int Database::getType(QString column)
{
    int index = columns.Find(cName [column]);
    return cType (columns[index]);
}

QString Database::getDefault(QString column)
{
    int index = columns.Find(cName [column]);
    return QString(cDefault (columns[index]));
}

void Database::setDefault(QString column, QString value)
{
    int index = columns.Find(cName [column]);
    cDefault (columns[index]) = value;
}

QString Database::isValidValue(int type, QString value)
{
    if (type == INTEGER) {
        bool ok = FALSE;
        value.toInt(&ok);
        if (!ok) {
            return "must be an integer";
        }
    }
    else if (type == FLOAT) {
        bool ok = FALSE;
        value.toDouble(&ok);
        if (!ok) {
            return "must be a decimal value";
        }
    }
    else if (type == BOOLEAN) {
        bool ok = FALSE;
        int val = value.toInt(&ok);
        if (!ok || val < 0 || val > 1) {
            return "must be 0 or 1";
        }
    }
    return "";
}

void Database::addColumn(int index, QString name, int type, QString defaultVal)
{
    columns.Add(cIndex [index] + cName [name] + cType [type]
                + cDefault [defaultVal]);
    addViewColumn("_all", name);
    int size = data.GetSize();
    if (type == INTEGER || type == BOOLEAN) {
        c4_IntProp newProp(name);
        int value = defaultVal.toInt();
        for (int i = 0; i < size; i++) {
            newProp (data[i]) = value;
        }
    }
    else if (type == FLOAT) {
        c4_FloatProp newProp(name);
        double value = defaultVal.toDouble();
        for (int i = 0; i < size; i++) {
            newProp (data[i]) = value;
        }
    }
    else {
        c4_StringProp newProp(name);
        for (int i = 0; i < size; i++) {
            newProp (data[i]) = defaultVal;
        }
    }
}

void Database::deleteColumn(QString name)
{
    // first make sure it exists
    int index = columns.Find(cName [name]);
    if (index == -1) {
        return;
    }
    // remove the column from any views containing it
    int count = views.GetSize();
    for (int i = 0; i < count; i++) {
        QString viewName(vName (views[i]));
        deleteViewColumn(viewName, name);
    }
    // remove the column from the definition
    columns.RemoveAt(index);
}

void Database::renameColumn(QString oldName, QString newName)
{
    // rename the column in any views containing it
    int nextIndex = viewColumns.Find(vcName [oldName]);
    while (nextIndex != -1) {
        vcName (viewColumns[nextIndex]) = newName;
        nextIndex = viewColumns.Find(vcName [oldName]);
    }
    // rename the column in the format definition
    int index = columns.Find(cName [oldName]);
    cName (columns[index]) = newName;
    // create a new data column, copying the contents of the old
    int type = cType (columns[index]);
    int size = data.GetSize();
    if (type == INTEGER || type == BOOLEAN) {
        c4_IntProp oldProp(oldName);
        c4_IntProp newProp(newName);
        for (int i = 0; i < size; i++) {
            newProp (data[i]) = oldProp (data[i]);
        }
    }
    else if (type == FLOAT) {
        c4_FloatProp oldProp(oldName);
        c4_FloatProp newProp(newName);
        for (int i = 0; i < size; i++) {
            newProp (data[i]) = oldProp (data[i]);
        }
    }
    else if (type == STRING) {
        c4_StringProp oldProp(oldName);
        c4_StringProp newProp(newName);
        for (int i = 0; i < size; i++) {
            newProp (data[i]) = oldProp (data[i]);
        }
    }
}

void Database::updateDataFormat()
{
    data = file->GetAs(formatString());
}

QStringList Database::getRow(int rowId)
{
    QStringList results;
    int index = data.Find(Id [rowId]);
    c4_RowRef row = data[index];
    int numCols = columns.GetSize();
    c4_View temp = columns.SortOn(cIndex);
    for (int i = 0; i < numCols; i++) {
        QString name(cName (temp[i]));
        int type = cType (temp[i]);
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
        else if (type == STRING) {
            c4_StringProp prop(name);
            QString value(prop (row));
            results.append(value);
        }
    }
    return results;
}

c4_View Database::sortData(QString column, bool ascending)
{
    int type = getType(column);
    if (type == INTEGER || type == BOOLEAN) {
        c4_IntProp prop(column);
        if (ascending) {
            return data.SortOn(prop);
        }
        else {
            return data.SortOnReverse(prop, prop);
        }
    }
    else if (type == FLOAT) {
        c4_FloatProp prop(column);
        if (ascending) {
            return data.SortOn(prop);
        }
        else {
            return data.SortOnReverse(prop, prop);
        }
    }
    else {
        c4_StringProp prop(column);
        if (ascending) {
            return data.SortOn(prop);
        }
        else {
            return data.SortOnReverse(prop, prop);
        }
    }
}

void Database::addRow(QStringList values)
{
    c4_Row row;
    Id (row) = maxId + 1;
    int count = columns.GetSize();
    c4_View temp = columns.SortOn(cIndex);
    for (int i = 0; i < count; i++) {
        int type = cType (temp[i]);
        QString name(cName (temp[i]));
        if (type == STRING) {
            c4_StringProp prop(name);
            prop (row) = values[i];
        }
        else if (type == INTEGER || type == BOOLEAN) {
            c4_IntProp prop(name);
            prop (row) = values[i].toInt();
        }
        else if (type == FLOAT) {
            c4_FloatProp prop(name);
            prop (row) = values[i].toDouble();
        }
    }
    data.Add(row);
    maxId++;
}

void Database::updateRow(int rowId, QStringList values)
{
    int index = data.Find(Id [rowId]);
    int count = columns.GetSize();
    c4_View temp = columns.SortOn(cIndex);
    for (int i = 0; i < count; i++) {
        int type = cType (temp[i]);
        QString name(cName (temp[i]));
        if (type == STRING) {
            c4_StringProp prop(name);
            prop (data[index]) = values[i];
        }
        else if (type == INTEGER || type == BOOLEAN) {
            c4_IntProp prop(name);
            prop (data[index]) = values[i].toInt();
        }
        else if (type == FLOAT) {
            c4_FloatProp prop(name);
            prop (data[index]) = values[i].toDouble();
        }
    }
}

void Database::deleteRow(int id)
{
    int index = data.Find(Id [id]);
    data.RemoveAt(index);
    int size = data.GetSize();
    for (int i = 0; i < size; i++) {
        c4_RowRef row = data[i];
        int rowId = Id (row);
        if (rowId > id) {
            Id (row) = rowId - 1;
        }
    }
    maxId--;
}

QStringList Database::listViewColumns(QString viewName)
{
    c4_View cols = viewColumns.Select(vcView [viewName]);
    cols = cols.SortOn(vcIndex);
    int size = cols.GetSize();
    QStringList names;
    for (int i = 0; i < size; i++) {
        QString name(vcName (cols[i]));
        names.append(name);
    }
    return names;
}

void Database::addViewColumn(QString viewName, QString columnName)
{
    if (views.Find(vName [viewName]) == -1) {
        // named view doesn't exist
        return;
    }
    c4_View cols = viewColumns.Select(vcView [viewName]);
    c4_Row colRow;
    vcView (colRow) = viewName;
    vcIndex (colRow) = cols.GetSize();
    vcName (colRow) = columnName;
    vcWidth (colRow) = 60;
    vcSortIndex (colRow) = 0;
    vcAscending (colRow) = 0;
    vcFilter (colRow) = "";
    viewColumns.Add(colRow);
}

void Database::deleteViewColumn(QString viewName, QString columnName)
{
    int removeIndex = viewColumns.Find(vcView [viewName]
                                       + vcName [columnName]);
    if (removeIndex == -1) {
        // no such column in this view
        return;
    }
    int position = vcIndex (viewColumns[removeIndex]);
    position++;
    int nextIndex = viewColumns.Find(vcView [viewName] + vcIndex [position]);
    while (nextIndex != -1) {
        vcIndex (viewColumns[nextIndex]) = position - 1;
        position++;
        nextIndex = viewColumns.Find(vcView [viewName] + vcIndex [position]);
    }
    viewColumns.RemoveAt(removeIndex);
}

void Database::setViewColumnSequence(QString viewName, QStringList colNames)
{
    int i = 0;
    int nextIndex = viewColumns.Find(vcView [viewName] + vcName [colNames[i]]);
    while (nextIndex != -1) {
        vcIndex (viewColumns[nextIndex]) = i;
        i++;
        nextIndex = viewColumns.Find(vcView [viewName] + vcName [colNames[i]]);
    }
}

QString Database::formatString()
{
    c4_View sorted = columns.SortOn(cIndex);
    int size = sorted.GetSize();

    QString result = "_data[_id:I";
    for (int i = 0; i < size; i++) {
        result += ',';
        c4_RowRef row = sorted[i];
        QString name(cName (row));
        result += name;
        int type = cType(row);
        if (type == INTEGER || type == BOOLEAN) {
            result += ":I";
        }
        else if (type == FLOAT) {
            result += ":F";
        }
        else {
            result += ":S";
        }
    }
    result += ']';
    return result;
}

void Database::commit()
{
    file->Commit();
}

QPixmap Database::getCheckBoxPixmap(int checked)
{
    if (checked) {
        return checkedPixmap;
    }
    else {
        return uncheckedPixmap;
    }
}

QString Database::importFromCSV(QString filename)
{
    QFile f(filename);
    if (!f.open(IO_ReadOnly)) {
        return PortaBase::tr("Unable to open file");
    }
    QTextStream input(&f);
    input.setEncoding(QTextStream::Locale);
    CSVUtils csv;
    QString line;
    int colCount = columns.GetSize();
    QStringList names = listColumns();
    int *types = listTypes();
    int rowNum = 1;
    QString message = "";
    while (!input.eof()) {
        line = input.readLine();
        QStringList values = csv.getRow(line);
        if ((int)values.count() != colCount) {
            message = PortaBase::tr("Error in line") + " "
                      + QString::number(rowNum) + "\n"
                      + PortaBase::tr("Wrong number of columns");
            break;
        }
        for (int i = 0; i < colCount; i++) {
            message = isValidValue(types[i], values[i]);
            if (message != "") {
                message = PortaBase::tr("Error in line") + " "
                          + QString::number(rowNum) + "\n" + names[i] + " "
                          + PortaBase::tr(message);
                break;
            }
        }
        if (message != "") {
            break;
        }
        addRow(values);
        rowNum++;
    }
    delete[] types;
    f.close();
    return message;
}

void Database::exportToCSV(QString filename)
{
    QFile f(filename);
    f.open(IO_WriteOnly);
    QTextStream output(&f);
    output.setEncoding(QTextStream::Locale);
    CSVUtils csv;
    int size = data.GetSize();
    for (int i = 0; i < size; i++) {
        QStringList row = getRow(i);
        output << csv.encodeRow(row);
    }
    f.close();
}
