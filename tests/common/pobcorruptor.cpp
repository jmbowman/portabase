/*
 * pobcorruptor.cpp
 *
 * (c) 2026 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Raw-Metakit mutation helpers for seeding structural corruption into
 * PortaBase test fixture files.
 */

#include <QFile>
#include <mk4.h>
#include "pobcorruptor.h"

namespace {

bool matches(const c4_RowRef &row, const PobCorrupt::Where &where)
{
    c4_StringProp prop(where.prop);
    return where.value == QByteArray((const char *)prop (row));
}

/** Canonical Metakit format string for each top-level PortaBase view. */
QByteArray canonicalFormat(const QString &viewName)
{
    if (viewName == "_global") {
        return "_global[_gversion:I,_gview:S,_gsort:S,_gfilter:S,_gcrypt:I]";
    }
    if (viewName == "_columns") {
        return "_columns[_cindex:I,_cname:S,_ctype:I,_cdefault:S,_cid:I]";
    }
    if (viewName == "_data") {
        return "_data[_id:I]";
    }
    if (viewName == "_views") {
        return "_views[_vname:S,_vrpp:I,_vdeskrpp:I,_vsort:S,_vfilter:S]";
    }
    if (viewName == "_viewcolumns") {
        return "_viewcolumns[_vcview:S,_vcindex:I,_vcname:S,_vcwidth:I,"
               "_vcdeskwidth:I]";
    }
    if (viewName == "_sorts") {
        return "_sorts[_sname:S]";
    }
    if (viewName == "_sortcolumns") {
        return "_sortcolumns[_scsort:S,_scindex:I,_scname:S,_scdesc:I]";
    }
    if (viewName == "_filters") {
        return "_filters[_fname:S]";
    }
    if (viewName == "_filterconditions") {
        return "_filterconditions[_fcfilter:S,_fcposition:I,_fccolumn:S,"
               "_fcoperator:I,_fcconstant:S,_fccase:I]";
    }
    if (viewName == "_enums") {
        return "_enums[_ename:S,_eid:I,_eindex:I]";
    }
    if (viewName == "_enumoptions") {
        return "_enumoptions[_eoenum:I,_eoindex:I,_eotext:S]";
    }
    if (viewName == "_calcs") {
        return "_calcs[_calcid:I,_calcdecimals:I]";
    }
    if (viewName == "_calcnodes") {
        return "_calcnodes[_cnid:I,_cnnodeid:I,_cnparentid:I,_cntype:I,"
               "_cnvalue:S]";
    }
    if (viewName == "_crypto") {
        return "_crypto[_criv:B,_crhash:B,_crdata:B]";
    }
    return "";
}

}

namespace PobCorrupt {

/** Set a string property on every row of the view matching the condition. */
bool setString(const QString &path, const char *viewName, const Where &where,
               const char *prop, const QByteArray &newValue)
{
    c4_Storage storage(QFile::encodeName(path), true);
    c4_View view = storage.View(viewName);
    c4_StringProp target(prop);
    bool found = false;
    int count = view.GetSize();
    for (int i = 0; i < count; i++) {
        if (matches(view[i], where)) {
            target (view[i]) = newValue;
            found = true;
        }
    }
    storage.Commit();
    return found;
}

/** Set a string property on rows matching both conditions. */
bool setString(const QString &path, const char *viewName, const Where &where1,
               const Where &where2, const char *prop,
               const QByteArray &newValue)
{
    c4_Storage storage(QFile::encodeName(path), true);
    c4_View view = storage.View(viewName);
    c4_StringProp target(prop);
    bool found = false;
    int count = view.GetSize();
    for (int i = 0; i < count; i++) {
        if (matches(view[i], where1) && matches(view[i], where2)) {
            target (view[i]) = newValue;
            found = true;
        }
    }
    storage.Commit();
    return found;
}

/** Set an integer property on every row of the view matching the condition. */
bool setInt(const QString &path, const char *viewName, const Where &where,
            const char *prop, int newValue)
{
    c4_Storage storage(QFile::encodeName(path), true);
    c4_View view = storage.View(viewName);
    c4_IntProp target(prop);
    bool found = false;
    int count = view.GetSize();
    for (int i = 0; i < count; i++) {
        if (matches(view[i], where)) {
            target (view[i]) = newValue;
            found = true;
        }
    }
    storage.Commit();
    return found;
}

/** Set an integer property on rows matching both conditions. */
bool setInt(const QString &path, const char *viewName, const Where &where1,
            const Where &where2, const char *prop, int newValue)
{
    c4_Storage storage(QFile::encodeName(path), true);
    c4_View view = storage.View(viewName);
    c4_IntProp target(prop);
    bool found = false;
    int count = view.GetSize();
    for (int i = 0; i < count; i++) {
        if (matches(view[i], where1) && matches(view[i], where2)) {
            target (view[i]) = newValue;
            found = true;
        }
    }
    storage.Commit();
    return found;
}

/** Set an integer property on the row at the given physical index. */
bool setIntAt(const QString &path, const char *viewName, int rowIndex,
              const char *prop, int newValue)
{
    c4_Storage storage(QFile::encodeName(path), true);
    c4_View view = storage.View(viewName);
    if (rowIndex < 0 || rowIndex >= view.GetSize()) {
        return false;
    }
    c4_IntProp target(prop);
    target (view[rowIndex]) = newValue;
    storage.Commit();
    return true;
}

/** Set a string property on the row at the given physical index. */
bool setStringAt(const QString &path, const char *viewName, int rowIndex,
                 const char *prop, const QByteArray &newValue)
{
    c4_Storage storage(QFile::encodeName(path), true);
    c4_View view = storage.View(viewName);
    if (rowIndex < 0 || rowIndex >= view.GetSize()) {
        return false;
    }
    c4_StringProp target(prop);
    target (view[rowIndex]) = newValue;
    storage.Commit();
    return true;
}

/** Remove every row of the view matching the condition. */
bool removeRows(const QString &path, const char *viewName, const Where &where)
{
    c4_Storage storage(QFile::encodeName(path), true);
    c4_View view = storage.View(viewName);
    bool found = false;
    for (int i = view.GetSize() - 1; i >= 0; i--) {
        if (matches(view[i], where)) {
            view.RemoveAt(i);
            found = true;
        }
    }
    storage.Commit();
    return found;
}

/** Remove rows matching both conditions. */
bool removeRows(const QString &path, const char *viewName, const Where &where1,
                const Where &where2)
{
    c4_Storage storage(QFile::encodeName(path), true);
    c4_View view = storage.View(viewName);
    bool found = false;
    for (int i = view.GetSize() - 1; i >= 0; i--) {
        if (matches(view[i], where1) && matches(view[i], where2)) {
            view.RemoveAt(i);
            found = true;
        }
    }
    storage.Commit();
    return found;
}

/** Duplicate the first row of the view matching the condition. */
bool duplicateRow(const QString &path, const char *viewName,
                  const Where &where)
{
    c4_Storage storage(QFile::encodeName(path), true);
    c4_View view = storage.View(viewName);
    int count = view.GetSize();
    for (int i = 0; i < count; i++) {
        if (matches(view[i], where)) {
            view.Add(view[i]);
            storage.Commit();
            return true;
        }
    }
    return false;
}

/** Duplicate the row at the given physical index (e.g. the _global row). */
bool duplicateRowAt(const QString &path, const char *viewName, int rowIndex)
{
    c4_Storage storage(QFile::encodeName(path), true);
    c4_View view = storage.View(viewName);
    if (rowIndex < 0 || rowIndex >= view.GetSize()) {
        return false;
    }
    view.Add(view[rowIndex]);
    storage.Commit();
    return true;
}

/** Remove the row at the given physical index. */
bool removeRowAt(const QString &path, const char *viewName, int rowIndex)
{
    c4_Storage storage(QFile::encodeName(path), true);
    c4_View view = storage.View(viewName);
    if (rowIndex < 0 || rowIndex >= view.GetSize()) {
        return false;
    }
    view.RemoveAt(rowIndex);
    storage.Commit();
    return true;
}

/**
 * Remove a PortaBase view definition and its column entries: the _views row
 * named viewName and every _viewcolumns row referencing it. This is the
 * "file with views but no _all" real-world state.
 */
bool removePortabaseView(const QString &path, const QString &viewName)
{
    bool removedView = removeRows(path, "_views", Where("_vname", viewName));
    removeRows(path, "_viewcolumns", Where("_vcview", viewName));
    return removedView;
}

/** Add a _crypto view with one (garbage-content) row. */
bool addCryptoView(const QString &path)
{
    c4_Storage storage(QFile::encodeName(path), true);
    c4_View crypto = storage.GetAs(canonicalFormat("_crypto"));
    c4_BytesProp criv("_criv");
    c4_BytesProp crhash("_crhash");
    c4_BytesProp crdata("_crdata");
    c4_Row row;
    criv (row) = c4_Bytes("iv", 2);
    crhash (row) = c4_Bytes("hash", 4);
    crdata (row) = c4_Bytes("data", 4);
    crypto.Add(row);
    storage.Commit();
    return true;
}

/**
 * Create a minimal raw PortaBase file from scratch: a _global row with the
 * given format version, plus an empty instance of each named top-level view
 * using its canonical format from docs/wiki/format.md. Used for testing
 * mandatory-view presence and version gating without fighting Metakit's
 * inability to drop a view from an existing file.
 */
bool createRawFile(const QString &path, int gversion,
                   const QStringList &viewNames, int gcrypt)
{
    if (QFile::exists(path)) {
        return false;
    }
    c4_Storage storage(QFile::encodeName(path), true);
    for (const QString &name : viewNames) {
        QByteArray format = canonicalFormat(name);
        if (format.isEmpty()) {
            return false;
        }
        c4_View view = storage.GetAs(format);
        if (name == "_global") {
            c4_IntProp gVersion("_gversion");
            c4_StringProp gView("_gview");
            c4_StringProp gSort("_gsort");
            c4_StringProp gFilter("_gfilter");
            c4_IntProp gCrypt("_gcrypt");
            c4_Row row;
            gVersion (row) = gversion;
            gView (row) = "_all";
            gSort (row) = "";
            gFilter (row) = "_allrows";
            gCrypt (row) = gcrypt;
            view.Add(row);
        }
        else if (name == "_views") {
            c4_StringProp vName("_vname");
            c4_IntProp vRpp("_vrpp");
            c4_IntProp vDeskRpp("_vdeskrpp");
            c4_StringProp vSort("_vsort");
            c4_StringProp vFilter("_vfilter");
            c4_Row row;
            vName (row) = "_all";
            vRpp (row) = 13;
            vDeskRpp (row) = 25;
            vSort (row) = "_none";
            vFilter (row) = "_none";
            view.Add(row);
        }
        else if (name == "_filters") {
            c4_StringProp fName("_fname");
            c4_Row row;
            fName (row) = "_allrows";
            view.Add(row);
        }
    }
    storage.Commit();
    return true;
}

}
