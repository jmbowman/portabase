/*
 * noteeditor.cpp
 *
 * (c) 2002-2004,2009 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file noteeditor.cpp
 * Source file for NoteEditor
 */

#include <QLayout>
#include <QSettings>
#include <QTextEdit>
#include "noteeditor.h"

/**
 * Constructor.
 *
 * @param colName Name of the column this note belongs to
 * @param readOnly True if the note is only being displayed, not edited
 * @param parent This dialog's parent widget
 */
NoteEditor::NoteEditor(const QString &colName, bool readOnly, QWidget *parent)
  : PBDialog(colName, parent)
{
    textBox = new QTextEdit(this);
    vbox->addWidget(textBox);
    textBox->setAcceptRichText(false);
    textBox->setReadOnly(readOnly);
    QSettings settings;
    if (!settings.value("General/NoteWrap", true).toBool()) {
      textBox->setLineWrapMode(QTextEdit::NoWrap);
    }
    else {
        textBox->setLineWrapMode(QTextEdit::WidgetWidth);
        if (settings.value("General/WrapAnywhere", false).toBool()) {
            textBox->setWordWrapMode(QTextOption::WrapAnywhere);
        }
    }
    finishLayout(true, !readOnly, 600, 400);
    textBox->setFocus();
}

/**
 * Get the current note content from the edit box.
 *
 * @return The current note text
 */
QString NoteEditor::content()
{
    return textBox->toPlainText();
}

/**
 * Set the note content to display in the edit box.
 *
 * @param text The new note text
 */
void NoteEditor::setContent(const QString &text)
{
    textBox->setPlainText(text);
}
