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
#include <qcombobox.h>
#include <qdialog.h>
#include <qgrid.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qscrollview.h>
#include "database.h"
#include "datatypes.h"
#include "datewidget.h"
#include "notebutton.h"
#include "roweditor.h"
#include "timewidget.h"

RowEditor::RowEditor(QWidget *parent, const char *name, WFlags f)
  : QDialog(parent, name, TRUE, f), db(0), colTypes(0), sv(0)
{
    setCaption(tr("Row Editor") + " - " + tr("PortaBase"));
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
            int timeWidgetIndex = 0;
            for (int i = 0; i < count; i++) {
                int type = colTypes[i];
                if (type == STRING || type == INTEGER || type == FLOAT) {
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
                else if (type == TIME) {
                    QString value = timeWidgets[timeWidgetIndex]->getTime();
                    QString error = db->isValidValue(type, value);
                    if (error != "") {
                        QString message = colNames[i] + " " + error;
		        QMessageBox::warning(this, tr("PortaBase"), message);
                        finished = FALSE;
                        break;
                    }
                    timeWidgetIndex++;
                }
            }
        }
    }
    if (!aborted) {
        QStringList values;
        int checkBoxIndex = 0;
        int lineEditIndex = 0;
        int noteButtonIndex = 0;
        int dateWidgetIndex = 0;
        int timeWidgetIndex = 0;
        int comboBoxIndex = 0;
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
            else if (type == DATE) {
                int dateInt = dateWidgets[dateWidgetIndex]->getDate();
                values.append(QString::number(dateInt));
                dateWidgetIndex++;
            }
            else if (type == TIME) {
                values.append(timeWidgets[timeWidgetIndex]->getTime());
                timeWidgetIndex++;
            }
            else if (type >= FIRST_ENUM) {
                values.append(comboBoxes[comboBoxIndex]->currentText());
                comboBoxIndex++;
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
    sv = new QScrollView(this);
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
    colTypes = db->listTypes();
    for (int i = 0; i < count; i++) {
        QString name = colNames[i];
        int type = colTypes[i];
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
        else if (type == DATE) {
            DateWidget *widget = new DateWidget(grid);
            widget->setDate(values[i].toInt());
            dateWidgets.append(widget);
        }
        else if (type == TIME) {
            TimeWidget *widget = new TimeWidget(grid);
            widget->setTime(values[i].toInt());
            timeWidgets.append(widget);
        }
        else if (type >= FIRST_ENUM) {
            QComboBox *combo = new QComboBox(FALSE, grid);
            QStringList options = db->listEnumOptions(type);
            combo->insertStringList(options);
            int index = options.findIndex(values[i]);
            combo->setCurrentItem(index);
            comboBoxes.append(combo);
        }
        else {
            lineEdits.append(new QLineEdit(values[i], grid));
        }
    }
    new QWidget(grid);
    new QWidget(grid);
    showMaximized();
}

void RowEditor::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    if (sv) {
        sv->resize(size());
    }
}
