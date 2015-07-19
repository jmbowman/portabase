/*
 * filtereditor.h
 *
 * (c) 2002-2004,2008 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file filtereditor.h
 * Header file for FilterEditor
 */

#ifndef FILTEREDITOR_H
#define FILTEREDITOR_H

#include "pbdialog.h"

class ConditionEditor;
class Database;
class Filter;
class QLineEdit;
class QListWidget;

/**
 * Dialog for editing a Filter.
 */
class FilterEditor: public PBDialog
{
    Q_OBJECT
public:
    FilterEditor(QWidget *parent = 0);
    ~FilterEditor();

    int edit(Database *subject, const QString &filterName);
    void applyChanges();
    QString getName();

private:
    void updateList();
    bool hasValidName();

private slots:
    void addCondition();
    void editCondition();
    void deleteCondition();
    void moveUp();
    void moveDown();

private:
    QLineEdit *nameBox; /**< Entry field for the filter's name */
    QListWidget *listWidget; /**< Display of the list of conditions */
    Database *db; /**< The database being edited */
    QString originalName; /**< The initial name of the filter being edited */
    Filter *filter; /**< The filter being edited */
    ConditionEditor *conditionEditor; /**< The dialog to use for editing conditions */
};

#endif
