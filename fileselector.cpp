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

bool PBFileSelector::copyFile(const DocLnk &src, const DocLnk &dst)
{
    FileManager fm;
    return fm.copyFile(src, dst);
}

bool PBFileSelector::renameFile(const DocLnk &src, const DocLnk &dst)
{
    // actual moving would be more efficient, but harder to implement...
    if (copyFile(src, dst)) {
        QFile::remove(src.file());
        QFile::remove(src.linkFile());
        return TRUE;
    }
    return FALSE;
}

void PBFileSelector::initFile(const DocLnk &doc)
{
    FileManager fm;
    fm.saveFile(doc, "");
}
