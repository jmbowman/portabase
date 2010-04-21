/*
 * filter.h
 *
 * (c) 2002,2008-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file filter.h
 * Header file for Filter
 */

#ifndef FILTER_H
#define FILTER_H

#include <QList>
#include <QString>
#include <mk4.h>

class Condition;
class Database;

/**
 * A named collection of conditions for filtering the rows of a database
 * which are to be displayed.  Currently, only the logical "AND" of all
 * the component Conditions is supported.  Filter names beginning with an
 * underscore are reserved for PortaBase internal usage, such as "_allrows",
 * "_none", and "_simple".
 */
class Filter
{
public:
    Filter(Database *dbase, const QString &name);
    ~Filter();
    QString getName();
    void setName(const QString &newName);
    int getConditionCount();
    Condition *getCondition(int index);
    void addCondition(Condition *condition, int index=-1);
    void deleteCondition(int index);
    bool moveConditionUp(int index);
    bool moveConditionDown(int index);
    c4_View apply(c4_View dbview);

private:
    Database *db; /**< The database that this filter belongs to */
    QString filterName; /**< The name of this filter */
    QList<Condition *> conditions; /**< List of Conditions in this filter */
};

#endif
