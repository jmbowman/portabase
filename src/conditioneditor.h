/*
 * conditioneditor.h
 *
 * (c) 2002-2004,2008-2009,2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file conditioneditor.h
 * Header file for ConditionEditor
 */

#ifndef CONDITIONEDITOR_H
#define CONDITIONEDITOR_H

#include <QStringList>
#include "condition.h"
#include "datatypes.h"
#include "pbdialog.h"
#include "qqutil/qqlineedit.h"

class Database;
class DateWidget;
class NumberWidget;
class QCheckBox;
class QComboBox;
class QStackedWidget;
class TimeWidget;

typedef QList<Condition::Operator> OperatorList;

/**
 * A dialog for editing an individual filter condition.  Used by FilterEditor.
 */
class ConditionEditor: public PBDialog
{
    Q_OBJECT
public:
    ConditionEditor(Database *dbase, QWidget *parent = 0, bool quickFilter = false);

    int edit(Condition *condition);
    void applyChanges(Condition *condition);

private slots:
    void updateDisplay(int columnIndex);

private:
    void fillFields(Condition *condition);
    bool isValidConstant();
    void updateOpList();

private:
    Database *db; /**< The database in use */
    QStringList colNames; /**< The list of non-image column names */
    IntList types; /**< The list of non-image column types */
    OperatorList stringOps; /**< The list of string operator IDs */
    OperatorList numberOps; /**< The list of numeric operator IDs */
    QComboBox *columnList; /**< The selection widget for the column name */
    QComboBox *opList; /**< The selection widget for the operator */
    QStringList stringOpList; /**< The list of string operator display strings */
    QStringList numberOpList; /**< The list of numeric operator display strings */
    QStackedWidget *constantStack; /**< The stack of constant value widgets */
    QQLineEdit *constantLine; /**< Entry field for text constants */
    QCheckBox *constantCheck; /**< Checkbox for boolean constants */
    DateWidget *constantDate; /**< Date constant selection widget */
    TimeWidget *constantTime; /**< Time constant selection widget */
    NumberWidget *constantInteger; /**< Entry widget for integer constants */
    NumberWidget *constantFloat; /**< Entry widget for floating point constants */
    QComboBox *constantCombo; /**< Selection list for enum constants */
    QCheckBox *caseCheck; /**< Case sensitivity indicator checkbox */
    int dataType; /**< The data type of the last selected column */
};

#endif
