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
#include "datatypes.h"

ColumnEditor::ColumnEditor(QWidget *parent, const char *name, WFlags f)
    : QDialog(parent, name, TRUE, f)
{
    setCaption(tr("PortaBase"));
    QGrid *grid = new QGrid(2, this);
    resize(120, 68);
    grid->resize(size());

    new QLabel(tr("Name"), grid);
    nameBox = new QLineEdit(grid);

    new QLabel(tr("Type"), grid);
    typeBox = new QComboBox(FALSE, grid);
    typeBox->insertItem(tr("String"));
    typeBox->insertItem(tr("Integer"));
    typeBox->insertItem(tr("Decimal"));
    typeBox->insertItem(tr("Boolean"));
    connect(typeBox, SIGNAL(activated(int)),
            this, SLOT(updateDefaultWidget(int)));

    new QLabel(tr("Default"), grid);
    defaultStack = new QWidgetStack(grid);
    defaultCheck = new QCheckBox(defaultStack);
    defaultLine = new QLineEdit(defaultStack);
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
    return typeBox->currentItem();
}

void ColumnEditor::setType(int newType)
{
    typeBox->setCurrentItem(newType);
    if (newType == BOOLEAN) {
        defaultStack->raiseWidget(defaultCheck);
    }
    else {
        defaultStack->raiseWidget(defaultLine);
    }
}

QString ColumnEditor::defaultValue()
{
    if (type() == BOOLEAN) {
        return defaultCheck->isChecked() ? "1" : "0";
    }
    else {
        return defaultLine->text();
    }
}

void ColumnEditor::setDefaultValue(QString newDefault)
{
    if (type() == BOOLEAN) {
        if (newDefault.toInt()) {
            defaultCheck->setChecked(TRUE);
        }
        else {
            defaultCheck->setChecked(FALSE);
        }
        defaultLine->setText("");
    }
    else {
        defaultLine->setText(newDefault);
        defaultCheck->setChecked(FALSE);
    }
}

void ColumnEditor::setTypeEditable(bool flag)
{
    typeBox->setEnabled(flag);
}

void ColumnEditor::updateDefaultWidget(int newType)
{
    if (newType == BOOLEAN) {
        defaultStack->raiseWidget(defaultCheck);
    }
    else {
        defaultStack->raiseWidget(defaultLine);
    }
}
