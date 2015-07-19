/*
 * condition.cpp
 *
 * (c) 2002-2004,2008-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file condition.cpp
 * Source file for Condition
 */

#include <QLocale>
#include <mk4.h>
#include "condition.h"
#include "database.h"
#include "datatypes.h"
#include "formatting.h"

/**
 * Constructor.
 *
 * @param dbase The database in use
 */
Condition::Condition(Database *dbase) : QObject(), colName("_anytext"), operation(Contains), constant(""), caseSensitive(false), description("")
{
    db = dbase;
}

/**
 * Get the name of the column that this condition applies to.
 *
 * @return The column name
 */
QString Condition::getColName()
{
    return colName;
}

/**
 * Set the name of the column that this condition applies to.
 *
 * @param newName The column name
 */
void Condition::setColName(const QString &newName)
{
    colName = newName;
}

/**
 * Get the ID of the operator used in this condition.
 *
 * @return The operator ID
 */
Condition::Operator Condition::getOperator()
{
    return operation;
}

/**
 * Set the ID of the operator used in this condition.
 *
 * @param op The operator ID
 */
void Condition::setOperator(Operator op)
{
    operation = op;
}

/**
 * Get the string representation of the constant value which is
 * being used in this condition.
 *
 * @return The string form of the constant in use
 */
QString Condition::getConstant()
{
    return constant;
}

/**
 * Set the constant value to use in this condition.
 *
 * @param newConstant The string representation of the constant to use
 */
void Condition::setConstant(const QString &newConstant)
{
    constant = newConstant;
}

/**
 * Determine if this condition uses a case sensitive text comparison or not.
 *
 * @return True for case sensitive text comparisons, false otherwise
 */
bool Condition::isCaseSensitive()
{
    return caseSensitive;
}

/**
 * Specifiy if this condition's comparison is to be performed in a case
 * sensitive manner or not.
 *
 * @param y True for case sensitive text comparisons, false otherwise
 */
void Condition::setCaseSensitive(bool y)
{
    caseSensitive = y;
}

/**
 * Filter the provided data so that only rows which match this condition
 * are retained.
 *
 * @param dbview The database view which is to be filtered
 * @return The subset of the data which matches this condition
 */
c4_View Condition::filter(c4_View dbview)
{
    if (colName == "_anytext") {
        c4_View result = dbview.Clone();
        int count = textCols.count();
        if (count == 0) {
            QStringList colNames = db->listColumns();
            int numCols = colNames.count();
            for (int i = 0; i < numCols; i++) {
                QString name = colNames[i];
                int type = db->getType(name);
                if (type == STRING || type == NOTE) {
                    textCols.append(name);
                }
            }
            count = textCols.count();
        }
        for (int i = 0; i < count; i++) {
            setColName(textCols[i]);
            result = result.Union(filterString(dbview));
        }
        setColName("_anytext");
        return result;
    }
    int type = db->getType(colName);
    if (type == INTEGER || type == BOOLEAN || type == DATE || type == TIME
            || type == SEQUENCE) {
        QString colId = db->getColId(colName);
        int value = QLocale::c().toInt(constant);
        return filterInt(dbview, colId, value);
    }
    else if (type >= FIRST_ENUM) {
        QString colId = db->getColId(colName, INTEGER);
        QStringList options = db->listEnumOptions(type);
        int value = options.indexOf(constant);
        return filterInt(dbview, colId, value);
    }
    else if (type == FLOAT || type == CALC) {
        return filterFloat(dbview);
    }
    else {
        return filterString(dbview);
    }
}

/**
 * Filter implementation for conditions involving integer columns.  Called by
 * filter() when appropriate.
 *
 * @param dbview The database view which is to be filtered
 * @param colId The ID string used for the column of interest in the data table
 * @param value The integer constant being compared against
 * @return The subset of the data which matches this condition
 */
c4_View Condition::filterInt(c4_View dbview, const QString &colId, int value)
{
    c4_IntProp prop(colId.toUtf8().data());
    c4_View result = dbview.Clone();
    int size = dbview.GetSize();
    if (operation == Equals) {
        result = dbview.Select(prop [value]);
    }
    else if (operation == NotEqual) {
        for (int i = 0; i < size; i++) {
            if (prop (dbview[i]) != value) {
                result.Add(dbview[i]);
            }
        }
    }
    else if (operation == LessThan) {
        for (int i = 0; i < size; i++) {
            if (prop (dbview[i]) < value) {
                result.Add(dbview[i]);
            }
        }
    }
    else if (operation == GreaterThan) {
        for (int i = 0; i < size; i++) {
            if (prop (dbview[i]) > value) {
                result.Add(dbview[i]);
            }
        }
    }
    else if (operation == LessEqual) {
        for (int i = 0; i < size; i++) {
            if (prop (dbview[i]) <= value) {
                result.Add(dbview[i]);
            }
        }
    }
    else if (operation == GreaterEqual) {
        for (int i = 0; i < size; i++) {
            if (prop (dbview[i]) >= value) {
                result.Add(dbview[i]);
            }
        }
    }
    return result;
}

/**
 * Filter implementation for conditions involving floating point columns.
 * Called by filter() when appropriate.
 *
 * @param dbview The database view which is to be filtered
 * @return The subset of the data which matches this condition
 */
c4_View Condition::filterFloat(c4_View dbview)
{
    double value = Formatting::parseDouble(constant);
    c4_FloatProp prop(db->getColId(colName).toLatin1().data());
    c4_View result = dbview.Clone();
    int size = dbview.GetSize();
    if (operation == Equals) {
        result = dbview.Select(prop [value]);
    }
    else if (operation == NotEqual) {
        for (int i = 0; i < size; i++) {
            if (prop (dbview[i]) != value) {
                result.Add(dbview[i]);
            }
        }
    }
    else if (operation == LessThan) {
        for (int i = 0; i < size; i++) {
            if (prop (dbview[i]) < value) {
                result.Add(dbview[i]);
            }
        }
    }
    else if (operation == GreaterThan) {
        for (int i = 0; i < size; i++) {
            if (prop (dbview[i]) > value) {
                result.Add(dbview[i]);
            }
        }
    }
    else if (operation == LessEqual) {
        for (int i = 0; i < size; i++) {
            if (prop (dbview[i]) <= value) {
                result.Add(dbview[i]);
            }
        }
    }
    else if (operation == GreaterEqual) {
        for (int i = 0; i < size; i++) {
            if (prop (dbview[i]) >= value) {
                result.Add(dbview[i]);
            }
        }
    }
    return result;
}

/**
 * Filter implementation for conditions involving text columns. Called by
 * filter() when appropriate.
 *
 * @param dbview The database view which is to be filtered
 * @return The subset of the data which matches this condition
 */
c4_View Condition::filterString(c4_View dbview)
{
    c4_StringProp prop(db->getColId(colName).toLatin1().data());
    c4_View result = dbview.Clone();
    int size = dbview.GetSize();
    QString target = constant;
    if (!caseSensitive) {
        target = target.toLower();
    }
    if (operation == Equals) {
        if (caseSensitive) {
            result = dbview.Select(prop [target.toUtf8()]);
        }
        else {
            for (int i = 0; i < size; i++) {
                QString value = QString::fromUtf8(prop (dbview[i]));
                if (value.toLower() == target) {
                    result.Add(dbview[i]);
                }
            }
        }
    }
    else if (operation == Contains) {
        for (int i = 0; i < size; i++) {
            QString value = QString::fromUtf8(prop (dbview[i]));
            Qt::CaseSensitivity cs = caseSensitive ? Qt::CaseSensitive
                                                   : Qt::CaseInsensitive;
            if (value.contains(constant, cs) > 0) {
                result.Add(dbview[i]);
            }
        }
    }
    else if (operation == StartsWith) {
        for (int i = 0; i < size; i++) {
            QString value = QString::fromUtf8(prop (dbview[i]));
            if (!caseSensitive) {
                value = value.toLower();
            }
            if (value.startsWith(target)) {
                result.Add(dbview[i]);
            }
        }
    }
    else if (operation == NotEqual) {
        for (int i = 0; i < size; i++) {
            QString value = QString::fromUtf8(prop (dbview[i]));
            if (!caseSensitive) {
                value = value.toLower();
            }
            if (value != target) {
                result.Add(dbview[i]);
            }
        }
    }
    return result;
}

/**
 * Get the text description of this condition, as shown in the Filter UI.
 *
 * @return A text description of this condition
 */
QString Condition::getDescription()
{
    if (description.isEmpty()) {
        updateDescription();
    }
    return description;
}

/**
 * Update the text description of this condition in order to reflect its
 * current definition.
 */
void Condition::updateDescription()
{
    int type = STRING;
    QString arg1;
    QString arg2;
    if (colName == "_anytext") {
        arg1 = tr("Any text column");
    }
    else {
        arg1 = colName;
        type = db->getType(colName);
    }
    if (type == BOOLEAN) {
        if (constant == "1") {
            description = tr("%1 is checked").arg(arg1);
        }
        else {
            description = tr("%1 is not checked").arg(arg1);
        }
        return;
    }
    if (type == STRING || type == NOTE || type >= FIRST_ENUM) {
        arg2 = "\"" + constant + "\"";
    }
    else if (type == DATE) {
        arg2 = Formatting::dateToString(constant.toInt());
    }
    else if (type == TIME) {
        arg2 = Formatting::timeToString(constant.toInt());
    }
    else if (type == INTEGER || type == SEQUENCE) {
        arg2 = QLocale::system().toString(constant.toInt());
    }
    else if (type == FLOAT) {
        arg2 = Formatting::toLocalDouble(constant);
    }
    else {
        arg2 = constant;
    }
    description = getOperatorText(operation).arg(arg1).arg(arg2);
}

/**
 * Get the description template appropriate for the given operator ID.
 *
 * @param op The ID of the operator to get a description template for
 * @return The appropriate description template
 */
QString Condition::getOperatorText(Operator op)
{
    if (op == Equals) {
        return "%1 = %2";
    }
    else if (op == Contains) {
        return tr("%1 contains %2");
    }
    else if (op == StartsWith) {
        return tr("%1 starts with %2");
    }
    else if (op == LessThan) {
        return "%1 < %2";
    }
    else if (op == GreaterThan) {
        return "%1 > %2";
    }
    else if (op == LessEqual) {
        return "%1 <= %2";
    }
    else if (op == GreaterEqual) {
        return "%1 >= %2";
    }
    else if (op == NotEqual) {
        return "%1 != %2";
    }
    // shouldn't get here
    return "";
}
