/*
 * dbeditordriver.cpp
 *
 * (c) 2026 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * Helpers for driving the Edit Columns dialog from QtTest.
 */

#include <QApplication>
#include <QTimer>
#include "dbeditordriver.h"
#include "columneditor.h"
#include "database.h"
#include "dbeditor.h"

namespace PbTest {

/**
 * Run one Edit Columns dialog session against the database, as
 * PortaBase::editColumns() would: construct a fresh DBEditor (the dialog is
 * single-use by design), let `script` perform staged edits through the
 * editor's slots once the dialog is up, then accept (or cancel) it and apply
 * the changes if accepted.
 *
 * @param db The database to edit
 * @param script Staged edits, invoked with the live editor
 * @param accept True to accept the dialog (and apply), false to cancel
 * @return True if the dialog was accepted
 */
bool runEditColumnsSession(Database *db,
                           const std::function<void(DBEditor *)> &script,
                           bool accept)
{
    DBEditor editor(0);
    QTimer::singleShot(0, &editor, [&editor, &script, accept]() {
        script(&editor);
        if (accept) {
            editor.accept();
        }
        else {
            editor.reject();
        }
    });
    int accepted = editor.edit(db);
    if (accepted) {
        editor.applyChanges();
    }
    return accepted != 0;
}

/**
 * Schedule filling in and accepting the ColumnEditor subdialog that the
 * *next* editor action (addColumn/editColumn) will open. Call this inside a
 * session script immediately before invoking the action that opens the
 * subdialog.
 *
 * @param context Object whose lifetime bounds the scheduled interaction
 * @param name Column name to set (null string to leave unchanged)
 * @param type Column type to set (-1 to leave unchanged)
 * @param defaultVal Default value to set (null string to leave unchanged)
 */
void scheduleColumnEditor(QObject *context, const QString &name, int type,
                          const QString &defaultVal)
{
    QTimer::singleShot(0, context, [name, type, defaultVal]() {
        ColumnEditor *columnEditor =
            qobject_cast<ColumnEditor *>(QApplication::activeModalWidget());
        if (!columnEditor) {
            qWarning("scheduleColumnEditor: no ColumnEditor is active");
            return;
        }
        if (!name.isNull()) {
            columnEditor->setName(name);
        }
        if (type != -1) {
            columnEditor->setType(type);
        }
        if (!defaultVal.isNull()) {
            columnEditor->setDefaultValue(defaultVal);
        }
        columnEditor->accept();
    });
}

/**
 * The Database-touching steps PortaBase::editColumns() runs after
 * applyChanges() (portabase.cpp): create the "_all" view if the file has
 * no views, otherwise re-sync its column positions; then reload the "_all"
 * view as the data viewer would. Mirrors the application's post-accept
 * sequence minus the display widgets.
 */
void completePostAcceptSequence(Database *db)
{
    if (db->listViews().isEmpty()) {
        db->addView("_all", db->listColumns(), "_none", "_none");
    }
    else {
        db->setViewColumnSequence("_all", db->listColumns());
    }
    db->getView("_all");
}

}
