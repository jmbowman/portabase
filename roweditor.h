/*
 * roweditor.h
 *
 * (c) 2002-2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef ROWEDITOR_H
#define ROWEDITOR_H

#include <qdialog.h>
#include <qstringlist.h>
#include <qvaluelist.h>
#include "datatypes.h"

class Database;
class DateWidget;
class QCheckBox;
class QComboBox;
class QLineEdit;
class NoteButton;
class NumberWidget;
class TimeWidget;

typedef QValueList<QCheckBox*> CheckBoxList;
typedef QValueList<QLineEdit*> LineEditList;
typedef QValueList<NoteButton*> NoteButtonList;
typedef QValueList<DateWidget*> DateWidgetList;
typedef QValueList<TimeWidget*> TimeWidgetList;
typedef QValueList<NumberWidget*> NumberWidgetList;
typedef QValueList<QComboBox*> ComboBoxList;

class RowEditor: public QDialog
{
    Q_OBJECT
public:
    RowEditor(QWidget *parent = 0, const char *name = 0, WFlags f = 0);
    ~RowEditor();

    bool edit(Database *subject, int rowId=-1, bool copy=FALSE);

private:
    void addContent(int rowId);

private:
    Database *db;
    QStringList colNames;
    int *colTypes;
    LineEditList lineEdits;
    CheckBoxList checkBoxes;
    NoteButtonList noteButtons;
    DateWidgetList dateWidgets;
    TimeWidgetList timeWidgets;
    NumberWidgetList numberWidgets;
    ComboBoxList comboBoxes;
};

#endif
