/*
 * fileselector.cpp
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qfiledialog.h>
#include <qfileinfo.h>
#include "filemanager.h"
#include "fileselector.h"
#include "qpeapplication.h"

PBFileSelector::PBFileSelector(const QString &dirPath, const QStringList &recentFiles, const QString&, QWidget *parent, const char *name)
  : QLabel(parent, name), parentWidget(parent), lastDir(dirPath), files(recentFiles)
{
    setText("<center>" + tr("No file selected") + "</center>");
}

PBFileSelector::~PBFileSelector()
{

}

const DocLnk *PBFileSelector::selected()
{
    QString file = QFileDialog::getOpenFileName(lastDir,
                       tr("PortaBase files") + " (*.pob)",
                       parentWidget, "file selector", tr("Choose a file"));
    if (!file.isEmpty()) {
        QFileInfo info(file);
        QPEApplication::setDocumentDir(info.dirPath(TRUE));
        files.prepend(info.absFilePath());
        if (files.count() > 5) {
            files.remove(files.fromLast());
        }
        return new DocLnk(file);
    }
    return 0;
}

void PBFileSelector::reread()
{
    // don't need to do anything since there's no file list displayed
}

QString PBFileSelector::currentDir()
{
    return lastDir;
}

QStringList PBFileSelector::recent()
{
    return files;
}

void PBFileSelector::initFile(const DocLnk &doc)
{
    FileManager fm;
    fm.saveFile(doc, "");
}
