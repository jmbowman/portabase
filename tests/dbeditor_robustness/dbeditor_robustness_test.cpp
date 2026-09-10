/*
 * dbeditor_robustness_test.cpp
 *
 * (c) 2026 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Resilience tests for the Edit Columns operation against inconsistent or
 * corrupt file states -- a missing "_all" view, view entries referencing
 * columns that no longer exist, and non-consecutive row IDs. Each seeds a
 * single invariant violation into an otherwise valid fixture (via the
 * raw-Metakit corruptor in tests/common/) and asserts the operation recovers
 * or survives. See docs/intent/column-editing/column-editing-specs.md
 * (COL-DB-007/008/009/010).
 *
 * Kept separate from tests/dbeditor/ (valid-file behavior) for one reason:
 * the cases that would crash on a regression run in a child process. The test
 * binary re-executes itself with --scenario <name> <file>, and the parent
 * asserts the child exits normally, so a reintroduced crash surfaces as a
 * clean test failure rather than a segfault that aborts the whole run.
 */

#include <QTest>
#include <QApplication>
#include <QProcess>
#include <QTemporaryDir>

#include "database.h"
#include "datatypes.h"
#include "dbeditor.h"
#include "dbeditordriver.h"
#include "pobcorruptor.h"
#include "testfixtures.h"
#include "view.h"

using PbTest::completePostAcceptSequence;
using PbTest::runEditColumnsSession;
using PbTest::scheduleColumnEditor;
using PobCorrupt::Where;

class TestDbEditorRobustness : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void reconcileCreatesMissingAllView();
    void reconcileRestoresMissingAllEntry();
    void reconcileDropsOrphanedAllEntry();
    void getViewOmitsOrphanedColumn();
    void postAcceptSurvivesMissingAllView();
    void postAcceptSurvivesOrphanedAllEntry();
    void applyChangesSurvivesNonConsecutiveRowIds();
    void addColumnSurvivesNonConsecutiveRowIds();

private:
    /** Fresh working copy of the shared fixture for one test. */
    QString makeWorkingCopy(QTemporaryDir &dir)
    {
        QString path = dir.filePath("work.pob");
        if (!dir.isValid() || !QFile::copy(fixturePath, path)) {
            return QString();
        }
        return path;
    }

    /**
     * Run a crash-prone scenario against the file in a child process and
     * assert it survives. A crashed child means a guard the spec requires
     * has regressed.
     */
    void expectScenarioSurvives(const QString &scenario, const QString &path)
    {
        QProcess child;
        child.start(QCoreApplication::applicationFilePath(),
                    QStringList() << "--scenario" << scenario << path);
        QVERIFY2(child.waitForFinished(30000), "child process did not finish");
        QVERIFY2(child.exitStatus() == QProcess::NormalExit,
                 "child process crashed -- unguarded lookup regressed");
        QCOMPARE(child.exitCode(), 0);
    }

    QTemporaryDir fixtureDir;
    QString fixturePath;
};

void TestDbEditorRobustness::initTestCase()
{
    QVERIFY(fixtureDir.isValid());
    fixturePath = fixtureDir.filePath("fixture.pob");
    QString error = PbTest::buildStandardFile(fixturePath);
    QVERIFY2(error.isEmpty(), qPrintable(error));
}

/**
 * Accepting the dialog against a file that has views but no "_all" view
 * (a real-world historical-corruption state) must recreate "_all" with the
 * full column set.
 */
// @spec COL-DB-008
void TestDbEditorRobustness::reconcileCreatesMissingAllView()
{
    QTemporaryDir dir;
    QString path = makeWorkingCopy(dir);
    QVERIFY(!path.isEmpty());
    QVERIFY(PobCorrupt::removePortabaseView(path, "_all"));

    Database::OpenResult result;
    Database db(path, &result);
    QCOMPARE(result, Database::Success);
    db.load();
    runEditColumnsSession(&db, [](DBEditor *editor) {
        scheduleColumnEditor(editor, "addedColumn", STRING);
        QMetaObject::invokeMethod(editor, "addColumn");
    });

    QVERIFY(db.listViews().contains("_all"));
    QStringList allCols = db.listViewColumns("_all");
    QCOMPARE(allCols, db.listColumns());
}

/**
 * Accepting the dialog against a file whose "_all" view is missing an entry
 * for one existing column must restore that entry.
 */
// @spec COL-DB-008
void TestDbEditorRobustness::reconcileRestoresMissingAllEntry()
{
    QTemporaryDir dir;
    QString path = makeWorkingCopy(dir);
    QVERIFY(!path.isEmpty());
    QVERIFY(PobCorrupt::removeRows(path, "_viewcolumns",
                                   Where("_vcview", "_all"),
                                   Where("_vcname", "colFloat")));

    Database::OpenResult result;
    Database db(path, &result);
    QCOMPARE(result, Database::Success);
    db.load();
    runEditColumnsSession(&db, [](DBEditor *editor) {
        scheduleColumnEditor(editor, "addedColumn", STRING);
        QMetaObject::invokeMethod(editor, "addColumn");
    });

    QCOMPARE(db.listViewColumns("_all"), db.listColumns());
}

/**
 * Accepting the dialog against a file whose "_all" view carries an entry for
 * a column that no longer exists must drop the orphaned entry.
 */
// @spec COL-DB-008
void TestDbEditorRobustness::reconcileDropsOrphanedAllEntry()
{
    QTemporaryDir dir;
    QString path = makeWorkingCopy(dir);
    QVERIFY(!path.isEmpty());
    QVERIFY(PobCorrupt::setString(path, "_viewcolumns",
                                  Where("_vcview", "_all"),
                                  Where("_vcname", "colFloat"),
                                  "_vcname", "colGhost"));

    Database::OpenResult result;
    Database db(path, &result);
    QCOMPARE(result, Database::Success);
    db.load();
    runEditColumnsSession(&db, [](DBEditor *editor) {
        scheduleColumnEditor(editor, "addedColumn", STRING);
        QMetaObject::invokeMethod(editor, "addColumn");
    });

    QStringList allCols = db.listViewColumns("_all");
    QVERIFY(!allCols.contains("colGhost"));
    QCOMPARE(allCols, db.listColumns());
}

/**
 * Loading a view that references a nonexistent column must omit the
 * reference, not crash. Runs in a child process because a regression here
 * crashes in Database::getView's column lookup.
 */
// @spec COL-DB-009
void TestDbEditorRobustness::getViewOmitsOrphanedColumn()
{
    QTemporaryDir dir;
    QString path = makeWorkingCopy(dir);
    QVERIFY(!path.isEmpty());
    QVERIFY(PobCorrupt::setString(path, "_viewcolumns",
                                  Where("_vcview", "_all"),
                                  Where("_vcname", "colFloat"),
                                  "_vcname", "colGhost"));
    expectScenarioSurvives("getview", path);
}

/**
 * The full Edit Columns operation (dialog session plus the application's
 * post-accept sequence) against a file that has views but no "_all" view
 * must not crash: reconciliation recreates "_all" before the view is
 * reloaded.
 */
// @spec COL-DB-007
void TestDbEditorRobustness::postAcceptSurvivesMissingAllView()
{
    QTemporaryDir dir;
    QString path = makeWorkingCopy(dir);
    QVERIFY(!path.isEmpty());
    QVERIFY(PobCorrupt::removePortabaseView(path, "_all"));
    expectScenarioSurvives("post-accept", path);
}

/**
 * The full Edit Columns operation against a file whose "_all" view carries an
 * orphaned column reference must not crash during the post-accept view
 * reload.
 */
// @spec COL-DB-007
void TestDbEditorRobustness::postAcceptSurvivesOrphanedAllEntry()
{
    QTemporaryDir dir;
    QString path = makeWorkingCopy(dir);
    QVERIFY(!path.isEmpty());
    QVERIFY(PobCorrupt::setString(path, "_viewcolumns",
                                  Where("_vcview", "_all"),
                                  Where("_vcname", "colFloat"),
                                  "_vcname", "colGhost"));
    expectScenarioSurvives("post-accept", path);
}

/**
 * Recalculating an existing calculated column when the file's row IDs are not
 * consecutive from zero must not crash. Database::calculateAll must address
 * each row by its actual _id rather than by physical position; this fires on
 * any accepted Edit Columns dialog (applyChanges recalculates every
 * calculated column), independent of what, if anything, is added -- here
 * nothing is added, isolating the mechanism. This was the confirmed root
 * cause of the originally-reported crash.
 */
// @spec COL-DB-010
void TestDbEditorRobustness::applyChangesSurvivesNonConsecutiveRowIds()
{
    QTemporaryDir dir;
    QString path = makeWorkingCopy(dir);
    QVERIFY(!path.isEmpty());
    // Bump one row's _id above the row count so 0 is no longer present; the
    // standard fixture has a calculated column (calcCol).
    QVERIFY(PobCorrupt::setIntAt(path, "_data", 0, "_id", 9));
    expectScenarioSurvives("accept-noop", path);
}

/**
 * COL-DB-007 as originally reported: adding a column to a file with a
 * calculated column and non-consecutive row IDs -- the exact shape of the
 * real crashing file (a calc column plus an ID sequence left non-contiguous
 * by earlier row deletions/imports). Same getRow index/ID conflation as the
 * test above.
 */
// @spec COL-DB-007, COL-DB-010
void TestDbEditorRobustness::addColumnSurvivesNonConsecutiveRowIds()
{
    QTemporaryDir dir;
    QString path = makeWorkingCopy(dir);
    QVERIFY(!path.isEmpty());
    QVERIFY(PobCorrupt::setIntAt(path, "_data", 0, "_id", 9));
    expectScenarioSurvives("post-accept", path);
}

/**
 * Child-process entry point: run one crash-prone scenario against the given
 * file. Exit code 0 means the scenario completed; a crash or nonzero exit
 * means a guard has regressed.
 */
static int runScenario(const QString &name, const QString &path)
{
    Database::OpenResult result;
    Database db(path, &result);
    if (result != Database::Success) {
        return 3;
    }
    db.load();
    if (name == "getview") {
        View *view = db.getView("_all");
        return view ? 0 : 4;
    }
    if (name == "post-accept" || name == "accept-noop") {
        bool accepted = runEditColumnsSession(&db, [name](DBEditor *editor) {
            if (name == "post-accept") {
                scheduleColumnEditor(editor, "crashColumn", STRING);
                QMetaObject::invokeMethod(editor, "addColumn");
            }
        });
        if (!accepted) {
            return 5;
        }
        completePostAcceptSequence(&db);
        db.commit();
        return 0;
    }
    return 6;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QStringList args = app.arguments();
    int scenarioIndex = args.indexOf("--scenario");
    if (scenarioIndex != -1 && scenarioIndex + 2 < args.size()) {
        return runScenario(args[scenarioIndex + 1], args[scenarioIndex + 2]);
    }
    TestDbEditorRobustness testCase;
    QTEST_SET_MAIN_SOURCE_PATH
    return QTest::qExec(&testCase, argc, argv);
}

#include "dbeditor_robustness_test.moc"
