/*
 * calcdateeditor.cpp
 *
 * (c) 2003-2004,2008 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file calctimeeditor.cpp
 * Source file for CalcTimeEditor
 */

#include <QApplication>
#include <QButtonGroup>
#include <QComboBox>
#include <QDateTime>
#include <QLayout>
#include <QMessageBox>
#include <QRadioButton>
#include "calcnode.h"
#include "calctimeeditor.h"
#include "../database.h"
#include "../datatypes.h"
#include "../timewidget.h"

/**
 * Constructor.
 *
 * @param dbase The database being edited
 * @param colNames Ordered list of all the database's column names
 * @param colTypes Ordered list of all the database's column types
 * @param parent This dialog's parent widget
 */
CalcTimeEditor::CalcTimeEditor(Database *dbase, const QStringList &colNames, int *colTypes, QWidget *parent)
  : PBDialog(tr("Calculation Node Editor"), parent), db(dbase)
{
    group = new QButtonGroup(this);
    QGridLayout *grid = new QGridLayout(this);
    vbox->addLayout(grid);
    QRadioButton *colButton = new QRadioButton(tr("Column"), this);
    grid->addWidget(colButton, 0, 0);
    group->addButton(colButton, 0);
    columnList = new QComboBox(this);
    grid->addWidget(columnList, 0, 1);
    int count = colNames.count();
    int i;
    for (i = 0; i < count; i++) {
        int type = colTypes[i];
        if (type == TIME) {
            columnList->addItem(colNames[i]);
        }
    }

    QRadioButton *constantButton = new QRadioButton(tr("Constant"), this);
    grid->addWidget(constantButton, 1, 0);
    group->addButton(constantButton, 1);
    timeWidget = new TimeWidget(this);
    grid->addWidget(timeWidget, 1, 1);

    if (columnList->count() > 0) {
        group->button(0)->setChecked(true);
    }
    else {
        // no date columns, so can't select one
        group->button(1)->setChecked(true);
        colButton->setEnabled(false);
    }

    finishLayout(parent->width() / 2);
}

/**
 * Reset the dialog's entry widgets to their default values.  Typically used
 * when relaunching it for a different node.
 */
void CalcTimeEditor::reset()
{
    group->button(0)->setChecked(true);
    if (columnList->count() > 0) {
        columnList->setCurrentIndex(0);
    }
    QTime now = QTime::currentTime();
    timeWidget->setTime(now);
}

/**
 * Create a new calculation node and set it's properties to match those
 * currently selected in this dialog.
 *
 * @return The newly created and configured node
 */
CalcNode *CalcTimeEditor::createNode()
{
    CalcNode *node = new CalcNode(CalcNode::TimeConstant, "");
    updateNode(node);
    return node;
}

/**
 * Set the properties of the provided node to match those currently
 * selected in this dialog.
 *
 * @param node The node to be updated
 */
void CalcTimeEditor::updateNode(CalcNode *node)
{
    int selection = group->checkedId();
    if (selection == 0) {
        node->setType(CalcNode::TimeColumn);
        node->setValue(columnList->currentText());
    }
    else {
        node->setType(CalcNode::TimeConstant);
        QString timeString = timeWidget->getTime();
        bool ok;
        QString time = db->parseTimeString(timeString, &ok);
        // isValid() has already been checked, so ignore "ok"
        node->setValue(time);
    }
}

/**
 * Update this dialog's properties to match those of the provided node.
 *
 * @param node The node to be reflected by the dialog
 */
void CalcTimeEditor::setNode(CalcNode *node)
{
    reset();
    CalcNode::NodeType type = node->type();
    QString value = node->value();
    if (type == CalcNode::TimeConstant) {
        group->button(1)->setChecked(true);
        timeWidget->setTime(value.toInt());
    }
    else if (type == CalcNode::TimeColumn) {
        group->button(0)->setChecked(true);
        int count = columnList->count();
        for (int i = 0; i < count; i++) {
            if (columnList->itemText(i) == value) {
                columnList->setCurrentIndex(i);
                break;
            }
        }
    }
}

/**
 * Determine if the currently entered values represent a valid node or not.
 *
 * @return True if the selected properties are valid, false otherwise
 */
bool CalcTimeEditor::isValid()
{
    if (group->checkedId() == 0) {
        return true;
    }
    QString time = timeWidget->getTime();
    QString error = db->isValidValue(TIME, time);
    if (!error.isEmpty()) {
        QString message = tr("Constant") + ": " + error;
        QMessageBox::warning(this, qApp->applicationName(), message);
        return false;
    }
    return true;
}
