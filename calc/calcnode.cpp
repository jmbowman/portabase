/*
 * calcnode.cpp
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <math.h>
#include <qdatetime.h>
#include <qobject.h>
#include "calcnode.h"
#include "../database.h"

CalcNode::CalcNode(int type, const QString &value) : nodeType(type)
{
    nodeValue = value;
}

CalcNode::~CalcNode()
{
    int count = children.count();
    for (int i = 0; i < count; i++) {
        delete children[i];
    }
}

int CalcNode::type()
{
    return nodeType;
}

QString CalcNode::value()
{
    return nodeValue;
}

void CalcNode::setType(int newType)
{
    nodeType = newType;
}

void CalcNode::setValue(const QString &newValue)
{
    nodeValue = newValue;
}

bool CalcNode::allowsAdd()
{
    int max = maxChildren();
    int count = children.count();
    return (max == -1 || count < max);
}

bool CalcNode::allowsEdit()
{
    switch (nodeType) {
        case CALC_CONSTANT:
        case CALC_COLUMN:
        case CALC_DATE_CONSTANT:
        case CALC_DATE_COLUMN:
            return TRUE;
        default:
            return FALSE;
    }
}

double CalcNode::value(const QStringList &row, const QStringList &colNames)
{
    if (nodeType == CALC_CONSTANT) {
        return nodeValue.toDouble();
    }
    if (nodeType == CALC_COLUMN) {
        int index = colNames.findIndex(nodeValue);
        return row[index].toDouble();
    }
    int count = children.count();
    if (count == 0) {
        return 0;
    }
    int i;
    double result = 0;
    if (nodeType == CALC_ADD || nodeType == CALC_AVERAGE) {
        for (i = 0; i < count; i++) {
            result += children[i]->value(row, colNames);
        }
        if (nodeType == CALC_AVERAGE) {
            result /= count;
        }
    }
    else if (nodeType == CALC_SUBTRACT) {
        result = children[0]->value(row, colNames);
        if (count > 1) {
            result -= children[1]->value(row, colNames);
        }
    }
    else if (nodeType == CALC_MULTIPLY) {
        result = 1;
        for (i = 0; i < count; i++) {
            result *= children[i]->value(row, colNames);
        }
    }
    else if (nodeType == CALC_DIVIDE) {
        result = children[0]->value(row, colNames);
        if (count > 1) {
            double denom = children[1]->value(row, colNames);
            if (denom == 0) {
                // avoid division by zero
                result = 0;
            }
            else {
                result /= denom;
            }
        }
    }
    else if (nodeType == CALC_DAYS) {
        if (count == 2) {
            int day1 = children[0]->days(row, colNames);
            int day2 = children[1]->days(row, colNames);
            if (day1 > 0 && day2 > 0) {
                return day2 - day1;
            }
        }
    }
    else if (nodeType == CALC_MAX) {
        for (i = 0; i < count; i++) {
            result = QMAX(result, children[i]->value(row, colNames));
        }
    }
    else if (nodeType == CALC_MIN) {
        for (i = 0; i < count; i++) {
            result = QMIN(result, children[i]->value(row, colNames));
        }
    }
    else if (nodeType == CALC_ABS) {
        result = QABS(children[0]->value(row, colNames));
    }
    else if (nodeType == CALC_SQRT) {
        result = sqrt(children[0]->value(row, colNames));
    }
    else if (nodeType == CALC_LOG) {
        result = log10(children[0]->value(row, colNames));
    }
    else if (nodeType == CALC_LN) {
        result = log(children[0]->value(row, colNames));
    }
    return result;
}

int CalcNode::days(const QStringList &row, const QStringList &colNames)
{
    int value = 0;
    if (nodeType == CALC_DATE_CONSTANT) {
        value = nodeValue.toInt();
    }
    if (nodeType == CALC_DATE_COLUMN) {
        int index = colNames.findIndex(nodeValue);
        value = row[index].toInt();
    }
    QDate firstDate(1752, 9, 14);
    int y = value / 10000;
    int m = (value - y * 10000) / 100;
    int d = value - y * 10000 - m * 100;
    QDate date(y, m, d);
    return firstDate.daysTo(date);
}

QString CalcNode::description(Database *db)
{
    return CalcNode::description(db, nodeType, nodeValue);
}

QString CalcNode::description(Database *db, int type, const QString &value)
{
    switch (type) {
        case CALC_CONSTANT:
        case CALC_COLUMN:
        case CALC_DATE_COLUMN:
            return value;
        case CALC_DATE_CONSTANT:
            return db->dateToString(value.toInt());
        case CALC_ADD:
            return "+";
        case CALC_SUBTRACT:
            return "-";
        case CALC_MULTIPLY:
            return "*";
        case CALC_DIVIDE:
            return "/";
        case CALC_DAYS:
            return QObject::tr("Days_Between");
        case CALC_MAX:
            return QObject::tr("MAX");
        case CALC_MIN:
            return QObject::tr("MIN");
        case CALC_AVERAGE:
            return QObject::tr("AVERAGE");
        case CALC_ABS:
            return QObject::tr("ABS");
        case CALC_SQRT:
            return QObject::tr("SQRT");
        case CALC_LOG:
            return QObject::tr("LOG");
        case CALC_LN:
            return QObject::tr("LN");
        default:
            return "";
    }
}

QString CalcNode::equation(Database *db, bool useParens)
{
    QString result;
    QString opString;
    int count = children.count();
    int i;
    switch (nodeType) {
        case CALC_CONSTANT:
        case CALC_COLUMN:
        case CALC_DATE_COLUMN:
        case CALC_DATE_CONSTANT:
            return description(db);
        case CALC_DAYS:
        case CALC_MAX:
        case CALC_MIN:
        case CALC_AVERAGE:
        case CALC_ABS:
        case CALC_SQRT:
        case CALC_LOG:
        case CALC_LN:
            result = description(db) + "(";
            if (count == 0) {
                result += "?";
            }
            else {
                result += children[0]->equation(db, FALSE);
            }
            for (i = 1; i < count; i++) {
                result += "," + children[i]->equation(db, FALSE);
            }
            result += ")";
            return result;
        case CALC_ADD:
        case CALC_SUBTRACT:
        case CALC_MULTIPLY:
        case CALC_DIVIDE:
            if (count == 0) {
                result = "?";
            }
            else {
                result = children[0]->equation(db, TRUE);
            }
            opString = " " + description(db) + " ";
            result += opString;
            if (count < 2) {
                result += "?";
            }
            else {
                result += children[1]->equation(db, TRUE);
            }
            for (i = 2; i < count; i++) {
                result += opString + children[i]->equation(db, TRUE);
            }
            if (useParens) {
                result = "(" + result + ")";
            }
            return result;
        default:
            return "";
    }
}

QStringList CalcNode::listOperations()
{
    QStringList ops;
    QString blank = "";
    for (int i = CALC_FIRST_OP; i <= CALC_LAST_OP; i++) {
        ops.append(CalcNode::description(0, i, blank));
    }
    return ops;
}

void CalcNode::addChild(CalcNode *child)
{
    children.append(child);
}

void CalcNode::removeChild(CalcNode *child)
{
    delete child;
    children.remove(child);
}

void CalcNode::moveChild(CalcNode *child, CalcNode *after)
{
    children.remove(after);
    CalcNodeList::Iterator iter = children.find(child);
    children.insert(iter, after);
}

CalcNodeList CalcNode::getChildren()
{
    return children;
}

CalcNode *CalcNode::clone()
{
    CalcNode *node = new CalcNode(nodeType, nodeValue);
    int count = children.count();
    for (int i = 0; i < count; i++) {
        node->addChild(children[i]->clone());
    }
    return node;
}

int CalcNode::maxChildren()
{
    switch (nodeType) {
        case CALC_CONSTANT:
        case CALC_COLUMN:
        case CALC_DATE_CONSTANT:
        case CALC_DATE_COLUMN:
            return 0;
        case CALC_SUBTRACT:
        case CALC_DIVIDE:
        case CALC_DAYS:
            return 2;
        case CALC_ABS:
        case CALC_SQRT:
        case CALC_LOG:
        case CALC_LN:
            return 1;
        default:
            return -1;
    }
}
