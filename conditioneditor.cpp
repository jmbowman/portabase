/*
 * conditioneditor.cpp
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdatetime.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qvbox.h>
#include <qwidgetstack.h>
#include "condition.h"
#include "conditioneditor.h"
#include "database.h"
#include "datewidget.h"

ConditionEditor::ConditionEditor(Database *dbase, QWidget *parent, const char *name, WFlags f) : QDialog(parent, name, TRUE, f), dataType(STRING)
{
    db = dbase;
    setCaption(tr("Condition Editor") + " - " + tr("PortaBase"));
    QVBox *vbox = new QVBox(this);
    resize(parent->width() - 10, 70);
    vbox->resize(size());

    QHBox *hbox = new QHBox(vbox);
    columnList= new QComboBox(FALSE, hbox);
    columnList->insertItem(tr("Any text column"));
    colNames = db->listColumns();
    int count = colNames.count();
    types = new int[count];
    for (int i = 0; i < count; i++) {
        QString name = colNames[i];
        types[i] = db->getType(name);
        columnList->insertItem(name);
    }
    connect(columnList, SIGNAL(activated(int)),
            this, SLOT(updateDisplay(int)));
    new QWidget(hbox);

    hbox = new QHBox(vbox);
    opList = new QComboBox(FALSE, hbox);
    stringOpList.append("=");
    stringOps.append(EQUALS);
    stringOpList.append(tr("contains"));
    stringOps.append(CONTAINS);
    stringOpList.append(tr("starts with"));
    stringOps.append(STARTSWITH);
    stringOpList.append("!=");
    stringOps.append(NOTEQUAL);

    numberOpList.append("=");
    numberOps.append(EQUALS);
    numberOpList.append("!=");
    numberOps.append(NOTEQUAL);
    numberOpList.append("<");
    numberOps.append(LESSTHAN);
    numberOpList.append(">");
    numberOps.append(GREATERTHAN);
    numberOpList.append("<=");
    numberOps.append(LESSEQUAL);
    numberOpList.append(">=");
    numberOps.append(GREATEREQUAL);

    updateOpList();
    new QLabel("  ", hbox);
    caseCheck = new QCheckBox(tr("Case sensitive"), hbox);
    new QWidget(hbox);

    constantStack = new QWidgetStack(vbox);
    constantLine = new QLineEdit(constantStack);
    constantCheck = new QCheckBox(constantStack);
    constantDate = new DateWidget(constantStack);
    constantStack->raiseWidget(constantLine);
}

ConditionEditor::~ConditionEditor()
{
    delete[] types;
}

int ConditionEditor::edit(Condition *condition)
{
    fillFields(condition);
    int result = exec();
    while (result) {
        if (isValidConstant()) {
            break;
        }
        else {
            result = exec();
        }
    }
    return result;
}

void ConditionEditor::fillFields(Condition *condition)
{
    QString colName = condition->getColName();
    int operation = condition->getOperator();
    QString constant = condition->getConstant();
    int type = STRING;
    int columnIndex = 0;
    if (colName != "_anytext") {
        columnIndex = colNames.findIndex(colName) + 1;
        type = types[columnIndex - 1];
    }
    if (type != dataType) {
        dataType = type;
        updateOpList();
    }
    columnList->setCurrentItem(columnIndex);
    caseCheck->setChecked(condition->isCaseSensitive());
    if (type == BOOLEAN) {
        if (constant == "1") {
            constantCheck->setChecked(TRUE);
        }
        else {
            constantCheck->setChecked(FALSE);
        }
    }
    else if (type == INTEGER || type == FLOAT) {
        int index = numberOps.findIndex(operation);
        opList->setCurrentItem(index);
        constantLine->setText(constant);
    }
    else if (type == DATE) {
        int index = numberOps.findIndex(operation);
        opList->setCurrentItem(index);
        constantDate->setDate(constant.toInt());
    }
    else {
        int index = stringOps.findIndex(operation);
        opList->setCurrentItem(index);
        constantLine->setText(constant);
    }
    updateDisplay(columnIndex);
}

void ConditionEditor::updateDisplay(int columnIndex)
{
    int type = STRING;
    if (columnIndex > 0) {
        type = types[columnIndex - 1];
    }
    if (type == BOOLEAN) {
        constantStack->raiseWidget(constantCheck);
        constantLine->setText("");
        QDate today = QDate::currentDate();
        constantDate->setDate(today);
        caseCheck->hide();
        caseCheck->setChecked(FALSE);
    }
    else if (type == INTEGER || type == FLOAT) {
        constantStack->raiseWidget(constantLine);
        constantCheck->setChecked(FALSE);
        QDate today = QDate::currentDate();
        constantDate->setDate(today);
        caseCheck->hide();
        caseCheck->setChecked(FALSE);
    }
    else if (type == DATE) {
        constantStack->raiseWidget(constantDate);
        constantLine->setText("");
        constantCheck->setChecked(FALSE);
        caseCheck->hide();
        caseCheck->setChecked(FALSE);
    }
    else {
        constantStack->raiseWidget(constantLine);
        constantCheck->setChecked(FALSE);
        QDate today = QDate::currentDate();
        constantDate->setDate(today);
        caseCheck->show();
    }
    if (type != dataType) {
        dataType = type;
        updateOpList();
    }
}

void ConditionEditor::updateOpList()
{
    opList->clear();
    if (dataType == BOOLEAN) {
        opList->insertItem("=");
        opList->setEnabled(FALSE);
        return;
    }
    opList->setEnabled(TRUE);
    if (dataType == INTEGER || dataType == FLOAT || dataType == DATE) {
        int count = numberOpList.count();
        for (int i = 0; i < count; i++) {
            opList->insertItem(numberOpList[i]);
        }
    }
    else {
        int count = stringOpList.count();
        for (int i = 0; i < count; i++) {
            opList->insertItem(stringOpList[i]);
        }
    }
}

bool ConditionEditor::isValidConstant()
{
    bool result = TRUE;
    int index = columnList->currentItem();
    if (index > 0) {
        int type = types[index - 1];
        if (type == INTEGER || type == FLOAT) {
            QString error = db->isValidValue(type, constantLine->text());
            if (error != "") {
                QMessageBox::warning(this, tr("PortaBase"),
                                     tr("Constant") + " " + tr(error));
                result = FALSE;
            }
        }
    }
    return result;
}

void ConditionEditor::applyChanges(Condition *condition)
{
    int type = STRING;
    int index = columnList->currentItem();
    if (index == 0) {
        condition->setColName("_anytext");
    }
    else {
        condition->setColName(colNames[index - 1]);
        type = types[index - 1];
    }
    if (type == BOOLEAN) {
        condition->setOperator(EQUALS);
        if (constantCheck->isChecked()) {
            condition->setConstant("1");
        }
        else {
            condition->setConstant("0");
        }
        condition->setCaseSensitive(FALSE);
    }
    else if (type == INTEGER || type == FLOAT) {
        condition->setOperator(numberOps[opList->currentItem()]);
        condition->setConstant(constantLine->text());
        condition->setCaseSensitive(FALSE);
    }
    else if (type == DATE) {
        condition->setOperator(numberOps[opList->currentItem()]);
        condition->setConstant(QString::number(constantDate->getDate()));
        condition->setCaseSensitive(FALSE);
    }
    else {
        condition->setOperator(stringOps[opList->currentItem()]);
        condition->setConstant(constantLine->text());
        condition->setCaseSensitive(caseCheck->isChecked());
    }
    condition->updateDescription();
}
