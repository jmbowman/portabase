/*
 * newfiledialog.cpp
 *
 * (c) 2003-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qpe/applnk.h>
#include <qcheckbox.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include "newfiledialog.h"
#include "qqdialog.h"

NewFileDialog::NewFileDialog(const QString &extension, QWidget *parent, const char *name, WFlags f) : QDialog(parent, name, TRUE, f)
{
    ext = extension;
    if (ext == ".pob") {
        setCaption(QQDialog::tr("PortaBase"));
    }
    else {
        setCaption(tr("Export"));
    }
    QVBoxLayout *vbox = new QVBoxLayout(this);
    QLabel *label = new QLabel(tr("Enter a name for the new file"), this);
    vbox->addWidget(label);
    nameBox = new QLineEdit(this);
    vbox->addWidget(nameBox);
    if (ext == ".pob") {
        encrypt = new QCheckBox(tr("Encrypted file"), this);
        vbox->addWidget(encrypt);
    }
    else {
        encrypt = 0;
    }
}

NewFileDialog::~NewFileDialog()
{

}

DocLnk *NewFileDialog::doc()
{
    QString name = nameBox->text();
    if (name.isEmpty()) {
        return 0;
    }
    DocLnk *f = new DocLnk();
    if (ext == ".pob") {
        f->setType("application/portabase");
    }
    else if (ext == ".csv") {
        f->setType("text/x-csv");
    }
    else if (ext == ".txt") {
        f->setType("text/plain");
    }
    else {
        f->setType("text/xml");
    }
    if (name.length() > 40) {
        name = name.left(40);
    }
    f->setName(name);
    QString defaultFile = f->file();
    QFileInfo info(defaultFile);
    // calling file() created an empty file, delete it now
    QFile::remove(defaultFile);
    f->setFile(info.dirPath(true) + "/" + info.baseName() + ext);
    return f;
}

bool NewFileDialog::encryption()
{
    if (encrypt == 0) {
        return FALSE;
    }
    return encrypt->isChecked();
}
