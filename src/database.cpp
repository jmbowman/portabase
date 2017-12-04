/*
 * database.cpp
 *
 * (c) 2002-2004,2008-2013,2015-2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file database.cpp
 * Source file for Database
 */

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QObject>
#include <QRegExp>
#include <QSettings>
#include "calc/calcnode.h"
#include "encryption/crypto.h"
#include "image/imageutils.h"
#include "condition.h"
#include "csvutils.h"
#include "database.h"
#include "filter.h"
#include "formatting.h"
#include "metakitfuncs.h"
#include "view.h"
#include "xmlexport.h"

/**
 * Constructor.  The value of "result" must be checked to see if the opened
 * file is encrypted, or if the file could not be opened because it was
 * created by a newer version of PortaBase.
 *
 * @param path Path to the PortaBase file to be opened or created
 * @param result Pointer to the value which will indicate the results of
 *               opening the file
 * @param encrypt True if an encrypted file is to be created, false otherwise
 */
Database::Database(const QString &path, OpenResult *result, bool encrypt)
  : file(0), crypto(0), version(0), newFile(false), curView(0), curFilter(0),
    maxId(0), Id("_id"), cIndex("_cindex"), cName("_cname"), cType("_ctype"),
    cDefault("_cdefault"), cId("_cid"), vName("_vname"), vRpp("_vrpp"),
    vDeskRpp("_vdeskrpp"), vSort("_vsort"), vFilter("_vfilter"),
    vcView("_vcview"), vcIndex("_vcindex"), vcName("_vcname"),
    vcWidth("_vcwidth"), vcDeskWidth("_vcdeskwidth"), sName("_sname"),
    scSort("_scsort"), scIndex("_scindex"), scName("_scname"),
    scDesc("_scdesc"), fName("_fname"), fcFilter("_fcfilter"),
    fcPosition("_fcposition"), fcColumn("_fccolumn"),
    fcOperator("_fcoperator"), fcConstant("_fcconstant"), fcCase("_fccase"),
    eName("_ename"), eId("_eid"), eIndex("_eindex"), eoEnum("_eoenum"),
    eoIndex("_eoindex"), eoText("_eotext"), calcId("_calcid"),
    calcDecimals("_calcdecimals"), cnId("_cnid"), cnNodeId("_cnnodeid"),
    cnParentId("_cnparentid"), cnType("_cntype"), cnValue("_cnvalue"),
    gVersion("_gversion"), gView("_gview"), gSort("_gsort"),
    gFilter("_gfilter"), gCrypt("_gcrypt")
{
    updatePreferences();

    QFileInfo info(path);
    bool canWrite = true;
    if (info.exists()) {
        if (!info.isReadable()) {
            *result = Failure;
            return;
        }
        canWrite = info.isWritable();
    }
#if defined(Q_OS_WIN)
    file = new c4_Storage(path.toUtf8(), canWrite);
#else
    file = new c4_Storage(QFile::encodeName(path), canWrite);
#endif
    global = file->GetAs("_global[_gversion:I,_gview:S,_gsort:S,_gfilter:S,_gcrypt:I]");
    if (global.GetSize() == 0) {
        // new file, add global data
        global.Add(gVersion [FILE_VERSION] + gView ["_all"] + gSort [""]
                   + gFilter ["_allrows"] + gCrypt [encrypt ? 1 : 0]);
        *result = Success;
        newFile = true;
        version = FILE_VERSION;
    }
    else if (gVersion (global[0]) > FILE_VERSION) {
        // trying to open a newer version of the file format
        *result = NewerVersion;
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
        *result = Success;
    }
    if (*result == Success) {
        if (gCrypt (global[0]) == 0) {
            storage = file;
        }
        else {
            storage = new c4_Storage();
            crypto = new Crypto(file, storage);
            *result = Encrypted;
        }
    }
}

/**
 * Destructor.
 */
Database::~Database()
{
    if (crypto) {
        delete crypto;
        delete storage;
    }
    if (file) {
        delete file;
    }
    if (curView) {
        delete curView;
    }
    if (curFilter) {
        delete curFilter;
    }
}

/**
 * Finish opening the file for use (after providing a valid password, if
 * the file is encrypted).
 *
 * @return An encryption-related error message, or an empty string if opened
 *         successfully
 */
QString Database::load()
{
    if (crypto) {
        if (!newFile) {
            QString error = crypto->open(version);
            if (!error.isEmpty()) {
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
        data = storage->GetAs(formatString().toLatin1());
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

/**
 * Determine if the database file is encrypted or not.
 *
 * @return True if the file is encrypted, false otherwise
 */
Crypto *Database::encryption()
{
    return crypto;
}

/**
 * Reload the application settings for "Any text column" conditions and screen
 * size.  Called when a file is first opened, and again whenever changes are
 * made in the Preferences dialog.
 */
void Database::updatePreferences()
{
    QSettings settings;
#ifdef MOBILE
    bool smallDefault = true;
#else
    bool smallDefault = false;
#endif
    anyTextIncludesEnums = settings.value("General/AnyTextIncludesEnums", false).toBool();
    smallScreen = settings.value("General/SmallScreen", smallDefault).toBool();

}

/**
 * Determine "any text column" conditions include enums in addition to string
 * and note fields.
 *
 * @return True if enums are included, false otherwise
 */
bool Database::anyTextColumnIncludesEnums()
{
    return anyTextIncludesEnums;
}

/**
 * Get the name of the database view currently in use.
 *
 * @return The name of the current database view
 */
QString Database::currentView()
{
    return QString::fromUtf8(gView(global[0]));
}

/**
 * Get the definition of the named database view.  Optionally, also apply its
 * default sorting and filter and/or set it to be the current view.
 *
 * @param name The name of the view to retrieve
 * @param applyDefaults Apply the view's default sorting and filter if true
 * @param setAsCurrent Set this to be the current view if true
 * @return The specified view's definition
 */
View *Database::getView(const QString &name, bool applyDefaults,
                        bool setAsCurrent)
{
    int index = views.Find(vName [name.toUtf8()]);
    int rpp = smallScreen ? vRpp (views[index]) : vDeskRpp (views[index]);
    if (applyDefaults) {
        QString defaultSort = QString::fromUtf8(vSort (views[index]));
        QString defaultFilter = QString::fromUtf8(vFilter (views[index]));
        if (defaultSort != "_none") {
            gSort (global[0]) = defaultSort.toUtf8();
        }
        if (defaultFilter != "_none") {
            getFilter(defaultFilter);
        }
    }
    c4_View cols = viewColumns.Select(vcView [name.toUtf8()]);
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
        int colIndex = columns.Find(cName [colName.toUtf8()]);
        types[i] = cType (columns[colIndex]);
        widths[i] = smallScreen ? vcWidth (cols[i]) : vcDeskWidth (cols[i]);
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
        gView(global[0]) = name.toUtf8();
        curView = view;
    }
    return view;
}

/**
 * Get the default sorting for the named view (may be "_none" if not
 * specified).
 *
 * @param viewName The name of the view whose default sorting to get
 * @return The name of the default sorting
 */
QString Database::getDefaultSort(const QString &viewName)
{
    int index = views.Find(vName [viewName.toUtf8()]);
    return QString::fromUtf8(vSort (views[index]));
}

/**
 * Get the default filter for the named view (may be "_none" if not
 * specified).
 *
 * @param viewName The name of the view whose default filter to get
 * @return The name of the default filter
 */
QString Database::getDefaultFilter(const QString &viewName)
{
    int index = views.Find(vName [viewName.toUtf8()]);
    return QString::fromUtf8(vFilter (views[index]));
}

/**
 * List all of the views defined in the database (including internal ones
 * such as "_all").
 *
 * @return The list of views, ordered alphabetically by name
 */
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

/**
 * Define a new view in the database.
 *
 * @param name The name of the view
 * @param names The names of the columns in the view, in order of appearance
 * @param defaultSort The view's default sorting (may be "_none")
 * @param defaultFilter The view's default filter (may be "_none")
 * @param rpp The number of rows per page to use on small-screen devices
 * @param deskrpp The number of rows per page to use on large-screen devices
 */
void Database::addView(const QString &name, const QStringList &names,
                       const QString &defaultSort,
                       const QString &defaultFilter, int rpp, int deskrpp)
{
    c4_Row row;
    vName (row) = name.toUtf8();
    QSettings settings;
    int defaultRpp = 13;
    int defaultDeskRpp = 25;
    if (smallScreen) {
        defaultRpp = settings.value("General/RowsPerPage", 13).toInt();
    }
    else {
        defaultDeskRpp = settings.value("General/RowsPerPage", 25).toInt();
    }
    vRpp (row) = (rpp == -1) ? defaultRpp : rpp;
    vDeskRpp (row) = (deskrpp == -1) ? defaultDeskRpp : deskrpp;
    vSort (row) = defaultSort.toUtf8();
    vFilter (row) = defaultFilter.toUtf8();
    views.Add(row);
    int count = names.count();
    for (int i = 0; i < count; i++) {
        c4_Row colRow;
        vcView (colRow) = name.toUtf8();
        vcIndex (colRow) = i;
        vcName (colRow) = names[i].toUtf8();
        vcWidth (colRow) = 120;
        vcDeskWidth (colRow) = 240;
        viewColumns.Add(colRow);
    }
}

/**
 * Remove the named view from the database.
 *
 * @param name The name of the view to delete
 */
void Database::deleteView(const QString &name)
{
    QByteArray utf8Name = name.toUtf8();
    int index = views.Find(vName [utf8Name]);
    views.RemoveAt(index);
    // delete the view's columns
    int nextIndex = viewColumns.Find(vcView [utf8Name]);
    while (nextIndex != -1) {
        viewColumns.RemoveAt(nextIndex);
        nextIndex = viewColumns.Find(vcView [utf8Name]);
    }
}

/**
 * Rename the specified view.
 *
 * @param oldName The view's current name
 * @param newName The new name for the view
 */
void Database::renameView(const QString &oldName, const QString &newName)
{
    if (newName == oldName) {
        return;
    }
    QByteArray utf8OldName = oldName.toUtf8();
    QByteArray utf8NewName = newName.toUtf8();
    int index = views.Find(vName [utf8OldName]);
    vName (views[index]) = utf8NewName;
    // rename references to the view in its columns
    int nextIndex = viewColumns.Find(vcView [utf8OldName]);
    while (nextIndex != -1) {
        vcView (viewColumns[nextIndex]) = utf8NewName;
        nextIndex = viewColumns.Find(vcView [utf8OldName]);
    }
}

/**
 * Sets the column widths, in order of appearance, in the current view to be
 * the specified pixel values.  Sets one of two sets of widths, depending on
 * whether the device in use has been specified in the application preferences
 * to have a small screen or not.  On Android, these are treated as
 * device-independent pixel counts (so the number of actual pixels that would
 * be used on a 160 dpi display).
 *
 * @param widths Array of new column widths (in pixels)
 */
void Database::setViewColWidths(int *widths)
{
    QByteArray viewName(gView(global[0]));
    int i = 0;
    int colIndex = viewColumns.Find(vcView [viewName] + vcIndex [i]);
    while (colIndex != -1) {
        if (smallScreen) {
            vcWidth (viewColumns[colIndex]) = widths[i];
        }
        else {
            vcDeskWidth (viewColumns[colIndex]) = widths[i];
        }
        i++;
        colIndex = viewColumns.Find(vcView [viewName] + vcIndex [i]);
    }
}

/**
 * Set the number of data rows per page to use for the current view on devices
 * with the same screen size (large or small) as the current one.
 *
 * @param rpp The new number of rows per page
 */
void Database::setViewRowsPerPage(int rpp)
{
    QByteArray viewName(gView(global[0]));
    int index = views.Find(vName [viewName]);
    if (smallScreen) {
        vRpp (views[index]) = rpp;
    }
    else {
        vDeskRpp (views[index]) = rpp;
    }
}

/**
 * Set the default sorting and filter (each of which may be "_none") for the
 * current view.
 *
 * @param sorting The name of the default sorting
 * @param filter The name of the default filter
 */
void Database::setViewDefaults(const QString &sorting, const QString &filter)
{
    QByteArray viewName(gView (global[0]));
    int index = views.Find(vName [viewName]);
    vSort (views[index]) = sorting.toUtf8();
    vFilter (views[index]) = filter.toUtf8();
}

/**
 * List all columns in the database, in the order in which they appear in the
 * ColumnsEditor dialog and the "All Columns" view.
 *
 * @return A list of column names
 */
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

/**
 * List the types of all columns in the database, in the order in which they
 * appear in the ColumnsEditor dialog and the "All Columns" view.
 *
 * @return A list of column type codes
 */
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

/**
 * Get the position index (in the data table definition) of the named column.
 *
 * @param column The name of the column to locate
 * @return The named column's position index
 */
int Database::getIndex(const QString &column)
{
    int index = columns.Find(cName [column.toUtf8()]);
    return cIndex (columns[index]);
}

/**
 * Set the position index (in the data table definition) of the named column.
 *
 * @param column The name of the column to locate
 * @param index The named column's new position index
 */
void Database::setIndex(const QString &column, int index)
{
    int i = columns.Find(cName [column.toUtf8()]);
    cIndex (columns[i]) = index;
}

/**
 * Get the data type code of the named column.
 *
 * @param column The name of the column in question
 * @return The named column's data type code
 */
int Database::getType(const QString &column)
{
    int index = columns.Find(cName [column.toUtf8()]);
    return cType (columns[index]);
}

/**
 * Get the default value of the named column.
 *
 * @param column The name of the column in question
 * @return The named column's default value
 */
QString Database::getDefault(const QString &column)
{
    int index = columns.Find(cName [column.toUtf8()]);
    if (cType (columns[index]) == CALC) {
        return "0";
    }
    return QString::fromUtf8(cDefault (columns[index]));
}

/**
 * Set the default value of the named column.
 *
 * @param column The name of the column to modify
 * @param value The named column's new default value
 */
void Database::setDefault(const QString &column, const QString &value)
{
    int index = columns.Find(cName [column.toUtf8()]);
    cDefault (columns[index]) = value.toUtf8();
}

/**
 * Get the identifier of the main Metakit table column corresponding to the
 * specified user-given column name.  The user-visible name was used as this
 * identifier in early versions of PortaBase, but this was changed to remove
 * some restrictions on allowable names.  The name alone isn't sufficient,
 * because some data types are actually stored in two different columns in
 * the table (for example, image format and image data).
 *
 * @param column The name of the column to get an identifier for
 * @param type The data type of the actual table column being identified
 * @return The identifier for a column in the main data table
 */
QString Database::getColId(const QString &column, int type)
{
    int index = columns.Find(cName [column.toUtf8()]);
    int colType = type;
    if (colType == -1) {
        colType = cType (columns[index]);
    }
    return makeColId(cId (columns[index]), colType);
}

/**
 * Determine if the given string represents a valid value for the specified
 * column type.
 *
 * @param type The data type to interpret the value as
 * @param value The value being tested
 * @param addNewEnumOptions True if an unknown enum option should be
 *                          automatically added rather than counted as an error
 * @return An appropriate error message if invalid, an empty string otherwise
 */
QString Database::isValidValue(int type, const QString &value,
                               bool addNewEnumOptions)
{
    if (type == INTEGER || type == SEQUENCE) {
        bool ok = false;
        QLocale::system().toInt(value, &ok);
        if (!ok) {
            return QObject::tr("must be an integer");
        }
    }
    else if (type == FLOAT) {
        bool ok = false;
        Formatting::parseDouble(value, &ok);
        if (!ok) {
            return QObject::tr("must be a decimal value");
        }
    }
    else if (type == BOOLEAN) {
        bool ok = false;
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
        Formatting::parseTimeString(value, &ok);
        if (!ok) {
            return QObject::tr("invalid time");
        }
    }
    else if (type >= FIRST_ENUM) {
        QStringList options = listEnumOptions(type);
        if (!options.contains(value)) {
            if (addNewEnumOptions) {
                addEnumOption(getEnumName(type), value);
            }
            else {
                return QObject::tr("no such option");
            }
        }
    }
    return "";
}

/**
 * Add a new column to the main data table.  Also adds it to the "All Columns"
 * view.  Must be followed by a call of updateDataFormat().
 *
 * @param index The position index at which to insert the column
 * @param name The column's name
 * @param type The column's data type code
 * @param defaultVal The column's default value
 * @param id The column's unique integer ID (-1 to generate one)
 */
void Database::addColumn(int index, const QString &name, int type,
                         const QString &defaultVal, int id)
{
    QByteArray utf8Value = defaultVal.toUtf8();
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
    columns.Add(cIndex [index] + cName [name.toUtf8()] + cType [type]
                + cDefault [utf8Value] + cId [nextId]);
    addViewColumn("_all", name);
    int size = data.GetSize();
    QByteArray idString = makeColId(nextId, type).toLatin1();
    if (type == INTEGER || type == BOOLEAN) {
        c4_IntProp newProp(idString);
        int value = QLocale::system().toInt(defaultVal);
        for (int i = 0; i < size; i++) {
            newProp (data[i]) = value;
        }
    }
    else if (type == SEQUENCE) {
        c4_IntProp newProp(idString);
        int value = QLocale::system().toInt(defaultVal);
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
        QByteArray stringColId = makeColId(nextId, STRING).toLatin1();
        c4_StringProp stringProp(stringColId);
        double value = Formatting::parseDouble(defaultVal);
        for (int i = 0; i < size; i++) {
            newProp (data[i]) = value;
            stringProp (data[i]) = defaultVal.toUtf8();
        }
    }
    else if (type == IMAGE) {
        c4_BytesProp newProp(idString);
        // separate column to store image type
        QByteArray stringColId = makeColId(nextId, STRING).toLatin1();
        c4_StringProp stringProp(stringColId);
        for (int i = 0; i < size; i++) {
            newProp (data[i]) = c4_Bytes();
            stringProp (data[i]) = "";
        }
    }
    else if (type >= FIRST_ENUM) {
        c4_StringProp newProp(idString);
        // separate column to store option indices for sorting & filtering
        QByteArray indexColId = makeColId(nextId, INTEGER).toLatin1();
        c4_IntProp indexProp(indexColId);
        QStringList options = listEnumOptions(type);
        int defaultIndex = options.indexOf(defaultVal);
        for (int i = 0; i < size; i++) {
            newProp (data[i]) = defaultVal.toUtf8();
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

/**
 * Delete a column from the main data table.  Also removes it from any views,
 * sortings, filters, or calculations that referenced it.  Must be followed
 * by a call of updateDataFormat().
 *
 * @param name The name of the column to delete
 */
void Database::deleteColumn(const QString &name)
{
    // first make sure it exists
    int index = columns.Find(cName [name.toUtf8()]);
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
    // clear the current filter cache, it may need to be reloaded
    if (curFilter) {
        delete curFilter;
        curFilter = 0;
    }
}

/**
 * Rename a column of the main data table.  Also renames it in any views,
 * sortings, filters, or calculations that reference it.
 *
 * @param oldName The column's current name
 * @param newName The column's new name
 */
void Database::renameColumn(const QString &oldName, const QString &newName)
{
    QByteArray utf8OldName = oldName.toUtf8();
    QByteArray utf8NewName = newName.toUtf8();
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
    // clear the current filter cache, it may need to be reloaded
    if (curFilter) {
        delete curFilter;
        curFilter = 0;
    }
}

/**
 * Update the definition of the main data table to reflect any changes which
 * have been made to the column definitions.
 */
void Database::updateDataFormat()
{
    data = storage->GetAs(formatString().toLatin1());
}

/**
 * Get the specified row of data from the main table.  When used to get a row
 * for export, provide an ImageUtils object which has been configured using
 * setExportPaths().
 *
 * @param rowId The ID field of the row to retrieve
 * @param utils Image export utility, or 0 if not exporting the row
 * @return A list of the row's data fields in position order
 */
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
            QByteArray idString = makeColId(cId (temp[i]), STRING).toLatin1();
            c4_StringProp prop(idString);
            QString format = QString::fromUtf8(prop (row));
            results.append(utils->exportImage(this, rowId, colName, format));
            continue;
        }
        if (type == FLOAT || type == CALC || type == IMAGE) {
            // want the string representation for this
            type = STRING;
        }
        QByteArray idString = makeColId(cId (temp[i]), type).toLatin1();
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

/**
 * Get the name of the sorting currently in use.
 *
 * @return The name of the current sorting
 */
QString Database::currentSorting()
{
    return QString::fromUtf8(gSort (global[0]));
}

/**
 * List all of the sortings defined in the database (including internal ones
 * such as "_single").
 *
 * @return The list of sortings, ordered alphabetically by name
 */
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

/**
 * Get the definition of the named sorting.  Needs to be provided with
 * pointers to two empty QStringLists which will be populated with the
 * sorting's information.
 *
 * @param sortingName The name of the sorting
 * @param allCols The list which will contain the names of all columns used
 *                in the sorting, in the order on which they are sorted
 * @param descCols The list which will contain the names of the columns in
 *                 the sorting which are sorted in descending order
 * @return False if the sorting doesn't exist or contains no columns
 */
bool Database::getSortingInfo(const QString &sortingName, QStringList *allCols,
                              QStringList *descCols)
{
    c4_View temp = sortColumns.Select(scSort [sortingName.toUtf8()]);
    int count = temp.GetSize();
    if (count == 0) {
        // non-existent or empty sorting; nothing to do
        return false;
    }
    temp = temp.SortOn(scIndex);
    for (int i = 0; i < count; i++) {
        QString name = QString::fromUtf8(scName (temp[i]));
        allCols->append(name);
        if (scDesc (temp[i]) == 1) {
            descCols->append(name);
        }
    }
    return true;
}

/**
 * Add a new sorting to the database.
 *
 * @param name The name of the sorting
 * @param allCols The names of all columns used in the sorting, in the order
 *                on which they are sorted
 * @param descCols The names of the columns in the sorting which are sorted
 *                 in descending order
 */
void Database::addSorting(const QString &name, const QStringList &allCols,
                          const QStringList &descCols)
{
    QByteArray utf8Name = name.toUtf8();
    sorts.Add(sName [utf8Name]);
    int count = allCols.count();
    for (int i = 0; i < count; i++) {
        c4_Row colRow;
        scSort (colRow) = utf8Name;
        scIndex (colRow) = i;
        scName (colRow) = allCols[i].toUtf8();
        if (descCols.indexOf(allCols[i]) == -1) {
            scDesc (colRow) = 0;
        }
        else {
            scDesc (colRow) = 1;
        }
        sortColumns.Add(colRow);
    }
}

/**
 * Delete the named sorting from the database.
 *
 * @param name The name of the sorting to delete
 */
void Database::deleteSorting(const QString &name)
{
    QByteArray utf8Name = name.toUtf8();
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

/**
 * Delete one column from the set of columns used in the named sorting.
 *
 * @param sortName The name of the sorting
 * @param columnName The name of the column to remove from the sorting
 */
void Database::deleteSortingColumn(const QString &sortName,
                                   const QString &columnName)
{
    QByteArray utf8SortName = sortName.toUtf8();
    int removeIndex = sortColumns.Find(scSort [utf8SortName]
                                       + scName [columnName.toUtf8()]);
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

/**
 * Get the main Metakit data table.
 *
 * @return The main data table
 */
c4_View Database::getData() const
{
    return data;
}

/**
 * Sort the provided table (either the main data table or a subset of its
 * rows) on the specified column.
 *
 * @param filteredData The table to sort
 * @param column The name of the column to sort on
 * @param order The order in which to sort the column
 * @return The sorted table
 */
c4_View Database::sortData(c4_View filteredData, const QString &column,
                           Qt::SortOrder order)
{
    QStringList colNames;
    colNames.append(column);
    QStringList descNames;
    if (order == Qt::DescendingOrder) {
        descNames.append(column);
    }
    deleteSorting("_single");
    addSorting("_single", colNames, descNames);
    gSort (global[0]) = "_single";
    c4_View sortView = createEmptyView(colNames);
    c4_View result;
    c4_View::stringCompareFunc = compareUsingLocale;
    if (order == Qt::AscendingOrder) {
        result = filteredData.SortOn(sortView);
    }
    else {
        result = filteredData.SortOnReverse(sortView, sortView);
    }
    c4_View::stringCompareFunc = strcmp;
    return result;
}

/**
 * Sort the provided table (either the main data table or a subset of its
 * rows) using the specified sorting.
 *
 * @param filteredData The table to sort
 * @param sortingName The name of the sorting to apply to the table
 * @return The sorted table
 */
c4_View Database::sortData(c4_View filteredData, const QString &sortingName)
{
    gSort (global[0]) = sortingName.toUtf8();
    QStringList allCols;
    QStringList descCols;
    if (!getSortingInfo(sortingName, &allCols, &descCols)) {
        return filteredData;
    }
    c4_View allView = createEmptyView(allCols);
    c4_View descView = createEmptyView(descCols);
    c4_View result;
    c4_View::stringCompareFunc = compareUsingLocale;
    result = filteredData.SortOnReverse(allView, descView);
    c4_View::stringCompareFunc = strcmp;
    return result;
}

/**
 * Create an empty data table with a subset of the columns of the main data
 * table.  Used in the implementation of data sorting.
 *
 * @param colNames The names of the columns to include in the view, in order
 * @return An empty table with the desired column sequence
 */
c4_View Database::createEmptyView(const QStringList &colNames)
{
    int count = colNames.count();
    c4_View result;
    for (int i = 0; i < count; i++) {
        int index = columns.Find(cName [colNames[i].toUtf8()]);
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
        QByteArray idString = makeColId(id, type).toLatin1();
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

/**
 * Get the name of the filter currently in use.
 *
 * @return The name of the current filter
 */
QString Database::currentFilter()
{
    return QString::fromUtf8(gFilter (global[0]));
}

/**
 * List all of the filters defined in the database (including internal ones
 * such as "_allrows").
 *
 * @return The list of filters, ordered alphabetically by name
 */
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

/**
 * Get the definition of the named filter.  Also sets the retrieved filter to
 * be the one currently in use.  The loaded filter is cached internally and
 * deleted when appropriate, so it shouldn't be cached elsewhere.
 *
 * @param name The name of the filter to get
 * @return The filter's definition
 */
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
    gFilter (global[0]) = name.toUtf8();
    curFilter = new Filter(this, name);
    return curFilter;
}

/**
 * Add a new filter to the database.
 *
 * @param filter The definition of the filter to be added
 * @param setAsCurrent If true, immediately apply the new filter
 */
void Database::addFilter(Filter *filter, bool setAsCurrent)
{
    QString name = filter->getName();
    QByteArray utf8Name = name.toUtf8();
    filters.Add(fName [utf8Name]);
    int count = filter->getConditionCount();
    for (int i = 0; i < count; i++) {
        Condition *cond = filter->getCondition(i);
        c4_Row condRow;
        fcFilter (condRow) = utf8Name;
        fcPosition (condRow) = i;
        fcColumn (condRow) = cond->getColName().toUtf8();
        fcOperator (condRow) = cond->getOperator();
        fcConstant (condRow) = cond->getConstant().toUtf8();
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

/**
 * Delete the named filter from the database.  Since this is typically
 * called when that filter is selected, it then reverts the database to the
 * "All Rows" filter.
 *
 * @param name The name of the filter to delete
 */
void Database::deleteFilter(const QString &name)
{
    QByteArray utf8Name = name.toUtf8();
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

/**
 * Delete all conditions in the named filter which refer to the given column.
 * Called for all filters when that column is deleted from the database.
 *
 * @param filterName The name of the sorting
 * @param columnName The name of the column being deleted
 */
void Database::deleteFilterColumn(const QString &filterName,
                                  const QString &columnName)
{
    QByteArray utf8FilterName = filterName.toUtf8();
    QByteArray utf8ColumnName = columnName.toUtf8();
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

/**
 * Get the number of conditions present in the named filter.
 *
 * @param filterName The filter's name
 * @return The number of conditions in the filter
 */
int Database::getConditionCount(const QString &filterName)
{
    c4_View conditions = filterConditions.Select(fcFilter [filterName.toUtf8()]);
    return conditions.GetSize();
}

/**
 * Get the condition at the specified position index in the named filter.
 *
 * @param filterName The filter's name
 * @param index The position index of the desired condition
 * @return The requested condition
 */
Condition *Database::getCondition(const QString &filterName, int index)
{
    int rowIndex = filterConditions.Find(fcFilter [filterName.toUtf8()]
                                         + fcPosition [index]);
    if (rowIndex == -1) {
        return new Condition(this);
    }
    c4_RowRef row = filterConditions[rowIndex];
    Condition *condition = new Condition(this);
    condition->setColName(QString::fromUtf8(fcColumn (row)));
    int op = fcOperator (row);
    condition->setOperator((Condition::Operator)op);
    condition->setConstant(QString::fromUtf8(fcConstant (row)));
    if (fcCase (row) == 1) {
        condition->setCaseSensitive(true);
    }
    else {
        condition->setCaseSensitive(false);
    }
    return condition;
}

/**
 * Add a new row to the main data table.
 *
 * @param values The row's values for each column, in position order
 * @param rowId Pointer at which to store the new row's ID (0 if not needed)
 * @param acceptSequenceVals True to use the given values for sequence
 *                           columns, false to generate new ones
 * @param fromcsv True if importing from CSV, so blank fields should be
 *                replaced with the appropriate default values
 * @param addNewEnumOptions True if unknown enum options should be
 *                          automatically added instead of treated as errors
 * @return An error message explaining why the row could not be added, or an
 *         empty string if it was added successfully
 */
QString Database::addRow(const QStringList &values, int *rowId,
                         bool acceptSequenceVals, bool fromcsv,
                         bool addNewEnumOptions)
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
        QString error = isValidValue(type, value, addNewEnumOptions);
        if (!error.isEmpty()) {
            // convert blank numbers in CSV import to the default value
            if (fromcsv && (value.isEmpty())
                    && (type == INTEGER || type == FLOAT)) {
                value = getDefault(name);
            }
            else {
                return name + " " + error;
            }
        }
        QByteArray idString = makeColId(cId (temp[i]), type).toLatin1();
        if (type == STRING || type == NOTE) {
            c4_StringProp prop(idString);
            prop (row) = value.toUtf8();
        }
        else if (type == INTEGER || type == BOOLEAN || type == DATE) {
            c4_IntProp prop(idString);
            prop (row) = QLocale::system().toInt(value);
        }
        else if (type == SEQUENCE) {
            c4_IntProp prop(idString);
            if (acceptSequenceVals) {
                prop (row) = QLocale::system().toInt(value);
            }
            else {
                int nextValue = getDefault(name).toInt();
                prop (row) = nextValue;
                setDefault(name, QString::number(nextValue + 1));
            }
        }
        else if (type == TIME) {
            bool ok;
            value = Formatting::parseTimeString(value, &ok);
            c4_IntProp prop(idString);
            prop (row) = value.toInt();
        }
        else if (type == FLOAT || type == CALC) {
            c4_FloatProp prop(idString);
            prop (row) = Formatting::parseDouble(value);
            // also need to save the string representation
            QByteArray stringColId = makeColId(cId (temp[i]), STRING).toLatin1();
            c4_StringProp stringProp(stringColId);
            stringProp (row) = value.toUtf8();
            // when importing, remember to rerun the calculation after this
        }
        else if (type == IMAGE) {
            c4_BytesProp prop(idString);
            // also need to enter the image type
            QByteArray stringColId = makeColId(cId (temp[i]), STRING).toLatin1();
            c4_StringProp stringProp(stringColId);
            if (value == "PNG" || value == "JPEG" || value.isEmpty()) {
                // adding from the row editor, or an empty import field
                prop (row) = c4_Bytes();
                stringProp (row) = value.toUtf8();
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
                QImage image = utils.load(path);
                if (image.isNull()) {
                    return utils.getErrorMessage() + ":\n" + value;
                }
                QString format = utils.getFormat();
                QByteArray data = ImageUtils::getImageData(image, format, path,
                                                           false);
                prop (row) = c4_Bytes(data.data(), data.size());
                stringProp (row) = format.toUtf8();
            }
        }
        else if (type >= FIRST_ENUM) {
            c4_StringProp prop(idString);
            prop (row) = value.toUtf8();
            // also need to save the option index
            QByteArray indexColId = makeColId(cId (temp[i]), INTEGER).toLatin1();
            c4_IntProp indexProp(indexColId);
            QStringList options = listEnumOptions(type);
            indexProp (row) = options.indexOf(value);
        }
    }
    data.Add(row);
    maxId++;
    if (rowId != 0) {
        *rowId = maxId;
    }
    return "";
}

/**
 * Update the specified row of the main data table with the given column
 * values.
 *
 * @param rowId The ID of the row to be updated
 * @param values The row's new values for each column, in position order
 */
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
        QByteArray idString = makeColId(cId (temp[i]), type).toLatin1();
        if (type == STRING || type == NOTE) {
            c4_StringProp prop(idString);
            prop (data[index]) = values[i].toUtf8();
        }
        else if (type == INTEGER || type == BOOLEAN || type == DATE) {
            c4_IntProp prop(idString);
            prop (data[index]) = QLocale::system().toInt(values[i]);
        }
        else if (type == TIME) {
            bool ok;
            QString value = Formatting::parseTimeString(values[i], &ok);
            c4_IntProp prop(idString);
            prop (data[index]) = value.toInt();
        }
        else if (type == FLOAT || type == CALC) {
            c4_FloatProp prop(idString);
            prop (data[index]) = Formatting::parseDouble(values[i]);
            // also need to save the string representation
            QByteArray stringColId = makeColId(cId (temp[i]), STRING).toLatin1();
            c4_StringProp stringProp(stringColId);
            stringProp (data[index]) = values[i].toUtf8();
        }
        else if (type >= FIRST_ENUM) {
            c4_StringProp prop(idString);
            prop (data[index]) = values[i].toUtf8();
            // also need to save the option index
            QByteArray indexColId = makeColId(cId (temp[i]), INTEGER).toLatin1();
            c4_IntProp indexProp(indexColId);
            QStringList options = listEnumOptions(type);
            indexProp (data[index]) = options.indexOf(values[i]);
        }
        // NOTE: sequence fields never get updated
    }
}

/**
 * Delete the specified row from the main data table.
 *
 * @param id The ID of the row to delete
 * @param compressRowIds True if higher row IDs should be adjusted down now,
 *                    false if compressRowIds() will be called afterwards
 */
void Database::deleteRow(int id, bool compressRowIds)
{
    int index = data.Find(Id [id]);
    data.RemoveAt(index);
    if (!compressRowIds) {
        return;
    }
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

/**
 * Compress the set of row IDs in use so there aren't any gaps.  Should only
 * be needed after a bulk delete operation in which this compacting was
 * explicitly deferred.
 */
void Database::compressRowIds()
{
    c4_View sorted = data.SortOn(Id);
    int size = sorted.GetSize();
    for (int i = 0; i < size; i++) {
        c4_RowRef row = sorted[i];
        int rowId = Id (row);
        if (rowId > i) {
            Id (row) = i;
        }
    }
    maxId = size - 1;
}

/**
 * Toggle the boolean value in the specified field to its opposite.
 *
 * @param rowId The ID of the row containing the field
 * @param colName The name of the column containing the field
 */
void Database::toggleBoolean(int rowId, const QString &colName)
{
    int index = data.Find(Id [rowId]);
    int colIndex = columns.Find(cName [colName.toUtf8()]);
    QByteArray idString = makeColId(cId (columns[colIndex]), BOOLEAN).toLatin1();
    c4_IntProp prop(idString);
    int value = prop (data[index]);
    prop (data[index]) = (value == 1) ? 0 : 1;
}

/**
 * Get the binary data (typically an image) stored in the specified field.
 *
 * @param rowId The ID of the row containing the field
 * @param colName The name of the column containing the field
 * @return The requested data
 */
QByteArray Database::getBinaryField(int rowId, const QString &colName)
{
    int index = data.Find(Id [rowId]);
    int colIndex = columns.Find(cName [colName.toUtf8()]);
    QByteArray idString = makeColId(cId (columns[colIndex]), IMAGE).toLatin1();
    c4_BytesProp prop(idString);
    c4_Bytes field = prop (data[index]);
    return QByteArray((const char*)field.Contents(), field.Size());
}

/**
 * Store the provided binary data (typically an image) in the specified field.
 *
 * @param rowId The ID of the row containing the field
 * @param colName The name of the column containing the field
 * @param bytes The data to store
 */
void Database::setBinaryField(int rowId, const QString &colName,
                              QByteArray bytes)
{
    int index = data.Find(Id [rowId]);
    int colIndex = columns.Find(cName [colName.toUtf8()]);
    QByteArray idString = makeColId(cId (columns[colIndex]), IMAGE).toLatin1();
    c4_BytesProp prop(idString);
    if (bytes.size() > 0) {
        prop (data[index]) = c4_Bytes(bytes.data(), bytes.size());
    }
    else {
        prop (data[index]) = c4_Bytes();
    }
}

/**
 * List the columns (in position order) included in the named view.
 *
 * @param viewName The name of the view
 * @return The names of the columns included in the view
 */
QStringList Database::listViewColumns(const QString &viewName)
{
    c4_View cols = viewColumns.Select(vcView [viewName.toUtf8()]);
    cols = cols.SortOn(vcIndex);
    int size = cols.GetSize();
    QStringList names;
    for (int i = 0; i < size; i++) {
        names.append(QString::fromUtf8(vcName (cols[i])));
    }
    return names;
}

/**
 * Add a column to the specified view.
 *
 * @param viewName The name of the view
 * @param columnName The name of the column to add
 * @param index The position index of the column in the view, -1 to add at
 *              the end
 * @param width The width of the column (in pixels) on small screens
 * @param deskwidth The width of the column (in pixels) on big screens
 */
void Database::addViewColumn(const QString &viewName,
                             const QString &columnName, int index,
                             int width, int deskwidth)
{
    QByteArray utf8ViewName = viewName.toUtf8();
    if (views.Find(vName [utf8ViewName]) == -1) {
        // named view doesn't exist
        return;
    }
    c4_View cols = viewColumns.Select(vcView [utf8ViewName]);
    c4_Row colRow;
    vcView (colRow) = utf8ViewName;
    vcIndex (colRow) = (index == -1) ? cols.GetSize() : index;
    vcName (colRow) = columnName.toUtf8();
    vcWidth (colRow) = width;
    vcDeskWidth (colRow) = deskwidth;
    viewColumns.Add(colRow);
}

/**
 * Remove a column from the specified view.
 *
 * @param viewName The name of the view to remove from
 * @param columnName The name of the column to remove
 */
void Database::deleteViewColumn(const QString &viewName,
                                const QString &columnName)
{
    QByteArray utf8ViewName = viewName.toUtf8();
    int removeIndex = viewColumns.Find(vcView [utf8ViewName]
                                       + vcName [columnName.toUtf8()]);
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

/**
 * Set the position sequence of the columns in the named view to match the
 * provided list.  Assumes that the view already consists of the correct set
 * of columns.
 *
 * @param viewName The view's name
 * @param colNames The names of the columns in the view, correctly ordered
 */
void Database::setViewColumnSequence(const QString &viewName,
                                     const QStringList &colNames)
{
    int count = colNames.count();
    QByteArray utf8ViewName = viewName.toUtf8();
    for (int i = 0; i < count; i++) {
        int nextIndex = viewColumns.Find(vcView [utf8ViewName]
                                         + vcName [colNames[i].toUtf8()]);
        vcIndex (viewColumns[nextIndex]) = i;
    }
}

/**
 * List the names of all enums defined in the database, in the order in which
 * they appear in the EnumManager dialog.
 *
 * @return The list of enum names
 */
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

/**
 * List the text for each option in the specified enum, in the order in which
 * they appear in the enum.
 *
 * @param id The data type ID of the enum
 * @return The list of enum options
 */
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

/**
 * Add a new enum to the database.
 *
 * @param name The enum's name
 * @param options The ordered list of options in the enum
 * @param index The new enum's position in the list of enums (-1 to append)
 * @param id The new enum's data type ID (-1 to use the next available one)
 */
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
    enums.Add(eName [name.toUtf8()] + eId[nextId] + eIndex [nextIndex]);
    int optionCount = options.count();
    for (i = 0; i < optionCount; i++) {
        enumOptions.Add(eoEnum [nextId] + eoIndex [i]
                        + eoText [options[i].toUtf8()]);
    }
}

/**
 * Rename an existing enum.
 *
 * @param oldName The enum's current name
 * @param newName The enum's new name
 */
void Database::renameEnum(const QString &oldName, const QString &newName)
{
    int index = enums.Find(eName [oldName.toUtf8()]);
    eName (enums[index]) = newName.toUtf8();
}

/**
 * Delete the specified enum from the database.  Also deletes all columns of
 * that enum type.
 *
 * @param name The name of the enum to delete
 */
void Database::deleteEnum(const QString &name)
{
    int index = enums.Find(eName [name.toUtf8()]);
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
    enumNames.removeAll(name);
    setEnumSequence(enumNames);
}

/**
 * Get the data type ID of the specified enum.
 *
 * @param name The enum's name
 * @return The enum's ID
 */
int Database::getEnumId(const QString &name)
{
    int index = enums.Find(eName [name.toUtf8()]);
    return eId (enums[index]);
}

/**
 * Get the name of the specified enum.
 *
 * @param id The enum's data type ID
 * @return The enum's name
 */
QString Database::getEnumName(int id)
{
    int index = enums.Find(eId [id]);
    return QString::fromUtf8(eName (enums[index]));
}

/**
 * List all of the columns which use the specified enum.  Used when
 * confirming whether the user actually wants to delete it, and again to
 * actually perform the deletion.
 *
 * @param enumName The enum's name
 * @return The names of the columns (in position order) of that enum type
 */
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

/**
 * Set the sequence of enums (as shown in the EnumManager dialog, the
 * ColumnEditor dialog, etc.) to match the provided list.  Assumes that
 * no enums need to be added or deleted.
 *
 * @param names The list of enum names whose order is to be matched
 */
void Database::setEnumSequence(const QStringList &names)
{
    int count = names.count();
    for (int i = 0; i < count; i++) {
        int index = enums.Find(eName [names[i].toUtf8()]);
        eIndex (enums[index]) = i;
    }
}

/**
 * Add a new option to the specified enum.
 *
 * @param enumName The name of the enum being added to
 * @param option The text of the new option
 * @param index The position index of the new option (-1 to append)
 */
void Database::addEnumOption(const QString &enumName, const QString &option, int index)
{
    int enumId = getEnumId(enumName);
    int optionIndex = index;
    if (optionIndex == -1) {
        c4_View options = enumOptions.Select(eoEnum [enumId]);
        optionIndex = options.GetSize();
    }
    enumOptions.Add(eoEnum [enumId] + eoIndex [optionIndex]
                    + eoText [option.toUtf8()]);
}

/**
 * Change the text of an enum option.  Also makes sure it's updated in all
 * data fields and filters.
 *
 * @param enumName The name of the enum the option belongs to
 * @param oldName The option's current text
 * @param newName The option's new text
 */
void Database::renameEnumOption(const QString &enumName,
                                const QString &oldName, const QString &newName)
{
    int enumId = getEnumId(enumName);
    // rename it in the option definition
    int index = enumOptions.Find(eoEnum [enumId] + eoText [oldName.toUtf8()]);
    eoText (enumOptions[index]) = newName.toUtf8();
    // rename it everywhere else
    replaceEnumOption(enumId, oldName, newName);
}

/**
 * Remove a particular option from the specified enum.
 *
 * @param enumName The name of the enum being modified
 * @param option The text of the option being removed
 * @param replace The text of the option to use wherever the one being deleted
 *                had been selected
 */
void Database::deleteEnumOption(const QString &enumName, const QString &option,
                                const QString &replace)
{
    int enumId = getEnumId(enumName);
    // delete the option definition
    int index = enumOptions.Find(eoEnum [enumId] + eoText [option.toUtf8()]);
    enumOptions.RemoveAt(index);
    // handle the substitution everywhere else
    replaceEnumOption(enumId, option, replace);
}

/**
 * Replace a particular enum option with another option of the same enum in
 * every data field and condition in which it is used.
 *
 * @param enumId The data type ID of the enum containing both options
 * @param oldOption The text of the option being replaced
 * @param newOption The text of the option to replace with
 */
void Database::replaceEnumOption(int enumId, const QString &oldOption,
                                 const QString &newOption)
{
    QByteArray utf8OldOption = oldOption.toUtf8();
    QByteArray utf8NewOption = newOption.toUtf8();
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
        QByteArray idString = makeColId(cId (cols[i]), enumId).toLatin1();
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
    // clear the current filter cache, it may need to be reloaded
    if (curFilter) {
        delete curFilter;
        curFilter = 0;
    }
}

/**
 * Set the sequence of options in the specified enum to match the provided
 * list.  Assumes that no options need to be added or deleted.
 *
 * @param enumName The name of the enum being modified
 * @param options The list of options whose order is to be matched
 */
void Database::setEnumOptionSequence(const QString &enumName,
                                     const QStringList &options)
{
    int enumId = getEnumId(enumName);
    int count = options.count();
    for (int i = 0; i < count; i++) {
        int index = enumOptions.Find(eoEnum [enumId]
                                     + eoText [options[i].toUtf8()]);
        eoIndex (enumOptions[index]) = i;
    }
    updateEnumDataIndices(enumName);
}

/**
 * Update any calculated columns that used a column which is being deleted.
 * Removes nodes from the calculation definitions as appropriate, and
 * recalculates the stored result values in every row for the affected
 * calculations.
 *
 * @param columnName The name of the column which is being deleted
 */
void Database::deleteCalcColumn(const QString &columnName)
{
    // delete all calculation nodes referring to this column
    QByteArray utf8ColumnName = columnName.toUtf8();
    int removeIndex = calcNodes.Find(cnValue [utf8ColumnName]);
    // keep track of which calculations need to be redone
    IntList redoIds;
    while (removeIndex != -1) {
        int colId = cnId (calcNodes[removeIndex]);
        if (redoIds.indexOf(colId) == -1) {
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

/**
 * Load the calculation definition for a calculated column based on that
 * column's name.  This is just a convenience method wrapping
 * Database::loadCalc(int, int*)
 *
 * @param colName The name of the column whose definition is desired
 * @param decimals A pointer to the integer in which to store the number of
 *                 decimal places that should be shown for calculation results
 * @return The root node of the calculation definition tree
 */
CalcNode *Database::loadCalc(const QString &colName, int *decimals)
{
    int index = columns.Find(cName [colName.toUtf8()]);
    if (index == -1) {
        return 0;
    }
    int colId = cId (columns[index]);
    return loadCalc(colId, decimals);
}

/**
 * Load the calculation definition for a calculated column based on that
 * column's ID.
 *
 * @param colId The ID of the column whose definition is desired
 * @param decimals A pointer to the integer in which to store the number of
 *                 decimal places that should be shown for calculation results
 * @return The root node of the calculation definition tree
 */
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
    CalcNode *root = new CalcNode((CalcNode::NodeType)type, value);
    array[id] = root;
    for (int i = 1; i < size; i++) {
        id = cnNodeId (nodes[i]);
        int parent = cnParentId (nodes[i]);
        type = cnType (nodes[i]);
        value = QString::fromUtf8(cnValue (nodes[i]));
        CalcNode *node = new CalcNode((CalcNode::NodeType)type, value);
        array[id] = node;
        CalcNode *parentNode = array[parent];
        parentNode->addChild(node);
    }
    delete[] array;
    return root;
}

/**
 * Delete the calculation definition for the calculated column with the
 * specified ID.  Does not delete the column definition or the corresponding
 * column in the data table.
 *
 * @param colId The ID of the column whose calculation definition is to be
 *              deleted
 */
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

/**
 * Recalculate the results of all calculated columns for every row and store
 * the results.
 */
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

/**
 * Recalculate the results of the specified calculation for every row in the
 * database and store the results.
 *
 * @param colId The ID of the calculated column to be recalculated
 * @param root The root node of the calculation definition
 * @param decimals The number of decimal places to show for calculated values
 */
void Database::calculateAll(int colId, CalcNode *root, int decimals)
{
    int size = data.GetSize();
    QStringList colNames = listColumns();
    c4_FloatProp floatProp(makeColId(colId, CALC).toLatin1());
    c4_StringProp stringProp(makeColId(colId, STRING).toLatin1());
    double value = 0;
    for (int i = 0; i < size; i++) {
        if (root != 0) {
            QStringList row = getRow(i);
            value = root->value(row, colNames);
        }
        floatProp (data[i]) = value;
        stringProp (data[i]) = Formatting::formatDouble(value, decimals).toUtf8();
    }
}

/**
 * Update a calculated column with a new calculation definition.  Deletes the
 * old definition, adds the new one, and recalculates all stored result
 * values.
 *
 * @param colName The name of the calculated column being updated
 * @param root The root node of the calculation definition tree
 * @param decimals The number of decimal places to retain for calculated
 *                 values
 */
void Database::updateCalc(const QString &colName, CalcNode *root, int decimals)
{
    int index = columns.Find(cName [colName.toUtf8()]);
    int colId = cId (columns[index]);
    deleteCalc(colId);
    c4_Row row;
    calcId (row) = colId;
    calcDecimals (row) = decimals;
    calcs.Add(row);
    addCalcNode(colId, root, 0, 0);
    calculateAll(colId, root, decimals);
}

/**
 * Add a new calculation definition tree node to the database.
 *
 * @param calcId The ID of the calculated column the node belongs to
 * @param node The node to be added
 * @param nodeId The ID of the node to be added; this is not a row ID, just
 *               a tool for preserving the tree hierarchy
 * @param parentId The ID of this node's parent node (this is 0 for the root
 *                 node)
 */
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
    cnValue (row) = node->value().toUtf8();
    calcNodes.Add(row);
    CalcNodeList children = node->getChildren();
    int count = children.count();
    int nextId = nodeId + 1;
    for (int i = 0; i < count; i++) {
        nextId = addCalcNode(calcId, children[i], nextId, nodeId);
    }
    return nextId;
}

/**
 * Make the column ID string used to represent a column in the Metakit view
 * definition of the main data table.
 *
 * @param colId The column's ID number
 * @param type The PortaBase data type ID of the column
 * @return A Metakit property definition string
 */
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

/**
 * Generate the Metakit view format string for the main data table.
 *
 * @param old True if working with a file from PortaBase 1.2 or older, which
 *            used a more primitive format string for this table
 * @return The data table's format string
 */
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

/**
 * Commit any changes made to the database since the last time it was saved.
 */
void Database::commit()
{
    if (crypto) {
        storage->Commit();
        crypto->save();
    }
    file->Commit();
}

/**
 * Set the base path for image files referenced in a file being imported.
 * This is generally the path of the directory containing the CSV or XML file
 * being imported.
 *
 * @param path The path of the directory containing any image files to import
 */
void Database::setImportBasePath(const QString &path)
{
    importBasePath = path;
}

/**
 * Import rows of data from a CSV file.
 *
 * @param filename The path to the CSV file to import
 * @param csv The settings to use for CSV import (delimiter, encoding, etc.)
 * @return Empty if no error occurred.  Otherwise, an error message optionally
 *         followed by the text of the record imported that triggered that
 *         error
 */
QStringList Database::importFromCSV(const QString &filename, CSVUtils *csv)
{
    return csv->parseFile(filename, this);
}

/**
 * Export the entire content of this database to an XML file.  If the database
 * contains any images, they will be exported as separate files in the same
 * directory as the output file.
 *
 * @param filename The path of the file to export to
 * @param fullView The Metakit view representing the main data table
 * @param filteredView The Metakit view resulting from applying the current
 *                     filter to the full data table
 * @param cols Ordered list of column names in the current view
 */
void Database::exportToXML(const QString &filename, c4_View &fullView,
                           c4_View &filteredView, const QStringList &cols)
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
        int index = columns.Find(cName [cols[i].toUtf8()]);
        ids[i] = cId (columns[index]);
        colIds.append(makeColId(ids[i], type));
        colNames.append(cols[i]);
    }
    int colIndex = count;
    for (i = 0; i < allColCount; i++) {
        if (cols.indexOf(allCols[i]) != -1) {
            continue;
        }
        int type = getType(allCols[i]);
        types[colIndex] = type;
        type = (type == FLOAT || type == CALC) ? STRING : type;
        int index = columns.Find(cName [allCols[i].toUtf8()]);
        ids[colIndex] = cId (columns[index]);
        colIds.append(makeColId(ids[colIndex], type));
        colNames.append(allCols[i]);
        colIndex++;
    }
    xml.addDataView(fullView, filteredView, ids, types, colIds, colNames);
    c4_View::stringCompareFunc = strcmp;
}

/**
 * Set the current view, sorting, and filter (all stored in the global
 * properties table).  Used during command-line exports and XML import.
 *
 * @param view The name of the view to set as current
 * @param sorting The name of the sorting to set as current
 * @param filter The name of the filter to set as current
 */
void Database::setGlobalInfo(const QString &view, const QString &sorting,
                             const QString &filter)
{
    gView (global[0]) = view.toUtf8();
    gSort (global[0]) = sorting.toUtf8();
    gFilter (global[0]) = filter.toUtf8();
}

/**
 * Make the changes necessary for files created by old versions of PortaBase
 * to support the type of main data table format string used by more recent
 * versions.
 *
 * <p>Starting in PortaBase 1.3, unique column IDs are used to identify
 * columns in the data view rather than their names; this allows for unicode
 * column names, since they don't appear in the format string.</p>
 *
 * <p>Also, the original string representation of decimal fields is now
 * stored along with the floating point version.</p>
 */
void Database::updateDataColumnFormat()
{
    data = storage->GetAs(formatString(true).toLatin1());
    int colCount = columns.GetSize();
    int rowCount = data.GetSize();
    for (int i = 0; i < colCount; i++) {
        // create a new data column, copying the contents of the old
        QByteArray name = QString(cName (columns[i])).toLatin1();
        cId (columns[i]) = i;
        int type = cType (columns[i]);
        QByteArray idString = makeColId(i, type).toLatin1();
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
            QByteArray stringColId = makeColId(i, STRING).toLatin1();
            c4_StringProp stringProp(stringColId);
            for (int i = 0; i < rowCount; i++) {
                double value = oldProp (data[i]);
                newProp (data[i]) = value;
                stringProp (data[i]) = Formatting::formatDouble(value).toUtf8();
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

/**
 * Update the text encoding used in files generated by old versions of
 * PortaBase.  Latin-1 was used prior to version 1.3...need to convert to
 * UTF-8 in case any characters beyond the ASCII range were stored.
 */
void Database::updateEncoding()
{
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

/**
 * Update the text encoding of all text fields in the specified Metakit view
 * from Latin-1 to UTF-8.
 *
 * @param view The Metakit view to be updated
 */
void Database::updateEncoding(c4_View &view)
{
    int propCount = view.NumProperties();
    int rowCount = view.GetSize();
    for (int i = 0; i < propCount; i++) {
        c4_Property prop = view.NthProperty(i);
        if (prop.Type() == 'S') {
            c4_StringProp sProp(prop.Name());
            for (int j = 0; j < rowCount; j++) {
                sProp (view[j]) = QString(sProp (view[j])).toUtf8();
            }
        }
    }
}

/**
 * Fix the position indices of conditions within filters defined in old
 * versions of PortaBase to work correctly.  Files created with PortaBase 1.4
 * and earlier require this correction.
 */
void Database::fixConditionIndices()
{
    int filterCount = filters.GetSize();
    for (int i = 0; i < filterCount; i++) {
        QByteArray filterName(fName (filters[i]));
        c4_View conditions = filterConditions.Select(fcFilter [filterName]);
        int conditionCount = conditions.GetSize();
        int index = -1;
        for (int j = 0; j < conditionCount; j++) {
            index = filterConditions.Find(fcFilter [filterName], index + 1);
            fcPosition (filterConditions[index]) = j;
        }
    }
}

/**
 * Update files from old versions of PortaBase to include display settings
 * for use on computers with full-size screens.  Files created with PortaBase
 * 1.4.1 and earlier require this correction.
 */
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

/**
 * Add or update the enum option indices in the main data table to support
 * non-alphabetical sorting of enum columns.  These were added in PortaBase
 * 1.6, but were sometimes incorrect until 1.7.
 */
void Database::addEnumDataIndices()
{
    QStringList enumList = listEnums();
    int count = enumList.count();
    int i;
    for (i = 0; i < count; i++) {
        updateEnumDataIndices(enumList[i]);
    }
}

/**
 * Add or update the enum option indices in the main data table for the named
 * enum column.
 *
 * @param enumName The name of the enum whose indices are to be updated
 */
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
        int colIndex = columns.Find(cName [colNames[i].toUtf8()]);
        int idNum = cId (columns[colIndex]);
        QByteArray textColId = makeColId(idNum, STRING).toLatin1();
        c4_StringProp textProp(textColId);
        QByteArray indexColId = makeColId(idNum, INTEGER).toLatin1();
        c4_IntProp indexProp(indexColId);
        for (j = 0; j < rowCount; j++) {
            QString option = QString::fromUtf8(textProp (data[j]));
            indexProp (data[j]) = options.indexOf(option);
        }
    }
}

/**
 * Update all PortaBase view definitions in old files to default to no
 * specified sorting or filter.  The ability to specify sortings or filters
 * to take effect when a view is switched to was introduced in PortaBase 1.6.
 */
void Database::addViewDefaults()
{
    int count = views.GetSize();
    int i;
    for (i = 0; i < count; i++) {
        vSort (views[i]) = "_none";
        vFilter (views[i]) = "_none";
    }
}
