/*
 * passdialog.cpp
 *
 * (c) 2003-2004,2008-2010,2017 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file passdialog.cpp
 * Source file for PasswordDialog
 */

#include <QApplication>
#include <QDialogButtonBox>
#include <QLabel>
#include <QMessageBox>
#include "database.h"
#include "factory.h"
#include "passdialog.h"
#include "encryption/crypto.h"
#include "qqutil/qqlineedit.h"

/**
 * Constructor.
 *
 * @param dbase The database being created, opened, or modified
 * @param dlgMode The mode in which this dialog will be used
 * @param parent This dialog's parent widget
 */
PasswordDialog::PasswordDialog(Database *dbase, DialogMode dlgMode, QWidget *parent)
  : PBDialog(tr("Password"), parent, true), db(dbase), mode(dlgMode)
{
    QGridLayout *grid = Factory::gridLayout(vbox);
    int currentRow = 0;
    if (mode == ChangePassword) {
        grid->addWidget(new QLabel(tr("Old password") + ":", this), 0, 0);
        oldPass = new QQLineEdit(this);
        oldPass->setEchoMode(QLineEdit::Password);
        grid->addWidget(oldPass, 0, 1);
        currentRow++;
    }
    QString label((mode == ChangePassword) ? tr("New password") : tr("Password"));
    grid->addWidget(new QLabel(label + ":", this), currentRow, 0);
    pass = new QQLineEdit(this);
    pass->setEchoMode(QLineEdit::Password);
    grid->addWidget(pass, currentRow, 1);
    currentRow++;
    if (mode != OpenFile) {
        label = (mode == NewPassword) ? tr("Repeat password") : tr("Repeat new password");
        grid->addWidget(new QLabel(label + ":", this), currentRow, 0);
        repeatPass = new QQLineEdit(this);
        repeatPass->setEchoMode(QLineEdit::Password);
        grid->addWidget(repeatPass, currentRow, 1);
    }
#if defined(Q_OS_ANDROID)
    if (mode == OpenFile) {
        pass->setEnterKeyType(Qt::EnterKeyDone);
    }
    else {
        repeatPass->setEnterKeyType(Qt::EnterKeyDone);
    }
    vbox->addStretch(1);
#endif
    finishLayout();
}

/**
 * Validate the entered password(s).  May fail if the incorrect password for
 * an existing file is entered, if two newly-entered passwords don't match,
 * if a newly-specified password isn't deemed suitable, etc.
 *
 * @return True if the password(s) have been accepted, false otherwise
 */
bool PasswordDialog::validate()
{
    QString password = pass->text();
    if (mode == OpenFile) {
        QString error = db->encryption()->setPassword(password, false);
        if (!error.isEmpty()) {
            QMessageBox::warning(this, qApp->applicationName(), error);
            return false;
        }
        error = db->load();
        if (!error.isEmpty()) {
            QMessageBox::warning(this, qApp->applicationName(), error);
            return false;
        }
    }
    else if (mode == NewPassword) {
        QString repeatPassword = repeatPass->text();
        if (password != repeatPassword) {
            QMessageBox::warning(this, qApp->applicationName(),
                                 tr("Repeated password doesn't match"));
            return false;
        }
        QString error = db->encryption()->setPassword(password, true);
        if (!error.isEmpty()) {
            QMessageBox::warning(this, qApp->applicationName(), error);
            return false;
        }
    }
    else {
        QString oldPassword = oldPass->text();
        QString repeatPassword = repeatPass->text();
        if (password != repeatPassword) {
            QMessageBox::warning(this, qApp->applicationName(),
                                 tr("Repeated password doesn't match"));
            return false;
        }
        QString error = db->encryption()->changePassword(oldPassword, password);
        if (!error.isEmpty()) {
            QMessageBox::warning(this, qApp->applicationName(), error);
            return false;
        }
    }
    return true;
}
