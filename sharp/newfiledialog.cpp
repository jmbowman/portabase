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

#include <qpe/applnk.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qmessagebox.h>
#include "newfiledialog.h"

NewFileDialog::NewFileDialog(const QString &extension, QWidget *parent, const char *name, WFlags f) : SlFileDialog(TRUE, parent, name, TRUE, f), ext(extension), encrypted(FALSE)
{
    setMimeType(mimeType());
    setNewDirEnabled(TRUE);
    setIconViewType(FALSE);
}

NewFileDialog::~NewFileDialog()
{

}

int NewFileDialog::exec()
{
    int result = SlFileDialog::exec();
    if (!result) {
        return result;
    }
    QString path = getFilePath();
    QFileInfo info(path);
    path = info.dirPath(true) + "/" + info.baseName() + ext;
    if (QFile::exists(path)) {
        int overwrite = QMessageBox::warning(this, tr("PortaBase"),
                            tr("File already exists; overwrite it?"),
                            tr("Yes"), tr("No"), QString::null, 1);
        if (overwrite == 1) {
            return QDialog::Rejected;
        }
    }
    if (ext == ".pob") {
        QMessageBox crypt(tr("PortaBase"), tr("Encrypt the file?"),
                          QMessageBox::NoIcon, QMessageBox::Yes,
                          QMessageBox::No | QMessageBox::Default,
                          QMessageBox::NoButton, this);
        result = crypt.exec();
        if (result == QMessageBox::Cancel) {
            return QDialog::Rejected;
        }
        else if (result == QMessageBox::Yes) {
            encrypted = TRUE;
        }
    }
    return result;
}

void NewFileDialog::accept()
{
    // at least the version on the SL-C760 seems to not do much by itself...
    SlFileDialog::accept();
    QDialog::accept();
}

DocLnk *NewFileDialog::doc()
{
    QString path = getFilePath();
    if (path.isEmpty()) {
        return 0;
    }
    DocLnk *f = new DocLnk();
    //f->setType(mimeType());
    QFileInfo info(path);
    f->setName(info.baseName());
    f->setFile(info.dirPath(true) + "/" + info.baseName() + ext);
    return f;
}

bool NewFileDialog::encryption()
{
    return encrypted;
}

QString NewFileDialog::mimeType()
{
    if (ext == ".pob") {
        return "application/portabase";
    }
    else if (ext == ".csv") {
        return "text/x-csv";
    }
    else if (ext == ".txt") {
        return "text/plain";
    }
    else {
        return "text/xml";
    }
}
