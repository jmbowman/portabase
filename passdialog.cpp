/*
 * passdialog.cpp
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#if defined(DESKTOP)
#include <qhbox.h>
#include <qpushbutton.h>
#include "desktop/resource.h"
#endif

#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include "database.h"
#include "passdialog.h"
#include "pbdialog.h"

PasswordDialog::PasswordDialog(Database *dbase, int dlgMode, QWidget *parent, const char *name, WFlags f) : QDialog(parent, name, TRUE, f), db(dbase), mode(dlgMode)
{
    setCaption(tr("PortaBase") + PBDialog::titleSuffix);
    // cheated a bit; ordered mode codes so this works
    int rows = mode + 1;
#if defined(DESKTOP)
    QGridLayout *grid = new QGridLayout(this, rows + 1, 2);
#else
    QGridLayout *grid = new QGridLayout(this, rows, 2);
#endif

    int currentRow = 0;
    if (mode == CHANGE_PASSWORD) {
        grid->addWidget(new QLabel(tr("Old password") + ":", this), 0, 0);
        oldPass = new QLineEdit(this);
        oldPass->setEchoMode(QLineEdit::Password);
        grid->addWidget(oldPass, 0, 1);
        currentRow++;
    }
    QString label;
    if (mode == CHANGE_PASSWORD) {
        label = tr("New password");
    }
    else {
        label = tr("Password");
    }
    grid->addWidget(new QLabel(label + ":", this), currentRow, 0);
    pass = new QLineEdit(this);
    pass->setEchoMode(QLineEdit::Password);
    grid->addWidget(pass, currentRow, 1);
    currentRow++;
    if (mode != OPEN_PASSWORD) {
        if (mode == NEW_PASSWORD) {
            label = tr("Repeat password");
        }
        else {
            label = tr("Repeat new password");
        }
        grid->addWidget(new QLabel(label + ":", this), currentRow, 0);
        repeatPass = new QLineEdit(this);
        repeatPass->setEchoMode(QLineEdit::Password);
        grid->addWidget(repeatPass, currentRow, 1);
        currentRow++;
    }

#if defined(DESKTOP)
    QHBox *hbox = new QHBox(this);
    new QWidget(hbox);
    QPushButton *okButton = new QPushButton(tr("OK"), hbox);
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    new QWidget(hbox);
    QPushButton *cancelButton = new QPushButton(tr("Cancel"), hbox);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    new QWidget(hbox);
    grid->addMultiCellWidget(hbox, currentRow, currentRow, 0, 1);
    grid->setResizeMode(QLayout::FreeResize);
    setMinimumWidth(parent->width() / 2);
    setIcon(Resource::loadPixmap("portabase"));
#endif
}

PasswordDialog::~PasswordDialog()
{

}

bool PasswordDialog::validate()
{
    QString password = pass->text();
    if (mode == OPEN_PASSWORD) {
        QString error = db->setPassword(password, FALSE);
        if (error != "") {
            QMessageBox::warning(this, tr("PortaBase"), error);
            return FALSE;
        }
        error = db->load();
        if (error != "") {
            QMessageBox::warning(this, tr("PortaBase"), error);
            return FALSE;
        }
    }
    else if (mode == NEW_PASSWORD) {
        QString repeatPassword = repeatPass->text();
        if (password != repeatPassword) {
            QMessageBox::warning(this, tr("PortaBase"),
                                 tr("Repeated password doesn't match"));
            return FALSE;
        }
        QString error = db->setPassword(password, TRUE);
        if (error != "") {
            QMessageBox::warning(this, tr("PortaBase"), error);
            return FALSE;
        }
    }
    else {
        QString oldPassword = oldPass->text();
        QString repeatPassword = repeatPass->text();
        if (password != repeatPassword) {
            QMessageBox::warning(this, tr("PortaBase"),
                                 tr("Repeated password doesn't match"));
            return FALSE;
        }
        QString error = db->changePassword(oldPassword, password);
        if (error != "") {
            QMessageBox::warning(this, tr("PortaBase"), error);
            return FALSE;
        }
    }
    return TRUE;
}
