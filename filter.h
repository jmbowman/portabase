/*
 * filter.h
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FILTER_H
#define FILTER_H

#include <mk4.h>

class Condition;
class Database;

class Filter
{
public:
    Filter(Database *dbase, QString name);
    ~Filter();
    QString getName();
    void setName(QString newName);
    int getConditionCount();
    Condition *getCondition(int index);
    void addCondition(Condition *condition);
    void deleteCondition(int index);
    bool moveConditionUp(int index);
    bool moveConditionDown(int index);
    c4_View apply(c4_View dbview);

private:
    Database *db;
    QString filterName;
    c4_View mapping;
    c4_IntProp mIndex;
    c4_IntProp mPointer;
};

#endif
