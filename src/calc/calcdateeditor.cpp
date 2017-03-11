/*
 * calcdateeditor.cpp
 *
 * (c) 2003-2004,2008,2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file calcdateeditor.cpp
 * Source file for CalcDateEditor
 */

#include <QButtonGroup>
#include <QComboBox>
#include <QDateTime>
#include <QLayout>
#include <QRadioButton>
#include "calcdateeditor.h"
#include "calcnode.h"
#include "../datatypes.h"
#include "../datewidget.h"
#include "../factory.h"

/**
 * Constructor.
 *
 * @param colNames The names of all columns in the database
 * @param colTypes The type IDs of all columns in the database
 * @param parent This dialog's parent widget
 */
CalcDateEditor::CalcDateEditor(const QStringList &colNames, int *colTypes, QWidget *parent)
  : PBDialog(tr("Calculation Node Editor"), parent)
{
    group = new QButtonGroup(this);
    QGridLayout *grid = Factory::gridLayout(vbox);
    QRadioButton *colButton = new QRadioButton(tr("Column"), this);
    grid->addWidget(colButton, 0, 0);
    group->addButton(colButton, 0);
    columnList = Factory::comboBox(this);
    columnList->setEditable(false);
    int count = colNames.count();
    int i;
    for (i = 0; i < count; i++) {
        int type = colTypes[i];
        if (type == DATE) {
            columnList->addItem(colNames[i]);
        }
    }
    grid->addWidget(columnList, 0, 1);

    QRadioButton *constantButton = new QRadioButton(tr("Constant"), this);
    grid->addWidget(constantButton, 1, 0);
    group->addButton(constantButton, 1);
    dateWidget = new DateWidget(this);
    grid->addWidget(dateWidget, 1, 1);

    if (columnList->count() > 0) {
        group->button(0)->setChecked(true);
    }
    else {
        // no date columns, so can't select one
        group->button(1)->setChecked(true);
        colButton->setEnabled(false);
    }

    finishLayout();
}

/**
 * Reset the dialog widgets to their default values.  Used when relaunching
 * it for a different node.
 */
void CalcDateEditor::reset()
{
    group->button(0)->setChecked(true);
    if (columnList->count() > 0) {
        columnList->setCurrentIndex(0);
    }
    QDate today = QDate::currentDate();
    dateWidget->setDate(today);
}

/**
 * Create a calculation node based on this dialog's current settings.
 *
 * @return The newly-created calculation node
 */
CalcNode *CalcDateEditor::createNode()
{
    CalcNode *node = new CalcNode(CalcNode::DateConstant, "");
    updateNode(node);
    return node;
}

/**
 * Update the specified node with the values currently specified in this
 * dialog.
 *
 * @param node The node to be updated
 */
void CalcDateEditor::updateNode(CalcNode *node)
{
    int selection = group->checkedId();
    if (selection == 0) {
        node->setType(CalcNode::DateColumn);
        node->setValue(columnList->currentText());
    }
    else {
        node->setType(CalcNode::DateConstant);
        node->setValue(QString::number(dateWidget->getDate()));
    }
}

/**
 * Populate this dialog's widgets with the values corresponding to the
 * provided calculation node.
 *
 * @param node The node whose settings are to be displayed
 */
void CalcDateEditor::setNode(CalcNode *node)
{
    reset();
    int type = node->type();
    QString value = node->value();
    if (type == CalcNode::DateConstant) {
        group->button(1)->setChecked(true);
        dateWidget->setDate(value.toInt());
    }
    else if (type == CalcNode::DateColumn) {
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
