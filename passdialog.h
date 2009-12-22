/*
 * passdialog.h
 *
 * (c) 2003-2004,2009 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file passdialog.h
 * Header file for PasswordDialog
 */

#ifndef PASSDIALOG_H
#define PASSDIALOG_H

#include "qqutil/qqdialog.h"

class Database;
class QLineEdit;

/**
 * Dialog for entering file encryption passwords.  Used when creating a new
 * encrypted file, opening an encrypted file, or changing the password on an
 * open encrypted file.
 */
class PasswordDialog: public QQDialog
{
    Q_OBJECT
public:
    /** Enumeration of possible purposes in launching this dialog */
    enum DialogMode {
        OpenFile = 0,
        NewPassword = 1,
        ChangePassword = 2
    };

    PasswordDialog(Database *dbase, DialogMode dlgMode, QWidget *parent = 0);

    bool validate();

private:
    Database *db; /**< The database being created, opened, or changed */
    DialogMode mode; /**< The purpose for which this dialog was launched */
    QLineEdit *oldPass; /**< Entry field for the old password, when changing it */
    QLineEdit *pass; /**< Entry field for the password */
    QLineEdit *repeatPass; /**< Entry field for confirming the entered password */
};

#endif
