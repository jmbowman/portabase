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

#include <qpe/config.h>
#include <qpe/filemanager.h>
#include <qdir.h>
#include "fileselector.h"

PBFileSelector::PBFileSelector(const QString &, const QStringList &recentFiles, const QString &f, QWidget *parent, const char *name)
  : SlFileSelector("", f, parent, name)
{
    setRecent(recentFiles);
    setSelected();
    connect(this, SIGNAL(fileSelected(const QFileInfo &)),
            this, SLOT(passSelection(const QFileInfo &)));
}

PBFileSelector::~PBFileSelector()
{

}

void PBFileSelector::reread()
{
    QDir current(currentDir());
    createFileList(current);
    setSelected();
}

const DocLnk *PBFileSelector::selected()
{
    QString path = currentFile();
    if (path.length() == 0|| path[path.length() - 1] == '*') {
        // no file selected
        return 0;
    }
    QFileInfo info(path);
    DocLnk *f = new DocLnk();
    f->setName(info.baseName());
    f->setFile(path);
    f->setType("application/portabase");
    return f;
}

void PBFileSelector::passSelection(const QFileInfo &file)
{
    selection.setName(file.baseName());
    selection.setFile(file.absFilePath());
    selection.setType("application/portabase");
    emit fileSelected(selection);
}

bool PBFileSelector::copyFile(const DocLnk &src, const DocLnk &dst)
{
    FileManager fm;
    bool ok = fm.copyFile(src, dst);
    if (ok) {
        QFile::remove(dst.linkFile());
    }
    return ok;
}

bool PBFileSelector::renameFile(const DocLnk &src, const DocLnk &dst)
{
    // actual moving would be more efficient, but harder to implement...
    if (copyFile(src, dst)) {
        QFile::remove(src.file());
        // just in case there is one, copied from an old ROM or whatever
        QFile::remove(src.linkFile());
        return TRUE;
    }
    return FALSE;
}

void PBFileSelector::initFile(const DocLnk &doc)
{
    FileManager fm;
    fm.saveFile(doc, "");
    // don't need .desktop files in the newer ROMs
    QFile::remove(doc.linkFile());
}
