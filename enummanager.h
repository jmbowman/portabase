/*
 * enummanager.h
 *
 * (c) 2002-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef ENUMMANAGER_H
#define ENUMMANAGER_H

#include "pbdialog.h"

class Database;
class QListBox;

class EnumManager: public PBDialog
{
    Q_OBJECT
public:
    EnumManager(Database *dbase, QWidget *parent = 0, const char *name = 0);
    ~EnumManager();

    void applyChanges();
    bool changesMade();

private slots:
    void addEnum();
    void editEnum();
    void deleteEnum();
    void moveUp();
    void moveDown();

private:
    QListBox *listBox;
    Database *db;
    bool contentChanged;
    bool orderChanged;
};

#endif
