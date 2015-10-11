/*
 * calceditor.cpp
 *
 * (c) 2003-2004,2008-2010,2015 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file calceditor.cpp
 * Source file for CalcEditor
 */

#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTreeWidget>
#include "calcdateeditor.h"
#include "calceditor.h"
#include "calcnode.h"
#include "calcnodeeditor.h"
#include "calctimeeditor.h"
#include "../factory.h"

/**
 * Constructor.
 *
 * @param dbase The database being edited
 * @param calcName The initial name of the calculated column
 * @param colNames The list of database column names
 * @param colTypes The list of database column type IDs
 * @param parent This dialog's parent widget
 */
CalcEditor::CalcEditor(Database *dbase, const QString &calcName, const QStringList &colNames, int *colTypes, QWidget *parent)
    : PBDialog(tr("Calculation Editor"), parent), db(dbase)
{
    QGridLayout *grid = Factory::gridLayout(vbox);
    grid->addWidget(new QLabel(tr("Column Name") + ": ", this), 0, 0);
    grid->addWidget(new QLabel(calcName, this), 0, 1);
    grid->addWidget(new QLabel(tr("Equation") + ":", this), 1, 0);
    equation = new QLineEdit(this);
    equation->setReadOnly(true);
    equation->setFrame(false);
    grid->addWidget(equation, 1, 1);
    grid->addWidget(new QLabel(tr("Decimal Places") + ":", this), 2, 0);
    decimalsBox = new QSpinBox(this);
    decimalsBox->setRange(0, 9);
    decimalsBox->setValue(2);
    grid->addWidget(decimalsBox, 2, 1);
    tree = Factory::treeWidget(this, QStringList());
    tree->setRootIsDecorated(true);
    tree->header()->hide();
    vbox->addWidget(tree);
    connect(tree, SIGNAL(itemSelectionChanged()),
            this, SLOT(updateButtons()));

    addEditButtons();
    connect(addButton, SIGNAL(clicked()), this, SLOT(addNode()));
    connect(editButton, SIGNAL(clicked()), this, SLOT(editNode()));
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteNode()));
    connect(upButton, SIGNAL(clicked()), this, SLOT(moveUp()));
    connect(downButton, SIGNAL(clicked()), this, SLOT(moveDown()));
    updateButtons();

    nodeEditor = new CalcNodeEditor(colNames, colTypes, true, this);
    valueEditor = new CalcNodeEditor(colNames, colTypes, false, this);
    dateEditor = new CalcDateEditor(colNames, colTypes, this);
    timeEditor = new CalcTimeEditor(db, colNames, colTypes, this);

    finishLayout(parent->width(), 400);
}

/**
 * Populate the dialog with the specified information.
 *
 * @param root The root node of the calculation to display
 * @param decimals The number of decimal places to show for calculated values
 */
void CalcEditor::load(CalcNode *root, int decimals)
{
    addNode(0, root);
    QTreeWidgetItem *item = tree->topLevelItem(0);
    tree->setCurrentItem(item);
    updateButtons();
    updateEquation();
    decimalsBox->setValue(decimals);
}

/**
 * Get the root node of the displayed calculation.
 *
 * @return The calculation's root node
 */
CalcNode *CalcEditor::getRootNode()
{
    QTreeWidgetItem *root = tree->topLevelItem(0);
    if (root == 0) {
        return 0;
    }
    return nodeMap[root];
}

/**
 * Get the number of decimal places to display for calculation results.
 *
 * @return The currently specified number of decimal places to show.
 */
int CalcEditor::getDecimals()
{
    return decimalsBox->value();
}

/**
 * Add a new node to the equation as a child of the specified entry in
 * the calculation definition tree.
 *
 * @param parent The displayed representation of the new node's parent
 * @param node The node to be added to the calculation definition tree
 */
void CalcEditor::addNode(QTreeWidgetItem *parent, CalcNode *node)
{
    QTreeWidgetItem *item = 0;
    QString text = node->description();
    if (parent == 0) {
        item = new QTreeWidgetItem(tree);
    }
    else {
        if (parent->childCount() == 0) {
            item = new QTreeWidgetItem(parent);
        }
        else {
            QTreeWidgetItem *sibling = parent->child(parent->childCount() - 1);
            item = new QTreeWidgetItem(parent, sibling);
        }
        parent->setExpanded(true);
    }
    item->setText(0, text);
    nodeMap.insert(item, node);
    CalcNodeList children = node->getChildren();
    int count = children.count();
    for (int i = 0; i < count; i++) {
        addNode(item, children[i]);
    }
}

/**
 * Update the status of the edit buttons based on the currently selected node
 * (or lack thereof).
 */
void CalcEditor::updateButtons()
{
    QTreeWidgetItem *item = tree->currentItem();
    if (item == 0) {
        // empty tree
        addButton->setEnabled(true);
        editButton->setEnabled(false);
        deleteButton->setEnabled(false);
        upButton->setEnabled(false);
        downButton->setEnabled(false);
        return;
    }
    CalcNode *node = nodeMap[item];
    addButton->setEnabled(node->allowsAdd());
    editButton->setEnabled(node->allowsEdit());
    deleteButton->setEnabled(true);
    QTreeWidgetItem *parent = item->parent();
    if (parent == 0) {
        // root node, no siblings
        upButton->setEnabled(false);
        downButton->setEnabled(false);
    }
    else {
        upButton->setEnabled(parent->child(0) != item);
        int index = parent->indexOfChild(item);
        downButton->setEnabled(parent->childCount() > index + 1);
    }
}

/**
 * Update the displayed text representation of the calculation.
 */
void CalcEditor::updateEquation()
{
     CalcNode *root = getRootNode();
     QString eq = (root == 0) ? QString("") : root->equation();
     equation->setText(eq);
}

/**
 * Add a new node to the calculation (as a child of the selected node, if
 * any).  Called when the "Add" button is clicked.
 */
void CalcEditor::addNode()
{
    QTreeWidgetItem *parent = tree->currentItem();
    if (parent != 0) {
        CalcNode *parentNode = nodeMap[parent];
        int type = parentNode->type();
        if (type == CalcNode::Days) {
            dateEditor->reset();
            if (!dateEditor->exec()) {
                return;
            }
            CalcNode *node = dateEditor->createNode();
            addNode(parent, node);
            parentNode->addChild(node);
            updateButtons();
            updateEquation();
            return;
        }
        else if (type == CalcNode::Seconds || type == CalcNode::Minutes
                 || type == CalcNode::Hours) {
            timeEditor->reset();
            bool finished = false;
            while (!finished) {
                if (!timeEditor->exec()) {
                    return;
                }
                else {
                    finished = timeEditor->isValid();
                }
            }
            CalcNode *node = timeEditor->createNode();
            addNode(parent, node);
            parentNode->addChild(node);
            updateButtons();
            updateEquation();
            return;
        }
    }
    nodeEditor->reset();
    bool finished = false;
    while (!finished) {
        if (!nodeEditor->exec()) {
            return;
        }
        else {
            finished = nodeEditor->isValid();
        }
    }
    CalcNode *node = nodeEditor->createNode();
    addNode(parent, node);
    QTreeWidgetItem *selection = parent;
    if (parent == 0) {
        selection = tree->topLevelItem(0);
        tree->setCurrentItem(selection);
    }
    else {
        CalcNode *parentNode = nodeMap[parent];
        parentNode->addChild(node);
    }
    updateButtons();
    updateEquation();
}

/**
 * Edit the currently selected calculation node.  Called when the "Edit"
 * button is clicked.
 */
void CalcEditor::editNode()
{
    QTreeWidgetItem *item = tree->currentItem();
    if (item == 0) {
        return;
    }
    CalcNode *node = nodeMap[item];
    int type = node->type();
    if (type == CalcNode::Column || type == CalcNode::Constant) {
        valueEditor->setNode(node);
        bool finished = false;
        while (!finished) {
            if (!valueEditor->exec()) {
                return;
            }
            else {
                finished = valueEditor->isValid();
            }
        }
        valueEditor->updateNode(node);
    }
    else if (type == CalcNode::TimeColumn || type == CalcNode::TimeConstant) {
        timeEditor->setNode(node);
        bool finished = false;
        while (!finished) {
            if (!timeEditor->exec()) {
                return;
            }
            else {
                finished = timeEditor->isValid();
            }
        }
        timeEditor->updateNode(node);
    }
    else {
        dateEditor->setNode(node);
        if (!dateEditor->exec()) {
            return;
        }
        dateEditor->updateNode(node);
    }
    item->setText(0, node->description());
    updateEquation();
}

/**
 * Delete the selected calculation node.  Called when the "Delete" button is
 * clicked.
 */
void CalcEditor::deleteNode()
{
    QTreeWidgetItem *item = tree->currentItem();
    if (item == 0) {
        return;
    }
    CalcNode *node = nodeMap[item];
    QTreeWidgetItem *parent = item->parent();
    if (parent != 0) {
        CalcNode *parentNode = nodeMap[parent];
        tree->setCurrentItem(parent);
        parentNode->removeChild(node);
    }
    else {
        delete node;
    }
    removeFromMap(item);
    delete item;
    updateButtons();
    updateEquation();
}

/**
 * Remove an entry from the display tree item to calculation node mapping.
 * All of the item's descendents are removed as well.
 *
 * @param item The display tree item to be removed from the mapping
 */
void CalcEditor::removeFromMap(QTreeWidgetItem *item)
{
    nodeMap.remove(item);
    int childCount = item->childCount();
    for (int i = 0; i < childCount; i++) {
        removeFromMap(item->child(i));
    }
}

/**
 * Move the position of the selected calculation node up by one among its
 * siblings.
 */
void CalcEditor::moveUp()
{
    QTreeWidgetItem *item = tree->currentItem();
    if (item == 0) {
        return;
    }
    QTreeWidgetItem *parent = item->parent();
    if (parent == 0) {
        return;
    }
    int index = parent->indexOfChild(item);
    if (index > 0) {
        QTreeWidgetItem *before = parent->child(index - 1);
        swap(before);
    }
    updateButtons();
    updateEquation();
}

/**
 * Move the position of the selected calculation node down by one among its
 * siblings.
 */
void CalcEditor::moveDown()
{
    QTreeWidgetItem *item = tree->currentItem();
    if (item == 0) {
        return;
    }
    QTreeWidgetItem *parent = item->parent();
    int index = parent->indexOfChild(item);
    if (index + 1 < parent->childCount()) {
        swap(item);
    }
    updateButtons();
    updateEquation();
}

/**
 * Swap the positions of two adjacent calculation nodes in the display tree
 * (and in the calculation definition that it represents).
 *
 * @param top The higher of the two nodes to be swapped
 */
void CalcEditor::swap(QTreeWidgetItem *top)
{
    CalcNode *parentNode = nodeMap[top->parent()];
    QTreeWidgetItem *parent = top->parent();
    int index = parent->indexOfChild(top);
    QTreeWidgetItem *removed = parent->takeChild(index);
    parent->insertChild(index + 1, removed);
    parentNode->swapChildren(index, index + 1);
}
