/*
 * noteeditor.h
 *
 * (c) 2002-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef NOTEEDITOR_H
#define NOTEEDITOR_H

#include "pbdialog.h"

class QMultiLineEdit;

class NoteEditor: public PBDialog
{
    Q_OBJECT
public:
    NoteEditor(QString colName, bool readOnly, QWidget *parent = 0,
               const char *name = 0);
    ~NoteEditor();

    QString content();
    void setContent(QString text);

private:
    QMultiLineEdit *textBox;
};

#endif
