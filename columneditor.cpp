/*
 * columneditor.cpp
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
#include <qgrid.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qwidgetstack.h>
#include "columneditor.h"
#include "database.h"
#include "enumeditor.h"
#include "notebutton.h"

ColumnEditor::ColumnEditor(Database *dbase, QWidget *parent, const char *name, WFlags f) : QDialog(parent, name, TRUE, f)
{
    db = dbase;
    setCaption(tr("PortaBase"));
    QGrid *grid = new QGrid(2, this);
    resize(200, 68);
    grid->resize(size());

    new QLabel(tr("Name"), grid);
    nameBox = new QLineEdit(grid);

    new QLabel(tr("Type"), grid);
    typeBox = new QComboBox(FALSE, grid);
    typeBox->insertItem(tr("String"));
    typeBox->insertItem(tr("Integer"));
    typeBox->insertItem(tr("Decimal"));
    typeBox->insertItem(tr("Boolean"));
    typeBox->insertItem(tr("Note"));
    typeBox->insertItem(tr("Date"));
    typeBox->insertItem(tr("Time"));
    typeBox->insertStringList(db->listEnums());
    typeBox->insertItem("(" + tr("New Enum") + ")");
    connect(typeBox, SIGNAL(activated(int)),
            this, SLOT(updateDefaultWidget(int)));
    lastType = 0;

    new QLabel(tr("Default"), grid);
    defaultStack = new QWidgetStack(grid);
    defaultCheck = new QCheckBox(defaultStack);
    defaultLine = new QLineEdit(defaultStack);
    defaultNote = new NoteButton(tr("Default Note"), defaultStack);
    defaultDate = new QComboBox(FALSE, defaultStack);
    defaultDate->insertItem(tr("Today"));
    defaultDate->insertItem(tr("None"));
    defaultTime = new QComboBox(FALSE, defaultStack);
    defaultTime->insertItem(tr("Now"));
    defaultTime->insertItem(tr("None"));
    defaultEnum = new QComboBox(FALSE, defaultStack);
    defaultStack->raiseWidget(defaultLine);
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
    if (colType > TIME) {
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
    else if (colType >= FIRST_ENUM) {
        return defaultEnum->currentText();
    }
    else {
        return defaultLine->text();
    }
}

void ColumnEditor::setDefaultValue(QString newDefault)
{
    int colType = type();
    if (colType == BOOLEAN) {
        if (newDefault.toInt()) {
            defaultCheck->setChecked(TRUE);
        }
        else {
            defaultCheck->setChecked(FALSE);
        }
        defaultLine->setText("");
        defaultNote->setContent("");
        defaultDate->setCurrentItem(0);
        defaultTime->setCurrentItem(0);
    }
    else if (colType == NOTE) {
        defaultLine->setText("");
        defaultCheck->setChecked(FALSE);
        defaultNote->setContent(newDefault);
        defaultDate->setCurrentItem(0);
        defaultTime->setCurrentItem(0);
    }
    else if (colType == DATE) {
        defaultLine->setText("");
        defaultCheck->setChecked(FALSE);
        defaultNote->setContent("");
        if (newDefault == "0") {
            defaultDate->setCurrentItem(0);
        }
        else {
            defaultDate->setCurrentItem(1);
        }
        defaultTime->setCurrentItem(0);
    }
    else if (colType == TIME) {
        defaultLine->setText("");
        defaultCheck->setChecked(FALSE);
        defaultNote->setContent("");
        defaultDate->setCurrentItem(0);
        if (newDefault == "0") {
            defaultTime->setCurrentItem(0);
        }
        else {
            defaultTime->setCurrentItem(1);
        }
    }
    else if (colType >= FIRST_ENUM) {
        defaultLine->setText("");
        defaultCheck->setChecked(FALSE);
        defaultNote->setContent("");
        defaultDate->setCurrentItem(0);
        defaultTime->setCurrentItem(0);
        QStringList options = db->listEnumOptions(colType);
        int selection = options.findIndex(newDefault);
        defaultEnum->setCurrentItem(selection);
    }
    else {
        defaultLine->setText(newDefault);
        defaultCheck->setChecked(FALSE);
        defaultNote->setContent("");
        defaultDate->setCurrentItem(0);
        defaultTime->setCurrentItem(0);
    }
}

void ColumnEditor::setTypeEditable(bool flag)
{
    typeBox->setEnabled(flag);
}

void ColumnEditor::updateDefaultWidget(int newType)
{
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
    if (newType > TIME) {
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
    else {
        defaultStack->raiseWidget(defaultLine);
        if (newType == INTEGER) {
            bool ok;
            defaultLine->text().toInt(&ok);
            if (!ok) {
                defaultLine->setText("0");
            }
        }
        else if (newType == FLOAT) {
            bool ok;
            defaultLine->text().toDouble(&ok);
            if (!ok) {
                defaultLine->setText("0.0");
            }
        }
    }
}
