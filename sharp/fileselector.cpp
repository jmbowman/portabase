/*
 * pbfileselector.cpp
 *
 * (c) 2003-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qpe/config.h>
#include <qdir.h>
#include <qfile.h>
#include "fileselector.h"

PBFileSelector::PBFileSelector(const QString &, const QStringList &recentFiles, const QString &f, QWidget *parent, const char *name)
  : SlFileSelector("", f, parent, name)
{
    setRecent(recentFiles);
    int count = recentFiles.count();
    for (int i = 0; i < count; i++) {
        if (QFile::exists(recentFiles[i])) {
            setCurrentFile(recentFiles[i]);
            break;
        }
    }
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
    f->setType("application/portabase");
    f->setFile(path);
    return f;
}

void PBFileSelector::passSelection(const QFileInfo &file)
{
    selection.setName(file.baseName());
    selection.setType("application/portabase");
    selection.setFile(file.absFilePath());
    emit fileSelected(selection);
}

void PBFileSelector::initFile(const DocLnk &doc)
{
    QFile file(doc.file());
    if (!file.open(IO_WriteOnly|IO_Raw)) {
	return;
    }
    file.writeBlock("", 0);
    file.close();
}
