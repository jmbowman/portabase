/*
 * enummanager.cpp
 *
 * (c) 2002-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qlistbox.h>
#include <qmessagebox.h>
#include <qstringlist.h>
#include "database.h"
#include "enumeditor.h"
#include "enummanager.h"

EnumManager::EnumManager(Database *dbase, QWidget *parent, const char *name)
  : PBDialog(tr("Enum Manager"), parent, name), contentChanged(FALSE), orderChanged(FALSE)
{
    db = dbase;
    listBox = new QListBox(this);
    vbox->addWidget(listBox);
    listBox->insertStringList(db->listEnums());

    addEditButtons();
    connect(addButton, SIGNAL(clicked()), this, SLOT(addEnum()));
    connect(editButton, SIGNAL(clicked()), this, SLOT(editEnum()));
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteEnum()));
    connect(upButton, SIGNAL(clicked()), this, SLOT(moveUp()));
    connect(downButton, SIGNAL(clicked()), this, SLOT(moveDown()));

    finishLayout();
}

EnumManager::~EnumManager()
{

}

void EnumManager::addEnum()
{
    EnumEditor enumEditor(this);
    if (enumEditor.edit(db, "")) {
        enumEditor.applyChanges();
        listBox->insertItem(enumEditor.getName());
        contentChanged = TRUE;
    }
}

void EnumManager::editEnum()
{
    int selected = listBox->currentItem();
    if (selected == -1) {
        return;
    }
    QString enumName = listBox->text(selected);
    EnumEditor enumEditor(this);
    if (enumEditor.edit(db, enumName)) {
        enumEditor.applyChanges();
        listBox->changeItem(enumEditor.getName(), selected);
        contentChanged = TRUE;
    }
}

void EnumManager::deleteEnum()
{
    int selected = listBox->currentItem();
    if (selected == -1) {
        return;
    }
    QString enumName = listBox->text(selected);
    QStringList deleteCols = db->columnsUsingEnum(enumName);
    int count = deleteCols.count();
    if (count > 0) {
        QString msg = tr("The following columns which\nuse this enum will also be\ndeleted:");
        for (int i = 0; i < count; i++) {
            msg += "\n     " + deleteCols[i];
        }
        msg += "\n" + tr("Continue?");
        int choice = QMessageBox::warning(this, QQDialog::tr("PortaBase"), msg,
                                          QObject::tr("Yes"), QObject::tr("No"));
        if (choice != 0) {
            return;
        }
    }
    db->deleteEnum(enumName);
    listBox->removeItem(selected);
    contentChanged = TRUE;
}

void EnumManager::moveUp()
{
    int selected = listBox->currentItem();
    if (selected < 1) {
        return;
    }
    QString enumName = listBox->text(selected);
    listBox->removeItem(selected);
    listBox->insertItem(enumName, selected - 1);
    listBox->setCurrentItem(selected - 1);
    orderChanged = TRUE;
}

void EnumManager::moveDown()
{
    int selected = listBox->currentItem();
    int count = listBox->count();
    if (selected == -1 || selected == count - 1) {
        return;
    }
    QString enumName = listBox->text(selected);
    listBox->removeItem(selected);
    listBox->insertItem(enumName, selected + 1);
    listBox->setCurrentItem(selected + 1);
    orderChanged = TRUE;
}

void EnumManager::applyChanges()
{
    if (!orderChanged && !contentChanged) {
        return;
    }
    QStringList names;
    int count = listBox->count();
    for (int i = 0; i < count; i++) {
        names.append(listBox->text(i));
    }
    db->setEnumSequence(names);
    contentChanged = TRUE;
}

bool EnumManager::changesMade()
{
    return contentChanged;
}
