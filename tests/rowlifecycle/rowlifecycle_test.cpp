/*
 * rowlifecycle_test.cpp
 *
 * (c) 2026 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Coverage for the row lifecycle's Database layer: adding rows, deleting
 * them, and the _id assignment both depend on
 * (docs/intent/row-lifecycle/row-lifecycle-specs.md, ROW-DB). Each test
 * builds its own structurally valid file (PbTest::buildStandardFile), with
 * the PobCorrupt helpers renumbering its rows when a test needs IDs that
 * have gaps.
 */

#include <QTest>
#include <QApplication>
#include <QProcess>
#include <QTemporaryDir>
#include <algorithm>
#include <memory>

#include "condition.h"
#include "database.h"
#include "datatypes.h"
#include "filter.h"
#include "pobcorruptor.h"
#include "testfixtures.h"
#include "view.h"

/**
 * A freshly built fixture file of a given size, opened as the application
 * would open it. When firstId is non-zero the rows are renumbered to
 * consecutive IDs starting there, leaving IDs that are unique but have a gap
 * below them (the shape of the real-world file behind COL-DB-010).
 */
class RowFile
{
public:
    explicit RowFile(int rows, int firstId = 0)
    {
        path = dir.filePath("rows.pob");
        error = dir.isValid() ? PbTest::buildStandardFile(path, rows)
                              : QString("no temporary directory");
        for (int i = 0; error.isEmpty() && firstId != 0 && i < rows; i++) {
            if (!PobCorrupt::setIntAt(path, "_data", i, "_id", firstId + i)) {
                error = QString("failed to renumber row %1").arg(i);
            }
        }
        if (error.isEmpty()) {
            Database::OpenResult result;
            db.reset(new Database(path, &result));
            if (result == Database::Success) {
                db->load();
            }
            else {
                error = QString("failed to open %1").arg(path);
            }
        }
    }

    QTemporaryDir dir;
    QString path;
    QString error;
    std::unique_ptr<Database> db;
};

class TestRowLifecycle : public QObject
{
    Q_OBJECT

private slots:
    void addRowAssignsNextId();
    void addRowAfterGapUsesLargestId();
    void addRowRejectsWrongValueCount_data();
    void addRowRejectsWrongValueCount();
    void addRowRejectsInvalidValue();
    void addRowCsvBlankNumberUsesDefault();
    void rejectedAddRowLeavesNoTrace_data();
    void rejectedAddRowLeavesNoTrace();
    void addRowAllocatesSequenceValue();
    void addRowWritesTypeProperties();
    void addRowAddsNewEnumOption();
    void deleteRowRenumbersHigherIds();
    void deleteRowIgnoresAbsentId();
    void compressRowIdsAfterBatchDelete_data();
    void compressRowIdsAfterBatchDelete();
    void deleteAllRowsMatchingNothingChangesNothing();

private:
    /** Valid values for a new row of the standard fixture, in column order. */
    static QStringList newRowValues(const QString &text)
    {
        QStringList values;
        values << text;          // colString
        values << "42";          // colInteger
        values << "2.5";         // colFloat
        values << "1";           // colBoolean
        values << "a note";      // colNote
        values << "20240115";    // colDate
        values << "3600";        // colTime
        values << "999";         // colSequence (ignored; allocated by addRow)
        values << "";            // colImage
        values << "Closed";      // colStatus
        values << "84";          // calcCol
        return values;
    }

    /** Every stored _id, in physical row order. */
    static QList<int> storedIds(Database *db)
    {
        QList<int> ids;
        c4_View data = db->getData();
        c4_IntProp idProp("_id");
        int count = data.GetSize();
        for (int i = 0; i < count; i++) {
            ids.append(idProp (data[i]));
        }
        return ids;
    }

    /** Every stored _id, in ascending order. */
    static QList<int> sortedIds(Database *db)
    {
        QList<int> ids = storedIds(db);
        std::sort(ids.begin(), ids.end());
        return ids;
    }

    /** The consecutive IDs first through first + count - 1. */
    static QList<int> idRange(int first, int count)
    {
        QList<int> ids;
        for (int i = 0; i < count; i++) {
            ids.append(first + i);
        }
        return ids;
    }

    /** The _id of the row whose colString holds text, or -1 if none does. */
    static int idOfRow(Database *db, const QString &text)
    {
        c4_View data = db->getData();
        c4_StringProp textProp(db->getColId("colString").toLatin1());
        int index = data.Find(textProp [text.toUtf8()]);
        if (index == -1) {
            return -1;
        }
        c4_IntProp idProp("_id");
        return idProp (data[index]);
    }

    /** The physical index of the row with the given _id. */
    static int indexOfId(Database *db, int id)
    {
        c4_IntProp idProp("_id");
        return db->getData().Find(idProp [id]);
    }
};

/**
 * Add a row to a file whose IDs are 0 and 1: addRow must report success
 * with an empty message, assign ID 2, and return it through the row-ID
 * out-parameter.
 */
// @spec ROW-DB-001, ROW-DB-003
void TestRowLifecycle::addRowAssignsNextId()
{
    RowFile file(2);
    QVERIFY2(file.error.isEmpty(), qPrintable(file.error));
    int id = -42;
    QString error = file.db->addRow(newRowValues("added"), &id);
    QCOMPARE(error, QString());
    QCOMPARE(id, 2);
    QCOMPARE(storedIds(file.db.get()), idRange(0, 3));
}

/**
 * Add a row to a file whose three rows hold IDs 74-76: the new row must get
 * ID 77, one past the largest stored ID, not 3 (the row count), which would
 * leave the file without a duplicate only by coincidence of the gap's size.
 */
// @spec ROW-DB-002
void TestRowLifecycle::addRowAfterGapUsesLargestId()
{
    RowFile file(3, 74);
    QVERIFY2(file.error.isEmpty(), qPrintable(file.error));
    int id = -42;
    QString error = file.db->addRow(newRowValues("added"), &id);
    QCOMPARE(error, QString());
    QCOMPARE(id, 77);
    QCOMPARE(storedIds(file.db.get()), idRange(74, 4));
}

void TestRowLifecycle::addRowRejectsWrongValueCount_data()
{
    QTest::addColumn<int>("extra");
    QTest::addColumn<QString>("expectedHeading");
    QTest::addColumn<QStringList>("expectedNames");

    QTest::newRow("excess") << 1 << "Excess columns"
                            << (QStringList() << "surplus");
    QTest::newRow("missing") << -2 << "Missing columns"
                             << (QStringList() << "colStatus" << "calcCol");
}

/**
 * Pass addRow one value too many, or two too few: it must reject the row
 * with a message listing the surplus value or the missing columns, and
 * leave the data untouched.
 */
// @spec ROW-DB-004
void TestRowLifecycle::addRowRejectsWrongValueCount()
{
    QFETCH(int, extra);
    QFETCH(QString, expectedHeading);
    QFETCH(QStringList, expectedNames);

    RowFile file(2);
    QVERIFY2(file.error.isEmpty(), qPrintable(file.error));
    QStringList values = newRowValues("added");
    if (extra > 0) {
        values << "surplus";
    }
    else {
        values = values.mid(0, values.count() + extra);
    }
    QString error = file.db->addRow(values);
    QVERIFY(error.startsWith(expectedHeading));
    for (int i = 0; i < expectedNames.count(); i++) {
        QVERIFY2(error.contains(expectedNames[i]), qPrintable(error));
    }
    QCOMPARE(storedIds(file.db.get()), idRange(0, 2));
}

/**
 * Pass addRow a non-numeric integer value: it must reject the row with a
 * message naming the column, and leave the data untouched.
 */
// @spec ROW-DB-005
void TestRowLifecycle::addRowRejectsInvalidValue()
{
    RowFile file(2);
    QVERIFY2(file.error.isEmpty(), qPrintable(file.error));
    QStringList values = newRowValues("added");
    values[1] = "abc";
    QString error = file.db->addRow(values);
    QVERIFY2(error.startsWith("colInteger"), qPrintable(error));
    QCOMPARE(storedIds(file.db.get()), idRange(0, 2));
}

/**
 * Pass addRow a blank integer value with CSV blank-number substitution
 * requested: instead of rejecting the row, it must store the column's
 * default in its place.
 */
// @spec ROW-DB-005
void TestRowLifecycle::addRowCsvBlankNumberUsesDefault()
{
    RowFile file(2);
    QVERIFY2(file.error.isEmpty(), qPrintable(file.error));
    file.db->setDefault("colInteger", "7");
    QStringList values = newRowValues("added");
    values[1] = "";
    int id = -42;
    QString error = file.db->addRow(values, &id, false, true);
    QCOMPARE(error, QString());
    c4_IntProp intProp(file.db->getColId("colInteger").toLatin1());
    QCOMPARE((int)intProp (file.db->getData()[indexOfId(file.db.get(), id)]),
             7);
}

void TestRowLifecycle::rejectedAddRowLeavesNoTrace_data()
{
    QTest::addColumn<QString>("image");
    QTest::addColumn<QString>("lastValue");
    QTest::addColumn<QString>("errorPrefix");

    QTest::newRow("invalid last value") << "" << "abc" << "colLate";
    QTest::newRow("missing image file") << "missing-image.png" << "7"
                                        << "Missing file";
}

/**
 * Add an integer column after the Sequence and enum columns, then pass
 * addRow a row that introduces a new enum option (with new options allowed)
 * but is rejected after the Sequence column: for an invalid value in that
 * last column, and for an image file that does not exist. The rejection
 * must leave no row added, the Sequence column's next value unadvanced, the
 * enum's options unchanged, and the row-ID out-parameter unwritten.
 */
// @spec ROW-DB-006
void TestRowLifecycle::rejectedAddRowLeavesNoTrace()
{
    QFETCH(QString, image);
    QFETCH(QString, lastValue);
    QFETCH(QString, errorPrefix);

    RowFile file(2);
    QVERIFY2(file.error.isEmpty(), qPrintable(file.error));
    Database *db = file.db.get();
    db->addColumn(11, "colLate", INTEGER, "0");
    db->updateDataFormat();
    QString nextSequence = db->getDefault("colSequence");
    int statusId = db->getEnumId("Status");
    QStringList options = db->listEnumOptions(statusId);

    QStringList values = newRowValues("added");
    values[8] = image;
    values[9] = "Pending";
    values << lastValue;
    int id = -42;
    QString error = db->addRow(values, &id, false, false, true);
    QVERIFY2(error.startsWith(errorPrefix), qPrintable(error));
    QCOMPARE(id, -42);
    QCOMPARE(storedIds(db), idRange(0, 2));
    QCOMPARE(db->getDefault("colSequence"), nextSequence);
    QCOMPARE(db->listEnumOptions(statusId), options);
}

/**
 * Delete a row, then add one with a Sequence value supplied in the input:
 * addRow must ignore the supplied value, store the column's stored next
 * value, and advance that by one, even though the new row's _id was
 * reused by the deletion's renumbering.
 */
// @spec ROW-DB-007
void TestRowLifecycle::addRowAllocatesSequenceValue()
{
    RowFile file(2);
    QVERIFY2(file.error.isEmpty(), qPrintable(file.error));
    Database *db = file.db.get();
    int nextSequence = db->getDefault("colSequence").toInt();
    db->deleteRow(1);
    int id = -42;
    QString error = db->addRow(newRowValues("added"), &id);
    QCOMPARE(error, QString());
    QCOMPARE(id, 1);
    c4_IntProp sequenceProp(db->getColId("colSequence").toLatin1());
    QCOMPARE((int)sequenceProp (db->getData()[indexOfId(db, id)]),
             nextSequence);
    QCOMPARE(db->getDefault("colSequence"), QString::number(nextSequence + 1));
}

/**
 * Add a row holding a decimal, a calculation, an enum option, and an image
 * format: each must be stored under every property the format defines for
 * its type -- float plus string for decimal and calculation values, text
 * plus option index for the enum, and the format string (with no bytes yet)
 * for the image.
 */
// @spec ROW-DB-008
void TestRowLifecycle::addRowWritesTypeProperties()
{
    RowFile file(2);
    QVERIFY2(file.error.isEmpty(), qPrintable(file.error));
    Database *db = file.db.get();
    QStringList values = newRowValues("added");
    values[8] = "PNG";
    int id = -42;
    QString error = db->addRow(values, &id);
    QCOMPARE(error, QString());
    c4_RowRef row = db->getData()[indexOfId(db, id)];

    c4_FloatProp floatProp(db->getColId("colFloat").toLatin1());
    c4_StringProp floatText(db->getColId("colFloat", STRING).toLatin1());
    QCOMPARE((double)floatProp (row), 2.5);
    QCOMPARE(QString::fromUtf8(floatText (row)), QString("2.5"));

    c4_FloatProp calcProp(db->getColId("calcCol").toLatin1());
    c4_StringProp calcText(db->getColId("calcCol", STRING).toLatin1());
    QCOMPARE((double)calcProp (row), 84.0);
    QCOMPARE(QString::fromUtf8(calcText (row)), QString("84"));

    c4_StringProp enumText(db->getColId("colStatus").toLatin1());
    c4_IntProp enumIndex(db->getColId("colStatus", INTEGER).toLatin1());
    QCOMPARE(QString::fromUtf8(enumText (row)), QString("Closed"));
    QCOMPARE((int)enumIndex (row), 1);

    c4_StringProp imageFormat(db->getColId("colImage", STRING).toLatin1());
    QCOMPARE(QString::fromUtf8(imageFormat (row)), QString("PNG"));
    QVERIFY(db->getBinaryField(id, "colImage").isEmpty());
}

/**
 * Add a row whose enum value is not yet one of the enum's options, with new
 * options allowed: addRow must append the value to the enum's options and
 * store the new option's position as the row's option index.
 */
// @spec ROW-DB-008
void TestRowLifecycle::addRowAddsNewEnumOption()
{
    RowFile file(2);
    QVERIFY2(file.error.isEmpty(), qPrintable(file.error));
    Database *db = file.db.get();
    int statusId = db->getEnumId("Status");
    QStringList options = db->listEnumOptions(statusId);
    QStringList values = newRowValues("added");
    values[9] = "Pending";
    int id = -42;
    QString error = db->addRow(values, &id, false, false, true);
    QCOMPARE(error, QString());

    QCOMPARE(db->listEnumOptions(statusId), options << "Pending");
    c4_RowRef row = db->getData()[indexOfId(db, id)];
    c4_StringProp enumText(db->getColId("colStatus").toLatin1());
    c4_IntProp enumIndex(db->getColId("colStatus", INTEGER).toLatin1());
    QCOMPARE(QString::fromUtf8(enumText (row)), QString("Pending"));
    QCOMPARE((int)enumIndex (row), 2);
}

/**
 * Delete ID 1 from a five-row file: the rows above it must each move down
 * one ID, the rows below keep theirs, and the next row added get ID 4.
 */
// @spec ROW-DB-009
void TestRowLifecycle::deleteRowRenumbersHigherIds()
{
    RowFile file(5);
    QVERIFY2(file.error.isEmpty(), qPrintable(file.error));
    Database *db = file.db.get();
    db->deleteRow(1);
    QCOMPARE(sortedIds(db), idRange(0, 4));
    QCOMPARE(idOfRow(db, "text0"), 0);
    QCOMPARE(idOfRow(db, "text1"), -1);
    QCOMPARE(idOfRow(db, "text2"), 1);
    QCOMPARE(idOfRow(db, "text4"), 3);
    int id = -42;
    QCOMPARE(db->addRow(newRowValues("added"), &id), QString());
    QCOMPARE(id, 4);
}

/**
 * Ask deleteRow to delete ID 7 from a three-row file, then add a row and
 * save: no row may be removed, no ID may change, and the added row must get
 * ID 3. The deletion runs in a child process (runScenario) because the
 * unguarded lookup crashes, which would otherwise abort the whole run.
 */
// @spec ROW-DB-010
void TestRowLifecycle::deleteRowIgnoresAbsentId()
{
    RowFile file(3);
    QVERIFY2(file.error.isEmpty(), qPrintable(file.error));
    file.db.reset();
    QProcess child;
    child.start(QCoreApplication::applicationFilePath(),
                QStringList() << "--scenario" << "deleteAbsentId" << file.path);
    QVERIFY2(child.waitForFinished(30000), "child process did not finish");
    QVERIFY2(child.exitStatus() == QProcess::NormalExit,
             "child process crashed -- deleteRow acted on a missed lookup");
    QCOMPARE(child.exitCode(), 0);

    Database::OpenResult result;
    Database db(file.path, &result);
    QCOMPARE(result, Database::Success);
    db.load();
    QCOMPARE(storedIds(&db), idRange(0, 4));
    QCOMPARE(idOfRow(&db, "text0"), 0);
    QCOMPARE(idOfRow(&db, "text1"), 1);
    QCOMPARE(idOfRow(&db, "text2"), 2);
    QCOMPARE(idOfRow(&db, "added"), 3);
}

void TestRowLifecycle::compressRowIdsAfterBatchDelete_data()
{
    QTest::addColumn<int>("rows");
    QTest::addColumn<int>("firstId");
    QTest::addColumn<QList<int>>("deleteIds");

    QTest::newRow("consecutive") << 10 << 0
                                 << (QList<int>() << 9 << 5 << 4 << 1);
    QTest::newRow("gap below") << 10 << 74
                               << (QList<int>() << 83 << 80 << 75);
    QList<int> scattered;
    for (int i = 98; i >= 0; i--) {
        if (i % 3 == 0 || (i >= 40 && i < 60)) {
            scattered.append(i);
        }
    }
    QTest::newRow("99 rows, scattered") << 99 << 0 << scattered;
}

/**
 * Delete a batch of rows with per-row renumbering suppressed, then compress
 * once, from files with consecutive IDs and with a gap below them: the
 * survivors must end up numbered 0 through n-1 in their original relative
 * order, and the next row added must get ID n.
 */
// @spec ROW-DB-011
void TestRowLifecycle::compressRowIdsAfterBatchDelete()
{
    QFETCH(int, rows);
    QFETCH(int, firstId);
    QFETCH(QList<int>, deleteIds);

    RowFile file(rows, firstId);
    QVERIFY2(file.error.isEmpty(), qPrintable(file.error));
    Database *db = file.db.get();
    QStringList survivors;
    for (int i = 0; i < rows; i++) {
        if (!deleteIds.contains(firstId + i)) {
            survivors.append(QString("text%1").arg(i));
        }
    }
    for (int i = 0; i < deleteIds.count(); i++) {
        db->deleteRow(deleteIds[i], false);
    }
    db->compressRowIds();

    int count = survivors.count();
    QCOMPARE(sortedIds(db), idRange(0, count));
    for (int i = 0; i < count; i++) {
        QCOMPARE(idOfRow(db, survivors[i]), i);
    }
    int id = -42;
    QCOMPARE(db->addRow(newRowValues("added"), &id), QString());
    QCOMPARE(id, count);
}

/**
 * Apply a filter that matches no rows to a file whose IDs are 74 through
 * 78, then delete all rows in the filter: every row must remain, in its
 * original order and with its _id unchanged.
 */
// @spec ROW-DB-012
void TestRowLifecycle::deleteAllRowsMatchingNothingChangesNothing()
{
    RowFile file(5, 74);
    QVERIFY2(file.error.isEmpty(), qPrintable(file.error));
    Database *db = file.db.get();
    Filter filter(db, "noneFilter");
    Condition *condition = new Condition(db);
    condition->setColName("colInteger");
    condition->setOperator(Condition::LessThan);
    condition->setConstant("0");
    filter.addCondition(condition);
    db->addFilter(&filter);
    View *view = db->getView("_all");
    view->prepareData();
    QCOMPARE(view->totalRowCount(), 0);

    view->deleteAllRows();
    QCOMPARE(storedIds(db), idRange(74, 5));
}

/**
 * Child-process body for the tests whose failure mode is a crash. Opens the
 * file, performs the named scenario, and saves; the parent test inspects
 * the saved file.
 *
 * @param scenario The scenario name passed after --scenario
 * @param path The PortaBase file to operate on
 * @return 0 on success, non-zero if the scenario could not run
 */
static int runScenario(const QString &scenario, const QString &path)
{
    Database::OpenResult result;
    Database db(path, &result);
    if (result != Database::Success) {
        return 2;
    }
    db.load();
    if (scenario == "deleteAbsentId") {
        db.deleteRow(7);
        QStringList values;
        values << "added" << "42" << "2.5" << "1" << "a note" << "20240115"
               << "3600" << "999" << "" << "Closed" << "84";
        if (!db.addRow(values).isEmpty()) {
            return 3;
        }
        db.commit();
        return 0;
    }
    return 4;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QStringList args = app.arguments();
    int scenarioIndex = args.indexOf("--scenario");
    if (scenarioIndex != -1 && scenarioIndex + 2 < args.size()) {
        return runScenario(args[scenarioIndex + 1], args[scenarioIndex + 2]);
    }
    TestRowLifecycle testCase;
    QTEST_SET_MAIN_SOURCE_PATH
    return QTest::qExec(&testCase, argc, argv);
}

#include "rowlifecycle_test.moc"
