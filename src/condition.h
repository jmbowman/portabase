/*
 * condition.h
 *
 * (c) 2002-2004,2008-2009,2015 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file condition.h
 * Header file for Condition
 */

#ifndef CONDITION_H
#define CONDITION_H

#include <QObject>
#include <QStringList>

class c4_View;
class Database;

/**
 * This class represents a single condition in a Filter.  The general form
 * of a condition is "{column} {operator} {constant}", e.g. 'Description
 * contains "yellow"' or 'Priority < 3'.
 */
class Condition : public QObject
{
    Q_OBJECT
public:
    /** Enumeration of possible condition operators */
    enum Operator {
        Equals = 0,
        Contains = 1,
        StartsWith = 2,
        LessThan = 3,
        GreaterThan = 4,
        LessEqual = 5,
        GreaterEqual = 6,
        NotEqual = 7
    };

    explicit Condition(Database *dbase);

    QString getColName();
    void setColName(const QString &newName);
    Operator getOperator();
    void setOperator(Operator op);
    QString getConstant();
    void setConstant(const QString &newConstant);
    bool isCaseSensitive();
    void setCaseSensitive(bool y);
    c4_View filter(c4_View dbview);
    QString getDescription();
    void updateDescription();

private:
    static QString getOperatorText(Operator op);
    c4_View filterInt(c4_View dbview, const QString &colId, int value);
    c4_View filterString(c4_View dbview);
    c4_View filterFloat(c4_View dbview);

private:
    Database *db; /**< The database in use */
    QString colName; /**< The name of the column this condition applies to */
    Operator operation; /**< The ID of the operation this condition uses */
    QString constant; /**< String representation of the constant being compared against */
    bool caseSensitive; /**< True for case-sensitive text comparisons */
    QStringList textCols; /**< Names of all text columns in the database */
    QString description; /**< A text description of this condition, as shown in the UI */
};

#endif
