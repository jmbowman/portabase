/*
 * calcnode.h
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef CALCNODE_H
#define CALCNODE_H

#define CALC_CONSTANT 0
#define CALC_COLUMN 1
#define CALC_DATE_CONSTANT 2
#define CALC_DATE_COLUMN 3

#define CALC_FIRST_OP 20
#define CALC_ADD 20
#define CALC_SUBTRACT 21
#define CALC_MULTIPLY 22
#define CALC_DIVIDE 23
#define CALC_DAYS 24
#define CALC_MAX 25
#define CALC_MIN 26
#define CALC_AVERAGE 27
#define CALC_ABS 28
#define CALC_SQRT 29
#define CALC_LOG 30
#define CALC_LN 31
#define CALC_LAST_OP 31

#include <qstringlist.h>

class CalcNode;
class Database;

typedef QValueList<CalcNode*> CalcNodeList;

class CalcNode
{
public:
    CalcNode(int type, const QString &value);
    ~CalcNode();

    int type();
    QString value();
    void setType(int newType);
    void setValue(const QString &newValue);
    bool allowsAdd();
    bool allowsEdit();
    double value(const QStringList &row, const QStringList &colNames);
    int days(const QStringList &row, const QStringList &colNames);
    QString description(Database *db);
    QString equation(Database *db, bool useParens=FALSE);
    static QStringList listOperations();
    CalcNodeList getChildren();
    void addChild(CalcNode *child);
    void removeChild(CalcNode *child);
    void moveChild(CalcNode *child, CalcNode *after);
    CalcNode *clone();
    int maxChildren();

private:
    static QString description(Database *db, int type, const QString &value);

private:
    int nodeType;
    QString nodeValue;
    CalcNodeList children;
};

#endif
