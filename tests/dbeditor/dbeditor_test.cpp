/*
 * dbeditor_test.cpp
 *
 * (c) 2026 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Coverage for the Edit Columns operation's behavior on valid files
 * (docs/intent/column-editing/column-editing-specs.md). Resilience to
 * inconsistent or corrupt file states -- a missing "_all" view, orphaned view
 * references, non-consecutive row IDs -- lives in tests/dbeditor_robustness/
 * instead, where the crash cases run subprocess-isolated.
 *
 * Each test copies a structurally valid fixture file (built once by
 * PbTest::buildStandardFile -- see tests/common/testfixtures.h) into a fresh
 * temporary file, opens it as a real second session would, and drives the
 * actual DBEditor dialog through PbTest::runEditColumnsSession.
 */

#include <QTest>
#include <QApplication>
#include <QMessageBox>
#include <QTemporaryDir>
#include <QTimer>
#include <algorithm>
#include <memory>

#include "calc/calcnode.h"
#include "columneditor.h"
#include "condition.h"
#include "database.h"
#include "datatypes.h"
#include "dbeditor.h"
#include "dbeditordriver.h"
#include "testfixtures.h"

using PbTest::completePostAcceptSequence;
using PbTest::runEditColumnsSession;
using PbTest::scheduleColumnEditor;

/**
 * A per-test working copy of the shared fixture: a temporary directory
 * holding a fresh copy of the fixture file, and a Database opened on it.
 */
class WorkingCopy
{
public:
    explicit WorkingCopy(const QString &fixturePath)
    {
        path = dir.filePath("work.pob");
        copied = dir.isValid() && QFile::copy(fixturePath, path);
        if (copied) {
            Database::OpenResult result;
            db.reset(new Database(path, &result));
            opened = (result == Database::Success);
            if (opened) {
                db->load();
            }
        }
    }

    bool valid() const { return copied && opened; }

    QTemporaryDir dir;
    QString path;
    std::unique_ptr<Database> db;
    bool copied = false;
    bool opened = false;
};

class TestDbEditor : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void addColumn_data();
    void addColumn();
    void addColumnRejectsInvalidName();

    void addEnumColumn();
    void addCalculatedColumn();
    void reorderThenDeleteColumn();
    void cancelDiscardsChanges();
    void editColumnDefault();
    void renameColumnCascades();
    void swapColumnNames();
    void deleteColumnCascades();
    void addThenDeleteNewColumn();
    void addThenRenameNewColumn();
    void deleteThenReAddSameName();
    void renameThenAddOldName();
    void secondSessionReusesColumnId();
    void deleteAllColumns();

private:
    /** Add one column of the given name/type through a full dialog session. */
    static void addColumnSession(Database *db, const QString &name, int type)
    {
        runEditColumnsSession(db, [&name, type](DBEditor *editor) {
            scheduleColumnEditor(editor, name, type);
            QMetaObject::invokeMethod(editor, "addColumn");
        });
    }

    QTemporaryDir fixtureDir;
    QString fixturePath;
};

void TestDbEditor::initTestCase()
{
    QVERIFY(fixtureDir.isValid());
    fixturePath = fixtureDir.filePath("fixture.pob");
    QString error = PbTest::buildStandardFile(fixturePath);
    QVERIFY2(error.isEmpty(), qPrintable(error));
}

void TestDbEditor::addColumn_data()
{
    QTest::addColumn<int>("type");

    QTest::newRow("STRING") << STRING;
    QTest::newRow("INTEGER") << INTEGER;
    QTest::newRow("FLOAT") << FLOAT;
    QTest::newRow("BOOLEAN") << BOOLEAN;
    QTest::newRow("NOTE") << NOTE;
    QTest::newRow("DATE") << DATE;
    QTest::newRow("TIME") << TIME;
    QTest::newRow("SEQUENCE") << SEQUENCE;
    QTest::newRow("IMAGE") << IMAGE;
}

/**
 * Add one column of each base type to an existing (previously-saved) file
 * through the real dialog flow, complete the application's post-accept
 * sequence, and confirm the column and its default values persist.
 */
// @spec COL-DB-002, COL-DB-003, COL-DB-006, COL-DB-007
void TestDbEditor::addColumn()
{
    QFETCH(int, type);
    WorkingCopy work(fixturePath);
    QVERIFY(work.valid());
    QStringList originalColumns = work.db->listColumns();

    addColumnSession(work.db.get(), "newColumn", type);
    completePostAcceptSequence(work.db.get());

    QStringList columns = work.db->listColumns();
    for (const QString &name : qAsConst(originalColumns)) {
        QVERIFY(columns.contains(name));
    }
    QVERIFY(columns.contains("newColumn"));
    QCOMPARE(work.db->getType("newColumn"), type);
    // COL-DB-003: the new column was registered in the "_all" view.
    QVERIFY(work.db->listViewColumns("_all").contains("newColumn"));
    // COL-DB-002: existing rows can be read back, with the new column's
    // value populated (getRow crashes on an unpopulated Metakit property).
    QStringList row = work.db->getRow(0);
    QCOMPARE(row.size(), columns.size());

    work.db->commit();

    // Confirm the added column actually persisted, not just in memory.
    Database::OpenResult result;
    Database reopened(work.path, &result);
    QCOMPARE(result, Database::Success);
    reopened.load();
    QVERIFY(reopened.listColumns().contains("newColumn"));
    QCOMPARE(reopened.getType("newColumn"), type);
}

/**
 * Add a column of an existing enum type through the dialog flow: the data
 * rows get the default option text and a consistent option index.
 */
// @spec COL-DB-002, COL-DB-003, COL-DB-007
void TestDbEditor::addEnumColumn()
{
    WorkingCopy work(fixturePath);
    QVERIFY(work.valid());
    int statusId = work.db->getEnumId("Status");
    QVERIFY(statusId >= FIRST_ENUM);

    addColumnSession(work.db.get(), "colPriority", statusId);
    completePostAcceptSequence(work.db.get());
    work.db->commit();

    QCOMPARE(work.db->getType("colPriority"), statusId);
    QVERIFY(work.db->listViewColumns("_all").contains("colPriority"));
    QStringList columns = work.db->listColumns();
    int position = columns.indexOf("colPriority");
    QString value = work.db->getRow(0).at(position);
    QVERIFY(work.db->listEnumOptions(statusId).contains(value));

    Database::OpenResult result;
    Database reopened(work.path, &result);
    QCOMPARE(result, Database::Success);
    reopened.load();
    QCOMPARE(reopened.getType("colPriority"), statusId);
}

/**
 * Add a calculated column through the dialog flow (the calculation set
 * programmatically, as the calc-editor subdialog would): the calculation
 * definition persists and existing rows get computed values.
 */
// @spec COL-DB-002, COL-DB-003, COL-DB-007
void TestDbEditor::addCalculatedColumn()
{
    WorkingCopy work(fixturePath);
    QVERIFY(work.valid());

    runEditColumnsSession(work.db.get(), [](DBEditor *editor) {
        QTimer::singleShot(0, editor, []() {
            ColumnEditor *columnEditor = qobject_cast<ColumnEditor *>(
                QApplication::activeModalWidget());
            if (!columnEditor) {
                qWarning("no ColumnEditor is active");
                return;
            }
            columnEditor->setName("calcTotal");
            columnEditor->setType(CALC);
            CalcNode *root = new CalcNode(CalcNode::Add, "");
            root->addChild(new CalcNode(CalcNode::Column, "colInteger"));
            root->addChild(new CalcNode(CalcNode::Constant, "5"));
            columnEditor->setCalculation(root, 2);
            columnEditor->accept();
        });
        QMetaObject::invokeMethod(editor, "addColumn");
    });
    completePostAcceptSequence(work.db.get());
    work.db->commit();

    QCOMPARE(work.db->getType("calcTotal"), CALC);
    QVERIFY(work.db->listViewColumns("_all").contains("calcTotal"));
    CalcNode *calcRoot = work.db->loadCalc("calcTotal");
    QVERIFY(calcRoot != 0);
    QVERIFY(calcRoot->equation().contains("colInteger"));
    delete calcRoot;
    // Row 0 has colInteger 40, so the computed value is 45.
    QStringList columns = work.db->listColumns();
    int position = columns.indexOf("calcTotal");
    QVERIFY(work.db->getRow(0).at(position).startsWith("45"));
}

/**
 * Entering an invalid (here, duplicate) name in the Add subdialog must be
 * rejected: the column is not added. Drives the nested modal sequence the
 * rejection produces -- the subdialog accepts, DBEditor rejects the name with
 * a warning box, the subdialog reopens -- by acting on whichever modal is
 * currently active, then cancels out. The net effect must be no new column.
 */
// @spec COL-UI-002
void TestDbEditor::addColumnRejectsInvalidName()
{
    WorkingCopy work(fixturePath);
    QVERIFY(work.valid());
    QStringList originalColumns = work.db->listColumns();

    int step = 0;
    QTimer driver;
    driver.setInterval(5);
    QObject::connect(&driver, &QTimer::timeout, [&]() {
        QWidget *modal = QApplication::activeModalWidget();
        if (!modal) {
            return;
        }
        if (step == 0) {
            if (ColumnEditor *ce = qobject_cast<ColumnEditor *>(modal)) {
                ce->setName("colString"); // duplicate of an existing column
                ce->setType(STRING);
                ce->accept();
                step = 1;
            }
        }
        else if (step == 1) {
            if (QMessageBox *box = qobject_cast<QMessageBox *>(modal)) {
                box->accept(); // acknowledge the "Duplicate name" warning
                step = 2;
            }
        }
        else if (step == 2) {
            if (ColumnEditor *ce = qobject_cast<ColumnEditor *>(modal)) {
                ce->reject(); // give up rather than enter a valid name
                driver.stop();
            }
        }
    });
    driver.start();

    runEditColumnsSession(work.db.get(), [](DBEditor *editor) {
        QMetaObject::invokeMethod(editor, "addColumn");
    });

    QVERIFY2(step == 2, "expected modal sequence did not complete");
    QCOMPARE(work.db->listColumns(), originalColumns);
}

/**
 * Reorder a column across another one, then delete a column that sits
 * between the reordered column's old and new staged position, and confirm
 * the persisted column index sequence comes out as a clean permutation of
 * 0..columnCount-1, not a duplicate and a gap.
 */
// @spec COL-UI-005, COL-UI-010
void TestDbEditor::reorderThenDeleteColumn()
{
    WorkingCopy work(fixturePath);
    QVERIFY(work.valid());
    int originalCount = work.db->listColumns().size();

    runEditColumnsSession(work.db.get(), [](DBEditor *editor) {
        // Move colInteger down twice: past colFloat, then past colBoolean.
        QMetaObject::invokeMethod(editor, "selectRow",
                                  Q_ARG(QString, QString("colInteger")));
        QMetaObject::invokeMethod(editor, "moveDown");
        QMetaObject::invokeMethod(editor, "selectRow",
                                  Q_ARG(QString, QString("colInteger")));
        QMetaObject::invokeMethod(editor, "moveDown");

        // Delete colFloat, which the first move above left sitting at
        // colInteger's original physical position in the staging view.
        QMetaObject::invokeMethod(editor, "selectRow",
                                  Q_ARG(QString, QString("colFloat")));
        QMetaObject::invokeMethod(editor, "deleteColumn");
    });
    work.db->commit();

    QStringList columns = work.db->listColumns();
    QCOMPARE(columns.size(), originalCount - 1);

    QList<int> indices;
    for (const QString &name : qAsConst(columns)) {
        indices.append(work.db->getIndex(name));
    }
    std::sort(indices.begin(), indices.end());
    QList<int> expected;
    for (int i = 0; i < columns.size(); i++) {
        expected.append(i);
    }
    QCOMPARE(indices, expected);
}

/**
 * Stage an addition and a deletion, then cancel the dialog: the database
 * must come through unmodified.
 */
// @spec COL-UI-001, COL-UI-009
void TestDbEditor::cancelDiscardsChanges()
{
    WorkingCopy work(fixturePath);
    QVERIFY(work.valid());
    QStringList originalColumns = work.db->listColumns();

    bool accepted = runEditColumnsSession(work.db.get(),
                                          [](DBEditor *editor) {
        scheduleColumnEditor(editor, "doomedColumn", STRING);
        QMetaObject::invokeMethod(editor, "addColumn");
        QMetaObject::invokeMethod(editor, "selectRow",
                                  Q_ARG(QString, QString("colFloat")));
        QMetaObject::invokeMethod(editor, "deleteColumn");
    }, false);
    QVERIFY(!accepted);

    QCOMPARE(work.db->listColumns(), originalColumns);
    QVERIFY(work.db->listViewColumns("_all").contains("colFloat"));
}

/**
 * Edit an existing column's default value (the one property besides the name
 * that the Edit subdialog allows changing) and confirm it lands, without
 * touching existing row data.
 */
// @spec COL-UI-003
void TestDbEditor::editColumnDefault()
{
    WorkingCopy work(fixturePath);
    QVERIFY(work.valid());
    QString originalValue = work.db->getRow(0).at(0); // colString, position 0

    runEditColumnsSession(work.db.get(), [](DBEditor *editor) {
        QMetaObject::invokeMethod(editor, "selectRow",
                                  Q_ARG(QString, QString("colString")));
        scheduleColumnEditor(editor, QString(), -1, "changed default");
        QMetaObject::invokeMethod(editor, "editColumn");
    });

    QCOMPARE(work.db->getDefault("colString"), QString("changed default"));
    QCOMPARE(work.db->getRow(0).at(0), originalValue);
}

/**
 * Rename a column that a user view, a sorting, a filter condition, and a
 * calculated column's formula all reference: every reference must follow the
 * rename.
 */
// @spec COL-UI-006, COL-DB-005
void TestDbEditor::renameColumnCascades()
{
    WorkingCopy work(fixturePath);
    QVERIFY(work.valid());

    runEditColumnsSession(work.db.get(), [](DBEditor *editor) {
        QMetaObject::invokeMethod(editor, "selectRow",
                                  Q_ARG(QString, QString("colInteger")));
        scheduleColumnEditor(editor, "colRenamed");
        QMetaObject::invokeMethod(editor, "editColumn");
    });

    QStringList columns = work.db->listColumns();
    QVERIFY(columns.contains("colRenamed"));
    QVERIFY(!columns.contains("colInteger"));

    QVERIFY(work.db->listViewColumns("intView").contains("colRenamed"));
    QVERIFY(work.db->listViewColumns("_all").contains("colRenamed"));

    QStringList sortCols, descCols;
    QVERIFY(work.db->getSortingInfo("intSort", &sortCols, &descCols));
    QVERIFY(sortCols.contains("colRenamed"));

    Condition *condition = work.db->getCondition("intFilter", 0);
    QCOMPARE(condition->getColName(), QString("colRenamed"));
    delete condition;

    CalcNode *calcRoot = work.db->loadCalc("calcCol");
    QVERIFY(calcRoot != 0);
    QVERIFY(calcRoot->equation().contains("colRenamed"));
    delete calcRoot;
}

/**
 * Swap two columns' names within one dialog session (via a temporary third
 * name, since staged names must stay unique): both columns, and every
 * reference to them, must end at their staged final names. A naive sequential
 * rename replay would transiently duplicate a name and mis-target the second
 * rename; the two-pass replay through temporary names handles it.
 */
// @spec COL-UI-014
void TestDbEditor::swapColumnNames()
{
    WorkingCopy work(fixturePath);
    QVERIFY(work.valid());

    auto rename = [](DBEditor *editor, const QString &from,
                     const QString &to) {
        QMetaObject::invokeMethod(editor, "selectRow", Q_ARG(QString, from));
        scheduleColumnEditor(editor, to);
        QMetaObject::invokeMethod(editor, "editColumn");
    };
    runEditColumnsSession(work.db.get(), [&rename](DBEditor *editor) {
        rename(editor, "colString", "colTemp");
        rename(editor, "colNote", "colString");
        rename(editor, "colTemp", "colNote");
    });

    // The column originally named colString (type STRING) is now colNote,
    // and the original colNote (type NOTE) is now colString.
    QCOMPARE(work.db->getType("colNote"), STRING);
    QCOMPARE(work.db->getType("colString"), NOTE);
    // References followed their columns: intView contained the original
    // colString column, whose final name is colNote.
    QStringList viewCols = work.db->listViewColumns("intView");
    QVERIFY(viewCols.contains("colNote"));
    QVERIFY(!viewCols.contains("colString"));
    // The _all view still lists every column exactly once.
    QStringList allCols = work.db->listViewColumns("_all");
    allCols.sort();
    QStringList expected = work.db->listColumns();
    expected.sort();
    QCOMPARE(allCols, expected);
}

/**
 * Delete a column that a user view, a sorting, a filter condition, and a
 * calculated column's formula all reference: every reference must be
 * removed, and the calculation invalidated.
 */
// @spec COL-UI-004, COL-UI-007, COL-DB-004
void TestDbEditor::deleteColumnCascades()
{
    WorkingCopy work(fixturePath);
    QVERIFY(work.valid());

    runEditColumnsSession(work.db.get(), [](DBEditor *editor) {
        QMetaObject::invokeMethod(editor, "selectRow",
                                  Q_ARG(QString, QString("colInteger")));
        QMetaObject::invokeMethod(editor, "deleteColumn");
    });

    QVERIFY(!work.db->listColumns().contains("colInteger"));
    QVERIFY(!work.db->listViewColumns("intView").contains("colInteger"));
    QVERIFY(!work.db->listViewColumns("_all").contains("colInteger"));

    QStringList sortCols, descCols;
    work.db->getSortingInfo("intSort", &sortCols, &descCols);
    QVERIFY(!sortCols.contains("colInteger"));

    QCOMPARE(work.db->getConditionCount("intFilter"), 0);

    // The reference is invalidated: nodes referencing the deleted column are
    // pruned from the formula (the whole formula is cleared only when its
    // root node is the reference).
    CalcNode *calcRoot = work.db->loadCalc("calcCol");
    QVERIFY(calcRoot == 0 || !calcRoot->equation().contains("colInteger"));
    delete calcRoot;
}

/**
 * A column added and then deleted within the same dialog session must leave
 * no trace in the database.
 */
// @spec COL-UI-011
void TestDbEditor::addThenDeleteNewColumn()
{
    WorkingCopy work(fixturePath);
    QVERIFY(work.valid());
    QStringList originalColumns = work.db->listColumns();

    runEditColumnsSession(work.db.get(), [](DBEditor *editor) {
        scheduleColumnEditor(editor, "ephemeral", STRING);
        QMetaObject::invokeMethod(editor, "addColumn");
        QMetaObject::invokeMethod(editor, "selectRow",
                                  Q_ARG(QString, QString("ephemeral")));
        QMetaObject::invokeMethod(editor, "deleteColumn");
    });

    QCOMPARE(work.db->listColumns(), originalColumns);
    QVERIFY(!work.db->listViewColumns("_all").contains("ephemeral"));
}

/**
 * A column added and then renamed within the same dialog session must be
 * created under its final staged name only.
 */
// @spec COL-UI-012
void TestDbEditor::addThenRenameNewColumn()
{
    WorkingCopy work(fixturePath);
    QVERIFY(work.valid());

    runEditColumnsSession(work.db.get(), [](DBEditor *editor) {
        scheduleColumnEditor(editor, "draftName", STRING);
        QMetaObject::invokeMethod(editor, "addColumn");
        QMetaObject::invokeMethod(editor, "selectRow",
                                  Q_ARG(QString, QString("draftName")));
        scheduleColumnEditor(editor, "finalName");
        QMetaObject::invokeMethod(editor, "editColumn");
    });

    QStringList columns = work.db->listColumns();
    QVERIFY(columns.contains("finalName"));
    QVERIFY(!columns.contains("draftName"));
}

/**
 * Delete a column and add a new one with the same name in one session: the
 * new column starts from its default value in every existing row (no
 * carry-over of the deleted column's data), and reuses the freed column ID.
 */
// @spec COL-UI-013, COL-DB-001
void TestDbEditor::deleteThenReAddSameName()
{
    WorkingCopy work(fixturePath);
    QVERIFY(work.valid());
    // colString is position 0 with column ID 0 in the standard fixture.
    QCOMPARE(work.db->getColId("colString"), QString("_S0"));

    runEditColumnsSession(work.db.get(), [](DBEditor *editor) {
        QMetaObject::invokeMethod(editor, "selectRow",
                                  Q_ARG(QString, QString("colString")));
        QMetaObject::invokeMethod(editor, "deleteColumn");
        scheduleColumnEditor(editor, "colString", INTEGER, "7");
        QMetaObject::invokeMethod(editor, "addColumn");
    });
    work.db->commit();

    QCOMPARE(work.db->getType("colString"), INTEGER);
    // COL-DB-001: the freed ID 0 was the lowest gap, so it gets reused.
    QCOMPARE(work.db->getColId("colString"), QString("_I0"));
    // COL-UI-013: every row holds the new column's default, not old data.
    QStringList columns = work.db->listColumns();
    int position = columns.indexOf("colString");
    QCOMPARE(work.db->getRow(0).at(position), QString("7"));
}

/**
 * Rename an existing column and add a new column under the now-free old name
 * in the same session; the apply order (renames before additions) must leave
 * both columns intact.
 */
// @spec COL-UI-008
void TestDbEditor::renameThenAddOldName()
{
    WorkingCopy work(fixturePath);
    QVERIFY(work.valid());

    runEditColumnsSession(work.db.get(), [](DBEditor *editor) {
        QMetaObject::invokeMethod(editor, "selectRow",
                                  Q_ARG(QString, QString("colString")));
        scheduleColumnEditor(editor, "colText");
        QMetaObject::invokeMethod(editor, "editColumn");
        scheduleColumnEditor(editor, "colString", BOOLEAN);
        QMetaObject::invokeMethod(editor, "addColumn");
    });

    QStringList columns = work.db->listColumns();
    QVERIFY(columns.contains("colText"));
    QVERIFY(columns.contains("colString"));
    QCOMPARE(work.db->getType("colText"), STRING);
    QCOMPARE(work.db->getType("colString"), BOOLEAN);
}

/**
 * Delete a column in one session, then add a different column in a second
 * session (fresh dialog, same file): the new column reuses the lowest freed
 * column ID.
 */
// @spec COL-DB-001
void TestDbEditor::secondSessionReusesColumnId()
{
    WorkingCopy work(fixturePath);
    QVERIFY(work.valid());
    // colFloat is position 2 with column ID 2 in the standard fixture.
    QCOMPARE(work.db->getColId("colFloat", STRING), QString("_S2"));

    runEditColumnsSession(work.db.get(), [](DBEditor *editor) {
        QMetaObject::invokeMethod(editor, "selectRow",
                                  Q_ARG(QString, QString("colFloat")));
        QMetaObject::invokeMethod(editor, "deleteColumn");
    });
    work.db->commit();

    addColumnSession(work.db.get(), "colSecondWave", INTEGER);
    work.db->commit();

    QCOMPARE(work.db->getColId("colSecondWave"), QString("_I2"));
}

/**
 * Deleting every column returns the file to the valid zero-column state (the
 * same state as a newly created file), from which columns can be added again.
 */
// @spec COL-UI-015
void TestDbEditor::deleteAllColumns()
{
    WorkingCopy work(fixturePath);
    QVERIFY(work.valid());
    QStringList allColumns = work.db->listColumns();

    runEditColumnsSession(work.db.get(), [&allColumns](DBEditor *editor) {
        for (const QString &name : qAsConst(allColumns)) {
            QMetaObject::invokeMethod(editor, "selectRow",
                                      Q_ARG(QString, name));
            QMetaObject::invokeMethod(editor, "deleteColumn");
        }
    });
    completePostAcceptSequence(work.db.get());
    work.db->commit();

    QCOMPARE(work.db->listColumns(), QStringList());
    QCOMPARE(work.db->listViewColumns("_all"), QStringList());

    // The state is recoverable: a fresh session can define columns again.
    Database::OpenResult result;
    Database reopened(work.path, &result);
    QCOMPARE(result, Database::Success);
    reopened.load();
    QCOMPARE(reopened.listColumns(), QStringList());
    addColumnSession(&reopened, "rebornColumn", STRING);
    QVERIFY(reopened.listColumns().contains("rebornColumn"));
}

QTEST_MAIN(TestDbEditor)
#include "dbeditor_test.moc"
