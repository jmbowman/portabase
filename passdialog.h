/*
 * passdialog.h
 *
 * (c) 2003-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef PASSDIALOG_H
#define PASSDIALOG_H

#include "qqdialog.h"

#define OPEN_PASSWORD 0
#define NEW_PASSWORD 1
#define CHANGE_PASSWORD 2

class Database;
class QLineEdit;

class PasswordDialog: public QQDialog
{
    Q_OBJECT
public:
    PasswordDialog(Database *dbase, int dlgMode, QWidget *parent = 0,
                   const char *name = 0);
    ~PasswordDialog();

    bool validate();

private:
    Database *db;
    int mode;
    QLineEdit *oldPass;
    QLineEdit *pass;
    QLineEdit *repeatPass;
};

#endif
