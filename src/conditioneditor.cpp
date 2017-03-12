/*
 * conditioneditor.cpp
 *
 * (c) 2002-2004,2008-2010,2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file conditioneditor.cpp
 * Source file for ConditionEditor
 */

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDate>
#include <QLabel>
#include <QMessageBox>
#include <QTime>
#include <QStackedWidget>
#include "conditioneditor.h"
#include "database.h"
#include "datewidget.h"
#include "factory.h"
#include "formatting.h"
#include "numberwidget.h"
#include "qqutil/qqlineedit.h"
#include "timewidget.h"

/**
 * Constructor.
 *
 * @param dbase The database in use
 * @param parent The dialog's parent widget, if any
 * @param quickFilter True if the dialog is being used directly from the data
 *                    viewer, false if it's from a filter editor
 */
ConditionEditor::ConditionEditor(Database *dbase, QWidget *parent, bool quickFilter)
  : PBDialog(tr("Condition Editor"), parent), db(dbase), dataType(STRING)
{
#if defined(Q_WS_HILDON) || defined(Q_WS_MAEMO_5)
    vbox->addStretch(1);
#endif
    columnList= Factory::comboBox(this);
    vbox->addWidget(columnList);
    columnList->addItem(tr("Any text column"));
    QStringList tempNames = db->listColumns();
    int count = tempNames.count();
    IntList tempTypes = db->listTypes();
    for (int i = 0; i < count; i++) {
        if (tempTypes[i] != IMAGE) {
            QString name = tempNames[i];
            colNames.append(name);
            types.append(tempTypes[i]);
            columnList->addItem(name);
        }
    }
    connect(columnList, SIGNAL(activated(int)),
            this, SLOT(updateDisplay(int)));

    QHBoxLayout *hbox = Factory::hBoxLayout(vbox);
    opList = Factory::comboBox(this);
    opList->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    hbox->addWidget(opList);
    stringOpList.append("=");
    stringOps.append(Condition::Equals);
    stringOpList.append(tr("contains"));
    stringOps.append(Condition::Contains);
    stringOpList.append(tr("starts with"));
    stringOps.append(Condition::StartsWith);
    stringOpList.append("!=");
    stringOps.append(Condition::NotEqual);

    numberOpList.append("=");
    numberOps.append(Condition::Equals);
    numberOpList.append("!=");
    numberOps.append(Condition::NotEqual);
    numberOpList.append("<");
    numberOps.append(Condition::LessThan);
    numberOpList.append(">");
    numberOps.append(Condition::GreaterThan);
    numberOpList.append("<=");
    numberOps.append(Condition::LessEqual);
    numberOpList.append(">=");
    numberOps.append(Condition::GreaterEqual);

    updateOpList();
    caseCheck = new QCheckBox(tr("Case sensitive"), this);
    hbox->addWidget(caseCheck);

    constantStack = new QStackedWidget(this);
    constantStack->setMaximumHeight(columnList->sizeHint().height());
    vbox->addWidget(constantStack);
    constantLine = new QQLineEdit(constantStack);
    constantStack->addWidget(constantLine);
    constantCheck = new QCheckBox(constantStack);
    constantStack->addWidget(constantCheck);
    constantDate = new DateWidget(constantStack);
    constantStack->addWidget(constantDate);
    constantTime = new TimeWidget(constantStack);
    constantStack->addWidget(constantTime);
    constantInteger = new NumberWidget(INTEGER, constantStack);
    constantStack->addWidget(constantInteger);
    constantFloat = new NumberWidget(FLOAT, constantStack);
    constantStack->addWidget(constantFloat);
    constantCombo = Factory::comboBox(constantStack);
    constantStack->addWidget(constantCombo);
    constantStack->setCurrentWidget(constantLine);
#if defined(Q_OS_ANDROID)
    Qt::EnterKeyType enterKeyType = Qt::EnterKeyDone;
    if (quickFilter) {
        enterKeyType = Qt::EnterKeySearch;
    }
    constantLine->setEnterKeyType(enterKeyType);
    constantInteger->setEnterKeyType(enterKeyType);
    constantFloat->setEnterKeyType(enterKeyType);
#else
    Q_UNUSED(quickFilter)
#endif

#ifdef MOBILE
    vbox->addStretch(1);
#endif
    finishLayout();
}

/**
 * Launch this dialog with the appropriate values for the specified condition
 * already selected.
 *
 * @param condition The condition to be edited
 * @return The integer code returned by exec(), indicating whether the changes
 *         are to be applied or not
 */
int ConditionEditor::edit(Condition *condition)
{
    fillFields(condition);
    int result = exec();
    while (result) {
        if (isValidConstant()) {
            break;
        }
        else {
            // relaunch if an invalid constant was entered
            result = exec();
        }
    }
    return result;
}

/**
 * Update the dialog's entry widgets to match the specified condition.
 *
 * @param condition The condition to be displayed in the dialog
 */
void ConditionEditor::fillFields(Condition *condition)
{
    QString colName = condition->getColName();
    Condition::Operator operation = condition->getOperator();
    QString constant = condition->getConstant();
    int type = STRING;
    int columnIndex = 0;
    if (colName != "_anytext") {
        columnIndex = colNames.indexOf(colName) + 1;
        type = types[columnIndex - 1];
    }
    if (type != dataType) {
        dataType = type;
        updateOpList();
    }
    columnList->setCurrentIndex(columnIndex);
    caseCheck->setChecked(condition->isCaseSensitive());
    updateDisplay(columnIndex);
    if (type == BOOLEAN) {
        if (constant == "1") {
            constantCheck->setChecked(true);
        }
        else {
            constantCheck->setChecked(false);
        }
    }
    else if (type == INTEGER || type == SEQUENCE) {
        int index = numberOps.indexOf(operation);
        opList->setCurrentIndex(index);
        constantInteger->setValue(constant);
    }
    else if (type == FLOAT || type == CALC) {
        int index = numberOps.indexOf(operation);
        opList->setCurrentIndex(index);
        constantFloat->setValue(constant);
    }
    else if (type == DATE) {
        int index = numberOps.indexOf(operation);
        opList->setCurrentIndex(index);
        constantDate->setDate(constant.toInt());
    }
    else if (type == TIME) {
        int index = numberOps.indexOf(operation);
        opList->setCurrentIndex(index);
        constantTime->setTime(constant.toInt());
    }
    else if (type >= FIRST_ENUM) {
        int index = numberOps.indexOf(operation);
        opList->setCurrentIndex(index);
        QStringList options = db->listEnumOptions(type);
        constantCombo->clear();
        constantCombo->addItems(options);
        index = options.indexOf(constant);
        constantCombo->setCurrentIndex(index);
    }
    else {
        int index = stringOps.indexOf(operation);
        opList->setCurrentIndex(index);
        constantLine->setText(constant);
    }
}

/**
 * Update the dialog fields to reflect the selection of a new column.
 *
 * @param columnIndex The index in columnList of the selected column
 */
void ConditionEditor::updateDisplay(int columnIndex)
{
    int type = STRING;
    if (columnIndex > 0) {
        type = types[columnIndex - 1];
    }
    if (type == BOOLEAN) {
        constantStack->setCurrentWidget(constantCheck);
        constantLine->setText("");
        QDate today = QDate::currentDate();
        constantDate->setDate(today);
        QTime now = QTime::currentTime();
        constantTime->setTime(now);
        caseCheck->hide();
        caseCheck->setChecked(false);
        constantInteger->setValue("0");
        constantFloat->setValue("0");
    }
    else if (type == INTEGER || type == SEQUENCE) {
        constantStack->setCurrentWidget(constantInteger);
        constantLine->setText("");
        constantCheck->setChecked(false);
        QDate today = QDate::currentDate();
        constantDate->setDate(today);
        QTime now = QTime::currentTime();
        constantTime->setTime(now);
        caseCheck->hide();
        caseCheck->setChecked(false);
        constantFloat->setValue("0");
    }
    else if (type == FLOAT || type == CALC) {
        constantStack->setCurrentWidget(constantFloat);
        constantLine->setText("");
        constantCheck->setChecked(false);
        QDate today = QDate::currentDate();
        constantDate->setDate(today);
        QTime now = QTime::currentTime();
        constantTime->setTime(now);
        caseCheck->hide();
        caseCheck->setChecked(false);
        constantInteger->setValue("0");
    }
    else if (type == DATE) {
        constantStack->setCurrentWidget(constantDate);
        constantLine->setText("");
        constantCheck->setChecked(false);
        QTime now = QTime::currentTime();
        constantTime->setTime(now);
        caseCheck->hide();
        caseCheck->setChecked(false);
        constantInteger->setValue("0");
        constantFloat->setValue("0");
    }
    else if (type == TIME) {
        constantStack->setCurrentWidget(constantTime);
        constantLine->setText("");
        constantCheck->setChecked(false);
        QDate today = QDate::currentDate();
        constantDate->setDate(today);
        caseCheck->hide();
        caseCheck->setChecked(false);
        constantInteger->setValue("0");
        constantFloat->setValue("0");
    }
    else if (type >= FIRST_ENUM) {
        if (type != dataType || constantStack->currentWidget() != constantCombo) {
            constantCombo->clear();
            constantCombo->addItems(db->listEnumOptions(type));
            constantStack->setCurrentWidget(constantCombo);
            constantLine->setText("");
            constantCheck->setChecked(false);
            QDate today = QDate::currentDate();
            constantDate->setDate(today);
            QTime now = QTime::currentTime();
            constantTime->setTime(now);
            caseCheck->hide();
            caseCheck->setChecked(false);
            constantInteger->setValue("0");
            constantFloat->setValue("0");
        }
    }
    else {
        constantStack->setCurrentWidget(constantLine);
        constantCheck->setChecked(false);
        QDate today = QDate::currentDate();
        constantDate->setDate(today);
        QTime now = QTime::currentTime();
        constantTime->setTime(now);
        caseCheck->show();
        constantInteger->setValue("0");
        constantFloat->setValue("0");
    }
    if (type != dataType) {
        dataType = type;
        updateOpList();
    }
}

/**
 * Update the operator selection widget to show only operators which are
 * valid for the currently selected column.
 */
void ConditionEditor::updateOpList()
{
    opList->clear();
    if (dataType == BOOLEAN) {
        opList->addItem("=");
        opList->setEnabled(false);
        return;
    }
    opList->setEnabled(true);
    if (dataType == INTEGER || dataType == FLOAT || dataType == DATE
            || dataType == TIME || dataType == CALC || dataType == SEQUENCE
            || dataType >= FIRST_ENUM) {
        opList->addItems(numberOpList);
    }
    else {
        opList->addItems(stringOpList);
    }
}

/**
 * Check the entered constant to make sure it is a valid value, for those
 * data types where it is actually possible to enter something invalid.
 *
 * @return True if valid, false otherwise
 */
bool ConditionEditor::isValidConstant()
{
    bool result = true;
    int index = columnList->currentIndex();
    if (index > 0) {
        int type = types[index - 1];
        QString error("");
        if (type == INTEGER || type == SEQUENCE) {
            error = db->isValidValue(type, constantInteger->getValue());
        }
        else if (type == FLOAT || type == CALC) {
            error = db->isValidValue(type, constantFloat->getValue());
        }
        else if (type == TIME) {
            error = db->isValidValue(type, constantTime->getTime());
        }
        if (!error.isEmpty()) {
            QMessageBox::warning(this, qApp->applicationName(),
                                 tr("Constant") + " " + error);
            result = false;
        }
    }
    return result;
}

/**
 * Apply the settings currently shown in the dialog to the specified
 * condition.
 *
 * @param condition The condition to be updated
 */
void ConditionEditor::applyChanges(Condition *condition)
{
    int type = STRING;
    int index = columnList->currentIndex();
    if (index == 0) {
        condition->setColName("_anytext");
    }
    else {
        condition->setColName(colNames[index - 1]);
        type = types[index - 1];
    }
    if (type == BOOLEAN) {
        condition->setOperator(Condition::Equals);
        if (constantCheck->isChecked()) {
            condition->setConstant("1");
        }
        else {
            condition->setConstant("0");
        }
        condition->setCaseSensitive(false);
    }
    else if (type == INTEGER || type == SEQUENCE) {
        condition->setOperator(numberOps[opList->currentIndex()]);
        condition->setConstant(constantInteger->getValue());
        condition->setCaseSensitive(false);
    }
    else if (type == FLOAT || type == CALC) {
        condition->setOperator(numberOps[opList->currentIndex()]);
        condition->setConstant(constantFloat->getValue());
        condition->setCaseSensitive(false);
    }
    else if (type == DATE) {
        condition->setOperator(numberOps[opList->currentIndex()]);
        condition->setConstant(QString::number(constantDate->getDate()));
        condition->setCaseSensitive(false);
    }
    else if (type == TIME) {
        condition->setOperator(numberOps[opList->currentIndex()]);
        bool ok;
        condition->setConstant(Formatting::parseTimeString(constantTime->getTime(),
                                                           &ok));
        condition->setCaseSensitive(false);
    }
    else if (type >= FIRST_ENUM) {
        condition->setOperator(numberOps[opList->currentIndex()]);
        condition->setConstant(constantCombo->currentText());
        condition->setCaseSensitive(true);
    }
    else {
        condition->setOperator(stringOps[opList->currentIndex()]);
        condition->setConstant(constantLine->text());
        condition->setCaseSensitive(caseCheck->isChecked());
    }
    condition->updateDescription();
}
