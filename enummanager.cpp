/*
 * enummanager.cpp
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qhbox.h>
#include <qlistbox.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qvbox.h>
#include "database.h"
#include "enumeditor.h"
#include "enummanager.h"

EnumManager::EnumManager(Database *dbase, QWidget *parent, const char *name, WFlags f) : QDialog(parent, name, TRUE, f)
{
    db = dbase;
    setCaption(tr("Enum Manager") + " - " + tr("PortaBase"));
    vbox = new QVBox(this);

    listBox = new QListBox(vbox);
    listBox->insertStringList(db->listEnums());

    QHBox *hbox = new QHBox(vbox);
    QPushButton *addButton = new QPushButton(tr("Add"), hbox);
    connect(addButton, SIGNAL(clicked()), this, SLOT(addEnum()));
    QPushButton *editButton = new QPushButton(tr("Edit"), hbox);
    connect(editButton, SIGNAL(clicked()), this, SLOT(editEnum()));
    QPushButton *deleteButton = new QPushButton(tr("Delete"), hbox);
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteEnum()));
    QPushButton *upButton = new QPushButton(tr("Up"), hbox);
    connect(upButton, SIGNAL(clicked()), this, SLOT(moveUp()));
    QPushButton *downButton = new QPushButton(tr("Down"), hbox);
    connect(downButton, SIGNAL(clicked()), this, SLOT(moveDown()));

    showMaximized();
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
        int choice = QMessageBox::warning(this, tr("PortaBase"), msg,
                                          tr("Yes"), tr("No"));
        if (choice != 0) {
            return;
        }
    }
    db->deleteEnum(enumName);
    listBox->removeItem(selected);
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
}

void EnumManager::applyChanges()
{
    QStringList names;
    int count = listBox->count();
    for (int i = 0; i < count; i++) {
        names.append(listBox->text(i));
    }
    db->setEnumSequence(names);
}

void EnumManager::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    vbox->resize(size());
}
