/*
 * roweditor.cpp
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qcheckbox.h>
#include <qdialog.h>
#include <qgrid.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qscrollview.h>
#include "database.h"
#include "datatypes.h"
#include "notebutton.h"
#include "roweditor.h"

RowEditor::RowEditor(QWidget *parent, const char *name, WFlags f)
  : QDialog(parent, name, TRUE, f), db(0), colTypes(0)
{
    setCaption(tr("PortaBase") + " - " + tr("Row Editor"));
    showMaximized();
    hide();
}

RowEditor::~RowEditor()
{

}

bool RowEditor::edit(Database *subject, int rowId)
{
    db = subject;
    addContent(rowId);
    int count = colNames.count();
    bool finished = FALSE;
    bool aborted = FALSE;
    while (!finished) {
        if (!exec()) {
            finished = TRUE;
            aborted = TRUE;
        }
        else {
            finished = TRUE;
            int lineEditIndex = 0;
            for (int i = 0; i < count; i++) {
                int type = colTypes[i];
                if (type != BOOLEAN && type != NOTE) {
                    QString value = lineEdits[lineEditIndex]->text();
                    QString error = db->isValidValue(type, value);
                    if (error != "") {
                        QString message = colNames[i] + " " + error;
		        QMessageBox::warning(this, tr("PortaBase"), message);
                        finished = FALSE;
                        break;
                    }
                    lineEditIndex++;
                }
            }
        }
    }
    if (!aborted) {
        QStringList values;
        int checkBoxIndex = 0;
        int lineEditIndex = 0;
        int noteButtonIndex = 0;
        for (int i = 0; i < count; i++) {
            int type = colTypes[i];
            if (type == BOOLEAN) {
                int value = checkBoxes[checkBoxIndex]->isChecked() ? 1 : 0;
                values.append(QString::number(value));
                checkBoxIndex++;
            }
            else if (type == NOTE) {
                values.append(noteButtons[noteButtonIndex]->content());
                noteButtonIndex++;
            }
            else {
                values.append(lineEdits[lineEditIndex]->text());
                lineEditIndex++;
            }
        }
        if (rowId == -1) {
            db->addRow(values);
        }
        else {
            db->updateRow(rowId, values);
        }
        return TRUE;
    }
    else {
        return FALSE;
    }
}

void RowEditor::addContent(int rowId)
{
    QScrollView *sv = new QScrollView(this);
    sv->resize(size());
    QGrid *grid = new QGrid(2, sv->viewport());
    sv->addChild(grid);
    sv->setResizePolicy(QScrollView::AutoOneFit);
    grid->resize(sv->visibleWidth(), sv->visibleHeight());
    colNames = db->listColumns();
    int count = colNames.count();
    QStringList values;
    if (rowId != -1) {
        values = db->getRow(rowId);
    }
    else {
        for (int i = 0; i < count; i++) {
            values.append(db->getDefault(colNames[i]));
        }
    }
    if (colTypes) {
        delete[] colTypes;
    }
    colTypes = new int[count];
    for (int i = 0; i < count; i++) {
        QString name = colNames[i];
        int type = db->getType(name);
        colTypes[i] = type;
        new QLabel(name, grid);
        if (type == BOOLEAN) {
            QCheckBox *box = new QCheckBox(grid);
            if (values[i].toInt()) {
                box->setChecked(TRUE);
            }
            checkBoxes.append(box);
        }
        else if (type == NOTE) {
            NoteButton *button = new NoteButton(name, grid);
            button->setContent(values[i]);
            noteButtons.append(button);
        }
        else {
            lineEdits.append(new QLineEdit(values[i], grid));
        }
    }
    new QWidget(grid);
    new QWidget(grid);
}
