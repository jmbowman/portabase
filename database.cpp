/*
 * database.cpp
 *
 * (c) 2002-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#if defined(Q_WS_QWS)
#include <qpe/config.h>
#else
#include "desktop/config.h"
#endif

#include <qdatetime.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qmessagebox.h>
#include <qobject.h>
#include <qregexp.h>
#include <qstringlist.h>
#include <stdlib.h>
#include <string.h>
#include "calc/calcnode.h"
#include "image/imageutils.h"
#include "condition.h"
#include "crypto.h"
#include "csvutils.h"
#include "database.h"
#include "filter.h"
#include "metakitfuncs.h"
#include "view.h"
#include "xmlexport.h"

Database::Database(QString path, int *result, int encrypt) : crypto(0), version(0), newFile(FALSE), curView(0), curFilter(0), Id("_id"), cIndex("_cindex"), cName("_cname"), cType("_ctype"), cDefault("_cdefault"), cId("_cid"), vName("_vname"), vRpp("_vrpp"), vDeskRpp("_vdeskrpp"), vSort("_vsort"), vFilter("_vfilter"), vcView("_vcview"), vcIndex("_vcindex"), vcName("_vcname"), vcWidth("_vcwidth"), vcDeskWidth("_vcdeskwidth"), sName("_sname"), scSort("_scsort"), scIndex("_scindex"), scName("_scname"), scDesc("_scdesc"), fName("_fname"), fcFilter("_fcfilter"), fcPosition("_fcposition"), fcColumn("_fccolumn"), fcOperator("_fcoperator"), fcConstant("_fcconstant"), fcCase("_fccase"), eName("_ename"), eId("_eid"), eIndex("_eindex"), eoEnum("_eoenum"), eoIndex("_eoindex"), eoText("_eotext"), calcId("_calcid"), calcDecimals("_calcdecimals"), cnId("_cnid"), cnNodeId("_cnnodeid"), cnParentId("_cnparentid"), cnType("_cntype"), cnValue("_cnvalue"), gVersion("_gversion"), gView("_gview"), gSort("_gsort"), gFilter("_gfilter"), gCrypt("_gcrypt")
{
    c4_Storage::win32FileOpenFunc = windowsFileOpen;
    updateDateTimePrefs();
    Config pbConfig("portabase");
    pbConfig.setGroup("General");
    showSeconds = pbConfig.readBoolEntry("ShowSeconds");

#if defined(Q_WS_WIN)
    file = new c4_Storage(path.utf8(), TRUE);
#else
    file = new c4_Storage(QFile::encodeName(path), TRUE);
#endif
    global = file->GetAs("_global[_gversion:I,_gview:S,_gsort:S,_gfilter:S,_gcrypt:I]");
    if (global.GetSize() == 0) {
        // new file, add global data
        global.Add(gVersion [FILE_VERSION] + gView ["_all"] + gSort [""]
                   + gFilter ["_allrows"] + gCrypt [encrypt]);
        *result = OPEN_SUCCESS;
        newFile = TRUE;
        version = FILE_VERSION;
    }
    else if (gVersion (global[0]) > FILE_VERSION) {
        // trying to open a newer version of the file format
        *result = OPEN_NEWER_VERSION;
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
        if (version < 8) {
            // encryption added in file version 8
            gCrypt (global[0]) = 0;
        }
        if (version < FILE_VERSION) {
            gVersion (global[0]) = FILE_VERSION;
        }
        *result = OPEN_SUCCESS;
    }
    if (*result == OPEN_SUCCESS) {
        if (gCrypt (global[0]) == 0) {
            storage = file;
        }
        else {
            storage = new c4_Storage();
            crypto = new Crypto(file, storage);
            *result = OPEN_ENCRYPTED;
        }
    }
}

Database::~Database()
{
    if (crypto) {
        delete crypto;
        delete storage;
    }
    delete file;
    if (curView) {
        delete curView;
    }
    if (curFilter) {
        delete curFilter;
    }
}

QString Database::load()
{
    if (crypto) {
        if (!newFile) {
            QString error = crypto->open(version);
            if (error != "") {
                return error;
            }
        }
        // from now on, use global view from storage; the file's doesn't change
        global = storage->GetAs("_global[_gversion:I,_gview:S,_gsort:S,_gfilter:S,_gcrypt:I]");
        if (newFile) {
            global.Add(gVersion [FILE_VERSION] + gView ["_all"] + gSort [""]
                       + gFilter ["_allrows"] + gCrypt [1]);
        }
    }
    columns = storage->GetAs("_columns[_cindex:I,_cname:S,_ctype:I,_cdefault:S,_cid:I]");
    views = storage->GetAs("_views[_vname:S,_vrpp:I,_vdeskrpp:I,_vsort:S,_vfilter:S]");
    viewColumns = storage->GetAs("_viewcolumns[_vcview:S,_vcindex:I,_vcname:S,_vcwidth:I,_vcdeskwidth:I]");
    sorts = storage->GetAs("_sorts[_sname:S]");
    sortColumns = storage->GetAs("_sortcolumns[_scsort:S,_scindex:I,_scname:S,_scdesc:I]");
    filters = storage->GetAs("_filters[_fname:S]");
    filterConditions = storage->GetAs("_filterconditions[_fcfilter:S,_fcposition:I,_fccolumn:S,_fcoperator:I,_fcconstant:S,_fccase:I]");
    enums = storage->GetAs("_enums[_ename:S,_eid:I,_eindex:I]");
    enumOptions = storage->GetAs("_enumoptions[_eoenum:I,_eoindex:I,_eotext:S]");
    calcs = storage->GetAs("_calcs[_calcid:I,_calcdecimals:I]");
    calcNodes = storage->GetAs("_calcnodes[_cnid:I,_cnnodeid:I,_cnparentid:I,_cntype:I,_cnvalue:S]");
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
        data = storage->GetAs(formatString());
    }
    if (version < 6) {
        fixConditionIndices();
    }
    if (version < 7) {
        addDesktopStats();
    }
    if (version < 8) {
        addViewDefaults();
    }
    if (version < 9) {
        // added in version 8, but may be out of synch due to an update bug
        addEnumDataIndices();
    }
    maxId = data.GetSize() - 1;
    return "";
}

bool Database::encrypted()
{
    return (crypto != 0);
}

QString Database::setPassword(const QString &pass, bool newPass)
{
    return crypto->setPassword(pass, newPass);
}

QString Database::changePassword(const QString &oldPass,
                                 const QString &newPass)
{
    return crypto->changePassword(oldPass, newPass);
}

void Database::updateDateTimePrefs()
{
#if defined(Q_WS_QWS)
    DateFormat format = TimeString::currentDateFormat();
#else
    PBDateFormat format = TimeString::currentDateFormat();
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

View *Database::getView(const QString &name, bool applyDefaults,
                        bool setAsCurrent)
{
    int index = views.Find(vName [name.utf8()]);
#if defined(Q_WS_QWS)
    int rpp = vRpp (views[index]);
#else
    int rpp = vDeskRpp (views[index]);
#endif
    if (applyDefaults) {
        QString defaultSort = QString::fromUtf8(vSort (views[index]));
        QString defaultFilter = QString::fromUtf8(vFilter (views[index]));
        if (defaultSort != "_none") {
            gSort (global[0]) = defaultSort.utf8();
        }
        if (defaultFilter != "_none") {
            getFilter(defaultFilter);
        }
    }
    c4_View cols = viewColumns.Select(vcView [name.utf8()]);
    cols = cols.SortOn(vcIndex);
    int size = cols.GetSize();
    QStringList names;
    int *types = new int[size];
    int *widths = new int[size];
    QStringList colIds;
    QStringList stringColIds;
    for (int i = 0; i < size; i++) {
        QString colName = QString::fromUtf8(vcName (cols[i]));
        names.append(colName);
        int colIndex = columns.Find(cName [colName.utf8()]);
        types[i] = cType (columns[colIndex]);
#if defined(Q_WS_QWS)
        widths[i] = vcWidth (cols[i]);
#else
        widths[i] = vcDeskWidth (cols[i]);
#endif
        int idNum = cId (columns[colIndex]);
        colIds.append(makeColId(idNum, types[i]));
        if (types[i] == FLOAT || types[i] == CALC || types[i] == IMAGE) {
            stringColIds.append(makeColId(idNum, STRING));
        }
        else {
            stringColIds.append("");
        }
    }
    if (curView && setAsCurrent) {
        delete curView;
    }
    View *view = new View(name, this, data, names, types, widths, colIds,
                          stringColIds, rpp);
    if (setAsCurrent) {
        gView(global[0]) = name.utf8();
        curView = view;
    }
    return view;
}

QString Database::getDefaultSort(const QString &viewName)
{
    int index = views.Find(vName [viewName.utf8()]);
    return QString::fromUtf8(vSort (views[index]));
}

QString Database::getDefaultFilter(const QString &viewName)
{
    int index = views.Find(vName [viewName.utf8()]);
    return QString::fromUtf8(vFilter (views[index]));
}

QStringList Database::listViews()
{
    c4_View::stringCompareFunc = compareUsingLocale;
    c4_View sorted = views.SortOn(vName);
    c4_View::stringCompareFunc = strcmp;
    int size = sorted.GetSize();
    QStringList list;
    for (int i = 0; i < size; i++) {
        list.append(QString::fromUtf8(vName (sorted[i])));
    }
    return list;
}

void Database::addView(const QString &name, const QStringList &names,
                       const QString &defaultSort,
                       const QString &defaultFilter, int rpp, int deskrpp)
{
    c4_Row row;
    vName (row) = name.utf8();
    Config conf("portabase");
    conf.setGroup("General");
#if defined(Q_WS_QWS)
    int defaultRpp = conf.readNumEntry("RowsPerPage", 13);
    int defaultDeskRpp = 25;
#else
    int defaultRpp = 13;
    int defaultDeskRpp = conf.readNumEntry("RowsPerPage", 25);
#endif
    vRpp (row) = (rpp == -1) ? defaultRpp : rpp;
    vDeskRpp (row) = (deskrpp == -1) ? defaultDeskRpp : deskrpp;
    vSort (row) = defaultSort.utf8();
    vFilter (row) = defaultFilter.utf8();
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

void Database::deleteView(const QString &name)
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

void Database::renameView(const QString &oldName, const QString &newName)
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
#if defined(Q_WS_QWS)
        vcWidth (viewColumns[colIndex]) = widths[i];
#else
        vcDeskWidth (viewColumns[colIndex]) = widths[i];
#endif
        i++;
        colIndex = viewColumns.Find(vcView [viewName] + vcIndex [i]);
    }
}

void Database::setViewRowsPerPage(int rpp)
{
    QCString viewName(gView(global[0]));
    int index = views.Find(vName [viewName]);
#if defined(Q_WS_QWS)
    vRpp (views[index]) = rpp;
#else
    vDeskRpp (views[index]) = rpp;
#endif
}

void Database::setViewDefaults(const QString &sorting, const QString &filter)
{
    QCString viewName(gView (global[0]));
    int index = views.Find(vName [viewName]);
    vSort (views[index]) = sorting.utf8();
    vFilter (views[index]) = filter.utf8();
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

IntList Database::listTypes()
{
    c4_View sorted = columns.SortOn(cIndex);
    int size = sorted.GetSize();
    IntList list;
    for (int i = 0; i < size; i++) {
        int type = cType (sorted[i]);
        list.append(type);
    }
    return list;
}

int Database::getIndex(const QString &column)
{
    int index = columns.Find(cName [column.utf8()]);
    return cIndex (columns[index]);
}

void Database::setIndex(const QString &column, int index)
{
    int i = columns.Find(cName [column.utf8()]);
    cIndex (columns[i]) = index;
}

int Database::getType(const QString &column)
{
    int index = columns.Find(cName [column.utf8()]);
    return cType (columns[index]);
}

QString Database::getDefault(const QString &column)
{
    int index = columns.Find(cName [column.utf8()]);
    if (cType (columns[index]) == CALC) {
        return "0";
    }
    return QString::fromUtf8(cDefault (columns[index]));
}

void Database::setDefault(const QString &column, const QString &value)
{
    int index = columns.Find(cName [column.utf8()]);
    cDefault (columns[index]) = value.utf8();
}

QString Database::getColId(const QString &column, int type)
{
    int index = columns.Find(cName [column.utf8()]);
    int colType = type;
    if (colType == -1) {
        colType = cType (columns[index]);
    }
    return makeColId(cId (columns[index]), colType);
}

QString Database::isValidValue(int type, const QString &value)
{
    if (type == INTEGER || type == SEQUENCE) {
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

void Database::addColumn(int index, const QString &name, int type,
                         const QString &defaultVal, int id)
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
    else if (type == SEQUENCE) {
        c4_IntProp newProp(idString);
        int value = defaultVal.toInt();
        for (int i = 0; i < size; i++) {
            newProp (data[i]) = value;
            value++;
        }
        setDefault(name, QString::number(value));
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
    else if (type == IMAGE) {
        c4_BytesProp newProp(idString);
        // separate column to store image type
        QString stringColId = makeColId(nextId, STRING);
        c4_StringProp stringProp(stringColId);
        for (int i = 0; i < size; i++) {
            newProp (data[i]) = c4_Bytes();
            stringProp (data[i]) = "";
        }
    }
    else if (type >= FIRST_ENUM) {
        c4_StringProp newProp(idString);
        // separate column to store option indices for sorting & filtering
        QString indexColId = makeColId(nextId, INTEGER);
        c4_IntProp indexProp(indexColId);
        QStringList options = listEnumOptions(type);
        int defaultIndex = options.findIndex(defaultVal);
        for (int i = 0; i < size; i++) {
            newProp (data[i]) = defaultVal.utf8();
            indexProp (data[i]) = defaultIndex;
        }
    }
    else if (type != CALC) {
        c4_StringProp newProp(idString);
        for (int i = 0; i < size; i++) {
            newProp (data[i]) = utf8Value;
        }
    }
    // calculation values get set after the calculation is saved
}

void Database::deleteColumn(const QString &name)
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
    // remove the column from any calculations containing it
    deleteCalcColumn(name);
    // if the column is a calculation, delete its definition
    deleteCalc(cId (columns[index]));
    // remove the column from the definition
    columns.RemoveAt(index);
}

void Database::renameColumn(const QString &oldName, const QString &newName)
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
    // rename the column in any calculations containing it
    nextIndex = calcNodes.Find(cnValue [utf8OldName]);
    while (nextIndex != -1) {
        cnValue (calcNodes[nextIndex]) = utf8NewName;
        nextIndex = calcNodes.Find(cnValue [utf8OldName]);
    }
    // rename the column in the format definition
    int index = columns.Find(cName [utf8OldName]);
    cName (columns[index]) = utf8NewName;
}

void Database::updateDataFormat()
{
    data = storage->GetAs(formatString());
}

QStringList Database::getRow(int rowId, ImageUtils *utils)
{
    QStringList results;
    int index = data.Find(Id [rowId]);
    c4_RowRef row = data[index];
    int numCols = columns.GetSize();
    c4_View temp = columns.SortOn(cIndex);
    for (int i = 0; i < numCols; i++) {
        int type = cType (temp[i]);
        if (utils && (type == IMAGE)) {
            QString colName = QString::fromUtf8(cName (temp[i]));
            QString idString = makeColId(cId (temp[i]), STRING);
            c4_StringProp prop(idString);
            QString format = QString::fromUtf8(prop (row));
            results.append(utils->exportImage(this, rowId, colName, format));
            continue;
        }
        if (type == FLOAT || type == CALC || type == IMAGE) {
            // want the string representation for this
            type = STRING;
        }
        QString idString = makeColId(cId (temp[i]), type);
        if (type == INTEGER || type == BOOLEAN || type == DATE
                || type == TIME || type == SEQUENCE) {
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
    c4_View::stringCompareFunc = compareUsingLocale;
    c4_View sorted = sorts.SortOn(sName);
    c4_View::stringCompareFunc = strcmp;
    int size = sorted.GetSize();
    QStringList list;
    for (int i = 0; i < size; i++) {
        list.append(QString::fromUtf8(sName (sorted[i])));
    }
    return list;
}

bool Database::getSortingInfo(const QString &sortingName, QStringList *allCols,
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

void Database::addSorting(const QString &name, const QStringList &allCols,
                          const QStringList &descCols)
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

void Database::deleteSorting(const QString &name)
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
    // check for any views using it as a default
    nextIndex = views.Find(vSort [utf8Name]);
    while (nextIndex != -1) {
        vSort (views[nextIndex]) = "_none";
        nextIndex = views.Find(vSort [utf8Name]);
    }
}

void Database::deleteSortingColumn(const QString &sortName,
                                   const QString &columnName)
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

c4_View Database::sortData(c4_View filteredData, const QString &column,
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
    c4_View::stringCompareFunc = compareUsingLocale;
    if (ascending) {
        return filteredData.SortOn(sortView);
    }
    else {
        return filteredData.SortOnReverse(sortView, sortView);
    }
    c4_View::stringCompareFunc = strcmp;
}

c4_View Database::sortData(c4_View filteredData, const QString &sortingName)
{
    gSort (global[0]) = sortingName.utf8();
    QStringList allCols;
    QStringList descCols;
    if (!getSortingInfo(sortingName, &allCols, &descCols)) {
        return filteredData;
    }
    c4_View allView = createEmptyView(allCols);
    c4_View descView = createEmptyView(descCols);
    c4_View::stringCompareFunc = compareUsingLocale;
    return filteredData.SortOnReverse(allView, descView);
    c4_View::stringCompareFunc = strcmp;
}

c4_View Database::createEmptyView(const QStringList &colNames)
{
    int count = colNames.count();
    c4_View result;
    for (int i = 0; i < count; i++) {
        int index = columns.Find(cName [colNames[i].utf8()]);
        int id = cId (columns[index]);
        int type = cType (columns[index]);
        if (type >= FIRST_ENUM) {
            // sort by option index, not alphabetically by text
            type = INTEGER;
        }
        else if (type == IMAGE) {
            // sort by image type
            type = STRING;
        }
        QString idString = makeColId(id, type);
        if (type == INTEGER || type == BOOLEAN || type == DATE
                || type == TIME || type == SEQUENCE) {
            c4_IntProp prop(idString);
            result.AddProperty(prop);
        }
        else if (type == FLOAT || type == CALC) {
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
    c4_View::stringCompareFunc = compareUsingLocale;
    c4_View sorted = filters.SortOn(fName);
    c4_View::stringCompareFunc = strcmp;
    int size = sorted.GetSize();
    QStringList list;
    for (int i = 0; i < size; i++) {
        list.append(QString::fromUtf8(fName (sorted[i])));
    }
    return list;
}

Filter *Database::getFilter(const QString &name)
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

void Database::deleteFilter(const QString &name)
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
    // check for any views using it as a default
    nextIndex = views.Find(vFilter [utf8Name]);
    while (nextIndex != -1) {
        vFilter (views[nextIndex]) = "_none";
        nextIndex = views.Find(vFilter [utf8Name]);
    }
    getFilter("_allrows");
}

void Database::deleteFilterColumn(const QString &filterName,
                                  const QString &columnName)
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

int Database::getConditionCount(const QString &filterName)
{
    c4_View conditions = filterConditions.Select(fcFilter [filterName.utf8()]);
    return conditions.GetSize();
}

Condition *Database::getCondition(const QString &filterName, int index)
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

QString Database::addRow(QStringList values, int *rowId,
                         bool acceptSequenceVals, bool fromcsv)
{
    c4_Row row;
    Id (row) = maxId + 1;
    int count = values.count();
    int colCount = columns.GetSize();
    int i;
    if (count > colCount) {
        QString error = QObject::tr("Excess columns") + ":\n";
        for (i = colCount; i < count; i++) {
            error += "\"" + values[i] + "\"\n";
        }
        return error;
    }
    else if (count < colCount) {
        QString error = QObject::tr("Missing columns") + ":\n";
        QStringList colNames = listColumns();
        for (i = count; i < colCount; i++) {
            error += colNames[i] + "\n";
        }
        return error;
    }
    c4_View temp = columns.SortOn(cIndex);
    for (i = 0; i < count; i++) {
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
            // convert blank numbers in CSV import to the default value
            if (fromcsv && (value == "")
                    && (type == INTEGER || type == FLOAT)) {
                value = getDefault(name);
            }
            else {
                return name + " " + error;
            }
        }
        QString idString = makeColId(cId (temp[i]), type);
        if (type == STRING || type == NOTE) {
            c4_StringProp prop(idString);
            prop (row) = value.utf8();
        }
        else if (type == INTEGER || type == BOOLEAN || type == DATE) {
            c4_IntProp prop(idString);
            prop (row) = value.toInt();
        }
        else if (type == SEQUENCE) {
            c4_IntProp prop(idString);
            if (acceptSequenceVals) {
                prop (row) = value.toInt();
            }
            else {
                int nextValue = getDefault(name).toInt();
                prop (row) = nextValue;
                setDefault(name, QString::number(nextValue + 1));
            }
        }
        else if (type == TIME) {
            bool ok;
            value = parseTimeString(value, &ok);
            c4_IntProp prop(idString);
            prop (row) = value.toInt();
        }
        else if (type == FLOAT || type == CALC) {
            c4_FloatProp prop(idString);
            prop (row) = value.toDouble();
            // also need to save the string representation
            QString stringColId = makeColId(cId (temp[i]), STRING);
            c4_StringProp stringProp(stringColId);
            stringProp (row) = value.utf8();
            // when importing, remember to rerun the calculation after this
        }
        else if (type == IMAGE) {
            c4_BytesProp prop(idString);
            // also need to enter the image type
            QString stringColId = makeColId(cId (temp[i]), STRING);
            c4_StringProp stringProp(stringColId);
            if (value == "PNG" || value == "JPEG" || value == "") {
                // adding from the row editor, or an empty import field
                prop (row) = c4_Bytes();
                stringProp (row) = value;
            }
            else {
                // importing an image named in an imported file
                QString path = value;
                QFileInfo info(path);
                if (info.isRelative()) {
                    // compensate for location of imported file
                    path = importBasePath + value;
                }
                if (!QFile::exists(path)) {
                    return QObject::tr("Missing file") + ":\n" + value;
                }
                ImageUtils utils;
                bool resized = FALSE;
                QImage image = utils.load(path, &resized);
                if (image.isNull()) {
                    return utils.getErrorMessage() + ":\n" + value;
                }
                QString format = utils.getFormat();
                int size;
                char *data = ImageUtils::getImageData(image, format, path,
                                                      resized, &size);
                prop (row) = c4_Bytes(data, size);
                stringProp (row) = format;
                free(data);
            }
        }
        else if (type >= FIRST_ENUM) {
            c4_StringProp prop(idString);
            prop (row) = value.utf8();
            // also need to save the option index
            QString indexColId = makeColId(cId (temp[i]), INTEGER);
            c4_IntProp indexProp(indexColId);
            QStringList options = listEnumOptions(type);
            indexProp (row) = options.findIndex(value);
        }
    }
    data.Add(row);
    maxId++;
    if (rowId != 0) {
        *rowId = maxId;
    }
    return "";
}

void Database::updateRow(int rowId, const QStringList &values)
{
    int index = data.Find(Id [rowId]);
    int count = columns.GetSize();
    c4_View temp = columns.SortOn(cIndex);
    for (int i = 0; i < count; i++) {
        int type = cType (temp[i]);
        if (type == IMAGE) {
            // only updating the image format for now
            type = STRING;
        }
        QString idString = makeColId(cId (temp[i]), type);
        if (type == STRING || type == NOTE) {
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
        else if (type == FLOAT || type == CALC) {
            c4_FloatProp prop(idString);
            prop (data[index]) = values[i].toDouble();
            // also need to save the string representation
            QString stringColId = makeColId(cId (temp[i]), STRING);
            c4_StringProp stringProp(stringColId);
            stringProp (data[index]) = values[i].utf8();
        }
        else if (type >= FIRST_ENUM) {
            c4_StringProp prop(idString);
            prop (data[index]) = values[i].utf8();
            // also need to save the option index
            QString indexColId = makeColId(cId (temp[i]), INTEGER);
            c4_IntProp indexProp(indexColId);
            QStringList options = listEnumOptions(type);
            indexProp (data[index]) = options.findIndex(values[i]);
        }
        // NOTE: sequence fields never get updated
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

void Database::toggleBoolean(int rowId, const QString &colName)
{
    int index = data.Find(Id [rowId]);
    int colIndex = columns.Find(cName [colName.utf8()]);
    QString idString = makeColId(cId (columns[colIndex]), BOOLEAN);
    c4_IntProp prop(idString);
    int value = prop (data[index]);
    prop (data[index]) = (value == 1) ? 0 : 1;
}

char *Database::getBinaryField(int rowId, const QString &colName, int *size)
{
    int index = data.Find(Id [rowId]);
    int colIndex = columns.Find(cName [colName.utf8()]);
    QString idString = makeColId(cId (columns[colIndex]), IMAGE);
    c4_BytesProp prop(idString);
    c4_Bytes field = prop (data[index]);
    *size = field.Size();
    char *contents = 0;
    if (*size > 0) {
        contents = (char*)malloc(*size);
        memcpy(contents, field.Contents(), *size);
    }
    return contents;
}

void Database::setBinaryField(int rowId, const QString &colName,
                              char *dataArray, int size)
{
    int index = data.Find(Id [rowId]);
    int colIndex = columns.Find(cName [colName.utf8()]);
    QString idString = makeColId(cId (columns[colIndex]), IMAGE);
    c4_BytesProp prop(idString);
    if (size > 0) {
        prop (data[index]) = c4_Bytes(dataArray, size);
    }
    else {
        prop (data[index]) = c4_Bytes();
    }
}

QStringList Database::listViewColumns(const QString &viewName)
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

void Database::addViewColumn(const QString &viewName,
                             const QString &columnName, int index,
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

void Database::deleteViewColumn(const QString &viewName,
                                const QString &columnName)
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

void Database::setViewColumnSequence(const QString &viewName,
                                     const QStringList &colNames)
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

void Database::addEnum(const QString &name, const QStringList &options,
                       int index, int id)
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

void Database::renameEnum(const QString &oldName, const QString &newName)
{
    int index = enums.Find(eName [oldName.utf8()]);
    eName (enums[index]) = newName.utf8();
}

void Database::deleteEnum(const QString &name)
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

int Database::getEnumId(const QString &name)
{
    int index = enums.Find(eName [name.utf8()]);
    return eId (enums[index]);
}

QString Database::getEnumName(int id)
{
    int index = enums.Find(eId [id]);
    return QString::fromUtf8(eName (enums[index]));
}

QStringList Database::columnsUsingEnum(const QString &enumName)
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

void Database::setEnumSequence(const QStringList &names)
{
    int count = names.count();
    for (int i = 0; i < count; i++) {
        int index = enums.Find(eName [names[i].utf8()]);
        eIndex (enums[index]) = i;
    }
}

void Database::addEnumOption(const QString &enumName, const QString &option, int index)
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

void Database::renameEnumOption(const QString &enumName,
                                const QString &oldName, const QString &newName)
{
    int enumId = getEnumId(enumName);
    // rename it in the option definition
    int index = enumOptions.Find(eoEnum [enumId] + eoText [oldName.utf8()]);
    eoText (enumOptions[index]) = newName.utf8();
    // rename it everywhere else
    replaceEnumOption(enumId, oldName, newName);
}

void Database::deleteEnumOption(const QString &enumName, const QString &option,
                                const QString &replace)
{
    int enumId = getEnumId(enumName);
    // delete the option definition
    int index = enumOptions.Find(eoEnum [enumId] + eoText [option.utf8()]);
    enumOptions.RemoveAt(index);
    // handle the substitution everywhere else
    replaceEnumOption(enumId, option, replace);
}

void Database::replaceEnumOption(int enumId, const QString &oldOption,
                                 const QString &newOption)
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

void Database::setEnumOptionSequence(const QString &enumName,
                                     const QStringList &options)
{
    int enumId = getEnumId(enumName);
    int count = options.count();
    for (int i = 0; i < count; i++) {
        int index = enumOptions.Find(eoEnum [enumId]
                                     + eoText [options[i].utf8()]);
        eoIndex (enumOptions[index]) = i;
    }
    updateEnumDataIndices(enumName);
}

void Database::deleteCalcColumn(const QString &columnName)
{
    // delete all calculation nodes referring to this column
    QCString utf8ColumnName = columnName.utf8();
    int removeIndex = calcNodes.Find(cnValue [utf8ColumnName]);
    // keep track of which calculations need to be redone
    IntList redoIds;
    while (removeIndex != -1) {
        int colId = cnId (calcNodes[removeIndex]);
        if (redoIds.findIndex(colId) == -1) {
            redoIds.append(colId);
        }
        calcNodes.RemoveAt(removeIndex);
        removeIndex = calcNodes.Find(cnValue [utf8ColumnName]);
    }
    // redo the appropriate calculations for all rows
    int count = redoIds.count();
    for (int i = 0; i < count; i++) {
        int colId = redoIds[i];
        int decimals = 2;
        CalcNode *root = loadCalc(colId, &decimals);
        calculateAll(colId, root, decimals);
        delete root;
    }
}

CalcNode *Database::loadCalc(const QString &colName, int *decimals)
{
    int index = columns.Find(cName [colName.utf8()]);
    if (index == -1) {
        return 0;
    }
    int colId = cId (columns[index]);
    return loadCalc(colId, decimals);
}

CalcNode *Database::loadCalc(int colId, int *decimals)
{
    if (decimals != 0) {
        int index = calcs.Find(calcId [colId]);
        if (index != -1) {
            *decimals = calcDecimals (calcs[index]);
        }
    }
    c4_View nodes = calcNodes.Select(cnId [colId]);
    int size = nodes.GetSize();
    if (size == 0) {
        return 0;
    }
    nodes = nodes.SortOn(cnNodeId);
    int maxId = cnNodeId (nodes[size - 1]);
    CalcNode **array = new CalcNode *[maxId + 1];
    int id = cnNodeId (nodes[0]);
    int type = cnType (nodes[0]);
    QString value = QString::fromUtf8(cnValue (nodes[0]));
    CalcNode *root = new CalcNode(type, value);
    array[id] = root;
    for (int i = 1; i < size; i++) {
        id = cnNodeId (nodes[i]);
        int parent = cnParentId (nodes[i]);
        type = cnType (nodes[i]);
        value = QString::fromUtf8(cnValue (nodes[i]));
        CalcNode *node = new CalcNode(type, value);
        array[id] = node;
        CalcNode *parentNode = array[parent];
        parentNode->addChild(node);
    }
    delete[] array;
    return root;
}

void Database::deleteCalc(int colId)
{
    int removeIndex = calcNodes.Find(cnId [colId]);
    while (removeIndex != -1) {
        calcNodes.RemoveAt(removeIndex);
        removeIndex = calcNodes.Find(cnId [colId]);
    }
    removeIndex = calcs.Find(calcId [colId]);
    if (removeIndex != -1) {
        calcs.RemoveAt(removeIndex);
    }
}

void Database::calculateAll()
{
    int colCount = columns.GetSize();
    int i;
    for (i = 0; i < colCount; i++) {
        int type = cType (columns[i]);
        if (type == CALC) {
            int colId = cId (columns[i]);
            int decimals = 2;
            CalcNode *root = loadCalc(colId, &decimals);
            calculateAll(colId, root, decimals);
            delete root;
        }
    }
}

void Database::calculateAll(int colId, CalcNode *root, int decimals)
{
    int size = data.GetSize();
    QStringList colNames = listColumns();
    c4_FloatProp floatProp(makeColId(colId, CALC));
    c4_StringProp stringProp(makeColId(colId, STRING));
    double value = 0;
    for (int i = 0; i < size; i++) {
        if (root != 0) {
            QStringList row = getRow(i);
            value = root->value(row, colNames);
        }
        floatProp (data[i]) = value;
        stringProp (data[i]) = formatDouble(value, decimals).utf8();
    }
}

QString Database::formatDouble(double value, int decimals)
{
    QString format("%.");
    format += QString::number(decimals) + "f";
    return QString().sprintf(format, value);
}

void Database::updateCalc(const QString &colName, CalcNode *root, int decimals)
{
    int index = columns.Find(cName [colName.utf8()]);
    int colId = cId (columns[index]);
    deleteCalc(colId);
    c4_Row row;
    calcId (row) = colId;
    calcDecimals (row) = decimals;
    calcs.Add(row);
    addCalcNode(colId, root, 0, 0);
    calculateAll(colId, root, decimals);
}

int Database::addCalcNode(int calcId, CalcNode *node, int nodeId, int parentId)
{
    if (node == 0) {
        return 0;
    }
    c4_Row row;
    cnId (row) = calcId;
    cnNodeId (row) = nodeId;
    cnParentId (row) = parentId;
    cnType (row) = node->type();
    cnValue (row) = node->value().utf8();
    calcNodes.Add(row);
    CalcNodeList children = node->getChildren();
    int count = children.count();
    int nextId = nodeId + 1;
    for (int i = 0; i < count; i++) {
        nextId = addCalcNode(calcId, children[i], nextId, nodeId);
    }
    return nextId;
}

QString Database::makeColId(int colId, int type)
{
    QString result("_");
    // include column type in case of clash with deleted column of other type
    if (type == INTEGER || type == BOOLEAN || type == DATE || type == TIME
            || type == SEQUENCE) {
        result += 'I';
    }
    else if (type == FLOAT || type == CALC) {
        result += 'F';
    }
    else if (type == IMAGE) {
        result += 'B';
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
                || type == TIME || type == SEQUENCE) {
            result += ":I";
        }
        else if (type == FLOAT || type == CALC) {
            result += ":F";
            // add string representation column
            result += "," + makeColId(cId (row), STRING) + ":S";
        }
        else if (type == IMAGE) {
            result += ":B";
            // add image type column
            result += "," + makeColId(cId (row), STRING) + ":S";
        }
        else if (type >= FIRST_ENUM) {
            result += ":S";
            // add option index column
            result += "," + makeColId(cId (row), INTEGER) + ":I";
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
    if (crypto) {
        storage->Commit();
        crypto->save();
    }
    file->Commit();
}

void Database::setImportBasePath(const QString &path)
{
    importBasePath = path;
}

QStringList Database::importFromCSV(const QString &filename,
                                    const QString &encoding)
{
    CSVUtils csv;
    return csv.parseFile(filename, encoding, this);
}

void Database::exportToXML(QString filename, c4_View &fullView,
                           c4_View &filteredView, QStringList cols)
{
    XMLExport xml(this, filename, cols);
    xml.addGlobalView(global);
    c4_View::stringCompareFunc = compareUsingLocale;
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
    xml.addView("calcs", calcs.SortOn(calcId));
    xml.addView("calcnodes", calcNodes.SortOn((cnId, cnNodeId)));
    QStringList colIds;
    QStringList colNames;
    QStringList allCols = listColumns();
    int allColCount = allCols.count();
    int *types = new int[allColCount];
    int *ids = new int[allColCount];
    int count = cols.count();
    int i;
    for (i = 0; i < count; i++) {
        int type = getType(cols[i]);
        types[i] = type;
        if (type == FLOAT || type == CALC || type == IMAGE) {
            type = STRING;
        }
        int index = columns.Find(cName [cols[i].utf8()]);
        ids[i] = cId (columns[index]);
        colIds.append(makeColId(ids[i], type));
        colNames.append(cols[i]);
    }
    int colIndex = count;
    for (i = 0; i < allColCount; i++) {
        if (cols.findIndex(allCols[i]) != -1) {
            continue;
        }
        int type = getType(allCols[i]);
        types[colIndex] = type;
        type = (type == FLOAT || type == CALC) ? STRING : type;
        int index = columns.Find(cName [allCols[i].utf8()]);
        ids[colIndex] = cId (columns[index]);
        colIds.append(makeColId(ids[colIndex], type));
        colNames.append(allCols[i]);
        colIndex++;
    }
    xml.addDataView(fullView, filteredView, ids, types, colIds, colNames);
    c4_View::stringCompareFunc = strcmp;
}

void Database::setGlobalInfo(const QString &view, const QString &sorting,
                             const QString &filter)
{
    gView (global[0]) = view.utf8();
    gSort (global[0]) = sorting.utf8();
    gFilter (global[0]) = filter.utf8();
}

bool Database::isNoneDate(const QDate &date)
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

QString Database::dateToString(const QDate &date)
{
    if (isNoneDate(date)) {
        return "";
    }
    int *parts = new int[3];
#if defined(Q_WS_QWS)
    DateFormat::Order ymd = DateFormat::YearMonthDay;
    DateFormat::Order mdy = DateFormat::MonthDayYear;
#else
    PBDateFormat::Order ymd = PBDateFormat::YearMonthDay;
    PBDateFormat::Order mdy = PBDateFormat::MonthDayYear;
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
    data = storage->GetAs(formatString(TRUE));
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

void Database::addEnumDataIndices()
{
    QStringList enumList = listEnums();
    int count = enumList.count();
    int i;
    for (i = 0; i < count; i++) {
        updateEnumDataIndices(enumList[i]);
    }
}

void Database::updateEnumDataIndices(const QString &enumName)
{
    QStringList colNames = columnsUsingEnum(enumName);
    int colCount = colNames.count();
    int rowCount = data.GetSize();
    int enumId = getEnumId(enumName);
    QStringList options = listEnumOptions(enumId);
    int i;
    int j;
    for (i = 0; i < colCount; i++) {
        int colIndex = columns.Find(cName [colNames[i].utf8()]);
        int idNum = cId (columns[colIndex]);
        QString textColId = makeColId(idNum, STRING);
        c4_StringProp textProp(textColId);
        QString indexColId = makeColId(idNum, INTEGER);
        c4_IntProp indexProp(indexColId);
        for (j = 0; j < rowCount; j++) {
            QString option = QString::fromUtf8(textProp (data[j]));
            indexProp (data[j]) = options.findIndex(option);
        }
    }
}

void Database::addViewDefaults()
{
    int count = views.GetSize();
    int i;
    for (i = 0; i < count; i++) {
        vSort (views[i]) = "_none";
        vFilter (views[i]) = "_none";
    }
}
