/*
 * integritycheck.cpp
 *
 * (c) 2026 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file integritycheck.cpp
 * Source file for IntegrityCheck
 *
 * The invariants checked here are specified in
 * docs/intent/integrity-check/integrity-check-specs.md (the CHK-* IDs cited
 * throughout) and derive from the file format documentation in
 * docs/wiki/format.md. This file deliberately reads raw Metakit views rather
 * than reusing Database: that class's accessors assume the very invariants
 * being checked, and its constructor upgrades old-format files in place.
 */

#include <QBitArray>
#include <QFile>
#include <QMap>
#include <QSet>
#include <QStringList>
#include <QTextStream>
#include <algorithm>
#include <mk4.h>
#include "datatypes.h"
#include "integritycheck.h"

namespace {

// Column type codes, the type-code bounds, and FILE_VERSION come from
// datatypes.h; see docs/wiki/format.md for what they mean on disk.
const int OLDEST_VERSION = 1;

/** How many per-row violations to report individually before aggregating. */
const int ROW_REPORT_LIMIT = 3;

/** One column definition read from the _columns view. */
struct ColumnDef {
    int index;
    QString name;
    int type;
    QString defaultVal;
    int id;
};

/** Is the byte sequence valid UTF-8? (Valid UTF-8 round-trips exactly.) */
bool isValidUtf8(const QByteArray &bytes)
{
    return QString::fromUtf8(bytes).toUtf8() == bytes;
}

/** Format a bounded list of integers for a finding message. */
QString boundedList(const IntList &values)
{
    const int limit = 8;
    QStringList parts;
    for (int i = 0; i < values.size() && i < limit; i++) {
        parts.append(QString::number(values.at(i)));
    }
    if (values.size() > limit) {
        parts.append(QString("... %1 more").arg(values.size() - limit));
    }
    return parts.join(", ");
}

/**
 * Format an ascending list of distinct integers, collapsing each run of three
 * or more consecutive values to "first-last" so that an intact but offset
 * sequence stays legible. Bounded like boundedList, counting runs rather than
 * values, so a heavily fragmented list cannot flood the report.
 *
 * @param values The values to format, ascending and without duplicates
 * @return The formatted list
 */
QString boundedRuns(const IntList &values)
{
    const int limit = 8;
    QStringList parts;
    int runs = 0;
    int i = 0;
    while (i < values.size()) {
        int start = i;
        while (i + 1 < values.size()
               && values.at(i + 1) == values.at(i) + 1) {
            i++;
        }
        if (runs < limit) {
            if (i - start >= 2) {
                parts.append(QString("%1-%2").arg(values.at(start))
                                             .arg(values.at(i)));
            }
            else {
                for (int j = start; j <= i; j++) {
                    parts.append(QString::number(values.at(j)));
                }
            }
        }
        runs++;
        i++;
    }
    if (runs > limit) {
        parts.append(QString("... %1 more").arg(runs - limit));
    }
    return parts.join(", ");
}

/**
 * Describe how a row-ID set deviates from the expected permutation of
 * 0..n-1: which IDs are present, which are duplicated, which expected IDs are
 * missing, and the largest present. The specifics matter for diagnosing how a
 * file got into this state (and how much addRow's maxId assumption endangers
 * it) -- a file whose IDs all sit outside the expected range says nothing
 * useful if only the absences are listed.
 *
 * @param ids The row IDs in any order; sorted in place as a side effect
 * @return The description, or an empty string if ids is empty
 */
QString describeIdAnomalies(IntList &ids)
{
    std::sort(ids.begin(), ids.end());
    IntList distinct;
    IntList duplicated;
    for (int i = 0; i < ids.size(); i++) {
        if (i > 0 && ids.at(i) == ids.at(i - 1)) {
            if (duplicated.isEmpty() || duplicated.last() != ids.at(i)) {
                duplicated.append(ids.at(i));
            }
        }
        else {
            distinct.append(ids.at(i));
        }
    }
    IntList missing;
    int next = 0;
    for (int expected = 0; expected < ids.size(); expected++) {
        while (next < distinct.size() && distinct.at(next) < expected) {
            next++;
        }
        if (next >= distinct.size() || distinct.at(next) != expected) {
            missing.append(expected);
        }
    }
    QStringList parts;
    if (!distinct.isEmpty()) {
        parts.append("present: " + boundedRuns(distinct));
    }
    if (!duplicated.isEmpty()) {
        parts.append("duplicated: " + boundedList(duplicated));
    }
    if (!missing.isEmpty()) {
        parts.append("missing from expected range: " + boundedList(missing));
    }
    if (!distinct.isEmpty()) {
        parts.append(QString("largest ID: %1").arg(distinct.last()));
    }
    return parts.join("; ");
}

/**
 * Are the values a permutation of 0..n-1? Marks each value off in a bitset
 * sized to the value count, so the check neither mutates its argument (which
 * would deep-copy an implicitly shared list at every call site) nor allocates
 * anything derived from a corrupt file's largest value.
 *
 * @param values The values to test
 * @return True if the values are exactly 0..n-1 in some order
 */
bool isPermutation(const IntList &values)
{
    QBitArray seen(values.size());
    for (const int value : values) {
        if (value < 0 || value >= values.size() || seen.testBit(value)) {
            return false;
        }
        seen.setBit(value);
    }
    return true;
}

/** Split at top level on commas, respecting [] nesting. */
QList<QByteArray> splitTopLevel(const QByteArray &text)
{
    QList<QByteArray> parts;
    int depth = 0;
    int start = 0;
    for (int i = 0; i < text.size(); i++) {
        char c = text.at(i);
        if (c == '[') {
            depth++;
        }
        else if (c == ']') {
            depth--;
        }
        else if (c == ',' && depth == 0) {
            parts.append(text.mid(start, i - start));
            start = i + 1;
        }
    }
    if (start < text.size()) {
        parts.append(text.mid(start));
    }
    return parts;
}

/**
 * Parse a Metakit storage description ("_global[_gversion:I,...],...") into
 * a map of view name -> property descriptors ("_gversion:I").
 */
QMap<QString, QStringList> parseDescription(const QByteArray &description)
{
    QMap<QString, QStringList> views;
    const QList<QByteArray> items = splitTopLevel(description);
    for (const QByteArray &item : items) {
        int bracket = item.indexOf('[');
        if (bracket == -1) {
            continue;
        }
        QString name = QString::fromLatin1(item.left(bracket));
        QByteArray inner = item.mid(bracket + 1);
        if (inner.endsWith(']')) {
            inner.chop(1);
        }
        QStringList props;
        const QList<QByteArray> propItems = splitTopLevel(inner);
        for (const QByteArray &propItem : propItems) {
            props.append(QString::fromLatin1(propItem));
        }
        views.insert(name, props);
    }
    return views;
}

/** The property names (without type suffixes) of one parsed view. */
QStringList propNames(const QStringList &props)
{
    QStringList names;
    for (const QString &prop : props) {
        names.append(prop.section(':', 0, 0));
    }
    return names;
}

/**
 * The _data property names implied by one column, per the column-ID
 * derivation rules in docs/wiki/format.md § The _data View (the counterpart of
 * Database::makeColId/formatString).
 */
QStringList expectedDataProps(const ColumnDef &column)
{
    QStringList props;
    QString id = QString::number(column.id);
    switch (column.type) {
    case 1: // integer
    case 3: // boolean
    case 5: // date
    case 6: // time
    case 8: // sequence
        props << "_I" + id;
        break;
    case FLOAT:
    case CALC:
        props << "_F" + id << "_S" + id;
        break;
    case IMAGE:
        props << "_B" + id << "_S" + id;
        break;
    default:
        if (column.type >= FIRST_ENUM) {
            props << "_S" + id << "_I" + id;
        }
        else {
            props << "_S" + id; // string, note
        }
        break;
    }
    return props;
}

/**
 * One full scan of one file. Collects everything the IntegrityCheck
 * accessors expose; the constructor copies the results out.
 */
struct Scan {
    QString path;

    IntegrityCheck::FileStatus status = IntegrityCheck::Unreadable;
    int version = -1;
    bool encryptedSkipped = false;
    QList<IntegrityCheck::Finding> findings;
    QList<IntegrityCheck::SkippedCheck> skipped;

    c4_Storage *storage = nullptr;
    QMap<QString, QStringList> viewFormats;
    QList<ColumnDef> columns;
    QStringList columnNames;
    // Per-check pass/fail state consulted by dependent checks (CHK-CORE-005)
    bool columnIdsUnique = true;
    bool columnTypesValid = true;
    bool columnIndexesContiguous = true;
    bool allViewPresent = true;
    // Buffered per-row violations, aggregated on flush (CHK-CORE-006)
    struct RowViolations {
        QString checkId;
        IntegrityCheck::Severity severity;
        QString location;
        QList<IntegrityCheck::Finding> perRow;
    };
    QMap<QString, RowViolations> rowViolations;

    /**
     * Record one finding for the report.
     *
     * @param checkId EARS spec ID of the violated invariant
     * @param severity Whether the violation is an error or warning
     * @param location Which view/row/column/name it was found at
     * @param message Human-readable description
     */
    void addFinding(const QString &checkId, IntegrityCheck::Severity severity,
                    const QString &location, const QString &message)
    {
        IntegrityCheck::Finding finding;
        finding.checkId = checkId;
        finding.severity = severity;
        finding.location = location;
        finding.message = message;
        findings.append(finding);
    }

    /**
     * Record that a check was skipped because a prerequisite check failed.
     *
     * @param checkId EARS spec ID of the check that was skipped
     * @param prerequisiteId EARS spec ID of the failed prerequisite
     */
    void addSkip(const QString &checkId, const QString &prerequisiteId)
    {
        IntegrityCheck::SkippedCheck skip;
        skip.checkId = checkId;
        skip.prerequisiteId = prerequisiteId;
        skipped.append(skip);
    }

    /**
     * Buffer one per-row violation, grouped by check and location, for
     * flushRowViolations() to aggregate.
     *
     * @param checkId EARS spec ID of the violated invariant
     * @param severity Whether the violation is an error or warning
     * @param groupLocation Location shared by the whole violation group
     * @param rowIndex Index of the offending row within the view
     * @param message Human-readable description
     */
    // @spec CHK-CORE-006
    void addRowViolation(const QString &checkId,
                         IntegrityCheck::Severity severity,
                         const QString &groupLocation, int rowIndex,
                         const QString &message)
    {
        QString key = checkId + "|" + groupLocation;
        RowViolations &group = rowViolations[key];
        group.checkId = checkId;
        group.severity = severity;
        group.location = groupLocation;
        IntegrityCheck::Finding finding;
        finding.checkId = checkId;
        finding.severity = severity;
        finding.location = QString("%1, row %2").arg(groupLocation)
                                                .arg(rowIndex);
        finding.message = message;
        group.perRow.append(finding);
    }

    /**
     * Emit each buffered row-violation group as up to ROW_REPORT_LIMIT
     * individual findings plus, if more rows remain, one aggregate finding
     * for the remainder.
     */
    void flushRowViolations()
    {
        for (const RowViolations &group : qAsConst(rowViolations)) {
            for (int i = 0; i < group.perRow.size()
                            && i < ROW_REPORT_LIMIT; i++) {
                findings.append(group.perRow.at(i));
            }
            int remainder = group.perRow.size() - ROW_REPORT_LIMIT;
            if (remainder > 0) {
                IntegrityCheck::Finding aggregate;
                aggregate.checkId = group.checkId;
                aggregate.severity = group.severity;
                aggregate.location = group.location;
                aggregate.message = QString("%1 more rows have the same "
                                            "problem").arg(remainder);
                findings.append(aggregate);
            }
        }
        rowViolations.clear();
    }

    /**
     * Whether the named view exists in the file's storage description.
     *
     * @param name The view name to look up
     * @return True if the view is present
     */
    bool viewPresent(const QString &name) const
    {
        return viewFormats.contains(name);
    }

    /**
     * Whether the named view has a property with the given name.
     *
     * @param viewName The view to check
     * @param propName The property name to look up
     * @return True if the property is present
     */
    bool propPresent(const QString &viewName, const QString &propName) const
    {
        return propNames(viewFormats.value(viewName)).contains(propName);
    }

    /**
     * All string-typed property names of a view (for UTF-8 validation).
     *
     * @param viewName The view to inspect
     * @return The view's string-typed property names
     */
    QStringList stringProps(const QString &viewName) const
    {
        QStringList result;
        const QStringList props = viewFormats.value(viewName);
        for (const QString &prop : props) {
            if (prop.section(':', 1, 1) == "S") {
                result.append(prop.section(':', 0, 0));
            }
        }
        return result;
    }

    /**
     * Perform the full scan: parse the storage description, validate
     * _global, then check either the encrypted outer structure or the full
     * unencrypted catalog. Leaves status/version/findings/skipped set for
     * the constructor to copy out.
     */
    void run()
    {
        // Read-only: the file must never be modified, whatever we find.
        // @spec CHK-CORE-001
        c4_Storage store(QFile::encodeName(path), false);
        storage = &store;
        viewFormats = parseDescription(store.Description());

        // @spec CHK-CORE-002
        if (!viewPresent("_global") || !propPresent("_global", "_gversion")) {
            status = IntegrityCheck::Unreadable;
            return;
        }
        c4_View global = store.View("_global");
        if (global.GetSize() == 0) {
            status = IntegrityCheck::Unreadable;
            return;
        }
        c4_IntProp gVersion("_gversion");
        version = gVersion (global[0]);

        // @spec CHK-GLOBAL-001
        if (global.GetSize() != 1) {
            addFinding("CHK-GLOBAL-001", IntegrityCheck::Error, "_global",
                       QString("expected exactly one row, found %1")
                       .arg(global.GetSize()));
        }
        // @spec CHK-GLOBAL-002
        if (version < OLDEST_VERSION || version > FILE_VERSION) {
            addFinding("CHK-GLOBAL-002", IntegrityCheck::Error, "_global",
                       QString("format version %1 is outside the known "
                               "range %2-%3").arg(version)
                       .arg(OLDEST_VERSION).arg(FILE_VERSION));
        }

        int encrypted = 0;
        if (versionAtLeast(8) && propPresent("_global", "_gcrypt")) {
            c4_IntProp gCrypt("_gcrypt");
            encrypted = gCrypt (global[0]);
        }
        if (encrypted == 1) {
            checkEncryptedOuterStructure();
        }
        else {
            runCatalog(global);
        }
        flushRowViolations();
        status = findings.isEmpty() ? IntegrityCheck::Clean
                                    : IntegrityCheck::HasFindings;
        storage = nullptr;
    }

    /**
     * Version gating: is the check's subject part of this file's format?
     *
     * @param minimum Format version the check's subject was introduced in
     * @return True if the file's (effective) format version is at least that
     */
    // @spec CHK-CORE-003
    bool versionAtLeast(int minimum) const
    {
        // An out-of-range declared version was already reported
        // (CHK-GLOBAL-002); gate the rest of the catalog as if the file were
        // the nearest known version.
        int effective = std::max(OLDEST_VERSION,
                                 std::min(version, FILE_VERSION));
        return effective >= minimum;
    }

    /**
     * Validate an encrypted file's outer structure: only _global and _crypto
     * present, with _crypto shaped correctly. The encrypted contents
     * themselves are never decrypted or checked.
     */
    // @spec CHK-CORE-004
    void checkEncryptedOuterStructure()
    {
        encryptedSkipped = true;
        const QStringList names = viewFormats.keys();
        for (const QString &name : names) {
            if (name != "_global" && name != "_crypto") {
                addFinding("CHK-CORE-004", IntegrityCheck::Error, name,
                           "view present in an encrypted file, which should "
                           "contain only _global and _crypto");
            }
        }
        // @spec CHK-GLOBAL-006
        if (!viewPresent("_crypto")) {
            addFinding("CHK-GLOBAL-006", IntegrityCheck::Error, "_global",
                       "_gcrypt is 1 but the file has no _crypto view");
            return;
        }
        c4_View crypto = storage->View("_crypto");
        if (crypto.GetSize() != 1) {
            addFinding("CHK-CORE-004", IntegrityCheck::Error, "_crypto",
                       QString("expected exactly one row, found %1")
                       .arg(crypto.GetSize()));
        }
        const QStringList expected = QStringList() << "_criv" << "_crhash"
                                                   << "_crdata";
        for (const QString &prop : expected) {
            if (!propPresent("_crypto", prop)) {
                addFinding("CHK-CORE-004", IntegrityCheck::Error, "_crypto",
                           QString("missing the %1 property").arg(prop));
            }
        }
    }

    /**
     * Run the full invariant catalog for an unencrypted file, across all
     * views present at the file's format version.
     *
     * @param global The _global view, for the current view/sorting/filter
     *               pointer checks in checkViews()
     */
    void runCatalog(c4_View &global)
    {
        // @spec CHK-GLOBAL-006
        if (viewPresent("_crypto")) {
            addFinding("CHK-GLOBAL-006", IntegrityCheck::Error, "_global",
                       "_crypto view present but _gcrypt is 0");
        }
        checkMandatoryViews();
        readColumns();
        IntList enumIds;
        QMap<int, QStringList> enumOptions; // id -> option text, by _eoindex
        if (versionAtLeast(4) && viewPresent("_enums")) {
            checkEnums(&enumIds, &enumOptions);
        }
        checkColumns(enumIds, enumOptions);
        if (viewPresent("_data")) {
            checkData(enumIds, enumOptions);
        }
        QStringList sortingNames;
        if (versionAtLeast(2) && viewPresent("_sorts")) {
            checkSortings(&sortingNames);
        }
        QStringList filterNames;
        if (versionAtLeast(3) && viewPresent("_filters")) {
            checkFilters(&filterNames);
        }
        if (viewPresent("_views")) {
            checkViews(global, sortingNames, filterNames);
        }
        if (versionAtLeast(10) && viewPresent("_calcs")) {
            checkCalcs();
        }
        checkUtf8Everywhere();
    }

    /**
     * Flag any top-level view required at the file's format version that is
     * missing.
     */
    // @spec CHK-CORE-008
    void checkMandatoryViews()
    {
        QMap<QString, int> mandatory;
        mandatory.insert("_global", 1);
        mandatory.insert("_columns", 1);
        mandatory.insert("_data", 1);
        mandatory.insert("_views", 1);
        mandatory.insert("_viewcolumns", 1);
        mandatory.insert("_sorts", 2);
        mandatory.insert("_sortcolumns", 2);
        mandatory.insert("_filters", 3);
        mandatory.insert("_filterconditions", 3);
        mandatory.insert("_enums", 4);
        mandatory.insert("_enumoptions", 4);
        mandatory.insert("_calcs", 10);
        mandatory.insert("_calcnodes", 10);
        for (auto iter = mandatory.constBegin();
             iter != mandatory.constEnd(); ++iter) {
            if (versionAtLeast(iter.value()) && !viewPresent(iter.key())) {
                addFinding("CHK-CORE-008", IntegrityCheck::Error, iter.key(),
                           QString("mandatory view missing for a format "
                                   "version %1 file").arg(version));
            }
        }
    }

    /** Read the _columns view into `columns`/`columnNames` for later checks. */
    void readColumns()
    {
        if (!viewPresent("_columns")) {
            return;
        }
        c4_View colView = storage->View("_columns");
        c4_IntProp cIndex("_cindex");
        c4_StringProp cName("_cname");
        c4_IntProp cType("_ctype");
        c4_StringProp cDefault("_cdefault");
        c4_IntProp cId("_cid");
        bool haveIds = versionAtLeast(4) && propPresent("_columns", "_cid");
        int count = colView.GetSize();
        for (int i = 0; i < count; i++) {
            ColumnDef column;
            column.index = cIndex (colView[i]);
            column.name = QString::fromUtf8(cName (colView[i]));
            column.type = cType (colView[i]);
            column.defaultVal = QString::fromUtf8(cDefault (colView[i]));
            column.id = haveIds ? (int)cId (colView[i]) : -1;
            columns.append(column);
            columnNames.append(column.name);
        }
    }

    /**
     * Validate column IDs, names, position indexes, types, and defaults.
     *
     * @param enumIds IDs of enums defined in the file (for type validation)
     * @param enumOptions Each enum ID's option text, by option index (for
     *                    default-value validation)
     */
    void checkColumns(const IntList &enumIds,
                      const QMap<int, QStringList> &enumOptions)
    {
        QSet<int> seenIds;
        QSet<QString> seenNames;
        IntList indexes;
        bool haveIds = versionAtLeast(4) && propPresent("_columns", "_cid");
        for (const ColumnDef &column : qAsConst(columns)) {
            QString location = "_columns: " + (column.name.isEmpty()
                               ? QString("(unnamed, index %1)")
                                 .arg(column.index)
                               : column.name);
            // @spec CHK-COL-001
            if (haveIds) {
                if (seenIds.contains(column.id)) {
                    addFinding("CHK-COL-001", IntegrityCheck::Error, location,
                               QString("column ID %1 is used by more than "
                                       "one column").arg(column.id));
                    columnIdsUnique = false;
                }
                seenIds.insert(column.id);
            }
            // @spec CHK-COL-002
            if (column.name.isEmpty()) {
                addFinding("CHK-COL-002", IntegrityCheck::Error, location,
                           "column name is empty");
            }
            else if (column.name.startsWith("_")) {
                addFinding("CHK-COL-002", IntegrityCheck::Error, location,
                           "column name starts with an underscore, which is "
                           "reserved for internal names");
            }
            else if (seenNames.contains(column.name)) {
                addFinding("CHK-COL-002", IntegrityCheck::Error, location,
                           "column name is used by more than one column");
            }
            seenNames.insert(column.name);
            indexes.append(column.index);
            // @spec CHK-COL-004
            bool knownType = (column.type >= 0
                              && column.type <= LAST_TYPE)
                || (column.type >= FIRST_ENUM
                    && enumIds.contains(column.type));
            if (!knownType) {
                addFinding("CHK-COL-004", IntegrityCheck::Error, location,
                           QString("type code %1 is not a base type or a "
                                   "defined enum").arg(column.type));
                columnTypesValid = false;
                continue;
            }
            // @spec CHK-COL-005
            checkDefault(column, location, enumOptions);
        }
        // @spec CHK-COL-003
        if (!columns.isEmpty() && !isPermutation(indexes)) {
            addFinding("CHK-COL-003", IntegrityCheck::Warning, "_columns",
                       QString("position indexes are not a permutation of "
                               "0..%1").arg(columns.size() - 1));
            columnIndexesContiguous = false;
        }
    }

    /**
     * Validate one column's default value against its type.
     *
     * @param column The column whose default value is being checked
     * @param location Report location for any finding
     * @param enumOptions Each enum ID's option text, by option index (for
     *                    enum-typed columns)
     */
    // @spec CHK-COL-005
    void checkDefault(const ColumnDef &column, const QString &location,
                      const QMap<int, QStringList> &enumOptions)
    {
        // Only types whose defaults have a locale-independent stored form
        // are checked; FLOAT defaults are written with the locale of the
        // machine that saved them, and DATE/TIME defaults use special
        // encoded values ("today"/"now"/none).
        bool ok = true;
        QString expected;
        switch (column.type) {
        case 1: // integer
        case 8: // sequence
            column.defaultVal.toInt(&ok);
            expected = "an integer";
            break;
        case BOOLEAN: {
            int value = column.defaultVal.toInt(&ok);
            ok = ok && (value == 0 || value == 1);
            expected = "0 or 1";
            break;
        }
        default:
            if (column.type >= FIRST_ENUM) {
                ok = column.defaultVal.isEmpty()
                    || enumOptions.value(column.type).contains(
                           column.defaultVal);
                expected = "one of the enum's options";
            }
            break;
        }
        if (!ok) {
            addFinding("CHK-COL-005", IntegrityCheck::Warning, location,
                       QString("default value \"%1\" is not %2")
                       .arg(column.defaultVal, expected));
        }
    }

    /**
     * Validate row IDs, the per-row property set against the column set, and
     * enum value/index consistency.
     *
     * @param enumIds IDs of enums defined in the file
     * @param enumOptions Each enum ID's option text, by option index
     */
    void checkData(const IntList &enumIds,
                   const QMap<int, QStringList> &enumOptions)
    {
        c4_View data = storage->View("_data");
        // @spec CHK-DATA-003
        if (propPresent("_data", "_id")) {
            c4_IntProp id("_id");
            int count = data.GetSize();
            // The clean path keeps no per-row state; the IDs are gathered for
            // the report only once a violation is confirmed, per the HLD
            // tenet that diagnostic scans stay cheap on clean files.
            QBitArray seen(count);
            bool consecutive = true;
            for (int i = 0; i < count && consecutive; i++) {
                int value = (int)id (data[i]);
                if (value < 0 || value >= count || seen.testBit(value)) {
                    consecutive = false;
                }
                else {
                    seen.setBit(value);
                }
            }
            if (!consecutive) {
                IntList ids;
                ids.reserve(count);
                for (int i = 0; i < count; i++) {
                    ids.append((int)id (data[i]));
                }
                addFinding("CHK-DATA-003", IntegrityCheck::Error, "_data",
                           QString("row IDs are not unique and consecutive "
                                   "from 0 to %1 (%2)").arg(count - 1)
                           .arg(describeIdAnomalies(ids)));
            }
        }
        else {
            addFinding("CHK-DATA-001", IntegrityCheck::Error, "_data",
                       "the _id property is missing");
        }
        // The property-set comparison needs trustworthy column IDs and
        // types; a duplicate ID or unknown type makes the expected property
        // set ill-defined. @spec CHK-CORE-005
        if (!versionAtLeast(4)) {
            return; // name-based data properties predate _cid; not checked
        }
        if (!columnIdsUnique) {
            addSkip("CHK-DATA-001", "CHK-COL-001");
            addSkip("CHK-DATA-002", "CHK-COL-001");
            return;
        }
        if (!columnTypesValid) {
            addSkip("CHK-DATA-001", "CHK-COL-004");
            addSkip("CHK-DATA-002", "CHK-COL-004");
            return;
        }
        QStringList expected;
        for (const ColumnDef &column : qAsConst(columns)) {
            expected.append(expectedDataProps(column));
        }
        QStringList stored = propNames(viewFormats.value("_data"));
        // @spec CHK-DATA-001
        for (const QString &prop : qAsConst(expected)) {
            if (!stored.contains(prop)) {
                addFinding("CHK-DATA-001", IntegrityCheck::Error,
                           "_data: " + prop,
                           "property required by the column set is missing "
                           "from the stored format");
            }
        }
        // @spec CHK-DATA-002
        for (const QString &prop : qAsConst(stored)) {
            if (prop != "_id" && !expected.contains(prop)) {
                addFinding("CHK-DATA-002", IntegrityCheck::Warning,
                           "_data: " + prop,
                           "stored property is not derivable from any "
                           "current column (deleted-column residue)");
            }
        }
        // Enum value/index consistency, per row. @spec CHK-DATA-004
        for (const ColumnDef &column : qAsConst(columns)) {
            if (column.type < FIRST_ENUM
                    || !enumIds.contains(column.type)) {
                continue;
            }
            QString textProp = "_S" + QString::number(column.id);
            QString indexProp = "_I" + QString::number(column.id);
            if (!stored.contains(textProp) || !stored.contains(indexProp)) {
                continue; // already reported by CHK-DATA-001
            }
            QByteArray textPropName = textProp.toLatin1();
            QByteArray indexPropName = indexProp.toLatin1();
            c4_StringProp text(textPropName.constData());
            c4_IntProp optionIndex(indexPropName.constData());
            QStringList options = enumOptions.value(column.type);
            QString location = "_data: " + column.name;
            int count = data.GetSize();
            for (int i = 0; i < count; i++) {
                QString value = QString::fromUtf8(text (data[i]));
                int storedIndex = optionIndex (data[i]);
                int expectedIndex = options.indexOf(value);
                if (expectedIndex == -1) {
                    addRowViolation("CHK-DATA-004", IntegrityCheck::Warning,
                                    location, i,
                                    QString("\"%1\" is not an option of this "
                                            "column's enum").arg(value));
                }
                // @spec CHK-DATA-005
                else if (versionAtLeast(8) && storedIndex != expectedIndex) {
                    addRowViolation("CHK-DATA-005", IntegrityCheck::Warning,
                                    location, i,
                                    QString("stored option index %1 does not "
                                            "match \"%2\" (option %3)")
                                    .arg(storedIndex).arg(value)
                                    .arg(expectedIndex));
                }
            }
        }
    }

    /**
     * Validate view names, _all's presence and contents, view-column
     * entries, and _global's current view/sorting/filter pointers.
     *
     * @param global The _global view, for the current view/sorting/filter
     *               pointer checks
     * @param sortingNames Names of sortings defined in the file
     * @param filterNames Names of filters defined in the file
     */
    void checkViews(c4_View &global, const QStringList &sortingNames,
                    const QStringList &filterNames)
    {
        c4_View views = storage->View("_views");
        c4_StringProp vName("_vname");
        c4_StringProp vSort("_vsort");
        c4_StringProp vFilter("_vfilter");
        QStringList viewNames;
        QSet<QString> seen;
        int count = views.GetSize();
        for (int i = 0; i < count; i++) {
            QString name = QString::fromUtf8(vName (views[i]));
            // @spec CHK-VIEW-003
            if (seen.contains(name)) {
                addFinding("CHK-VIEW-003", IntegrityCheck::Warning,
                           "_views: " + name,
                           "view name is used by more than one view");
            }
            seen.insert(name);
            viewNames.append(name);
            // @spec CHK-VIEW-007
            QString location = "_views: " + name;
            if (versionAtLeast(8) && propPresent("_views", "_vsort")) {
                QString sort = QString::fromUtf8(vSort (views[i]));
                if (sort != "_none" && !sortingNames.contains(sort)) {
                    addFinding("CHK-VIEW-007", IntegrityCheck::Error,
                               location,
                               QString("default sorting \"%1\" does not "
                                       "exist").arg(sort));
                }
                QString filter = QString::fromUtf8(vFilter (views[i]));
                if (filter != "_none" && !filterNames.contains(filter)) {
                    addFinding("CHK-VIEW-007", IntegrityCheck::Error,
                               location,
                               QString("default filter \"%1\" does not "
                                       "exist").arg(filter));
                }
            }
        }
        // @spec CHK-GLOBAL-003
        if (propPresent("_global", "_gview")) {
            c4_StringProp gView("_gview");
            QString current = QString::fromUtf8(gView (global[0]));
            if (!viewNames.contains(current)) {
                addFinding("CHK-GLOBAL-003", IntegrityCheck::Error, "_global",
                           QString("current view \"%1\" does not exist")
                           .arg(current));
            }
        }
        // @spec CHK-GLOBAL-004
        if (versionAtLeast(2) && propPresent("_global", "_gsort")) {
            c4_StringProp gSort("_gsort");
            QString current = QString::fromUtf8(gSort (global[0]));
            if (!current.isEmpty() && !sortingNames.contains(current)) {
                addFinding("CHK-GLOBAL-004", IntegrityCheck::Error, "_global",
                           QString("current sorting \"%1\" does not exist")
                           .arg(current));
            }
        }
        // @spec CHK-GLOBAL-005
        if (versionAtLeast(3) && propPresent("_global", "_gfilter")) {
            c4_StringProp gFilter("_gfilter");
            QString current = QString::fromUtf8(gFilter (global[0]));
            if (!current.isEmpty() && !filterNames.contains(current)) {
                addFinding("CHK-GLOBAL-005", IntegrityCheck::Error, "_global",
                           QString("current filter \"%1\" does not exist")
                           .arg(current));
            }
        }
        // @spec CHK-VIEW-001
        if (!viewNames.contains("_all")) {
            addFinding("CHK-VIEW-001", IntegrityCheck::Error, "_views",
                       "no view named _all exists");
            allViewPresent = false;
        }
        if (!viewPresent("_viewcolumns")) {
            return;
        }
        c4_View viewColumns = storage->View("_viewcolumns");
        c4_StringProp vcView("_vcview");
        c4_StringProp vcName("_vcname");
        c4_IntProp vcIndex("_vcindex");
        QMap<QString, IntList> positionsByView;
        QMap<int, QString> allEntriesByPosition;
        count = viewColumns.GetSize();
        for (int i = 0; i < count; i++) {
            QString owner = QString::fromUtf8(vcView (viewColumns[i]));
            QString colName = QString::fromUtf8(vcName (viewColumns[i]));
            int position = vcIndex (viewColumns[i]);
            QString location = QString("_viewcolumns: view %1, column %2")
                               .arg(owner, colName);
            // @spec CHK-VIEW-004
            if (!viewNames.contains(owner)) {
                addFinding("CHK-VIEW-004", IntegrityCheck::Warning, location,
                           "entry belongs to a view that does not exist");
            }
            // @spec CHK-VIEW-005
            if (!columnNames.contains(colName)) {
                addFinding("CHK-VIEW-005", IntegrityCheck::Error, location,
                           "entry references a column that does not exist");
            }
            positionsByView[owner].append(position);
            if (owner == "_all") {
                allEntriesByPosition.insert(position, colName);
            }
        }
        // @spec CHK-VIEW-006
        for (auto iter = positionsByView.constBegin();
             iter != positionsByView.constEnd(); ++iter) {
            if (!isPermutation(iter.value())) {
                addFinding("CHK-VIEW-006", IntegrityCheck::Warning,
                           "_viewcolumns: view " + iter.key(),
                           QString("position indexes are not a permutation "
                                   "of 0..%1").arg(iter.value().size() - 1));
            }
        }
        // @spec CHK-VIEW-002 (and CHK-CORE-005 for its prerequisites)
        if (!allViewPresent) {
            addSkip("CHK-VIEW-002", "CHK-VIEW-001");
        }
        else if (!columnIndexesContiguous) {
            addSkip("CHK-VIEW-002", "CHK-COL-003");
        }
        else {
            QMap<int, QString> columnsByPosition;
            for (const ColumnDef &column : qAsConst(columns)) {
                columnsByPosition.insert(column.index, column.name);
            }
            // Both maps are keyed by position, so .values() yields both
            // sides in position order.
            if (allEntriesByPosition.values() != columnsByPosition.values()) {
                addFinding("CHK-VIEW-002", IntegrityCheck::Error,
                           "_viewcolumns: view _all",
                           "the _all view's entries are not exactly the "
                           "column set in position order");
            }
        }
    }

    /**
     * Validate sorting names and their column entries.
     *
     * @param sortingNames Out param: names of sortings found in the file
     */
    void checkSortings(QStringList *sortingNames)
    {
        c4_View sorts = storage->View("_sorts");
        c4_StringProp sName("_sname");
        QSet<QString> seen;
        int count = sorts.GetSize();
        for (int i = 0; i < count; i++) {
            QString name = QString::fromUtf8(sName (sorts[i]));
            // @spec CHK-SORT-001
            if (seen.contains(name)) {
                addFinding("CHK-SORT-001", IntegrityCheck::Warning,
                           "_sorts: " + name,
                           "sorting name is used by more than one sorting");
            }
            seen.insert(name);
            sortingNames->append(name);
        }
        if (!viewPresent("_sortcolumns")) {
            return;
        }
        c4_View sortColumns = storage->View("_sortcolumns");
        c4_StringProp scSort("_scsort");
        c4_StringProp scName("_scname");
        c4_IntProp scIndex("_scindex");
        c4_IntProp scDesc("_scdesc");
        QMap<QString, IntList> positionsBySorting;
        count = sortColumns.GetSize();
        for (int i = 0; i < count; i++) {
            QString owner = QString::fromUtf8(scSort (sortColumns[i]));
            QString colName = QString::fromUtf8(scName (sortColumns[i]));
            QString location = QString("_sortcolumns: sorting %1, column %2")
                               .arg(owner, colName);
            // @spec CHK-SORT-002
            if (!sortingNames->contains(owner)) {
                addFinding("CHK-SORT-002", IntegrityCheck::Warning, location,
                           "entry belongs to a sorting that does not exist");
            }
            // @spec CHK-SORT-003
            if (!columnNames.contains(colName)) {
                addFinding("CHK-SORT-003", IntegrityCheck::Error, location,
                           "entry references a column that does not exist");
            }
            // @spec CHK-SORT-004
            int descending = scDesc (sortColumns[i]);
            if (descending != 0 && descending != 1) {
                addFinding("CHK-SORT-004", IntegrityCheck::Warning, location,
                           QString("descending flag is %1, not 0 or 1")
                           .arg(descending));
            }
            positionsBySorting[owner].append((int)scIndex (sortColumns[i]));
        }
        for (auto iter = positionsBySorting.constBegin();
             iter != positionsBySorting.constEnd(); ++iter) {
            if (!isPermutation(iter.value())) {
                addFinding("CHK-SORT-004", IntegrityCheck::Warning,
                           "_sortcolumns: sorting " + iter.key(),
                           QString("position indexes are not a permutation "
                                   "of 0..%1").arg(iter.value().size() - 1));
            }
        }
    }

    /**
     * Validate filter names, the mandatory _allrows filter, and filter
     * conditions.
     *
     * @param filterNames Out param: names of filters found in the file
     */
    void checkFilters(QStringList *filterNames)
    {
        c4_View filters = storage->View("_filters");
        c4_StringProp fName("_fname");
        QSet<QString> seen;
        int count = filters.GetSize();
        for (int i = 0; i < count; i++) {
            QString name = QString::fromUtf8(fName (filters[i]));
            // @spec CHK-FILTER-002
            if (seen.contains(name)) {
                addFinding("CHK-FILTER-002", IntegrityCheck::Warning,
                           "_filters: " + name,
                           "filter name is used by more than one filter");
            }
            seen.insert(name);
            filterNames->append(name);
        }
        // @spec CHK-FILTER-001
        if (!filterNames->contains("_allrows")) {
            addFinding("CHK-FILTER-001", IntegrityCheck::Warning, "_filters",
                       "the mandatory _allrows filter is missing");
        }
        if (!viewPresent("_filterconditions")) {
            return;
        }
        c4_View conditions = storage->View("_filterconditions");
        c4_StringProp fcFilter("_fcfilter");
        c4_StringProp fcColumn("_fccolumn");
        c4_IntProp fcPosition("_fcposition");
        c4_IntProp fcOperator("_fcoperator");
        c4_IntProp fcCase("_fccase");
        bool havePositions = propPresent("_filterconditions", "_fcposition");
        QMap<QString, IntList> positionsByFilter;
        count = conditions.GetSize();
        for (int i = 0; i < count; i++) {
            QString owner = QString::fromUtf8(fcFilter (conditions[i]));
            QString colName = QString::fromUtf8(fcColumn (conditions[i]));
            QString location = QString("_filterconditions: filter %1, "
                                       "column %2").arg(owner, colName);
            // @spec CHK-FILTER-003
            if (!filterNames->contains(owner)) {
                addFinding("CHK-FILTER-003", IntegrityCheck::Warning,
                           location,
                           "condition belongs to a filter that does not "
                           "exist");
            }
            // @spec CHK-FILTER-004
            if (colName != "_anytext" && !columnNames.contains(colName)) {
                addFinding("CHK-FILTER-004", IntegrityCheck::Error, location,
                           "condition references a column that does not "
                           "exist");
            }
            // @spec CHK-FILTER-005
            int op = fcOperator (conditions[i]);
            if (op < 0 || op > 7) {
                addFinding("CHK-FILTER-005", IntegrityCheck::Warning,
                           location,
                           QString("operator code %1 is outside 0-7")
                           .arg(op));
            }
            int caseSensitive = fcCase (conditions[i]);
            if (caseSensitive != 0 && caseSensitive != 1) {
                addFinding("CHK-FILTER-005", IntegrityCheck::Warning,
                           location,
                           QString("case-sensitivity flag is %1, not 0 or 1")
                           .arg(caseSensitive));
            }
            if (havePositions) {
                positionsByFilter[owner].append(
                    (int)fcPosition (conditions[i]));
            }
        }
        for (auto iter = positionsByFilter.constBegin();
             iter != positionsByFilter.constEnd(); ++iter) {
            if (!isPermutation(iter.value())) {
                addFinding("CHK-FILTER-005", IntegrityCheck::Warning,
                           "_filterconditions: filter " + iter.key(),
                           QString("position indexes are not a permutation "
                                   "of 0..%1").arg(iter.value().size() - 1));
            }
        }
    }

    /**
     * Validate enum names/IDs and their options.
     *
     * @param enumIds Out param: IDs of enums found in the file
     * @param enumOptions Out param: each enum ID's option text, by option
     *                    index
     */
    void checkEnums(IntList *enumIds, QMap<int, QStringList> *enumOptions)
    {
        c4_View enums = storage->View("_enums");
        c4_StringProp eName("_ename");
        c4_IntProp eId("_eid");
        c4_IntProp eIndex("_eindex");
        QSet<QString> seenNames;
        IntList indexes;
        int count = enums.GetSize();
        for (int i = 0; i < count; i++) {
            QString name = QString::fromUtf8(eName (enums[i]));
            int id = eId (enums[i]);
            QString location = "_enums: " + name;
            // @spec CHK-ENUM-001
            if (seenNames.contains(name)) {
                addFinding("CHK-ENUM-001", IntegrityCheck::Error, location,
                           "enum name is used by more than one enum");
            }
            seenNames.insert(name);
            if (enumIds->contains(id)) {
                addFinding("CHK-ENUM-001", IntegrityCheck::Error, location,
                           QString("enum ID %1 is used by more than one "
                                   "enum").arg(id));
            }
            if (id < FIRST_ENUM) {
                addFinding("CHK-ENUM-001", IntegrityCheck::Error, location,
                           QString("enum ID %1 is below the minimum of %2")
                           .arg(id).arg(FIRST_ENUM));
            }
            enumIds->append(id);
            indexes.append((int)eIndex (enums[i]));
        }
        // @spec CHK-ENUM-002
        if (count > 0 && !isPermutation(indexes)) {
            addFinding("CHK-ENUM-002", IntegrityCheck::Warning, "_enums",
                       QString("position indexes are not a permutation of "
                               "0..%1").arg(count - 1));
        }
        if (!viewPresent("_enumoptions")) {
            return;
        }
        c4_View options = storage->View("_enumoptions");
        c4_IntProp eoEnum("_eoenum");
        c4_IntProp eoIndex("_eoindex");
        c4_StringProp eoText("_eotext");
        QMap<int, IntList> indexesByEnum;
        QMap<int, QMap<int, QString> > optionsByEnum;
        count = options.GetSize();
        for (int i = 0; i < count; i++) {
            int owner = eoEnum (options[i]);
            QString text = QString::fromUtf8(eoText (options[i]));
            // @spec CHK-ENUM-003
            if (!enumIds->contains(owner)) {
                addFinding("CHK-ENUM-003", IntegrityCheck::Warning,
                           "_enumoptions: " + text,
                           QString("option belongs to enum ID %1, which does "
                                   "not exist").arg(owner));
            }
            indexesByEnum[owner].append((int)eoIndex (options[i]));
            optionsByEnum[owner].insert((int)eoIndex (options[i]), text);
        }
        // @spec CHK-ENUM-004
        for (auto iter = indexesByEnum.constBegin();
             iter != indexesByEnum.constEnd(); ++iter) {
            if (!isPermutation(iter.value())) {
                addFinding("CHK-ENUM-004", IntegrityCheck::Warning,
                           QString("_enumoptions: enum ID %1").arg(iter.key()),
                           QString("option indexes are not a permutation of "
                                   "0..%1").arg(iter.value().size() - 1));
            }
        }
        for (auto iter = optionsByEnum.constBegin();
             iter != optionsByEnum.constEnd(); ++iter) {
            enumOptions->insert(iter.key(), iter.value().values());
        }
    }

    /** Validate calculation definitions and their expression-tree nodes. */
    void checkCalcs()
    {
        c4_View calcs = storage->View("_calcs");
        c4_IntProp calcId("_calcid");
        IntList calcIds;
        int count = calcs.GetSize();
        for (int i = 0; i < count; i++) {
            calcIds.append((int)calcId (calcs[i]));
        }
        IntList calcColumnIds;
        for (const ColumnDef &column : qAsConst(columns)) {
            if (column.type == CALC) {
                calcColumnIds.append(column.id);
                // @spec CHK-CALC-001
                if (calcIds.count(column.id) != 1) {
                    addFinding("CHK-CALC-001", IntegrityCheck::Warning,
                               "_calcs: " + column.name,
                               QString("calculation column has %1 "
                                       "calculation definitions instead of "
                                       "one").arg(calcIds.count(column.id)));
                }
            }
        }
        // @spec CHK-CALC-002
        for (const int id : qAsConst(calcIds)) {
            if (!calcColumnIds.contains(id)) {
                addFinding("CHK-CALC-002", IntegrityCheck::Warning,
                           QString("_calcs: calculation ID %1").arg(id),
                           "calculation does not match any calculation-type "
                           "column");
            }
        }
        if (!viewPresent("_calcnodes")) {
            return;
        }
        c4_View nodes = storage->View("_calcnodes");
        c4_IntProp cnId("_cnid");
        c4_IntProp cnType("_cntype");
        c4_StringProp cnValue("_cnvalue");
        count = nodes.GetSize();
        for (int i = 0; i < count; i++) {
            int owner = cnId (nodes[i]);
            int type = cnType (nodes[i]);
            QString value = QString::fromUtf8(cnValue (nodes[i]));
            QString location = QString("_calcnodes: calculation ID %1")
                               .arg(owner);
            // @spec CHK-CALC-003
            if (!calcIds.contains(owner)) {
                addRowViolation("CHK-CALC-003", IntegrityCheck::Warning,
                                location, i,
                                "node belongs to a calculation that does not "
                                "exist");
            }
            // @spec CHK-CALC-004
            if ((type == 1 || type == 3 || type == 5)
                    && !columnNames.contains(value)) {
                addFinding("CHK-CALC-004", IntegrityCheck::Error, location,
                           QString("node references column \"%1\", which "
                                   "does not exist").arg(value));
            }
            // @spec CHK-CALC-005
            bool knownType = (type >= 0 && type <= 5)
                || (type >= 20 && type <= 34);
            if (!knownType) {
                addFinding("CHK-CALC-005", IntegrityCheck::Warning, location,
                           QString("node type code %1 is not documented")
                           .arg(type));
            }
        }
    }

    /** Flag any stored string property that is not valid UTF-8. */
    // @spec CHK-CORE-007
    void checkUtf8Everywhere()
    {
        const QStringList names = viewFormats.keys();
        for (const QString &viewName : names) {
            const QStringList textProps = stringProps(viewName);
            if (textProps.isEmpty()) {
                continue;
            }
            QByteArray viewNameBytes = viewName.toLatin1();
            c4_View view = storage->View(viewNameBytes.constData());
            int count = view.GetSize();
            for (const QString &propName : textProps) {
                QByteArray propNameBytes = propName.toLatin1();
                c4_StringProp prop(propNameBytes.constData());
                QString location = QString("%1: %2").arg(viewName, propName);
                for (int i = 0; i < count; i++) {
                    QByteArray bytes((const char *)prop (view[i]));
                    if (!isValidUtf8(bytes)) {
                        addRowViolation("CHK-CORE-007",
                                        IntegrityCheck::Warning, location, i,
                                        "stored string is not valid UTF-8");
                    }
                }
            }
        }
    }
};

}

/**
 * Check the file at the given path.  The scan runs immediately here; the
 * accessors below expose its results.  The file is only ever read, never
 * written.
 *
 * @param filePath The path of the PortaBase file to check
 */
IntegrityCheck::IntegrityCheck(const QString &filePath)
    : path(filePath), fileStatus(Unreadable), version(-1),
      encryptedSkipped(false)
{
    Scan scan;
    scan.path = filePath;
    scan.run();
    fileStatus = scan.status;
    version = scan.version;
    encryptedSkipped = scan.encryptedSkipped;
    foundProblems = scan.findings;
    skipped = scan.skipped;
}

/**
 * Get the path of the file that was checked.
 *
 * @return The file path passed to the constructor
 */
QString IntegrityCheck::filePath() const
{
    return path;
}

/**
 * Get the overall result of checking the file.
 *
 * @return Clean, HasFindings, or Unreadable
 */
IntegrityCheck::FileStatus IntegrityCheck::status() const
{
    return fileStatus;
}

/**
 * Get the file's declared PortaBase format version.
 *
 * @return The _gversion value, or -1 if the file could not be read
 */
int IntegrityCheck::formatVersion() const
{
    return version;
}

/**
 * Determine whether the file was encrypted and so had only its outer
 * structure checked rather than its contents.
 *
 * @return True if the encrypted contents were not checked
 */
bool IntegrityCheck::encryptedContentsSkipped() const
{
    return encryptedSkipped;
}

/**
 * Get the invariant violations found in the file.
 *
 * @return The list of findings, empty if the file is clean
 */
QList<IntegrityCheck::Finding> IntegrityCheck::findings() const
{
    return foundProblems;
}

/**
 * Get the checks that were skipped because a prerequisite check failed.
 *
 * @return The list of skipped checks
 */
QList<IntegrityCheck::SkippedCheck> IntegrityCheck::skippedChecks() const
{
    return skipped;
}

/**
 * Build the human-readable report for the checked file: its name, format
 * version, each finding and skipped check, and a counts summary.  A clean
 * file yields only its name, version, and a clean summary line.
 *
 * @return The multi-line report text
 */
// @spec CHK-CLI-002, CHK-CLI-003
QString IntegrityCheck::report() const
{
    QString result;
    QTextStream out(&result);
    if (fileStatus == Unreadable) {
        out << path << ": not a readable PortaBase file\n";
        return result;
    }
    out << path << ": PortaBase format version " << version;
    if (fileStatus == Clean) {
        out << " -- clean\n";
        if (encryptedSkipped) {
            out << "  note: encrypted file, contents were not checked\n";
        }
        return result;
    }
    out << "\n";
    if (encryptedSkipped) {
        out << "  note: encrypted file, contents were not checked\n";
    }
    int errors = 0;
    int warnings = 0;
    for (const Finding &finding : foundProblems) {
        if (finding.severity == Error) {
            out << "  ERROR   ";
            errors++;
        }
        else {
            out << "  WARNING ";
            warnings++;
        }
        out << finding.checkId << " (" << finding.location << "): "
            << finding.message << "\n";
    }
    for (const SkippedCheck &skip : skipped) {
        out << "  skipped " << skip.checkId << ": prerequisite "
            << skip.prerequisiteId << " failed\n";
    }
    out << "  " << errors << " error(s), " << warnings << " warning(s)\n";
    return result;
}

/**
 * Combine the per-file check results into a process exit code for the
 * `check` command: 0 if every file is clean, 1 if any produced findings, 2
 * if any was unreadable.  Unreadable (2) takes precedence over findings (1).
 *
 * @param statuses The result status of each checked file
 * @return The exit code
 */
// @spec CHK-CLI-004
int IntegrityCheck::exitCode(const QList<FileStatus> &statuses)
{
    int code = 0;
    for (const FileStatus status : statuses) {
        if (status == Unreadable) {
            code = std::max(code, 2);
        }
        else if (status == HasFindings) {
            code = std::max(code, 1);
        }
    }
    return code;
}
