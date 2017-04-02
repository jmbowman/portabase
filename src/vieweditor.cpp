/*
 * vieweditor.cpp
 *
 * (c) 2002-2004,2010,2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file vieweditor.cpp
 * Source file for ViewEditor
 */

#include <QAction>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include "database.h"
#include "factory.h"
#include "qqutil/qqlineedit.h"
#include "vieweditor.h"

/**
 * Constructor.
 *
 * @param parent This dialog's parent widget
 */
ViewEditor::ViewEditor(QWidget *parent)
    : PBDialog(tr("View Editor"), parent), db(0)
{
    QHBoxLayout *hbox = Factory::hBoxLayout(vbox);
    hbox->addWidget(new QLabel(tr("View Name") + " ", this));
    nameBox = new QQLineEdit(this);
    hbox->addWidget(nameBox);

#ifdef MOBILE
    hbox = Factory::hBoxLayout(vbox);
    sortingBox = Factory::comboBox(this);
    hbox->addWidget(sortingBox);
    filterBox = Factory::comboBox(this);
    hbox->addWidget(filterBox);
#else
    hbox = Factory::hBoxLayout(vbox);
    hbox->addWidget(new QLabel(tr("Default Sorting") + " ", this));
    sortingBox = Factory::comboBox(this);
    hbox->addWidget(sortingBox, 1);

    hbox = Factory::hBoxLayout(vbox);
    hbox->addWidget(new QLabel(tr("Default Filter") + " ", this));
    filterBox = Factory::comboBox(this);
    hbox->addWidget(filterBox, 1);
#endif

    QStringList headers;
    headers << tr("Include") << tr("Column Name");
    table = Factory::treeWidget(this, headers);
    vbox->addWidget(table);

    addEditButtons(true);
    connect(upAction, SIGNAL(triggered()), this, SLOT(moveUp()));
    connect(downAction, SIGNAL(triggered()), this, SLOT(moveDown()));

    finishLayout();
    nameBox->setFocus();
}

/**
 * Launch this dialog to edit the view with the specified attributes.
 *
 * @param subject The database being edited
 * @param viewName The initial name of the view to edit
 * @param currentCols Ordered list of the columns included in the view
 * @param defaultSort Name of the view's default sorting (may be "_none")
 * @param defaultFilter Name of the view's default filter (may be "_none")
 */
int ViewEditor::edit(Database *subject, const QString &viewName,
                     const QStringList &currentCols, const QString &defaultSort,
                     const QString &defaultFilter)
{
    db = subject;
    originalName = viewName;
    oldNames = currentCols;
    nameBox->setText(viewName);

    sortingBox->clear();
    sortingBox->addItem(tr("No default sorting"));
    QStringList sortings = db->listSortings();
    sortings.removeAll("_single");
    sortingBox->addItems(sortings);
    int count = sortingBox->count();
    int i;
    if (defaultSort != "_none") {
        for (i = 1; i < count; i++) {
            if (sortingBox->itemText(i) == defaultSort) {
                sortingBox->setCurrentIndex(i);
                break;
            }
        }
    }

    filterBox->clear();
    filterBox->addItem(tr("No default filter"));
    filterBox->addItem(tr("All Rows"));
    QStringList filters = db->listFilters();
    filters.removeAll("_simple");
    filters.removeAll("_allrows");
    filterBox->addItems(filters);
    if (defaultFilter == "_allrows") {
        filterBox->setCurrentIndex(1);
    }
    else if (defaultFilter != "_none") {
        count = filterBox->count();
        for (i = 2; i < count; i++) {
            if (filterBox->itemText(i) == defaultFilter) {
                filterBox->setCurrentIndex(i);
                break;
            }
        }
    }

    updateTable();
    int result = exec();
    while (result) {
        if (validateName(nameBox->text(), originalName, db->listViews())) {
            break;
        }
        else {
            result = exec();
        }
    }
    return result;
}

/**
 * Get the name for the view as currently shown in the dialog.
 *
 * @return The (possibly changed) view name
 */
QString ViewEditor::getName()
{
    return nameBox->text();
}

/**
 * Move the selected row higher in the table, which has the effect of moving
 * the field it represents one position earlier in the view.
 */
void ViewEditor::moveUp()
{
    QTreeWidgetItem *item = table->currentItem();
    if (item == 0) {
        return;
    }
    int index = table->indexOfTopLevelItem(item);
    if (index > 0) {
        item = table->takeTopLevelItem(index);
        table->insertTopLevelItem(index - 1, item);
        table->setCurrentItem(item);
    }
}

/**
 * Move the selected row lower in the table, which has the effect of moving
 * the field it represents one position later in the view.
 */
void ViewEditor::moveDown()
{
    QTreeWidgetItem *item = table->currentItem();
    if (item == 0) {
        return;
    }
    int index = table->indexOfTopLevelItem(item);
    if (index < table->topLevelItemCount() - 1) {
        item = table->takeTopLevelItem(index);
        table->insertTopLevelItem(index + 1, item);
        table->setCurrentItem(item);
    }
}

/**
 * Update the display table to reflect the settings of the view which has
 * just begun to be edited.
 */
void ViewEditor::updateTable()
{
    table->clear();
    QStringList colNames = db->listColumns();
    // move the current view columns to the top of the list, in correct order
    int count = oldNames.count();
    int i;
    for (i = count - 1; i > -1; i--) {
        QString name = oldNames[i];
        colNames.removeAll(name);
        colNames.prepend(name);
    }
    count = colNames.count();
    QTreeWidgetItem *item = 0;
    for (i = 0; i < count; i++) {
        QString name = colNames[i];
        if (i == 0) {
            item = new QTreeWidgetItem(table);
        }
        else {
            item = new QTreeWidgetItem(table, item);
        }
        item->setTextAlignment(0, Qt::AlignHCenter);
        bool included = oldNames.contains(name);
        item->setCheckState(0, included ? Qt::Checked : Qt::Unchecked);
        item->setText(1, name);
    }
}

/**
 * Apply to the database any changes made the last time this dialog was shown.
 */
void ViewEditor::applyChanges()
{
    QString viewName = nameBox->text();
    QString defaultSort = sortingBox->currentText();
    if (sortingBox->currentIndex() == 0) {
        defaultSort = "_none";
    }
    QString defaultFilter = filterBox->currentText();
    if (filterBox->currentIndex() == 0) {
        defaultFilter = "_none";
    }
    else if (filterBox->currentIndex() == 1) {
        defaultFilter = "_allrows";
    }
    int count = table->topLevelItemCount();
    if (originalName.isEmpty()) {
        // new view, just add it and return
        QStringList sequence;
        for (int i = 0; i < count; i++) {
            QTreeWidgetItem *item = table->topLevelItem(i);
            if (item->checkState(0) == Qt::Checked) {
                sequence.append(item->text(1));
            }
        }
        db->addView(viewName, sequence, defaultSort, defaultFilter);
        return;
    }
    db->setViewDefaults(defaultSort, defaultFilter);
    if (viewName != originalName) {
        db->renameView(originalName, viewName);
    }
    QStringList sequence;
    for (int i = 0; i < count; i++) {
        QTreeWidgetItem *item = table->topLevelItem(i);
        QString name = item->text(1);
        if (item->checkState(0) == Qt::Checked) {
            sequence.append(name);
            if (!oldNames.contains(name)) {
                // newly added column
                db->addViewColumn(viewName, name);
            }
        }
        else {
            if (oldNames.contains(name)) {
                // removed column
                db->deleteViewColumn(viewName, name);
            }
        }
    }
    db->setViewColumnSequence(viewName, sequence);
}
