/*
 * condition.cpp
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <mk4.h>
#include "condition.h"
#include "database.h"
#include "datatypes.h"
#include "portabase.h"

Condition::Condition(Database *dbase) : colName("_anytext"), operation(CONTAINS), constant(""), caseSensitive(FALSE), description("")
{
    db = dbase;
}

Condition::~Condition()
{

}

QString Condition::getColName()
{
    return colName;
}

void Condition::setColName(QString newName)
{
    colName = newName;
}

int Condition::getOperator()
{
    return operation;
}

void Condition::setOperator(int op)
{
    operation = op;
}

QString Condition::getConstant()
{
    return constant;
}

void Condition::setConstant(QString newConstant)
{
    constant = newConstant;
}

bool Condition::isCaseSensitive()
{
    return caseSensitive;
}

void Condition::setCaseSensitive(bool y)
{
    caseSensitive = y;
}

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
    if (type == INTEGER || type == BOOLEAN || type == DATE) {
        return filterInt(dbview);
    }
    else if (type == FLOAT) {
        return filterFloat(dbview);
    }
    else {
        return filterString(dbview);
    }
}

c4_View Condition::filterInt(c4_View dbview)
{
    int value = constant.toInt();
    c4_IntProp prop(colName);
    c4_View result = dbview.Clone();
    int size = dbview.GetSize();
    if (operation == EQUALS) {
        result = dbview.Select(prop [value]);
    }
    else if (operation == NOTEQUAL) {
        for (int i = 0; i < size; i++) {
            if (prop (dbview[i]) != value) {
                result.Add(dbview[i]);
            }
        }
    }
    else if (operation == LESSTHAN) {
        for (int i = 0; i < size; i++) {
            if (prop (dbview[i]) < value) {
                result.Add(dbview[i]);
            }
        }
    }
    else if (operation == GREATERTHAN) {
        for (int i = 0; i < size; i++) {
            if (prop (dbview[i]) > value) {
                result.Add(dbview[i]);
            }
        }
    }
    else if (operation == LESSEQUAL) {
        for (int i = 0; i < size; i++) {
            if (prop (dbview[i]) <= value) {
                result.Add(dbview[i]);
            }
        }
    }
    else if (operation == GREATEREQUAL) {
        for (int i = 0; i < size; i++) {
            if (prop (dbview[i]) >= value) {
                result.Add(dbview[i]);
            }
        }
    }
    return result;
}

c4_View Condition::filterFloat(c4_View dbview)
{
    double value = constant.toDouble();
    c4_FloatProp prop(colName);
    c4_View result = dbview.Clone();
    int size = dbview.GetSize();
    if (operation == EQUALS) {
        result = dbview.Select(prop [value]);
    }
    else if (operation == NOTEQUAL) {
        for (int i = 0; i < size; i++) {
            if (prop (dbview[i]) != value) {
                result.Add(dbview[i]);
            }
        }
    }
    else if (operation == LESSTHAN) {
        for (int i = 0; i < size; i++) {
            if (prop (dbview[i]) < value) {
                result.Add(dbview[i]);
            }
        }
    }
    else if (operation == GREATERTHAN) {
        for (int i = 0; i < size; i++) {
            if (prop (dbview[i]) > value) {
                result.Add(dbview[i]);
            }
        }
    }
    else if (operation == LESSEQUAL) {
        for (int i = 0; i < size; i++) {
            if (prop (dbview[i]) <= value) {
                result.Add(dbview[i]);
            }
        }
    }
    else if (operation == GREATEREQUAL) {
        for (int i = 0; i < size; i++) {
            if (prop (dbview[i]) >= value) {
                result.Add(dbview[i]);
            }
        }
    }
    return result;
}

c4_View Condition::filterString(c4_View dbview)
{
    c4_StringProp prop(colName);
    c4_View result = dbview.Clone();
    int size = dbview.GetSize();
    QString target = constant;
    if (!caseSensitive) {
        target = target.lower();
    }
    if (operation == EQUALS) {
        if (caseSensitive) {
            result = dbview.Select(prop [target]);
        }
        else {
            for (int i = 0; i < size; i++) {
                QString value(prop (dbview[i]));
                if (value.lower() == target) {
                    result.Add(dbview[i]);
                }
            }
        }
    }
    else if (operation == CONTAINS) {
        for (int i = 0; i < size; i++) {
            QString value(prop (dbview[i]));
            if (value.contains(constant, caseSensitive) > 0) {
                result.Add(dbview[i]);
            }
        }
    }
    else if (operation == STARTSWITH) {
        for (int i = 0; i < size; i++) {
            QString value(prop (dbview[i]));
            if (!caseSensitive) {
                value = value.lower();
            }
            if (value.startsWith(target)) {
                result.Add(dbview[i]);
            }
        }
    }
    else if (operation == NOTEQUAL) {
        for (int i = 0; i < size; i++) {
            QString value(prop (dbview[i]));
            if (!caseSensitive) {
                value = value.lower();
            }
            if (value != target) {
                result.Add(dbview[i]);
            }
        }
    }
    return result;
}

QString Condition::getDescription()
{
    if (description == "") {
        updateDescription();
    }
    return description;
}

void Condition::updateDescription()
{
    int type = STRING;
    if (colName == "_anytext") {
        description = PortaBase::tr("Any text column");
    }
    else {
        description = colName;
        type = db->getType(colName);
    }
    description += " ";
    if (type == BOOLEAN) {
        if (constant == "1") {
            description += "is checked";
        }
        else {
            description += "is not checked";
        }
        return;
    }
    description += getOperatorText(operation) + " ";
    if (type == STRING || type == NOTE) {
        description += "\"" + constant + "\"";
    }
    else if (type == DATE) {
        description += db->dateToString(constant.toInt());
    }
    else {
        description += constant;
    }
}

QString Condition::getOperatorText(int op)
{
    if (op == EQUALS) {
        return "=";
    }
    else if (op == CONTAINS) {
        return PortaBase::tr("contains");
    }
    else if (op == STARTSWITH) {
        return PortaBase::tr("starts with");
    }
    else if (op == LESSTHAN) {
        return "<";
    }
    else if (op == GREATERTHAN) {
        return ">";
    }
    else if (op == LESSEQUAL) {
        return "<=";
    }
    else if (op == GREATEREQUAL) {
        return ">=";
    }
    else if (op == NOTEQUAL) {
        return "!=";
    }
    // shouldn't get here
    return "";
}
