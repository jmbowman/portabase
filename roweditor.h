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

class CalcWidget;
class Database;
class DateWidget;
class DynamicEdit;
class QCheckBox;
class QComboBox;
class NoteButton;
class NumberWidget;
class TimeWidget;

typedef QValueList<QCheckBox*> CheckBoxList;
typedef QValueList<NoteButton*> NoteButtonList;
typedef QValueList<DateWidget*> DateWidgetList;
typedef QValueList<TimeWidget*> TimeWidgetList;
typedef QValueList<NumberWidget*> NumberWidgetList;
typedef QValueList<QComboBox*> ComboBoxList;
typedef QValueList<DynamicEdit*> DynamicEditList;
typedef QValueList<CalcWidget*> CalcWidgetList;

class RowEditor: public QDialog
{
    Q_OBJECT
public:
    RowEditor(QWidget *parent = 0, const char *name = 0, WFlags f = 0);
    ~RowEditor();

    bool edit(Database *subject, int rowId=-1, bool copy=FALSE);
    bool isValid();
    QStringList getRow(bool doCalcs=TRUE);

private:
    void addContent(int rowId);

private:
    Database *db;
    QStringList colNames;
    int *colTypes;
    CheckBoxList checkBoxes;
    NoteButtonList noteButtons;
    DateWidgetList dateWidgets;
    TimeWidgetList timeWidgets;
    NumberWidgetList numberWidgets;
    ComboBoxList comboBoxes;
    DynamicEditList dynamicEdits;
    CalcWidgetList calcWidgets;
};

#endif
