/*
 * dbeditordriver.h
 *
 * (c) 2026 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Helpers for driving the Edit Columns dialog (DBEditor) and its ColumnEditor
 * subdialog from QtTest, and for completing the post-accept sequence the
 * application runs after the dialog closes. Shared test infrastructure for
 * the column-editing test suites.
 */

#ifndef DBEDITORDRIVER_H
#define DBEDITORDRIVER_H

#include <QString>
#include <functional>

class Database;
class DBEditor;
class QObject;

namespace PbTest {

bool runEditColumnsSession(Database *db,
                           const std::function<void(DBEditor *)> &script,
                           bool accept = true);

void scheduleColumnEditor(QObject *context, const QString &name,
                          int type = -1,
                          const QString &defaultVal = QString());

void completePostAcceptSequence(Database *db);

}

#endif
