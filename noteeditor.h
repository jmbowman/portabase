/*
 * noteeditor.h
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef NOTEEDITOR_H
#define NOTEEDITOR_H

#include <qdialog.h>

class QMultiLineEdit;

class NoteEditor: public QDialog
{
    Q_OBJECT
public:
    NoteEditor(QString colName, QWidget *parent = 0, const char *name = 0,
               WFlags f = 0);
    ~NoteEditor();

    QString content();
    void setContent(QString text);
    void setReadOnly(bool y);

protected:
    void resizeEvent(QResizeEvent *event);

private:
    QMultiLineEdit *textBox;
};

#endif
