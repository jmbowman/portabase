/*
 * conditioneditor.cpp
 *
 * (c) 2002-2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#if defined(DESKTOP)
#include <qpushbutton.h>
#include "desktop/resource.h"
#endif

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdatetime.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qwidgetstack.h>
#include "condition.h"
#include "conditioneditor.h"
#include "database.h"
#include "datewidget.h"
#include "timewidget.h"

ConditionEditor::ConditionEditor(Database *dbase, QWidget *parent, const char *name, WFlags f) : QDialog(parent, name, TRUE, f), dataType(STRING)
{
    db = dbase;
    setCaption(tr("Condition Editor") + " - " + tr("PortaBase"));
    QVBoxLayout *vbox = new QVBoxLayout(this);

    QHBox *hbox = new QHBox(this);
    vbox->addWidget(hbox);
    columnList= new QComboBox(FALSE, hbox);
    columnList->insertItem(tr("Any text column"));
    colNames = db->listColumns();
    int count = colNames.count();
    types = db->listTypes();
    for (int i = 0; i < count; i++) {
        QString name = colNames[i];
        columnList->insertItem(name);
    }
    connect(columnList, SIGNAL(activated(int)),
            this, SLOT(updateDisplay(int)));
    new QWidget(hbox);

    hbox = new QHBox(this);
    vbox->addWidget(hbox);
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

    enumOpList.append("=");
    enumOps.append(EQUALS);
    enumOpList.append("!=");
    enumOps.append(NOTEQUAL);

    updateOpList();
    new QLabel("  ", hbox);
    caseCheck = new QCheckBox(tr("Case sensitive"), hbox);
    new QWidget(hbox);

    constantStack = new QWidgetStack(this);
    constantStack->setMaximumHeight(columnList->height());
    vbox->addWidget(constantStack);
    constantLine = new QLineEdit(constantStack);
    constantCheck = new QCheckBox(constantStack);
    constantDate = new DateWidget(constantStack);
    constantTime = new TimeWidget(constantStack);
    constantCombo = new QComboBox(FALSE, constantStack);
    constantStack->raiseWidget(constantLine);

#if defined(DESKTOP)
    hbox = new QHBox(this);
    new QWidget(hbox);
    QPushButton *okButton = new QPushButton(tr("OK"), hbox);
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    new QWidget(hbox);
    QPushButton *cancelButton = new QPushButton(tr("Cancel"), hbox);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    new QWidget(hbox);
    vbox->addWidget(hbox);
    setIcon(Resource::loadPixmap("portabase"));
#endif
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
    else if (type == TIME) {
        int index = numberOps.findIndex(operation);
        opList->setCurrentItem(index);
        constantTime->setTime(constant.toInt());
    }
    else if (type >= FIRST_ENUM) {
        int index = enumOps.findIndex(operation);
        opList->setCurrentItem(index);
        QStringList options = db->listEnumOptions(type);
        constantCombo->clear();
        constantCombo->insertStringList(options);
        index = options.findIndex(constant);
        constantCombo->setCurrentItem(index);
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
        QTime now = QTime::currentTime();
        constantTime->setTime(now);
        caseCheck->hide();
        caseCheck->setChecked(FALSE);
    }
    else if (type == INTEGER || type == FLOAT) {
        constantStack->raiseWidget(constantLine);
        constantCheck->setChecked(FALSE);
        QDate today = QDate::currentDate();
        constantDate->setDate(today);
        QTime now = QTime::currentTime();
        constantTime->setTime(now);
        caseCheck->hide();
        caseCheck->setChecked(FALSE);
    }
    else if (type == DATE) {
        constantStack->raiseWidget(constantDate);
        constantLine->setText("");
        constantCheck->setChecked(FALSE);
        QTime now = QTime::currentTime();
        constantTime->setTime(now);
        caseCheck->hide();
        caseCheck->setChecked(FALSE);
    }
    else if (type == TIME) {
        constantStack->raiseWidget(constantTime);
        constantLine->setText("");
        constantCheck->setChecked(FALSE);
        QDate today = QDate::currentDate();
        constantDate->setDate(today);
        caseCheck->hide();
        caseCheck->setChecked(FALSE);
    }
    else if (type >= FIRST_ENUM) {
        constantCombo->clear();
        constantCombo->insertStringList(db->listEnumOptions(type));
        constantStack->raiseWidget(constantCombo);
        constantLine->setText("");
        constantCheck->setChecked(FALSE);
        QDate today = QDate::currentDate();
        constantDate->setDate(today);
        QTime now = QTime::currentTime();
        constantTime->setTime(now);
        caseCheck->hide();
        caseCheck->setChecked(FALSE);
    }
    else {
        constantStack->raiseWidget(constantLine);
        constantCheck->setChecked(FALSE);
        QDate today = QDate::currentDate();
        constantDate->setDate(today);
        QTime now = QTime::currentTime();
        constantTime->setTime(now);
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
    if (dataType == INTEGER || dataType == FLOAT || dataType == DATE
            || dataType == TIME) {
        opList->insertStringList(numberOpList);
    }
    else if (dataType >= FIRST_ENUM) {
        opList->insertStringList(enumOpList);
    }
    else {
        opList->insertStringList(stringOpList);
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
                                     tr("Constant") + " " + error);
                result = FALSE;
            }
        }
        else if (type == TIME) {
            QString error = db->isValidValue(type, constantTime->getTime());
            if (error != "") {
                QMessageBox::warning(this, tr("PortaBase"),
                                     tr("Constant") + " " + error);
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
    else if (type == TIME) {
        condition->setOperator(numberOps[opList->currentItem()]);
        bool ok;
        condition->setConstant(db->parseTimeString(constantTime->getTime(),
                                                   &ok));
        condition->setCaseSensitive(FALSE);
    }
    else if (type >= FIRST_ENUM) {
        condition->setOperator(enumOps[opList->currentItem()]);
        condition->setConstant(constantCombo->currentText());
        condition->setCaseSensitive(TRUE);
    }
    else {
        condition->setOperator(stringOps[opList->currentItem()]);
        condition->setConstant(constantLine->text());
        condition->setCaseSensitive(caseCheck->isChecked());
    }
    condition->updateDescription();
}
