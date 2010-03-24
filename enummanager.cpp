/*
 * enummanager.cpp
 *
 * (c) 2002-2004,2008-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file enummanager.cpp
 * Source file for EnumManager
 */

#include <QApplication>
#include <QLayout>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QStringList>
#include "database.h"
#include "enumeditor.h"
#include "enummanager.h"
#include "factory.h"

/**
 * Constructor.
 *
 * @param dbase The database to be edited
 * @param parent This dialog's parent widget
 */
EnumManager::EnumManager(Database *dbase, QWidget *parent)
  : PBDialog(tr("Enum Manager"), parent), contentChanged(false), orderChanged(false)
{
    db = dbase;
    listWidget = Factory::listWidget(this);
    vbox->addWidget(listWidget);
    listWidget->addItems(db->listEnums());

    addEditButtons();
    connect(addButton, SIGNAL(clicked()), this, SLOT(addEnum()));
    connect(editButton, SIGNAL(clicked()), this, SLOT(editEnum()));
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteEnum()));
    connect(upButton, SIGNAL(clicked()), this, SLOT(moveUp()));
    connect(downButton, SIGNAL(clicked()), this, SLOT(moveDown()));

    finishLayout();
}

/**
 * Create a new enumeration.  Triggered by the "Add" button.
 */
void EnumManager::addEnum()
{
    EnumEditor enumEditor(this);
    if (enumEditor.edit(db, "")) {
        enumEditor.applyChanges();
        listWidget->addItem(enumEditor.getName());
        contentChanged = true;
    }
}

/**
 * Edit the selected enumeration.  Triggered by the "Edit" button.
 */
void EnumManager::editEnum()
{
    QListWidgetItem *item = listWidget->currentItem();
    if (!item) {
        return;
    }
    QString enumName = item->text();
    EnumEditor enumEditor(this);
    if (enumEditor.edit(db, enumName)) {
        enumEditor.applyChanges();
        item->setText(enumEditor.getName());
        contentChanged = true;
    }
}

/**
 * Delete the selected enumeration.  Triggered by the "Delete" button.
 */
void EnumManager::deleteEnum()
{
    QListWidgetItem *item = listWidget->currentItem();
    if (!item) {
        return;
    }
    QString enumName = item->text();
    QStringList deleteCols = db->columnsUsingEnum(enumName);
    int count = deleteCols.count();
    if (count > 0) {
        QString msg = tr("The following columns which\nuse this enum will also be\ndeleted:");
        for (int i = 0; i < count; i++) {
            msg += "\n     " + deleteCols[i];
        }
        msg += "\n" + tr("Continue?");
        int choice = QMessageBox::warning(this, qApp->applicationName(), msg,
                                           QMessageBox::Yes|QMessageBox::No,
                                           QMessageBox::No);
        if (choice != QMessageBox::Yes) {
            return;
        }
    }
    db->deleteEnum(enumName);
    delete item;
    contentChanged = true;
}

/**
 * Move the selected enumeration up by one in the list.  Triggered by the
 * "Up" button.
 */
void EnumManager::moveUp()
{
    int row = listWidget->currentRow();
    if (row > 0) {
        QListWidgetItem *item = listWidget->takeItem(row);
        listWidget->insertItem(row - 1, item);
        listWidget->setCurrentRow(row - 1);
        orderChanged = true;
    }
}

/**
 * Move the selected enumeration down by one in the list.  Triggered by the
 * "Down" button.
 */
void EnumManager::moveDown()
{
    int row = listWidget->currentRow();
    int count = listWidget->count();
    if (row < count - 1) {
        QListWidgetItem *item = listWidget->takeItem(row);
        listWidget->insertItem(row + 1, item);
        listWidget->setCurrentRow(row + 1);
        orderChanged = true;
    }
}

/**
 * Apply the changes made in this dialog to the database.
 */
void EnumManager::applyChanges()
{
    if (!orderChanged && !contentChanged) {
        return;
    }
    QStringList names;
    int count = listWidget->count();
    for (int i = 0; i < count; i++) {
        names.append(listWidget->item(i)->text());
    }
    db->setEnumSequence(names);
    contentChanged = true;
}

/**
 * Determine if any changes were made in this dialog.
 *
 * @return True if changes were made, false otherwise
 */
bool EnumManager::changesMade()
{
    return contentChanged;
}
