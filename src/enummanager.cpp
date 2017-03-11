/*
 * enummanager.cpp
 *
 * (c) 2002-2004,2008-2010,2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file enummanager.cpp
 * Source file for EnumManager
 */

#include <QAction>
#include <QApplication>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QStringList>
#include <QStackedWidget>
#include "database.h"
#include "enumeditor.h"
#include "enummanager.h"
#include "factory.h"
#include "viewdisplay.h"

/**
 * Constructor.
 *
 * @param dbase The database to be edited
 * @param parent This dialog's parent widget
 * @param viewDisplay The main data display widget
 */
EnumManager::EnumManager(Database *dbase, QWidget *parent, ViewDisplay *viewDisplay)
  : PBDialog(tr("Enums"), parent), viewer(viewDisplay),
  contentChanged(false), orderChanged(false)
{
    stack = new QStackedWidget(this);
    vbox->addWidget(stack, 1);
    QString text("<center>%1<br><br>%2</center>");
    text = text.arg(tr("No enumerated column types defined"));
    text = text.arg(tr("Press the \"Add\" button to create one"));
    noEnums = new QLabel(text, stack);
    stack->addWidget(noEnums);

    db = dbase;
    listWidget = Factory::listWidget(stack);
    stack->addWidget(listWidget);
    listWidget->addItems(db->listEnums());
    if (listWidget->count() == 0) {
        stack->setCurrentWidget(noEnums);
    }
    else {
        stack->setCurrentWidget(listWidget);
    }

    addEditButtons();
    connect(addAction, SIGNAL(triggered()), this, SLOT(addEnum()));
    connect(editAction, SIGNAL(triggered()), this, SLOT(editEnum()));
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(deleteEnum()));
    connect(upAction, SIGNAL(triggered()), this, SLOT(moveUp()));
    connect(downAction, SIGNAL(triggered()), this, SLOT(moveDown()));

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
        stack->setCurrentWidget(listWidget);
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
        viewer->closeView();
        enumEditor.applyChanges();
        viewer->setDatabase(db);
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
    viewer->closeView();
    db->deleteEnum(enumName);
    viewer->setDatabase(db);
    delete item;
    if (listWidget->count() == 0) {
        stack->setCurrentWidget(noEnums);
    }
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
