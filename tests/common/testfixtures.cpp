/*
 * testfixtures.cpp
 *
 * (c) 2026 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Shared builder for structurally valid PortaBase test fixture files.
 */

#include <QFile>
#include <QStringList>
#include "testfixtures.h"
#include "calc/calcnode.h"
#include "condition.h"
#include "database.h"
#include "datatypes.h"
#include "filter.h"

namespace PbTest {

/** Position-ordered column names created by buildStandardFile(). */
QStringList standardColumnNames()
{
    QStringList names;
    names << "colString" << "colInteger" << "colFloat" << "colBoolean"
          << "colNote" << "colDate" << "colTime" << "colSequence"
          << "colImage" << "colStatus" << "calcCol";
    return names;
}

/**
 * Build a structurally valid PortaBase file at the given path, matching what
 * the real application produces (see docs/wiki/format.md): one column of every
 * base type plus an enum column and a calculated column, the given number of
 * populated data rows, the mandatory "_all" view listing every column in
 * order, and one user-defined view ("intView"), sorting ("intSort"), and
 * filter ("intFilter") that all reference the "colInteger" column.
 *
 * Columns, in position order: colString, colInteger, colFloat, colBoolean,
 * colNote, colDate, colTime, colSequence, colImage, colStatus (enum
 * "Status": Open/Closed), calcCol (calculation: colInteger * 2).
 *
 * @param path Where to create the file (must not already exist)
 * @param dataRows How many data rows to populate (0 or more)
 * @return An empty string on success, an error message otherwise
 */
QString buildStandardFile(const QString &path, int dataRows)
{
    if (QFile::exists(path)) {
        return QString("fixture file already exists: %1").arg(path);
    }
    Database::OpenResult result;
    Database db(path, &result);
    if (result != Database::Success) {
        return QString("failed to create fixture file: %1").arg(path);
    }
    db.load();

    db.addEnum("Status", QStringList() << "Open" << "Closed");
    int statusId = db.getEnumId("Status");

    db.addColumn(0, "colString", STRING, "");
    db.addColumn(1, "colInteger", INTEGER, "0");
    db.addColumn(2, "colFloat", FLOAT, "0");
    db.addColumn(3, "colBoolean", BOOLEAN, "0");
    db.addColumn(4, "colNote", NOTE, "");
    db.addColumn(5, "colDate", DATE, "0");
    db.addColumn(6, "colTime", TIME, "0");
    db.addColumn(7, "colSequence", SEQUENCE, "1");
    db.addColumn(8, "colImage", IMAGE, "");
    db.addColumn(9, "colStatus", statusId, "Open");
    db.updateDataFormat();

    for (int i = 0; i < dataRows; i++) {
        QStringList values;
        values << QString("text%1").arg(i);      // colString
        values << QString::number(40 + i);       // colInteger
        values << QString("3.5");                // colFloat
        values << QString::number(i % 2);        // colBoolean
        values << QString("note %1").arg(i);     // colNote
        values << "20240115";                    // colDate
        values << "3600";                        // colTime
        values << QString::number(i + 1);        // colSequence
        values << "";                            // colImage (blank -- no file)
        values << ((i % 2 == 0) ? "Open" : "Closed"); // colStatus
        int rowId = 0;
        QString error = db.addRow(values, &rowId, false);
        if (!error.isEmpty()) {
            return QString("addRow failed: %1").arg(error);
        }
    }

    // The calculated column is added after the data rows: addColumn() writes
    // no data values for CALC columns, and updateCalc() recalculates every
    // existing row once the calculation definition is saved.
    db.addColumn(10, "calcCol", CALC, "");
    db.updateDataFormat();
    CalcNode *root = new CalcNode(CalcNode::Multiply, "");
    root->addChild(new CalcNode(CalcNode::Column, "colInteger"));
    root->addChild(new CalcNode(CalcNode::Constant, "2"));
    db.updateCalc("calcCol", root, 2);
    delete root;

    // The real application creates "_all" the first time the Edit Columns
    // dialog is accepted (PortaBase::editColumns); a valid saved file always
    // has it, listing every column in position order.
    db.addView("_all", db.listColumns(), "_none", "_none");

    db.addView("intView", QStringList() << "colInteger" << "colString",
               "_none", "_none");
    db.addSorting("intSort", QStringList() << "colInteger", QStringList());
    Filter filter(&db, "intFilter");
    Condition *condition = new Condition(&db);
    condition->setColName("colInteger");
    condition->setOperator(Condition::GreaterEqual);
    condition->setConstant("0");
    filter.addCondition(condition);
    db.addFilter(&filter, false);

    db.commit();
    return "";
}

}
