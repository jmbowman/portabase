/*
 * roweditor.h
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef ROWEDITOR_H
#define ROWEDITOR_H

#include <qdialog.h>
#include <qvaluelist.h>
#include "datatypes.h"

class Database;
class DateWidget;
class QCheckBox;
class QLineEdit;
class QScrollView;
class NoteButton;

typedef QValueList<QCheckBox*> CheckBoxList;
typedef QValueList<QLineEdit*> LineEditList;
typedef QValueList<NoteButton*> NoteButtonList;
typedef QValueList<DateWidget*> DateWidgetList;

class RowEditor: public QDialog
{
    Q_OBJECT
public:
    RowEditor(QWidget *parent = 0, const char *name = 0, WFlags f = 0);
    ~RowEditor();

    bool edit(Database *subject, int rowId = -1);

protected:
    void resizeEvent(QResizeEvent *event);

private:
    void addContent(int rowId);

private:
    Database *db;
    QStringList colNames;
    int *colTypes;
    QScrollView *sv;
    LineEditList lineEdits;
    CheckBoxList checkBoxes;
    NoteButtonList noteButtons;
    DateWidgetList dateWidgets;
};

#endif
