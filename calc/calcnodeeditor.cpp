/*
 * calcnodeeditor.cpp
 *
 * (c) 2003-2004,2008-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file calcnodeeditor.cpp
 * Source file for CalcNodeEditor
 */

#include <QApplication>
#include <QButtonGroup>
#include <QComboBox>
#include <QListWidget>
#include <QMessageBox>
#include <QRadioButton>
#include "calcnode.h"
#include "calcnodeeditor.h"
#include "../datatypes.h"
#include "../factory.h"
#include "../formatting.h"
#include "../numberwidget.h"

/**
 * Constructor.
 *
 * @param colNames Ordered list of all the database's column names
 * @param colTypes Ordered list of all the database's column types
 * @param showOps True if the list of operations is to be shown, false otherwise
 * @param parent This dialog's parent widget
 */
CalcNodeEditor::CalcNodeEditor(const QStringList &colNames, int *colTypes, bool showOps, QWidget *parent)
  : PBDialog(tr("Calculation Node Editor"), parent)
{
    group = new QButtonGroup(this);
    QGridLayout *grid = Factory::gridLayout(vbox);
    QRadioButton *colButton = new QRadioButton(tr("Column"), this);
    group->addButton(colButton, 0);
    grid->addWidget(colButton, 0, 0);
    columnList = new QComboBox(this);
    grid->addWidget(columnList, 0, 1);
    int count = colNames.count();
    int i;
    for (i = 0; i < count; i++) {
        int t = colTypes[i];
        if (t == INTEGER || t == FLOAT || t == SEQUENCE || t == BOOLEAN) {
            columnList->addItem(colNames[i]);
        }
    }
    connect(columnList, SIGNAL(activated(int)),
            this, SLOT(columnSelected(int)));

    QRadioButton *constantButton = new QRadioButton(tr("Constant"), this);
    group->addButton(constantButton, 1);
    grid->addWidget(constantButton, 1, 0);
    number = new NumberWidget(FLOAT, this);
    grid->addWidget(number, 1, 1);

    if (showOps) {
        QRadioButton *opButton = new QRadioButton(tr("Operation"), this);
        group->addButton(opButton, 2);
        grid->addWidget(opButton, 2, 0, Qt::AlignLeft | Qt::AlignTop);
        opList = Factory::listWidget(this);
        grid->addWidget(opList, 2, 1);
        opList->addItems(CalcNode::listOperations());
        connect(opList, SIGNAL(itemSelectionChanged()),
                this, SLOT(operationSelected()));
    }
    else {
        opList = 0;
    }
    if (columnList->count() > 0) {
        group->button(0)->setChecked(true);
    }
    else {
        // no numeric columns, so can't select one
        group->button(1)->setChecked(true);
        colButton->setEnabled(false);
    }

    int minHeight = showOps ? parent->height() : -1;
    finishLayout(parent->width() / 2, minHeight);
}

/**
 * Reset the dialog's entry widgets to their default values.  Typically used
 * when relaunching it for a different node.
 */
void CalcNodeEditor::reset()
{
    if (columnList->count() > 0) {
        columnList->setCurrentIndex(0);
    }
    number->setValue("0");
    if (opList != 0) {
        opList->setCurrentRow(0);
    }
    group->button(0)->setChecked(true);
}

/**
 * Create a new calculation node and set it's properties to match those
 * currently selected in this dialog.
 *
 * @return The newly created and configured node
 */
CalcNode *CalcNodeEditor::createNode()
{
    CalcNode *node = new CalcNode(CalcNode::Constant, "");
    updateNode(node);
    return node;
}

/**
 * Set the properties of the provided node to match those currently
 * selected in this dialog.
 *
 * @param node The node to be updated
 */
void CalcNodeEditor::updateNode(CalcNode *node)
{
    int selection = group->checkedId();
    if (selection == 0) {
        node->setType(CalcNode::Column);
        node->setValue(columnList->currentText());
    }
    else if (selection == 1) {
        node->setType(CalcNode::Constant);
        node->setValue(number->getValue());
    }
    else {
        node->setType((CalcNode::NodeType)(opList->currentRow() + CALC_FIRST_OP));
        node->setValue("");
    }
}

/**
 * Update this dialog's properties to match those of the provided node.
 *
 * @param node The node to be reflected by the dialog
 */
void CalcNodeEditor::setNode(CalcNode *node)
{
    reset();
    CalcNode::NodeType type = node->type();
    QString value = node->value();
    if (type == CalcNode::Constant) {
        group->button(1)->setChecked(true);
        number->setValue(value);
    }
    else if (type == CalcNode::Column) {
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
bool CalcNodeEditor::isValid()
{
    if (group->checkedId() != 1) {
        return true;
    }
    bool ok = false;
    Formatting::parseDouble(number->getValue(), &ok);
    if (!ok) {
        QMessageBox::warning(this, qApp->applicationName(),
                             tr("Constant must be a decimal value"));
    }
    return ok;
}

/**
 * Called when a column is selected from the combo box, in order to
 * automatically check the appropriate radio button.
 */
void CalcNodeEditor::columnSelected(int)
{
    group->button(0)->setChecked(true);
}

/**
 * Called when an operation is selected from the list, in order to
 * automatically check the appropriate radio button.
 */
void CalcNodeEditor::operationSelected()
{
    group->button(2)->setChecked(true);
}
