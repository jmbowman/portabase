/*
 * dbeditor.cpp
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qheader.h>
#include <qlistview.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qvbox.h>
#include "columneditor.h"
#include "database.h"
#include "datatypes.h"
#include "dbeditor.h"
#include "portabase.h"

DBEditor::DBEditor(QWidget *parent, const char *name, WFlags f)
    : QDialog(parent, name, TRUE, f), db(0), ceName("_cename"),
      ceType("_cetype"), ceDefault("_cedefault"), ceOldIndex("_ceoldindex"),
      ceNewIndex("_cenewindex"), resized(FALSE)
{
    setCaption(tr("PortaBase") + " - " + tr("Columns Editor"));
    vbox = new QVBox(this);
    vbox->resize(size());
    table = new QListView(vbox);
    table->setAllColumnsShowFocus(TRUE);
    table->setSorting(-1);
    table->header()->setMovingEnabled(FALSE);
    int colWidth = width() / 3 - 2;
    table->addColumn(tr("Name"), colWidth);
    table->addColumn(tr("Type"), colWidth);
    table->addColumn(tr("Default"), colWidth);

    columnEditor = new ColumnEditor(this);

    QHBox *hbox = new QHBox(vbox);
    QPushButton *addButton = new QPushButton(tr("Add"), hbox);
    connect(addButton, SIGNAL(clicked()), this, SLOT(addColumn()));
    QPushButton *editButton = new QPushButton(tr("Edit"), hbox);
    connect(editButton, SIGNAL(clicked()), this, SLOT(editColumn()));
    QPushButton *deleteButton = new QPushButton(tr("Delete"), hbox);
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteColumn()));
    QPushButton *upButton = new QPushButton(tr("Up"), hbox);
    connect(upButton, SIGNAL(clicked()), this, SLOT(moveUp()));
    QPushButton *downButton = new QPushButton(tr("Down"), hbox);
    connect(downButton, SIGNAL(clicked()), this, SLOT(moveDown()));
    showMaximized();
}

DBEditor::~DBEditor()
{

}

int DBEditor::edit(Database *subject)
{
    db = subject;
    originalCols = db->listColumns();
    renamedCols = db->listColumns();
    int count = originalCols.count();
    for (int i = 0; i < count; i++) {
        QString name = originalCols[i];
        int type = db->getType(name);
        QString defaultVal = db->getDefault(name);
        info.Add(ceName [name] + ceType [type] + ceDefault [defaultVal]
                 + ceOldIndex [i] + ceNewIndex [i]);
    }
    updateTable();
    return exec();
}

void DBEditor::addColumn()
{
    QString name = "";
    int type = STRING;
    QString defaultVal = "";
    columnEditor->setName(name);
    columnEditor->setType(type);
    columnEditor->setTypeEditable(TRUE);
    columnEditor->setDefaultValue(defaultVal);
    bool finished = FALSE;
    bool aborted = FALSE;
    while (!finished) {
        if (!columnEditor->exec()) {
            finished = TRUE;
            aborted = TRUE;
        }
        else {
            name = columnEditor->name();
            type = columnEditor->type();
            defaultVal = columnEditor->defaultValue();
            finished = (isValidName(name) && isValidDefault(type, defaultVal));
        }
        
    }
    if (!aborted) {
        int size = info.GetSize();
        info.Add(ceName [name] + ceType [type] + ceDefault[defaultVal]
                 + ceOldIndex [-1] + ceNewIndex[size]);
        updateTable();
    }
}

void DBEditor::editColumn()
{
    QListViewItem *item = table->selectedItem();
    if (item == 0) {
        return;
    }
    QString name = item->text(0);
    QString oldName = name;
    int index = info.Find(ceName [name]);
    int type = ceType (info[index]);
    QString defaultVal(ceDefault (info[index]));

    columnEditor->setName(name);
    columnEditor->setType(type);
    columnEditor->setTypeEditable(FALSE);
    columnEditor->setDefaultValue(defaultVal);
    bool finished = FALSE;
    bool aborted = FALSE;
    while (!finished) {
        if (!columnEditor->exec()) {
            finished = TRUE;
            aborted = TRUE;
        }
        else {
            name = columnEditor->name();
            type = columnEditor->type();
            defaultVal = columnEditor->defaultValue();
            if (name != oldName) {
                finished = (isValidName(name)
                            && isValidDefault(type, defaultVal));
                if (finished) {
                    int oldIndex = ceOldIndex (info[index]);
                    if (oldIndex != -1) {
                        renamedCols[oldIndex] = name;
                    }
                }
            }
            else {
                finished = isValidDefault(type, defaultVal);
            }
        }
    }
    if (!aborted) {
        ceName (info[index]) = name;
        ceDefault (info[index]) = defaultVal;
        updateTable();
    }
}

bool DBEditor::isValidName(QString colName)
{
    if (colName.isEmpty()) {
        QMessageBox::warning(this, tr("PortaBase"),
                             tr("No name entered"));
        return FALSE;
    }
    if (colName[0] == '_') {
        QMessageBox::warning(this, tr("PortaBase"),
                             tr("Name must not start with '_'"));
        return FALSE;
    }
    if (colName.find(':') != -1) {
        QMessageBox::warning(this, tr("PortaBase"),
                             tr("Name must not contain ':'"));
        return FALSE;
    }
    // check for other columns with same name
    bool result = TRUE;
    int size = info.GetSize();
    for (int i = 0; i < size; i++) {
        QString name(ceName (info[i]));
        if (name == colName) {
            result = FALSE;
            break;
        }
    }
    if (!result) {
        QMessageBox::warning(this, tr("PortaBase"), tr("Duplicate name"));
    }
    return result;
}

bool DBEditor::isValidDefault(int type, QString defaultVal)
{
    if (type == DATE) {
        return TRUE;
    }
    QString error = db->isValidValue(type, defaultVal);
    if (error != "") {
        QString message = tr("Default") + " " + tr(error);
        QMessageBox::warning(this, tr("PortaBase"), message);
        return FALSE;
    }
    else {
        return TRUE;
    }
}

void DBEditor::deleteColumn()
{
    QListViewItem *item = table->selectedItem();
    if (item == 0) {
        return;
    }
    QString name = item->text(0);
    int index = info.Find(ceName [name]);
    int oldIndex = ceOldIndex (info[index]);
    info.RemoveAt(index);
    if (oldIndex != -1) {
        deletedCols.append(originalCols[oldIndex]);
    }
    int size = info.GetSize();
    for (int i = index; i < size; i++) {
        ceNewIndex (info[i]) = ceNewIndex (info[i]) - 1;
    }
    updateTable();
}

void DBEditor::moveUp()
{
    QListViewItem *item = table->selectedItem();
    if (item == 0) {
        return;
    }
    QString name = item->text(0);
    int row1 = info.Find(ceName [name]);
    int index = ceNewIndex (info[row1]);
    if (index > 0) {
        int row2 = info.Find(ceNewIndex [index - 1]);
        ceNewIndex (info[row1]) = index - 1;
        ceNewIndex (info[row2]) = index;
        updateTable();
        selectRow(name);
    }
}

void DBEditor::moveDown()
{
    QListViewItem *item = table->selectedItem();
    if (item == 0) {
        return;
    }
    QString name = item->text(0);
    int row1 = info.Find(ceName [name]);
    int index = ceNewIndex (info[row1]);
    int size = info.GetSize();
    if (index < size - 1) {
        int row2 = info.Find(ceNewIndex [index + 1]);
        ceNewIndex (info[row1]) = index + 1;
        ceNewIndex (info[row2]) = index;
        updateTable();
        selectRow(name);
    }
}

void DBEditor::selectRow(QString colName)
{
    QListViewItem *item = table->firstChild();
    if (item) {
        if (item->text(0) == colName) {
            table->setSelected(item, TRUE);
        }
        else {
            QListViewItem *next = item->nextSibling();
	    while (next) {
                if (next->text(0) == colName) {
                    table->setSelected(next, TRUE);
                    break;
                }
                next = next->nextSibling();
            }
        }
    }
}

void DBEditor::updateTable()
{
    table->clear();
    c4_View temp = info.SortOn(ceNewIndex);
    int size = temp.GetSize();
    QListViewItem *last = 0;
    for (int i = 0; i < size; i++) {
        QString name(ceName (temp[i]));
        int type = ceType (temp[i]);
        QString typeString = getTypeString(type);
        QString defaultVal(ceDefault (temp[i]));
        if (i == 0) {
            last = new QListViewItem(table, name, typeString, defaultVal);
        }
        else {
            last = new QListViewItem(table, last, name, typeString,
                                     defaultVal);
        }
        if (type == BOOLEAN) {
            last->setText(2, "");
            last->setPixmap(2, db->getCheckBoxPixmap(defaultVal.toInt()));
        }
        else if (type == NOTE) {
            last->setPixmap(2, PortaBase::getNotePixmap());
        }
        else if (type == DATE) {
            last->setText(2, tr("Today"));
        }
    }
}

QString DBEditor::getTypeString(int type)
{
    if (type == STRING) {
        return tr("String");
    }
    else if (type == INTEGER) {
        return tr("Integer");
    }
    else if (type == FLOAT) {
        return tr("Decimal");
    }
    else if (type == BOOLEAN) {
        return tr("Boolean");
    }
    else if (type == NOTE) {
        return tr("Note");
    }
    else {
        return tr("Date");
    }
}

void DBEditor::applyChanges()
{
    // handle deletions of original columns
    int deleteCount = deletedCols.count();
    for (int i = 0; i < deleteCount; i++) {
        QString name = deletedCols[i];
        db->deleteColumn(name);
    }
    // make minor changes to remaining original columns
    int oldCount = originalCols.count();
    for (int oldIndex = 0; oldIndex < oldCount; oldIndex++) {
        QString oldName = originalCols[oldIndex];
        int index = info.Find(ceOldIndex [oldIndex]);
        if (index == -1) {
            // deleted already, move on
            continue;
        }
        int newIndex = ceNewIndex (info[index]);
        if (newIndex != oldIndex) {
            db->setIndex(oldName, newIndex);
        }
        QString defaultVal(ceDefault (info[index]));
        db->setDefault(oldName, defaultVal);
    }
    // handle renames of original columns
    for (int i = 0; i < oldCount; i++) {
        QString oldName = originalCols[i];
        QString newName = renamedCols[i];
        if (oldName != newName) {
            if (deletedCols.findIndex(oldName) == -1) {
                // hasn't been deleted, go ahead and rename
                db->renameColumn(oldName, newName);
            }
        }
    }
    // add new columns
    c4_View temp = info.Select(ceOldIndex [-1]);
    int newCount = temp.GetSize();
    for (int i = 0; i < newCount; i++) {
        int index = ceNewIndex (temp[i]);
        QString name(ceName (temp[i]));
        int type = ceType (temp[i]);
        QString defaultVal(ceDefault (temp[i]));
        db->addColumn(index, name, type, defaultVal);
    }
    db->updateDataFormat();
}

void DBEditor::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    vbox->resize(size());
    if (!resized) {
        int colWidth = width() / 3 - 2;
        table->setColumnWidth(0, colWidth);
        table->setColumnWidth(1, colWidth);
        table->setColumnWidth(2, colWidth);
        resized = TRUE;
    }
}
