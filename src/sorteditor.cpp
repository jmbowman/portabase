/*
 * sorteditor.cpp
 *
 * (c) 2002-2004,2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file sorteditor.cpp
 * Source file for SortEditor
 */

#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include "database.h"
#include "factory.h"
#include "portabase.h"
#include "sorteditor.h"

/**
 * Constructor.
 *
 * @param parent This dialog's parent widget
 */
SortEditor::SortEditor(QWidget *parent)
    : PBDialog(tr("Sorting Editor"), parent), db(0), updating(false)
{
    QHBoxLayout *hbox = Factory::hBoxLayout(vbox);
    hbox->addWidget(new QLabel(tr("Sorting Name") + " ", this));
    nameBox = new QLineEdit(this);
    hbox->addWidget(nameBox);

    QStringList headers;
    headers << tr("Sort") << tr("Column Name") << tr("Direction");
    table = Factory::treeWidget(this, headers);
    vbox->addWidget(table);
    connect(table, SIGNAL(itemClicked(QTreeWidgetItem*, int)),
            this, SLOT(tableClicked(QTreeWidgetItem*, int)));
    connect(table->model(), SIGNAL(dataChanged(QModelIndex, QModelIndex)),
            this, SLOT(dataChanged(QModelIndex, QModelIndex)));

    addEditButtons(true);
    connect(upButton, SIGNAL(clicked()), this, SLOT(moveUp()));
    connect(downButton, SIGNAL(clicked()), this, SLOT(moveDown()));

    finishLayout();
    nameBox->setFocus();
}

/**
 * Launch this dialog to edit the specified sorting.
 *
 * @param subject The database being edited
 * @param sortingName The current name of the sorting to edit
 */
int SortEditor::edit(Database *subject, const QString &sortingName)
{
    updating = true;
    db = subject;
    originalName = sortingName;
    nameBox->setText(sortingName);
    QStringList colNames = db->listColumns();
    db->getSortingInfo(sortingName, &sortCols, &descCols);
    // move currently sorted columns to the top of the list, in correct order
    int count = sortCols.count();
    for (int i = count - 1; i > -1; i--) {
        QString name = sortCols[i];
        colNames.removeAll(name);
        colNames.prepend(name);
    }
    updateTable(colNames);
    updating = false;
    int result = exec();
    while (result) {
        if (validateName(nameBox->text(), originalName, db->listSortings())) {
            break;
        }
        else {
            result = exec();
        }
    }
    return result;
}

/**
 * Get the name for the sorting as currently shown in the dialog.
 *
 * @return The (possibly changed) sorting name
 */
QString SortEditor::getName()
{
    return nameBox->text();
}

/**
 * Move the selected row higher in the table, which has the effect of sorting
 * on the field it represents before those of any other rows it moves above.
 * Called when the "Up" button is clicked.
 */
void SortEditor::moveUp()
{
    QTreeWidgetItem *item = table->currentItem();
    if (item == 0) {
        return;
    }
    int index = table->indexOfTopLevelItem(item);
    if (index > 0) {
        updating = true;
        item = table->takeTopLevelItem(index);
        table->insertTopLevelItem(index - 1, item);
        table->setCurrentItem(item);
        updating = false;
    }
}

/**
 * Move the selected row lower in the table, which has the effect of sorting
 * on the field it represents after those of any other rows it moves below.
 * Called when the "Down" button is clicked.
 */
void SortEditor::moveDown()
{
    QTreeWidgetItem *item = table->currentItem();
    if (item == 0) {
        return;
    }
    int index = table->indexOfTopLevelItem(item);
    if (index < table->topLevelItemCount() - 1) {
        updating = true;
        item = table->takeTopLevelItem(index);
        table->insertTopLevelItem(index + 1, item);
        table->setCurrentItem(item);
        updating = false;
    }
}

/**
 * Update the displayed table to match the current information in sortCols
 * and descCols.
 *
 * @param colNames The field names in the order in which they are appear in
 *                 the table
 */
void SortEditor::updateTable(const QStringList &colNames)
{
    table->clear();
    int count = colNames.count();
    QTreeWidgetItem *item = 0;
    for (int i = 0; i < count; i++) {
        if (i == 0) {
            item = new QTreeWidgetItem(table);
        }
        else {
            item = new QTreeWidgetItem(table, item);
        }
        item->setTextAlignment(0, Qt::AlignHCenter);
        QString name = colNames[i];
        QString direction = "";
        if (sortCols.contains(name)) {
            if (descCols.contains(name)) {
                direction = tr("Descending");
            }
            else {
                direction = tr("Ascending");
            }
            item->setCheckState(0, Qt::Checked);
        }
        else {
            item->setCheckState(0, Qt::Unchecked);
        }
        item->setText(1, name);
        item->setText(2, direction);
    }
}

/**
 * Handler for clicks on the information table.  A click in the third column
 * toggles the sorting order for the appropriate field (if it's being sorted
 * on).  Clicks in the first column trigger a data change in the model and
 * are handled by dataChanged() instead.
 *
 * @param item The row of the table in which the click occurred
 * @param column The index of the column in which the click occurred
 */
void SortEditor::tableClicked(QTreeWidgetItem *item, int column)
{
    if (item == 0) {
        // no row selected
        return;
    }
    if (column == 2) {
        QString name = item->text(1);
        if (sortCols.contains(name)) {
            if (!descCols.contains(name)) {
                descCols.append(name);
                item->setText(2, tr("Descending"));
            }
            else {
                descCols.removeAll(name);
                item->setText(2, tr("Ascending"));
            }
        }
    }
}

/**
 * Handler for information table model data changes.  Used to detect when the
 * user clicks a field selection checkbox (because the kinetic scrolling
 * on Maemo Fremantle sometimes prevents the itemClicked() signal from
 * being emitted when the checkbox is toggled).
 *
 * @param topLeft The top left position of the changed data region
 * @param bottomRight The bottom right position of the changed data region
 */
void SortEditor::dataChanged(const QModelIndex &topLeft, const QModelIndex &)
{
    if (updating || topLeft.column() != 0) {
        return;
    }
    QTreeWidgetItem *item = table->topLevelItem(topLeft.row());
    Qt::CheckState state = item->checkState(0);
    QString name = item->text(1);
    if (state == Qt::Unchecked && sortCols.contains(name)) {
        sortCols.removeAll(name);
        descCols.removeAll(name);
        item->setText(2, "");
    }
    else if (state == Qt::Checked && !sortCols.contains(name)) {
        sortCols.append(name);
        item->setText(2, tr("Ascending"));
    }
}

/**
 * Apply to the database any changes made the last time this dialog was shown.
 */
void SortEditor::applyChanges()
{
    QString sortingName = nameBox->text();
    QStringList orderedSort;
    QStringList orderedDesc;
    int count = table->topLevelItemCount();
    for (int i = 0; i < count; i++) {
        QString name = table->topLevelItem(i)->text(1);
        if (sortCols.contains(name)) {
            orderedSort.append(name);
            if (descCols.contains(name)) {
                orderedDesc.append(name);
            }
        }
    }
    db->deleteSorting(originalName);
    db->addSorting(sortingName, orderedSort, orderedDesc);
}
