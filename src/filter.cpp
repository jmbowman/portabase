/*
 * filter.cpp
 *
 * (c) 2002,2008-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file filter.cpp
 * Source file for Filter
 */

#include "condition.h"
#include "database.h"
#include "filter.h"

/**
 * Constructor.
 *
 * @param dbase The database to which this filter belongs
 * @param name The name of this filter
 */
Filter::Filter(Database *dbase, const QString &name)
{
    db = dbase;
    filterName = name;
    int count = db->getConditionCount(name);
    for (int i = 0; i < count; i++) {
        conditions << db->getCondition(name, i);
    }
}

/**
 * Destructor.
 */
Filter::~Filter()
{
    while (!conditions.isEmpty()) {
        delete conditions.takeFirst();
    }
}

/**
 * Get the name of this filter.
 *
 * @return The filter's name
 */
QString Filter::getName() const
{
    return filterName;
}

/**
 * Set the name of this filter.
 *
 * @param newName This filter's new name
 */
void Filter::setName(const QString &newName)
{
    filterName = newName;
}

/**
 * Get the number of conditions that this filter contains.
 */
int Filter::getConditionCount()
{
    return conditions.count();
}

/**
 * Get the Condition with the specified position index.
 *
 * @param index The position index of the Condition to be retrieved
 * @return A pointer to the desired Condition
 */
Condition *Filter::getCondition(int index)
{
    return conditions[index];
}

/**
 * Add a new condition to the filter.  A position can be specified, but this
 * should only be done in cases like file import where it is known that the
 * specified position won't clash with any other indices; no effort is made
 * here to avoid two Conditions having the same position.
 *
 * @param condition A pointer to the Condition to be added
 * @param index The position index at which to insert the new Condition
 */
void Filter::addCondition(Condition *condition, int index)
{
    if (index == -1) {
        conditions << condition;
    }
    else {
        if (index < conditions.count()) {
            conditions[index] = condition;
        }
        else {
            while (conditions.count() < index) {
                conditions << 0;
            }
            conditions << condition;
        }
    }
}

/**
 * Remove the condition at the specified position index from this filter.
 * Adjusts the position indices of later conditions accordingly.
 *
 * @param index The position index of the condition to remove
 */
void Filter::deleteCondition(int index)
{
    Condition *condition = conditions.takeAt(index);
    delete condition;
}

/**
 * Move the condition at the specified position index up by one in the
 * sequence.  Adjusts the positions of other conditions accordingly.
 * Returns false if an invalid operation was specified (like trying to
 * move the first condition up).
 *
 * @param index The position index of the condition to be moved
 * @return True if a change was made, false otherwise
 */
bool Filter::moveConditionUp(int index)
{
    if (index <= 0 || index >= conditions.count()) {
        return false;
    }
    conditions.swap(index, index - 1);
    return true;
}

/**
 * Move the condition at the specified position index down by one in the
 * sequence.  Adjusts the positions of other conditions accordingly.
 * Returns false if an invalid operation was specified (like trying to
 * move the last condition down).
 *
 * @param index The position index of the condition to be moved
 * @return True if a change was made, false otherwise
 */
bool Filter::moveConditionDown(int index)
{
    if (index < 0 || index >= conditions.count() - 1) {
        return false;
    }
    conditions.swap(index, index + 1);
    return true;
}

/**
 * Apply this filter to the provided data table and return the result.
 *
 * @param dbview The data table to be filtered
 * @return The filtered data table
 */
c4_View Filter::apply(c4_View dbview)
{
    int count = conditions.count();
    c4_View result = dbview;
    for (int i = 0; i < count; i++) {
        result = conditions[i]->filter(result);
    }
    return result;
}
