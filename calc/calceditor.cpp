/*
 * calceditor.cpp
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#if defined(DESKTOP)
#include "../desktop/resource.h"
#endif

#include <qgrid.h>
#include <qhbox.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qvbox.h>
#include "calcdateeditor.h"
#include "calceditor.h"
#include "calcnode.h"
#include "calcnodeeditor.h"

CalcEditor::CalcEditor(Database *dbase, const QString &calcName, const QStringList &colNames, int *colTypes, QWidget *parent, const char *name, WFlags f)
    : QDialog(parent, name, TRUE, f), db(dbase)
{
    setCaption(tr("Calculation Editor") + " - " + tr("PortaBase"));
    vbox = new QVBox(this);
#if defined(Q_WS_WIN)
    setSizeGripEnabled(TRUE);
    new QLabel("<center><b>" + tr("Calculation Editor") + "</b></center>",
               vbox);
#endif
    QGrid *grid = new QGrid(2, vbox);
    new QLabel(tr("Column Name") + ": ", grid);
    new QLabel(calcName, grid);
    new QLabel(tr("Equation") + ":", grid);
    equation = new QLineEdit(grid);
    new QLabel(tr("Decimal Places") + ":", grid);
    decimalsBox = new QSpinBox(0, 9, 1, grid);
    equation->setReadOnly(TRUE);
    tree = new QListView(vbox);
    tree->setSorting(-1);
    tree->addColumn("", vbox->width());
    tree->setColumnWidthMode(0, QListView::Manual);
    tree->header()->hide();
    connect(tree, SIGNAL(currentChanged(QListViewItem*)),
            this, SLOT(updateButtons(QListViewItem*)));

    QHBox *hbox = new QHBox(vbox);
    addButton = new QPushButton(tr("Add"), hbox);
    connect(addButton, SIGNAL(clicked()), this, SLOT(addNode()));
    editButton = new QPushButton(tr("Edit"), hbox);
    connect(editButton, SIGNAL(clicked()), this, SLOT(editNode()));
    deleteButton = new QPushButton(tr("Delete"), hbox);
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteNode()));
    upButton = new QPushButton(tr("Up"), hbox);
    connect(upButton, SIGNAL(clicked()), this, SLOT(moveUp()));
    downButton = new QPushButton(tr("Down"), hbox);
    connect(downButton, SIGNAL(clicked()), this, SLOT(moveDown()));
    updateButtons(0);

    nodeEditor = new CalcNodeEditor(colNames, colTypes, TRUE, this);
    valueEditor = new CalcNodeEditor(colNames, colTypes, FALSE, this);
    dateEditor = new CalcDateEditor(colNames, colTypes, this);

#if defined(DESKTOP)
    new QLabel(" ", vbox);
    hbox = new QHBox(vbox);
    new QWidget(hbox);
    QPushButton *okButton = new QPushButton(tr("OK"), hbox);
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    new QWidget(hbox);
    QPushButton *cancelButton = new QPushButton(tr("Cancel"), hbox);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    new QWidget(hbox);
    setIcon(Resource::loadPixmap("portabase"));
    setMinimumWidth(parent->width());
    setMinimumHeight(400);
#else
    showMaximized();
#endif
}

CalcEditor::~CalcEditor()
{

}

void CalcEditor::load(CalcNode *root, int decimals)
{
    addNode(0, root);
    if (root != 0) {
        tree->setSelected(tree->firstChild(), TRUE);
    }
    updateEquation();
    decimalsBox->setValue(decimals);
}

CalcNode *CalcEditor::getRootNode()
{
    QListViewItem *root = tree->firstChild();
    if (root == 0) {
        return 0;
    }
    return nodeMap[root];
}

int CalcEditor::getDecimals()
{
    return decimalsBox->value();
}

void CalcEditor::addNode(QListViewItem *parent, CalcNode *node)
{
    QListViewItem *item = 0;
    QString text = node->description(db);
    if (parent == 0) {
        item = new QListViewItem(tree, text);
    }
    else {
        QListViewItem *sibling = parent->firstChild();
        if (sibling == 0) {
            item = new QListViewItem(parent, text);
        }
        else {
            while (sibling->nextSibling() != 0) {
                sibling = sibling->nextSibling();
            }
            item = new QListViewItem(parent, sibling, text);
        }
        tree->setOpen(parent, TRUE);
    }
    nodeMap.insert(item, node);
    CalcNodeList children = node->getChildren();
    int count = children.count();
    for (int i = 0; i < count; i++) {
        addNode(item, children[i]);
    }
}

void CalcEditor::updateButtons(QListViewItem *item)
{
    if (item == 0) {
        // empty tree
        addButton->setEnabled(TRUE);
        editButton->setEnabled(FALSE);
        deleteButton->setEnabled(FALSE);
        upButton->setEnabled(FALSE);
        downButton->setEnabled(FALSE);
        return;
    }
    CalcNode *node = nodeMap[item];
    addButton->setEnabled(node->allowsAdd());
    editButton->setEnabled(node->allowsEdit());
    deleteButton->setEnabled(TRUE);
    QListViewItem *parent = item->parent();
    if (parent == 0) {
        // root node, no siblings
        upButton->setEnabled(FALSE);
        downButton->setEnabled(FALSE);
    }
    else {
        upButton->setEnabled(parent->firstChild() != item);
        downButton->setEnabled(item->nextSibling() != 0);
    }
}

void CalcEditor::updateEquation()
{
     CalcNode *root = getRootNode();
     QString eq = (root == 0) ? "" : root->equation(db);
     equation->setText(eq);
}

void CalcEditor::addNode()
{
    QListViewItem *parent = tree->selectedItem();
    if (parent != 0) {
        CalcNode *parentNode = nodeMap[parent];
        if (parentNode->type() == CALC_DAYS) {
            dateEditor->reset();
            if (!dateEditor->exec()) {
                return;
            }
            CalcNode *node = dateEditor->createNode();
            addNode(parent, node);
            parentNode->addChild(node);
            return;
        }
    }
    nodeEditor->reset();
    bool finished = FALSE;
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
    QListViewItem *selection = parent;
    if (parent == 0) {
        selection = tree->firstChild();
        tree->setSelected(selection, TRUE);
    }
    else {
        CalcNode *parentNode = nodeMap[parent];
        parentNode->addChild(node);
    }
    updateButtons(selection);
    updateEquation();
}

void CalcEditor::editNode()
{
    QListViewItem *item = tree->selectedItem();
    if (item == 0) {
        return;
    }
    CalcNode *node = nodeMap[item];
    int type = node->type();
    if (type == CALC_COLUMN || type == CALC_CONSTANT) {
        valueEditor->setNode(node);
        bool finished = FALSE;
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
    else {
        dateEditor->setNode(node);
        if (!dateEditor->exec()) {
            return;
        }
        dateEditor->updateNode(node);
    }
    item->setText(0, node->description(db));
    updateEquation();
}

void CalcEditor::deleteNode()
{
    QListViewItem *item = tree->selectedItem();
    if (item == 0) {
        return;
    }
    CalcNode *node = nodeMap[item];
    QListViewItem *parent = item->parent();
    if (parent != 0) {
        CalcNode *parentNode = nodeMap[parent];
        tree->setSelected(parent, TRUE);
        parentNode->removeChild(node);
    }
    else {
        delete node;
    }
    removeFromMap(item);
    delete item;
    updateButtons(parent);
    updateEquation();
}

void CalcEditor::removeFromMap(QListViewItem *item)
{
    nodeMap.remove(item);
    QListViewItem *child = item->firstChild();
    while (child != 0) {
        removeFromMap(child);
        child = child->nextSibling();
    }
}

void CalcEditor::moveUp()
{
    QListViewItem *item = tree->selectedItem();
    if (item == 0) {
        return;
    }
    QListViewItem *parent = item->parent();
    if (parent == 0) {
        return;
    }
    QListViewItem *sibling = parent->firstChild();
    QListViewItem *before = 0;
    while (sibling != item && sibling != 0) {
        before = sibling;
        sibling = sibling->nextSibling();
    }
    if (before != 0) {
        swap(before, item);
    }
    updateButtons(item);
    updateEquation();
}

void CalcEditor::moveDown()
{
    QListViewItem *item = tree->selectedItem();
    if (item == 0) {
        return;
    }
    QListViewItem *after = item->nextSibling();
    if (after != 0) {
        swap(item, after);
    }
    updateButtons(item);
    updateEquation();
}

void CalcEditor::swap(QListViewItem *top, QListViewItem *bottom)
{
    CalcNode *parentNode = nodeMap[top->parent()];
    CalcNode *topNode = nodeMap[top];
    CalcNode *bottomNode = nodeMap[bottom];
    top->moveItem(bottom);
    parentNode->moveChild(topNode, bottomNode);
}

void CalcEditor::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    vbox->resize(size());
    tree->setColumnWidth(0, width() - 2);
}
