/*
 * newfiledialog.cpp
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include "newfiledialog.h"

NewFileDialog::NewFileDialog(QWidget *parent, const char *name, WFlags f) : QDialog(parent, name, TRUE, f)
{
    setCaption(tr("PortaBase"));
    QVBoxLayout *vbox = new QVBoxLayout(this);
    QLabel *label = new QLabel(tr("Enter a name for the new file"), this);
    vbox->addWidget(label);
    nameBox = new QLineEdit(this);
    vbox->addWidget(nameBox);
    encrypt = new QCheckBox(tr("Encrypted file"), this);
    vbox->addWidget(encrypt);
}

NewFileDialog::~NewFileDialog()
{

}

QString NewFileDialog::name()
{
    return nameBox->text();
}

bool NewFileDialog::encryption()
{
    return encrypt->isChecked();
}
