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

#include <qdialog.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qmessagebox.h>
#include "applnk.h"
#include "newfiledialog.h"
#include "qpeapplication.h"
#include "../qqdialog.h"

NewFileDialog::NewFileDialog(const QString &extension, QWidget *parent)
  : QObject(), parentWidget(parent), encrypted(FALSE)
{
    filename = "";
    ext = extension;
}

NewFileDialog::~NewFileDialog()
{

}

int NewFileDialog::exec()
{
    filename = "";
    encrypted = FALSE;
    QString filter;
    if (ext == ".pob") {
        filter = tr("PortaBase files") + " (*.pob)";
    }
    else if (ext == ".csv") {
        filter = tr("Text files with comma separated values") + " (*.csv)";
    }
    else if (ext == ".txt") {
#if defined(Q_WS_WIN)
        filter = tr("Text files") + " (*.txt)";
#else
        filter = QString::null;
#endif
    }
    else {
        filter = tr("XML files") + " (*.xml)";
    }
    filename = QFileDialog::getSaveFileName(QPEApplication::documentDir(),
                   filter, parentWidget, "new file dialog",
                   tr("Choose a filename to save under"));
    if (filename.isEmpty()) {
        return QDialog::Rejected;
    }
    if (QFile::exists(filename)) {
        int overwrite = QMessageBox::warning(parentWidget,
                             QQDialog::tr("PortaBase"),
                             tr("File already exists; overwrite it?"),
                             QObject::tr("Yes"), QObject::tr("No"),
                             QString::null, 1);
        if (overwrite == 1) {
            return QDialog::Rejected;
        }
    }
    if (ext == ".pob") {
        QMessageBox crypt(QQDialog::tr("PortaBase"),
                          tr("Encrypt the file?"),
                          QMessageBox::NoIcon, QMessageBox::Yes,
                          QMessageBox::No | QMessageBox::Default,
                          QMessageBox::NoButton, parentWidget);
        int result = crypt.exec();
        if (result == QMessageBox::Cancel) {
            return QDialog::Rejected;
        }
        else if (result == QMessageBox::Yes) {
            encrypted = TRUE;
        }
    }
    return QDialog::Accepted;
}

DocLnk *NewFileDialog::doc()
{
    if (filename.isEmpty()) {
        return 0;
    }
    QFileInfo info(filename);
    DocLnk *f = new DocLnk();
    if (ext == ".pob") {
        f->setType("application/portabase");
    }
    else if (ext == ".csv") {
        f->setType("text/x-csv");
    }
    else {
        f->setType("text/xml");
    }
    f->setName(info.baseName());
    f->setFile(info.dirPath(TRUE) + "/" + info.baseName() + ext);
    QPEApplication::setDocumentDir(info.dirPath(TRUE));
    return f;
}

bool NewFileDialog::encryption()
{
    return encrypted;
}
