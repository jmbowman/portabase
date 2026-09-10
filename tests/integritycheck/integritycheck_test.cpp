/*
 * integritycheck_test.cpp
 *
 * (c) 2026 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Tests for the IntegrityCheck scanner (src/integritycheck.h), written
 * tests-first against docs/intent/integrity-check/integrity-check-specs.md.
 * They fail against the current skeleton implementation and pass once the
 * checker is implemented.
 *
 * The invariant catalog is exercised table-driven: each row builds a fresh
 * copy of a structurally valid fixture, seeds exactly one violation with a
 * raw-Metakit corruption helper, and asserts the expected finding (check ID
 * and severity) is reported.
 */

#include <QTest>
#include <QCryptographicHash>
#include <QProcess>
#include <QTemporaryDir>
#include <functional>

#include "integritycheck.h"
#include "pobcorruptor.h"
#include "testfixtures.h"

using PobCorrupt::Where;

/** Applies one corruption to the file at the given path. */
typedef std::function<bool (const QString &)> CorruptFn;
Q_DECLARE_METATYPE(CorruptFn)

namespace {

QStringList findingIds(const IntegrityCheck &check)
{
    QStringList ids;
    const QList<IntegrityCheck::Finding> findings = check.findings();
    for (const IntegrityCheck::Finding &finding : findings) {
        ids.append(finding.checkId);
    }
    return ids;
}

QString findingMessage(const IntegrityCheck &check, const QString &checkId)
{
    const QList<IntegrityCheck::Finding> findings = check.findings();
    for (const IntegrityCheck::Finding &finding : findings) {
        if (finding.checkId == checkId) {
            return finding.message;
        }
    }
    return QString();
}

int countFindings(const IntegrityCheck &check, const QString &checkId)
{
    int count = 0;
    const QList<IntegrityCheck::Finding> findings = check.findings();
    for (const IntegrityCheck::Finding &finding : findings) {
        if (finding.checkId == checkId) {
            count++;
        }
    }
    return count;
}

QByteArray fileHash(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return QByteArray();
    }
    return QCryptographicHash::hash(file.readAll(), QCryptographicHash::Sha1);
}

/** The full list of top-level views in a current-format (v11) file. */
QStringList allV11Views()
{
    QStringList views;
    views << "_global" << "_columns" << "_data" << "_views" << "_viewcolumns"
          << "_sorts" << "_sortcolumns" << "_filters" << "_filterconditions"
          << "_enums" << "_enumoptions" << "_calcs" << "_calcnodes";
    return views;
}

}

class TestIntegrityCheck : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void cleanFileHasNoFindings();
    void catalog_data();
    void catalog();
    void neverWritesToCheckedFile();
    void garbageFileIsUnreadable();
    void metakitFileWithoutGlobalIsUnreadable();
    void checksAreVersionGated();
    void encryptedFileOuterStructureOnly();
    void encryptedFileWithStrayViewIsFlagged();
    void dependentCheckSkippedWithNote();
    void perRowFindingsAreAggregated();
    void offsetIdBlockIsReportedAsARange();
    void missingMandatoryViewsAreFlagged();
    void reportListsFindings();
    void cleanReportIsOneSummary();
    void exitCodes();
    void cliCheckCommand();

private:
    /** Copy the valid fixture into dir and return the copy's path. */
    QString makeWorkingCopy(QTemporaryDir &dir)
    {
        QString path = dir.filePath("work.pob");
        if (!dir.isValid() || !QFile::copy(fixturePath, path)) {
            return QString();
        }
        return path;
    }

    QTemporaryDir fixtureDir;
    QString fixturePath;
};

void TestIntegrityCheck::initTestCase()
{
    QVERIFY(fixtureDir.isValid());
    fixturePath = fixtureDir.filePath("fixture.pob");
    QString error = PbTest::buildStandardFile(fixturePath);
    QVERIFY2(error.isEmpty(), qPrintable(error));
}

/**
 * The structurally valid fixture must scan clean -- the catalog produces no
 * false positives on a healthy current-format file.
 */
void TestIntegrityCheck::cleanFileHasNoFindings()
{
    IntegrityCheck check(fixturePath);
    QCOMPARE(check.status(), IntegrityCheck::Clean);
    QCOMPARE(check.formatVersion(), 11);
    QVERIFY2(check.findings().isEmpty(),
             qPrintable(findingIds(check).join(", ")));
}

/**
 * One table row per invariant in the catalog: seed exactly one violation,
 * expect exactly the corresponding finding.
 *
 * In the standard fixture (see tests/common/testfixtures.h): column IDs are
 * 0-10 in position order, the enum column colStatus has ID 9 (data
 * properties _S9/_I9), and the "Status" enum's options are Open (index 0)
 * and Closed (index 1).
 */
// @spec CHK-GLOBAL-001, CHK-GLOBAL-002, CHK-GLOBAL-003, CHK-GLOBAL-004, CHK-GLOBAL-005, CHK-GLOBAL-006
// @spec CHK-COL-001, CHK-COL-002, CHK-COL-003, CHK-COL-004, CHK-COL-005
// @spec CHK-DATA-001, CHK-DATA-002, CHK-DATA-003, CHK-DATA-004, CHK-DATA-005
// @spec CHK-VIEW-001, CHK-VIEW-002, CHK-VIEW-003, CHK-VIEW-004, CHK-VIEW-005, CHK-VIEW-006, CHK-VIEW-007
// @spec CHK-SORT-001, CHK-SORT-002, CHK-SORT-003, CHK-SORT-004
// @spec CHK-FILTER-001, CHK-FILTER-002, CHK-FILTER-003, CHK-FILTER-004, CHK-FILTER-005
// @spec CHK-ENUM-001, CHK-ENUM-002, CHK-ENUM-003, CHK-ENUM-004
// @spec CHK-CALC-001, CHK-CALC-002, CHK-CALC-003, CHK-CALC-004, CHK-CALC-005
// @spec CHK-CORE-007
void TestIntegrityCheck::catalog_data()
{
    QTest::addColumn<CorruptFn>("corrupt");
    QTest::addColumn<QString>("expectedId");
    QTest::addColumn<int>("expectedSeverity");

    auto row = [](const char *name, CorruptFn fn, const char *id,
                  IntegrityCheck::Severity severity) {
        QTest::newRow(name) << fn << QString(id) << (int)severity;
    };
    const IntegrityCheck::Severity ERR = IntegrityCheck::Error;
    const IntegrityCheck::Severity WARN = IntegrityCheck::Warning;

    row("global: two rows", [](const QString &p) {
        return PobCorrupt::duplicateRowAt(p, "_global", 0);
    }, "CHK-GLOBAL-001", ERR);
    row("global: version out of range", [](const QString &p) {
        return PobCorrupt::setIntAt(p, "_global", 0, "_gversion", 99);
    }, "CHK-GLOBAL-002", ERR);
    row("global: current view missing", [](const QString &p) {
        return PobCorrupt::setStringAt(p, "_global", 0, "_gview", "ghostView");
    }, "CHK-GLOBAL-003", ERR);
    row("global: current sorting missing", [](const QString &p) {
        return PobCorrupt::setStringAt(p, "_global", 0, "_gsort", "ghostSort");
    }, "CHK-GLOBAL-004", ERR);
    row("global: current filter missing", [](const QString &p) {
        return PobCorrupt::setStringAt(p, "_global", 0, "_gfilter", "ghost");
    }, "CHK-GLOBAL-005", ERR);
    row("global: _crypto present but _gcrypt 0", [](const QString &p) {
        return PobCorrupt::addCryptoView(p);
    }, "CHK-GLOBAL-006", ERR);

    row("columns: duplicate ID", [](const QString &p) {
        return PobCorrupt::setInt(p, "_columns", Where("_cname", "colInteger"),
                                  "_cid", 0);
    }, "CHK-COL-001", ERR);
    row("columns: duplicate name", [](const QString &p) {
        return PobCorrupt::setString(p, "_columns",
                                     Where("_cname", "colInteger"),
                                     "_cname", "colString");
    }, "CHK-COL-002", ERR);
    row("columns: empty name", [](const QString &p) {
        return PobCorrupt::setString(p, "_columns",
                                     Where("_cname", "colInteger"),
                                     "_cname", "");
    }, "CHK-COL-002", ERR);
    row("columns: underscore name", [](const QString &p) {
        return PobCorrupt::setString(p, "_columns",
                                     Where("_cname", "colInteger"),
                                     "_cname", "_sneaky");
    }, "CHK-COL-002", ERR);
    row("columns: duplicate index", [](const QString &p) {
        return PobCorrupt::setInt(p, "_columns", Where("_cname", "colInteger"),
                                  "_cindex", 0);
    }, "CHK-COL-003", WARN);
    row("columns: invalid type code", [](const QString &p) {
        return PobCorrupt::setInt(p, "_columns", Where("_cname", "colInteger"),
                                  "_ctype", 55);
    }, "CHK-COL-004", ERR);
    row("columns: unparseable default", [](const QString &p) {
        return PobCorrupt::setString(p, "_columns",
                                     Where("_cname", "colInteger"),
                                     "_cdefault", "notanumber");
    }, "CHK-COL-005", WARN);
    row("columns: invalid UTF-8 in name", [](const QString &p) {
        return PobCorrupt::setString(p, "_columns", Where("_cname", "colNote"),
                                     "_cname", QByteArray("\xff\xfe" "bad"));
    }, "CHK-CORE-007", WARN);

    // Changing a column's ID makes the property the column set expects
    // (_I77) missing from the stored format, and strands the old property
    // (_I1) as residue -- one corruption, two distinct findings.
    row("data: expected property missing", [](const QString &p) {
        return PobCorrupt::setInt(p, "_columns", Where("_cname", "colInteger"),
                                  "_cid", 77);
    }, "CHK-DATA-001", ERR);
    row("data: residual property", [](const QString &p) {
        return PobCorrupt::setInt(p, "_columns", Where("_cname", "colInteger"),
                                  "_cid", 77);
    }, "CHK-DATA-002", WARN);
    row("data: broken row ID sequence", [](const QString &p) {
        return PobCorrupt::setIntAt(p, "_data", 0, "_id", 5);
    }, "CHK-DATA-003", ERR);
    row("data: enum value not an option", [](const QString &p) {
        return PobCorrupt::setStringAt(p, "_data", 0, "_S9", "Bogus");
    }, "CHK-DATA-004", WARN);
    row("data: enum index mismatch", [](const QString &p) {
        return PobCorrupt::setIntAt(p, "_data", 0, "_I9", 1);
    }, "CHK-DATA-005", WARN);

    row("views: _all missing", [](const QString &p) {
        return PobCorrupt::removePortabaseView(p, "_all");
    }, "CHK-VIEW-001", ERR);
    row("views: _all missing a column", [](const QString &p) {
        return PobCorrupt::removeRows(p, "_viewcolumns",
                                      Where("_vcview", "_all"),
                                      Where("_vcname", "colFloat"));
    }, "CHK-VIEW-002", ERR);
    row("views: _all order mismatch", [](const QString &p) {
        return PobCorrupt::setInt(p, "_viewcolumns", Where("_vcview", "_all"),
                                  Where("_vcname", "colString"), "_vcindex", 1)
            && PobCorrupt::setInt(p, "_viewcolumns", Where("_vcview", "_all"),
                                  Where("_vcname", "colInteger"),
                                  "_vcindex", 0);
    }, "CHK-VIEW-002", ERR);
    row("views: duplicate view name", [](const QString &p) {
        return PobCorrupt::duplicateRow(p, "_views", Where("_vname", "intView"));
    }, "CHK-VIEW-003", WARN);
    row("views: orphaned view-column entry", [](const QString &p) {
        return PobCorrupt::setString(p, "_viewcolumns",
                                     Where("_vcview", "intView"),
                                     Where("_vcname", "colString"),
                                     "_vcview", "ghostView");
    }, "CHK-VIEW-004", WARN);
    row("views: view references missing column", [](const QString &p) {
        return PobCorrupt::setString(p, "_viewcolumns",
                                     Where("_vcview", "intView"),
                                     Where("_vcname", "colString"),
                                     "_vcname", "colGhost");
    }, "CHK-VIEW-005", ERR);
    row("views: duplicate position in view", [](const QString &p) {
        return PobCorrupt::setInt(p, "_viewcolumns",
                                  Where("_vcview", "intView"),
                                  Where("_vcname", "colInteger"),
                                  "_vcindex", 1);
    }, "CHK-VIEW-006", WARN);
    row("views: default sorting missing", [](const QString &p) {
        return PobCorrupt::setString(p, "_views", Where("_vname", "intView"),
                                     "_vsort", "ghostSort");
    }, "CHK-VIEW-007", ERR);

    row("sorts: duplicate sorting name", [](const QString &p) {
        return PobCorrupt::duplicateRow(p, "_sorts", Where("_sname", "intSort"));
    }, "CHK-SORT-001", WARN);
    row("sorts: orphaned sorting column", [](const QString &p) {
        return PobCorrupt::setString(p, "_sortcolumns",
                                     Where("_scsort", "intSort"),
                                     "_scsort", "ghostSort");
    }, "CHK-SORT-002", WARN);
    row("sorts: sorting references missing column", [](const QString &p) {
        return PobCorrupt::setString(p, "_sortcolumns",
                                     Where("_scsort", "intSort"),
                                     "_scname", "colGhost");
    }, "CHK-SORT-003", ERR);
    row("sorts: broken position sequence", [](const QString &p) {
        return PobCorrupt::setInt(p, "_sortcolumns",
                                  Where("_scsort", "intSort"), "_scindex", 3);
    }, "CHK-SORT-004", WARN);
    row("sorts: invalid descending flag", [](const QString &p) {
        return PobCorrupt::setInt(p, "_sortcolumns",
                                  Where("_scsort", "intSort"), "_scdesc", 2);
    }, "CHK-SORT-004", WARN);

    row("filters: _allrows missing", [](const QString &p) {
        return PobCorrupt::removeRows(p, "_filters", Where("_fname", "_allrows"));
    }, "CHK-FILTER-001", WARN);
    row("filters: duplicate filter name", [](const QString &p) {
        return PobCorrupt::duplicateRow(p, "_filters",
                                        Where("_fname", "intFilter"));
    }, "CHK-FILTER-002", WARN);
    row("filters: orphaned condition", [](const QString &p) {
        return PobCorrupt::setString(p, "_filterconditions",
                                     Where("_fcfilter", "intFilter"),
                                     "_fcfilter", "ghostFilter");
    }, "CHK-FILTER-003", WARN);
    row("filters: condition references missing column", [](const QString &p) {
        return PobCorrupt::setString(p, "_filterconditions",
                                     Where("_fcfilter", "intFilter"),
                                     "_fccolumn", "colGhost");
    }, "CHK-FILTER-004", ERR);
    row("filters: invalid operator", [](const QString &p) {
        return PobCorrupt::setInt(p, "_filterconditions",
                                  Where("_fcfilter", "intFilter"),
                                  "_fcoperator", 55);
    }, "CHK-FILTER-005", WARN);
    row("filters: broken position sequence", [](const QString &p) {
        return PobCorrupt::setInt(p, "_filterconditions",
                                  Where("_fcfilter", "intFilter"),
                                  "_fcposition", 3);
    }, "CHK-FILTER-005", WARN);

    row("enums: ID below 100", [](const QString &p) {
        return PobCorrupt::setInt(p, "_enums", Where("_ename", "Status"),
                                  "_eid", 5);
    }, "CHK-ENUM-001", ERR);
    row("enums: duplicate enum", [](const QString &p) {
        return PobCorrupt::duplicateRow(p, "_enums", Where("_ename", "Status"));
    }, "CHK-ENUM-001", ERR);
    row("enums: broken index sequence", [](const QString &p) {
        return PobCorrupt::setInt(p, "_enums", Where("_ename", "Status"),
                                  "_eindex", 4);
    }, "CHK-ENUM-002", WARN);
    row("enums: orphaned option", [](const QString &p) {
        return PobCorrupt::setInt(p, "_enumoptions", Where("_eotext", "Open"),
                                  "_eoenum", 999);
    }, "CHK-ENUM-003", WARN);
    row("enums: broken option index sequence", [](const QString &p) {
        return PobCorrupt::setInt(p, "_enumoptions", Where("_eotext", "Closed"),
                                  "_eoindex", 5);
    }, "CHK-ENUM-004", WARN);

    row("calcs: CALC column without calculation", [](const QString &p) {
        return PobCorrupt::removeRowAt(p, "_calcs", 0);
    }, "CHK-CALC-001", WARN);
    row("calcs: orphaned calculation", [](const QString &p) {
        return PobCorrupt::setIntAt(p, "_calcs", 0, "_calcid", 77);
    }, "CHK-CALC-002", WARN);
    row("calcs: orphaned nodes", [](const QString &p) {
        bool ok = true;
        for (int i = 0; i < 3; i++) {
            ok = PobCorrupt::setIntAt(p, "_calcnodes", i, "_cnid", 77) && ok;
        }
        return ok;
    }, "CHK-CALC-003", WARN);
    row("calcs: node references missing column", [](const QString &p) {
        return PobCorrupt::setString(p, "_calcnodes",
                                     Where("_cnvalue", "colInteger"),
                                     "_cnvalue", "colGhost");
    }, "CHK-CALC-004", ERR);
    row("calcs: invalid node type", [](const QString &p) {
        return PobCorrupt::setInt(p, "_calcnodes",
                                  Where("_cnvalue", "colInteger"),
                                  "_cntype", 99);
    }, "CHK-CALC-005", WARN);
}

void TestIntegrityCheck::catalog()
{
    QFETCH(CorruptFn, corrupt);
    QFETCH(QString, expectedId);
    QFETCH(int, expectedSeverity);

    QTemporaryDir dir;
    QString path = makeWorkingCopy(dir);
    QVERIFY(!path.isEmpty());
    QVERIFY2(corrupt(path), "corruption helper did not apply");

    IntegrityCheck check(path);
    QCOMPARE(check.status(), IntegrityCheck::HasFindings);
    bool found = false;
    const QList<IntegrityCheck::Finding> findings = check.findings();
    for (const IntegrityCheck::Finding &finding : findings) {
        if (finding.checkId == expectedId) {
            QCOMPARE((int)finding.severity, expectedSeverity);
            QVERIFY2(!finding.location.isEmpty(), "finding has no location");
            found = true;
        }
    }
    QVERIFY2(found, qPrintable(QString("expected %1, got: %2")
                               .arg(expectedId,
                                    findingIds(check).join(", "))));
}

/** Checking a file -- even a corrupt one -- must never modify it. */
// @spec CHK-CORE-001
void TestIntegrityCheck::neverWritesToCheckedFile()
{
    QTemporaryDir dir;
    QString path = makeWorkingCopy(dir);
    QVERIFY(!path.isEmpty());
    QVERIFY(PobCorrupt::setString(path, "_viewcolumns",
                                  Where("_vcview", "_all"),
                                  Where("_vcname", "colFloat"),
                                  "_vcname", "colGhost"));
    QByteArray before = fileHash(path);
    IntegrityCheck check(path);
    QCOMPARE(check.status(), IntegrityCheck::HasFindings);
    QCOMPARE(fileHash(path), before);
}

/** A file that is not Metakit storage at all reports Unreadable, no crash. */
// @spec CHK-CORE-002
void TestIntegrityCheck::garbageFileIsUnreadable()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString path = dir.filePath("garbage.pob");
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("this is not a database");
    file.close();

    IntegrityCheck check(path);
    QCOMPARE(check.status(), IntegrityCheck::Unreadable);
    QCOMPARE(check.formatVersion(), -1);
    QVERIFY(check.findings().isEmpty());
}

/** Valid Metakit storage missing the mandatory _global view is Unreadable. */
// @spec CHK-CORE-002
void TestIntegrityCheck::metakitFileWithoutGlobalIsUnreadable()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString path = dir.filePath("noglobal.pob");
    QVERIFY(PobCorrupt::createRawFile(path, 11,
                                      QStringList() << "_columns" << "_data"));

    IntegrityCheck check(path);
    QCOMPARE(check.status(), IntegrityCheck::Unreadable);
}

/**
 * A format version 9 file legitimately has no _calcs/_calcnodes views (they
 * arrived in version 10): their absence must not be flagged, and a minimal
 * clean v9 file must scan clean.
 */
// @spec CHK-CORE-003
void TestIntegrityCheck::checksAreVersionGated()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString path = dir.filePath("v9.pob");
    QStringList v9Views = allV11Views();
    v9Views.removeAll("_calcs");
    v9Views.removeAll("_calcnodes");
    QVERIFY(PobCorrupt::createRawFile(path, 9, v9Views));

    IntegrityCheck check(path);
    QCOMPARE(check.formatVersion(), 9);
    QVERIFY2(check.findings().isEmpty(),
             qPrintable(findingIds(check).join(", ")));
    QCOMPARE(check.status(), IntegrityCheck::Clean);
}

/**
 * An encrypted file gets outer-structure validation only: exactly _global
 * and _crypto present, one _crypto row -- and the report notes that the
 * contents were not checked.
 */
// @spec CHK-CORE-004
void TestIntegrityCheck::encryptedFileOuterStructureOnly()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString path = dir.filePath("encrypted.pob");
    QVERIFY(PobCorrupt::createRawFile(path, 11, QStringList() << "_global", 1));
    QVERIFY(PobCorrupt::addCryptoView(path));

    IntegrityCheck check(path);
    QVERIFY2(check.findings().isEmpty(),
             qPrintable(findingIds(check).join(", ")));
    QCOMPARE(check.status(), IntegrityCheck::Clean);
    QVERIFY(check.encryptedContentsSkipped());
}

/**
 * An encrypted file with an extra top-level view beyond _global/_crypto
 * (which outer-structure validation can still see) is flagged, even though
 * its contents are never decrypted or checked.
 */
// @spec CHK-CORE-004
void TestIntegrityCheck::encryptedFileWithStrayViewIsFlagged()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString path = dir.filePath("encrypted-stray.pob");
    QVERIFY(PobCorrupt::createRawFile(path, 11,
                                      QStringList() << "_global" << "_columns",
                                      1));
    QVERIFY(PobCorrupt::addCryptoView(path));

    IntegrityCheck check(path);
    QCOMPARE(check.status(), IntegrityCheck::HasFindings);
    QVERIFY(findingIds(check).contains("CHK-CORE-004"));
}

/**
 * When _cindex is not a permutation, the _all-ordering check (CHK-VIEW-002)
 * has no well-defined column order to check against: it must be skipped
 * with a note naming the failed prerequisite, not run or silently dropped.
 */
// @spec CHK-CORE-005
void TestIntegrityCheck::dependentCheckSkippedWithNote()
{
    QTemporaryDir dir;
    QString path = makeWorkingCopy(dir);
    QVERIFY(!path.isEmpty());
    QVERIFY(PobCorrupt::setInt(path, "_columns", Where("_cname", "colInteger"),
                               "_cindex", 0));

    IntegrityCheck check(path);
    QVERIFY(findingIds(check).contains("CHK-COL-003"));
    bool skippedViewOrder = false;
    const QList<IntegrityCheck::SkippedCheck> skips = check.skippedChecks();
    for (const IntegrityCheck::SkippedCheck &skip : skips) {
        if (skip.checkId == "CHK-VIEW-002") {
            QCOMPARE(skip.prerequisiteId, QString("CHK-COL-003"));
            skippedViewOrder = true;
        }
    }
    QVERIFY2(skippedViewOrder, "CHK-VIEW-002 was not skipped");
}

/**
 * A per-row violation present in many rows reports the first three rows
 * individually plus one aggregate finding with the remainder count.
 */
// @spec CHK-CORE-006
void TestIntegrityCheck::perRowFindingsAreAggregated()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString path = dir.filePath("manyrows.pob");
    QString error = PbTest::buildStandardFile(path, 6);
    QVERIFY2(error.isEmpty(), qPrintable(error));
    // Corrupt the stored enum option index in five of the six rows.
    for (int i = 0; i < 5; i++) {
        QVERIFY(PobCorrupt::setIntAt(path, "_data", i, "_I9", 1 - (i % 2)));
    }

    IntegrityCheck check(path);
    QCOMPARE(countFindings(check, "CHK-DATA-005"), 4);
}

/**
 * A file whose row IDs are an intact but offset block -- the shape left by
 * deleting a filtered group of older rows -- names the block it actually has
 * as a range, rather than only listing the expected IDs it lacks.
 */
// @spec CHK-DATA-003
void TestIntegrityCheck::offsetIdBlockIsReportedAsARange()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString path = dir.filePath("offsetids.pob");
    QString error = PbTest::buildStandardFile(path, 4);
    QVERIFY2(error.isEmpty(), qPrintable(error));
    // Renumber 0..3 as 74..77, leaving a contiguous ascending run.
    for (int i = 0; i < 4; i++) {
        QVERIFY(PobCorrupt::setIntAt(path, "_data", i, "_id", 74 + i));
    }

    IntegrityCheck check(path);
    QCOMPARE(countFindings(check, "CHK-DATA-003"), 1);
    const QString message = findingMessage(check, "CHK-DATA-003");
    QVERIFY2(message.contains("74-77"),
             qPrintable("expected a 74-77 range in: " + message));
    QVERIFY2(message.contains("present"),
             qPrintable("expected the present IDs named in: " + message));
}

/**
 * A file missing an always-required top-level view (_sorts/_sortcolumns, not
 * gated by format version) is flagged once per missing view.
 */
// @spec CHK-CORE-008
void TestIntegrityCheck::missingMandatoryViewsAreFlagged()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString path = dir.filePath("nosorts.pob");
    QStringList views = allV11Views();
    views.removeAll("_sorts");
    views.removeAll("_sortcolumns");
    QVERIFY(PobCorrupt::createRawFile(path, 11, views));

    IntegrityCheck check(path);
    QCOMPARE(check.status(), IntegrityCheck::HasFindings);
    QCOMPARE(countFindings(check, "CHK-CORE-008"), 2);
}

/**
 * The per-file report names the file, its format version, and each finding's
 * severity and check ID.
 */
// @spec CHK-CLI-002
void TestIntegrityCheck::reportListsFindings()
{
    QTemporaryDir dir;
    QString path = makeWorkingCopy(dir);
    QVERIFY(!path.isEmpty());
    QVERIFY(PobCorrupt::setString(path, "_viewcolumns",
                                  Where("_vcview", "intView"),
                                  Where("_vcname", "colString"),
                                  "_vcname", "colGhost"));

    IntegrityCheck check(path);
    QString report = check.report();
    QVERIFY(report.contains(path));
    QVERIFY(report.contains("11"));
    QVERIFY(report.contains("CHK-VIEW-005"));
    QVERIFY(report.contains("ERROR"));
}

/** A clean file's report is just the name, version, and clean summary. */
// @spec CHK-CLI-003
void TestIntegrityCheck::cleanReportIsOneSummary()
{
    IntegrityCheck check(fixturePath);
    QString report = check.report();
    QVERIFY(report.contains(fixturePath));
    QVERIFY(report.contains("11"));
    QVERIFY(!report.contains("CHK-"));
}

/**
 * The process exit code is the worst status across all files checked: 0 if
 * every file is Clean, 1 if the worst is HasFindings, 2 if any is Unreadable.
 */
// @spec CHK-CLI-004
void TestIntegrityCheck::exitCodes()
{
    typedef QList<IntegrityCheck::FileStatus> Statuses;
    QCOMPARE(IntegrityCheck::exitCode(Statuses()), 0);
    QCOMPARE(IntegrityCheck::exitCode(Statuses() << IntegrityCheck::Clean), 0);
    QCOMPARE(IntegrityCheck::exitCode(Statuses() << IntegrityCheck::Clean
                                      << IntegrityCheck::HasFindings), 1);
    QCOMPARE(IntegrityCheck::exitCode(Statuses() << IntegrityCheck::HasFindings
                                      << IntegrityCheck::Unreadable), 2);
    QCOMPARE(IntegrityCheck::exitCode(Statuses() << IntegrityCheck::Unreadable
                                      << IntegrityCheck::Clean), 2);
}

/**
 * End-to-end check of the `portabase check` subcommand: run the built
 * application binary on a clean file and on a corrupted copy, and confirm it
 * reports and exits as specified. Skipped when the application has not been
 * built (a tests-only build), since the subcommand lives in the app binary,
 * not in this test binary. The child inherits an offscreen QPA platform so it
 * needs no display.
 */
// @spec CHK-CLI-001
void TestIntegrityCheck::cliCheckCommand()
{
    // The test binary lives in tests/integritycheck/build/; the application
    // builds to build/ at the repo root. Resolve relative to the test
    // binary's own location so the lookup does not depend on the current
    // working directory.
    QString base = QCoreApplication::applicationDirPath();
    QString app;
    const QStringList candidates =
        QStringList() << base + "/../../../build/portabase"
                      << base + "/../../../build/PortaBase";
    for (const QString &candidate : candidates) {
        if (QFile::exists(candidate)) {
            app = candidate;
            break;
        }
    }
    if (app.isEmpty()) {
        QSKIP("PortaBase binary not built; run qmake portabase.pro && make");
    }
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("QT_QPA_PLATFORM", "offscreen");

    QProcess clean;
    clean.setProcessEnvironment(env);
    clean.start(app, QStringList() << "check" << fixturePath);
    QVERIFY(clean.waitForFinished(30000));
    QCOMPARE(clean.exitStatus(), QProcess::NormalExit);
    QCOMPARE(clean.exitCode(), 0);
    QVERIFY(QString::fromLocal8Bit(clean.readAllStandardOutput())
            .contains("clean"));

    QTemporaryDir dir;
    QString path = makeWorkingCopy(dir);
    QVERIFY(!path.isEmpty());
    QVERIFY(PobCorrupt::setString(path, "_viewcolumns",
                                  PobCorrupt::Where("_vcview", "_all"),
                                  PobCorrupt::Where("_vcname", "colFloat"),
                                  "_vcname", "colGhost"));
    QProcess findings;
    findings.setProcessEnvironment(env);
    findings.start(app, QStringList() << "check" << path);
    QVERIFY(findings.waitForFinished(30000));
    QCOMPARE(findings.exitCode(), 1);
    QVERIFY(QString::fromLocal8Bit(findings.readAllStandardOutput())
            .contains("CHK-VIEW-005"));
}

QTEST_MAIN(TestIntegrityCheck)
#include "integritycheck_test.moc"
