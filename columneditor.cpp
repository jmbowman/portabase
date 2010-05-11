/*
 * columneditor.cpp
 *
 * (c) 2002-2004,2008-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file columneditor.cpp
 * Source file for ColumnEditor
 */

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStringList>
#include <QStackedWidget>
#include "calc/calceditor.h"
#include "calc/calcnode.h"
#include "columneditor.h"
#include "database.h"
#include "dbeditor.h"
#include "enumeditor.h"
#include "factory.h"
#include "notebutton.h"
#include "numberwidget.h"

/**
 * Constructor.
 *
 * @param dbase The database in use
 * @param parent The parent DBEditor dialog
 */
ColumnEditor::ColumnEditor(Database *dbase, DBEditor *parent)
  : PBDialog(tr("Column Definition"), parent), db(dbase), dbEditor(parent),
  calcRoot(0), calcDecimals(2)
{
    QGridLayout *grid = Factory::gridLayout(vbox);
    grid->addWidget(new QLabel(tr("Name"), this), 0, 0);
    nameBox = new QLineEdit(this);
    grid->addWidget(nameBox, 0, 1);

    grid->addWidget(new QLabel(tr("Type"), this), 1, 0);
    typeBox = new QComboBox(this);
    grid->addWidget(typeBox, 1, 1);
    typeBox->addItem(tr("String"));
    typeBox->addItem(tr("Integer"));
    typeBox->addItem(tr("Decimal"));
    typeBox->addItem(tr("Boolean"));
    typeBox->addItem(tr("Note"));
    typeBox->addItem(tr("Date"));
    typeBox->addItem(tr("Time"));
    typeBox->addItem(tr("Calculation"));
    typeBox->addItem(tr("Sequence"));
    typeBox->addItem(tr("Image"));
    typeBox->addItems(db->listEnums());
    typeBox->addItem("(" + tr("New Enum") + ")");
    connect(typeBox, SIGNAL(activated(int)),
            this, SLOT(updateDefaultWidget(int)));
    lastType = 0;

    defaultLabel = new QLabel(tr("Default"), this);
    grid->addWidget(defaultLabel, 2, 0);
    defaultStack = new QStackedWidget(this);
    defaultStack->setMaximumHeight(typeBox->sizeHint().height());
    grid->addWidget(defaultStack, 2, 1);
    defaultCheck = new QCheckBox(defaultStack);
    defaultStack->addWidget(defaultCheck);
    defaultLine = new QLineEdit(defaultStack);
    defaultStack->addWidget(defaultLine);
    defaultNote = new NoteButton(tr("Default Note"), defaultStack);
    defaultStack->addWidget(defaultNote);
    defaultInteger = new NumberWidget(INTEGER, defaultStack);
    defaultStack->addWidget(defaultInteger);
    defaultFloat = new NumberWidget(FLOAT, defaultStack);
    defaultStack->addWidget(defaultFloat);
    defaultDate = new QComboBox(defaultStack);
    defaultDate->addItem(tr("Today"));
    defaultDate->addItem(tr("None"));
    defaultStack->addWidget(defaultDate);
    defaultTime = new QComboBox(defaultStack);
    defaultTime->addItem(tr("Now"));
    defaultTime->addItem(tr("None"));
    defaultStack->addWidget(defaultTime);
    defaultEnum = new QComboBox(defaultStack);
    defaultStack->addWidget(defaultEnum);
    calcButton = new QPushButton(tr("Edit calculation"), defaultStack);
    defaultStack->addWidget(calcButton);
    connect(calcButton, SIGNAL(clicked()), this, SLOT(editCalculation()));
    defaultSequence = new NumberWidget(INTEGER, defaultStack);
    defaultStack->addWidget(defaultSequence);
    defaultBlank = new QWidget(defaultStack);
    defaultStack->addWidget(defaultBlank);
    defaultStack->setCurrentWidget(defaultLine);

    finishLayout();
}

/**
 * Get the name of the column being edited.
 *
 * @return The column name, as displayed and/or entered in this dialog
 */
QString ColumnEditor::name()
{
    return nameBox->text();
}

/**
 * Set the name of the column being edited.
 *
 * @param newName The column name
 */
void ColumnEditor::setName(const QString &newName)
{
    nameBox->setText(newName);
}

/**
 * Get the type of column being edited.
 *
 * @return A column type ID
 */
int ColumnEditor::type()
{
    int colType = typeBox->currentIndex();
    if (colType > LAST_TYPE) {
        QString enumName = typeBox->itemText(colType);
        colType = db->getEnumId(enumName);
    }
    return colType;
}

/**
 * Set the type of the column being edited.
 *
 * @param newType A column type ID
 */
void ColumnEditor::setType(int newType)
{
    if (newType == lastType) {
        return;
    }
    int index = newType;
    if (newType >= FIRST_ENUM) {
        QString enumName = db->getEnumName(newType);
        int count = typeBox->count() - 1;
        for (int i = TIME + 1; i < count; i++) {
            if (typeBox->itemText(i) == enumName) {
                index = i;
                break;
            }
        }
    }
    typeBox->setCurrentIndex(index);
    updateDefaultWidget(index);
    lastType = index;
}

/**
 * Get the default value of the column being edited.
 *
 * @return The default value, as stored in the database
 */
QString ColumnEditor::defaultValue()
{
    int colType = type();
    if (colType == BOOLEAN) {
        return defaultCheck->isChecked() ? "1" : "0";
    }
    else if (colType == INTEGER) {
        return defaultInteger->getValue();
    }
    else if (colType == FLOAT) {
        return defaultFloat->getValue();
    }
    else if (colType == NOTE) {
        return defaultNote->content();
    }
    else if (colType == DATE) {
        int selection = defaultDate->currentIndex();
        if (selection == TODAY) {
            return QString::number(0);
        }
        else {
            return QString::number(17520914);
        }
    }
    else if (colType == TIME) {
        int selection = defaultTime->currentIndex();
        if (selection == NOW) {
            return QString::number(0);
        }
        else {
            return QString::number(-1);
        }
    }
    else if (colType == CALC) {
        if (calcRoot == 0) {
            return "";
        }
        return calcRoot->equation(db);
    }
    else if (colType == SEQUENCE) {
        return defaultSequence->getValue();
    }
    else if (colType == IMAGE) {
        return "";
    }
    else if (colType >= FIRST_ENUM) {
        return defaultEnum->currentText();
    }
    else {
        return defaultLine->text();
    }
}

/**
 * Set the default value of the column being edited.
 *
 * @param newDefault The default value, as stored in the database
 */
void ColumnEditor::setDefaultValue(const QString &newDefault)
{
    defaultLine->setText("");
    defaultCheck->setChecked(false);
    defaultNote->setContent("");
    defaultDate->setCurrentIndex(0);
    defaultTime->setCurrentIndex(0);
    defaultInteger->setValue("0");
    defaultFloat->setValue("0.0");
    defaultSequence->setValue("0");
    int colType = type();
    if (colType == BOOLEAN) {
        if (newDefault.toInt()) {
            defaultCheck->setChecked(true);
        }
        else {
            defaultCheck->setChecked(false);
        }
    }
    else if (colType == NOTE) {
        defaultNote->setContent(newDefault);
    }
    else if (colType == DATE) {
        if (newDefault == "0") {
            defaultDate->setCurrentIndex(0);
        }
        else {
            defaultDate->setCurrentIndex(1);
        }
    }
    else if (colType == TIME) {
        if (newDefault == "0") {
            defaultTime->setCurrentIndex(0);
        }
        else {
            defaultTime->setCurrentIndex(1);
        }
    }
    else if (colType >= FIRST_ENUM) {
        QStringList options = db->listEnumOptions(colType);
        int selection = options.indexOf(newDefault);
        defaultEnum->setCurrentIndex(selection);
    }
    else if (colType == INTEGER) {
        defaultInteger->setValue(newDefault);
    }
    else if (colType == FLOAT) {
        defaultFloat->setValue(newDefault);
    }
    else if (colType == SEQUENCE) {
        defaultSequence->setValue(newDefault);
    }
    else {
        defaultLine->setText(newDefault);
    }
}

/**
 * Get the calculation defined for this column.  Used for calculated columns.
 *
 * @param decimals A pointer to an int which will be used to store the number
 *                 of decimal places to display for calculation results.
 * @return The root node of the calculation definition tree
 */
CalcNode *ColumnEditor::calculation(int *decimals)
{
    if (decimals != 0) {
        *decimals = calcDecimals;
    }
    return calcRoot;
}

/**
 * Set the calculation to be used for this column.  Used for calculated
 * columns.
 *
 * @param root The root node of the calculation definition tree
 * @param decimals The number of decimal places to display for results
 */
void ColumnEditor::setCalculation(CalcNode *root, int decimals)
{
    calcRoot = root;
    calcDecimals = decimals;
}

/**
 * Set whether or not the column type can be changed.  This is typically
 * true for new columns and false when editing existing columns.
 *
 * @param flag True if the column type is editable, false otherwise
 */
void ColumnEditor::setTypeEditable(bool flag)
{
    typeBox->setEnabled(flag);
    defaultSequence->setEnabled(flag);
}

/**
 * Override of QDialog::exec() which ensures that the column name field
 * has focus upon launch.
 */
int ColumnEditor::exec()
{
    nameBox->setFocus();
    return QQDialog::exec();
}

/**
 * Display the correct default value widget for the specified column type.
 * Called automatically whenever a new column type is selected from the list
 * of options.
 *
 * @param newType The ID of the newly selected column type
 */
void ColumnEditor::updateDefaultWidget(int newType)
{
    defaultLabel->setText(tr("Default"));
    if (newType == typeBox->count() - 1) {
        // creating a new enum type
        EnumEditor editor(this);
        if (editor.edit(db, "")) {
            editor.applyChanges();
            typeBox->insertItem(newType, editor.getName());
            typeBox->setCurrentIndex(newType);
        }
        else {
            typeBox->setCurrentIndex(lastType);
            return;
        }
    }
    if (newType > LAST_TYPE) {
        QString enumName = typeBox->itemText(newType);
        int enumId = db->getEnumId(enumName);
        QStringList options = db->listEnumOptions(enumId);
        defaultEnum->clear();
        defaultEnum->addItems(options);
        defaultEnum->setCurrentIndex(0);
        defaultStack->setCurrentWidget(defaultEnum);
    }
    else if (newType == BOOLEAN) {
        defaultStack->setCurrentWidget(defaultCheck);
    }
    else if (newType == NOTE) {
        defaultStack->setCurrentWidget(defaultNote);
    }
    else if (newType == DATE) {
        defaultStack->setCurrentWidget(defaultDate);
    }
    else if (newType == TIME) {
        defaultStack->setCurrentWidget(defaultTime);
    }
    else if (newType == INTEGER) {
        defaultStack->setCurrentWidget(defaultInteger);
    }
    else if (newType == FLOAT) {
        defaultStack->setCurrentWidget(defaultFloat);
    }
    else if (newType == CALC) {
        defaultLabel->setText("");
        defaultStack->setCurrentWidget(calcButton);
    }
    else if (newType == SEQUENCE) {
        defaultLabel->setText(tr("Next value"));
        defaultStack->setCurrentWidget(defaultSequence);
    }
    else if (newType == IMAGE) {
        defaultLabel->setText("");
        defaultStack->setCurrentWidget(defaultBlank);
    }
    else {
        defaultStack->setCurrentWidget(defaultLine);
    }
    lastType = typeBox->currentIndex();
}

/**
 * Edit the calculation defined for this column (if it is of the appropriate
 * type).  Called automatically when the "Edit calculation" button is clicked.
 */
void ColumnEditor::editCalculation()
{
    int *types = dbEditor->columnTypes();
    CalcEditor editor(db, name(), dbEditor->columnNames(), types, this);
    if (calcRoot != 0) {
        CalcNode *calcCopy = calcRoot->clone();
        editor.load(calcCopy, calcDecimals);
    }
    if (editor.exec()) {
        // delete the original, keep the edited copy
        if (calcRoot != 0) {
            delete calcRoot;
        }
        calcRoot = editor.getRootNode();
        calcDecimals = editor.getDecimals();
    }
    else {
        // keep the original, delete the edited copy
        CalcNode *root = editor.getRootNode();
        if (root != 0) {
            delete root;
        }
    }
    delete[] types;
}
