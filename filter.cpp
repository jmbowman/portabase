/*
 * filter.cpp
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "condition.h"
#include "database.h"
#include "filter.h"

Filter::Filter(Database *dbase, QString name)
    : mIndex("_mindex"), mPointer("_mpointer")
{
    db = dbase;
    filterName = name;
    int count = db->getConditionCount(name);
    for (int i = 0; i < count; i++) {
        Condition *condition = db->getCondition(name, i);
        mapping.Add(mIndex [i] + mPointer [(int)condition]);
    }
}

Filter::~Filter()
{
    int size = mapping.GetSize();
    for (int i = 0; i < size; i++) {
        int pointer = mPointer (mapping[i]);
        Condition *condition = (Condition*)pointer;
        delete condition;
    }
}

QString Filter::getName()
{
    return filterName;
}

void Filter::setName(QString newName)
{
    filterName = newName;
}

int Filter::getConditionCount()
{
    return mapping.GetSize();
}

Condition *Filter::getCondition(int index)
{
    int mapIndex = mapping.Find(mIndex [index]);
    int pointer = mPointer (mapping[mapIndex]);
    return (Condition*)pointer;
}

void Filter::addCondition(Condition *condition, int index)
{
    int nextIndex = index;
    if (nextIndex == -1) {
        nextIndex = mapping.GetSize();
    }
    mapping.Add(mIndex [nextIndex] + mPointer [(int)condition]);
}

void Filter::deleteCondition(int index)
{
    int mapIndex = mapping.Find(mIndex [index]);
    int pointer = mPointer (mapping[mapIndex]);
    Condition *condition = (Condition*)pointer;
    delete condition;
    mapping.RemoveAt(mapIndex);
    int count = mapping.GetSize();
    for (int i = 0; i < count; i++) {
        int condIndex = mIndex (mapping[i]);
        if (condIndex > index) {
            mIndex (mapping[i]) = condIndex - 1;
        }
    }
}

bool Filter::moveConditionUp(int index)
{
    if (index == 0) {
        return FALSE;
    }
    int rowIndex = mapping.Find(mIndex [index]);
    int aboveRowIndex = mapping.Find(mIndex [index - 1]);
    mIndex (mapping[rowIndex]) = index - 1;
    mIndex (mapping[aboveRowIndex]) = index;
    return TRUE;
}

bool Filter::moveConditionDown(int index)
{
    if (index == getConditionCount() - 1) {
        return FALSE;
    }
    int rowIndex = mapping.Find(mIndex [index]);
    int belowRowIndex = mapping.Find(mIndex [index + 1]);
    mIndex (mapping[rowIndex]) = index + 1;
    mIndex (mapping[belowRowIndex]) = index;
    return TRUE;
}

c4_View Filter::apply(c4_View dbview)
{
    int count = getConditionCount();
    c4_View result = dbview;
    for (int i = 0; i < count; i++) {
        Condition *condition = getCondition(i);
        result = condition->filter(result);
    }
    return result;
}
