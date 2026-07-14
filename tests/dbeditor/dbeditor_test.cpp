#include <QTest>
#include <QApplication>
#include <QTemporaryDir>
#include <QTimer>
#include <algorithm>

#include "columneditor.h"
#include "database.h"
#include "datatypes.h"
#include "dbeditor.h"

/**
 * Regression coverage for COL-DB-007: adding a column to an existing
 * (previously-saved) database file must not crash. The fixture file is
 * built once, with one column of every base type and a populated data row,
 * then copied fresh for each test case rather than rebuilt from scratch --
 * matching a real previously-saved file more closely than an empty one, and
 * keeping per-type test cases cheap.
 */
class TestDbEditor : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void addColumn_data();
    void addColumn();

    void reorderThenDeleteColumn();

private:
    void addColumnOfType(int type);

    QTemporaryDir fixtureDir;
    QString fixturePath;
};

void TestDbEditor::initTestCase()
{
    QVERIFY(fixtureDir.isValid());
    fixturePath = fixtureDir.filePath("fixture.pob");

    Database::OpenResult result;
    Database db(fixturePath, &result);
    QCOMPARE(result, Database::Success);
    db.load();

    db.addColumn(0, "colString", STRING, "");
    db.addColumn(1, "colInteger", INTEGER, "0");
    db.addColumn(2, "colFloat", FLOAT, "0");
    db.addColumn(3, "colBoolean", BOOLEAN, "0");
    db.addColumn(4, "colNote", NOTE, "");
    db.addColumn(5, "colDate", DATE, "0");
    db.addColumn(6, "colTime", TIME, "0");
    db.addColumn(7, "colSequence", SEQUENCE, "1");
    db.addColumn(8, "colImage", IMAGE, "");
    db.updateDataFormat();

    QStringList values;
    values << "hello";        // colString
    values << "42";           // colInteger
    values << "3.5";          // colFloat
    values << "1";            // colBoolean
    values << "a note";       // colNote
    values << "20240115";     // colDate
    values << "3600";         // colTime
    values << "1";            // colSequence
    values << "";             // colImage (blank -- no file to load in a test)
    int rowId = 0;
    QString error = db.addRow(values, &rowId, true);
    QVERIFY2(error.isEmpty(), qPrintable(error));

    db.commit();
}

/**
 * Copy the shared fixture into a fresh temporary file, open it as a real
 * second session would (PortaBase::openFile() constructs a new Database
 * against the path -- it doesn't reuse the object that wrote it), then add
 * one column of the given type through the actual DBEditor dialog flow
 * (PortaBase::editColumns()'s sequence: edit() then applyChanges()).
 */
// @spec COL-DB-007
void TestDbEditor::addColumnOfType(int type)
{
    QTemporaryDir workDir;
    QVERIFY(workDir.isValid());
    QString path = workDir.filePath("work.pob");
    QVERIFY(QFile::copy(fixturePath, path));

    Database::OpenResult result;
    Database db(path, &result);
    QCOMPARE(result, Database::Success);
    db.load();
    QStringList originalColumns = db.listColumns();

    DBEditor editor(0);
    QTimer::singleShot(0, &editor, [&editor, type]() {
        QTimer::singleShot(0, &editor, [type]() {
            ColumnEditor *columnEditor =
                qobject_cast<ColumnEditor*>(QApplication::activeModalWidget());
            QVERIFY(columnEditor);
            columnEditor->setName("newColumn");
            columnEditor->setType(type);
            columnEditor->accept();
        });
        QMetaObject::invokeMethod(&editor, "addColumn");
        editor.accept();
    });
    int accepted = editor.edit(&db);
    QVERIFY(accepted);

    editor.applyChanges();

    QStringList columns = db.listColumns();
    foreach (const QString &name, originalColumns) {
        QVERIFY(columns.contains(name));
    }
    QVERIFY(columns.contains("newColumn"));
    QCOMPARE(db.getType("newColumn"), type);

    db.commit();

    // Confirm the added column actually persisted, not just staged in memory.
    Database::OpenResult result2;
    Database reopened(path, &result2);
    QCOMPARE(result2, Database::Success);
    reopened.load();
    QStringList columns2 = reopened.listColumns();
    QVERIFY(columns2.contains("newColumn"));
    QCOMPARE(reopened.getType("newColumn"), type);
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

void TestDbEditor::addColumn()
{
    QFETCH(int, type);
    addColumnOfType(type);
}

/**
 * Reorder a column across another one, then delete a column that sits
 * between the reordered column's old and new staged position, and confirm
 * the persisted column index sequence comes out as a clean permutation of
 * 0..columnCount-1, not a duplicate and a gap.
 */
// @spec COL-UI-010
void TestDbEditor::reorderThenDeleteColumn()
{
    QTemporaryDir workDir;
    QVERIFY(workDir.isValid());
    QString path = workDir.filePath("work.pob");
    QVERIFY(QFile::copy(fixturePath, path));

    Database::OpenResult result;
    Database db(path, &result);
    QCOMPARE(result, Database::Success);
    db.load();

    DBEditor editor(0);
    QTimer::singleShot(0, &editor, [&editor]() {
        // Move colInteger down twice: past colFloat, then past colBoolean.
        QMetaObject::invokeMethod(&editor, "selectRow", Q_ARG(QString, QString("colInteger")));
        QMetaObject::invokeMethod(&editor, "moveDown");
        QMetaObject::invokeMethod(&editor, "selectRow", Q_ARG(QString, QString("colInteger")));
        QMetaObject::invokeMethod(&editor, "moveDown");

        // Delete colFloat, which the first move above left sitting at
        // colInteger's original physical position in `info`.
        QMetaObject::invokeMethod(&editor, "selectRow", Q_ARG(QString, QString("colFloat")));
        QMetaObject::invokeMethod(&editor, "deleteColumn");

        editor.accept();
    });
    int accepted = editor.edit(&db);
    QVERIFY(accepted);

    editor.applyChanges();
    db.commit();

    QStringList columns = db.listColumns();
    QCOMPARE(columns.count(), 8); // one column (colFloat) deleted from the 9 in the fixture

    QList<int> indices;
    foreach (const QString &name, columns) {
        indices.append(db.getIndex(name));
    }
    std::sort(indices.begin(), indices.end());
    QList<int> expected;
    for (int i = 0; i < columns.count(); i++) {
        expected.append(i);
    }
    QCOMPARE(indices, expected);
}

QTEST_MAIN(TestDbEditor)
#include "dbeditor_test.moc"
