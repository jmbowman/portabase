/*
 * pbfileselector.cpp
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qpe/filemanager.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qmessagebox.h>
#include "fileselector.h"
#include "inputdialog.h"

PBFileSelector::PBFileSelector(const QString &, const QStringList &, const QString &f, QWidget *parent, const char *name)
  : FileSelector(f, parent, name, FALSE, FALSE)
{

}

PBFileSelector::~PBFileSelector()
{

}

// meaningless in this version
QString PBFileSelector::currentDir()
{
    return "";
}

// meaningless in this version
QStringList PBFileSelector::recent()
{
    return QStringList();
}

bool PBFileSelector::duplicate()
{
    const DocLnk *selection = selected();
    if (selection == 0) {
        return FALSE;
    }
    bool ok;
    QString name = InputDialog::getText(tr("PortaBase"),
                                        tr("Enter a name for the new file"),
                                        QLineEdit::Normal, QString::null,
                                        &ok, this);
    if (ok && !name.isEmpty()) {
        DocLnk copy;
        configureDocLnk(copy, name);
        FileManager fm;
        ok = fm.copyFile(*selection, copy);
    }
    if (ok) {
        reread();
    }
    else {
        QMessageBox::warning(this, tr("PortaBase"),
                             tr("Unable to create new file"));
    }
    delete selection;
    return ok;
}

bool PBFileSelector::rename()
{
    const DocLnk *selection = selected();
    if (selection == 0) {
        return FALSE;
    }
    bool ok;
    QString name = InputDialog::getText(tr("PortaBase"),
                                        tr("Enter the new file name"),
                                        QLineEdit::Normal, QString::null, &ok,
                                        this);
    if (ok) {
        ok = !name.isEmpty();
    }
    if (ok) {
        DocLnk copy;
        configureDocLnk(copy, name);
        // actual moving would be more efficient, but harder to implement...
        FileManager fm;
        if (fm.copyFile(*selection, copy)) {
            QFile::remove(selection->file());
            QFile::remove(selection->linkFile());
            ok = TRUE;
        }
        else {
            ok = FALSE;
        }
    }
    if (ok) {
        reread();
    }
    else {
        QMessageBox::warning(this, tr("PortaBase"),
                             tr("Unable to rename the file"));
    }
    delete selection;
    return ok;
}

void PBFileSelector::initFile(const DocLnk &doc)
{
    FileManager fm;
    fm.saveFile(doc, "");
}

void PBFileSelector::configureDocLnk(DocLnk &doclnk, const QString &name)
{
    doclnk.setType("application/portabase");
    QString filename(name);
    if (filename.length() > 40) {
        filename = filename.left(40);
    }
    doclnk.setName(filename);
    QString defaultFile = doclnk.file();
    QFileInfo info(defaultFile);
    // calling file() created an empty file, delete it now
    QFile::remove(defaultFile);
    doclnk.setFile(info.dirPath(true) + "/" + info.baseName() + ".pob");
}
