/*
 * conditioneditor.h
 *
 * (c) 2002-2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef CONDITIONEDITOR_H
#define CONDITIONEDITOR_H

#include <qstringlist.h>
#include "datatypes.h"
#include "pbdialog.h"

class Condition;
class Database;
class DateWidget;
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QWidgetStack;
class TimeWidget;
class NumberWidget;

class ConditionEditor: public PBDialog
{
    Q_OBJECT
public:
    ConditionEditor(Database *dbase, QWidget *parent = 0, const char *name = 0,
                    WFlags f = 0);
    ~ConditionEditor();

    int edit(Condition *condition);
    void applyChanges(Condition *condition);

private slots:
    void updateDisplay(int columnIndex);

private:
    void fillFields(Condition *condition);
    bool isValidConstant();
    void updateOpList();

private:
    Database *db;
    QStringList colNames;
    int *types;
    IntList stringOps;
    IntList numberOps;
    QComboBox *columnList;
    QComboBox *opList;
    QStringList stringOpList;
    QStringList numberOpList;
    QWidgetStack *constantStack;
    QLineEdit *constantLine;
    QCheckBox *constantCheck;
    DateWidget *constantDate;
    TimeWidget *constantTime;
    NumberWidget *constantInteger;
    NumberWidget *constantFloat;
    QComboBox *constantCombo;
    QCheckBox *caseCheck;
    int dataType;
};

#endif
