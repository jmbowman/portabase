/*
 * filemanager.cpp
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <errno.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <stdlib.h>
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
    if (::rename(fn.latin1(), f.file().latin1()) < 0) {
	qWarning("problem renaming file %s to %s, errno: %d", fn.latin1(),
		 f.file().latin1(), errno);
	// remove the tmp file, otherwise, it will just lay around...
	QFile::remove(fn.latin1());
    }
    return TRUE;
}
