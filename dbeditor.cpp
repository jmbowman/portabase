/*
 * dbeditor.cpp
 *
 * (c) 2002-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qheader.h>
#include <qlistview.h>
#include <qmessagebox.h>
#include <qregexp.h>
#include "calc/calcnode.h"
#include "columneditor.h"
#include "database.h"
#include "datatypes.h"
#include "dbeditor.h"
#include "portabase.h"
#include "shadedlistitem.h"

DBEditor::DBEditor(QWidget *parent, const char *name)
    : PBDialog(tr("Columns Editor"),parent, name), db(0), ceName("_cename"),
      ceType("_cetype"), ceDefault("_cedefault"), ceOldIndex("_ceoldindex"),
      ceNewIndex("_cenewindex"), resized(FALSE)
{
    table = new QListView(this);
    vbox->addWidget(table);
    table->setAllColumnsShowFocus(TRUE);
    table->setSorting(-1);
    table->header()->setMovingEnabled(FALSE);
    int colWidth = -1;
#if !defined(Q_WS_WIN)
    colWidth = width() / 3 - 2;
#endif
    table->addColumn(tr("Name"), colWidth);
    table->addColumn(tr("Type"), colWidth);
    table->addColumn(tr("Default"), colWidth);

    addEditButtons();
    connect(addButton, SIGNAL(clicked()), this, SLOT(addColumn()));
    connect(editButton, SIGNAL(clicked()), this, SLOT(editColumn()));
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteColumn()));
    connect(upButton, SIGNAL(clicked()), this, SLOT(moveUp()));
    connect(downButton, SIGNAL(clicked()), this, SLOT(moveDown()));

    finishLayout();
}

DBEditor::~DBEditor()
{
    NameCalcMap::Iterator iter;
    for (iter = calcMap.begin(); iter != calcMap.end(); ++iter) {
        CalcNode *calcRoot = iter.data();
        if (calcRoot != 0) {
            delete calcRoot;
        }
    }
}

int DBEditor::edit(Database *subject)
{
    db = subject;
    columnEditor = new ColumnEditor(db, this);
    originalCols = db->listColumns();
    renamedCols = db->listColumns();
    int count = originalCols.count();
    for (int i = 0; i < count; i++) {
        QString name = originalCols[i];
        int type = db->getType(name);
        QString defaultVal = db->getDefault(name);
        if (type == CALC) {
            int decimals = 2;
            CalcNode *root = db->loadCalc(name, &decimals);
            calcMap.insert(name, root);
            decimalsMap.insert(name, decimals);
            if (root != 0) {
                defaultVal = root->equation(db);
            }
        }
        info.Add(ceName [name.utf8()] + ceType [type]
                 + ceDefault [defaultVal.utf8()]
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
    int decimals = 2;
    CalcNode *calcRoot = columnEditor->calculation(&decimals);
    if (!aborted) {
        int size = info.GetSize();
        info.Add(ceName [name.utf8()] + ceType [type]
                 + ceDefault[defaultVal.utf8()]
                 + ceOldIndex [-1] + ceNewIndex[size]);
        if (type == CALC) {
            calcMap.insert(name, calcRoot);
            decimalsMap.insert(name, decimals);
            columnEditor->setCalculation(0, 2);
            calcRoot = 0;
        }
        updateTable();
    }
    if (calcRoot != 0) {
        delete calcRoot;
        columnEditor->setCalculation(0, 2);
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
    int index = info.Find(ceName [name.utf8()]);
    int type = ceType (info[index]);
    QString defaultVal = QString::fromUtf8(ceDefault (info[index]));

    columnEditor->setName(name);
    columnEditor->setType(type);
    columnEditor->setTypeEditable(FALSE);
    columnEditor->setDefaultValue(defaultVal);
    CalcNode *calcRoot = 0;
    int decimals = 2;
    if (type == CALC) {
        calcRoot = calcMap[name];
        decimals = decimalsMap[name];
        // use a copy in case the edit is cancelled
        if (calcRoot != 0) {
            calcRoot = calcRoot->clone();
        }
        columnEditor->setCalculation(calcRoot, decimals);
    }
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
                    renameColumnRefs(oldName, name);
                }
            }
            else {
                finished = isValidDefault(type, defaultVal);
            }
        }
    }
    calcRoot = columnEditor->calculation(&decimals);
    if (!aborted) {
        ceName (info[index]) = name.utf8();
        ceDefault (info[index]) = defaultVal.utf8();
        if (type == CALC) {
            CalcNode *oldRoot = calcMap[oldName];
            if (oldRoot != 0) {
                delete oldRoot;
            }
            calcMap.remove(oldName);
            decimalsMap.remove(oldName);
            calcMap.insert(name, calcRoot);
            decimalsMap.insert(name, decimals);
            columnEditor->setCalculation(0, 2);
            calcRoot = 0;
        }
        updateTable();
    }
    if (calcRoot != 0) {
        delete calcRoot;
        columnEditor->setCalculation(0, 2);
    }
}

bool DBEditor::isValidName(QString colName)
{
    QStringList nameList;
    int size = info.GetSize();
    for (int i = 0; i < size; i++) {
        nameList.append(QString::fromUtf8(ceName (info[i])));
    }
    return validateName(colName, "", nameList);
}

bool DBEditor::isValidDefault(int type, QString defaultVal)
{
    if (type == DATE || type == TIME || type >= FIRST_ENUM) {
        return TRUE;
    }
    QString error = db->isValidValue(type, defaultVal);
    if (error != "") {
        QString message = tr("Default") + " " + error;
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
    int index = info.Find(ceName [name.utf8()]);
    int oldIndex = ceOldIndex (info[index]);
    info.RemoveAt(index);
    if (oldIndex != -1) {
        deletedCols.append(originalCols[oldIndex]);
    }
    int size = info.GetSize();
    for (int i = index; i < size; i++) {
        ceNewIndex (info[i]) = ceNewIndex (info[i]) - 1;
    }
    if (calcMap.contains(name)) {
        CalcNode *calcRoot = calcMap[name];
        if (calcRoot != 0) {
            delete calcRoot;
        }
        calcMap.remove(name);
        decimalsMap.remove(name);
    }
    deleteColumnRefs(name);
    updateTable();
}

void DBEditor::moveUp()
{
    QListViewItem *item = table->selectedItem();
    if (item == 0) {
        return;
    }
    QString name = item->text(0);
    int row1 = info.Find(ceName [name.utf8()]);
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
    int row1 = info.Find(ceName [name.utf8()]);
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
        QString name = QString::fromUtf8(ceName (temp[i]));
        int type = ceType (temp[i]);
        QString typeString = getTypeString(type);
        QString defaultVal = QString::fromUtf8(ceDefault (temp[i]));
        if (type == NOTE) {
            defaultVal = defaultVal.replace(QRegExp("\n"), " ");
        }
        if (i == 0) {
            last = new ShadedListItem(0, table, name, typeString, defaultVal);
        }
        else {
            last = new ShadedListItem(i, table, last, name, typeString,
                                      defaultVal);
        }
        if (type == BOOLEAN) {
            last->setText(2, "");
            int checked = defaultVal.toInt();
            last->setPixmap(2, PortaBase::getCheckBoxPixmap(checked));
        }
        else if (type == NOTE) {
            last->setPixmap(2, PortaBase::getNotePixmap());
        }
        else if (type == DATE) {
            if (defaultVal == "0") {
                last->setText(2, tr("Today"));
            }
            else {
                last->setText(2, tr("None"));
            }
        }
        else if (type == TIME) {
            if (defaultVal == "0") {
                last->setText(2, tr("Now"));
            }
            else {
                last->setText(2, tr("None"));
            }
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
    else if (type == DATE) {
        return tr("Date");
    }
    else if (type == TIME) {
        return tr("Time");
    }
    else if (type == CALC) {
        return tr("Calculation");
    }
    else if (type == SEQUENCE) {
        return tr("Sequence");
    }
    else if (type == IMAGE) {
        return tr("Image");
    }
    else {
        return db->getEnumName(type);
    }
}

void DBEditor::applyChanges()
{
    // handle deletions of original columns
    int deleteCount = deletedCols.count();
    int i;
    for (i = 0; i < deleteCount; i++) {
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
        int type = ceType (info[index]);
        if (type != CALC) {
            QString defaultVal = QString::fromUtf8(ceDefault (info[index]));
            db->setDefault(oldName, defaultVal);
        }
    }
    // handle renames of original columns
    for (i = 0; i < oldCount; i++) {
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
    for (i = 0; i < newCount; i++) {
        int index = ceNewIndex (temp[i]);
        QString name = QString::fromUtf8(ceName (temp[i]));
        int type = ceType (temp[i]);
        QString defaultVal = QString::fromUtf8(ceDefault (temp[i]));
        if (type == CALC) {
            defaultVal = "";
        }
        db->addColumn(index, name, type, defaultVal);
    }
    db->updateDataFormat();
    // add and update calculations
    NameCalcMap::Iterator iter;
    for (iter = calcMap.begin(); iter != calcMap.end(); ++iter) {
        QString name = iter.key();
        CalcNode *calcRoot = iter.data();
        db->updateCalc(name, calcRoot, decimalsMap[name]);
        if (calcRoot != 0) {
            delete calcRoot;
        }
    }
    calcMap.clear();
}

void DBEditor::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    if (!resized) {
        int colWidth = width() / 3 - 2;
        table->setColumnWidth(0, colWidth);
        table->setColumnWidth(1, colWidth);
        table->setColumnWidth(2, colWidth);
        resized = TRUE;
    }
}

QStringList DBEditor::columnNames()
{
    QStringList names;
    c4_View temp = info.SortOn(ceNewIndex);
    int size = temp.GetSize();
    for (int i = 0; i < size; i++) {
        names.append(QString::fromUtf8(ceName (temp[i])));
    }
    return names;
}

int *DBEditor::columnTypes()
{
    c4_View temp = info.SortOn(ceNewIndex);
    int size = temp.GetSize();
    int *types = new int[size];
    for (int i = 0; i < size; i++) {
        types[i] = ceType (temp[i]);
    }
    return types;
}

void DBEditor::renameColumnRefs(const QString &oldName, const QString &newName)
{
    NameCalcMap::Iterator iter;
    for (iter = calcMap.begin(); iter != calcMap.end(); ++iter) {
        QString calcName = iter.key();
        CalcNode *calcRoot = iter.data();
        if (calcRoot != 0) {
            int index = info.Find(ceName [calcName.utf8()]);
            calcRoot->renameColumn(oldName, newName);
            ceDefault (info[index]) = calcRoot->equation(db).utf8();
        }
    }
}

void DBEditor::deleteColumnRefs(const QString &name)
{
    NameCalcMap::Iterator iter;
    QStringList deletions;
    for (iter = calcMap.begin(); iter != calcMap.end(); ++iter) {
        QString calcName = iter.key();
        CalcNode *calcRoot = iter.data();
        if (calcRoot != 0) {
            int index = info.Find(ceName [calcName.utf8()]);
            if (calcRoot->deleteColumn(name)) {
                delete calcRoot;
                deletions.append(calcName);
                ceDefault (info[index]) = "";
            }
            else {
                ceDefault (info[index]) = calcRoot->equation(db).utf8();
            }
        }
    }
    int count = deletions.count();
    for (int i = 0; i < count; i++) {
        calcMap.remove(deletions[i]);
        calcMap.insert(deletions[i], 0);
    }
}
