/*
 * columneditor.cpp
 *
 * (c) 2002-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#if !defined(Q_WS_QWS)
#include <qhbox.h>
#include <qpushbutton.h>
#endif

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qstringlist.h>
#include <qwidgetstack.h>
#include "calc/calceditor.h"
#include "calc/calcnode.h"
#include "columneditor.h"
#include "database.h"
#include "dbeditor.h"
#include "enumeditor.h"
#include "notebutton.h"
#include "numberwidget.h"

ColumnEditor::ColumnEditor(Database *dbase, DBEditor *parent, const char *name)
  : QQDialog("", parent, name, TRUE), db(dbase), dbEditor(parent), calcRoot(0), calcDecimals(2)
{
#if defined(Q_WS_QWS)
    QGridLayout *grid = new QGridLayout(this, 3, 2);
#else
    QGridLayout *grid = new QGridLayout(this, 4, 2);
#endif

    grid->addWidget(new QLabel(tr("Name"), this), 0, 0);
    nameBox = new QLineEdit(this);
    grid->addWidget(nameBox, 0, 1);

    grid->addWidget(new QLabel(tr("Type"), this), 1, 0);
    typeBox = new QComboBox(FALSE, this);
    grid->addWidget(typeBox, 1, 1);
    typeBox->insertItem(tr("String"));
    typeBox->insertItem(tr("Integer"));
    typeBox->insertItem(tr("Decimal"));
    typeBox->insertItem(tr("Boolean"));
    typeBox->insertItem(tr("Note"));
    typeBox->insertItem(tr("Date"));
    typeBox->insertItem(tr("Time"));
    typeBox->insertItem(tr("Calculation"));
    typeBox->insertItem(tr("Sequence"));
    typeBox->insertItem(tr("Image"));
    typeBox->insertStringList(db->listEnums());
    typeBox->insertItem("(" + tr("New Enum") + ")");
    connect(typeBox, SIGNAL(activated(int)),
            this, SLOT(updateDefaultWidget(int)));
    lastType = 0;

    defaultLabel = new QLabel(tr("Default"), this);
    grid->addWidget(defaultLabel, 2, 0);
    defaultStack = new QWidgetStack(this);
    defaultStack->setMaximumHeight(typeBox->height());
    grid->addWidget(defaultStack, 2, 1);
    defaultCheck = new QCheckBox(defaultStack);
    defaultLine = new QLineEdit(defaultStack);
    defaultNote = new NoteButton(tr("Default Note"), defaultStack);
    defaultInteger = new NumberWidget(INTEGER, defaultStack);
    defaultFloat = new NumberWidget(FLOAT, defaultStack);
    defaultDate = new QComboBox(FALSE, defaultStack);
    defaultDate->insertItem(tr("Today"));
    defaultDate->insertItem(tr("None"));
    defaultTime = new QComboBox(FALSE, defaultStack);
    defaultTime->insertItem(tr("Now"));
    defaultTime->insertItem(tr("None"));
    defaultEnum = new QComboBox(FALSE, defaultStack);
    calcButton = new QPushButton(tr("Edit calculation"), defaultStack);
    connect(calcButton, SIGNAL(clicked()), this, SLOT(editCalculation()));
    defaultSequence = new NumberWidget(INTEGER, defaultStack);
    defaultBlank = new QWidget(defaultStack);
    defaultStack->raiseWidget(defaultLine);

    int minWidth = -1;
#if !defined(Q_WS_QWS)
    QHBox *hbox = new QHBox(this);
    new QWidget(hbox);
    QPushButton *okButton = new QPushButton(tr("OK"), hbox);
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    new QWidget(hbox);
    QPushButton *cancelButton = new QPushButton(tr("Cancel"), hbox);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    new QWidget(hbox);
    grid->addMultiCellWidget(hbox, 3, 3, 0, 1);
    grid->setResizeMode(QLayout::FreeResize);
    minWidth = parent->width() / 2;
#endif
    finishConstruction(FALSE, minWidth);
}

ColumnEditor::~ColumnEditor()
{

}

QString ColumnEditor::name()
{
    return nameBox->text();
}

void ColumnEditor::setName(QString newName)
{
    nameBox->setText(newName);
}

int ColumnEditor::type()
{
    int colType = typeBox->currentItem();
    if (colType > LAST_TYPE) {
        QString enumName = typeBox->text(colType);
        colType = db->getEnumId(enumName);
    }
    return colType;
}

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
            if (typeBox->text(i) == enumName) {
                index = i;
                break;
            }
        }
    }
    typeBox->setCurrentItem(index);
    updateDefaultWidget(index);
    lastType = index;
}

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
        int selection = defaultDate->currentItem();
        if (selection == TODAY) {
            return QString::number(0);
        }
        else {
            return QString::number(17520914);
        }
    }
    else if (colType == TIME) {
        int selection = defaultTime->currentItem();
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

void ColumnEditor::setDefaultValue(QString newDefault)
{
    defaultLine->setText("");
    defaultCheck->setChecked(FALSE);
    defaultNote->setContent("");
    defaultDate->setCurrentItem(0);
    defaultTime->setCurrentItem(0);
    defaultInteger->setValue("0");
    defaultFloat->setValue("0");
    defaultSequence->setValue("0");
    int colType = type();
    if (colType == BOOLEAN) {
        if (newDefault.toInt()) {
            defaultCheck->setChecked(TRUE);
        }
        else {
            defaultCheck->setChecked(FALSE);
        }
    }
    else if (colType == NOTE) {
        defaultNote->setContent(newDefault);
    }
    else if (colType == DATE) {
        if (newDefault == "0") {
            defaultDate->setCurrentItem(0);
        }
        else {
            defaultDate->setCurrentItem(1);
        }
    }
    else if (colType == TIME) {
        if (newDefault == "0") {
            defaultTime->setCurrentItem(0);
        }
        else {
            defaultTime->setCurrentItem(1);
        }
    }
    else if (colType >= FIRST_ENUM) {
        QStringList options = db->listEnumOptions(colType);
        int selection = options.findIndex(newDefault);
        defaultEnum->setCurrentItem(selection);
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

CalcNode *ColumnEditor::calculation(int *decimals)
{
    if (decimals != 0) {
        *decimals = calcDecimals;
    }
    return calcRoot;
}

void ColumnEditor::setCalculation(CalcNode *root, int decimals)
{
    calcRoot = root;
    calcDecimals = decimals;
}

void ColumnEditor::setTypeEditable(bool flag)
{
    typeBox->setEnabled(flag);
    defaultSequence->setEnabled(flag);
}

int ColumnEditor::exec()
{
    nameBox->setFocus();
    return QDialog::exec();
}

void ColumnEditor::updateDefaultWidget(int newType)
{
    defaultLabel->setText(tr("Default"));
    if (newType == typeBox->count() - 1) {
        // creating a new enum type
        EnumEditor editor(this);
        if (editor.edit(db, "")) {
            editor.applyChanges();
            typeBox->insertItem(editor.getName(), newType);
            typeBox->setCurrentItem(newType);
        }
        else {
            typeBox->setCurrentItem(lastType);
            return;
        }
    }
    if (newType > LAST_TYPE) {
        QString enumName = typeBox->text(newType);
        int enumId = db->getEnumId(enumName);
        QStringList options = db->listEnumOptions(enumId);
        defaultEnum->clear();
        defaultEnum->insertStringList(options);
        defaultEnum->setCurrentItem(0);
        defaultStack->raiseWidget(defaultEnum);
    }
    else if (newType == BOOLEAN) {
        defaultStack->raiseWidget(defaultCheck);
    }
    else if (newType == NOTE) {
        defaultStack->raiseWidget(defaultNote);
    }
    else if (newType == DATE) {
        defaultStack->raiseWidget(defaultDate);
    }
    else if (newType == TIME) {
        defaultStack->raiseWidget(defaultTime);
    }
    else if (newType == INTEGER) {
        defaultStack->raiseWidget(defaultInteger);
    }
    else if (newType == FLOAT) {
        defaultStack->raiseWidget(defaultFloat);
    }
    else if (newType == CALC) {
        defaultLabel->setText("");
        defaultStack->raiseWidget(calcButton);
    }
    else if (newType == SEQUENCE) {
        defaultLabel->setText(tr("Next value"));
        defaultStack->raiseWidget(defaultSequence);
    }
    else if (newType == IMAGE) {
        defaultLabel->setText("");
        defaultStack->raiseWidget(defaultBlank);
    }
    else {
        defaultStack->raiseWidget(defaultLine);
    }
    lastType = typeBox->currentItem();
}

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
