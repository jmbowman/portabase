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
#include "notebutton.h"

ColumnEditor::ColumnEditor(QWidget *parent, const char *name, WFlags f)
    : QDialog(parent, name, TRUE, f)
{
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
    connect(typeBox, SIGNAL(activated(int)),
            this, SLOT(updateDefaultWidget(int)));

    new QLabel(tr("Default"), grid);
    defaultStack = new QWidgetStack(grid);
    defaultCheck = new QCheckBox(defaultStack);
    defaultLine = new QLineEdit(defaultStack);
    defaultNote = new NoteButton(tr("Default Note"), defaultStack);
    defaultDate = new QLabel("  " + tr("Today"), defaultStack);
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
    else if (newType == NOTE) {
        defaultStack->raiseWidget(defaultNote);
    }
    else if (newType == DATE) {
        defaultStack->raiseWidget(defaultDate);
    }
    else {
        defaultStack->raiseWidget(defaultLine);
    }
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
        return QString::number(TODAY);
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
    }
    else if (colType == NOTE) {
        defaultLine->setText("");
        defaultCheck->setChecked(FALSE);
        defaultNote->setContent(newDefault);
    }
    else if (colType == DATE) {
        defaultLine->setText("");
        defaultCheck->setChecked(FALSE);
        defaultNote->setContent("");
    }
    else {
        defaultLine->setText(newDefault);
        defaultCheck->setChecked(FALSE);
        defaultNote->setContent("");
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
    else if (newType == NOTE) {
        defaultStack->raiseWidget(defaultNote);
    }
    else if (newType == DATE) {
        defaultStack->raiseWidget(defaultDate);
    }
    else {
        defaultStack->raiseWidget(defaultLine);
    }
}
