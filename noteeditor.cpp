/*
 * noteeditor.cpp
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qmultilineedit.h>
#include "noteeditor.h"

NoteEditor::NoteEditor(QString colName, QWidget *parent, const char *name,
    WFlags f) : QDialog(parent, name, TRUE, f)
{
    setCaption(tr("PortaBase") + " - " + colName);
    showMaximized();
    hide();
    textBox = new QMultiLineEdit(this);
    textBox->resize(size());
}

NoteEditor::~NoteEditor()
{

}

QString NoteEditor::content()
{
    return textBox->text();
}

void NoteEditor::setContent(QString text)
{
    textBox->setText(text);
}

void NoteEditor::setReadOnly(bool y)
{
    textBox->setReadOnly(y);
}
