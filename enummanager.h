/*
 * enummanager.h
 *
 * (c) 2002-2004,2008-2009 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file enummanager.h
 * Header file for EnumManager
 */

#ifndef ENUMMANAGER_H
#define ENUMMANAGER_H

#include "pbdialog.h"

class Database;
class QListWidget;

/**
 * Dialog for managing a database's enumeration column types.
 */
class EnumManager: public PBDialog
{
    Q_OBJECT
public:
    EnumManager(Database *dbase, QWidget *parent = 0);

    void applyChanges();
    bool changesMade();

private slots:
    void addEnum();
    void editEnum();
    void deleteEnum();
    void moveUp();
    void moveDown();

private:
    QListWidget *listWidget; /**< Display list of all defined enumerations */
    Database *db; /**< The database being edited */
    bool contentChanged; /**< True if enumerations have been added, edited, or deleted */
    bool orderChanged; /**< True if enumerations have been reordered */
};

#endif
