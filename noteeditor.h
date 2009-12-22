/*
 * noteeditor.h
 *
 * (c) 2002-2004,2009 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file noteeditor.h
 * Header file for NoteEditor
 */

#ifndef NOTEEDITOR_H
#define NOTEEDITOR_H

#include "pbdialog.h"

class QTextEdit;

/**
 * Dialog for displaying and/or editing the content of a note field.
 */
class NoteEditor: public PBDialog
{
    Q_OBJECT
public:
    NoteEditor(const QString &colName, bool readOnly, QWidget *parent = 0);

    QString content();
    void setContent(const QString &text);

private:
    QTextEdit *textBox; /**< The text box containing the note content */
};

#endif
