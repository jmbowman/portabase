/*
 * enummanager.h
 *
 * (c) 2002-2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef ENUMMANAGER_H
#define ENUMMANAGER_H

#include <qdialog.h>

class Database;
class QListBox;

class EnumManager: public QDialog
{
    Q_OBJECT
public:
    EnumManager(Database *dbase, QWidget *parent = 0, const char *name = 0,
                WFlags f = 0);
    ~EnumManager();

    void applyChanges();

private slots:
    void addEnum();
    void editEnum();
    void deleteEnum();
    void moveUp();
    void moveDown();

private:
    QListBox *listBox;
    Database *db;
};

#endif
