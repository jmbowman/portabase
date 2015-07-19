/*
 * enumeditor.h
 *
 * (c) 2002-2004,2008-2010 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file enumeditor.h
 * Header file for EnumEditor
 */

#ifndef ENUMEDITOR_H
#define ENUMEDITOR_H

#include <mk4.h>
#include "pbdialog.h"

#define ADD_OPTION 0
#define RENAME_OPTION 1
#define DELETE_OPTION 2

class Database;
class QLabel;
class QLineEdit;
class QListWidget;
class QStackedWidget;

/**
 * Dialog for editing the name and option list of an enumerated column type.
 * Used by EnumManager.
 */
class EnumEditor: public PBDialog
{
    Q_OBJECT
public:
    EnumEditor(QWidget *parent = 0);

    int edit(Database *subject, const QString &enumName);
    void applyChanges();
    QString getName();

private:
    void updateList();
    bool hasValidName();
    bool isValidOption(const QString &option);
    QStringList listCurrentOptions();

private slots:
    void sortOptions();
    void importOptions();
    void exportOptions();
    void addOption();
    void editOption();
    void deleteOption();
    void moveUp();
    void moveDown();

private:
    /** Enumeration of option sorting states */
    enum OptionSorting {
        NotSorted = 0,
        Ascending = 1,
        Descending = 2
    };
    QLineEdit *nameBox; /**< Entry field for the enumeration's name */
    QStackedWidget *stack; /**< Option data widget stack (option list and "no options" label) */
    QListWidget *listWidget; /**< Display of the enumeration options */
    QLabel *noOptions; /**< "No options" placeholder label */
    Database *db; /**< The database being edited */
    QString originalName; /**< The name of the enumeration before editing started */
    c4_View info; /**< Data table for the current enumeration options */
    c4_StringProp eeiName; /**< Column for enumeration option names */
    c4_IntProp eeiIndex; /**< Column for enumeration option position indices */
    c4_View changes; /**< Data table for changes made to enumeration options */
    c4_IntProp eecIndex; /**< Column for the sequence of changes made */
    c4_IntProp eecType; /**< Column for the type of changes made */
    c4_StringProp eecOldName; /**< Column for the original names of options */
    c4_StringProp eecNewName; /**< Column for the new names of options */
    OptionSorting sorting; /**< The type of option sorting currently in use */
};

#endif
