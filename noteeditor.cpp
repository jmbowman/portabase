/*
 * noteeditor.cpp
 *
 * (c) 2002-2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#if defined(DESKTOP)
#include "desktop/config.h"
#else
#include <qpe/config.h>
#endif

#include <qmultilineedit.h>
#include "noteeditor.h"

NoteEditor::NoteEditor(QString colName, bool readOnly, QWidget *parent,
    const char *name, WFlags f) : PBDialog(colName, parent, name, f)
{
    textBox = new QMultiLineEdit(this);
    vbox->addWidget(textBox);
    textBox->setReadOnly(readOnly);
    Config conf("portabase");
    conf.setGroup("General");
    if (conf.readBoolEntry("NoteWrap", TRUE)) {
        textBox->setWordWrap(QMultiLineEdit::WidgetWidth);
        if (conf.readBoolEntry("WrapAnywhere")) {
            textBox->setWrapPolicy(QMultiLineEdit::Anywhere);
        }
    }
    finishLayout(TRUE, !readOnly, TRUE, 600, 400);
    textBox->setFocus();
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
