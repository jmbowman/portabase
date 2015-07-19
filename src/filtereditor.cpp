/*
 * filtereditor.cpp
 *
 * (c) 2002-2004,2008-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file filtereditor.cpp
 * Source file for FilterEditor
 */

#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include "database.h"
#include "condition.h"
#include "conditioneditor.h"
#include "factory.h"
#include "filter.h"
#include "filtereditor.h"

/**
 * Constructor.
 *
 * @param parent This dialog's parent widget
 */
FilterEditor::FilterEditor(QWidget *parent)
    : PBDialog(tr("Filter Editor"), parent), db(0), filter(0)
{
    QHBoxLayout *hbox = Factory::hBoxLayout(vbox);
    hbox->addWidget(new QLabel(tr("Filter Name") + " ", this));
    nameBox = new QLineEdit(this);
    hbox->addWidget(nameBox);

    listWidget = Factory::listWidget(this);
    vbox->addWidget(listWidget);

    addEditButtons();
    connect(addButton, SIGNAL(clicked()), this, SLOT(addCondition()));
    connect(editButton, SIGNAL(clicked()), this, SLOT(editCondition()));
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteCondition()));
    connect(upButton, SIGNAL(clicked()), this, SLOT(moveUp()));
    connect(downButton, SIGNAL(clicked()), this, SLOT(moveDown()));

    finishLayout();
    nameBox->setFocus();
}

/**
 * Destructor.
 */
FilterEditor::~FilterEditor()
{
    if (filter) {
        delete filter;
    }
}

/**
 * Launch this dialog to edit a filter.
 *
 * @param subject The database being edited
 * @param filterName The name of the filter to edit
 * @return 1 if the dialog's changes are to be committed, 0 otherwise
 */
int FilterEditor::edit(Database *subject, const QString &filterName)
{
    db = subject;
    originalName = filterName;
    nameBox->setText(filterName);
    if (filter) {
        delete filter;
    }
    conditionEditor = new ConditionEditor(db, this);
    filter = new Filter(db, filterName);
    updateList();
    int result = exec();
    while (result) {
        if (hasValidName()) {
            break;
        }
        else {
            result = exec();
        }
    }
    delete conditionEditor;
    return result;
}

/**
 * Get the last specified name for this filter.
 *
 * @return The filter's name
 */
QString FilterEditor::getName()
{
    return nameBox->text();
}

/**
 * Add a new condition to the filter.  Triggered by the "Add" button.
 */
void FilterEditor::addCondition()
{
    Condition *condition = new Condition(db);
    if (conditionEditor->edit(condition)) {
        conditionEditor->applyChanges(condition);
        filter->addCondition(condition);
        updateList();
    }
    else {
        delete condition;
    }
}

/**
 * Edit one of the filter's existing conditions.  Triggered by the "Edit"
 * button.
 */
void FilterEditor::editCondition()
{
    int selected = listWidget->currentRow();
    if (selected == -1) {
        return;
    }
    Condition *condition = filter->getCondition(selected);
    if (conditionEditor->edit(condition)) {
        conditionEditor->applyChanges(condition);
        updateList();
        listWidget->setCurrentRow(selected);
    }
}

/**
 * Delete the currently selected condition.  Triggered by the "Delete"
 * button.
 */
void FilterEditor::deleteCondition()
{
    int selected = listWidget->currentRow();
    if (selected == -1) {
        return;
    }
    filter->deleteCondition(selected);
    updateList();
}

/**
 * Move the currently selected condition up by one in the sequence.
 * Triggered by the "Up" button.
 */
void FilterEditor::moveUp()
{
    int selected = listWidget->currentRow();
    if (selected == -1) {
        return;
    }
    if (filter->moveConditionUp(selected)) {
        updateList();
        listWidget->setCurrentRow(selected - 1);
    }
}

/**
 * Move the currently selected condition down by one in the sequence.
 * Triggered by the "Down" button.
 */
void FilterEditor::moveDown()
{
    int selected = listWidget->currentRow();
    if (selected == -1) {
        return;
    }
    if (filter->moveConditionDown(selected)) {
        updateList();
        listWidget->setCurrentRow(selected + 1);
    }
}

/**
 * Update the displayed list of conditions.
 */
void FilterEditor::updateList()
{
    listWidget->clear();
    int count = filter->getConditionCount();
    for (int i = 0; i < count; i++) {
        listWidget->addItem(filter->getCondition(i)->getDescription());
    }
}

/**
 * Determine if the currently specified filter name is valid or not.
 * Prevents duplicates and names beginning with an underscore.
 *
 * @return True if the name is valid, false otherwise
 */
bool FilterEditor::hasValidName()
{
    return validateName(nameBox->text(), originalName, db->listFilters());
}

/**
 * Apply any changes made in this dialog.
 */
void FilterEditor::applyChanges()
{
    db->deleteFilter(originalName);
    filter->setName(nameBox->text());
    db->addFilter(filter);
}
