/*
 * calcdateeditor.cpp
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qdatetime.h>
#include <qgrid.h>
#include <qhbox.h>
#include <qmessagebox.h>
#include <qradiobutton.h>
#include "calcnode.h"
#include "calctimeeditor.h"
#include "../database.h"
#include "../datatypes.h"
#include "../timewidget.h"

CalcTimeEditor::CalcTimeEditor(Database *dbase, const QStringList &colNames, int *colTypes, QWidget *parent, const char *name, WFlags f)
  : PBDialog(tr("Calculation Node Editor"), parent, name, f), db(dbase)
{
    group = new QButtonGroup(this);
    group->hide();
    QGrid *grid = new QGrid(2, this);
    vbox->addWidget(grid);
    QRadioButton *colButton = new QRadioButton(tr("Column"), grid);
    group->insert(colButton, 0);
    columnList = new QComboBox(FALSE, grid);
    int count = colNames.count();
    int i;
    for (i = 0; i < count; i++) {
        int type = colTypes[i];
        if (type == TIME) {
            columnList->insertItem(colNames[i]);
        }
    }

    group->insert(new QRadioButton(tr("Constant"), grid), 1);
    timeWidget = new TimeWidget(grid);

    if (columnList->count() > 0) {
        group->setButton(0);
    }
    else {
        // no date columns, so can't select one
        group->setButton(1);
        colButton->setEnabled(FALSE);
    }

    finishLayout(TRUE, TRUE, FALSE, parent->width() / 2);
}

CalcTimeEditor::~CalcTimeEditor()
{

}

void CalcTimeEditor::reset()
{
    group->setButton(0);
    if (columnList->count() > 0) {
        columnList->setCurrentItem(0);
    }
    QTime now = QTime::currentTime();
    timeWidget->setTime(now);
}

CalcNode *CalcTimeEditor::createNode()
{
    CalcNode *node = new CalcNode(0, "");
    updateNode(node);
    return node;
}

void CalcTimeEditor::updateNode(CalcNode *node)
{
    int selection = group->id(group->selected());
    if (selection == 0) {
        node->setType(CALC_TIME_COLUMN);
        node->setValue(columnList->currentText());
    }
    else {
        node->setType(CALC_TIME_CONSTANT);
        QString timeString = timeWidget->getTime();
        bool ok;
        QString time = db->parseTimeString(timeString, &ok);
        // isValid() has already been checked, so ignore "ok"
        node->setValue(time);
    }
}

void CalcTimeEditor::setNode(CalcNode *node)
{
    reset();
    int type = node->type();
    QString value = node->value();
    if (type == CALC_TIME_CONSTANT) {
        group->setButton(1);
        timeWidget->setTime(value.toInt());
    }
    else if (type == CALC_TIME_COLUMN) {
        group->setButton(0);
        int count = columnList->count();
        for (int i = 0; i < count; i++) {
            if (columnList->text(i) == value) {
                columnList->setCurrentItem(i);
                break;
            }
        }
    }
}

bool CalcTimeEditor::isValid()
{
    if (group->id(group->selected()) == 0) {
        return TRUE;
    }
    QString time = timeWidget->getTime();
    QString error = db->isValidValue(TIME, time);
    if (error != "") {
        QString message = tr("Constant") + ": " + error;
        QMessageBox::warning(this, tr("PortaBase"), message);
        return FALSE;
    }
    return TRUE;
}
