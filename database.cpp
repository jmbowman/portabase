/*
 * database.cpp
 *
 * (c) 2002-2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#if defined(DESKTOP)
#include "desktop/config.h"
#include "desktop/resource.h"
#else
#include <qpe/config.h>
#include <qpe/resource.h>
#endif

#include <qdatetime.h>
#include <qmessagebox.h>
#include <qobject.h>
#include <qregexp.h>
#include <qstringlist.h>
#include "condition.h"
#include "csvutils.h"
#include "database.h"
#include "filter.h"
#include "view.h"
#include "xmlexport.h"

Database::Database(QString path, bool *ok) : curView(0), curFilter(0), Id("_id"), cIndex("_cindex"), cName("_cname"), cType("_ctype"), cDefault("_cdefault"), cId("_cid"), vName("_vname"), vRpp("_vrpp"), vDeskRpp("_vdeskrpp"), vcView("_vcview"), vcIndex("_vcindex"), vcName("_vcname"), vcWidth("_vcwidth"), vcDeskWidth("_vcdeskwidth"), sName("_sname"), scSort("_scsort"), scIndex("_scindex"), scName("_scname"), scDesc("_scdesc"), fName("_fname"), fcFilter("_fcfilter"), fcPosition("_fcposition"), fcColumn("_fccolumn"), fcOperator("_fcoperator"), fcConstant("_fcconstant"), fcCase("_fccase"), eName("_ename"), eId("_eid"), eIndex("_eindex"), eoEnum("_eoenum"), eoIndex("_eoindex"), eoText("_eotext"), gVersion("_gversion"), gView("_gview"), gSort("_gsort"), gFilter("_gfilter")
{
    c4_View::_CaseSensitive = TRUE;
    checkedPixmap = Resource::loadPixmap("portabase/checked");
    uncheckedPixmap = Resource::loadPixmap("portabase/unchecked");
    updateDateTimePrefs();
    Config pbConfig("portabase");
    pbConfig.setGroup("General");
    showSeconds = pbConfig.readBoolEntry("ShowSeconds");

    file = new c4_Storage(path, TRUE);
    global = file->GetAs("_global[_gversion:I,_gview:S,_gsort:S,_gfilter:S]");
    int version = 0;
    bool newFile = FALSE;
    if (global.GetSize() == 0) {
        // new file, add global data
        global.Add(gVersion [FILE_VERSION] + gView ["_all"] + gSort [""]
                   + gFilter ["_allrows"]);
        *ok = TRUE;
        newFile = TRUE;
        version = FILE_VERSION;
    }
    else if (gVersion (global[0]) > FILE_VERSION) {
        // trying to open a newer version of the file format
        *ok = FALSE;
    }
    else {
        version = gVersion (global[0]);
        if (version < 2) {
            // sorting name added in file version 2
            gSort (global[0]) = "";
        }
        if (version < 3) {
            // filters added in file version 3
            gFilter (global[0]) = "_allrows";
        }
        if (version < FILE_VERSION) {
            gVersion (global[0]) = FILE_VERSION;
        }
        *ok = TRUE;
    }
    if (*ok) {
        columns = file->GetAs("_columns[_cindex:I,_cname:S,_ctype:I,_cdefault:S,_cid:I]");
        views = file->GetAs("_views[_vname:S,_vrpp:I,_vdeskrpp:I]");
        viewColumns = file->GetAs("_viewcolumns[_vcview:S,_vcindex:I,_vcname:S,_vcwidth:I,_vcdeskwidth:I]");
        sorts = file->GetAs("_sorts[_sname:S]");
        sortColumns = file->GetAs("_sortcolumns[_scsort:S,_scindex:I,_scname:S,_scdesc:I]");
        filters = file->GetAs("_filters[_fname:S]");
        filterConditions = file->GetAs("_filterconditions[_fcfilter:S,_fcposition:I,_fccolumn:S,_fcoperator:I,_fcconstant:S,_fccase:I]");
        enums = file->GetAs("_enums[_ename:S,_eid:I,_eindex:I]");
        enumOptions = file->GetAs("_enumoptions[_eoenum:I,_eoindex:I,_eotext:S]");
        if (version < 3 || newFile) {
            filters.Add(fName ["_allrows"]);
        }
        if (version < 4) {
            // adapt to using column IDs instead of names in data view
            updateDataColumnFormat();
            // change from Latin-1 to UTF-8
            updateEncoding();
        }
        else {
            data = file->GetAs(formatString());
        }
        if (version < 6) {
            fixConditionIndices();
        }
        if (version < 7) {
            addDesktopStats();
        }
        maxId = data.GetSize() - 1;
    }
}

Database::~Database()
{
    delete file;
    if (curView) {
        delete curView;
    }
    if (curFilter) {
        delete curFilter;
    }
}

void Database::updateDateTimePrefs()
{
#if defined(DESKTOP)
    PBDateFormat format = TimeString::currentDateFormat();
#else
    DateFormat format = TimeString::currentDateFormat();
#endif
    dateOrder = format.shortOrder();
    dateSeparator = format.separator();
    Config qpeConfig("qpe");
    qpeConfig.setGroup("Time");
    ampm = qpeConfig.readBoolEntry("AMPM");
}

QString Database::currentView()
{
    return QString::fromUtf8(gView(global[0]));
}

View *Database::getView(QString name)
{
    int index = views.Find(vName [name.utf8()]);
#if defined(DESKTOP)
    int rpp = vDeskRpp (views[index]);
#else
    int rpp = vRpp (views[index]);
#endif
    c4_View cols = viewColumns.Select(vcView [name.utf8()]);
    cols = cols.SortOn(vcIndex);
    int size = cols.GetSize();
    QStringList names;
    int *types = new int[size];
    int *widths = new int[size];
    QStringList colIds;
    QStringList floatStringIds;
    for (int i = 0; i < size; i++) {
        QString name = QString::fromUtf8(vcName (cols[i]));
        names.append(name);
        int colIndex = columns.Find(cName [name.utf8()]);
        types[i] = cType (columns[colIndex]);
#if defined(DESKTOP)
        widths[i] = vcDeskWidth (cols[i]);
#else
        widths[i] = vcWidth (cols[i]);
#endif
        int idNum = cId (columns[colIndex]);
        colIds.append(makeColId(idNum, types[i]));
        if (types[i] == FLOAT) {
            floatStringIds.append(makeColId(idNum, STRING));
        }
        else {
            floatStringIds.append("");
        }
    }
    if (curView) {
        delete curView;
    }
    gView(global[0]) = name.utf8();
    curView = new View(this, data, names, types, widths, colIds,
                       floatStringIds, rpp);
    return curView;
}

QStringList Database::listViews()
{
    c4_View sorted = views.SortOn(vName);
    int size = sorted.GetSize();
    QStringList list;
    for (int i = 0; i < size; i++) {
        list.append(QString::fromUtf8(vName (sorted[i])));
    }
    return list;
}

void Database::addView(QString name, QStringList names, int rpp, int deskrpp)
{
    c4_Row row;
    vName (row) = name.utf8();
    vRpp (row) = (rpp == -1) ? 13 : rpp;
    vDeskRpp (row) = (deskrpp == -1) ? 25 : deskrpp;
    views.Add(row);
    int count = names.count();
    for (int i = 0; i < count; i++) {
        c4_Row colRow;
        vcView (colRow) = name.utf8();
        vcIndex (colRow) = i;
        vcName (colRow) = names[i].utf8();
        vcWidth (colRow) = 60;
        vcDeskWidth (colRow) = 120;
        viewColumns.Add(colRow);
    }
}

void Database::deleteView(QString name)
{
    QCString utf8Name = name.utf8();
    int index = views.Find(vName [utf8Name]);
    views.RemoveAt(index);
    // delete the view's columns
    int nextIndex = viewColumns.Find(vcView [utf8Name]);
    while (nextIndex != -1) {
        viewColumns.RemoveAt(nextIndex);
        nextIndex = viewColumns.Find(vcView [utf8Name]);
    }
}

void Database::renameView(QString oldName, QString newName)
{
    if (newName == oldName) {
        return;
    }
    QCString utf8OldName = oldName.utf8();
    QCString utf8NewName = newName.utf8();
    int index = views.Find(vName [utf8OldName]);
    vName (views[index]) = utf8NewName;
    // rename references to the view in its columns
    int nextIndex = viewColumns.Find(vcView [utf8OldName]);
    while (nextIndex != -1) {
        vcView (viewColumns[nextIndex]) = utf8NewName;
        nextIndex = viewColumns.Find(vcView [utf8OldName]);
    }
}

void Database::setViewColWidths(int *widths)
{
    QCString viewName(gView(global[0]));
    int i = 0;
    int colIndex = viewColumns.Find(vcView [viewName] + vcIndex [i]);
    while (colIndex != -1) {
#if defined(DESKTOP)
        vcDeskWidth (viewColumns[colIndex]) = widths[i];
#else
        vcWidth (viewColumns[colIndex]) = widths[i];
#endif
        i++;
        colIndex = viewColumns.Find(vcView [viewName] + vcIndex [i]);
    }
}

void Database::setViewRowsPerPage(int rpp)
{
    QCString viewName(gView(global[0]));
    int index = views.Find(vName [viewName]);
#if defined(DESKTOP)
    vDeskRpp (views[index]) = rpp;
#else
    vRpp (views[index]) = rpp;
#endif
}

QStringList Database::listColumns()
{
    c4_View sorted = columns.SortOn(cIndex);
    int size = sorted.GetSize();
    QStringList list;
    for (int i = 0; i < size; i++) {
        list.append(QString::fromUtf8(cName (sorted[i])));
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
    int index = columns.Find(cName [column.utf8()]);
    return cIndex (columns[index]);
}

void Database::setIndex(QString column, int index)
{
    int i = columns.Find(cName [column.utf8()]);
    cIndex (columns[i]) = index;
}

int Database::getType(QString column)
{
    int index = columns.Find(cName [column.utf8()]);
    return cType (columns[index]);
}

QString Database::getDefault(QString column)
{
    int index = columns.Find(cName [column.utf8()]);
    return QString::fromUtf8(cDefault (columns[index]));
}

void Database::setDefault(QString column, QString value)
{
    int index = columns.Find(cName [column.utf8()]);
    cDefault (columns[index]) = value.utf8();
}

QString Database::getColId(QString column)
{
    int index = columns.Find(cName [column.utf8()]);
    return makeColId(cId (columns[index]), cType (columns[index]));
}

QString Database::isValidValue(int type, QString value)
{
    if (type == INTEGER) {
        bool ok = FALSE;
        value.toInt(&ok);
        if (!ok) {
            return QObject::tr("must be an integer");
        }
    }
    else if (type == FLOAT) {
        bool ok = FALSE;
        value.toDouble(&ok);
        if (!ok) {
            return QObject::tr("must be a decimal value");
        }
    }
    else if (type == BOOLEAN) {
        bool ok = FALSE;
        int val = value.toInt(&ok);
        if (!ok || val < 0 || val > 1) {
            return QObject::tr("must be 0 or 1");
        }
    }
    else if (type == DATE) {
        if (value.length() != 8) {
            return QObject::tr("invalid date");
        }
        int y = value.left(4).toInt();
        int m = value.mid(4, 2).toInt();
        int d = value.right(2).toInt();
        if (!QDate::isValid(y, m, d)) {
            return QObject::tr("invalid date");
        }
    }
    else if (type == TIME) {
        bool ok;
        parseTimeString(value, &ok);
        if (!ok) {
            return QObject::tr("invalid time");
        }
    }
    else if (type >= FIRST_ENUM) {
        QStringList options = listEnumOptions(type);
        if (options.findIndex(value) == -1) {
            return QObject::tr("no such option");
        }
    }
    return "";
}

void Database::addColumn(int index, QString name, int type, QString defaultVal,
                         int id)
{
    QCString utf8Value = defaultVal.utf8();
    int nextId = id;
    if (nextId == -1) {
        nextId = 0;
        // Find the next available column ID number
        c4_View idsort = columns.SortOn(cId);
        int colCount = columns.GetSize();
        for (int i = 0; i < colCount; i++) {
            if (nextId == cId (idsort[i])) {
                nextId++;
            }
            else {
                // found a gap in the ID sequence (a column was deleted)
                break;
            }
        }
    }
    columns.Add(cIndex [index] + cName [name.utf8()] + cType [type]
                + cDefault [utf8Value] + cId [nextId]);
    addViewColumn("_all", name);
    int size = data.GetSize();
    QString idString = makeColId(nextId, type);
    if (type == INTEGER || type == BOOLEAN) {
        c4_IntProp newProp(idString);
        int value = defaultVal.toInt();
        for (int i = 0; i < size; i++) {
            newProp (data[i]) = value;
        }
    }
    else if (type == DATE) {
        c4_IntProp newProp(idString);
        int value = defaultVal.toInt();
        if (value == TODAY) {
            QDate date = QDate::currentDate();
            value = date.year() * 10000 + date.month() * 100 + date.day();
        }
        for (int i = 0; i < size; i++) {
            newProp (data[i]) = value;
        }
    }
    else if (type == TIME) {
        c4_IntProp newProp(idString);
        int value = defaultVal.toInt();
        if (value == NOW) {
            QTime time = QTime::currentTime();
            value = time.hour() * 3600 + time.minute() * 60 + time.second();
        }
        else {
            value = -1;
        }
        for (int i = 0; i < size; i++) {
            newProp (data[i]) = value;
        }
    }
    else if (type == FLOAT) {
        c4_FloatProp newProp(idString);
        // separate column to store number strings as they are entered
        QString stringColId = makeColId(nextId, STRING);
        c4_StringProp stringProp(stringColId);
        double value = defaultVal.toDouble();
        for (int i = 0; i < size; i++) {
            newProp (data[i]) = value;
            stringProp (data[i]) = defaultVal.utf8();
        }
    }
    else {
        c4_StringProp newProp(idString);
        for (int i = 0; i < size; i++) {
            newProp (data[i]) = utf8Value;
        }
    }
}

void Database::deleteColumn(QString name)
{
    // first make sure it exists
    int index = columns.Find(cName [name.utf8()]);
    if (index == -1) {
        return;
    }
    // remove the column from any views containing it
    int count = views.GetSize();
    int i;
    for (i = 0; i < count; i++) {
        deleteViewColumn(QString::fromUtf8(vName (views[i])), name);
    }
    // remove the column from any sortings containing it
    count = sorts.GetSize();
    for (i = 0; i < count; i++) {
        deleteSortingColumn(QString::fromUtf8(sName (sorts[i])), name);
    }
    // remove the column from any filters containing it
    count = filters.GetSize();
    for (i = 0; i < count; i++) {
        deleteFilterColumn(QString::fromUtf8(fName (filters[i])), name);
    }
    // remove the column from the definition
    columns.RemoveAt(index);
}

void Database::renameColumn(QString oldName, QString newName)
{
    QCString utf8OldName = oldName.utf8();
    QCString utf8NewName = newName.utf8();
    // rename the column in any views containing it
    int nextIndex = viewColumns.Find(vcName [utf8OldName]);
    while (nextIndex != -1) {
        vcName (viewColumns[nextIndex]) = utf8NewName;
        nextIndex = viewColumns.Find(vcName [utf8OldName]);
    }
    // rename the column in any sortings containing it
    nextIndex = sortColumns.Find(scName [utf8OldName]);
    while (nextIndex != -1) {
        scName (sortColumns[nextIndex]) = utf8NewName;
        nextIndex = sortColumns.Find(scName [utf8OldName]);
    }
    // rename the column in any filters containing it
    nextIndex = filterConditions.Find(fcColumn [utf8OldName]);
    while (nextIndex != -1) {
        fcColumn (filterConditions[nextIndex]) = utf8NewName;
        nextIndex = filterConditions.Find(fcColumn [utf8OldName]);
    }
    // rename the column in the format definition
    int index = columns.Find(cName [utf8OldName]);
    cName (columns[index]) = utf8NewName;
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
        int type = cType (temp[i]);
        if (type == FLOAT) {
            // want the string representation for this
            type = STRING;
        }
        QString idString = makeColId(cId (temp[i]), type);
        if (type == INTEGER || type == BOOLEAN || type == DATE
                || type == TIME) {
            c4_IntProp prop(idString);
            int value = prop (row);
            results.append(QString::number(value));
        }
        else if (type == STRING || type == NOTE || type >= FIRST_ENUM) {
            c4_StringProp prop(idString);
            results.append(QString::fromUtf8(prop (row)));
        }
    }
    return results;
}

QString Database::currentSorting()
{
    return QString::fromUtf8(gSort (global[0]));
}

QStringList Database::listSortings()
{
    c4_View sorted = sorts.SortOn(sName);
    int size = sorted.GetSize();
    QStringList list;
    for (int i = 0; i < size; i++) {
        list.append(QString::fromUtf8(sName (sorted[i])));
    }
    return list;
}

bool Database::getSortingInfo(QString sortingName, QStringList *allCols,
                              QStringList *descCols)
{
    c4_View temp = sortColumns.Select(scSort [sortingName.utf8()]);
    int count = temp.GetSize();
    if (count == 0) {
        // non-existent or empty sorting; nothing to do
        return FALSE;
    }
    temp = temp.SortOn(scIndex);
    for (int i = 0; i < count; i++) {
        QString name = QString::fromUtf8(scName (temp[i]));
        allCols->append(name);
        if (scDesc (temp[i]) == 1) {
            descCols->append(name);
        }
    }
    return TRUE;
}

void Database::addSorting(QString name, QStringList allCols,
                          QStringList descCols)
{
    QCString utf8Name = name.utf8();
    sorts.Add(sName [utf8Name]);
    int count = allCols.count();
    for (int i = 0; i < count; i++) {
        c4_Row colRow;
        scSort (colRow) = utf8Name;
        scIndex (colRow) = i;
        scName (colRow) = allCols[i].utf8();
        if (descCols.findIndex(allCols[i]) == -1) {
            scDesc (colRow) = 0;
        }
        else {
            scDesc (colRow) = 1;
        }
        sortColumns.Add(colRow);
    }
}

void Database::deleteSorting(QString name)
{
    QCString utf8Name = name.utf8();
    int index = sorts.Find(sName [utf8Name]);
    if (index == -1) {
        return;
    }
    sorts.RemoveAt(index);
    // delete the sorting's columns
    int nextIndex = sortColumns.Find(scSort [utf8Name]);
    while (nextIndex != -1) {
        sortColumns.RemoveAt(nextIndex);
        nextIndex = sortColumns.Find(scSort [utf8Name]);
    }
}

void Database::deleteSortingColumn(QString sortName, QString columnName)
{
    QCString utf8SortName = sortName.utf8();
    int removeIndex = sortColumns.Find(scSort [utf8SortName]
                                       + scName [columnName.utf8()]);
    if (removeIndex == -1) {
        // no such column in this sorting
        return;
    }
    int position = scIndex (sortColumns[removeIndex]);
    position++;
    int nextIndex = sortColumns.Find(scSort [utf8SortName]
                                     + scIndex [position]);
    while (nextIndex != -1) {
        scIndex (sortColumns[nextIndex]) = position - 1;
        position++;
        nextIndex = sortColumns.Find(scSort [utf8SortName]
                                     + scIndex [position]);
    }
    sortColumns.RemoveAt(removeIndex);
}

c4_View Database::getData()
{
    return data;
}

c4_View Database::sortData(c4_View filteredData, QString column,
                           bool ascending)
{
    QStringList colNames;
    colNames.append(column);
    QStringList descNames;
    if (!ascending) {
        descNames.append(column);
    }
    deleteSorting("_single");
    addSorting("_single", colNames, descNames);
    gSort (global[0]) = "_single";
    c4_View sortView = createEmptyView(colNames);
    if (ascending) {
        return filteredData.SortOn(sortView);
    }
    else {
        return filteredData.SortOnReverse(sortView, sortView);
    }
}

c4_View Database::sortData(c4_View filteredData, QString sortingName)
{
    gSort (global[0]) = sortingName.utf8();
    QStringList allCols;
    QStringList descCols;
    if (!getSortingInfo(sortingName, &allCols, &descCols)) {
        return filteredData;
    }
    c4_View allView = createEmptyView(allCols);
    c4_View descView = createEmptyView(descCols);
    return filteredData.SortOnReverse(allView, descView);
}

c4_View Database::createEmptyView(QStringList colNames)
{
    int count = colNames.count();
    c4_View result;
    for (int i = 0; i < count; i++) {
        int index = columns.Find(cName [colNames[i].utf8()]);
        int id = cId (columns[index]);
        int type = cType (columns[index]);
        QString idString = makeColId(id, type);
        if (type == INTEGER || type == BOOLEAN || type == DATE
                || type == TIME) {
            c4_IntProp prop(idString);
            result.AddProperty(prop);
        }
        else if (type == FLOAT) {
            c4_FloatProp prop(idString);
            result.AddProperty(prop);
        }
        else {
            c4_StringProp prop(idString);
            result.AddProperty(prop);
        }
    }
    return result;
}

QString Database::currentFilter()
{
    return QString::fromUtf8(gFilter (global[0]));
}

QStringList Database::listFilters()
{
    c4_View sorted = filters.SortOn(fName);
    int size = sorted.GetSize();
    QStringList list;
    for (int i = 0; i < size; i++) {
        list.append(QString::fromUtf8(fName (sorted[i])));
    }
    return list;
}

Filter *Database::getFilter(QString name)
{
    if (curFilter) {
        if (QString::fromUtf8(gFilter (global[0])) == name) {
            return curFilter;
        }
        else {
            delete curFilter;
        }
    }
    gFilter (global[0]) = name.utf8();
    curFilter = new Filter(this, name);
    return curFilter;
}

void Database::addFilter(Filter *filter, bool setAsCurrent)
{
    QString name = filter->getName();
    QCString utf8Name = name.utf8();
    filters.Add(fName [utf8Name]);
    int count = filter->getConditionCount();
    for (int i = 0; i < count; i++) {
        Condition *cond = filter->getCondition(i);
        c4_Row condRow;
        fcFilter (condRow) = utf8Name;
        fcPosition (condRow) = i;
        fcColumn (condRow) = cond->getColName().utf8();
        fcOperator (condRow) = cond->getOperator();
        fcConstant (condRow) = cond->getConstant().utf8();
        if (cond->isCaseSensitive()) {
            fcCase (condRow) = 1;
        }
        else {
            fcCase (condRow) = 0;
        }
        filterConditions.Add(condRow);
    }
    if (setAsCurrent) {
        getFilter(name);
    }
}

void Database::deleteFilter(QString name)
{
    QCString utf8Name = name.utf8();
    int index = filters.Find(fName [utf8Name]);
    if (index == -1) {
        return;
    }
    filters.RemoveAt(index);
    // delete the filter's conditions
    int nextIndex = filterConditions.Find(fcFilter [utf8Name]);
    while (nextIndex != -1) {
        filterConditions.RemoveAt(nextIndex);
        nextIndex = filterConditions.Find(fcFilter [utf8Name]);
    }
    getFilter("_allrows");
}

void Database::deleteFilterColumn(QString filterName, QString columnName)
{
    QCString utf8FilterName = filterName.utf8();
    QCString utf8ColumnName = columnName.utf8();
    int removeIndex = filterConditions.Find(fcFilter [utf8FilterName]
                                            + fcColumn [utf8ColumnName]);
    while (removeIndex != -1) {
        int position = fcPosition (filterConditions[removeIndex]);
        position++;
        int nextIndex = filterConditions.Find(fcFilter [utf8FilterName]
                                              + fcPosition [position]);
        while (nextIndex != -1) {
            fcPosition (filterConditions[nextIndex]) = position - 1;
            position++;
            nextIndex = filterConditions.Find(fcFilter [utf8FilterName]
                                              + fcPosition [position]);
        }
        filterConditions.RemoveAt(removeIndex);
        removeIndex = filterConditions.Find(fcFilter [utf8FilterName]
                                            + fcColumn [utf8ColumnName]);
    }
}

int Database::getConditionCount(QString filterName)
{
    c4_View conditions = filterConditions.Select(fcFilter [filterName.utf8()]);
    return conditions.GetSize();
}

Condition *Database::getCondition(QString filterName, int index)
{
    int rowIndex = filterConditions.Find(fcFilter [filterName.utf8()]
                                         + fcPosition [index]);
    if (rowIndex == -1) {
        return new Condition(this);
    }
    c4_RowRef row = filterConditions[rowIndex];
    Condition *condition = new Condition(this);
    condition->setColName(QString::fromUtf8(fcColumn (row)));
    condition->setOperator(fcOperator (row));
    condition->setConstant(QString::fromUtf8(fcConstant (row)));
    if (fcCase (row) == 1) {
        condition->setCaseSensitive(TRUE);
    }
    else {
        condition->setCaseSensitive(FALSE);
    }
    return condition;
}

QString Database::addRow(QStringList values, int *rowId)
{
    c4_Row row;
    Id (row) = maxId + 1;
    int count = values.count();
    if (count != columns.GetSize()) {
        return QObject::tr("Wrong number of columns");
    }
    c4_View temp = columns.SortOn(cIndex);
    for (int i = 0; i < count; i++) {
        int type = cType (temp[i]);
        QString name = QString::fromUtf8(cName (temp[i]));
        QString value = values[i];
        if (type == DATE) {
            // strip out common formatting characters (used in CSV import)
            value = value.replace(QRegExp("/"), "");
            value = value.replace(QRegExp("-"), "");
            value = value.replace(QRegExp("\\."), "");
        }
        QString error = isValidValue(type, value);
        if (error != "") {
            return name + " " + error;
        }
        QString idString = makeColId(cId (temp[i]), type);
        if (type == STRING || type == NOTE || type >= FIRST_ENUM) {
            c4_StringProp prop(idString);
            prop (row) = value.utf8();
        }
        else if (type == INTEGER || type == BOOLEAN || type == DATE) {
            c4_IntProp prop(idString);
            prop (row) = value.toInt();
        }
        else if (type == TIME) {
            bool ok;
            value = parseTimeString(value, &ok);
            c4_IntProp prop(idString);
            prop (row) = value.toInt();
        }
        else if (type == FLOAT) {
            c4_FloatProp prop(idString);
            prop (row) = value.toDouble();
            // also need to save the string as entered
            QString stringColId = makeColId(cId (temp[i]), STRING);
            c4_StringProp stringProp(stringColId);
            stringProp (row) = value.utf8();
        }
    }
    data.Add(row);
    maxId++;
    if (rowId != 0) {
        *rowId = maxId;
    }
    return "";
}

void Database::updateRow(int rowId, QStringList values)
{
    int index = data.Find(Id [rowId]);
    int count = columns.GetSize();
    c4_View temp = columns.SortOn(cIndex);
    for (int i = 0; i < count; i++) {
        int type = cType (temp[i]);
        QString idString = makeColId(cId (temp[i]), type);
        if (type == STRING || type == NOTE || type >= FIRST_ENUM) {
            c4_StringProp prop(idString);
            prop (data[index]) = values[i].utf8();
        }
        else if (type == INTEGER || type == BOOLEAN || type == DATE) {
            c4_IntProp prop(idString);
            prop (data[index]) = values[i].toInt();
        }
        else if (type == TIME) {
            bool ok;
            QString value = parseTimeString(values[i], &ok);
            c4_IntProp prop(idString);
            prop (data[index]) = value.toInt();
        }
        else if (type == FLOAT) {
            c4_FloatProp prop(idString);
            prop (data[index]) = values[i].toDouble();
            // also need to save the string as entered
            QString stringColId = makeColId(cId (temp[i]), STRING);
            c4_StringProp stringProp(stringColId);
            stringProp (data[index]) = values[i].utf8();
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

void Database::toggleBoolean(int rowId, QString colName)
{
    int index = data.Find(Id [rowId]);
    int colIndex = columns.Find(cName [colName.utf8()]);
    QString idString = makeColId(cId (columns[colIndex]), BOOLEAN);
    c4_IntProp prop(idString);
    int value = prop (data[index]);
    prop (data[index]) = (value == 1) ? 0 : 1;
}

QStringList Database::listViewColumns(QString viewName)
{
    c4_View cols = viewColumns.Select(vcView [viewName.utf8()]);
    cols = cols.SortOn(vcIndex);
    int size = cols.GetSize();
    QStringList names;
    for (int i = 0; i < size; i++) {
        names.append(QString::fromUtf8(vcName (cols[i])));
    }
    return names;
}

void Database::addViewColumn(QString viewName, QString columnName, int index,
                             int width, int deskwidth)
{
    QCString utf8ViewName = viewName.utf8();
    if (views.Find(vName [utf8ViewName]) == -1) {
        // named view doesn't exist
        return;
    }
    c4_View cols = viewColumns.Select(vcView [utf8ViewName]);
    c4_Row colRow;
    vcView (colRow) = utf8ViewName;
    vcIndex (colRow) = (index == -1) ? cols.GetSize() : index;
    vcName (colRow) = columnName.utf8();
    vcWidth (colRow) = (width == -1) ? 60 : width;
    vcDeskWidth (colRow) = (deskwidth == -1) ? 120 : deskwidth;
    viewColumns.Add(colRow);
}

void Database::deleteViewColumn(QString viewName, QString columnName)
{
    QCString utf8ViewName = viewName.utf8();
    int removeIndex = viewColumns.Find(vcView [utf8ViewName]
                                       + vcName [columnName.utf8()]);
    if (removeIndex == -1) {
        // no such column in this view
        return;
    }
    int position = vcIndex (viewColumns[removeIndex]);
    position++;
    int nextIndex = viewColumns.Find(vcView [utf8ViewName]
                                     + vcIndex [position]);
    while (nextIndex != -1) {
        vcIndex (viewColumns[nextIndex]) = position - 1;
        position++;
        nextIndex = viewColumns.Find(vcView [utf8ViewName]
                                     + vcIndex [position]);
    }
    viewColumns.RemoveAt(removeIndex);
}

void Database::setViewColumnSequence(QString viewName, QStringList colNames)
{
    int count = colNames.count();
    int nextIndex = -1;
    QCString utf8ViewName = viewName.utf8();
    for (int i = 0; i < count; i++) {
        nextIndex = viewColumns.Find(vcView [utf8ViewName]
                                     + vcName [colNames[i].utf8()]);
        vcIndex (viewColumns[nextIndex]) = i;
    }
}

QStringList Database::listEnums()
{
    c4_View sorted = enums.SortOn(eIndex);
    int size = sorted.GetSize();
    QStringList list;
    for (int i = 0; i < size; i++) {
        list.append(QString::fromUtf8(eName (sorted[i])));
    }
    return list;
}

QStringList Database::listEnumOptions(int id)
{
    c4_View options = enumOptions.Select(eoEnum [id]);
    options = options.SortOn(eoIndex);
    int size = options.GetSize();
    QStringList list;
    for (int i = 0; i < size; i++) {
        list.append(QString::fromUtf8(eoText (options[i])));
    }
    return list;
}

void Database::addEnum(QString name, QStringList options, int index, int id)
{
    int enumCount = enums.GetSize();
    int nextIndex = index;
    if (nextIndex == -1) {
        nextIndex = enumCount;
    }
    int nextId = id;
    int i;
    if (nextId == -1) {
        nextId = FIRST_ENUM;
        c4_View idsort = enums.SortOn(eId);
        for (i = 0; i < enumCount; i++) {
            if (nextId == eId (idsort[i])) {
                nextId++;
            }
            else {
                // found a gap in the ID sequence (an enum was deleted)
                break;
            }
        }
    }
    enums.Add(eName [name.utf8()] + eId[nextId] + eIndex [nextIndex]);
    int optionCount = options.count();
    for (i = 0; i < optionCount; i++) {
        enumOptions.Add(eoEnum [nextId] + eoIndex [i]
                        + eoText [options[i].utf8()]);
    }
}

void Database::renameEnum(QString oldName, QString newName)
{
    int index = enums.Find(eName [oldName.utf8()]);
    eName (enums[index]) = newName.utf8();
}

void Database::deleteEnum(QString name)
{
    int index = enums.Find(eName [name.utf8()]);
    int id = eId (enums[index]);
    // delete all columns of this enum type
    c4_View deletions = columns.Select(cType [id]);
    int delCount = deletions.GetSize();
    QStringList deleteNames;
    int i;
    for (i = 0; i < delCount; i++) {
        deleteNames.append(QString::fromUtf8(cName (deletions[i])));
    }
    for (i = 0; i < delCount; i++) {
        deleteColumn(deleteNames[i]);
    }
    // delete the enum's options
    int nextIndex = enumOptions.Find(eoEnum [id]);
    while (nextIndex != -1) {
        enumOptions.RemoveAt(nextIndex);
        nextIndex = enumOptions.Find(eoEnum [id]);
    }
    // delete the enum definition and update the position indices
    QStringList enumNames = listEnums();
    enums.RemoveAt(index);
    enumNames.remove(name);
    setEnumSequence(enumNames);
}

int Database::getEnumId(QString name)
{
    int index = enums.Find(eName [name.utf8()]);
    return eId (enums[index]);
}

QString Database::getEnumName(int id)
{
    int index = enums.Find(eId [id]);
    return QString::fromUtf8(eName (enums[index]));
}

QStringList Database::columnsUsingEnum(QString enumName)
{
    int enumId = getEnumId(enumName);
    c4_View cols = columns.Select(cType [enumId]);
    cols = cols.SortOn(cIndex);
    int count = cols.GetSize();
    QStringList colNames;
    for (int i = 0; i < count; i++) {
        colNames.append(QString::fromUtf8(cName (cols[i])));
    }
    return colNames;
}

void Database::setEnumSequence(QStringList names)
{
    int count = names.count();
    for (int i = 0; i < count; i++) {
        int index = enums.Find(eName [names[i].utf8()]);
        eIndex (enums[index]) = i;
    }
}

void Database::addEnumOption(QString enumName, QString option, int index)
{
    int enumId = getEnumId(enumName);
    int optionIndex = index;
    if (optionIndex == -1) {
        c4_View options = enumOptions.Select(eoEnum [enumId]);
        optionIndex = options.GetSize();
    }
    enumOptions.Add(eoEnum [enumId] + eoIndex [optionIndex]
                    + eoText [option.utf8()]);
}

void Database::renameEnumOption(QString enumName, QString oldName,
                                QString newName)
{
    int enumId = getEnumId(enumName);
    // rename it in the option definition
    int index = enumOptions.Find(eoEnum [enumId] + eoText [oldName.utf8()]);
    eoText (enumOptions[index]) = newName.utf8();
    // rename it everywhere else
    replaceEnumOption(enumId, oldName, newName);
}

void Database::deleteEnumOption(QString enumName, QString option,
                                QString replace)
{
    int enumId = getEnumId(enumName);
    // delete the option definition
    int index = enumOptions.Find(eoEnum [enumId] + eoText [option.utf8()]);
    enumOptions.RemoveAt(index);
    // handle the substitution everywhere else
    replaceEnumOption(enumId, option, replace);
}

void Database::replaceEnumOption(int enumId, QString oldOption,
                                 QString newOption)
{
    QCString utf8OldOption = oldOption.utf8();
    QCString utf8NewOption = newOption.utf8();
    // replace it for any enum columns where it is the default value
    int nextIndex = columns.Find(cType [enumId] + cDefault [utf8OldOption]);
    while (nextIndex != -1) {
        cDefault (columns[nextIndex]) = utf8NewOption;
        nextIndex = columns.Find(cType [enumId] + cDefault [utf8OldOption]);
    }
    // for the rest we need to loop through all columns of this enum type
    c4_View cols = columns.Select(cType [enumId]);
    int colCount = cols.GetSize();
    for (int i = 0; i < colCount; i++) {
        // replace it in any data fields that use it
        QString idString = makeColId(cId (cols[i]), enumId);
        c4_StringProp prop(idString);
        nextIndex = data.Find(prop [utf8OldOption]);
        while (nextIndex != -1) {
            prop (data[nextIndex]) = utf8NewOption;
            nextIndex = data.Find(prop [utf8OldOption]);
        }
        // replace it in any conditions that use it
        nextIndex = filterConditions.Find(fcColumn [cName (cols[i])]
                                          + fcConstant [utf8OldOption]);
        while (nextIndex != -1) {
            fcConstant (filterConditions[nextIndex]) = utf8NewOption;
            nextIndex = filterConditions.Find(fcColumn [cName (cols[i])]
                                              + fcConstant [utf8OldOption]);
        }
    }
}

void Database::setEnumOptionSequence(QString enumName, QStringList options)
{
    int enumId = getEnumId(enumName);
    int count = options.count();
    for (int i = 0; i < count; i++) {
        int index = enumOptions.Find(eoEnum [enumId]
                                     + eoText [options[i].utf8()]);
        eoIndex (enumOptions[index]) = i;
    }
}

QString Database::makeColId(int colId, int type)
{
    QString result("_");
    // include column type in case of clash with deleted column of other type
    if (type == INTEGER || type == BOOLEAN || type == DATE || type == TIME) {
        result += 'I';
    }
    else if (type == FLOAT) {
        result += 'F';
    }
    else {
        result += 'S';
    }
    result += QString::number(colId);
    return result;
}

QString Database::formatString(bool old)
{
    c4_View sorted = columns.SortOn(cIndex);
    int size = sorted.GetSize();

    QString result = "_data[_id:I";
    for (int i = 0; i < size; i++) {
        result += ',';
        c4_RowRef row = sorted[i];
        int type = cType(row);
        if (old) {
            result += QString(cName (row));
        }
        else {
            result += makeColId(cId (row), type);
        }
        if (type == INTEGER || type == BOOLEAN || type == DATE
                || type == TIME) {
            result += ":I";
        }
        else if (type == FLOAT) {
            result += ":F";
            // add string representation column
            result += "," + makeColId(cId (row), STRING) + ":S";
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
    CSVUtils csv;
    return csv.parseFile(filename, this);
}

void Database::exportToXML(QString filename, c4_View &fullView,
                           c4_View &filteredView, QStringList cols)
{
    XMLExport xml(this, filename, cols);
    xml.addGlobalView(global);
    xml.addView("enums", enums.SortOn(eIndex));
    xml.addView("enumoptions", enumOptions.SortOn((eoEnum, eoIndex)));
    xml.addView("columns", columns.SortOn(cIndex));
    xml.addView("views", views.SortOn(vName));
    xml.addView("viewcolumns", viewColumns.SortOn((vcView, vcIndex)));
    xml.addView("sorts", sorts.SortOn(sName));
    xml.addView("sortcolumns", sortColumns.SortOn((scSort, scIndex)));
    xml.addView("filters", filters.SortOn(fName));
    xml.addView("filterconditions",
                filterConditions.SortOn((fcFilter, fcPosition)));
    QStringList colIds;
    QStringList allCols = listColumns();
    int allColCount = allCols.count();
    int *types = new int[allColCount];
    int *ids = new int[allColCount];
    int count = cols.count();
    int i;
    for (i = 0; i < count; i++) {
        int type = getType(cols[i]);
        types[i] = type;
        type = (type == FLOAT) ? STRING : type;
        int index = columns.Find(cName [cols[i].utf8()]);
        ids[i] = cId (columns[index]);
        colIds.append(makeColId(ids[i], type));
    }
    int colIndex = count;
    for (i = 0; i < allColCount; i++) {
        if (cols.findIndex(allCols[i]) != -1) {
            continue;
        }
        int type = getType(allCols[i]);
        types[colIndex] = type;
        type = (type == FLOAT) ? STRING : type;
        int index = columns.Find(cName [allCols[i].utf8()]);
        ids[colIndex] = cId (columns[index]);
        colIds.append(makeColId(ids[colIndex], type));
        colIndex++;
    }
    xml.addDataView(fullView, filteredView, ids, types, colIds);
}

void Database::setGlobalInfo(const QString &view, const QString &sorting,
                             const QString &filter)
{
    gView (global[0]) = view.utf8();
    gSort (global[0]) = sorting.utf8();
    gFilter (global[0]) = filter.utf8();
}

bool Database::isNoneDate(QDate &date)
{
    return (date.year() == 1752 && date.month() == 9 && date.day() == 14);
}

QString Database::dateToString(int date)
{
    int y = date / 10000;
    int m = (date - y * 10000) / 100;
    int d = date - y * 10000 - m * 100;
    QDate dateObj(y, m, d);
    return dateToString(dateObj);
}

QString Database::dateToString(QDate &date)
{
    if (isNoneDate(date)) {
        return "";
    }
    int *parts = new int[3];
#if defined(DESKTOP)
    PBDateFormat::Order ymd = PBDateFormat::YearMonthDay;
    PBDateFormat::Order mdy = PBDateFormat::MonthDayYear;
#else
    DateFormat::Order ymd = DateFormat::YearMonthDay;
    DateFormat::Order mdy = DateFormat::MonthDayYear;
#endif
    if (dateOrder == ymd) {
        parts[0] = date.year();
        parts[1] = date.month();
        parts[2] = date.day();
    }
    else if (dateOrder == mdy) {
        parts[0] = date.month();
        parts[1] = date.day();
        parts[2] = date.year();
    }
    else {
        parts[0] = date.day();
        parts[1] = date.month();
        parts[2] = date.year();
    }
    QString result = QString::number(parts[0]) + dateSeparator;
    result += QString::number(parts[1]) + dateSeparator;
    result += QString::number(parts[2]);
    return result;
}

QString Database::parseTimeString(QString value, bool *ok)
{
    // check for imported blank
    if (value == "") {
        *ok = TRUE;
        return "-1";
    }
    int length = value.length();
    int firstColon = value.find(':');
    if (firstColon == -1) {
        // assume it's a number of seconds, as used internally
        int totalSeconds = value.toInt(ok);
        if (!(*ok) || totalSeconds < -1 || totalSeconds > 86399) {
            *ok = FALSE;
        }
        return value;
    }
    // from here on is used only when importing
    if (firstColon < 1 || length < firstColon + 3) {
        *ok = FALSE;
        return value;
    }
    int hours = value.left(firstColon).toInt(ok);
    if (!(*ok)) {
        return value;
    }
    int minutes = value.mid(firstColon + 1, 2).toInt(ok);
    if (!(*ok)) {
        return value;
    }
    int seconds = 0;
    int secondColon = value.find(':', firstColon + 1);
    if (secondColon != -1 && length > secondColon + 2) {
        seconds = value.mid(secondColon + 1, 2).toInt(ok);
        if (!(*ok)) {
            return value;
        }
    }
    if (value.find("pm", 0, FALSE) != -1) {
        if (hours < 12) {
            hours += 12;
        }
    }
    else if (value.find("am", 0, FALSE) != -1 && hours == 12) {
        hours = 0;
    }
    QTime time;
    if (!time.setHMS(hours, minutes, seconds)) {
        *ok = FALSE;
        return value;
    }
    QTime midnight;
    int totalSeconds = midnight.secsTo(time);
    *ok = TRUE;
    return QString::number(totalSeconds);
}

QString Database::timeToString(int time)
{
    if (time == -1) {
        return "";
    }
    QTime midnight;
    QTime timeObj = midnight.addSecs(time);
    return TimeString::timeString(timeObj, ampm, showSeconds);
}

void Database::setShowSeconds(bool show)
{
    showSeconds = show;
}

void Database::updateDataColumnFormat()
{
    // Starting in PortaBase 1.3, unique column IDs are used to identify
    // columns in the data view rather than their names; this allows
    // for unicode column names, since they don't appear in the format string

    // Also, the original string representation of decimal fields is now
    // stored along with the floating point version
    data = file->GetAs(formatString(TRUE));
    int colCount = columns.GetSize();
    int rowCount = data.GetSize();
    for (int i = 0; i < colCount; i++) {
        // create a new data column, copying the contents of the old
        QString name(cName (columns[i]));
        cId (columns[i]) = i;
        int type = cType (columns[i]);
        QString idString = makeColId(i, type);
        if (type == INTEGER || type == BOOLEAN || type == DATE) {
            c4_IntProp oldProp(name);
            c4_IntProp newProp(idString);
            for (int i = 0; i < rowCount; i++) {
                newProp (data[i]) = oldProp (data[i]);
            }
        }
        else if (type == FLOAT) {
            c4_FloatProp oldProp(name);
            c4_FloatProp newProp(idString);
            QString stringColId = makeColId(i, STRING);
            c4_StringProp stringProp(stringColId);
            for (int i = 0; i < rowCount; i++) {
                double value = oldProp (data[i]);
                newProp (data[i]) = value;
                stringProp (data[i]) = QString::number(value).utf8();
            }
        }
        else if (type == STRING || type == NOTE) {
            c4_StringProp oldProp(name);
            c4_StringProp newProp(idString);
            for (int i = 0; i < rowCount; i++) {
                newProp (data[i]) = oldProp (data[i]);
            }
        }
    }
    updateDataFormat();
}

void Database::updateEncoding()
{
    // PortaBase used Latin-1 prior to version 1.3...need to convert to
    // UTF-8 in case any characters beyond the ASCII range were stored
    updateEncoding(global);
    updateEncoding(columns);
    updateEncoding(views);
    updateEncoding(viewColumns);
    updateEncoding(sorts);
    updateEncoding(sortColumns);
    updateEncoding(filters);
    updateEncoding(filterConditions);
    updateEncoding(data);
}

void Database::updateEncoding(c4_View &view)
{
    int propCount = view.NumProperties();
    int rowCount = view.GetSize();
    for (int i = 0; i < propCount; i++) {
        c4_Property prop = view.NthProperty(i);
        if (prop.Type() == 'S') {
            c4_StringProp sProp(prop.Name());
            for (int j = 0; j < rowCount; j++) {
                sProp (view[j]) = QString(sProp (view[j])).utf8();
            }
        }
    }
}

void Database::fixConditionIndices()
{
    int filterCount = filters.GetSize();
    for (int i = 0; i < filterCount; i++) {
        QCString filterName(fName (filters[i]));
        c4_View conditions = filterConditions.Select(fcFilter [filterName]);
        int conditionCount = conditions.GetSize();
        int index = -1;
        for (int j = 0; j < conditionCount; j++) {
            index = filterConditions.Find(fcFilter [filterName], index + 1);
            fcPosition (filterConditions[index]) = j;
        }
    }
}

void Database::addDesktopStats()
{
    int count = viewColumns.GetSize();
    int i;
    for (i = 0; i < count; i++) {
        vcDeskWidth (viewColumns[i]) = 2 * vcWidth (viewColumns[i]);
    }
    count = views.GetSize();
    for (i = 0; i < count; i++) {
        vDeskRpp (views[i]) = 25;
    }
}
