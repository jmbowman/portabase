/*
 * calcnodeeditor.cpp
 *
 * (c) 2003-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qgrid.h>
#include <qlistbox.h>
#include <qmessagebox.h>
#include <qradiobutton.h>
#include <qvbox.h>
#include "calcnode.h"
#include "calcnodeeditor.h"
#include "../datatypes.h"
#include "../numberwidget.h"

CalcNodeEditor::CalcNodeEditor(const QStringList &colNames, int *colTypes, bool showOps, QWidget *parent, const char *name)
  : PBDialog(tr("Calculation Node Editor"), parent, name)
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
        if (type == INTEGER || type == FLOAT || type == SEQUENCE) {
            columnList->insertItem(colNames[i]);
        }
    }
    connect(columnList, SIGNAL(activated(int)),
            this, SLOT(columnSelected(int)));

    group->insert(new QRadioButton(tr("Constant"), grid), 1);
    number = new NumberWidget(FLOAT, grid);

    if (showOps) {
        QVBox *panel = new QVBox(grid);
        group->insert(new QRadioButton(tr("Operation"), panel), 2);
        new QWidget(panel);
        opList = new QListBox(grid);
        opList->insertStringList(CalcNode::listOperations());
        connect(opList, SIGNAL(selectionChanged()),
                this, SLOT(operationSelected()));
    }
    else {
        opList = 0;
    }
    if (columnList->count() > 0) {
        group->setButton(0);
    }
    else {
        // no numeric columns, so can't select one
        group->setButton(1);
        colButton->setEnabled(FALSE);
    }

    int minHeight = showOps ? parent->height() : -1;
    finishLayout(TRUE, TRUE, FALSE, parent->width() / 2, minHeight);
}

CalcNodeEditor::~CalcNodeEditor()
{

}

void CalcNodeEditor::reset()
{
    if (columnList->count() > 0) {
        columnList->setCurrentItem(0);
    }
    number->setValue("0");
    if (opList != 0) {
        opList->setCurrentItem(0);
    }
    group->setButton(0);
}

CalcNode *CalcNodeEditor::createNode()
{
    CalcNode *node = new CalcNode(0, "");
    updateNode(node);
    return node;
}

void CalcNodeEditor::updateNode(CalcNode *node)
{
    int selection = group->id(group->selected());
    if (selection == 0) {
        node->setType(CALC_COLUMN);
        node->setValue(columnList->currentText());
    }
    else if (selection == 1) {
        node->setType(CALC_CONSTANT);
        node->setValue(number->getValue());
    }
    else {
        node->setType(opList->currentItem() + CALC_FIRST_OP);
        node->setValue("");
    }
}

void CalcNodeEditor::setNode(CalcNode *node)
{
    reset();
    int type = node->type();
    QString value = node->value();
    if (type == CALC_CONSTANT) {
        group->setButton(1);
        number->setValue(value);
    }
    else if (type == CALC_COLUMN) {
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

bool CalcNodeEditor::isValid()
{
    if (group->id(group->selected()) != 1) {
        return TRUE;
    }
    bool ok = FALSE;
    number->getValue().toDouble(&ok);
    if (!ok) {
        QMessageBox::warning(this, QQDialog::tr("PortaBase"),
                             tr("Constant must be a decimal value"));
    }
    return ok;
}

void CalcNodeEditor::columnSelected(int)
{
    group->setButton(0);
}

void CalcNodeEditor::operationSelected()
{
    group->setButton(2);
}
