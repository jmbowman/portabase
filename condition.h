/*
 * condition.h
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef CONDITION_H
#define CONDITION_H

#define EQUALS 0
#define CONTAINS 1
#define STARTSWITH 2
#define LESSTHAN 3
#define GREATERTHAN 4
#define LESSEQUAL 5
#define GREATEREQUAL 6
#define NOTEQUAL 7

#include <qstringlist.h>

class c4_View;
class Database;

class Condition
{
public:
    Condition(Database *dbase);
    ~Condition();
    QString getColName();
    void setColName(QString newName);
    int getOperator();
    void setOperator(int op);
    QString getConstant();
    void setConstant(QString newConstant);
    bool isCaseSensitive();
    void setCaseSensitive(bool y);
    c4_View filter(c4_View dbview);
    QString getDescription();
    void updateDescription();
    static QString getOperatorText(int op);

private:
    c4_View filterInt(c4_View dbview);
    c4_View filterString(c4_View dbview);
    c4_View filterFloat(c4_View dbview);

private:
    Database *db;
    QString colName;
    int operation;
    QString constant;
    bool caseSensitive;
    QStringList textCols;
    QString description;
};

#endif
