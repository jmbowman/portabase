/*
 * calcnode.h
 *
 * (c) 2003-2004,2008 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file calcnode.h
 * Header file for CalcNode
 */

#ifndef CALCNODE_H
#define CALCNODE_H

#define CALC_FIRST_OP 20
#define CALC_LAST_OP 34

#include <QObject>
#include <QStringList>

class CalcNode;
class Database;

typedef QList<CalcNode*> CalcNodeList;

/**
 * Class representing one node in the definition of a calculation.  Can be
 * a column reference, a constant value, or an operation; if it's an operation,
 * it usually has child nodes (but will fail gracefully to a default value if
 * none have been defined).
 */
class CalcNode : public QObject
{
    Q_OBJECT
public:
    /** Enumeration of possible node types */
    enum NodeType {
        Constant = 0,
        Column = 1,
        DateConstant = 2,
        DateColumn = 3,
        TimeConstant = 4,
        TimeColumn = 5,
        Add = 20,
        Subtract = 21,
        Multiply = 22,
        Divide = 23,
        Days = 24,
        Max = 25,
        Min = 26,
        Average = 27,
        Abs = 28,
        Sqrt = 29,
        Log = 30,
        Ln = 31,
        Seconds = 32,
        Minutes = 33,
        Hours = 34
    };
    CalcNode(NodeType type, const QString &value);
    ~CalcNode();

    NodeType type();
    QString value();
    void setType(NodeType newType);
    void setValue(const QString &newValue);
    bool allowsAdd();
    bool allowsEdit();
    double value(const QStringList &row, const QStringList &colNames);
    int days(const QStringList &row, const QStringList &colNames);
    QString description();
    QString equation(bool useParens=false);
    static QStringList listOperations();
    CalcNodeList getChildren();
    void addChild(CalcNode *child);
    void removeChild(CalcNode *child);
    void swapChildren(int i, int j);
    CalcNode *clone();
    int maxChildren();
    bool deleteColumn(const QString &name);
    void renameColumn(const QString &oldName, const QString &newName);

private:
    static QString description(NodeType type, const QString &value);

private:
    NodeType nodeType; /**< The type of node that the object represents */
    QString nodeValue; /**< The value of this node (column name, constant value, etc.) */
    CalcNodeList children; /**< This node's child nodes, if any */
};

#endif
