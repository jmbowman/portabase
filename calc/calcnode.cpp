/*
 * calcnode.cpp
 *
 * (c) 2003-2004,2008-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file calcnode.cpp
 * Source file for CalcNode
 */

#include <math.h>
#include <QDateTime>
#include "calcnode.h"
#include "../database.h"
#include "../formatting.h"

/**
 * Constructor.
 *
 * @param type The type of node to create
 * @param value The node's initial value
 */
CalcNode::CalcNode(NodeType type, const QString &value) : QObject(), nodeType(type)
{
    nodeValue = value;
}

/**
 * Destructor.  Ensures that any child nodes are also deleted.
 */
CalcNode::~CalcNode()
{
    int count = children.count();
    for (int i = 0; i < count; i++) {
        delete children[i];
    }
}

/**
 * Get this node's type.
 *
 * @return The type of node this is
 */
CalcNode::NodeType CalcNode::type()
{
    return nodeType;
}

/**
 * Get this node's value.
 *
 * @return This node's value
 */
QString CalcNode::value()
{
    return nodeValue;
}

/**
 * Set this node's type.
 *
 * @param newType The type of node this will become
 */
void CalcNode::setType(NodeType newType)
{
    nodeType = newType;
}

/**
 * Set this node's value.
 *
 * @param newValue This node's new value
 */
void CalcNode::setValue(const QString &newValue)
{
    nodeValue = newValue;
}

/**
 * Determine if this node allows any (more) child nodes to be added.
 * Determined dynamically based on the node type and the number of existing
 * child nodes.
 *
 * @return True if more child nodes may be added, false otherwise
 */
bool CalcNode::allowsAdd()
{
    int max = maxChildren();
    int count = children.count();
    return (max == -1 || count < max);
}

/**
 * Determine if this node can be edited.  Many node types, especially
 * operations, cannot be edited because it would be difficult to ensure that
 * the child node configuration is still valid after the change.  Such nodes
 * must be deleted and then replaced in order to be changed.
 *
 * @return True if this node may be edited, false otherwise
 */
bool CalcNode::allowsEdit()
{
    switch (nodeType) {
        case Constant:
        case Column:
        case DateConstant:
        case DateColumn:
            return true;
        default:
            return false;
    }
}

/**
 * Get the value of this node of the calculation for a particular data row.
 * The corresponding column names are also provided for ease of looking up
 * the value of particular columns.
 *
 * @param row The row of data for which the calculation is being performed
 * @param colNames The list of column names
 * @return The calculated value
 */
double CalcNode::value(const QStringList &row, const QStringList &colNames)
{
    if (nodeType == Constant || nodeType == TimeConstant) {
        return Formatting::parseDouble(nodeValue);
    }
    if (nodeType == Column || nodeType == TimeColumn) {
        int index = colNames.indexOf(nodeValue);
        return Formatting::parseDouble(row[index]);
    }
    int count = children.count();
    if (count == 0) {
        return 0;
    }
    int i;
    double result = 0;
    if (nodeType == Add || nodeType == Average) {
        for (i = 0; i < count; i++) {
            result += children[i]->value(row, colNames);
        }
        if (nodeType == Average) {
            result /= count;
        }
    }
    else if (nodeType == Subtract) {
        result = children[0]->value(row, colNames);
        if (count > 1) {
            result -= children[1]->value(row, colNames);
        }
    }
    else if (nodeType == Multiply) {
        result = 1;
        for (i = 0; i < count; i++) {
            result *= children[i]->value(row, colNames);
        }
    }
    else if (nodeType == Divide) {
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
    else if (nodeType == Days) {
        if (count == 2) {
            int day1 = children[0]->days(row, colNames);
            int day2 = children[1]->days(row, colNames);
            if (day1 > 0 && day2 > 0) {
                return day2 - day1;
            }
        }
    }
    else if (nodeType == Max) {
        result = children[0]->value(row, colNames);
        for (i = 1; i < count; i++) {
            result = qMax(result, children[i]->value(row, colNames));
        }
    }
    else if (nodeType == Min) {
        result = children[0]->value(row, colNames);
        for (i = 1; i < count; i++) {
            result = qMin(result, children[i]->value(row, colNames));
        }
    }
    else if (nodeType == Abs) {
        result = qAbs(children[0]->value(row, colNames));
    }
    else if (nodeType == Sqrt) {
        result = sqrt(children[0]->value(row, colNames));
    }
    else if (nodeType == Log) {
        result = log10(children[0]->value(row, colNames));
    }
    else if (nodeType == Ln) {
        result = log(children[0]->value(row, colNames));
    }
    else if (nodeType == Seconds) {
        if (count == 2) {
            result = children[1]->value(row, colNames);
            result -= children[0]->value(row, colNames);
        }
    }
    else if (nodeType == Minutes) {
        if (count == 2) {
            result = children[1]->value(row, colNames);
            result -= children[0]->value(row, colNames);
        }
        result /= 60;
    }
    else if (nodeType == Hours) {
        if (count == 2) {
            result = children[1]->value(row, colNames);
            result -= children[0]->value(row, colNames);
        }
        result /= 3600;
    }
    return result;
}

/**
 * Get the number of days from the first day of the Gregorian calendar
 * until the date represented by this node.  Used in "Days" operations to
 * get the number of days between two dates.
 *
 * @param row The row of data on which the calculation is being performed
 * @param colNames The list of column names
 * @return The appropriate number of days
 */
int CalcNode::days(const QStringList &row, const QStringList &colNames)
{
    int value = 0;
    if (nodeType == DateConstant) {
        value = nodeValue.toInt();
    }
    if (nodeType == DateColumn) {
        int index = colNames.indexOf(nodeValue);
        value = row[index].toInt();
    }
    QDate firstDate(1752, 9, 14);
    int y = value / 10000;
    int m = (value - y * 10000) / 100;
    int d = value - y * 10000 - m * 100;
    QDate date(y, m, d);
    return firstDate.daysTo(date);
}

/**
 * The text description of this node, as it will appear in the definition
 * tree display.
 *
 * @param db The database that this calculation belongs to
 * @return This node's description
 */
QString CalcNode::description(Database *db)
{
    return CalcNode::description(db, nodeType, nodeValue);
}

/**
 * Get the text description of a node with the specified properties.
 *
 * @param db The database that the calculation belongs to
 * @param type The type of calculation node
 * @param value The calculation's value
 * @return An appopriate text description
 */
QString CalcNode::description(Database *db, NodeType type, const QString &value)
{
    switch (type) {
        case Constant:
        case Column:
        case DateColumn:
        case TimeColumn:
            return value;
        case DateConstant:
            return db->dateToString(value.toInt());
        case TimeConstant:
            return db->timeToString(value.toInt());
        case Add:
            return "+";
        case Subtract:
            return "-";
        case Multiply:
            return "*";
        case Divide:
            return "/";
        case Days:
            return tr("Days_Between");
        case Max:
            return tr("MAX");
        case Min:
            return tr("MIN");
        case Average:
            return tr("AVERAGE");
        case Abs:
            return tr("ABS");
        case Sqrt:
            return tr("SQRT");
        case Log:
            return tr("LOG");
        case Ln:
            return tr("LN");
        case Seconds:
            return tr("Seconds_Between");
        case Minutes:
            return tr("Minutes_Between");
        case Hours:
            return tr("Hours_Between");
        default:
            return "";
    }
}

/**
 * Get a text representation of this node (and it's descendents) suitable for
 * display as a human-readable equation.
 *
 * @param db The database that this calculation belongs to
 * @param useParens True if the result will be used in a context where
 *                  parentheses may be needed for clarity
 * @return A text equation representing this node and its descendents
 */
QString CalcNode::equation(Database *db, bool useParens)
{
    QString result;
    QString opString;
    int count = children.count();
    int i;
    bool needsSecondParam = false;
    switch (nodeType) {
        case Constant:
        case Column:
        case DateColumn:
        case DateConstant:
        case TimeColumn:
        case TimeConstant:
            return description(db);
        case Days:
        case Seconds:
        case Minutes:
        case Hours:
            needsSecondParam = true;
            // fall through to next case
        case Max:
        case Min:
        case Average:
        case Abs:
        case Sqrt:
        case Log:
        case Ln:
            result = description(db) + "(";
            if (count == 0) {
                result += "?";
            }
            else {
                result += children[0]->equation(db, false);
            }
            for (i = 1; i < count; i++) {
                result += "," + children[i]->equation(db, false);
            }
            if (count == 1 && needsSecondParam) {
                result += ",?";
            }
            result += ")";
            return result;
        case Add:
        case Subtract:
        case Multiply:
        case Divide:
            if (count == 0) {
                result = "?";
            }
            else {
                result = children[0]->equation(db, true);
            }
            opString = " " + description(db) + " ";
            result += opString;
            if (count < 2) {
                result += "?";
            }
            else {
                result += children[1]->equation(db, true);
            }
            for (i = 2; i < count; i++) {
                result += opString + children[i]->equation(db, true);
            }
            if (useParens) {
                result = "(" + result + ")";
            }
            return result;
        default:
            return "";
    }
}

/**
 * Get a list of descriptions of each operation node type.
 *
 * @return A list of operation descriptions
 */
QStringList CalcNode::listOperations()
{
    QStringList ops;
    QString blank = "";
    for (int i = CALC_FIRST_OP; i <= CALC_LAST_OP; i++) {
        ops.append(CalcNode::description(0, (NodeType)i, blank));
    }
    return ops;
}

/**
 * Add the given node as a child of this one.
 *
 * @param child The new child node
 */
void CalcNode::addChild(CalcNode *child)
{
    children.append(child);
}

/**
 * Remove the specified child node and delete it.
 */
void CalcNode::removeChild(CalcNode *child)
{
    delete child;
    children.removeOne(child);
}

/**
 * Swap the positions of two of this node's child nodes.
 *
 * @param i The position index of one of the children to be swapped
 * @param j The position index of the other child node to be swapped
 */
void CalcNode::swapChildren(int i, int j)
{
    children.swap(i, j);
}

/**
 * Get a list of this node's child nodes.
 *
 * @return The list of this node's children
 */
CalcNodeList CalcNode::getChildren()
{
    return children;
}

/**
 * Create a copy of this node.
 *
 * @return The newly created node
 */
CalcNode *CalcNode::clone()
{
    CalcNode *node = new CalcNode(nodeType, nodeValue);
    int count = children.count();
    for (int i = 0; i < count; i++) {
        node->addChild(children[i]->clone());
    }
    return node;
}

/**
 * Determine the maximum number of child nodes that this node is allowed to
 * have.
 *
 * @return The maximum allowed number of child nodes
 */
int CalcNode::maxChildren()
{
    switch (nodeType) {
        case Constant:
        case Column:
        case DateConstant:
        case DateColumn:
            return 0;
        case Subtract:
        case Divide:
        case Days:
        case Seconds:
        case Minutes:
        case Hours:
            return 2;
        case Abs:
        case Sqrt:
        case Log:
        case Ln:
            return 1;
        default:
            return -1;
    }
}

/**
 * Remove any of this node's descendents which refer to the specified
 * column (usually because it has been deleted from the database).
 *
 * @param name The name of the column being deleted
 * @return True if this node needs to be deleted also, false otherwise
 */
bool CalcNode::deleteColumn(const QString &name)
{
    int count = children.count();
    int i;
    CalcNodeList removals;
    for (i = 0; i < count; i++) {
        CalcNode *child = children[i];
        if (child->deleteColumn(name)) {
            removals.append(child);
        }
    }
    count = removals.count();
    for (i = 0; i < count; i++) {
        removeChild(removals[i]);
    }
    if (nodeType == Column || nodeType == DateColumn
            || nodeType == TimeColumn) {
        if (nodeValue == name) {
            return true;
        }
    }
    return false;
}

/**
 * Update any of this node and its descendents which refer to the specified
 * column to use its new name instead.
 *
 * @param oldName The column's last name
 * @param newName The column's new name
 */
void CalcNode::renameColumn(const QString &oldName, const QString &newName)
{
    int count = children.count();
    for (int i = 0; i < count; i++) {
        children[i]->renameColumn(oldName, newName);
    }
    if (nodeType == Column || nodeType == DateColumn
            || nodeType == TimeColumn) {
        if (nodeValue == oldName) {
            nodeValue = newName;
        }
    }
}
