/*
 * dbeditor.cpp
 *
 * (c) 2002-2004,2008-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file dbeditor.cpp
 * Source file for DBEditor
 */

#include <QApplication>
#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QRegExp>
#include <QStackedWidget>
#include <QTreeWidget>
#include "calc/calcnode.h"
#include "columneditor.h"
#include "database.h"
#include "datatypes.h"
#include "dbeditor.h"
#include "factory.h"

/**
 * Constructor.
 *
 * @param parent This dialog's parent widget.
 */
DBEditor::DBEditor(QWidget *parent)
    : PBDialog(tr("Columns Editor"), parent), db(0), ceName("_cename"),
      ceType("_cetype"), ceDefault("_cedefault"), ceOldIndex("_ceoldindex"),
      ceNewIndex("_cenewindex"), resized(false)
{
    stack = new QStackedWidget(this);
    vbox->addWidget(stack, 1);
    QString text("<center>%1<br><br>%2</center>");
    text = text.arg(tr("No columns defined"));
    text = text.arg(tr("Press the \"Add\" button to create one"));
    noColumns = new QLabel(text, stack);
    stack->addWidget(noColumns);

    QStringList headers;
    headers << ColumnEditor::tr("Name");
    headers << ColumnEditor::tr("Type");
    headers << ColumnEditor::tr("Default");
    table = Factory::treeWidget(stack, headers);
    stack->addWidget(table);
    int colWidth = width() / 3 - 2;
    table->setColumnWidth(0, colWidth);
    table->setColumnWidth(1, colWidth);
    table->setColumnWidth(2, colWidth);
    stack->setCurrentWidget(noColumns);

    addEditButtons();
    connect(addButton, SIGNAL(clicked()), this, SLOT(addColumn()));
    connect(editButton, SIGNAL(clicked()), this, SLOT(editColumn()));
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteColumn()));
    connect(upButton, SIGNAL(clicked()), this, SLOT(moveUp()));
    connect(downButton, SIGNAL(clicked()), this, SLOT(moveDown()));

    finishLayout();
}

/**
 * Destructor.
 */
DBEditor::~DBEditor()
{
    NameCalcMap::Iterator iter;
    for (iter = calcMap.begin(); iter != calcMap.end(); ++iter) {
        CalcNode *calcRoot = iter.value();
        if (calcRoot != 0) {
            delete calcRoot;
        }
    }
}

/**
 * Launch this dialog in order to edit the format of the specified database.
 *
 * @param subject The database whose format is to be edited
 */
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
        info.Add(ceName [name.toUtf8()] + ceType [type]
                 + ceDefault [defaultVal.toUtf8()]
                 + ceOldIndex [i] + ceNewIndex [i]);
    }
    updateTable();
    return exec();
}

/**
 * Add a new column to the database via the ColumnEditor dialog.  Called when
 * the "Add" button is pressed.
 */
void DBEditor::addColumn()
{
    QString name = "";
    int type = STRING;
    QString defaultVal = "";
    columnEditor->setName(name);
    columnEditor->setType(type);
    columnEditor->setTypeEditable(true);
    columnEditor->setDefaultValue(defaultVal);
    bool finished = false;
    bool aborted = false;
    while (!finished) {
        if (!columnEditor->exec()) {
            finished = true;
            aborted = true;
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
        info.Add(ceName [name.toUtf8()] + ceType [type]
                 + ceDefault[defaultVal.toUtf8()]
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

/**
 * Use the ColumnEditor dialog to edit the currently selected column
 * definition.  Called when the "Edit" button is pressed.
 */
void DBEditor::editColumn()
{
    QTreeWidgetItem *item = table->currentItem();
    if (!item) {
        return;
    }
    QString name = item->text(0);
    QString oldName = name;
    int index = info.Find(ceName [name.toUtf8()]);
    int type = ceType (info[index]);
    QString defaultVal = QString::fromUtf8(ceDefault (info[index]));

    columnEditor->setName(name);
    columnEditor->setType(type);
    columnEditor->setTypeEditable(false);
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
    bool finished = false;
    bool aborted = false;
    while (!finished) {
        if (!columnEditor->exec()) {
            finished = true;
            aborted = true;
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
        ceName (info[index]) = name.toUtf8();
        ceDefault (info[index]) = defaultVal.toUtf8();
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

/**
 * Determine if the provided text can be used as a valid column name.
 *
 * @param colName The candidate column name
 * @return True if valid, false otherwise
 */
bool DBEditor::isValidName(const QString &colName)
{
    return validateName(colName, "", columnNames());
}

/**
 * Determine if the provided string represents a valid default value for a
 * column of the specified type.  Assumes that the proposed value was
 * retrieved from the ColumnEditor dialog.
 *
 * @param type The type of column in question
 * @param defaultVal The proposed default value
 * @return True if valid, false otherwise
 */
bool DBEditor::isValidDefault(int type, const QString &defaultVal)
{
    if (type == DATE || type == TIME || type >= FIRST_ENUM) {
        // The column editor doesn't allow free entry for these
        return true;
    }
    QString error = db->isValidValue(type, defaultVal);
    if (!error.isEmpty()) {
        QString message = ColumnEditor::tr("Default") + " " + error;
        QMessageBox::warning(this, qApp->applicationName(), message);
        return false;
    }
    else {
        return true;
    }
}

/**
 * Delete the currently selected column definition.
 */
void DBEditor::deleteColumn()
{
    QTreeWidgetItem *item = table->currentItem();
    if (!item) {
        return;
    }
    QString name = item->text(0);
    int index = info.Find(ceName [name.toUtf8()]);
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

/**
 * Move the currently selected column definition up by one (unless it is
 * already the first one).  This results in that column's position index
 * being swapped with the one which had preceded it.
 */
void DBEditor::moveUp()
{
    QTreeWidgetItem *item = table->currentItem();
    if (!item) {
        return;
    }
    QString name = item->text(0);
    int row1 = info.Find(ceName [name.toUtf8()]);
    int index = ceNewIndex (info[row1]);
    if (index > 0) {
        int row2 = info.Find(ceNewIndex [index - 1]);
        ceNewIndex (info[row1]) = index - 1;
        ceNewIndex (info[row2]) = index;
        updateTable();
        selectRow(name);
    }
}

/**
 * Move the currently selected column definition down by one (unless it is
 * already the last one).  This results in that column's position index being
 * swapped with the one which had followed it.
 */
void DBEditor::moveDown()
{
    QTreeWidgetItem *item = table->currentItem();
    if (!item) {
        return;
    }
    QString name = item->text(0);
    int row1 = info.Find(ceName [name.toUtf8()]);
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

/**
 * Select the column definition with the specified name.
 *
 * @param colName The column name of the entry to highlight
 */
void DBEditor::selectRow(const QString &colName)
{
    int count = table->topLevelItemCount();
    for (int i = 0; i < count; i++) {
        QTreeWidgetItem *item = table->topLevelItem(i);
        if (item && item->text(0) == colName) {
            table->setCurrentItem(item);
            break;
        }
    }
}

/**
 * Update the displayed table of column definitions.
 */
void DBEditor::updateTable()
{
    table->clear();
    c4_View temp = info.SortOn(ceNewIndex);
    int size = temp.GetSize();
    QTreeWidgetItem *last = 0;
    for (int i = 0; i < size; i++) {
        QString name = QString::fromUtf8(ceName (temp[i]));
        int type = ceType (temp[i]);
        QString typeString = getTypeString(type);
        QString defaultVal = QString::fromUtf8(ceDefault (temp[i]));
        if (type == NOTE) {
            defaultVal = defaultVal.replace(QRegExp("\n"), " ");
        }
        if (i == 0) {
            last = new QTreeWidgetItem(table);
        }
        else {
            last = new QTreeWidgetItem(table, last);
        }
        last->setText(0, name);
        last->setText(1, typeString);
        last->setText(2, defaultVal);
        if (type == BOOLEAN) {
            last->setText(2, "");
            int checked = defaultVal.toInt();
            last->setCheckState(2, checked ? Qt::Checked : Qt::Unchecked);
        }
        else if (type == NOTE) {
            last->setIcon(2, QIcon(":/icons/note.png"));
        }
        else if (type == DATE) {
            if (defaultVal == "0") {
                last->setText(2, ColumnEditor::tr("Today"));
            }
            else {
                last->setText(2, ColumnEditor::tr("None"));
            }
        }
        else if (type == TIME) {
            if (defaultVal == "0") {
                last->setText(2, ColumnEditor::tr("Now"));
            }
            else {
                last->setText(2, ColumnEditor::tr("None"));
            }
        }
    }
    if (size == 0) {
        stack->setCurrentWidget(noColumns);
    }
    else {
        stack->setCurrentWidget(table);
    }
}

/**
 * Get the display name of the specified column type.
 *
 * @param type A column type ID
 * @return A human-readable column type name
 */
QString DBEditor::getTypeString(int type)
{
    if (type == STRING) {
        return ColumnEditor::tr("String");
    }
    else if (type == INTEGER) {
        return ColumnEditor::tr("Integer");
    }
    else if (type == FLOAT) {
        return ColumnEditor::tr("Decimal");
    }
    else if (type == BOOLEAN) {
        return ColumnEditor::tr("Boolean");
    }
    else if (type == NOTE) {
        return ColumnEditor::tr("Note");
    }
    else if (type == DATE) {
        return ColumnEditor::tr("Date");
    }
    else if (type == TIME) {
        return ColumnEditor::tr("Time");
    }
    else if (type == CALC) {
        return ColumnEditor::tr("Calculation");
    }
    else if (type == SEQUENCE) {
        return ColumnEditor::tr("Sequence");
    }
    else if (type == IMAGE) {
        return ColumnEditor::tr("Image");
    }
    else {
        return db->getEnumName(type);
    }
}

/**
 * Apply all of the changes that have been made in this dialog to the
 * database being edited.
 */
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
            if (!deletedCols.contains(oldName)) {
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
        CalcNode *calcRoot = iter.value();
        db->updateCalc(name, calcRoot, decimalsMap[name]);
        if (calcRoot != 0) {
            delete calcRoot;
        }
    }
    calcMap.clear();
}

/**
 * Override QDialog's base resizing behavior to set the display table's
 * column widths appropriately when the dialog is first resized as part of
 * initial setup.
 *
 * @param event The dialog resizing event
 */
void DBEditor::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    if (!resized) {
        int colWidth = width() / 3 - 2;
        table->setColumnWidth(0, colWidth);
        table->setColumnWidth(1, colWidth);
        table->setColumnWidth(2, colWidth);
        resized = true;
    }
}

/**
 * List all of the currently defined column names, ordered by position.
 *
 * @return The list of column names
 */
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

/**
 * List the types of each currently defined column, ordered by position.
 *
 * @return An array of column type IDs
 */
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

/**
 * Update any defined calculations to reflect a column renaming.
 *
 * @param oldName The old column name
 * @param newName The new column name
 */
void DBEditor::renameColumnRefs(const QString &oldName, const QString &newName)
{
    NameCalcMap::Iterator iter;
    for (iter = calcMap.begin(); iter != calcMap.end(); ++iter) {
        QString calcName = iter.key();
        CalcNode *calcRoot = iter.value();
        if (calcRoot != 0) {
            int index = info.Find(ceName [calcName.toUtf8()]);
            calcRoot->renameColumn(oldName, newName);
            ceDefault (info[index]) = calcRoot->equation(db).toUtf8();
        }
    }
}

/**
 * Update any defined calculations to reflect the deletion of a column.
 *
 * @param name The name of the column that was deleted
 */
void DBEditor::deleteColumnRefs(const QString &name)
{
    NameCalcMap::Iterator iter;
    QStringList deletions;
    for (iter = calcMap.begin(); iter != calcMap.end(); ++iter) {
        QString calcName = iter.key();
        CalcNode *calcRoot = iter.value();
        if (calcRoot != 0) {
            int index = info.Find(ceName [calcName.toUtf8()]);
            if (calcRoot->deleteColumn(name)) {
                delete calcRoot;
                deletions.append(calcName);
                ceDefault (info[index]) = "";
            }
            else {
                ceDefault (info[index]) = calcRoot->equation(db).toUtf8();
            }
        }
    }
    int count = deletions.count();
    for (int i = 0; i < count; i++) {
        calcMap.remove(deletions[i]);
        calcMap.insert(deletions[i], 0);
    }
}
