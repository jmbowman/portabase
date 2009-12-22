/*
 * roweditor.cpp
 *
 * (c) 2002-2004,2008-2009 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file roweditor.cpp
 * Source file for RowEditor
 */

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QScrollArea>
#include "calc/calcwidget.h"
#include "image/imageselector.h"
#include "database.h"
#include "datatypes.h"
#include "datewidget.h"
#include "dynamicedit.h"
#include "factory.h"
#include "notebutton.h"
#include "numberwidget.h"
#include "roweditor.h"
#include "timewidget.h"

/**
 * Constructor.
 *
 * @param parent This dialog's parent widget
 */
RowEditor::RowEditor(QWidget *parent)
  : PBDialog(tr("Row Editor"), parent), db(0)
{

}

/**
 * Launch the dialog to edit (or copy) a particular row of the database.
 *
 * @param subject The database being modified
 * @param rowId The ID of the row to edit
 * @param copy True if the row should be copied rather than edited
 * @return True if any changes made in the dialog were committed
 */
bool RowEditor::edit(Database *subject, int rowId, bool copy)
{
    db = subject;
    addContent(rowId);
    bool finished = false;
    bool aborted = false;
    while (!finished) {
        if (!exec()) {
            finished = true;
            aborted = true;
        }
        else {
            finished = isValid();
        }
    }
    if (!aborted) {
        QStringList values = getRow();
        int id;
        if (rowId == -1 || copy) {
            db->addRow(values, &id);
        }
        else {
            db->updateRow(rowId, values);
            id = rowId;
        }
        int count = imageSelectors.count();
        for (int i = 0; i < count; i++) {
            ImageSelector *widget = imageSelectors[i];
            widget->saveImage(id);
        }
        return true;
    }
    else {
        return false;
    }
}

/**
 * Determine if all of the values entered in the dialog were acceptable
 * for their respective data types.  In particular, validation is performed
 * for integer, float, and time fields.
 *
 * @return True if all the values validated successfully, false otherwise
 */
bool RowEditor::isValid()
{
    int count = colNames.count();
    int numberWidgetIndex = 0;
    int timeWidgetIndex = 0;
    for (int i = 0; i < count; i++) {
        int type = colTypes[i];
        if (type == INTEGER || type == FLOAT) {
            QString value = numberWidgets[numberWidgetIndex]->getValue();
            QString error = db->isValidValue(type, value);
            if (!error.isEmpty()) {
                QString message = colNames[i] + " " + error;
                QMessageBox::warning(this, qApp->applicationName(), message);
                return false;
            }
            numberWidgetIndex++;
        }
        else if (type == TIME) {
            QString value = timeWidgets[timeWidgetIndex]->getTime();
            QString error = db->isValidValue(type, value);
            if (!error.isEmpty()) {
                QString message = colNames[i] + " " + error;
                QMessageBox::warning(this, qApp->applicationName(), message);
                return false;
            }
            timeWidgetIndex++;
        }
    }
    return true;
}

/**
 * Get the row of data as currently shown in the dialog.
 *
 * @param doCalcs True if calculated fields should be recalculated
 * @return The row of data this dialog was editing
 */
QStringList RowEditor::getRow(bool doCalcs)
{
    QStringList values;
    int checkBoxIndex = 0;
    int noteButtonIndex = 0;
    int dateWidgetIndex = 0;
    int timeWidgetIndex = 0;
    int calcWidgetIndex = 0;
    int numberWidgetIndex = 0;
    int comboBoxIndex = 0;
    int dynamicEditIndex = 0;
    int labelIndex = 0;
    int imageSelectorIndex = 0;
    int count = colNames.count();
    for (int i = 0; i < count; i++) {
        int type = colTypes[i];
        if (type == BOOLEAN) {
            int value = checkBoxes[checkBoxIndex]->isChecked() ? 1 : 0;
            values.append(QString::number(value));
            checkBoxIndex++;
        }
        else if (type == INTEGER || type == FLOAT) {
            values.append(numberWidgets[numberWidgetIndex]->getValue());
            numberWidgetIndex++;
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
            QString timeString = timeWidgets[timeWidgetIndex]->getTime();
            bool ok;
            values.append(db->parseTimeString(timeString, &ok));
            timeWidgetIndex++;
        }
        else if (type == CALC) {
            if (doCalcs) {
                calcWidgets[calcWidgetIndex]->calculate();
            }
            values.append(calcWidgets[calcWidgetIndex]->getValue());
            calcWidgetIndex++;
        }
        else if (type == SEQUENCE) {
            values.append(sequenceLabels[labelIndex]->text());
            labelIndex++;
        }
        else if (type == IMAGE) {
            ImageSelector *widget = imageSelectors[imageSelectorIndex];
            values.append(widget->getFormat());
            imageSelectorIndex++;
        }
        else if (type >= FIRST_ENUM) {
            values.append(comboBoxes[comboBoxIndex]->currentText());
            comboBoxIndex++;
        }
        else {
            values.append(dynamicEdits[dynamicEditIndex]->toPlainText());
            dynamicEditIndex++;
        }
    }
    return values;
}

/**
 * Populate this dialog with the appropriate field editor widgets for the
 * database's columns, and set them to match the data in the specified row.
 *
 * @param rowId The data row to be edited or copied
 */
void RowEditor::addContent(int rowId)
{
    QScrollArea *sa = new QScrollArea(this);
    vbox->addWidget(sa);
    QWidget *grid = new QWidget();
    sa->setWidgetResizable(true);
    colNames = db->listColumns();
    int count = colNames.count();
    QGridLayout *layout = Factory::gridLayout(grid, true);
    QStringList values;
    if (rowId != -1) {
        values = db->getRow(rowId);
    }
    else {
        for (int i = 0; i < count; i++) {
            values.append(db->getDefault(colNames[i]));
        }
    }
    colTypes = db->listTypes();
    for (int i = 0; i < count; i++) {
        QString name = colNames[i];
        int type = colTypes[i];
        layout->addWidget(new QLabel(name + " ", grid), i, 0);
        if (type == BOOLEAN) {
            QCheckBox *box = new QCheckBox(grid);
            layout->addWidget(box, i, 1);
            if (values[i].toInt()) {
                box->setChecked(true);
            }
            checkBoxes.append(box);
        }
        else if (type == INTEGER || type == FLOAT) {
            NumberWidget *widget = new NumberWidget(type, grid);
            layout->addWidget(widget, i, 1);
            widget->setValue(values[i]);
            numberWidgets.append(widget);
        }
        else if (type == NOTE) {
            NoteButton *button = new NoteButton(name, grid);
            layout->addWidget(button, i, 1);
            button->setContent(values[i]);
            noteButtons.append(button);
        }
        else if (type == DATE) {
            DateWidget *widget = new DateWidget(grid);
            layout->addWidget(widget, i, 1);
            widget->setDate(values[i].toInt());
            dateWidgets.append(widget);
        }
        else if (type == TIME) {
            TimeWidget *widget = new TimeWidget(grid);
            layout->addWidget(widget, i, 1);
            int defaultTime = values[i].toInt();
            if (defaultTime == 0) {
                defaultTime = -2;
            }
            widget->setTime(defaultTime);
            timeWidgets.append(widget);
        }
        else if (type == CALC) {
            CalcWidget *widget = new CalcWidget(db, name, colNames, this, grid);
            layout->addWidget(widget, i, 1);
            widget->setValue(values[i]);
            calcWidgets.append(widget);
        }
        else if (type == SEQUENCE) {
            QLabel *label = new QLabel(values[i], grid);
            layout->addWidget(label, i, 1);
            sequenceLabels.append(label);
        }
        else if (type == IMAGE) {
            ImageSelector *widget = new ImageSelector(db, grid);
            layout->addWidget(widget, i, 1);
            widget->setField(rowId, name);
            widget->setFormat(values[i]);
            imageSelectors.append(widget);
        }
        else if (type >= FIRST_ENUM) {
            QComboBox *combo = new QComboBox(grid);
            layout->addWidget(combo, i, 1);
            QStringList options = db->listEnumOptions(type);
            combo->addItems(options);
            int index = options.indexOf(values[i]);
            combo->setCurrentIndex(index);
            comboBoxes.append(combo);
        }
        else {
            DynamicEdit *edit = new DynamicEdit(grid);
            layout->addWidget(edit, i, 1);
            edit->setPlainText(values[i]);
            dynamicEdits.append(edit);
        }
    }
    layout->addWidget(new QWidget(grid), count, 0, 1, 2);
    layout->setRowStretch(count, 1);
    sa->setWidget(grid);

    finishLayout(true, true, 400, 400);
}
