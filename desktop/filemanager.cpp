/*
 * filemanager.cpp
 *
 * (c) 2003-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include "applnk.h"
#include "filemanager.h"

FileManager::FileManager()
{
}

FileManager::~FileManager()
{

}

bool FileManager::saveFile(const DocLnk &f, const QString &text)
{
    QString fn = f.file() + ".new";
    QFile fl(fn);
    if (!fl.open(IO_WriteOnly|IO_Raw)) {
        qDebug("open failed: %s", fn.latin1());
        return FALSE;
    }

    QCString cstr = text.utf8();
    int total_written;
    total_written = fl.writeBlock(cstr.data(), cstr.length());
    fl.close();
    if (total_written != int(cstr.length())) {
        QFile::remove(fn);
        return FALSE;
    }
    // okay now rename the file...
    QFileInfo info(f.file());
    QDir dir(info.dir());
    if (!dir.rename(info.fileName() + ".new", info.fileName())) {
        qWarning("problem renaming file %s to %s", fn.latin1(),
                 f.file().latin1());
        // remove the tmp file, otherwise, it will just lay around...
        QFile::remove(fn);
    }
    return TRUE;
}
